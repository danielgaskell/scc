/*
 *	Ld symbol loading support. For the moment we don't do anything clever
 *	to avoid memory wastage.
 *
 *	Theory of operation
 *
 *	We scan the header and load the symbols (non debug) for each object
 *	We then compute the total size of each segment
 *	We calculate the base address of each object file code/data/bss
 *	We write a dummy executable header
 *	We relocate each object code and write all the code to the file
 *	We do the same with the data
 *	We set up the header and bss sizes
 *	We write out symbols (optional)
 *	We write the header back
 *
 *	The relocation can be one of three forms eventually
 *	ld -r:
 *		We write the entire object out as one .o file with all the
 *		internal references resolved and all the symbols adjusted
 *		versus that. Undefined symbols are allowed and carried over
 *	a.out (or similar format)
 *		We resolve the entire object as above but write out with a
 *		binary header. No undefined symbols are allowed
 *	bin:
 *		We resolve the entire object and perform all relocations
 *		to generate a binary with a fixed load address. No undefined
 *		symbols or relocations are left
 *
 *	There are a few things not yet addressed
 *	1.	For speed libraries can start with an _RANLIB ar file node which
 *		is an index of all the symbols by library module for speed.
 *	2.	Banked binaries (segments 5-7 ?).
 *	3.	Use typedefs and the like to support 32bit as well as 16bit
 *		addresses when built on bigger machines..
 *	4.	For word addressing
 *		- we need to deal with bytepointers (ptr scaling encoding needs
 *		  adding)
 *		- we need to scale o_base when adjusting symbols as o_base
 *		  is in bytes.
 *		- we need to deal with overflows resulting from word to byte
 *		  conversion on symbols.
 *		- we need to deal with the fact some stuff is tracked at
 *		  byte level.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/stat.h>

#include "linux.h"
#include "symhead.h"
#include "obj.h"
#include "ld.h"
#include "ar.h"				/* Pick up our ar.h just in case the
					   compiling OS has a weird ar.h */

#ifndef ENABLE_RESCAN
#define ENABLE_RESCAN	0
#endif

#define io_readaddr()	io_read16()
#define MAXSIZE		2
typedef int16_t		addrdiff_t;

static char *arg0;			/* Command name */
static struct object *processing;	/* Object being processed */
static const char *libentry;		/* Library entry name if relevant */
static struct object *objects, *otail;	/* List of objects */
static struct symbol *symhash[NHASH];	/* Symbol has tables */
static addr_t base[OSEG];		/* Base of each segment */
static addr_t size[OSEG];		/* Size of each segment */
static addr_t align = 1;		/* Alignment */
static addr_t baseset[OSEG];		/* Did the user force this one */
#define LD_RELOC	0		/* Output a relocatable binary stream */
#define LD_RFLAG	1		/* Output an object module */
#define LD_ABSOLUTE	2		/* Output a linked binary */
#define LD_FUZIX	3		/* Output a Fuzix binary */
static uint_fast8_t ldmode = LD_FUZIX;	/* Operating mode */
static uint_fast8_t rawstream;		/* Outputting raw or quoted ? */

static uint_fast8_t split_id;		/* True if code and data both zero based */
static uint_fast8_t verbose;		/* Verbose reporting */
static int err;				/* Error tracking */
/*static int dbgsyms = 1;*/			/* Set to dumb debug symbols */
static int strip = 0;			/* Set to strip symbols */
static const char *mapname;		/* Name of map file to write */
static const char *outname;		/* Name of output file */
static char *appname = NULL; /* SymbOS application name */
static char *appicon = NULL; /* SymbOS application icon */
static char *appicon16 = NULL; /* SymbOS 16-color application icon */
static char *heapsize = NULL; /* SymbOS heap size */
static char *widget_opts = NULL; /* SymbOS widget options */
static addr_t dot;			/* Working address as we link */

static unsigned progress;		/* Did we make forward progress ?
					   Used while library linking */
static const char *segmentorder = "CDBLdt";	/* Segment default order */

static int_fast8_t rel_shift;		/* Relocation scaling */
static addr_t rel_mask;			/* Relocation mask */
static uint_fast8_t rel_check;		/* Check fits mask */

static FILE *relocf;

static char iconbuf[298];
static char widgetbuf[22] = {'S', 'W', 'G', '1'};

/*
 *	Report an error, and if possible give the object or library that
 *	we were processing.
 */
void warning(const char *p)
{
	if (processing)
		fprintf(stderr, "While processing: %s", processing->path);
	if (libentry)
		fprintf(stderr, "(%.16s)", libentry);
	fputc('\n', stderr);
	fputs(p, stderr);
	fputc('\n', stderr);
	err |= 2;
}

void error(const char *p)
{
	warning(p);
	exit(err);
}

/*
 *	Standard routines wrapped with error exit tests
 */
static void *xmalloc(size_t s)
{
	void *p = malloc(s);
	if (p == NULL)
		error("out of memory");
	return p;
}

/*
 *	Optimized disk I/O for library scanning
 */

static uint8_t iobuf[4096];
static uint8_t *ioptr;
static unsigned ioblock;
static unsigned iopos;
static unsigned iolen;
static int iofd;

static void io_close(void)
{
	close(iofd);
	iofd = -1;
}

static unsigned io_get(unsigned block)
{
    int preseek;
    int seekerr;
	if (block != ioblock + 1 && lseek(iofd, ((off_t)block) << 12, SEEK_SET) < 0) {
		perror("lseek");
		exit(err | 1);
	}
/*	printf("io_get: seek to %lx\n", (off_t)(block << 12)); */
	ioblock = block;
	preseek = lseek(iofd, 0, SEEK_CUR);
	seekerr = read(iofd, iobuf, 4096);
	iolen = lseek(iofd, 0, SEEK_CUR) - preseek; // DEG - return value of read() is not reliable on mingw (??? unclear), calculate change manually
	if (seekerr == -1) {
		perror("read");
		exit(err | 1);
	}
	iopos = 0;
	ioptr = iobuf;
/*	printf("io_get read block %d iolen now %d\n", block, iolen); */
	return iolen;
}

static int io_lseek(off_t pos)
{
	unsigned block = pos >> 12;
	if (block != ioblock)
		io_get(block);
	iopos = pos & 4095;
	ioptr = iobuf + iopos;
	if (iopos > iolen)
		return -1;
/*	printf("io_lseek %lx, block %d pos %d len %d\n",
		pos, ioblock, iopos, iolen); */
	return 0;
}

#if 0
static off_t io_getpos(void)
{
	return (((off_t)ioblock) << 12) | iopos;
}
#endif

static int io_read(void *bufp, unsigned len)
{
	unsigned left = iolen - iopos;
	uint8_t *buf = bufp;
	unsigned n;
	unsigned bytes = 0;

	while(len) {
/*		printf("len %d, left %d, ptr %p, iopos %d iolen %d\n",
			len, left, ioptr, iopos, iolen);
		if (ioptr != iobuf + iopos) {
			fprintf(stderr, "Botch %p %p\n",
				ioptr, iobuf + iopos);
			exit(1);
		} */
		n = len;
		if (n > left)
			n = left;
		if (n) {
			memcpy(buf, ioptr, n);
			ioptr += n;
			iopos += len;
			len -= n;
			bytes += n;
			buf += n;
		}
		if (len) {
			if (io_get(ioblock + 1) == 0)
				break;
			left = iolen;
		}
	}
	//printf("io_read %d\n", bytes);
	return bytes;
}

static unsigned io_read16(void)
{
	uint8_t p[2];
	io_read(p, 2);
#ifdef SYMBUILD
    return *((unsigned int*)p);
#else
	return (p[1] << 8) | p[0];
#endif
}

/* Our embedded relocs make this a hot path so optimize it. We may
   want a helper that hands back blocks until the reloc marker ? */

unsigned io_readb(uint_fast8_t *ch)
{
	if (iopos < iolen) {
		iopos++;
		*ch = *ioptr++;
		return 1;
	} else
		return io_read(ch, 1);
}

static int io_open(const char *path)
{
	iofd = open(path, O_RDONLY | O_BINARY);
	ioblock = 0xFFFF;	/* Force a re-read */
    if (iofd == -1 || io_lseek(0)) {
		perror(path);
		exit(err | 1);
	}
/*	printf("opened %s\n", path); */
	return iofd;
}

static uint8_t outbuf[1024];
static uint8_t *outptr;
static uint8_t *outend;
off_t outpos;
int ofd;

static void out_flush(void) {
    unsigned short len = outptr - outbuf;
    write(ofd, outbuf, len);
    outptr = outbuf;
    outpos += len;
}

static void out_byte(unsigned char ch) {
    *outptr++ = ch;
    if (outptr >= outend)
        out_flush();
}

static void out_write(void* addr, unsigned short len) {
    if (len > outend - outptr)
        out_flush();
    memcpy(outptr, addr, len);
    outptr += len;
}

static off_t out_tell(void) {
    return outpos + (outptr - outbuf);
}

static void out_seek(off_t pos) {
    out_flush();
	if (lseek(ofd, pos, SEEK_SET) < 0) {
		perror("lseek");
		exit(err | 1);
	}
	outpos = pos;
}

static FILE *xfopen(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);
	if (fp == NULL) {
		perror(path);
		exit(err | 1);
	}
	return fp;
}

static void xfclose(FILE *fp)
{
	if (fclose(fp)) {
		perror("fclose");
		exit(err | 1);
	}
}

static void xfseek(FILE *fp, off_t pos)
{
	if (fseek(fp, pos, SEEK_SET) < 0) {
		perror("fseek");
		exit(err | 1);
	}
}

static addr_t xstrtoul(const char *p)
{
	char *r;
	unsigned long x = strtoul(p, &r, 0);
	if (r == p) {
		fprintf(stderr, "'%s' is not a valid numeric constant.\n", p);
		exit(1);
	}
	if (x > 65535) {
		fprintf(stderr, "'%s' is not in the range 0-65535.\n", p);
		exit(1);
	}
	return x;
}

/*
 *	Manage the linked list of object files and object modules within
 *	libraries that we have seen.
 */
static struct object *new_object(void)
{
	struct object *o = xmalloc(sizeof(struct object));
	o->next = NULL;
	o->syment = NULL;
	return o;
}

static void insert_object(struct object *o)
{
	if (otail)
		otail->next = o;
	else
		objects = o;
	otail = o;
}

static void free_object(struct object *o)
{
	if (o->syment)
		free(o->syment);
	if (o->oh)
		free(o->oh);
	free(o);
}

/*
 *	Add a symbol to our symbol tables as we discover it. Log the
 *	fact if tracing.
 */
struct symbol *new_symbol(const char *name, int hash)
{
	struct symbol *s = xmalloc(sizeof(struct symbol));
	strncpy(s->name, name, NAMELEN);
	s->next = symhash[hash];
	symhash[hash] = s;
	if (verbose)
		printf("+%.*s\n", NAMELEN, name);
	return s;
}

/*
 *	Find a symbol in a given has table
 */
struct symbol *find_symbol(const char *name, int hash)
{
	struct symbol *s = symhash[hash];
	while (s) {
		if (strncmp(s->name, name, NAMELEN) == 0)
			return s;
		s = s->next;
	}
	return NULL;
}

/*
 *	A simple but adequate hashing algorithm. A better one would
 *	be worth it for performance.
 */
static uint8_t hash_symbol(const char *name)
{
	int hash = 0;
	uint_fast8_t n = 0;

	while(*name && n++ < NAMELEN)
		hash += *name++;
	return (hash&(NHASH-1));
}

/*
 *	Check if a symbol name is known but undefined. We use this to decide
 *	whether to incorporate a library module
 */
static int is_undefined(const char *name)
{
	int hash = hash_symbol(name);
	struct symbol *s = find_symbol(name, hash);
	if (s == NULL || !(s->type & S_UNKNOWN))
		return 0;
	/* This is a symbol we need */
	progress++;
	return 1;
}

/*
 *	Check that two versions of a symbol are compatible.
 */
static void segment_mismatch(struct symbol *s, uint_fast8_t type2)
{
	uint_fast8_t seg1 = s->type & S_SEGMENT;
	uint_fast8_t seg2 = type2 & S_SEGMENT;

	/* Matching */
	if (seg1 == seg2)
		return;
	/* Existing entry was 'anything'. Co-erce to definition */
	if (seg1 == S_ANY) {
		s->type &= ~S_SEGMENT;
		s->type |= seg2;
		return;
	}
	/* Regardless of the claimed type, an absolute definition fulfills
	   any need. */
	if (seg2 == ABSOLUTE || seg2 == S_ANY)
		return;
	fprintf(stderr, "Segment mismatch for symbol '%.*s'.\n", NAMELEN, s->name);
	fprintf(stderr, "Want segment %d but constrained to %d.\n",
		seg2, seg1);
	err |= 2;
}

/*
 *	We have learned about a new symbol. Find the symbol if it exists, or
 *	create it if not. Do the necessary work to promote unknown symbols
 *	to known and also to ensure we don't have multiple incompatible
 *	definitions or have incompatible definition and requirement.
 */
static struct symbol *find_alloc_symbol(struct object *o, uint_fast8_t type, const char *id, addr_t value)
{
	uint8_t hash = hash_symbol(id);
	struct symbol *s = find_symbol(id, hash);

	if (s == NULL) {
		s = new_symbol(id, hash);
		s->type = type;
/*FIXME         strlcpy(s->name, id, NAMELEN); */
		strncpy(s->name, id, NAMELEN);
		s->value = value;
		if (!(type & S_UNKNOWN))
			s->definedby = o;
		else
			s->definedby = NULL;
		return s;
	}
	/* Already exists. See what is going on */
	if (type & S_UNKNOWN) {
		/* We are an external reference to a symbol. No work needed */
		segment_mismatch(s, type);
		return s;
	}
	if (s->type & S_UNKNOWN) {
		/* We are referencing a symbol that was previously unknown but which
		   we define. Fill in the details */
		segment_mismatch(s, type);
		s->type &= ~S_UNKNOWN;
		s->value = value;
		s->definedby = o;
		return s;
	}
	/* Two definitions.. usually bad but allow duplicate absolutes */
	if (((s->type | type) & S_SEGMENT) != ABSOLUTE || s->value != value) {
		/* FIXME: expand to report files somehow ? */
		fprintf(stderr, "%.*s: multiply defined.\n", NAMELEN, id);
	}
	/* Duplicate absolutes - just keep current entry */
	return s;
}

/*
 *	Add the internal symbols indicating where the segments start and
 *	end.
 */
static void insert_internal_symbol(const char *name, int seg, addr_t val)
{
	if (seg == -1)
		find_alloc_symbol(NULL, S_ANY | S_UNKNOWN, name, 0);
	else
		find_alloc_symbol(NULL, seg | S_PUBLIC, name, val);
}

/*
 *	Number the symbols that we will write out in the order they will
 *	appear in the output file. We don't care about the mode too much.
 *	In valid reloc mode we won't have any S_UNKNOWN symbols anyway
 */
static void renumber_symbols(void)
{
	static int sym = 0;
	struct symbol *s;
	int i;
	for (i = 0; i < NHASH; i++)
		for (s = symhash[i]; s != NULL; s=s->next)
			if (s->type & (S_PUBLIC|S_UNKNOWN))
				s->number = sym++;
}

/* Write the symbols to the output file */
static void write_symbols(void)
{
	struct symbol *s;
	int i;
	for (i = 0; i < NHASH; i++) {
		for (s = symhash[i]; s != NULL; s=s->next) {
			out_byte(s->type);
			out_write(s->name, NAMELEN);
			out_byte(s->value);
			out_byte(s->value >> 8);
		}
	}
}

/*
 *	TODO: Fold all these symbol table walks into a helper
 */

/*
 *	Print a symbol for the map file
 */
static void print_symbol(struct symbol *s, FILE *fp)
{
	char c;
	if (s->type & S_UNKNOWN)
		c = 'U';
	else {
		c = "ACDBZLdt"[s->type & S_SEGMENT];
		if (s->type & S_PUBLIC)
			c = toupper(c);
	}
	fprintf(fp, "%04X %c %.*s\n", s->value, c, NAMELEN, s->name);
}

/*
 *	Walk the symbol table generating a map file as we go
 */

static void write_map_file(FILE *fp)
{
	struct symbol *s;
	int i;
	for (i = 0; i < NHASH; i++) {
		for (s = symhash[i]; s != NULL; s=s->next)
			print_symbol(s, fp);
	}
}

/*
 *	See if we already merged an object module. With a library we
 *	scan mutiple times but we don't import the same module twice
 */

static int have_object(off_t pos, const char *name)
{
	struct object *o = objects;
	while(o) {
		if (o->off == pos && strcmp(name, o->path) == 0)
			return 1;
		o = o->next;
	}
	return 0;
}

static unsigned get_object(struct object *o)
{
	o->oh = xmalloc(sizeof(struct objhdr));
	io_lseek(o->off);
	return io_read(o->oh, sizeof(struct objhdr));
}

static void put_object(struct object *o)
{
	if (o->oh)
		free(o->oh);
	o->oh = NULL;
}

/*
 *	Open an object file and seek to the right location in case it is
 *	a library module.
 */
static void openobject(struct object *o)
{
	io_open(o->path);
	get_object(o);
}


/*
 *	Load a new object file. The off argument allows us to load an
 *	object module out of a library by giving the library file handle
 *	and the byte offset into it.
 *
 *	Do all the error reporting and processing needed to incorporate
 *	the module, and load and add all the symbols.
 */
static struct object *load_object(off_t off, int lib, const char *path)
{
	int i;
	uint_fast8_t type;
	char name[NAMELEN + 1];
	struct object *o = new_object();
	struct symbol **sp;
	int nsym;
	addr_t value;

	o->path = path;
	o->off = off;
	processing = o;	/* For error reporting */

	if (get_object(o) != sizeof(struct objhdr) || o->oh->o_magic != MAGIC_OBJ || o->oh->o_symbase == 0) {
		/* A library may contain other things, just ignore them */
		if (lib) {
			free_object(o);
			processing = NULL;
			return NULL;
		}
		else	/* But an object file must be valid */
			error("bad object file (1)");
	}
	/* Load up the symbols */
	nsym = (o->oh->o_dbgbase - o->oh->o_symbase) / S_ENTRYSIZE;
	if (nsym < 0||nsym > 65535)
		error("bad object file (2)");
	/* Allocate the symbol entries */
	o->syment = (struct symbol **) xmalloc(sizeof(struct symbol *) * nsym);
	o->nsym = nsym;
restart:
	io_lseek(off + o->oh->o_symbase);
	sp = o->syment;
	for (i = 0; i < nsym; i++) {
		io_readb(&type);
        io_read(name, NAMELEN);
		name[NAMELEN] = 0;
		value = io_readaddr();	/* Little endian */
		if (!(type & S_UNKNOWN) && (type & S_SEGMENT) >= OSEG) {
			fprintf(stderr, "Symbol %s\n", name);
			if ((type & S_SEGMENT) == UNKNOWN)
				error("exported but undefined");
			else
				error("bad symbol");
		}
		/* In library mode we look for a symbol that means we will load
		   this object - and then restart wih lib = 0 */
		if (lib) {
			if (!(type & S_UNKNOWN) && is_undefined(name)) {
				if (verbose)
					printf("importing for '%s'\n", name);
				lib = 0;
				goto restart;
			}
		} else
			*sp++ = find_alloc_symbol(o, type, name, value);
	}
	/* If we get here with lib set then this was a library module we didn't
	   in fact require */
	if (lib) {
		free_object(o);
		processing = NULL;
		return NULL;
	}
	insert_object(o);
	/* The CPU features required is the sum of all the flags in the objects */
	processing = NULL;
	put_object(o);
	return o;
}

/*
 *	Helper for layout computation. Add one segment after another
 *	and ensure it fits. If a segment base is hand set don't touch it
 */

static void append_segment(int a, int b)
{
	if (baseset[a])
		return;
	base[a] = ((base[b] + size[b] + align - 1)/align) * align;
	if (base[a] < base[b])
		error("image too large");
}

static char segnames[] = "CDBZLdt";

static void order_segments(void)
{
	const char *s = segmentorder;
	unsigned last = 0xFF;
	unsigned n;

	while(*s) {
		char *p = strchr(segnames, *s);
		if (p == NULL) {
			fprintf(stderr, "Unknown segment '%c'.\n", *s);
			error("invalid segment order");
		}
		n = p - segnames + 1;
		if (!baseset[n]) {
			if (last != 0xFF)
				append_segment(n, last);
		}
		last = n;
		s++;
	}
}

/*
 *	Once all the objects are loaded this function walks the list and
 *	assigns each object file a base address for each segment. We do
 *	this by walking the list once to find the total size of code/data/bss
 *	and then a second time to set the offsets.
 */
static void set_segment_bases(void)
{
	struct object *o;
	addr_t pos[OSEG];
	int i;

	/* We are doing a simple model here without split I/D for now */
	for (i = 1; i < OSEG; i++)
		size[i] = 0;
	/* Now run through once computing the basic size of each segment */
	for (o = objects; o != NULL; o = o->next) {
		openobject(o);
		if (verbose)
			printf("%s:\n", o->path);
		for (i = 1; i < OSEG; i++) {
		    size[i] += o->oh->o_size[i];
			if (verbose)
				printf("\t%c : %04X  %04X\n",
					"ACDBZLdt????"[i], o->oh->o_size[i],
						size[i]);
			if (size[i] < o->oh->o_size[i])
				error("segment too large");
		}
		put_object(o);
		io_close();
	}

	// add extra room for 16-color icon, if applicable
    if (appicon16)
        size[BSS] += 298; // initially added to BSS segment, but actually exists at the start of the SYMDATA segment (below)

	if (verbose) {
		for (i = 1; i < OSEG; i++)
			printf("Segment %c Size %04X\n", "ACDBZLdt"[i], size[i]);
	}
	/* We now know where to put the binary */
	if (ldmode == LD_RELOC) {
		/* Creating a binary - put the segments together */
		if (split_id && !baseset[2]) {
			base[2] = 0;
			baseset[2] = 1;
		}
	}
	order_segments();

	if (ldmode != LD_RFLAG) {
		/* ZP if any is assumed to be set on input */
		/* FIXME: check the literals fit .. make this a more sensible
		   overlap check loop ? */
		if (base[3] < base[2] || base[3] + size[3] < base[3])
			error("image too large");
		/* Whoopee it fits */
		/* Insert the linker symbols */
		/* FIXME: symbols for all OSEG segments */
		insert_internal_symbol("__code", CODE, 0);
		insert_internal_symbol("__data", DATA, 0);
		insert_internal_symbol("__bss", BSS, 0);
		insert_internal_symbol("__end", BSS, size[3]);
		insert_internal_symbol("__zp", ZP, 0);
		insert_internal_symbol("__literal", LITERAL, 0);
		insert_internal_symbol("__symdata", SYMDATA, 0);
		insert_internal_symbol("__symtrans", SYMTRANS, 0);
		insert_internal_symbol("__code_size", ABSOLUTE, size[CODE]);
		insert_internal_symbol("__data_size", ABSOLUTE, size[DATA]);
		insert_internal_symbol("__bss_size", ABSOLUTE, size[BSS]);
		insert_internal_symbol("__literal_size", ABSOLUTE, size[LITERAL]);
		insert_internal_symbol("__zp_size", ABSOLUTE, size[ZP]);
		insert_internal_symbol("__symdata_size", ABSOLUTE, size[SYMDATA]);
		insert_internal_symbol("__symtrans_size", ABSOLUTE, size[SYMTRANS]);
	}
	/* Now set the base of each object appropriately */
	memcpy(&pos, &base, sizeof(pos));
	for (o = objects; o != NULL; o = o->next) {
		openobject(o);
		o->base[0] = 0;
		for (i = 1; i < OSEG; i++) {
			o->base[i] = pos[i];
			pos[i] += o->oh->o_size[i];
		}
		put_object(o);
		io_close();
	}
	/* At this point we have correctly relocated the base for each object. What
	   we have yet to do is to relocate the symbols. Internal symbols are always
	   created as absolute with no definedby */
	for (i = 0; i < NHASH; i++) {
		struct symbol *s = symhash[i];
		while (s != NULL) {
			uint_fast8_t seg = s->type & S_SEGMENT;
			/* base will be 0 for absolute */
			if (s->definedby)
				s->value += s->definedby->base[seg];
			else
				s->value += base[seg];
			/* FIXME: check overflow */
			s = s->next;
		}
	}

    if (appicon16) { // move 16-color icon to SYMDATA segment, so it can be compressed correctly
        size[BSS] -= 298;
        size[SYMDATA] += 298;
    }

	/* We now know all the base addresses and all the symbol values are
	   corrected. Everything needed for relocation is present */
}

/*
 *	Write a target byte with correct quoting if needed
 *
 *	We quote if we are outputing a new link binary (ld -r), or a
 *	relocatable.
 */

static void target_pquoteb(uint8_t v)
{
	if (v == REL_ESC && !rawstream) {
		out_byte(v);
		out_byte(REL_REL);
	} else
		out_byte(v);
}

/*
 *	Write a word to the target in the correct endianness
 */
static void target_put(addr_t value, uint16_t size)
{
	/* Tighter short paths for 16bit so it can run nicely on small
	   boxes */
	if (size == 1)
		target_pquoteb(value);
	else {
        target_pquoteb(value);
        target_pquoteb(value >> 8);
	}
}

static uint_fast8_t target_pgetb(void)
{
	uint_fast8_t c;
	if (io_readb(&c) == 0)
		error("unexpected EOF");
	return c;
}

/*
 *	Read a work from the target in the correct endianness. For
 *	better or worse all our relocation streams are always little endian
 *	while the instruction stream being relocated is of necessity native
 *	endian.
 */
static addr_t target_get(struct object *o, uint16_t size)
{
	/* Tighter hardcoded for speed on 8bit boxes */
	if (size == 1)
		return target_pgetb();
	else
        return target_pgetb() + (target_pgetb() << 8);
}

static void record_reloc(unsigned high, unsigned size, unsigned seg, addr_t addr)
{
	if (!relocf)
		return;

	/* Absolutes are .. absolute */
	if (seg == ABSOLUTE)
		return;

	if (seg == ZP) {
		fputc(0, relocf);
		fputc(0, relocf);
		fputc((addr & 0xFF), relocf);
		return;
	}
	if (size == 1 && !high)
		return;
	/* Record the address of the high byte */
	fputc(1, relocf);
	fputc((addr >> 8), relocf);
	fputc(addr, relocf);
}

static unsigned is_code(unsigned seg)
{
	/* TODO: when we add banking/overlays this will need to change */
	if (seg == CODE)
		return 1;
	return 0;
}

/*
 *	Relocate the stream of input from ip to op
 *
 * We support three behaviours
 * LD_ABSOLUTE: all symbols are resolved and the relocation quoting is removed
 * LD_RFLAG: quoting is copied, internal symbols are resolved, externals kept
 * LD_RELOC: a relocation stream is output with no remaining symbol relocations
 *	     and all internal relocations resolved.
 */
static void relocate_stream(struct object *o, int segment)
{
	uint8_t size;
	uint_fast8_t code;
	addr_t r;
	struct symbol *s;
	uint_fast8_t tmp;
	uint_fast8_t seg;
	uint16_t sv;

	processing = o;

	while (io_readb(&code) == 1) {
		uint_fast8_t optype;
		uint_fast8_t overflow = 1;
		uint_fast8_t high = 0;
		uint_fast8_t pcrel = 0;

//		if (ldmode == LD_ABSOLUTE && ftell(op) != dot) {
//			fprintf(stderr, "%ld not %d\n",
//				(long)ftell(op), dot);
//		}

		/* Unescaped material is just copied over */
		if (code != REL_ESC) {
			out_byte(code);
			dot++;
			continue;
		}
		io_readb(&code);
		if (code == REL_EOF) {
			processing = NULL;
			return;
		}
		/* Escaped 0xDA byte. Just copy it over, and if in absolute mode
		   remove the REL_REL marker */
		if (code == REL_REL) {
			if (!rawstream) {
				out_byte(REL_ESC);
				out_byte(REL_REL);
			} else
				out_byte(REL_ESC);
			dot++;
			continue;
		}
		if (code == REL_ORG) {
			if (ldmode != LD_ABSOLUTE) {
				fprintf(stderr, "%s: absolute addressing requires '-b'.\n", o->path);
				exit(1);
			}
			if (segment != ABSOLUTE) {
				fprintf(stderr, "%s: cannot set address in non absolute segment.\n", o->path);
				exit(1);
			}
			dot = io_readaddr();
			/* Image has code bank raw then data raw */
			/* TODO: assumes 16bit */
			if (!is_code(segment) && split_id)
				out_seek(dot + 0x10000);
			else
				out_seek(dot);
			continue;
		}
		/* Set the relocation scaling properties (this is sticky) */
		if (code == REL_MOD) {
			io_readb(&tmp);
			/* Copy through but do not change properties or
			   we will end up double scaling stuff via ld -r */
			if (!rawstream) {
				out_byte(REL_ESC);
				out_byte(tmp);
				io_readb(&tmp);
				out_byte(tmp);
				continue;
			}
			/* 0x1F is sufficient as for a 32bit full mask
			   as 0x00 will give us a mask of 0 which inverted
			   is 0xFFFFFFFF */
			rel_mask = (1UL << (tmp & 0x1F)) - 1;
			if (tmp & 0x80)
				rel_mask = ~rel_mask;
			rel_check = tmp & 0x40;
			io_readb(&tmp);
			rel_shift = tmp & 0x0F;
			if (tmp & 0x80)
				rel_shift = -rel_shift;
			continue;
		}
#ifdef TARGET_RELOC
		if (code == REL_TARGET) {
			target_relocate(o, segment, op);
			continue;
		}
#endif
		if (code == REL_PCR) {
			pcrel = 1;
			io_readb(&code);
		}
		if (code == REL_OVERFLOW) {
			overflow = 0;
			io_readb(&code);
		}
		if (code == REL_HIGH) {
			high = 1;
			overflow = 0;
			io_readb(&code);
		}
		/* Relocations */
		size = ((code & S_SIZE) >> 4) + 1;

		/* Simple relocation - adjust versus base of a segment of this object */
		if (code & REL_SIMPLE) {
			seg = code & S_SEGMENT;
			/* Check entry is valid */
			if (seg == ABSOLUTE || seg >= OSEG || size > MAXSIZE) {
				fprintf(stderr, "%s invalid reloc %d %d\n",
					o->path, seg, size);
				fprintf(stderr, "pcrel %u over %u high %u\n",
					pcrel, overflow, high);
				error("invalid reloc");
			}
			/* If we are not building an absolute then keep the tag */
			if (!rawstream) {
                out_byte(REL_ESC);
				if (pcrel)
					out_byte(REL_PCREL);
				if (!overflow)
					out_byte(REL_OVERFLOW);
				if (high)
					out_byte(REL_HIGH);
				out_byte(code);
			}
			if (pcrel) {
				r = target_get(o, size);
				r += o->base[seg];
				/* r is now the abs address */
				r -= dot;
				if (rel_shift) {
					if (rel_shift < 0)
						r <<= -rel_shift;
					else
						r >>= rel_shift;
				}
				if (rel_check && (r & ~rel_mask))
					error("relocation exceeds mask");
				r &= rel_mask;
				/* r is now relative to the address of the relocation */
				/* TODO: General confusion about if high is top 8bit or >>8 to
				   deal with */
				if (high && rawstream) {
					r >>= 8;
					size = 1;
				}
				target_put(r, size);
				dot += size;
				/* No need to record this for reloc as it's relative */
				continue;
			} else {
				/* Relocate the value versus the new segment base and offset of the
				   object */
				r = target_get(o, size);
				//			fprintf(stderr, "Target is %x, Segment %d base is %x\n",
				//				r, seg, o->base[seg]);
				r += o->base[seg];
				/* TODO: Should do reloc fitting for bigger sizes on ARCH32 ? */
				if (overflow && (r < o->base[seg] || (size == 1 && r > 255))) {
					fprintf(stderr, "%d width relocation offset %d does not	fit.\n", size, r);
					fprintf(stderr, "relocation failed at 0x%04X\n", dot);
					warning("relocation exceeded");
				}
				if (rel_shift) {
					if (rel_shift < 0)
						r <<= -rel_shift;
					else
						r >>= rel_shift;
				}
				if (rel_check && (r & ~rel_mask))
					error("relocation exceeds mask");
				r &= rel_mask;
				/* REL_HIGH is only defined for 8/16bit addresses */
				/* A high relocation had a 16bit input value we relocate versus
				   the base then chop down */
				if (high && rawstream) {
					r >>= 8;
					size = 1;
				}
				target_put(r, size);
				if (ldmode == LD_FUZIX)
					record_reloc(high, size, seg, dot);
				dot += size;
				continue;
			}
		}
		optype = code & REL_TYPE;
		/* Symbolic relocations - may be inter-segment and inter-object */
		switch(optype)
		{
			default:
				error("invalid reloc type");
			case REL_SYMBOL:
			case REL_PCREL:
				r = io_read16();
				/* r is the symbol number */
				if (r >= o->nsym) {
                    printf("%i, %s: %s: %i, %i at %i (%i %i)\n", code, o->path, s->name, r, o->nsym, (int)(ioptr - iobuf), *(ioptr - 2), *(ioptr - 1));
                    error("invalid reloc sym");
				}
				s = o->syment[r];
/*				fprintf(stderr, "relocating sym %d (%s : %x)\n", r, s->name, s->type); */
				if (s->type & S_UNKNOWN) {
					if (ldmode != LD_RFLAG) {
						if (processing)
							fprintf(stderr, "%s: Unknown symbol '%.*s'.\n", o->path, NAMELEN, s->name);
						err |= 1;
					}
					if (!rawstream) {
						/* Rewrite the record with the new symbol number */
						out_byte(REL_ESC);
						if (!overflow)
							out_byte(REL_OVERFLOW);
						if (high)
							out_byte(REL_HIGH);
						out_byte(code);
						out_byte(s->number);
						out_byte(s->number >> 8);
					}
					/* Copy the bytes to relocate */
					io_readb(&tmp);
					out_byte(tmp);
					if (size == 2 || optype == REL_PCREL) {
						io_readb(&tmp);
						out_byte(tmp);
					}
				} else {
					/* Get the relocation data */
					r = target_get(o, optype == REL_PCREL ? MAXSIZE : size);
					/* Add the offset from the output segment base to the
					   symbol */
					sv = s->value;
					r += sv;
					if (optype == REL_PCREL) {
						addrdiff_t off = r;
						/* Word addressed but our dot is byte counting */
						off -= dot;
						if (rel_shift) {
							if (rel_shift < 0)
								off <<= -rel_shift;
							else
								off >>= rel_shift;
						}
/* FIXME - signed handling needed		if (rel_check && (off & ~rel_mask))
							error("relocation exceeds mask");
*/
						off &= rel_mask;
						if (overflow && size == 1 && (off < -128 || off > 127))
							error("byte relocation exceeded");
						r = (addr_t)off;
					} else {
						/* Check again */
						if (rel_shift) {
							if (rel_shift < 0) {
								r <<= -rel_shift;
								sv <<= -rel_shift;
							} else {
								r >>= rel_shift;
								sv >>= rel_shift;
							}
						}
						/* This needs sign extending on mask for check */
						if (rel_check && (r & ~rel_mask))
							error("relocation exceeds mask");
						r &= rel_mask;
						if (overflow && (r < sv || (size == 1 && r > 255))) {
							fprintf(stderr, "width %d relocation offset %d, %d, %d does not fit.\n", size, r-s->value, s->value, r);
							error("relocation exceeded");
						}
					}
					/* If we are not fully resolving then turn this into a
					   simple relocation */
					if (!rawstream && optype != REL_PCREL) {
						out_byte(REL_ESC);
						if (!overflow)
							out_byte(REL_OVERFLOW);
						if (high)
							out_byte(REL_HIGH);
						out_byte(REL_SIMPLE | (s->type & S_SEGMENT) | (size - 1) << 4);
					}
					/* REL_HIGH is only defined for 16bit addresses */
					if (rawstream && high) {
						r >>= 8;
						size = 1;
					}
				}
				if (optype == REL_SYMBOL && ldmode == LD_FUZIX)
					record_reloc(high, size, (s->type & S_SEGMENT), dot);
				target_put(r, size);
				dot += size;
				break;
		}
	}
	error("corrupt reloc stream");
}

/*
 *	Write out all the segments of the output file with any needed
 *	relocations performed. We write out the code and data segments but
 *	BSS and ZP must always be zero filled so do not need writing.
 */
static void write_stream(int seg)
{
	struct object *o = objects;

	/* Start with clear defaults for each segment */
	rel_shift = 0;
	rel_mask = 0xFFFF;
	rel_check = 0;

	while (o != NULL) {
		openobject(o);	/* So we can hide library gloop */
		if (verbose)
			printf("Writing %s#%ld:%d\n", o->path, o->off, seg);
		io_lseek(o->off + o->oh->o_segbase[seg]);
		if (verbose)
			printf("%s:Segment %d file seek base %d\n",
				o->path,
				seg, o->oh->o_segbase[seg]);
		dot = o->base[seg];
		/* For Fuzix we place segments in absolute space but don't
		   bother writing out the empty page before */
		if (ldmode == LD_FUZIX) {
			out_seek(dot - base[1]);
		/* In absolute mode we place segments wherever they should
		   be in the binary space */
		} else if (ldmode == LD_ABSOLUTE) {
			if (verbose)
				printf("Writing seg %d from %x\n", seg, dot);
			/* TODO: assumes 16bit */
			if (!is_code(seg) && split_id)
				out_seek(dot + 0x10000);
			else
				out_seek(dot);
		}
        //printf("segment %i to %i\n", seg, ftell(op));
		relocate_stream(o, seg);
		put_object(o);
		io_close();
		o = o->next;
	}
	if (!rawstream) {
		out_byte(REL_ESC);
		out_byte(REL_EOF);
	}
}

/*
 *	Write out the target file including any necessary headers if we are
 *	writing out a relocatable object. At this point we don't have any
 *	support for writing out a nice loader friendly format, and that
 *	may want to be a separate tool that consumes a resolved relocatable
 *	binary and generates a separate relocation block in the start of
 *	BSS.
 */
static void write_binary(FILE *mp)
{
	static struct objhdr hdr;
	static struct objhdr blankhdr;
	static struct symbos_hdr symhdr;
	uint8_t i;
	uint16_t extra, iconloc16;
	FILE* ficn;
	int j;

	memset(&blankhdr, 0, sizeof(blankhdr));

	hdr.o_segbase[0] = sizeof(hdr);
	for (i = 0; i < OSEG; ++i)
        hdr.o_size[i] = size[i];

	memset(&symhdr, 0, sizeof(symhdr));
	symhdr.len_code = size[CODE] + size[DATA] + size[BSS];
	symhdr.len_data = size[SYMDATA] + size[LITERAL];
	symhdr.len_transfer = size[SYMTRANS];
    symhdr.origin = 0;
    strcpy(&symhdr.name[0], "Application");
    memcpy(&symhdr.exeid[0], "SymExe10", 8);
    symhdr.minor_version = 0;
    symhdr.major_version = 2;

	out_seek(0);

	if (verbose)
		printf("Writing binary\n");
	if (!rawstream)
        out_write(&hdr, sizeof(hdr));
	/* For LD_RFLAG number the symbols for output, for other forms
	   check for unknown symbols and error them out */
	if (!rawstream)
		renumber_symbols();
	if ((ldmode == LD_FUZIX || ldmode == LD_RFLAG) && size[0]) {
		fprintf(stderr, "Cannot build a SymbOS or relocatable binary including absolute segments.\n");
		exit(1);
	}
    if (ldmode != LD_FUZIX)
		write_stream(ABSOLUTE);
	write_stream(CODE);
	hdr.o_segbase[1] = out_tell();
	write_stream(DATA);
	if (ldmode == LD_FUZIX)
		write_stream(LITERAL);
	if (ldmode == LD_FUZIX) {
        // It's not necessary to output a blank BSS section, because the file will automatically be extended to accommodate the data and transfer segments.
        write_stream(SYMDATA);
        write_stream(SYMTRANS);
	}
	/* Absolute images may contain things other than code/data/bss */
	if (ldmode == LD_ABSOLUTE) {
		for (i = 4; i < OSEG; i++)
			write_stream(i);
	} else if (!rawstream && !strip) {
        hdr.o_symbase = out_tell();
        write_symbols();
	}
	hdr.o_dbgbase = out_tell();
	hdr.o_magic = MAGIC_OBJ;
	/* TODO: needs a special pass
	if (dbgsyms )
		copy_debug_all(op, mp);*/
    if (rawstream && ldmode == LD_FUZIX) { // overwrite the start of crt0.o's header (used for addresses) with our own
        out_seek(0);
        out_write(&symhdr, 8); // codelen, datalen, translen, origin
        if (appname) {
            out_seek(15);
            out_write(appname, strlen(appname) + 1); // app name
        }
        if (appicon) {
            out_seek(109);
            ficn = xfopen(appicon, "rb");
            if (ficn == NULL)
                error("Cannot open icon file");
            fread(iconbuf, 147, 1, ficn);
            out_write(iconbuf, 147);
            fclose(ficn);
        }
        if (appicon16) {
            iconloc16 = size[CODE] + size[DATA] + size[BSS];
            out_seek(iconloc16);
            ficn = xfopen(appicon16, "rb");
            if (ficn == NULL)
                error("Cannot open icon file");
            fread(iconbuf, 298, 1, ficn);
            out_write(iconbuf, 298);
            fclose(ficn);
            out_seek(40);
            out_byte(1);                // flag: 1 = 16-color icon included
            out_write(&iconloc16, 2);   // icon location
        }
        if (heapsize) {
            extra = atoi(heapsize);
            if (extra > 0)
                extra += 256; // unclear why this is needed - path suffix seems to overwrite last 256 bytes of extra if present
            out_seek(56);
            out_write(&extra, 2);
            out_seek(280);
            out_write(&extra, 2);
        }
        if (widget_opts) {
            widgetbuf[5] = atoi(widget_opts);
            i = 0;
            while (*widget_opts) {
                while (*widget_opts != ',') {
                    if (*widget_opts == 0)
                        goto widget_done;
                    ++widget_opts;
                }
                ++widget_opts;
                if (i >= 8) {
                    warning("Extra widget sizes ignored");
                    break;
                }
                *(unsigned short*)&widgetbuf[i + 6] = atoi(widget_opts);
                i += 2;
            }
        widget_done:
            if (i == 0 || i & 1)
                error("Invalid widget settings");
            widgetbuf[4] = ((i - 1) >> 1);
            out_seek(256);
            out_write(widgetbuf, sizeof(widgetbuf));
        }
    }
	if (err == 0) {
		if (!rawstream) {
			out_seek(0);
            out_write(&hdr, sizeof(hdr));
		}
		//} else	/* FIXME: honour umask! */
		//	fchmod (fileno(op), 0755);
	}
	out_flush();
}

/*
 *	Scan through all the object modules in this ar archive and offer
 *	them to the linker.
 */
static void process_library(const char *name)
{
	static struct ar_hdr ah;
	off_t pos = SARMAG;
	unsigned long size;

	while(1) {
		io_lseek(pos);
		if (io_read(&ah, sizeof(ah)) != sizeof(ah))
			break;
		if (ah.ar_fmag[0] != 96 || ah.ar_fmag[1] != '\n') {
			printf("Bad fmag %02X %02X\n",
				ah.ar_fmag[0],ah.ar_fmag[1]);
			break;
		}
		size = atol(ah.ar_size);
#if 0 /* TODO */
		if (strncmp(ah.ar_name, ".RANLIB", 8) == 0)
			process_ranlib();
#endif
		libentry = ah.ar_name;
		pos += sizeof(ah);
		if (!have_object(pos, name))
			load_object(pos, 1, name);
		pos += size;
		if (pos & 1)
			pos++;
	}
	libentry = NULL;
}

/*
 *	This is called for each object module and library passed on the
 *	command line and in the order given. We process them in that order
 *	including libraries, so you need to put termcap.a before libc.a etc
 *	or you'll get errors.
 */
static void add_object(const char *name, off_t off, int lib)
{
	static uint8_t x[SARMAG];
	io_open(name);
	if (off == 0 && !lib) {
		/* Is it a bird, is it a plane ? */
		io_read(x, SARMAG);
		if (memcmp(x, ARMAG, SARMAG) == 0) {
			/* No it's a library. Do the library until a
			   pass of the library resolves nothing. This isn't
			   as fast as we'd like but we need ranlib support
			   to do faster */
			do {
				if (verbose)
					printf(":: Library scan %s\n", name);
				progress = 0;
				io_lseek(SARMAG);
				process_library(name);
				if (verbose)
					printf(":: Pass resovled %d symbols\n", progress);
			/* FIXME: if we counted unresolved symbols we might
			   be able to exit earlier ? */
			/* Don't rescan libs */
			} while(ENABLE_RESCAN && progress);
			io_close();
			return;
		}
	}
	io_lseek(off);
	load_object(off, lib, name);
	io_close();
}

/*
 *	Process the arguments, open the files and run the entire show
 */
int main(int argc, char *argv[])
{
	int opt;
	int i;
	FILE *mp = NULL;

	arg0 = argv[0];

	while ((opt = getopt(argc, argv, "rbvtsiu:o:m:f:R:A:B:C:D:G:N:Z:d:g:h:T:w:")) != -1) {
		switch (opt) {
		case 'r':
			ldmode = LD_RFLAG;
			break;
		case 'b':
			ldmode = LD_ABSOLUTE;
			strip = 1;
			break;
		case 'v':
			printf("SCC LD 0.2.3\n");
			break;
		case 't':
			verbose = 1;
			break;
		case 'o':
			outname = optarg;
			break;
		case 'm':
			mapname = optarg;
			break;
		case 'N':
			appname = optarg;
			break;
		case 'G':
			appicon = optarg;
			break;
		case 'g':
			appicon16 = optarg;
			break;
		case 'h':
			heapsize = optarg;
			break;
		case 'u':
			insert_internal_symbol(optarg, -1, 0);
			break;
		case 's':
			strip = 1;
			break;
		case 'i':
			split_id = 1;
			break;
		case 'f':
			segmentorder = optarg;
			break;
		case 'R':
			relocf = xfopen(optarg, "wb");
			break;
		case 'A':
			align = xstrtoul(optarg);
			if (align == 0)
				align = 1;
			break;
		case 'B':	/* BSS */
			base[3] = xstrtoul(optarg);
			baseset[3] = 1;
			break;
		case 'C':	/* CODE */
			base[1] = xstrtoul(optarg);
			baseset[1] = 1;
			break;
		case 'D':	/* DATA */
			base[2] = xstrtoul(optarg);
			baseset[2] = 1;
			break;
		case 'L':	/* LITERAL */
			base[5] = xstrtoul(optarg);
			baseset[5] = 1;
			break;
		case 'Z':	/* ZP / DP */
			base[4] = xstrtoul(optarg);
			baseset[4] = 1;
			break;
		case 'd':	/* SYMDATA */
			base[6] = xstrtoul(optarg);
			baseset[6] = 1;
			break;
		case 'T':	/* SYMTRANS */
			base[7] = xstrtoul(optarg);
			baseset[7] = 1;
			break;
        case 'w':
            widget_opts = optarg;
            break;
		default:
			fprintf(stderr, "%s: name ...\n", argv[0]);
			exit(1);
		}
	}
	if (outname == NULL)
		outname = "a.out";

	if (ldmode != LD_ABSOLUTE) {
		for (i = 0; i < OSEG; i++) {
			if (baseset[i]) {
				fprintf(stderr, "%s: cannot set addresses except in absolute mode.\n", argv[0]);
				exit(1);
			}
		}
	}
	if (ldmode == LD_FUZIX) {
		/* Relocatable Fuzix binaries live at logical address 0 */
		baseset[CODE] = 1;
		if (relocf) {
			base[CODE] = 0x0000;
			base[ZP] = 0x00;	/* Will be relocated past I/O if needed */
		} else {
			base[CODE] = 0x0100;
			base[ZP] = 0x00;
		}
	}
	if (ldmode == LD_FUZIX || ldmode == LD_ABSOLUTE)
		rawstream = 1;
	while (optind < argc) {
		if (verbose)
			printf("Loading %s\n", argv[optind]);
		add_object(argv[optind], 0, 0);
		optind++;
	}
	if (verbose)
		printf("Computing memory map.\n");
	set_segment_bases();
	if (verbose)
		printf("Writing output.\n");

    outptr = outbuf;
    outend = outbuf + sizeof(outbuf);
    outpos = 0;
    #ifdef SYMBUILD
	ofd = open(outname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC);
    #else
	ofd = open(outname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, 0600);
    #endif // SYMBUILD
	if (ofd < 0) {
        perror("open");
        exit(err);
	}
	if (mapname) {
		mp = xfopen(mapname, "wb");
		if (verbose)
			printf("Writing map file.\n");
		write_map_file(mp);
		fclose(mp);
	}
	write_binary(mp);
	close(ofd);
	exit(err);
}
