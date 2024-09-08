/*
  Ed Davis, 2022, public domain
  Minor tweaks for SCC compatibility, error checking, fixes for the most
    flagrant bugs, and a few extra features by Daniel E. Gaskell, 2024

  Loosely based (but greatly expanded) on Zserge's 166 line toy basic -
  https://zserge.com/posts/transpilers/ - which in turn was
  apparently loosely based on the IOCC Basic interpreter entry -
  https://github.com/ioccc-src/winner/blob/master/1991/dds.ansi.c

  To compile: gcc.exe -Wall -Wextra -pedantic -s -Os basic.c -o basic
      SymbOS: cc -h 16384 -N "Tiny BASIC" -o basic.com basic.c
  Notice that the same code can run unmodified on GCC and SCC!

  Implements enough BASIC to play Tiny Star Trek (roughly Palo Alto Tiny BASIC):
    - Variables: A-Z, 16-bit signed integer only
    - Arrays: one @() array with 1024 entries
    - Operators: (, ), ^, MOD, *, /, +, 0, <, >, <=, >=, <>, =, AND, OR, NOT
    - Functions: ABS(n), ASC("c"), RND(n), SGN(n), SQR(n), FRE
    - Commands: var = expr, END, GOTO line, GOSUB line, RETURN, NEXT, REM, '
                IF expr GOTO/GOSUB line
                IF expr THEN ...
                FOR var = expr TO expr
                PRINT "Text"; #width, expr;
                INPUT "Prompt: ", var
    - Interactive commands: RUN, LIST, NEW, SAVE, LOAD, BYE

  This interpreter is not actually a very good BASIC for SymbOS (it's limited
  and its evaluation method is *extremely* slow) but serves as an interesting
  demo of SCC's capabilities.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <malloc.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define LINE_MAX    255
#define ARRAYSIZE   1024
#define CODELEN     16250
#define GOSUBS      64
int vars[27];             /* global variables, 'a'..'z' */
char line[LINE_MAX + 1];  /* current input line */
char* code;               /* complete program code */
char* codeend;            /* end of used code buffer */
char *ip;                 /* current pointer */
int at[ARRAYSIZE];        /* the @() array */
int gsp, gosub_line[GOSUBS], for_line[27], for_limit[27];
char *gosub_off[GOSUBS], *for_off[27];

static int expr(int minprec);

/* reads a number from a stream */
static int num(void) { return strtol(ip, &ip, 10); }

/* returns the end of the current line */
static char *eol(void) { return strchr(ip, '\n'); }

/* skips whitespace */
void space(void) {
    int c;
    for (; ((c = *ip) != 0) && (c == ' ' || c == '\t'); ip++);
}

/* skips until whitespace */
void token(void) {
    int c;
    for (; ((c = *ip) != 0) && (c != ' ' && c != '\t'); ip++);
}

static int var(void) {  /* return index of the variable at current position */
  int c;
  space();
  c = *ip | 0x20;           /* lowercase the character */
  if (isalpha(c)) {
    ip++;
    return c - 'a' + 1;
  }
  return 0;
}

static int accept(const char *s) {
  size_t n;
  space();
  n = strlen(s);
  if (eol() - ip < (int)n) { return false; }
  if (memicmp(s, ip, n) == 0) {
    // if last char of s is alpha, make sure next char of ip is not alnum
    if (isalpha(s[0]) && isalpha(ip[n])) { return false; }
    ip += n;
    return true;
  }
  return false;
}

static int expect(const char *s) {
  if (accept(s)) { return true; }
  printf("expecting %s, but found %.*s\n", s, (int)MIN(strlen(ip), 40), ip);
  return false;
}

static int sqrti(unsigned int s) {
  unsigned int x0, x1;
  if (s <= 1)
    return s;
  x0 = s / 2;
  x1 = (x0 + s / x0) / 2;
  while (x1 < x0) {
    x0 = x1;
    x1 = (x0 + s / x0) / 2;
  }
  return x0;
}

static int pexpr(void) {
  int n;
  space();
  expect("(");
  n = expr(0);
  expect(")");
  return n;
}

static int expr(int minprec) {
  int n;
  space();
  if (accept("-"))          { n = -expr(7);
  } else if(accept("+"))    { n = expr(7);
  } else if (isdigit(*ip))  { n = num();
  } else if (accept("(")) {
    n = expr(0);
    expect(")");
  } else if (accept("not")) { n = !expr(3);
  } else if (accept("@"))   { n = at[pexpr()];
  } else if (accept("abs")) { n = abs(pexpr());
  } else if (accept("asc")) { // asc("x")
    expect("(");
    expect("\"");
    n = *ip++;
    expect("\"");
    expect(")");
  } else if (accept("rnd") || accept("irnd")) { n = rand() % pexpr() + 1;
  } else if (accept("sgn"))  { n = pexpr(); n = (n > 0) - (n < 0);
  } else if (accept("sqr"))  { n = pexpr(); n = sqrti(abs(n));
  } else if (accept("fre"))  { n = CODELEN - (codeend - code);
  } else if (isalpha(ip[0])) { n = vars[var()];
  } else {
    printf("expecting a variable, but found %.*s\n", (int)MIN(strlen(ip), 20), ip);
    return 0;
  }

  for (;;) {
    if        (minprec <= 1 && accept("or"))  {n = n | expr(2);
    } else if (minprec <= 2 && accept("and")) {n = n & expr(3);

    } else if (minprec <= 4 && accept("="))  {n = n == expr(5);
    } else if (minprec <= 4 && accept("<>")) {n = n != expr(5);
    } else if (minprec <= 4 && accept("<=")) {n = n <= expr(5);
    } else if (minprec <= 4 && accept(">=")) {n = n >= expr(5);
    } else if (minprec <= 4 && accept("<"))  {n = n <  expr(5);
    } else if (minprec <= 4 && accept(">"))  {n = n >  expr(5);

    } else if (minprec <= 5 && accept("+"))  {n = n +  expr(6);
    } else if (minprec <= 5 && accept("-"))  {n = n -  expr(6);

    } else if (minprec <= 6 && accept("*"))  {n = n *  expr(7);
    } else if (minprec <= 6 && accept("/"))  {n = n /  expr(7);
    } else if (minprec <= 6 && accept("\\")) {n = n /  expr(7);
    } else if (minprec <= 6 && accept("mod")){n = n %  expr(7);

    } else if (minprec <= 8 && accept("^"))  {n = pow(n, expr(9));

    } else break;
  }
  return n;
}

static char *find(int n) {  /* find line number 'n' in the code buffer */
  ip = code;
  while (*ip) {
    char *line = ip;
    int i = num();
    ip = eol() + 1;
    if (i >= n) { return ip = line; }
  }
  return ip = (code + strlen(code));
}

static int stmt(int curline) { // return 0 on error, 1 for continue, -1 to exit
  int line_num;
  int cz;
  char *s, *s2;
  int v, v2, v3, print_nl, width, ndx, gosub;
  again:
  s = ip;
  v = var();

  //----------[assign]-----------------------------------------------------
  if (accept("=")) {
    vars[v] = expr(0);
  //----------[@assign]-----------------------------------------------------
  } else if (accept("@")) {       // @(expr) = expr
    ndx = pexpr();
    if (!expect("=")) { return 0; }  //********* return 0 *********************
    if (ndx >= ARRAYSIZE) {
        printf("Array overflow\n");
        return 0;
    }
    at[ndx] = expr(0);
  } else {
      ip = s;
  //----------[end]-----------------------------------------------------
      if (accept("end") || accept("stop")) {
        return -1;                  //********* return -1 ********************
  //----------[for]-----------------------------------------------------
      } else if (accept("for")) {          // for i=expr to expr
          v2 = var();
          expect("=");        // skip '='
          vars[v] = expr(0);
          expect("to");       // skip "to"
          for_limit[v2] = expr(0);
          for_off[v2]   = ip;
          for_line[v2]  = curline;
  //----------[gosub/goto]-----------------------------------------------------
      } else if ((gosub = accept("gosub")) != 0 || accept("goto")) { // goto/gosub
          line_num = expr(0);
          if (gosub) {
            if (gsp >= GOSUBS) {
              printf("Too many nested GOSUBs\n");
              return 0;
            }
            gsp++;
            gosub_line[gsp] = curline;
            gosub_off[gsp]  = ip;
          }
          gotoline:

          find(line_num);
          if (num() == line_num) {
            curline = line_num;
            goto again;               //********* goto again *******************
          } else {
            printf("Line %d not found\n", line_num);
            return 0;                 //********* return 0 *********************
          }
  //----------[if]-----------------------------------------------------
      } else if (accept("if")) {
          if (expr(0)) {
            accept("then");
            line_num = num();
            if (line_num > 0) {
              goto gotoline;          //********* goto gotoline
            } else {
              goto again;             //********* goto again
            }
          } else {
            ip = eol(); return 1;      //********* return 1
          }
  //----------[input]-----------------------------------------------------
      } else if (accept("input")) { // input [string,] var
          if (accept("\"")) {
            for (; ((cz = *ip) != 0) && (cz != '"'); ip++) {
                putchar(cz);
            }
            expect("\"");
            expect(",");
          }
          v3 = var();
          s2 = ip;
          ip = fgets(line, LINE_MAX, stdin);
          vars[v3] = isdigit(*ip) ? expr(0) : *ip;
          ip = s2;
  //----------[next]-----------------------------------------------------
      } else if (accept("next")) {    // next i
          v3 = var();
          vars[v3]++;
          if (vars[v3] <= for_limit[v3]) {
            curline = for_line[v3];
            ip       = for_off[v3];
            goto again;               //********* goto again
          }
  //----------[print]-----------------------------------------------------
      } else if (accept("print") || accept("?")) { // 'print' [[#num ',' ] expr { ',' [#num ','] expr }] [','] {':' stmt} eol
          print_nl = 1;
          for (;;) {
              width = 0;
              space();
              if (*ip == ':' || *ip == '\n' || *ip == '\r' || *ip == '\'') { break; }
              print_nl = 1;
              if (accept("#")) {
                  width = expr(0);
                  expect(",");
              }

              if (accept("\"")) {
                for (; ((cz = *ip) != 0) && (cz != '"'); ip++) {
                    putchar(cz);
                }
                expect("\"");
              } else
                  printf("%*d", width, expr(0));

              if (accept(",") || accept(";")) {
                  print_nl = false;
              } else
                  break;
          }
          if (print_nl) { printf("\n"); }
  //----------[return]-----------------------------------------------------
      } else if (accept("return")) {
          curline = gosub_line[gsp];
          ip       = gosub_off[gsp];
          gsp--;
          goto again;               //********* goto again
      }
  }

  if (accept(":")) { goto again; }//********* goto again
  if (accept("'") || accept("rem")) {
    ip = eol(); return 1;          //********* return 1
  }
  space();
  if (ip[0] == '\r') ++ip;
  if (ip[0] == '\n') { return 1; }

  printf("Syntax error: %.*s\n", (int)MIN(strlen(ip), 40), ip);
  return 0;
}

static void run(void) {
  int n, n2;
  ip = code;
  for (;;) {
    n = num();
    if (n == 0) { break; }
    n2 = stmt(n);
    if (n2 == -1) { break; }
    if (n2 == 0) {
      printf("Err in line %i\n", n);
      break;
    }
  }
}

static void getfilename(void) {
  char *p;
  space();
  if (ip[0] == '\n' || ip[0] == '\r') {
    printf("File: ");
    ip = line;
    if (fgets(line, LINE_MAX, stdin) == NULL) return;
  }
  if (ip[0] == '\n' || ip[0] == '\r') { return; }
  if (ip[0] == '"') { ip++; }
  p = eol();
  p[0] = '\0';
  if (*--p == '"') { *p = '\0'; }
}

static void save(void) {
  FILE *fp;

  getfilename();
  if ((fp = fopen(ip, "w")) == NULL) {
    printf("File error: %s\n", ip);
    return;
  }
  fwrite(code, 1, strlen(code), fp);
  fclose(fp);
}

static void load(void) {
  FILE *fp;
  int readlen;

  getfilename();
  if ((fp = fopen(ip, "r")) == NULL) {
    printf("File not found: %s\n", ip);
    return;
  }
  code[0] = 0;
  codeend = code;
  printf("Loading...\n");
  readlen = fread(code, 1, CODELEN, fp);
  codeend += readlen;
  if (readlen == CODELEN)
    printf("Out of memory\n");
  /*while (fgets(line, LINE_MAX, fp) != NULL) { // FIXME: this is the original (awful) way of loading files, which occasionally triggered freezes on SymbOS - figure out why, but for now, the new way is 9283% better anyway.
    //printf(line);
    codeend += strlen(line);
    if (codeend >= code + CODELEN) {
      printf("Out of memory\n");
      break;
    }
    strcat(code, line);
  }*/
  fclose(fp);
  printf("OK\n");
  ip = code;
}

int main(int argc, char* argv[]) {
  int n, an, len;
  char *start;
  char *end;

  code = malloc(CODELEN);
  memset(code, 0, CODELEN);
  codeend = code;

  printf("Tiny BASIC, (C)2022 Ed Davis\n");
  printf("%i bytes free\n", CODELEN);
  printf("OK\n");

  for (;;) {
    if (fgets(line, LINE_MAX, stdin) == NULL) { break; }
    ip = line;
    if ((n = num()) != 0) {
      start = find(n);
      end = eol();
      an = num();
      len = strlen(line);
      if (codeend + len >= code + CODELEN) {
        printf("Out of memory\n");
        continue;
      }
      if (an == n) {  // found our line - replace it
        memmove(start, end + 1, strlen(end) + 1); // remove the existing line
        codeend -= (end + 1 - start);
      }
      if (an >= n) {  // insert at this position
        memmove(start + len, start, strlen(start) + 1); // make a hole
        codeend += (start - (start + len));
      } else {
        ++len; // to get zero terminator
      }
      memmove(start, line, len);
      codeend += len;
    } else {
      if      (accept("new"))  { code[0] = '\0'; }
      else if (accept("run"))  { run(); }
      else if (accept("list")) { puts(code); }
      else if (accept("load")) { load(); }
      else if (accept("save")) { save(); }
      else if (accept("bye") || accept("quit")) { break; }
      else { stmt(0); }  // try to run it
    }
  }
  return 0;
}
