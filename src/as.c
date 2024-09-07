/*
 * Assembler.
 * Command line processing
 * and main driver.
 *
 * FIXME: normal Unix as option parsing.
 */
#define TARGET_Z80

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "obj.h"
#include "as.h"

#include "as1.c"
#include "as2.c"
#include "as3.c"
#include "as4.c"
#include "as6.c"

int     ifp;
FILE	*ofp;
FILE	*lfp;
char	eb[NERR];
char	ib[NINPUT];
char	lb[NINPUT];
char    fb[NINBUF];
char	*cp;
char	*ep;
char	*ip;
char    *fp = NULL;
char    *fep = NULL;
char    *foep = NULL;
char    *fop;
char	*fname;
char 	*listname;
VALUE	dot[OSEG];
int	segment = 1;
SYM	*phash[NHASH];
SYM	*uhash[NHASH];
int	pass;
int	line;
jmp_buf	env;
int	debug_write = 1 ;
int	noobj;
int	cpu_flags = ARCH_CPUFLAGS;

static void usage(void)
{
	fprintf(stderr, "as [-o object.o] source.s.\n");
	exit(1);
}

static void oom(void)
{
	fprintf(stderr, "Out of memory.\n");
	exit(1);
}

static char *xstrdup(const char *p)
{
	char *n = strdup(p);
	if (!n)
		oom();
	return n;
}

static int listbytes;

static void list_header(void)
{
	fprintf(lfp, "%c %04X : ", "ACDBZXSLsbdt"[segment], dot[segment]);
	listbytes = 0;
}

static void list_beginline(void)
{
	if (pass !=3 || !lfp)
		return;
	strcpy(lb, ib);	/* Save the input buffer */
	list_header();
}

void list_addbyte(uint8_t byte)
{
	if (pass == 3 && lfp) {
		/* Deal with wrapping nicely (eg for .ascii) */
		if (listbytes == 8) {
			fputs(lb, lfp);
			strcpy(lb, "...\n");
			list_header();
		}
		listbytes++;
		fprintf(lfp, "%02X ", byte);
	}
}

void list_endline(void)
{
	if (pass == 3 && lfp) {
		while(listbytes++ < 8)
			fputs("   ", lfp);
		fputs(lb, lfp);
	}
}

// buffered line read (much faster than fgets on SymbOS)
unsigned char getlineb(char* buf, unsigned int maxlen, int fid) {
    #ifndef SYMBUILD
    int oldseek;
    #endif
    int readlen;
    unsigned char c;
    fop = buf;
    foep = fop + maxlen;
    while (fop < foep) {
        if (fp >= fep) {
            #ifndef SYMBUILD
            oldseek = lseek(fid, 0, SEEK_CUR); // get read length manually - mingw problem
            read(fid, fb, NINBUF);
            readlen = lseek(fid, 0, SEEK_CUR) - oldseek;
            #else
            readlen = read(fid, fb, NINBUF);
            #endif
            if (readlen == 0) {
                *fop = 0;
                return fop != buf; // still return success if we've read something
            }
            fp = fb;
            fep = fb + readlen;
        }
        c = *fp++;
        if (c == 0x1A) { // AMSDOS EOF
            fep = fp;
            break;
        }
        if (c != '\r')
            *fop++ = c;
        if (c == '\n')
            break;
    }
    *fop = 0;
    return 1;
}

int main(int argc, char *argv[])
{
	char *ifn;
	char *ofn = NULL;
	char *p, *e;

	int opt;

	/* Lots of options need adding yet */
	while ((opt = getopt(argc, argv, "o:l:")) != -1) {
		switch (opt) {
		case 'o':
			ofn = optarg;
			break;
		case 'l':
			listname = optarg;
			break;
		default:
			usage();
			break;
		}
	}
	if (optind != argc - 1)
		usage();
	ifn = argv[optind];

	if ((ifp=open(ifn, O_RDONLY | O_BINARY)) == -1) {
		fprintf(stderr, "%s: cannot open.\n", ifn);
		exit(BAD);
	}

	if (ofn == NULL) {
		ofn = xstrdup(ifn);
		p = strrchr(ofn, '.');
		if (p == NULL || p[1] == 0) {
			fprintf(stderr, "%s: expected extensions.\n", ifn);
			exit(BAD);
		}
		p[1] = 'o';
		p[2] = '\0';
	}

	if ((ofp=fopen(ofn, "wb")) == NULL) {
		fprintf(stderr, "%s: cannot create.\n", ofn);
		exit(BAD);
	}
	if (listname) {
		lfp = fopen(listname, "wb");
		if (lfp == NULL) {
			fprintf(stderr, "%s: cannot create.\n", listname);
			exit(BAD);
		}
	}

	syminit();
	fname = xstrdup(ifn);
	for (pass=0; pass<4; ++pass) {
		if (outpass() == 0)
			continue;
		line = 1;
		memset(dot, 0, sizeof(dot));
		lseek(ifp, 0L, 0);
		while (getlineb(ib, NINPUT, ifp)) {
			/* Pre-processor output */
			if (*ib == '#' && ib[1] == ' ') {
				free(fname);
				line = strtoul(ib +2, &p, 10);
				p++;
				e = strrchr(p , '"');
				if (e)
					*e = 0;
				fname = xstrdup(p);
			/* Normal assembly */
			} else {
				list_beginline();
				ep = &eb[0];
				ip = &ib[0];
				if (setjmp(env) == 0)
					asmline();
				list_endline();
				++line;
			}
		}
		/* Don't continue once we know we failed */
		if (noobj)
			break;
	}
	if (!noobj) {
		pass = 3;
		outeof();
	} else {
		if (unlink(ofn))
			perror(ofn);
	}

	close(ifp);
	/* Return an error code if no object was created */
	exit(noobj);
}

