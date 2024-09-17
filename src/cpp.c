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
#define NAMELEN 32
#define TEXTLEN 256
#define PATHSTRLEN 128
#define LINELEN 256
#define BLOCKLEN 128
#define MAXVARS 8
#define MAXPATHS 16

// LUT for checking if a character is alphanumeric - optimization
char alphanum[128] =
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};

typedef struct {
    int fd;
    long seek;
    unsigned short line;
    char name[NAMELEN + 1];
    char path[PATHSTRLEN + 1];
} FileRecord;

char* symbol_first[96];
char* symbol_last[96];
char fblocks[MAXVARS][BLOCKLEN];

char *line1, *line2, *tokstr, *valstr, *inbuf, *outbuf, *textbuf, *path;
FileRecord files[MAXDEPTH];

char* paths[MAXPATHS];
unsigned char pathcount = 0;
char* apppath;

char* linein;
char* lineout;
char* inptr;
char* outptr;
char* outbufend;
char* tokstrend;
char* valstrend;
char* inlptr;
char* outlptr;

char* usage = "Usage: cpp -Dxxx -Uxxx -Ixxx infile -o outfile\n";
char* outfile = 0;
char* infile = 0;

signed char ifd, ofd;
unsigned char fr = 0;
unsigned char ifdepth = 0;
unsigned short line;
long seek;
unsigned char linenum_pending = 1;

// general globals for speed (careful!)
unsigned char gc, gn, state, inquotes, escaped, incomment;
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

void flush_out(int len) {
    if (len && *outbuf) {
        if (write(ofd, outbuf, len) == -1)
            fatal("cannot write");
        outptr = outbuf;
    }
}

void write_line(void) {
    int len = outlptr - lineout;
    if (len >= outptr - outbufend)
        flush_out(outptr - outbuf);
    memcpy(outptr, lineout, len);
    outptr += len;
    outlptr = lineout;
    *outlptr = 0;
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
    linenum_pending = 0;
    snprintf(textbuf, TEXTLEN, "# %i \"%s\"\n", line, files[fr].name);
    put_str(textbuf);
    write_line();
}

unsigned char get_token(void) {
    gc = *inlptr++;
    if (!gc) // EOL
        return 0;
    state = 0;
    escaped = 0;
    inquotes = 0;
    gptr = tokstr;
    *gptr = 0;
    for (;;) {
        if (gc == '"') {
            if (escaped == 0)
                inquotes ^= 1;
        }
        if (inquotes) {
            // output string
            put_char(gc);
            if (gc == '\\') {
                if (inquotes != 0 && escaped == 0)
                    escaped = 2;
            } else if (gc == '\n') { // EOL
                return 0;
            } else if (!gc) { // EOF
                return 0;
            }
        } else if (alphanum[gc]) {
            // start alphanumeric token
            state = 1;
            *gptr++ = gc;
        } else {
            if (state) {
                // return tokens in tokstr
                --inlptr;
                *gptr = 0;
                return 1;
            } else if (!gc) {
                // EOL
                *gptr = 0;
                return 0;
            } else if (gc == '(' || gc == ')' || gc == ',') {
                // return ( ) , as their own tokens, for define parsing
                *gptr++ = gc;
                *gptr = 0;
                return 1;
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
    if (symbol_last[branch])
        *(char**)symbol_last[branch] = ptr;
    symbol_last[branch] = ptr;

    // write symbol content
    *(char**)ptr = 0;
    ptr[sizeof(gptr)] = params;
    ptr += sizeof(gptr) + 1;
    memcpy(ptr, symbol, symbol_len);
    ptr += symbol_len;
    memcpy(ptr, val, val_len);
}

void add_symbol_long(char* symbol) {
    char* val = "";
    gptr = symbol;
    while (gc = *gptr) {
        if (gc == '=') {
            *gptr = 0;
            val = gptr + 1;
            break;
        }
        ++gptr;
    }
    add_symbol(symbol, val);
}

void remove_symbol(char* symbol) {
    unsigned char branch;
    char *ptr, *oldptr;
    branch = *symbol - ' ';
    if (ptr = symbol_first[branch]) {
        oldptr = ptr;
        while (ptr) {
            if (!strcmp(symbol, ptr + sizeof(gptr) + 1)) {
                // FIXME this will break the symbol_last logic! Do bidirectional chain?
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

unsigned char fill_buf(void) {
    int readlen;
    if (*inptr == 0x7F) {
        readlen = rread(ifd, inbuf, INBUF);
        if (readlen == 0)
            return 1;
        inptr = inbuf;
        inbuf[readlen] = 0x7F;
    }
    return 0;
}

unsigned char read_line(void) {
    gptr = linein;
    gptr2 = linein + LINELEN;
    inlptr = linein;
    outlptr = lineout;
    inquotes = 0;
    escaped = 0;
    for (;;) {
        if (gptr >= gptr2)
            fatal("line too long");
        if (fill_buf())
            goto truncated;
        gc = *inptr++;
        if (gc == '\\' && inquotes != 0 && escaped == 0) {
            escaped = 2;
        } else if (gc == '"' && escaped == 0) {
            inquotes ^= 1;
        } else if (gc == '/' && inquotes == 0) {
            fill_buf();
            if (*inptr == '/') { // single-line comment, skip rest of line
                *gptr++ = '\n';
                while (gc != '\n' && gc != 0x1A) {
                    if (fill_buf())
                        goto truncated;
                    gc = *inptr++;
                }
                break;
            } else if (*inptr == '*') {
                ++inptr;
                incomment = 1;
            }
        } else if (gc == 0x1A) { // AMSDOS EOF
            *inptr = 0x7F;
            break;
        } else if (gc == '\n') {
            *gptr++ = '\n';
            break;
        }
        if (incomment) {
            if (gc == '*') {
                if (fill_buf())
                    goto truncated;
                if (*inptr == '/') {
                    ++inptr;
                    incomment = 0;
                }
            }
        } else {
            *gptr++ = gc;
        }
        if (escaped)
            --escaped;
    }
    *gptr = 0;
    return 1;

truncated:
    *gptr = 0;
    return gptr != linein; // still return success if we've read something
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
            pp_file(valstr);

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
        put_char('\n');

    } else {
        // normal line, output tokens
        if (linenum_pending)
            output_linenum();
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
    if (*filename == '"' || *filename == '<') { // enclosed in brackets or quotes
        filename[strlen(filename) - 1] = 0; // strip terminator
        strncpy(files[fr].name, filename + 1, NAMELEN);
    } else { // passed directly
        strncpy(files[fr].name, filename, NAMELEN);
    }

    // open file
    gptr = files[fr].path;
    ifd = -1;
    if (*filename == '<') {
        // bracketed include, append to system include path
        strcpy(gptr, apppath);
        #ifdef SYMBUILD
        strcat(gptr, "/lib/include/");
        #else
        strcat(gptr, "/../lib/include/");
        #endif // SYMBUILD
        strcat(gptr, filename + 1);
        ifd = open(gptr, O_RDONLY | O_BINARY);
    } else if (*filename == '"') {
        // quoted include, first try local path
        if (fr == 1) {
            // first file - relative to shell/specified path
            strncpy(gptr, filename + 1, PATHSTRLEN);
        } else {
            // subsequent paths - relative to previous file's path
            strncpy(gptr, files[fr-1].path, PATHSTRLEN);
            gptr2 = strrchr(gptr, '/');
            if (!gptr2)
                gptr2 = strrchr(gptr, '\\');
            if (!gptr2)
                gptr2 = gptr - 1;
            *(gptr2 + 1) = 0;
            strcat(gptr, filename + 1);
        }
        ifd = open(gptr, O_RDONLY | O_BINARY);
        if (ifd == -1) {
            // failed - try registered paths
            for (i = 0; i < pathcount; ++i) {
                strcpy(gptr, paths[i]);
                strcat(gptr, "\\");
                strcat(gptr, filename + 1);
                ifd = open(gptr, O_RDONLY | O_BINARY);
                if (ifd != -1)
                    break;
            }
        }
        if (ifd == -1) {
            // still failed - try system path
            strcpy(gptr, apppath);
            #ifdef SYMBUILD
            strcat(gptr, "/lib/include/");
            #else
            strcat(gptr, "/../lib/include/");
            #endif // SYMBUILD
            strcat(gptr, filename + 1);
            ifd = open(gptr, O_RDONLY | O_BINARY);
        }
    } else {
        // passed as parameter, open directly
        strncpy(gptr, infile, PATHSTRLEN);
        ifd = open(gptr, O_RDONLY | O_BINARY);
    }
    if (ifd == -1)
        fatal("cannot open input file");

    // finish file open
    files[fr].fd = ifd;
    line = 1;
    seek = 0;

    // process file
    incomment = 0;
    inbuf[INBUF] = 0x7F; // end-of-buffer marker (optimization)
    i = rread(ifd, inbuf, INBUF);
    if (i) {
        inptr = inbuf;
        inbuf[i] = 0x7F;
        inlptr = linein;
        outlptr = lineout;
        output_linenum();
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
        inbuf[i] = 0x7F;
        inptr = inbuf;
        linenum_pending = 1;
    }
}

int main(int argc, char* argv[]) {
    int ar;

#ifndef SYMBUILD
    memset(symbol_first, 0, sizeof(symbol_first));
    memset(symbol_last, 0, sizeof(symbol_last));
#endif // SYMBUILD
    // malloc() buffers to reduce SymbOS file size
    line1 = malloc(LINELEN);
    line2 = malloc(LINELEN);
    tokstr = malloc(TOKLEN);
    valstr = malloc(VALLEN);
    inbuf = malloc(INBUF);
    outbuf = malloc(OUTBUF);
    textbuf = malloc(TEXTLEN);
    path = malloc(PATHSTRLEN);

    // get application path
    apppath = argv[0];
    gptr = strrchr(apppath, '/');
    if (!gptr)
        gptr = strrchr(apppath, '\\');
    if (!gptr)
        gptr = apppath;
    *gptr = 0;

    // process command-line options
    for (ar = 1; ar < argc; ar++) {
		if (argv[ar][0] == '-') {
			switch (argv[ar][1]) {
			case 'I':
                if (pathcount >= MAXPATHS) {
                    printf("cpp: too many paths specified\n");
                    exit(1);
                }
				if (argv[ar][2]) {
                    paths[pathcount] = argv[ar] + 2;
				} else {
					ar++;
					if (ar >= argc) {
						printf(usage);
						exit(1);
					}
                    paths[pathcount] = argv[ar];
				}
				++pathcount;
				break;
			case 'D':
				if (argv[ar][2]) {
                    add_symbol_long(argv[ar] + 2);
				} else {
					ar++;
					if (ar >= argc) {
						printf(usage);
						exit(1);
					}
                    add_symbol_long(argv[ar]);
				}
				break;
			case 'U':
				if (argv[ar][2]) {
                    remove_symbol(argv[ar] + 2);
				} else {
					ar++;
					if (ar >= argc) {
						printf(usage);
						exit(1);
					}
                    remove_symbol(argv[ar]);
				}
				break;
			case 'o':
			    if (outfile) {
			        printf(usage);
                    exit(1);
			    }
				if (argv[ar][2]) {
					outfile = argv[ar] + 2;
				} else {
					ar++;
					if (ar >= argc) {
						printf(usage);
						exit(1);
					}
					outfile = argv[ar];
				}
				break;
			default:
				printf("cpp: unknown option '%s'\n", argv[ar]);
				exit(1);
			}
		} else if (!infile) {
			infile = argv[ar];
		} else {
			printf(usage);
			exit(1);
		}
	}

	if (!infile) {
		printf(usage);
		exit(1);
	}
	if (!outfile)
        outfile = "$stream0.c";

//unsigned short test;
//while (test++) {
    memset(symbol_first, 0, sizeof(symbol_first));
    memset(symbol_last, 0, sizeof(symbol_last));
    #ifdef SYMBUILD
    ofd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
    #else
    ofd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0700);
    #endif // SYMBUILD
    if (ofd == -1)
        fatal("cannot open output stream");

    linein = line1;
    *linein = 0;
    lineout = line2;
    *lineout = 0;
    outptr = outbuf;
    outlptr = outptr;
    outbufend = outbuf + OUTBUF;
    tokstrend = tokstr + TOKLEN;
    valstrend = valstr + VALLEN;
    pp_file(infile);

    flush_out(outptr - outbuf);
    close(ofd);
//}
    return 0;
}
