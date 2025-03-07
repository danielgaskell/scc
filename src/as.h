/*
 * Assembler.
 * Header file, used by all
 * parts of the assembler.
 */
#ifndef TARGET_Z80
#define TARGET_Z80
#endif

#include	<stdio.h>
#include	<string.h>
#include 	<stdlib.h>
#include	<stdint.h>
#include	<ctype.h>
#include	<setjmp.h>

#ifdef SYMBUILD
extern unsigned char _debugtrace;
#endif

/*
 * Table sizes, etc.
 */
#define	NCPS	NAMELEN			/* # of characters in symbol */
#define	NHASH	64			/* # of hash buckets */
#define	HMASK	077			/* Mask for above */
#define	NFNAME	32			/* # of characters in filename */
#define	NERR	10			/* Size of error buffer */
#define	NCODE	128			/* # of characters in code buffer */
#define	NINPUT	128			/* # of characters in input line */
#define NINBUF  1024        /* # of characters in input buffer */
#define	NLPP	60			/* # of lines on a page */
#define	XXXX	0			/* Unused value */

/*
 * Exit codes.
 */
#define	GOOD	0
#define	BAD	1

/* We generate intentionally wrapping 16bit maths for relocations */
#define TARGET_RELOC_OVERFLOW_OK

typedef	uint16_t	VALUE;		/* For symbol values */

#define ARCH OA_8080
#define ARCH_FLAGS 0
#define ARCH_CPUFLAGS OA_8080_Z80

#define CPU_Z80		80
#define CPU_Z180	180
#define CPU_Z280	280
#define CPU_Z80N	1
#define CPU_EZ80	2

/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x007F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define TSEGMENT 0x0D00			/* segments by number */
#define TEXPORT 0x0E00			/* symbol export */
#define	TNOP	0x0F00			/* nop */
#define	TRST	0x1000			/* restarts */
#define	TREL	0x1100			/* djnz, jr */
#define	TRET	0x1200			/* ret */
#define	TJMP	0x1300			/* call, jp */
#define	TPUSH	0x1400			/* push, pop */
#define	TIM	0x1500			/* im */
#define	TIO	0x1600			/* in, out */
#define	TBIT	0x1700			/* set, res, bit */
#define	TSHR	0x1800			/* sl, sr et al */
#define	TINC	0x1900			/* inc, dec */
#define	TEX	0x1A00			/* ex */
#define	TADD	0x1B00			/* add, adc, sbc */
#define	TLD	0x1C00			/* ld */
#define	TCC	0x1D00			/* condition code */
#define	TSUB	0x1E00			/* sub et al */
#define TNOP180	0x2100			/* Z180 immediate */
#define TTST180	0x2200			/* TST m/g/(hl) */
#define TIMMED8	0x2300			/* TSTIO m */
#define	TMUL	0x2400			/* MUL */
#define TIO180	0x2500			/* IN0/OUT0 */
#define TSETCPU 0x2600			/* .setcpu */
#define TNOPN	0x2700			/* Z80N implied */
#define TNEXT	0x2800			/* Z80N nextreg forms */
#define TIMMED8N 0x2900			/* TIMMED8 Next only */
#define TBS2	0x2A00			/* Barrel shifts DE,B */
#define TBPTR	0x2B00			/* BCPL/B pointer */

/*
 * Registers.
 */
#define	B	0
#define	C	1
#define	D	2
#define	E	3
#define	H	4
#define	L	5
#define	M	6
#define	A	7
#define	IX	8
#define	IY	9

#define	BC	0
#define	DE	1
#define	HL	2
#define	SP	3
#define	AF	4
#define	AFPRIME	5

#define	I	0
#define	R	1

/*
 * Condition codes.
 */
#define	CNZ	0
#define	CZ	1
#define	CNC	2
#define	CC	3
#define	CPO	4
#define	CPE	5
#define	CP	6
#define	CM	7

/*
 *	Error message numbers
 */

#define BRACKET_EXPECTED 1
#define MISSING_COMMA	2
#define SQUARE_EXPECTED 3
#define PERCENT_EXPECTED 4
#define UNEXPECTED_CHR	10
#define PHASE_ERROR	11
#define MULTIPLE_DEFS	12
#define SYNTAX_ERROR	13
#define MUST_BE_ABSOLUTE	14
#define MISSING_DELIMITER 15
#define INVALID_CONST	16
#define BRA_RANGE	17
#define CONDCODE_ONLY	18
#define INVALID_REG	19
#define ADDR_REQUIRED	20
#define INVALID_ID	21
#define REG_MUST_BE_C	22
#define DIVIDE_BY_ZERO	23
#define CONSTANT_RANGE  24
#define DATA_IN_BSS	 25
#define SEGMENT_OVERFLOW 26
#define DATA_IN_ZP	27
#define REQUIRE_Z180	28
#define	SEGMENT_CLASH	29
#define UNKNOWN_SYMBOL	30
#define TOO_MANY_JR	31
#define REQUIRE_Z80N	32

/*
 * Segments
 */
#define UNKNOWN		15
#define ABSOLUTE	0
#define CODE		1
#define DATA		2
#define BSS		    3
#define SYMDATA		6
#define SYMTRANS	7

/*
 * Expression priority
 */

#define	LOPRI	0
#define	ADDPRI	1
#define	MULPRI	2
#define	HIPRI	3

#ifndef SEGMENT_LIMIT
#define SEGMENT_LIMIT	0xFFFF
#endif

/*
 * Address description.
 */
typedef	struct	ADDR	{
	uint16_t a_type;			/* Type */
	VALUE	a_value;		/* Index offset, etc */
	uint8_t	a_segment;		/* Segment relative to */
	uint8_t a_flags;		/* To track high */
#define A_HIGH		1
#define A_LOW		2
	struct SYM *a_sym;		/* Symbol tied to this address */
					/* NULL indicates simple own segment */
}	ADDR;

/*
 * Symbol.
 */
typedef	struct	SYM	{
	struct	SYM *s_fp;		/* Link in hash */
	char	s_id[NCPS];		/* Name */
	int	s_type;			/* Type */
	VALUE	s_value;		/* Value */
	uint16_t s_number;		/* Symbol number 1..n, also usable for
					   tokens as extra data */
	int	s_segment;		/* Segment this symbol is relative to */
}	SYM;

/*
 * External variables.
 */
extern	char	*cp;
extern	char	*ip;
extern	char	ib[];
extern	int 	ifp;
extern	FILE	*ofp;
extern	FILE	*lfp;
extern	int	line;
extern	int	lmode;
extern	VALUE	laddr;
extern	SYM	sym[];
extern	int	pass;
extern	SYM	*phash[];
extern	SYM	*uhash[];
extern	int	lflag;
extern	jmp_buf	env;
extern	VALUE   dot[OSEG];
extern  int	segment;
extern	int	debug_write;
extern	char	*fname;
extern  char	*listname;
extern	int	noobj;
extern	int	cpu_flags;

extern int passbegin(int pass);
extern void list_addbyte(uint8_t);
extern void asmline(void);
extern void doflush(void);
extern void comma(void);
extern void istuser(ADDR *);
extern int symhash(char *);
extern void err(char, uint8_t);
extern void uerr(char *);
extern void aerr(uint8_t);
extern void qerr(uint8_t);
extern void storerror(int);
extern void getid(char *, int);
extern SYM *lookup(char *, SYM *[], int);
extern int symeq(char *, char *);
extern void symcopy(char *, char *);
extern int get(void);
extern int getnb(void);
extern void unget(int);
extern void getaddr(ADDR *);
extern void expr1(ADDR *, int, int);
extern void expr2(ADDR *);
extern void expr3(ADDR *, int);
extern void isokaors(ADDR *, int);
extern int outpass(void);
extern void outabsolute(addr_t);
extern void outsegment(int);
extern void outab(uint8_t);
extern void outabyte(uint8_t);
extern void outab2(uint8_t);
extern void outabchk2(ADDR *);
extern void outrab(ADDR *);
extern void outraw(ADDR *);
extern void outrabrel(ADDR *);
extern void outrawrel(ADDR *);
extern void outeof(void);
extern void outbyte(uint8_t);
extern void outflush(void);
extern void syminit(void);
extern void reservebyte(void);
extern void outscale(unsigned, unsigned);

extern char *etext[];

#include "obj.h"
