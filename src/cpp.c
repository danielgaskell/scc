/*  Mini-CPP for SCC
    Copyright (C) 2024 Daniel E. Gaskell

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>

#define TOKLEN 64
#define VALLEN 256
#define INBUF 1024
#define OUTBUF 1024
#define MAXDEPTH 6
#define NAMELEN 13
#define TEXTLEN 256
#define LINELEN 256
#define BLOCKLEN 128
#define MAXVARS 8

/* TODO:
   - Path resolution
   - Comment stripping
   - Command-line options
*/

typedef struct {
    int fd;
    long seek;
    unsigned short line;
    char name[NAMELEN + 1];
} FileRecord;

char* symbol_first[96];
char line1[LINELEN];
char line2[LINELEN];
char fblocks[MAXVARS][BLOCKLEN];

char tokstr[TOKLEN];
char valstr[VALLEN];
char inbuf[INBUF];
char outbuf[OUTBUF];
char textbuf[TEXTLEN];
FileRecord files[MAXDEPTH];

char* linein;
char* lineout;
char* inptr;
char* outptr;
char* inbufend;
char* outbufend;
char* tokstrend;
char* valstrend;
char* inlptr;
char* outlptr;

signed char ifd, ofd;
unsigned char fr = 0;
unsigned char ifdepth = 0;
unsigned short line;
long seek;
unsigned char linenum_pending = 1;

// general globals for speed (careful!)
unsigned char gc, gn, state, inquotes, escaped;
unsigned int gi;
char *gptr, *gptr2, *gptr3;

#ifdef SYMBUILD
#define rread(x,y,z) read(x,y,z)
#else
// Expanded read() that more consistently returns read length (seems to be a mingw problem...?)
int rread(int fd, void* buf, unsigned int maxchars) {
    int preseek, readerr;
    preseek = lseek(fd, 0, SEEK_CUR);
    readerr = read(fd, buf, maxchars);
    if (readerr)
        return readerr;
    else
        return lseek(fd, 0, SEEK_CUR) - preseek;
}
#endif

void fatal(char* str) {
    printf("%s:%i: %s\n", files[fr].name, line, str);
    exit(1);
}

void* xmalloc(unsigned short len) {
    void* result = malloc(len);
    if (!result)
        fatal("out of memory");
    return result;
}

char* match_symbol(char* token) {
    unsigned char branch = *token - ' ';
    gptr = symbol_first[branch];

    while (gptr) {
        gptr2 = token;
        gptr3 = gptr + sizeof(gptr) + 1;
        while (*gptr2++ == *gptr3++) {
            if (*gptr2 == 0 && *gptr3 == 0)
                return gptr;
        }
        gptr = *(char**)gptr;
    }
    return 0;
}

unsigned char fill_inbuf(void) {
    if (inptr >= inbufend) {
        seek += (inbufend - inbuf);
        gi = rread(ifd, inbuf, INBUF);
        if (!gi)
            return 0;
        inptr = inbuf;
        inbufend = inbuf + gi;
    }
    return 1;
}

void flush_out(int len) {
    if (len && *outbuf) {
        if (write(ofd, outbuf, len) == -1)
            fatal("cannot write");
        outptr = outbuf;
    }
}

void put_char(unsigned char ch) {
    *outlptr++ = ch;
    // FIXME bounds check
}

void put_str(char* str) {
    gptr = str;
    while (gc = *gptr++)
        put_char(gc);
}

void output_linenum(void) {
    snprintf(textbuf, TEXTLEN, "# %i \"%s\"\n", line, files[fr].name);
    put_str(textbuf);
}

void resolve_path(char* str) {
    int len;
    if (*str == '"') {
        // literal path, easy (FIXME relative to source file!)
        strncpy(textbuf, str + 1, TEXTLEN);
        len = strlen(textbuf);
        if (textbuf[len-1] == '"')
            textbuf[len-1] = 0;
    } else if (*str == '<') {
        // internal path, check include dirs (FIXME!)
    } else {
        fatal("invalid path");
    }
}

unsigned char get_token(void) {
    gc = *inlptr++;
    if (!gc) // EOL
        return 0;
    state = 0;
    inquotes = 0;
    gptr = tokstr;
    *gptr = 0;
    for (;;) {
        if (gc == '\\' && inquotes != 0 && escaped == 0)
            escaped = 2;
        if (gc == '"' && escaped == 0)
            inquotes ^= 1;
        if (!inquotes && ((gc >= '0' && gc <= '9') || (gc >= 'A' && gc <= 'Z') || (gc >= 'a' && gc <= 'z') || (gc == '_'))) {
            state = 1;
            *gptr++ = gc;
        } else {
            if (state) {
                // return tokens in tokstr
                --inlptr;
                *gptr = 0;
                return 1;
            } else if (gc == '(' || gc == ')' || gc == ',') {
                // return ( ) , as their own tokens, for define parsing
                *gptr++ = gc;
                *gptr = 0;
                return 1;
            } else if (!gc) {
                // EOL
                *gptr = 0;
                return 0;
            } else {
                // otherwise just output intermediary text
                put_char(gc);
                if (gc == '\n')
                    return 0;
            }
        }
        gc = *inlptr++;
        if (escaped)
            --escaped;
    }
}

char func_vars[MAXVARS + 1];

// structure of a symbol record is:
//  - sizeof(char*): pointer to next (0 = end of this list)
//  - char: number of function parameters
//  - cstring: symbol
//    (first parentheses will be zeroed for matching purposes)
//  - cstring: value [parameters converted to high ASCII = 128, 129, etc.]
void add_symbol(char* symbol, char* val) {
    unsigned char i, branch, symbol_len, val_len, params = 0;
    char *ptr, *ptr2;

    // deal with function format
    ptr = strchr(symbol, '(');
    if (ptr) {
        // extract parameter values
        *ptr++ = 0;
        memset(func_vars, 0, sizeof(func_vars));
        while (gc = *ptr++) {
            if (gc == ',') {
                if (!func_vars[params])
                    fatal("missing parameter");
                ++params;
                if (params >= MAXVARS)
                    fatal("too many parameters");
            } else if (gc == ')') {
                gc = *ptr;
                if (gc != 0 && gc != ' ' && gc != '\t')
                    fatal("unexpected character");
                break;
            } else if (gc != ' ' && gc != '\t') {
                if (func_vars[params])
                    fatal("invalid parameter");
                func_vars[params] = gc;
            }
        }

        // convert equivalent parameters in value to high ASCII
        ptr = inlptr;   // hijack the token machinery to read/write from where we need (faster than passing pointers)
        ptr2 = outlptr;
        inlptr = val;
        outlptr = &fblocks[1][0];
        strcpy(&fblocks[0][0], symbol); // preserve symbol (previously in tokbuf, which is overwritten)
        while (gc = get_token()) {
            if (tokstr[0] != 0 && tokstr[1] == 0) { // one-char token, might be insert
                for (i = 0; i <= params; ++i) {
                    if (func_vars[i] == *tokstr) {
                        put_char(i + 128);
                        goto continue2;
                    }
                }
            }
            put_str(tokstr);
            continue2:;
        }
        *outlptr = 0;
        strcpy(symbol, &fblocks[0][0]);
        strcpy(val, &fblocks[1][0]);
        inlptr = ptr;
        outlptr = ptr2;
        ++params;
    }

    // write symbol
    symbol_len = strlen(symbol) + 1;
    val_len = strlen(val) + 1;
    ptr = xmalloc(symbol_len + val_len + sizeof(gptr) + 1);
    branch = *symbol - ' ';
    if (symbol_first[branch] == 0)
        symbol_first[branch] = ptr;

    // write symbol content
    *(char**)ptr = 0;
    ptr[sizeof(gptr)] = params;
    ptr += sizeof(gptr) + 1;
    memcpy(ptr, symbol, symbol_len);
    ptr += symbol_len;
    memcpy(ptr, val, val_len);
}

void remove_symbol(char* symbol) {
    unsigned char branch;
    char *ptr, *oldptr;
    branch = *symbol - ' ';
    if (ptr = symbol_first[branch]) {
        oldptr = ptr;
        while (ptr) {
            if (!strcmp(symbol, ptr + sizeof(gptr) + 1)) {
                *(char**)oldptr = *(char**)ptr; // unlink
                if (ptr == oldptr)
                    symbol_first[branch] = 0;
                return;
            }
            oldptr = ptr;
            ptr = *(char**)ptr;
        }
    }
    fatal("symbol not previously defined");
}

void skip_white(void) {
    while (*inlptr == ' ' || *inlptr == '\t')
        ++inlptr;
}

void define_out(char* def) {
    unsigned char paren_depth, in_quotes, param, need_param;
    char *ptr;

    if (need_param = def[sizeof(gptr)]) {
        // function format - read into blocks
        paren_depth = 0;
        in_quotes = 0;
        escaped = 0;
        param = 0;
        skip_white();
        ptr = outlptr; // hijack write machinery to go to block
        outlptr = &fblocks[0][0];
        while (gc = get_token()) {
            if (paren_depth == 0 && *tokstr != '(')
                fatal("expected '('");
            if (*tokstr == '(') {
                if (paren_depth)
                    put_char(*tokstr);
                ++paren_depth;
            } else if (*tokstr == ')') {
                if (paren_depth == 0)
                    fatal("mismatched parentheses");
                --paren_depth;
                if (!paren_depth)
                    break;
                put_char(*tokstr);
            } else if (*tokstr == ',') {
                if (paren_depth == 1) {
                    if (param == need_param)
                        fatal("too many parameters");
                    *outlptr = 0;
                    outlptr = &fblocks[++param][0];
                }
            } else {
                gptr = match_symbol(tokstr);
                if (gptr)
                    define_out(gptr);
                else
                    put_str(tokstr);
            }
            put_char(' ');
        }
        *outlptr = 0;
        if (paren_depth)
            fatal("mismatched parentheses");
        if (param < need_param - 1)
            fatal("too few parameters");

        // write blocks to line
        outlptr = ptr;
        ptr = def + strlen(def + sizeof(gptr) + 1) + sizeof(gptr) + 2;
        while (gc = *ptr++) {
            if (gc >= 128)
                put_str(&fblocks[gc - 128][0]);
            else
                put_char(gc);
        }
        *ptr = 0;

    } else {
        // normal format
        put_str(def + strlen(def + sizeof(gptr) + 1) + sizeof(gptr) + 2);
    }
}

void get_directive(void) {
    unsigned char paren_depth = 0;
    *tokstr = 0;
    gptr = tokstr;
    for (;;) {
        gc = *inlptr++;
        if (gc == ' ' && *inlptr == '(' && paren_depth == 0) // skip space in "dir (" format for directives
            gc = *inlptr++;
        if (gc == '(')
            ++paren_depth;
        if (gc == ')') {
            if (!paren_depth)
                fatal("mismatched parentheses");
            --paren_depth;
        }
        if (((gc == ' ' || gc == '\t') && !paren_depth) || gc == '\r' || gc == '\n' || gc == 0x1A || gc == 0) {
            *gptr = 0;
            return;
        }
        if (gptr < tokstrend)
            *gptr++ = gc;
    }
    if (paren_depth)
        fatal("mismatched parentheses");
}

void get_dirval(void) {
    skip_white();
    *valstr = 0;
    gptr = valstr;
    for (;;) {
        gc = *inlptr++;
        if (gc == '\r' || gc == '\n' || gc == 0x1A || gc == 0) {
            *gptr = 0;
            return;
        }
        if (gptr >= valstrend)
            fatal("define too long");
        *gptr++ = gc;
    }
}

unsigned char read_line(void) {
    int readlen;
    gptr = linein;
    gptr2 = linein + LINELEN;
    inlptr = linein;
    outlptr = lineout;
    for (;;) {
        if (gptr >= gptr2)
            fatal("line too long");
        if (inptr >= inbufend) {
            readlen = rread(ifd, inbuf, INBUF);
            if (readlen == 0) {
                *gptr = 0;
                return gptr != linein; // still return success if we've read something
            }
            inptr = inbuf;
            inbufend = inbuf + readlen;
        }
        gc = *inptr++;
        if (gc == 0x1A) { // AMSDOS EOF
            inbufend = inptr;
            break;
        }
        *gptr++ = gc;
        if (gc == '\n')
            break;
    }
    *gptr = 0;
    return 1;
}

void write_line(void) {
    int len = outlptr - lineout;
    if (len >= outptr - outbufend)
        flush_out(outptr - outbuf);
    memcpy(outptr, lineout, len);
    outptr += len;
}

void skip_block(void) {
    char depth = 1;
    while (depth) {
        read_line();
        skip_white();
        gc = *inlptr++;
        if (gc == '#') {
            get_directive();
            if (!strcmp(tokstr, "ifdef") || !strcmp(tokstr, "ifndef")) {
                ++depth;
            } else if (!strcmp(tokstr, "endif")) {
                --depth;
            } else if (depth == 1 && !strcmp(tokstr, "else")) {
                ++ifdepth;
                --depth;
            }
        }
        while (gc != '\n' && gc != 0 && gc != 0x1A)
            gc = *inlptr++;
        ++line;
    }
    linenum_pending = 1;
    --ifdepth;
}

// swap line-buffer pointers so last output becomes new input
void out_to_in(void) {
    char *tmpptr;
    tmpptr = linein;
    linein = lineout;
    lineout = tmpptr;
    inlptr = linein;
    outlptr = lineout;
    *outlptr = 0;
}

void pp_file(char* filename);

void pp_line_raw(void) {
    while (gc = get_token()) {
        gptr = match_symbol(tokstr);
        if (gptr)
            define_out(gptr);
        else
            put_str(tokstr);
        put_char(' ');
    }
}

unsigned char pp_line(void) {
    char* ptr;

    if (!read_line())
        return 0;
    skip_white();
    if (*inlptr == '#') {
        // directive
        ptr = inlptr;
        ++inlptr;
        get_directive();

        if (!strcmp(tokstr, "define")) {
            inlptr = ptr;  // rewind, then...
            pp_line_raw(); // ...process as a raw line to resolve symbols...
            out_to_in();   // ...and swap input/output and process as a directive
            skip_white();
            ++inlptr;
            get_directive();
            skip_white();
            get_directive();
            get_dirval();
            add_symbol(tokstr, valstr);

        } else if (!strcmp(tokstr, "include")) {
            get_dirval();
            resolve_path(valstr);
            pp_file(textbuf);

        } else if (!strcmp(tokstr, "ifdef")) {
            get_directive();
            ++ifdepth;
            if (!match_symbol(tokstr))
                skip_block();

        } else if (!strcmp(tokstr, "ifndef")) {
            get_directive();
            ++ifdepth;
            if (match_symbol(tokstr))
                skip_block();

        } else if (!strcmp(tokstr, "undef")) {
            skip_white();
            get_directive();
            remove_symbol(tokstr);

        } else if (!strcmp(tokstr, "else")) {
            if (!ifdepth)
                fatal("#else without #ifdef");
            else
                skip_block();

        } else if (!strcmp(tokstr, "endif")) {
            if (!ifdepth)
                fatal("#endif without #ifdef");
            else
                --ifdepth;

        } else if (!strcmp(tokstr, "if")) {
            fatal("#if not supported");

        } else if (!strcmp(tokstr, "elif")) {
            fatal("#elif not supported");

        } else {
            printf("-- '%'\n", tokstr);
            fatal("unknown directive");
        }
        linenum_pending = 1;

    } else {
        // normal line, output tokens
        if (linenum_pending) {
            output_linenum();
            linenum_pending = 0;
        }
        pp_line_raw();
    }
    ++line;
    write_line();
    return 1;
}

void pp_file(char* filename) {
    int i;

    // update old file record
    files[fr].line = line;
    files[fr].seek = seek + (inptr - inbuf);

    // set up new file record
    ++fr;
    if (fr > MAXDEPTH)
        fatal("too many nested includes");
    gptr = strrchr(filename, '\\');
    if (!gptr)
        gptr = strrchr(filename, '/');
    if (!gptr)
        gptr = filename;
    strncpy(files[fr].name, gptr, NAMELEN);

    // open file
    ifd = open(filename, O_RDONLY | O_BINARY);
    if (ifd == -1) {
        --fr; // so it prints the correct name
        fatal("cannot open input file");
    }
    files[fr].fd = ifd;
    line = 1;
    seek = 0;
    output_linenum();
    linenum_pending = 0;

    // process file
    inbufend = inbuf;
    i = rread(ifd, inbuf, INBUF);
    if (i) {
        inptr = inbuf;
        inbufend = inbuf + i;
        while (pp_line());
    }

    // return to correct place in prior file
    close(ifd);
    --fr;
    if (fr) {
        line = files[fr].line;
        ifd = files[fr].fd;
        seek = files[fr].seek;
        lseek(ifd, seek, SEEK_SET);
        i = rread(ifd, inbuf, INBUF);
        inbufend = inbuf + i;
        inptr = inbuf;
        linenum_pending = 1;
    }
}

int main(int argc, char* argv[]) {
    ofd = open("$stream0.c", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
    if (ofd == -1)
        fatal("cannot open output stream");

    linein = line1;
    *linein = 0;
    lineout = line2;
    *lineout = 0;
    outptr = outbuf;
    outlptr = outptr;
    outbufend = outbuf + OUTBUF;
    memset(symbol_first, 0, sizeof(symbol_first));
    tokstrend = tokstr + TOKLEN;
    valstrend = valstr + VALLEN;
    pp_file("test.c");

    flush_out(outptr - outbuf);
    close(ofd);
    return 0;
}
