/*
 *	It's easiest to think of what cc does as a sequence of four
 *	conversions. Each conversion produces the inputs to the next step
 *	and the number of types is reduced. If the step is the final
 *	step for the conversion then the file is generated with the expected
 *	name but if it will be consumed by a later stage it is a temporary
 *	scratch file.
 *
 *	Stage 1: (-c -o overrides object name)
 *
 *	Ending			Action
 *	$1.S			preprocessor - may make $1.s
 *	$1.s			nothing
 *	$1.c			preprocessor, no-op or /dev/tty
 *	$1.o			nothing
 *	$1.a			nothing (library)
 *
 *	Stage 2: (not -E)
 *
 *	Ending			Action
 *	$1.s			nothing
 *	$1.%			cc, opt - make $1.s
 *	$1.o			nothing
 *	$1.a			nothing (library)
 *
 *	Stage 3: (not -E or -S)
 *
 *	Ending			Action
 *	$1.s			assembler - makes $1.o
 *	$1.o			nothing
 *	$1.a			nothing (library)
 *
 *	Stage 4: (run if no -c -E -S)
 *
 *	ld [each .o|.a in order] [each -l lib in order] -lc
 *	(with -b -o $1 etc)
 *
 *	TODO:
 *
 *	Platform specifics
 *	Search library paths for libraries (or pass to ld and make ld do it)
 *	Turn on temp removal once confident
 *	Split I/D
 */

#define DEBUG

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
//#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef SYMBUILD
#include <symbos.h>
#include <iobuf.h>
#define CPATHSIZE	256
char* shell_append;
#else
#ifdef LINUXBUILD
#include "linux.h"
#include <sys/wait.h>
#define MAX_PATH 4096
#define CPATHSIZE (MAX_PATH+16)
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#else
#include <windows.h>
#define CPATHSIZE (MAX_PATH+16)
#endif
#endif
#include <malloc.h>

/*
 *	For all non native compilers the directories moved and the rules
 *	are
 *
 *	BINPATH
 *		as
 *		cc		(this)
 *		ld		(try and share)
 *		reloc		(try and share)
 *
 *	LIBPATH
 *		cpp		(shared by all)
 *		cc0		(possibly shared may need work)
 *		cc1.cpuid
 *		cc2.cpuid
 *		copt		(shared by all)
 *		copt.cpuname
 *		cpuname/lib/
 *		cpuname/include/
 *
 *	For the native compiler it gets built single CPU supporting
 *	as /bin/ccc /lib/cpp /lib/lib*.a /usr/include etc, but other processors
 *	are scanned in the usual rule.
 *
 *	Naming is two part
 *		cpuid		general naming for arch (eg 85, z80)
 *		cpuname		mach specific (z80,z180,8080,8085)
 */

#ifndef BINPATH
char BINPATH[MAX_PATH];
char LIBPATH[MAX_PATH];
#endif

struct obj {
	struct obj *next;
	char *name;
	uint8_t type;
#define TYPE_S			1
#define TYPE_C			2
#define TYPE_s			3
#define TYPE_C_pp		4
#define TYPE_O			5
#define TYPE_A			6
	uint8_t used;
};

struct objhead {
	struct obj *head;
	struct obj *tail;
};

struct objhead objlist;
struct objhead liblist;
struct objhead inclist;
struct objhead deflist;
struct objhead libpathlist;
struct objhead ccargs;		/* Arguments to pass on to the compiler */

struct cpu_table {
	const char *name;	/* Name to match */
	const char *set;	/* Binary names in bin */
	const char *cpudot;	/* .xxxx name for lib helpers */
	const char *lib;	/* System library path naming */
	const char *cpudir;	/* Directory for this CPU include etc (some may share) */
	const char **defines;	/* CPU defines */
	const char **ldopts;	/* LD link rules */
	const char *cpucode;	/* CPU code value for backend */
	unsigned has_reloc;	/* Has relocatable binary support */
	const char **cpufeat;	/* Feature option names in bit flag order */
};

const char *defz80[] = { "__z80__", NULL };
const char *z80feat[] = {
	"banked",
	"noix",
	"noiy",
	NULL
};

const char *ld8080[] = { "-b", "-C", "256", NULL };
const char *cpucode;

struct cpu_table cpu_rules = { "z80", "z80", ".z80", "libz80.a", "z80",
                               defz80, ld8080, "80" , 1, z80feat };

/* Need to set these via the cpu type lookup etc */
const char *cpuset;		/* Which binary compiler tool names */
const char *cpudot;		/* Which internal tool names */
const char *cpudir;		/* CPU specific directory */
const char *cpulib;		/* Dir for this compiler */
const char **cpudef;		/* List of defines */
const char **ldopts;		/* Linker opts for default link */
unsigned has_relocs;		/* Do we have relocations ? */
const char **feats;		/* CPU features */
unsigned long features;		/* Bit mask of feature info for pass 2 */

/* We will need to do more with ldopts for different OS and machine targets
   eventually */

const char *crtname = "crt0.o";

int print_passes=0;
int keep_temp;
int last_phase = 4;
int only_one_input;
char *target;
char *appname = NULL;
char *appicon = NULL;
char *appicon16 = NULL;
char *heapsize = "4096";
char heapsize_cust = 0;
int strip;
int c_files;
int standalone;
char *cpu = "z80";
int mapfile;

#define OS_NONE		0
#define OS_FUZIX	1
int targetos = OS_FUZIX;
int fuzixsub;
char optimize = '1';
char *codeseg;

char *symtab;

#define MAXARG	512

int arginfd, argoutfd;
const char *arglist[MAXARG];
const char **argptr;
char *rmlist[MAXARG];
char **rmptr = rmlist;

static void remove_temporaries(void)
{
	char **p = rmlist;
	while (p < rmptr) {
		if (keep_temp == 0)
			unlink(*p);
		free(*p++);
	}
	rmptr = rmlist;
}

static void fatal(void)
{
	remove_temporaries();
	if (symtab && keep_temp == 0)
		unlink(symtab);
	exit(1);
}

static void memory(void)
{
	fprintf(stderr, "cc: out of memory.\n");
	fatal();
}

static char *xstrdup(char *p, int extra)
{
	char *n = malloc(strlen(p) + extra + 1);
	if (n == NULL)
		memory();
	strcpy(n, p);
	return n;
}

#ifdef SYMBUILD
#define _P_WAIT 0
int _spawnvp(int mode, char* file, char* arglist[]) {
    unsigned char i, t, j, pid;
    char* ptr;
    _io_buf[0] = 0;
    t = 0;
    j = 0;
    for (i = 0; arglist[i]; ++i) {
        j = strlen(arglist[i]);
        if (t + j > MAX_PATH) {
            fprintf(stderr, "cc: too many arguments: %s...\n", _io_buf);
            fatal();
        }
        strcat(_io_buf, arglist[i]);
        strcat(_io_buf, " ");
        t += j;
    }
    strcat(_io_buf, shell_append);
    ptr = _io_buf + strlen(_io_buf) - 8;
    *ptr++ = '0' + _sympid / 10;
    *ptr   = '0' + _sympid % 10;
    pid = App_Run(_symbank, _io_buf, 1) >> 8;
    if (pid) {
        while (1) {
            _symmsg[0] = 0;
            Msg_Sleep(_sympid, pid, _symmsg);
            if (_symmsg[0] == 68) { // Shell_Exit()
                return _symmsg[2]; // internal knowledge: _exit() passes back the status as unused char 2
            } else if (_symmsg[0]) {
                Msg_Send(_sympid, _shellpid, _symmsg);           // forward message to SymShell...
                while (!Msg_Sleep(_sympid, _shellpid, _symmsg)); // ...wait for a response...
                Msg_Send(_sympid, pid, _symmsg);                 // ...and forward this to the subapp
            }
        }
    } else {
        return 1;
    }
}
#else
#ifdef LINUXBUILD
#define _P_WAIT 0
int _spawnvp(int mode, const char* file, const char* arglist[]) {
    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid == 0) {
        // child process
        execvp(file, (char* const*)arglist);
        _exit(EXIT_FAILURE);
    } else {
        // parent process
        int status;
        if (waitpid(pid, &status, 0) == -1)
            return -1;
        return status;
    }
}
#endif
#endif // SYMBUILD

static char pathbuf[CPATHSIZE];
static char namebuf[27];

/* Binaries. Native ones in /bin, non-native ones in
   <bindir>/app{.cpu} */
/*static char *make_bin_name(const char *app, const char *t)
{
	// TODO use strlcpy/cat
	if (native)
		snprintf(pathbuf, CPATHSIZE, "/bin/%s", app);
	else
		snprintf(pathbuf, CPATHSIZE, "%s%s", app, t);
		//snprintf(pathbuf, CPATHSIZE, "%s/%s%s", BINPATH, app, t);
	return pathbuf;
}*/

/* Binary area. Native ones in /bin, non-native ones in
   <libdir>/app{.cpu} */
static char *make_bin_name(const char *app, const char *tail)
{
    snprintf(pathbuf, CPATHSIZE, "%s/%s%s", BINPATH, app, tail);
	return pathbuf;
}

/* Library area. Native ones in /lib, non-native ones in
   <libdir>/app{.cpu} */
static char *make_lib_name(const char *app, const char *tail)
{
	snprintf(pathbuf, CPATHSIZE, "%s%s", app, tail);
	//snprintf(pathbuf, CPATHSIZE, "%s/%s%s", LIBPATH, app, tail);
	return pathbuf;
}

/* The library area for a target. For native this is /lib and /usr/include but
   for non-native we use <libdir>/<cpu>/{include, lib, ..} */
static char *make_lib_dir(const char *base, const char *tail)
{
	snprintf(pathbuf, CPATHSIZE, "%s/%s", LIBPATH, tail);
	return pathbuf;
}

static char *make_lib_file(const char *base, const char *dir, const char *tail)
{
	snprintf(pathbuf, CPATHSIZE, "%s/%s", LIBPATH, tail);
	return pathbuf;
}

/*
 *	Work out what we actually need to run
 */

static void set_for_processor(struct cpu_table *r)
{
	cpuset = r->set;
	cpudot = r->cpudot;
	cpudir = r->cpudir;
	cpulib = r->lib;
	cpudef = r->defines;
	ldopts = r->ldopts;
	cpucode = r->cpucode;
	has_relocs = r->has_reloc;
	feats = r->cpufeat;
}

static void append_obj(struct objhead *h, char *p, uint8_t type)
{
	struct obj *o = malloc(sizeof(struct obj));
	if (o == NULL)
		memory();
	o->name = p;
	o->next = NULL;
	o->used = 0;
	o->type = type;
	if (h->tail)
		h->tail->next = o;
	else
		h->head = o;
	h->tail = o;
}

/* Modify the filename p in-place, losing the f suffix
   and applying the t suffix. Return a copy of the p pointer.
   Add the filename to the list of files to remove if rmif
   is earlier than the last phase.

   However, if this_phase matches last_phase,
   simply return the target filename if it exists. */
static char *pathmod(char *p, char *f, char *t, int rmif, int this_phase)
{
	char *x;

	if (this_phase == last_phase && target != NULL)
		return(target);
	x = strrchr(p, '.');
	if (x == NULL) {
		fprintf(stderr, "cc: no extension on '%s'.\n", p);
		fatal();
	}
#if 0
	if (strcmp(x, f)) {
		fprintf(stderr, "cc: internal got '%s' expected '%s'.\n",
			p, t);
		fatal();
	}
#endif
	strcpy(x, t);
	if (last_phase > rmif) {
		*rmptr++ = xstrdup(p, 0);
	}
	return p;
}

static void add_argument(const char *p)
{
	if (argptr == &arglist[MAXARG]) {
		fprintf(stderr, "cc: too many arguments to command.\n");
		fatal();
	}
	*argptr++ = p;
	*argptr = 0;
}

static void add_argument_list(char *header, struct objhead *h)
{
	struct obj *i = h->head;
	while (i) {
		if (header)
			add_argument(header);
		add_argument(i->name);
		i->used = 1;
		i = i->next;
	}
}

static char *resolve_library(char *p)
{
	static char buf[512];
	struct obj *o = libpathlist.head;
	if (strchr(p, '/') || strchr(p, '.'))
		return p;
	while(o) {
		snprintf(buf, 512, "%slib%s.a", o->name, p);
		if (access(buf, 0) == 0)
			return xstrdup(buf, 0);
		o = o->next;
	}
	return NULL;
}

/* This turns -L/opt/foo/lib  -lfoo -lbar into resolved names like
   /opt/foo/lib/libfoo.a */
static void resolve_libraries(void)
{
	struct obj *o = liblist.head;
	while(o != NULL) {
		char *p = resolve_library(o->name);
		if (p == NULL) {
			fprintf(stderr, "cc: unable to find library '%s'.\n", o->name);
			fatal();
		}
		add_argument(p);
		o = o->next;
	}
}

static void run_command(void)
{
	pid_t pid, p;
	const char **ptr;
	int status;
	int i, t, j;

	if (print_passes) {
	    i = 0;
	    while (arglist[i])
            printf("%s ", arglist[i++]);
        printf("\n");
	}
	fflush(stdout);
	status = _spawnvp(_P_WAIT, arglist[0], arglist);
	if (status != 0) {
        fprintf(stderr, "cc: %s failed with signal %d.\n", arglist[0], status);
		fatal();
	}
	// FIXME deal with pass-stage errors
/*
	*argptr = NULL;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		fatal();
	}
	if (pid == 0) {
#ifdef DEBUG
		if (print_passes) {
			ptr = arglist;
			printf("[");
			while(*ptr)
				printf("%s ", *ptr++);
			printf("]\n");
		}
#endif
		fflush(stdout);
		if (arginfd != -1) {
			dup2(arginfd, 0);
			close(arginfd);
		}
		if (argoutfd != -1) {
			dup2(argoutfd, 1);
			close(argoutfd);
		}
		execv(arglist[0], (char **)arglist);
		perror(arglist[0]);
		exit(255);
	}
	if (arginfd)
		close(arginfd);
	if (argoutfd)
		close(argoutfd);
	while ((p = waitpid(pid, &status, 0)) != pid) {
		if (p == -1) {
			perror("waitpid");
			fatal();
		}
	}
	if (WIFSIGNALED(status)) {
		// Scream loudly if it exploded
		fprintf(stderr, "cc: %s failed with signal %d.\n", arglist[0],
			WTERMSIG(status));
		fatal();
	}
	// Quietly exit if the stage errors. That means it has reported
    // things to the user
	if (WEXITSTATUS(status))
		fatal();
*/
}

static void redirect_in(const char *p)
{
    perror("redirection is broken in this build");
	arginfd = open(p, O_RDONLY | O_BINARY);
	if (arginfd == -1) {
		perror(p);
		fatal();
	}
#ifdef DEBUG
	if (print_passes)
		printf("<%s\n", p);
#endif
}

static void redirect_out(const char *p)
{
    perror("redirection is broken in this build");
	argoutfd = open(p, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0666);
	if (argoutfd == -1) {
		perror(p);
		fatal();
	}
#ifdef DEBUG
	if (print_passes)
		printf(">%s\n", p);
#endif
}

static void build_arglist(char *p)
{
	arginfd = -1;
	argoutfd = -1;
	argptr = arglist;
	add_argument(p);
}


void convert_s_to_o(char *path)
{
	char *origpath= strdup(path);
	#ifdef LINUXBUILD
	build_arglist(make_bin_name("as", ""));
	#else
	build_arglist(make_bin_name("as.exe", ""));
	#endif
	add_argument("-o");
	add_argument(pathmod(path, ".s", ".o", 5, 3));
	add_argument(origpath);
	run_command();
	free(origpath);
}

void convert_c_to_s(char *path)
{
	char *tmp, *p;
	char optstr[2];
	char featstr[16];
	char* stream3 = "$stream3.s";

	snprintf(featstr, 16, "%lu", features);
	*rmptr++ = xstrdup("$symtab", 0);

	#ifdef LINUXBUILD
	build_arglist(make_bin_name("cc0", ""));
	#else
	build_arglist(make_bin_name("cc0.exe", ""));
    #endif
	//add_argument(path);
	add_argument("$stream0.c");
	*rmptr++ = xstrdup("$stream0.c", 0);
	run_command();

	#ifdef LINUXBUILD
	build_arglist(make_bin_name("cc1", ""));
	#else
	build_arglist(make_bin_name("cc1.exe", ""));
	#endif
	add_argument(featstr);
	*rmptr++ = xstrdup("$stream1", 0);
	*rmptr++ = xstrdup("$stream2", 0);
	run_command();

    pathmod(path, ".c", ".s", 2, 2);
	#ifdef LINUXBUILD
	build_arglist(make_bin_name("cc2", ""));
	#else
	build_arglist(make_bin_name("cc2.exe", ""));
	#endif
	/* FIXME: need to change backend.c parsing for above and also
	   add another arg when we do the new subcpu bits like -banked */
    optstr[0] = optimize;
	optstr[1] = '\0';
	add_argument(optstr);
	add_argument(featstr);
#ifdef SYMBUILD
    if (optimize == '0' || optimize == '1') {
#else
    if (optimize == '0') {
#endif
        stream3 = path;
        add_argument(stream3);
    }
    if (codeseg) {
        if (stream3 != path)
            add_argument(stream3);
		add_argument(codeseg);
    }
	*rmptr++ = xstrdup(stream3, 0);
	run_command();

#ifdef SYMBUILD
    if (optimize != '1' && optimize != '0') {
#else
    if (optimize != '0') {
#endif
        #ifdef LINUXBUILD
        p = xstrdup(make_bin_name("copt", ""), 0);
        #else
        p = xstrdup(make_bin_name("copt.exe", ""), 0);
        #endif
        build_arglist(p);
        add_argument(path);
        add_argument(make_bin_name("rules.z80", ""));
        run_command();
        free(p);
    }
}

// FIXME: this is broken by redirection
void convert_S_to_s(char *path)
{
	char *tmp;
	printf("FIXME: broken\n");
	return;
	#ifdef LINUXBUILD
	build_arglist("cpp");
	#else
	build_arglist(make_bin_name("cpp.exe", ""));
	#endif
	add_argument("-E");
	add_argument(path);
	tmp = xstrdup(path, 0);
	redirect_out(pathmod(tmp, ".S", ".s", 1, 2));
	run_command();
	pathmod(path, ".S", ".s", 5, 2);
}

void preprocess_c(char *path)
{
	char *tmp;

	#ifdef LINUXBUILD
	build_arglist("cpp");
	#else
	build_arglist(make_bin_name("cpp.exe", ""));
	#endif

	tmp = xstrdup("-D HEAP_SIZE=", strlen(heapsize) + 1);
	strcat(tmp, heapsize);
	add_argument(tmp);
	//add_argument("-I");
	//add_argument("./include");
	add_argument_list("-I", &inclist);
	add_argument_list("-D", &deflist);
	//add_argument("-E");
	add_argument(path);
	add_argument("-o");
	add_argument("$stream0.c");
	run_command();
}

void link_phase(void)
{
	char *relocs = NULL;
	char *p, *l, *c;
	#ifdef LINUXBUILD
	p = xstrdup(make_bin_name("ld", ""), 0);
	#else
	p = xstrdup(make_bin_name("ld.exe", ""), 0);
	#endif

	/* Set the target as infile.exe if there is no target */
	if (target==NULL) {
		target=xstrdup(objlist.head->name, 4);
        strcpy(strrchr(target, '.'), ".exe");
	}

	build_arglist(p);
	switch (targetos) {
		case OS_FUZIX:
			switch(fuzixsub) {
			case 0:
				if (has_relocs) {
					relocs = xstrdup(target, 4);
					strcpy(strrchr(relocs, '.'), ".rel");
				}
				break;
			case 1:
				add_argument("-b");
				add_argument("-C");
				add_argument("256");
				break;
			case 2:
				add_argument("-b");
				add_argument("-C");
				add_argument("512");
				break;
			}
			break;
		case OS_NONE:
		default: {
				const char **x = ldopts;
				while(*x) {
					add_argument(*x);
					x++;
				}
			}
			break;
	}
	if (strip)
		add_argument("-s");
	add_argument("-o");
	add_argument(target);
	if (appname) {
        add_argument("-N");
        strcpy(namebuf, "\"");
        memcpy(namebuf + 1, appname, 24);
        strcat(namebuf, "\"");
        add_argument(namebuf);
	}
	if (appicon) {
        add_argument("-G");
        add_argument(appicon);
	}
	if (appicon16) {
        add_argument("-g");
        add_argument(appicon16);
	}
	if (heapsize_cust) {
        add_argument("-h");
        add_argument(heapsize);
	}
	if (mapfile) {
		/* For now output a map file. One day we'll have debug symbols
		   nailed to the binary */
		char *n = xstrdup(target, 4);
		strcat(n, ".map");
		add_argument("-m");
		add_argument(n);
	}
	if (relocs) {
		add_argument("-R");
		add_argument(relocs);
	}
	/* <root>/8080/lib/ */
	l = xstrdup(make_lib_dir("", ""), 0);
	c = NULL;
	if (!standalone) {
		/* Start with crt0.o, end with libc.a and support libraries */
		c = xstrdup(make_lib_file("", "lib", crtname), 0);
		add_argument(c);
		append_obj(&libpathlist, l, 0);
		append_obj(&libpathlist, ".", 0);
		append_obj(&liblist, "sym", TYPE_A);
		append_obj(&liblist, "c", TYPE_A);
	}
	/* Will be <root>/8080/lib/lib8080.a etc */
	append_obj(&liblist, make_lib_file("", "lib", cpulib), TYPE_A);
	add_argument_list(NULL, &objlist);
	resolve_libraries();
	run_command();

	free(c);
	free(l);
	free(p);

	if (relocs) {
		/* The unlink will free it not us */
		*rmptr++ = relocs;
		#ifdef LINUXBUILD
		build_arglist(make_bin_name("reloc", ""));
		#else
		build_arglist(make_bin_name("reloc.exe", ""));
		#endif
		add_argument(target);
		add_argument(relocs);
		run_command();
	}
}

void sequence(struct obj *i)
{
/*	printf("Last Phase %d\n", last_phase); */
/*	printf("1:Processing %s %d\n", i->name, i->type); */
	if (i->type == TYPE_S) {
		convert_S_to_s(i->name);
		i->type = TYPE_s;
		i->used = 1;
	}
	if (i->type == TYPE_C) {
		preprocess_c(i->name);
		i->type = TYPE_C_pp;
		i->used = 1;
	}
	if (last_phase == 1)
		return;
/*	printf("2:Processing %s %d\n", i->name, i->type); */
	if (i->type == TYPE_C_pp || i->type == TYPE_C) {
		convert_c_to_s(i->name);
		i->type = TYPE_s;
		i->used = 1;
	}
	if (last_phase == 2)
		return;
/*	printf("3:Processing %s %d\n", i->name, i->type); */
	if (i->type == TYPE_s) {
		convert_s_to_o(i->name);
		i->type = TYPE_O;
		i->used = 1;
	}
}

void processing_loop(void)
{
	struct obj *i = objlist.head;
	while (i) {
		sequence(i);
		remove_temporaries();
		i = i->next;
	}
	if (last_phase < 4)
		return;
	link_phase();
	/* And clean up anything we couldn't wipe earlier */
	last_phase = 255;
	remove_temporaries();
}

void unused_files(void)
{
	struct obj *i = objlist.head;
	while (i) {
		if (!i->used)
			fprintf(stderr, "cc: warning file %s unused.\n",
				i->name);
		i = i->next;
	}
}

void usage(void)
{
	printf("SCC v1.2, (C)2024 Daniel E. Gaskell, Alan Cox, and others\n");
	printf("cc [option and file list]\n\n");
	printf("options:\n\n");
	printf("-c      compile to object modules only\n");
	printf("-D x    define a macro for the C preprocessor\n");
	printf("-E      preprocess only, to $stream0.c\n");
	printf("-i      enable split I/D if supported by this target\n");
	printf("-I x    add a directory to the include path\n");
	printf("-G x    set the SymbOS 4-color application icon (.SGX file)\n");
	printf("-g x    set the SymbOS 16-color application icon (.SGX file)\n");
	printf("-h x    set the malloc() heap size to x bytes (default 4096)\n");
	printf("-l x    add a library name to link\n");
	printf("-L x    add a path to the library search path\n");
	printf("-M x    create a map file\n");
	printf("-N \"x\"  set the internal SymbOS application name\n");
	printf("-o x    set the output file of the executable\n");
	printf("-Ox     set optimization level 0-2, or for size '-Os'\n");
	printf("-s      build standalone, without OS libraries and include paths\n");
	printf("-S      compile to assembly source only\n");
	printf("-T x    set the starting address of the text/code segment\n");
	printf("-V      verbosely print pass information\n");
	printf("-X      keep temporary files for debugging\n\n");
	printf("long options:\n");
	printf("--dlib:	build a loadable object module instead\n\n");
	printf("z80 feature options:\n");
	printf("-mz80-banked: Z80 with banked code\n");
	printf("-mz80-noix: do not touch IX\n");
	printf("-mz80-noiy: do not touch IY\n");
	fatal();
}

char **add_macro(char **p)
{
	if ((*p)[2])
		append_obj(&deflist, *p + 2, 0);
	else
		append_obj(&deflist, *++p, 0);
	return p;
}

char **add_library(char **p)
{
	if ((*p)[2])
		append_obj(&liblist, *p + 2, TYPE_A);
	else
		append_obj(&liblist, *++p, TYPE_A);
	return p;
}

char **add_library_path(char **p)
{
	if ((*p)[2])
		append_obj(&libpathlist, *p + 2, 0);
	else
		append_obj(&libpathlist, *++p, 0);
	return p;
}


char **add_includes(char **p)
{
	if ((*p)[2])
		append_obj(&inclist, *p + 2, 0);
	else
		append_obj(&inclist, *++p, 0);
	return p;
}

void add_system_include(void)
{
	append_obj(&inclist, xstrdup(make_lib_dir("usr", "include"), 0), 0);
}

void dunno(const char *p)
{
	fprintf(stderr, "cc: don't know what to do with '%s'.\n", p);
	fatal();
}

void add_file(char *p)
{
	char *x = strrchr(p, '.');
	if (x == NULL)
		dunno(p);
	switch (x[1]) {
	case 'a':
		append_obj(&objlist, p, TYPE_A);
		break;
	case 's':
		append_obj(&objlist, p, TYPE_s);
		break;
	case 'S':
		append_obj(&objlist, p, TYPE_S);
		break;
	case 'c':
		append_obj(&objlist, p, TYPE_C);
		c_files++;
		break;
	case 'o':
		append_obj(&objlist, p, TYPE_O);
		break;
	default:
		dunno(p);
	}
}

void one_input(void)
{
	fprintf(stderr, "cc: too many files for -E\n");
	fatal();
}

void uniopt(char *p)
{
	if (p[2])
		usage();
}

void extended_opt(const char *p)
{
	if (strcmp(p, "dlib") == 0) {
		crtname = "lib0.o";
		return;
	}
	usage();
}

void find_opt(const char *p)
{
	const char **op = feats;
	unsigned n = 1;
	if (op) {
		while(*op) {
			if (strcmp(p, *op) == 0) {
				features |= n;
				return;
			}
			op++;
			n *= 2;
		}
	}
	usage();
}

int main(int argc, char *argv[]) {
	char **p = argv;
	unsigned c;
	char *o;
	char o2;

	// get absolute path of executable
#ifdef SYMBUILD
    // SymbOS
    Dir_PathAdd(0, "", BINPATH);
#else
#ifdef LINUXBUILD
#ifdef __APPLE__
    // MacOS
    int size = MAX_PATH;
    _NSGetExecutablePath(BINPATH, &size);
#else
    // Linux
    readlink("/proc/self/exe", BINPATH, MAX_PATH-1);
    BINPATH[MAX_PATH-1] = 0; // ensure termination even if it overflows
#endif
#else
    // Windows
    GetModuleFileName(NULL, BINPATH, MAX_PATH);
#endif
#endif
#ifdef LINUXBUILD
    *strrchr(BINPATH, '/') = 0;
#else
    *(strrchr(BINPATH, '.') - 3) = 0;
#endif

    // get absolute path of libraries
    strcpy(LIBPATH, BINPATH);
#ifdef SYMBUILD
    strcat(LIBPATH, "\\lib");
#else
#ifdef LINUXBUILD
    strcat(LIBPATH, "/../lib");
#else
    strcat(LIBPATH, "\\..\\lib");
#endif
#endif

    // parse command-line
	while (*++p) {
#ifdef SYMBUILD
        if (**p == '%' && (*p)[1] == 's' && (*p)[2] == 'p') { // SymShell end parameter
            shell_append = *p;
            break;
        }
#endif

		/* filename or option ? */
		if (**p != '-') {
			add_file(*p);
			continue;
		}
		c = (*p)[1];
		if (c == '-') {
			extended_opt(*p + 2);
			continue;
		}
		switch (c) {
			/* Extended options (for now never with args) */
			/* Don't link */
		case 'c':
			uniopt(*p);
			last_phase = 3;
			break;
			/* Don't assemble */
		case 'S':
			uniopt(*p);
			last_phase = 2;
			break;
			/* Only pre-process */
		case 'E':
			uniopt(*p);
			last_phase = 1;
			only_one_input = 1;
			break;
		case 'l':
			p = add_library(p);
			break;
		case 'I':
			p = add_includes(p);
			break;
		case 'L':
			p = add_library_path(p);
			break;
		case 'D':
			p = add_macro(p);
			break;
		case 'i':
/*                    split_id();*/
			uniopt(*p);
			break;
		case 'o':
			if (target != NULL) {
				fprintf(stderr,
					"cc: -o can only be used once.\n");
				fatal();
			}
			if ((*p)[2])
				target = *p + 2;
			else if (*p)
				target = *++p;
			else {
				fprintf(stderr, "cc: no target given.\n");
				fatal();
			}
			break;
		case 'N':
			if (appname != NULL) {
				fprintf(stderr,
					"cc: -N can only be used once.\n");
				fatal();
			}
			if ((*p)[2])
				appname = *p + 2;
			else if (*p)
				appname = *++p;
			else {
				fprintf(stderr, "cc: no application name given.\n");
				fatal();
			}
			break;
		case 'g':
			if (appicon16 != NULL) {
				fprintf(stderr,
					"cc: -g can only be used once.\n");
				fatal();
			}
			if ((*p)[2])
				appicon16 = *p + 2;
			else if (*p)
				appicon16 = *++p;
			else {
				fprintf(stderr, "cc: no icon file given.\n");
				fatal();
			}
			break;
		case 'G':
			if (appicon != NULL) {
				fprintf(stderr,
					"cc: -G can only be used once.\n");
				fatal();
			}
			if ((*p)[2])
				appicon = *p + 2;
			else if (*p)
				appicon = *++p;
			else {
				fprintf(stderr, "cc: no icon file given.\n");
				fatal();
			}
			break;
		case 'h':
			if ((*p)[2])
				heapsize = *p + 2;
			else if (*p)
				heapsize = *++p;
			else {
				fprintf(stderr, "cc: no heap size given.\n");
				fatal();
			}
			heapsize_cust = 1;
			break;
		case 'O':
			if ((*p)[2]) {
				o2 = (*p)[2];
				if (o2 >= '0' && o2 <= '2')
					optimize = o2;
				else if (o2 == 's')
					optimize = 's';
				else {
					fprintf(stderr, "cc: unknown optimisation level.\n");
					fatal();
				}
			} else
				optimize = '1';
			break;
		case 's':	/* FIXME: for now - switch to getopt */
			standalone = 1;
			break;
		case 'm':
			cpu = *p + 2;
			break;
        case 'V':
			print_passes = 1;
			break;
		case 'X':
			uniopt(*p);
			keep_temp++;
			break;
		case 'M':
			mapfile = 1;
			break;
		case 'T':
			codeseg = *p + 2;
			break;
		default:
			usage();
		}
	}

	o = strchr(cpu, '-');
	if (o)
		*o++ = 0;
	set_for_processor(&cpu_rules);
	if (o) {
		o = strtok(o, "-");
		while(o) {
			find_opt(o);
			o = strtok(NULL, "-");
		}
	}

	/*while(*cpudef)
		append_obj(&deflist, (char *)*cpudef++, 0);*/ // unnecessary since we only use Z80

	if (!standalone)
		add_system_include();

	if (only_one_input && c_files > 1)
		one_input();

    if (!objlist.head) {
        usage();
        exit(1);
    }

	symtab = xstrdup(".symtmp", 6);
	//snprintf(symtab + 7, 6, "%x", getpid());
	processing_loop();
	unused_files();
	if (keep_temp < 2)
		unlink(symtab);
	return 0;
}
