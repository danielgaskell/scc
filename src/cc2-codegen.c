/*
 *	Z80 Backend
 *
 *	The Z80 essentially has three ways of dealing with locals
 *
 *	1.	8080 style	LD HL,offset ADD HL,SP  { LD E,(HL), INC HL, LD D,(HL) }
 *				7 bytes, 21 + 20 -> 41 clocks.
 *	2.	Call helpers	Akin again to the 8080 port
 *				3 - 5 bytes, but slower (28 cycles minimum extra cost) 66 for word helper
 *	3.	IX frame pointer style
 *				LD E,(ix + n), LD D,(ix + n  + 1)
 *				6 bytes 38 cycles
 *				much higher function entry/exit cost
 *
 *	We use IX, IY and BC as "register" variables. If IX or IY is free and we are not size
 *	optimizing we use IX or IY as a frame pointer. If not helpers.
 *
 *	The situation only improves if we get to Z280 or Rabbit processors
 *
 *	R2000/R3000:	LD	HL,(SP + n)  8bit unsigned (ditto IX IY)
 *			LD	HL,(HL + n) ; IX+ IY+
 * 			LD	(HL + n), HL	; IX+ IY+
 *			LD	(SP + n), HL  ; IX+ IY+
 *
 *	Z80n		As Z80 but can add A and constants to 8/16bit directly which helps in
 *			some stack accessing by avoiding a load and push constant
 *
 *	eZ80		Adds LD BC/DE?HL,(HL) and reverse or HL/IX/IY so now frame pointer
 *			is actually useful
 *
 *	Z280		Can use IX+off for 16bit ops including loads and add so like the ez80
 *			frame pointers look useful, but this extends to other ops like ADDW INCW
 *			etc. Has LDA (ie LEA) so can LDA IX,(SP - n) etc. Can also load and
 *			save IX IY and HL from (SP + n). Can also push constants and relative
 *			addresses
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LWDIRECT 24	/* Number of __ldword1 __ldword2 etc forms for fastest access */


const char ccnormal[] = "nzz ";
const char ccinvert[] = "z nz";

/*
 *	State for the current function
 */
unsigned frame_len;		/* Number of bytes of stack frame */
unsigned sp;			/* Stack pointer offset tracking */
unsigned argbase;		/* Argument offset in current function */
unsigned unreachable;		/* Code after an unconditional jump */
unsigned func_cleanup;		/* Zero if we can just ret out */
unsigned use_fp;		/* Using a frame pointer this function */

const char *ccflags = ccnormal;/* True, False flags */

static const char *regnames[] = {	/* Register variable names */
	NULL,
	"bc",
	"ix",
	"iy"
};

void gen_tree(struct node *n)
{
	codegen_lr(n);
	fprintf(fdo, ";\n");
/*	fprintf(fdo, ";SP=%u\n", sp); */
}

/*
 *	Try and generate shorter code for stuff we can directly access
 */

/*
 *	Locals are complicated on the Z80. There is no simply cheap way
 *	to access them so put all the gunk in a helper we can refine
 *	over time for different CPU variants and frame pointers etc
 *
 *	Given a size and offset on the stack, write the code to put
 *	the value into HL without trashing BC or DE
 *
 *	If to_de is set we want the result in de, otherwise HL
 */
unsigned generate_lref(unsigned v, unsigned size, unsigned to_de)
{
	const char *name;
	const char *rp = "hl";

	if (to_de)
		rp = "de";

	if (size == 4)
		return 0;

	/* Rabbit amd Z280 have LD HL,(SP + n) */
	if (HAS_LDHLSP && v + sp <= 255 && !to_de) {
		/* FIXME: We will load an extra byte for 16bit, so hopefully non MMIO TODO */
		fprintf(fdo, "\tld hl,(sp+%u)\n", v + sp);
		return 1;
	}
	/* This has to be a local so if it is byte sized we will load the
	   low byte and crap above and all is good */
	if (v + sp == 0 && size <= 2) {
		fprintf(fdo, "\tpop %s\n\tpush %s\n", rp, rp);
		return 1;
	}
	/* Frame pointers */
	if (use_fp && v < 128 - size) {
		fprintf(fdo, "\tld %c,(iy + %u)\n", rp[1], v);
		if (size == 2)
			fprintf(fdo, "\tld %c,(iy + %u)\n", rp[0], v + 1);
		return 1;
	}

	/* Correct for current SP location */
	v += sp;

	/* Byte load is shorter inline for most cases */
	if (size == 1 && (!optsize || v >= LWDIRECT)) {
		if (to_de)
			fprintf(fdo, "\tex de,hl\n");
		fprintf(fdo, "\tld hl,0x%x\n\tadd hl,sp\n\tld l,(hl)\n", v);
		if (to_de)
			fprintf(fdo, "\tex de,hl\n");
		return 1;
	}
	/* Word load is long winded on Z80 */
	if (size == 2 && opt > 2) {
		fprintf(fdo, "\tld hl,0x%x\n\tadd hl,sp\n", WORD(v));
		if (IS_RABBIT)
			fprintf(fdo, "\tld hl,(hl + %u\n)\n", 0);
		else if (to_de)
			fprintf(fdo, "\tld e,(hl)\n\tinc hl\n\tld d,(hl)\n");
		else
			fprintf(fdo, "\tld a,(hl)\n\tinc hl\n\tld h,(hl)\n\tld l,a\n");
		return 1;
	}
	/* Via helper magic for compactness on Z80 */
	if (size == 1)
		name = "ldbyte";
	else
		name = "ldword";
	/* Check for lref out of range. Our long form helpers currently
	   trash DE so can't be used when we are loading DE */
	if (to_de && v >= 253)
		error("lrr");
	/* We do a call so the stack offset is two bigger */
	if (to_de)
		fprintf(fdo, "\tex de,hl\n");

	if (v < LWDIRECT)
		fprintf(fdo, "\tcall __%s%u\n", name, v + 2);
	else if (v < 253)
		fprintf(fdo, "\tcall __%s\n\t.byte %u\n", name, v + 2);
	else
		fprintf(fdo, "\tcall __%sw\n\t.word %u\n", name, v + 2);

	if (to_de)
		fprintf(fdo, "\tex de,hl\n");

	return 1;
}

/* Get an lref value into A without destroying HL. DE is fair game. This one
   is allowed to fail */
unsigned generate_lref_a(unsigned v)
{
	/* Correct for current SP location */
	v += sp;

	/* Sadly pop af gets us the wrong byte and sneakily adjusting
	   sp kills us in an interrupt handler. This is is still cheaper
	   than messing around */
	if (v == 0) {
		fprintf(fdo, "\tpop de\n\tpush de\n");
		fprintf(fdo, "\tld a,e\n");
		return 1;
	}
	/* Offset 1 does work however, although we almost never get that */
	if (v == 1) {
		fprintf(fdo, "\tpop af\n\tpush af\n");
		return 1;
	}
	if (use_fp) {
		fprintf(fdo, "\tld a,(iy + %d)\n", v - sp);
		return 1;
	}
	/* Byte load and inline are about the same size so inline for
	   speed */
	fprintf(fdo, "\tex de,hl\n");
	fprintf(fdo, "\tld hl,0x%x\n\tadd hl,sp\n\tld a,(hl)\n", v);
	fprintf(fdo, "\tex de,hl\n");
	return 1;
}

/*
 *	Return 1 if the node can be turned into direct access. The VOID check
 *	is a special case we need to handle stack clean up of void functions.
 *
 *	Note that we think of RDEREF as accessible. There is a specific case
 *	it is not which is loading IX or IY indirect from a pointer. The
 *	register callers must handle the fact this returns 1 and load_r_with
 *	then says 0
 */

static unsigned access_direct(struct node *n)
{
	unsigned op = n->op;

	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */

	 /* For now we only do word sized lrefs FIXME */
	if (op == T_LREF && get_size(n->type) == 2 && n->value + sp < 253)
		return 1;
	if (op != T_CONSTANT && op != T_NAME && op != T_LABEL && op != T_NREF && op != T_LBREF && op != T_REG && op != T_RREF && op != T_RDEREF)
		return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 */

static unsigned load_r_with(const char *rp, struct node *n)
{
	unsigned v = WORD(n->value);

	char r = *rp;

	switch(n->op) {
	case T_NAME:
		fprintf(fdo, "\tld %s,_%s+%u\n", rp, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		fprintf(fdo, "\tld %s,T%u+%u\n", rp, n->val2, v);
		return 1;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		fprintf(fdo, "\tld %s,0x%x\n", rp, v);
		return 1;
	case T_NREF:
		/* We know it is int or pointer */
		fprintf(fdo, "\tld %s,(_%s+%u)\n", rp, namestr(n->snum), v);
		return 1;
		break;
	/* TODO: fold together cleanly with NREF */
	case T_LBREF:
		fprintf(fdo, "\tld %s,(T%u+%u)\n", rp, n->val2, v);
		return 1;
	case T_RREF:
		/* Assumes that BC isn't corrupted yet so is already the right value. Use
		   this quirk with care */
		if (n->value == 1 && r != 'i') {
			if (r == 'd')
				fprintf(fdo, "\tld d,b\n\tld e,c\n");
			else
				fprintf(fdo, "\tld h,b\n\tld l,c\n");
			return 1;
		}
		fprintf(fdo, "\tpush %s\n\tpop %s\n", regnames[n->value], rp);
		return 1;
	case T_RDEREF:
		/* One oddity here - we can't load IX or IY from (ix) or (iy) */
		if (*rp == 'i')
			return 0;
		fprintf(fdo, "\tld %c,(%s + %u)\n", rp[1], regnames[n->value], n->val2);
		fprintf(fdo, "\tld %c,(%s + %u)\n", *rp, regnames[n->value], n->val2 + 1);
		return 1;
	default:
		return 0;
	}
	return 1;
}

static unsigned load_de_with(struct node *n)
{
	/* DE we can lref because our helpers don't touch DE so we can
	   ex de,hl around it */
	if (n->op == T_LREF)
		return generate_lref(n->value, 2, 1);
	return load_r_with("de", n);
}

static unsigned load_hl_with(struct node *n)
{
	/* An HL lref is fine as we can trash HL and A to get HL */
	if (n->op == T_LREF)
		return generate_lref(n->value, 2, 0);
	return load_r_with("hl", n);
}

static unsigned load_a_with(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		fprintf(fdo, "\tld a,0x%x\n", BYTE(n->value));
		break;
	case T_NREF:
		fprintf(fdo, "\tld a,(_%s+%u)\n", namestr(n->snum), WORD(n->value));
		break;
	case T_LBREF:
		fprintf(fdo, "\tld a,(T%u+%u)\n", n->val2, WORD(n->value));
		break;
	case T_RREF:
		/* TODO: ix/iy (can they happen ? */
		if (n->value == 1) {
			fprintf(fdo, "\tld a,c\n");
			break;
		}
		return 0;
	case T_RDEREF:
		if (n->value == 1) {
			fprintf(fdo, "\tld a,(bc)\n");
			break;
		}
		fprintf(fdo, "\tld a,(%s+%u)\n", regnames[n->value], n->val2);
		break;
	case T_LREF:
		return generate_lref_a(n->value);
	default:
		return 0;
	}
	return 1;
}

static void repeated_op(const char *o, unsigned n)
{
	while(n--)
		fprintf(fdo, "\t%s\n", o);
}

static void get_regvar(unsigned reg, struct node *n, unsigned s)
{
	if (n && (n->flags & NORETURN))
		return;
	if (reg == 1) {
		fprintf(fdo, "\tld l,c\n");
		if (s == 2)
			fprintf(fdo, "\tld h,b\n");
		return;
	}
	fprintf(fdo, "\tpush %s\n\tpop hl\n", regnames[reg]);
}

static void load_regvar(unsigned r, unsigned s)
{
	if (r == 1) {
		fprintf(fdo, "\tld c,l\n");
		if (s == 2)
			fprintf(fdo, "\tld b,h\n");
		return;
	}
	fprintf(fdo, "\tpush hl\n\tpop %s\n", regnames[r]);
}

/* We use "DE" as a name but A as register for 8bit ops... probably ought to rework one day */
static unsigned gen_deop(const char *op, struct node *n, struct node *r, unsigned sign)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	/* Generate ld e, forms of de helpers if the value is below 256 as
	   is often the case */
	if (optsize && s == 2 && r->op == T_CONSTANT) {
		unsigned v= r->value;
		if (v < 256) {
			fprintf(fdo, "\tld e,%u\n", v);
			fprintf(fdo, "\tcall __%s", op);
			if (sign)
				helper_type(n->type, sign);
			fprintf(fdo, "0d\n");
			return 1;
		}
	}
	if (s == 2) {
		if (load_de_with(r) == 0)
			return 0;
	} else {
		if (load_a_with(r) == 0)
			return 0;
	}
	if (sign)
		helper_s(n, op);
	else
		helper(n, op);
	return 1;
}

static unsigned gen_compc(const char *op, struct node *n, struct node *r, unsigned sign)
{
	unsigned s = get_size(n->type);
	/* TODO: Z280 has CPW HL,DE CPW HL, const */
	if (r->op == T_CONSTANT) {
		/* Some minimal cases to work out how best to use CCONLY */
		if (n->op == T_BANGEQ && (n->flags & CCONLY)) {
			if (r->value == 0) {
				if (s == 1) {
					fprintf(fdo, "\txor a\n\tcp l\n");
					n->flags |= USECC;
					return 1;
				}
				if (s == 2) {
					fprintf(fdo, "\tld a,h\n\tor l\n");
					n->flags |= USECC;
					return 1;
				}
			}
			if (s == 1 && r->value == 255) {
				fprintf(fdo, "\tinc l\n");
				n->flags |= USECC;
				return 1;
			}
			if (s == 2 && r->value == 0xFFFF) {
				fprintf(fdo, "\tld a,h\n\tand l\n\tinc a\n");
				n->flags |= USECC;
				return 1;
			}
			if (s == 2 && r->value <= 0xFF) {
				/* The xor is a compare resulting in 0 if equal,
				   and the or h then checks the high byte matches */
				fprintf(fdo, "\tld a,0x%x\n", (unsigned)r->value);
				fprintf(fdo, "\txor l\n");
				fprintf(fdo, "\tor h\n");
				n->flags |= USECC;
				return 1;
			}
			if (s == 2 && (r->value & 0xFF) == 0) {
				fprintf(fdo, "\tld a,0x%x\n", (unsigned)r->value >> 8);
				fprintf(fdo, "\txor h\n");
				fprintf(fdo, "\tor l\n");
				n->flags |= USECC;
				return 1;
			}
			if (s == 1) {
				fprintf(fdo, "\tld a,0x%x\n", (unsigned)r->value & 0xFF);
				fprintf(fdo, "\tcp l\n");
				return 1;
			}
		}
		if (n->op == T_EQEQ && (n->flags & CCONLY) && !(n->flags & CCFIXED)) {
			if (r->value == 0) {
				if (s == 1) {
					fprintf(fdo, "\txor a\n\tcp l\n");
					ccflags = ccinvert;
					n->flags |= USECC;
					return 1;
				}
				if (s == 2) {
					fprintf(fdo, "\tld a,h\n\tor l\n");
					ccflags = ccinvert;
					n->flags |= USECC;
					return 1;
				}
			}
			if (r->value == 255 && s == 1) {
				fprintf(fdo, "\tinc l\n");
				ccflags = ccinvert;
				n->flags |= USECC;
				return 1;
			}
			if (r->value == 0xFFFF && s == 2) {
				fprintf(fdo, "\tld a,h\n\tand l\n\tinc a\n");
				ccflags = ccinvert;
				n->flags |= USECC;
				return 1;
			}
			if (s == 2 && r->value <= 0xFF) {
				/* The xor is a compare resulting in 0 if equal,
				   and the or h then checks the high byte matches */
				fprintf(fdo, "\tld a,0x%x\n", (unsigned)r->value);
				fprintf(fdo, "\txor l\n");
				fprintf(fdo, "\tor h\n");
				ccflags = ccinvert;
				n->flags |= USECC;
				return 1;
			}
			if (s == 2 && (r->value & 0xFF) == 0) {
				fprintf(fdo, "\tld a,0x%x\n", (unsigned)r->value >> 8);
				fprintf(fdo, "\txor h\n");
				fprintf(fdo, "\tor l\n");
				ccflags = ccinvert;
				fprintf(fdo, ";EQEQ z true z nz\n");
				n->flags |= USECC;
				return 1;
			}
			if (s == 1) {
				fprintf(fdo, "\tld a,0x%x\n", (unsigned)r->value & 0xFF);
				fprintf(fdo, "\tcp l\n");
				ccflags = ccinvert;
				n->flags |= USECC;
				return 1;
			}
		}
		/* TODO : float helper */
		if (r->value == 0 && r->type != FLOAT) {
			char buf[10];
			strcpy(buf, op);
			strcat(buf, "0");
			if (sign)
				helper_s(n, buf);
			else
				helper(n, buf);
			n->flags |= ISBOOL;
			return 1;
		}
	}
	/* Generate short cases with BC */
	if (r->op == T_RREF && s == 2 && r->value == 1 && (n->flags & CCONLY)) {
		if (n->op == T_BANGEQ) {
			fprintf(fdo, "\tor a\n\tsbc hl,bc\n");
			n->flags |= USECC;
			return 1;
		}
		if (n->op == T_EQEQ && !(n->flags & CCFIXED)) {
			fprintf(fdo, "\tor a\n\tsbc hl,bc\n");
			ccflags = ccinvert;
			n->flags |= USECC;
			return 1;
		}
	}
	/* We need to set a variable to the type of branch to use for the condition and then pick it up
	   (once) in the following branch instruction to do more than the basics */
	if (n->op == T_BANGEQ && (n->flags & CCONLY)) {
		if (s == 1 && load_a_with(r) == 1) {
			fprintf(fdo, "\tcp l\n");
			n->flags |= USECC;
			return 1;
		}
		if (s == 2 && load_de_with(r) == 1) {
			fprintf(fdo, "\tor a\n\tsbc hl,de\n");
			n->flags |= USECC;
			return 1;
		}
	}
	if (n->op == T_EQEQ && (n->flags & CCONLY) && !(n->flags & CCFIXED)) {
		if (s == 1 && load_a_with(r) == 1) {
			fprintf(fdo, "\tcp l\n");
			ccflags = ccinvert;
			n->flags |= USECC;
			return 1;
		}
		if (s == 2 && load_de_with(r) == 1) {
			fprintf(fdo, "\tor a\n\tsbc hl,de\n");
			ccflags = ccinvert;
			n->flags |= USECC;
			return 1;
		}
	}
	/* TODO: unsigned gt and lt etc including moving const by one to
	   get easy to inline form */
	if (gen_deop(op, n, r, sign)) {
		n->flags |= ISBOOL;
		return 1;
	}
	return 0;
}

static int count_mul_cost(register unsigned n)
{
	register int cost = 0;
	if ((n & 0xFF) == 0) {
		n >>= 8;
		cost += 3;		/* mov mvi */
	}
	while(n > 1) {
		if (n & 1)
			cost += 3;	/* push pop add hl,de */
		n >>= 1;
		cost++;			/* dad h */
	}
	return cost;
}

/* Write the multiply for any value > 0 */
static void write_mul(unsigned n)
{
	unsigned pops = 0;
	if ((n & 0xFF) == 0) {
		fprintf(fdo, "\tld h,l\n\tld l,0\n");
		n >>= 8;
	}
	while(n > 1) {
		if (n & 1) {
			pops++;
			fprintf(fdo, "\tpush hl\n");
		}
		fprintf(fdo, "\tadd hl,hl\n");
		n >>= 1;
	}
	while(pops--) {
		fprintf(fdo, "\tpop de\n\tadd hl,de\n");
	}
}

static unsigned can_fast_mul(unsigned s, unsigned n)
{
	/* Pulled out of my hat 8) */
	unsigned cost = 15 + 3 * opt;
	if (s > 2)
		return 0;

	/* The base cost of the helper is 6 lxi de, n; call, but this may be too aggressive
	   given the cost of mulde TODO */
	if (optsize)
		cost = 10;
	if (n == 0 || count_mul_cost(n) <= cost)
		return 1;
	return 0;
}

static void gen_fast_mul(unsigned s, unsigned n)
{

	if (n == 0)
		fprintf(fdo, "\tld hl,0x0\n");
	else
		write_mul(n);
}

static unsigned gen_fast_div(unsigned n, unsigned s, unsigned u)
{
	u &= UNSIGNED;
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n == 256 && u) {
		fprintf(fdo, "\tld l,h\n\tld h,0x0\n");
		return 1;
	}
	if (n & (n - 1))
		return 0;


	if (u) {
		while(n > 1) {
			fprintf(fdo, "\tsrl h\n\trr l\n");
			n >>= 1;
		}
	} else {
		while(n > 1) {
			fprintf(fdo, "\tsra h\n\trr l\n");
			n >>= 1;
		}
	}
	return 1;
}


/* TODO : we could in theory optimize xor 255 with cpl ? */
static unsigned gen_logicc(struct node *n, unsigned s, const char *op,
				unsigned v, unsigned code)
{
	unsigned h = (v >> 8) & 0xFF;
	unsigned l = v & 0xFF;

	/* Rabbit has 16bit logic operators */
	if (s == 2 && IS_RABBIT) {
		if (load_de_with(n) == 0)
			return 0;
		fprintf(fdo, "\t%s hl,de\n", op);
		return 1;
	}

	if (s > 2 || (n && n->op != T_CONSTANT))
		return 0;

	/* If we are trying to be compact only inline the short ones */
	if (optsize && ((h != 0 && h != 255) || (l != 0 && l != 255)))
		return 0;

	if (s == 2) {
		if (h == 0) {
			if (code == 1)
				fprintf(fdo, "\tld h,0x0\n");
		}
		else if (h == 255 && code != 3) {
			if (code == 2)
				fprintf(fdo, "\tld h,0xff\n");
		} else {
			fprintf(fdo, "\tld a,h\n\t%s 0x%x\n\tld h,a\n", op, h);
		}
	}
	if (l == 0) {
		if (code == 1)
			fprintf(fdo, "\tld l,0x0\n");
	} else if (l == 255 && code != 3) {
		if (code == 2)
			fprintf(fdo, "\tld l,0xff\n");
	} else {
		fprintf(fdo, "\tld a,l\n\t%s 0x%x\n\tld l,a\n", op, l);
	}
	return 1;
}

static unsigned gen_fast_remainder(unsigned n, unsigned s)
{
	unsigned mask;
	if (s != 2)
		return 0;
	if (n == 1) {
		fprintf(fdo, "\tld hl,0x00\n");
		return 1;
	}
	if (n == 256) {
		fprintf(fdo, "\tld h,0x0\n");
		return 1;
	}
	if (n & (n - 1))
		return 0;
	if (!optsize) {
		mask = n - 1;
		gen_logicc(NULL, s, "and", mask, 1);
		return 1;
	}
	return 0;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	The 8080 is pretty basic so there isn't a lot we turn around here. As
 *	proof of concept we deal with the add case. Other processors may be
 *	able to handle a lot more.
 *
 *	If your processor is good at subtracts you may also want to rewrite
 *	constant on the left subtracts in the rewrite rules into some kind of
 *	rsub operator.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned v;
	unsigned nr = n->flags & NORETURN;

	/* We only deal with simple cases for now */
	if (r) {
		if (!access_direct(n->right))
			return 0;
		v = r->value;
	}

	switch (n->op) {
	case T_CLEANUP:
		gen_cleanup(v);
		return 1;
	case T_NSTORE:
		if (s > 2)
			return 0;
		if (s == 1)
			fprintf(fdo, "\tld a,l\n");
		fprintf(fdo, "\tld (_%s+%u),", namestr(n->snum), WORD(n->value));
			return 1;
		if (s == 1)
			fprintf(fdo, "a\n");
		else
			fprintf(fdo, "hl\n");
		/* TODO 4/8 for long etc */
		return 0;
	case T_LBSTORE:
		if (s > 2)
			return 0;
		if (s == 1)
			fprintf(fdo, "\tld a,l\n");
		fprintf(fdo, "\tld (T%u+%u), ", n->val2, v);
		if (s == 1)
			fprintf(fdo, "a\n");
		else
			fprintf(fdo, "hl\n");
		return 1;
	case T_RSTORE:
		load_regvar(n->value, s);
		return 1;
	case T_EQ:
		/* The address is in HL at this point */
		if (s == 1) {
			/* We need to end up with the value in l if this is not NORETURN, also
			   we can optimize constant a step more */
			if (r->op == T_CONSTANT && nr)
				fprintf(fdo, "\tld (hl),0x%x\n", v & 0xFF);
			else {
				if (load_a_with(r) == 0)
					return 0;
				fprintf(fdo, "\tld (hl),a\n");
				if (!nr)
					fprintf(fdo, "\tld l,a\n");
			}
			return 1;
		}
		if (s == 2 && r->op == T_CONSTANT) {
			if (nr)	{ /* usual case */
				fprintf(fdo, "\tld (hl),0x%x\n", v & 0xFF);
				fprintf(fdo, "\tinc hl\n");
				fprintf(fdo, "\tld (hl),0x%x\n", v >> 8);
			} else {
				fprintf(fdo, "\tld de,0x%x\n", v);
				fprintf(fdo, "\tld (hl),e\n");
				fprintf(fdo, "\tinc hl\n");
				fprintf(fdo, "\tld (hl),d\n");
				fprintf(fdo, "\tex de,hl\n");
			}
			return 1;
		}
		if (s == 4 && r->op == T_CONSTANT) {
			if (v < 0x02) {
				fprintf(fdo, "\tcall __assign%ul\n", v);
				return 1;
			}
			if (v <= 0xFF) {
				fprintf(fdo, "\tld a,0x%x\n", v);
				helper(n, "assign0la");
				return 1;
			}
			if (v <= 0xFFFF) {
				fprintf(fdo, "\tld de,%u\n", v);
				helper(n, "assignl0de");
				return 1;
			}
		}
		return 0;
	case T_PLUS:
		/* Zero should be eliminated in cc1 FIXME */
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (v < 4 && s <= 2) {
				if (s == 1)
					repeated_op("inc l", v);
				else
					repeated_op("inc hl", v);
				return 1;
			}
		}
		/* Faster case for foo + bc to avoid transfer to de */
		if (r->op == T_RREF && v == 1) {
			fprintf(fdo, "\tadd hl,bc\n");
			return 1;
		}
		if (s <= 2) {
			/* LHS is in HL at the moment, end up with the result in HL */
			if (s == 1) {
				if (load_a_with(r) == 0)
					return 0;
				fprintf(fdo, "\tld e,a\n");
			}
			if (s > 2 || load_de_with(r) == 0)
				return 0;
			fprintf(fdo, "\tadd hl,de\n");
			return 1;
		}
		return 0;
	case T_MINUS:
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (v < 6 && s <= 2) {
				if (s == 1)
					repeated_op("dec l", v);
				else
					repeated_op("dec hl", v);
				return 1;
			}
			fprintf(fdo, "\tld de,0x%x\n", 65536 - v);
			fprintf(fdo, "\tadd hl,de\n");
			return 1;
		}
		/* Avoid loading BC into DE unnecessarily. No shortcut for IXY though */
		if (r->op == T_REG && v == 1) {
			fprintf(fdo, "\tor a\n\tsbc hl,bc\n");
			return 1;
		}
		/* TODO: Can we ex de,hl, load into hl and go? - check direction too */
		if (load_de_with(r) == 0)
			return 0;
		fprintf(fdo, "\tor a\n\tsbc hl,de\n");
		return 1;
	case T_STAR:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && can_fast_mul(s, v)) {
				gen_fast_mul(s, v);
				return 1;
			}
		}
		return gen_deop("mulde", n, r, 0);
	case T_SLASH:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && gen_fast_div(s, v, n->type))
				return 1;
		}
		return gen_deop("divde", n, r, 1);
	case T_PERCENT:
		if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
			if (s <= 2 && gen_fast_remainder(s, v))
				return 1;
		}
		return gen_deop("remde", n, r, 1);
	case T_AND:
		if (gen_logicc(r, s, "and", v, 1))
			return 1;
		if (r->op == T_CONSTANT && s <= 2) {
			int b = bitcheck0(v, s);
			if (b >= 0) {
				/* Single clear bit */
				if (b < 8)
					fprintf(fdo, "\tres %u,l\n", b);
				else
					fprintf(fdo, "\tres %u,h\n", b - 8);
				return 1;
			}
		}
		return gen_deop("bandde", n, r, 0);
	case T_OR:
		if (r->op == T_CONSTANT && v == 0)
			return 1;
		if (r->op == T_CONSTANT && s <= 2) {
			int b = bitcheck1(v, s);
			if (b >= 0) {
				/* Single set bit */
				if (b < 8)
					fprintf(fdo, "\tset %u,l\n", b);
				else
					fprintf(fdo, "\tset %u,h\n", b - 8);
				return 1;
			}
			/* For small values it's more efficient to do this inline even
			   in -Os */
			if (!(v & 0xFF00)) {
				fprintf(fdo, "\tld a,0x%x\n\tor l\n\tld l,a\n", v);
				return 1;
			}
			if (!(v & 0x00FF)) {
				fprintf(fdo, "\tld a,0x%x\n\tor h\n\tld h,a\n", v >> 8);
				return 1;
			}
		}
		if (gen_logicc(r, s, "or", r->value, 2))
			return 1;
		return gen_deop("borde", n, r, 0);
	case T_HAT:
		/* For small values it's more efficient to do this inline even
		   in -Os */
		if (r->op == T_CONSTANT && s <= 2) {
			if (v == 0)
				return 1;
			if (!(v & 0xFF00)) {
				fprintf(fdo, "\tld a,0x%x\n\txor l\n\tld l,a\n", v);
				return 1;
			}
			if (!(v & 0x00FF)) {
				fprintf(fdo, "\tld a,0x%x\n\txor h\n\tld h,a\n", v >> 8);
				return 1;
			}
		}
		if (gen_logicc(r, s, "xor", r->value, 3))
			return 1;
		return gen_deop("bxorde", n, r, 0);
	/* TODO: add sbc hl,de etc versions of these when we can - or in optimizer ? */
	case T_EQEQ:
		return gen_compc("cmpeq", n, r, 0);
	case T_GTEQ:
		return gen_compc("cmpgteq", n, r, 1);
	case T_GT:
		return gen_compc("cmpgt", n, r, 1);
	case T_LTEQ:
		return gen_compc("cmplteq", n, r, 1);
	case T_LT:
		return gen_compc("cmplt", n, r, 1);
	case T_BANGEQ:
		return gen_compc("cmpne", n, r, 0);
	case T_LTLT:
		/* TODO: Z80N has some shift helpers */
		if (s <= 2 && r->op == T_CONSTANT && v <= 8) {
			if (r->value < 8)
				repeated_op("add hl,hl", v);
			else
				fprintf(fdo, "\tld h,l\n\tld l,0x0\n");
			return 1;
		}
		return gen_deop("shlde", n, r, 0);
	case T_GTGT:
		/* >> by 8 unsigned */
		if (s == 2 && (n->type & UNSIGNED) && r->op == T_CONSTANT && r->value == 8) {
			fprintf(fdo, "\tld l,h\n\tld h,0x0\n");
			return 1;
		}
		return gen_deop("shrde", n, r, 1);
	/* Shorten post inc/dec if result not needed - in which case it's the same as
	   pre inc/dec */
	case T_PLUSPLUS:
		/* This occurs a lot with pointers */
		if (s == 2 && r->op == T_CONSTANT) {
			if (v <= 4) {
				fprintf(fdo, "\tcall __postinc%u\n", v);
				return 1;
			}
			if (v <= 255) {
				fprintf(fdo, "\tld e,0x%x\n", v);
				fprintf(fdo, "\tcall __postince\n");
				return 1;
			}
			fprintf(fdo, "\tld de,0x%x\n", v);
			fprintf(fdo, "\tcall __postincde\n");
			return 1;
		}
		if (!(n->flags & NORETURN))
			return 0;
	case T_PLUSEQ:
		if (s == 1) {
			if (r->op == T_CONSTANT && v < 4 && (n->flags & NORETURN)) {
				repeated_op("inc (hl)", v);
			} else {
				if (load_a_with(r) == 0)
					return 0;
				fprintf(fdo, "\tadd a,(hl)\n\tld (hl),a\n");
				if (!(n->flags & NORETURN))
					fprintf(fdo, "\tld l,a\n");
			}
			return 1;
		}
		/* These are very common */
		if (s == 2 && r->op == T_CONSTANT) {
			if (v <= 4) {
				fprintf(fdo, "\tcall __pluseq%u\n", v);
				return 1;
			}
			if (v < 256) {
				fprintf(fdo, "\tld e,%u\n", v);
				fprintf(fdo, "\tcall __pluseqe\n");
				return 1;
			}
			/* High only */
			if ((v & 0x00FF) == 0) {
				fprintf(fdo, "\tinc hl\n");
				if (v < (4 << 8))
					repeated_op("inc (hl)", v >> 8);
				else {
					fprintf(fdo, "\tld a,0x%x\n", v >> 8);
					fprintf(fdo, "\tadd a,(hl)\n");
					fprintf(fdo, "\tld (hl),a\n");
				}
				return 1;
			}
		}
		return gen_deop("pluseqde", n, r, 0);
	case T_MINUSMINUS:
		/* This occurs a lot with pointers */
		if (s == 2 && r->op == T_CONSTANT) {
			if (v <= 4) {
				fprintf(fdo, "\tcall __postdec%u\n", v);
				return 1;
			}
			if (v <= 255) {
				fprintf(fdo, "\tld e,0x%x\n", v);
				fprintf(fdo, "\tcall __postdece\n");
				return 1;
			}
			fprintf(fdo, "\tld de,0x%x\n", v);
			fprintf(fdo, "\tcall __postdecde\n");
			return 1;
		}
		if (!(n->flags & NORETURN))
			return 0;
	case T_MINUSEQ:
		if (s == 1) {
			/* Shortcut for small 8bit values */
			if (r->op == T_CONSTANT && r->value < 4 && (n->flags & NORETURN)) {
				repeated_op("dec (hl)", r->value);
			} else {
				/* Subtraction is not transitive so this is
				   messier */
				if (r->op == T_CONSTANT) {
					if (r->value == 1)
						fprintf(fdo, "\tld a,(hl)\n\tdec a\n\tld (hl),a\n");
					else
						fprintf(fdo, "\tld a,(hl)\n\tsub 0x%x\n\tld (hl),a",
							(int)r->value);
				} else {
					if (load_a_with(r) == 0)
						return 0;
					fprintf(fdo, "\tcpl\n\tinc a\n\n");
					fprintf(fdo, "\tsub (hl)\n\tld (hl),a\n");
				}
				if (!(n->flags & NORETURN))
					fprintf(fdo, "\tld l,a\n");
			}
			return 1;
		}
		/* These are very common */
		if (s == 2 && r->op == T_CONSTANT) {
			if (v <= 4) {
				fprintf(fdo, "\tcall __minuseq%u\n", v);
				return 1;
			}
			if (v < 256) {
				fprintf(fdo, "\tld e,%u\n", v);
				fprintf(fdo, "\tcall __minuseqe\n");
				return 1;
			}
		}
		return gen_deop("minuseqde", n, r, 0);
	case T_ANDEQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			fprintf(fdo, "\tand (hl)\n\tld (hl),a\n");
			if (!(n->flags & NORETURN))
				fprintf(fdo, "\tld l,a\n");
			return 1;
		}
		return gen_deop("andeqde", n, r, 0);
	case T_OREQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			fprintf(fdo, "\tor (hl)\n\tld (hl),a\n");
			if (!(n->flags & NORETURN))
				fprintf(fdo, "\tld l,a\n");
			return 1;
		}
		return gen_deop("oreqde", n, r, 0);
	case T_HATEQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			fprintf(fdo, "\txor (hl)\n\tld (hl),a\n");
			if (!(n->flags & NORETURN))
				fprintf(fdo, "\tld l,a\n");
			return 1;
		}
		return gen_deop("xoreqde", n, r, 0);
	}
	return 0;
}

/*
 *	Allow the code generator to shortcut the generation of the argument
 *	of a single argument operator (for example to shortcut constant cases
 *	or simple name loads that can be done better directly)
 */
unsigned gen_uni_direct(struct node *n)
{
	struct node *r = n->right;
	unsigned v;
	/* There are some uni operations on registers we can do
	   non destructively but directly on BC */
	if (r && r->op == T_RREF && r->value == 1) {
		if (n->op == T_BTST) {
			/* Bit test on BC - always CCONLY */
			v = n->value;
			if (v < 8)
				fprintf(fdo, "\tbit %u,c\n", v);
			else
				fprintf(fdo, "\tbit %u,b\n", v - 8);
		}
	}
	return 0;
}

static unsigned reg_canincdec(unsigned reg, struct node *n, unsigned s, int v)
{
	/* We only deal with short and char for register */
	if (reg > 1 && s != 2)
		return 0;
	/* Is the shortcut worth it ? */
	if (n->op != T_CONSTANT || v > 8 || v < -8)
		return 0;
	return 1;
}

static unsigned reg_incdec(unsigned reg, unsigned s, int v)
{
	char buf[16];
	char *op = "inc";
	if (v < 0) {
		op = "dec";
		v = -v;
	}
	if (s == 1)
		sprintf(buf, "%s %c", op, regnames[reg][1]);
	else
		sprintf(buf, "%s %s", op, regnames[reg]);

	repeated_op(buf, v);
	return 1;
}

static void reghelper_s(struct node *n, const char *p)
{
	static char x[16];
	strcpy(x, p);
	switch(n->left->value) {
	case 2:
		*x = 'i';
		x[1] = 'x';
		break;
	case 3:
		*x = 'i';
		x[1] = 'y';
		break;
	}
	helper_s(n, p);
}

static void reghelper(struct node *n, const char *p)
{
	static char x[16];
	strcpy(x, p);
	switch(n->left->value) {
	case 2:
		*x = 'i';
		x[1] = 'x';
		break;
	case 3:
		*x = 'i';
		x[1] = 'y';
		break;
	}
	helper(n, p);
}

static void reg_logic(struct node *n, unsigned s, unsigned op, const char *i)
{
	/* TODO : constant optimization */
	codegen_lr(n->right);
	/* HL is now the value to combine with BC */
	if (opt > 1 && n->left->value == 1) {
		/* TODO - can avoid the reload into HL if NORETURN */
		/* TODO: check we never end up with IX or IY here */
		if (s == 2)
			fprintf(fdo, "\tld a,b\n\t%s h\n\tld b,a\n\tld h,a\n", i + 2);
		fprintf(fdo, "\tld a,c\n\t%s c\n\tld c,a\n\tld l,a\n", i + 2);
	} else {
		reghelper(n, i);
		get_regvar(n->left->value, NULL, s);
	}
}

/* Operators where we can push CCONLY downwards */
static unsigned is_ccdown(struct node *n)
{
	unsigned op = n->op;
	if (op == T_ANDAND || op == T_OROR)
		return 1;
	if (op == T_BOOL)
		return 1;
	if (op == T_BANG && !(n->flags & CCFIXED))
		return 1;
	return 0;
}

/* Operators that we known to handle as CCONLY if possible
   TODO: add logic ops as we can BIT many of them */
static unsigned is_cconly(struct node *n)
{
	unsigned op = n->op;
	if (op == T_EQEQ || op == T_BANGEQ || op == T_BYTEEQ ||
		op == T_BYTENE || op == T_ANDAND || op == T_OROR ||
		op == T_BOOL || op == T_BTST)
		return 1;
	if (op == T_BANG && !(n->flags & CCFIXED))
		return 1;
	return 0;
}

/*
 *	Try and push CCONLY down through the tree
 */
static void propogate_cconly(struct node *n)
{
	struct node *l, *r;
	unsigned sz = get_size(n->type);
	unsigned val;

	l = n->left;
	r = n->right;


/*	fprintf(fdo, "; considering %x %x\n", n->op, n->flags); */
	/* Only do this for nodes that are CCONLY. For example if we hit
	   an EQ (assign) then whilst the result of the assign may be
	   CC only, the subtree of the assignment is most definitely not */
	if (n->op != T_AND && !is_cconly(n) && !(n->flags & CCONLY))
		return;

	/* We have to special case BIT unfortunately, and this is ugly */

	/* A common C idiom is if (a & bit) which we can rewrite into
	   bit n,h or bit n,l */

	if (n->op == T_AND) {
/*		fprintf(fdo, ";AND %x %x %x\n", n->op, r->op, n->flags); */
		if (r->op == T_CONSTANT && sz <= 2) {
			val = bitcheck1(r->value, sz);
			if (val != -1) {
				n->op = T_BTST;
				n->value = val;
				free_node(r);
				n->right = l;
				n->left = NULL;
				r = l;
				l = NULL;
			}
		} else
			return;
	}

	n->flags |= CCONLY;
	/* Deal with the CCFIXED limitations for now */
	if (n->flags & CCFIXED) {
		if (l)
			l->flags |= CCFIXED;
		if (r)
			r->flags |= CCFIXED;
	}
/*	fprintf(fdo, ";made cconly %x\n", n->op); */
	/* Are we a node that can CCONLY downwards */
	if (is_ccdown(n)) {
/*		fprintf(fdo, ";ccdown of %x L\n", n->op); */
		if (l)
			propogate_cconly(l);
/*		fprintf(fdo, ";ccdown cont %x R\n", n->op); */
		if (r)
			propogate_cconly(r);
/*		fprintf(fdo, ";ccdown done %x\n", n->op); */
	}
}

/*
 *	Allow the code generator to short cut any subtrees it can directly
 *	generate. Also our point to do any private tree mods downwards
 */
unsigned gen_shortcut(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned v;
	struct node *r = n->right;
	struct node *l = n->left;
	unsigned nr = n->flags & NORETURN;

	/* Don't generate unreachable blocks */
	if (unreachable)
		return 1;

	/* Try and rewrite this node subtree for CC only */
	if ((opt || optsize) && (n->flags & CCONLY))
		propogate_cconly(n);

	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		l->flags |= NORETURN;
		codegen_lr(l);
 		/* Parent determines child node requirements */
		codegen_lr(r);
		r->flags |= nr;
		return 1;
	}
	if (n->op == T_BOOL) {
		/* Logic via jumping always uses CC in our case */
		if (r->op == T_ANDAND || r->op == T_OROR)
			r->flags |= USECC;
		codegen_lr(r);
		/* Condition code only doesn't need us to do anything if
		   our child is already CCONLY */
/*		fprintf(fdo, "; N %x R %x NF %x RF %x\n",
			n->op, r->op, n->flags, r->flags); */
		/* If we want CC flags and the subtree is CC flags do nothing */
		if ((n->flags & CCONLY) && (r->flags & USECC)) {
			n->flags |= USECC;
			return 1;
		}
		if (r->flags & USECC) {
			/* Should never happen */
			fprintf(fdo, "\t;BOTCH %04X:%04X\n", r->op, r->flags);
			fprintf(fdo, "\tcall __cctobool\n");
		}
		/* If the result is bool do nothing */
		if (r->flags & ISBOOL)
			return 1;
		s = get_size(r->type);
		if (s <= 2 && (n->flags & CCONLY)) {
			if (s == 2)
				fprintf(fdo, "\tld a,h\n\tor l\n");
			else
				fprintf(fdo, "\tld a,l\n\tor a\n");
			return 1;
		}
		if (IS_RABBIT && s <= 2) {
			/* quick way to bool byte */
			if (s == 1)
				fprintf(fdo, "\tld h,l\n");
			fprintf(fdo, "\tbool hl\n");
		} else
			/* Too big or value needed */
			helper(n, "bool");
		n->flags |= ISBOOL;
		return 1;
	}
	/* Re-order assignments we can do the simple way */
	/* TODO: LBSTORE */
	if (n->op == T_NSTORE && s <= 2) {
		/* Handle const nr specially */
		if (s == 1 && r->op == T_CONSTANT && (n->flags & NORETURN)) {
			fprintf(fdo, "\tld a,0x%x\n", (uint8_t)r->value);
		} else {
			codegen_lr(n->right);
			if (s == 1)
				fprintf(fdo, "\tld a,l\n");
		}
		/* Expression result is now in HL or A or both as needed */
		if (s == 1)
			fprintf(fdo, "\tld (_%s+%u), a\n", namestr(n->snum), WORD(n->value));
		else
			fprintf(fdo, "\tld (_%s+%u), hl\n", namestr(n->snum), WORD(n->value));
		return 1;
	}
	/* Locals we can do on some later processors, Z80 is doable but messy - so not worth it */
	if (n->op == T_LSTORE && s <= 2) {
		v = n->value;
		if (use_fp && v < 128 - s && r->op == T_CONSTANT && (n->flags & NORETURN)) {
			/* Constant assign odd case */
			fprintf(fdo, "\tld (iy + %u), %d\n",v ,
				((unsigned)r->value) & 0xFF);
			if (s == 2)
				fprintf(fdo, "\tld (iy + %u), %d\n", v + 1,
					((unsigned)r->value) >> 8);
			return 1;
		}
		/* Rabbit and Z280 */
		if (HAS_LDHLSP && v + sp < 255 && s == 2) {
			codegen_lr(n->right);
			fprintf(fdo, "\tld (sp + %u),hl\n", v + sp);
			return 1;
		}
		if (use_fp && v <= 128 - s) {
			codegen_lr(n->right);
			fprintf(fdo, "\tld (iy + %u), l\n", v);
			if (s == 2)
				fprintf(fdo, "\tld (iy + %u), h\n", v + 1);
			return 1;
		}
		/* General case */
		if (v + sp == 0 && s == 2) {
			/* The one case Z80 is worth doing */
			codegen_lr(n->right);
			if (nr)
				fprintf(fdo, "\tex (sp),hl\n");
			else
				fprintf(fdo, "\tpop af\n\tpush hl\n");
			return 1;
		}
	}
	/* Shortcut any initialization of BC/IX/IY we can do directly */
	if (n->op == T_RSTORE) {
		if (s == 2 && nr)
			return load_r_with(regnames[n->value], r);
		/* Can in theory do byte sized shortcuts but need a suitable
		   helper adding for the few we can - notably c,(ix+n) */
	}

	/* Assignment to *BC is byte pointer always */
	if (n->op == T_REQ) {
		const char *rp = regnames[n->value];
		switch(n->value) {
		case 1:
			/* Try and get the value into A */
			if (!load_a_with(r)) {
				codegen_lr(r);		/* If not then into HL */
				fprintf(fdo, "\tld a,l\n");
			}
			fprintf(fdo, "\tld (bc),a\n");	/* Do in case volatile */
			if (!nr)
				fprintf(fdo, "\tld l,a\n");
			return 1;
		case 2:
		case 3:
			if (s == 1) {
				if (!load_a_with(r)) {
					codegen_lr(r);
					fprintf(fdo, "\tld (%s + %u),l\n", rp, n->val2);
					return 1;
				}
				if (*rp == 'i')
					fprintf(fdo, "\tld (%s + %u),a\n", rp, n->val2);
				else
					fprintf(fdo, "\tld (%s),a\n", rp);
				return 1;
			}
			if (s == 2) {
				if (!load_hl_with(r))
					codegen_lr(r);
				fprintf(fdo, "\tld (%s + %u),l\n", rp, n->val2);
				fprintf(fdo, "\tld (%s + %u),h\n", rp, n->val2 + 1);
				return 1;
			}
			if (s == 4) {
				/* No fast HL load with a 32bit value */
				codegen_lr(r);
				fprintf(fdo, "\tld (%s + %u),l\n", rp, n->val2);
				fprintf(fdo, "\tld (%s + %u),h\n", rp, n->val2 + 1);
				fprintf(fdo, "\tld de, (__hireg)\n");
				fprintf(fdo, "\tld (%s + %u),e\n", rp, n->val2 + 2);
				fprintf(fdo, "\tld (%s + %u),d\n", rp, n->val2 + 3);
				return 1;
			}
		}
		return 0;
	}
	/* Special case - we can compare BC with zero efficiently for flags */
	if (l && r && r->op == T_CONSTANT && r->value == 0 &&
		l->op == T_RREF && l->value == 1 && (n->flags & CCONLY)) {
		if (n->op == T_BANGEQ) {
			if (s == 1) {
				fprintf(fdo, "\txor a\n\tcp c\n");
				n->flags |= USECC;
				return 1;
			}
			if (s == 2) {
				fprintf(fdo, "\tld a,c\n\tor b\n");
				n->flags |= USECC;
				return 1;
			}
		}
		if (n->op == T_EQEQ && !(n->flags & CCFIXED)) {
			if (s == 1) {
				fprintf(fdo, "\txor a\n\tcp c\n");
				ccflags = ccinvert;
				n->flags |= USECC;
				return 1;
			}
			if (s == 2) {
				fprintf(fdo, "\tld a,c\n\tor b\n");
				ccflags = ccinvert;
				n->flags |= USECC;
				return 1;
			}
		}
		/* TODO: we can also do >= 0 and < 0 (bit 7,b) */
	}
	/* Register targetted ops. These are totally different to the normal EQ ops because
	   we don't have an address we can push and then do a memory op */
	if (l && l->op == T_REG) {
		unsigned reg = l->value;
		v = r->value;
		switch(n->op) {
		case T_PLUSPLUS:
			/* x++ with result discarded can be treated as ++x */
			if (nr) {
				if (reg_canincdec(reg, r, s, v)) {
					reg_incdec(reg, s, v);
					return 1;
				}
			}
			if (!nr) {
				fprintf(fdo, "\tpush %s\n", regnames[reg]);
				sp += 2;
			}
			/* Fall through */
		case T_PLUSEQ:
			if (reg_canincdec(reg, r, s, v)) {
				reg_incdec(reg, s, v);
				if (nr)
					return 1;
				if (n->op == T_PLUSEQ) {
					get_regvar(reg, n, s);
				}
			} else {
				/* Amount to add into HL */
				if (reg == 1) {
					codegen_lr(r);
					fprintf(fdo, "\tadd hl,bc\n");
					fprintf(fdo, "\tld c,l\n");
					if (s == 2)
						fprintf(fdo, "\tld b,h\n");
				} else {
					codegen_lr(r);
					fprintf(fdo, "\tex de,hl\n\tadd %s,de\n", regnames[reg]);
					if (!(n->flags & NORETURN))
						get_regvar(reg, n, s);
				}
			}
			if (n->op == T_PLUSPLUS && !(n->flags & NORETURN)) {
				fprintf(fdo, "\tpop hl\n");
				sp -= 2;
			}
			return 1;
		case T_MINUSMINUS:
			if (!(n->flags & NORETURN)) {
				if (reg_canincdec(reg, r, s, -v)) {
					get_regvar(reg, n, s);
					reg_incdec(reg, s, -v);
					return 1;
				}
				/* Z80 has no 16bit subtract for IX or IT */
				if (reg != 1) {
					if (r->op == T_CONSTANT) {
						fprintf(fdo, "\tld de,%u\n", -v);
						fprintf(fdo, "\tadd %s,de\n", regnames[reg]);
						return 1;
					}
					codegen_lr(r);
					helper(n, "negate");
					fprintf(fdo, "\tex de,hl\n");
					fprintf(fdo, "\tadd %s,de\n", regnames[reg]);
					return 1;
				}
				codegen_lr(r);
				if (s == 1) {
					fprintf(fdo, "\tld a,c\n\tsub l\n\tld l,c\n\tld c,a\n");
					return 1;
				}
				reghelper(n, "bcsub");
				return 1;
			}
			/* If we don't care about the return they look the same so fall
			   through */
		case T_MINUSEQ:
			if (reg_canincdec(reg, r, s, -v)) {
				reg_incdec(reg, s, -v);
				get_regvar(reg, n, s);
				return 1;
			}
			/* Get the subtraction value into HL */
			codegen_lr(r);
			if (reg != 1) {
				if (r->op == T_CONSTANT) {
					fprintf(fdo, "\tlxi d,%u\n", -v);
					fprintf(fdo, "\tadd %s,de\n", regnames[reg]);
				} else {
					codegen_lr(r);
					helper(n, "negate");
					fprintf(fdo, "\tex de,hl\n");
					fprintf(fdo, "\tadd %s,de\n", regnames[reg]);
				}
			} else
				reghelper(n, "bcsub");
			/* Result is only left in reg var reload if needed */
			get_regvar(reg, n, s);
			return 1;
		/* For now - we can do better - maybe just rewrite them into load,
		   op, store ? */
		case T_STAREQ:
			/* TODO: constant multiply */
			if (r->op == T_CONSTANT) {
				if (can_fast_mul(s, v)) {
					get_regvar(reg, NULL, s);
					gen_fast_mul(s, v);
					load_regvar(reg, s);
					return 1;
				}
			}
			codegen_lr(r);
			reghelper(n, "bcmul");
			return 1;
		case T_SLASHEQ:
			/* TODO: power of 2 constant divide maybe ? */
			codegen_lr(r);
			reghelper_s(n, "bcdiv");
			return 1;
		case T_PERCENTEQ:
			/* TODO: spot % 256 case */
			codegen_lr(r);
			reghelper(n, "bcrem");
			return 1;
		case T_SHLEQ:
			/* TODO IX and IY for the simple cases */
			if (r->op == T_CONSTANT && reg == 1) {
				if (s == 1 && v >= 8) {
					fprintf(fdo, "\tld c,0x0\n");
					get_regvar(reg, n, s);
					return 1;
				}
				if (s == 1) {
					fprintf(fdo, "\tld a,c\n");
					repeated_op("add a,a", v);
					fprintf(fdo, "\tld c,a\n");
					get_regvar(reg, n, s);
					return 1;
				}
				/* 16 bit */
				if (v >= 16) {
					fprintf(fdo, "\tld bc,0x0\n");
					get_regvar(reg, n, s);
					return 1;
				}
				if (v == 8) {
					fprintf(fdo, "\tld b,c\n\tld c,0x0\n");
					get_regvar(reg, n, s);
					return 1;
				}
				if (v > 8) {
					fprintf(fdo, "\tld a,c\n");
					repeated_op("add a", v - 8);
					fprintf(fdo, "\tld b,a\nld c,0x0\n");
					get_regvar(reg, n, s);
					return 1;
				}
				/* 16bit full shifting */
				get_regvar(reg, NULL, s);
				repeated_op("add hl,hl", v);
				load_regvar(reg, s);
				return 1;
			}
			codegen_lr(r);
			reghelper(n, "bcshl");
			return 1;
		case T_SHREQ:
			if (r->op == T_CONSTANT && reg == 1) {
				if (v >= 8 && s == 1) {
					fprintf(fdo, "\tld c,0x0\n");
					get_regvar(reg, n, s);
					return 1;
				}
				if (v >= 16) {
					fprintf(fdo, "\tld bc,0x0\n");
					get_regvar(reg, n, s);
					return 1;
				}
				if (v == 8 && (n->type & UNSIGNED)) {
					fprintf(fdo, "\tld c,b\nld b,0x0\n");
					get_regvar(reg, n, s);
					return 1;
				}
			}
			codegen_lr(r);
			reghelper_s(n, "bcshr");
			return 1;
		case T_ANDEQ:
			reg_logic(n, s, 0, "bcand");
			return 1;
		case T_OREQ:
			reg_logic(n, s, 1, "bcor");
			return 1;
		case T_HATEQ:
			reg_logic(n, s, 2, "bcxor");
			return 1;
		}
		fprintf(stderr, "unfixed regleft on %04X\n", n->op);
	}
	/* ?? LBSTORE TODO */
	return 0;
}

/* Stack the node which is currently in the working register */
unsigned gen_push(struct node *n)
{
	unsigned size = get_stack_size(n->type);

	/* Our push will put the object on the stack, so account for it */
	sp += size;

	/* Drop a trailing ; in to tell the optimizer HL is now dead. This is safe
	   as we currently operate but that may need review if we get clever */
	switch(size) {
	case 2:
		fprintf(fdo, "\tpush hl\n;\n");
		return 1;
	case 4:
		if (optsize)
			fprintf(fdo, "\tcall __pushl\n;\n");
		else
			fprintf(fdo, "\tex de,hl\n\tld hl,(__hireg)\n\tpush hl\n\tpush de\n;\n");
		return 1;
	default:
		return 0;
	}
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	ls = get_size(lt);

	/* Size shrink is free */
	if ((lt & ~UNSIGNED) <= (rt & ~UNSIGNED))
		return 1;
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED) || ls > 2)
		return 0;
	fprintf(fdo, "\tld h,0x0\n");
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	char *name;
	unsigned nr = n->flags & NORETURN;
	struct node *r = n->right;

	/* We adjust sp so track the pre-adjustment one too when we need it */

	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);

	switch (n->op) {
		/* Load from a name */
	case T_NREF:
		name = namestr(n->snum);
		if (size == 1)
			fprintf(fdo, "\tld a,(_%s+%u)\n\tld l,a\n", name, v);
		else {
			if (size == 4) {
				fprintf(fdo, "\tld hl,(_%s+%u)\n"
				       "\tld (__hireg),hl\n", name, v + 2);
			}
			fprintf(fdo, "\tld hl,(_%s+%u)\n", name, v);
		}
		return 1;
	case T_LBREF:
		if (size == 1)
			fprintf(fdo, "\tld a,(T%u+%u)\n\tld l,a\n", n->val2, v);
		else {
			if (size == 4) {
				fprintf(fdo, "\tld hl, (T%u+%u)\n"
				       "\tld (__hireg),hl\n", n->val2, v + 2);
			}
			fprintf(fdo, "\tld hl,(T%u+%u)\n", n->val2, v);
		}
		return 1;
	case T_LREF:
		/* We are loading something then not using it, and it's local
		   so can go away */
		if (nr)
			return 1;
		if (generate_lref(v, size, 0))
			return 1;
		error("lrb");
		return 0;
	case T_RREF:
		if (nr)
			return 1;
		if (n->value == 1) {
			fprintf(fdo, "\tld l,c\n");
			if (size == 2)
				fprintf(fdo, "\tld h,b\n");
		} else {
			fprintf(fdo, "\tpush %s\n\tpop hl\n", regnames[n->value]);
		}
 		return 1;
	case T_NSTORE:
		name = namestr(n->snum);
		if (size == 1) {
			fprintf(fdo, "\tld a,l\n"
			       "\tld (_%s+%u),", name, v);
			return 1;
		}
		fprintf(fdo, "\tld (%s+%u), hl\n", name, v);
		if (size == 4)
			fprintf(fdo, "\tld de,(__hireg)\nld (%s+%u),de\n",
				name, v + 2);
		return 1;
	case T_LBSTORE:
		if (size == 1) {
			fprintf(fdo, "\tld a,l\n\tld (T%u+%u),a\n", n->val2, v);
			return 1;
		}
		fprintf(fdo, "\tld (T%u+%u),hl\n", n->val2, v);
		if (size == 4)
			fprintf(fdo, "\tld de,(__hireg)\n\tld (T%u+%u),de\n",
				n->val2, v + 2);
		return 1;
	case T_LSTORE:
/*		fprintf(fdo, ";L sp %u spval %u %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		v += sp;
		if (HAS_LDHLSP && v <= 255) {
			if (size == 2) {
				fprintf(fdo, "\tld (sp+%u),hl\n", v);
				return 1;
			}
			if (HAS_LDASP && size == 1) {
				fprintf(fdo, "\tld a,l\n\tld (sp + %u),a d\n", v);
				return 1;
			}
		}
		if (v == 0 && size == 2) {
			if (nr)
				fprintf(fdo, "\tex (sp).hl\n");
			else
				fprintf(fdo, "\tpop af\n\tpush hl\n");
			return 1;
		}
		/* Rabbit can LD HL,(HL + 0) so we can construct a load fairly ok */
		if (IS_RABBIT && size == 2) {
			fprintf(fdo, "\tld hl,0x%x\n\tadd hl,sp\n\tld hl,(hl + 0)\n", v);
			return 1;
		}
		if (size == 1 && (!optsize || v >= LWDIRECT)) {
			fprintf(fdo, "\tld a,l\n\tld hl,%u\n\tadd hl,sp\n\tld (hl),a\n", WORD(v));
			if (!(n->flags & NORETURN))
				fprintf(fdo, "\tld l,a\n");
			return 1;
		}
		/* For -O3 they asked for it so inline the lot */
		/* We dealt with size one above */
		if (opt > 2 && size == 2) {
			fprintf(fdo, "\tex de,hl\n\tld hl,0x%x\n\tadd hl,sp\n"
			        "\tld (hl),e\n\tinc hl\n\tld (hl),d\n"
					, WORD(v));
			if (!nr)
				fprintf(fdo, "\tex de,hl\n");
			return 1;
		}
		/* Via helper magic for compactness on Z80 */
		/* Can rewrite some of them into rst if need be */
		if (size == 1)
			name = "stbyte";
		else
			name = "stword";
		/* Like load the helper is offset by two because of the
		   stack */
		if (v < 24)
			fprintf(fdo, "\tcall __%s%u\n", name, v + 2);
		else if (v < 253)
			fprintf(fdo, "\tcall __%s\n\t.byte %u\n", name, v + 2);
		else
			fprintf(fdo, "\tcall __%sw\n\t.word %u\n", name, v + 2);
		return 1;
	case T_RSTORE:
		load_regvar(n->value, size);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		if (cpufeat & 1)
			fprintf(fdo, "\tpush af\n");
		fprintf(fdo, "\tcall _%s+%u\n", namestr(n->snum), v);
		if (cpufeat & 1)
			fprintf(fdo, "\tpop af\n");
		return 1;
	case T_EQ:
		if (size == 2) {
            fprintf(fdo, "\tex de,hl\n\tpop hl\n\tld (hl),e\n\tinc hl\n\tld (hl),d\n");
			if (!nr)
				fprintf(fdo, "\tex de,hl\n");
			return 1;
		}
		if (size == 1) {
			fprintf(fdo, "\tpop de\n\tex de,hl\n\tld (hl),e\n");
			if (!nr)
				fprintf(fdo, "\tex de,hl\n");
			return 1;
		}
		break;
	case T_RDEREF:
		switch(n->value) {
		case 1:
			/* BC is never anything but a byte sized ptr */
			fprintf(fdo, "\tld a,(bc)\n");	/* NORETURN but generated imples volatile so keep the ldax */
			if (!nr)
				fprintf(fdo, "\tld l,a\n");
			return 1;
		case 2:
			if (size == 4) {
				fprintf(fdo, "\tld h,(ix + %u)\n", n->val2 + 3);
				fprintf(fdo, "\tld l,(ix + %u)\n", n->val2 + 2);
				fprintf(fdo, "\tld (__hireg),hl\n");
			}
			if (size > 1)
				fprintf(fdo, "\tld h,(ix + %u)\n", n->val2 + 1);
			fprintf(fdo, "\tld l,(ix + %u)\n", n->val2);
			return 1;
		case 3:
			if (size == 4) {
				fprintf(fdo, "\tld h,(iy + %u)\n", n->val2 + 3);
				fprintf(fdo, "\tld l,(iy + %u)\n", n->val2 + 2);
				fprintf(fdo, "\tld (__hireg),hl\n");
			}
			if (size > 1)
				fprintf(fdo, "\tld h,(iy + %u)\n", n->val2 + 1);
			fprintf(fdo, "\tld l,(iy + %u)\n", n->val2);
			return 1;
		}
		return 0;
	case T_DEREF:
		if (size == 2) {
			if (HAS_LDHLHL)
				fprintf(fdo, "\tld hl,(hl)\n");
			else
				fprintf(fdo, "\tld e,(hl)\n\tinc hl\n\tld d,(hl)\n\tex de,hl\n");
			return 1;
		}
		if (size == 1) {
			fprintf(fdo, "\tld l,(hl)\n");
			return 1;
		}
		break;
	case T_FUNCCALL:
		/* Banking has no other effect as indirectly referenced calls go via the stub
		   table so the function has a valid 16bit "address". callhl must live in common */
		fprintf(fdo, "\tcall __callhl\n");
		return 1;
	case T_LABEL:
		/* Used for const strings and local static */
		fprintf(fdo, "\tld hl,T%u+%u\n", n->val2, v);
		return 1;
	case T_CONSTANT:
		switch(size) {
		case 4:
		    //fprintf(fdo, ";here: %x %x %x %x\n", *(unsigned char*)&v, *((unsigned char*)&v + 1), *((unsigned char*)&v + 2), *((unsigned char*)&v + 3));
			fprintf(fdo, "\tld hl,0x%x\n", (unsigned short)((n->value >> 16)));
			fprintf(fdo, "\tld (__hireg),hl\n");
		case 2:
			fprintf(fdo, "\tld hl,0x%x\n", (unsigned short)(n->value));
			return 1;
		case 1:
			fprintf(fdo, "\tld l,0x%x\n", (v & 0xFF));
			return 1;
		}
		break;
	case T_NAME:
		fprintf(fdo, "\tld hl,");
		fprintf(fdo, "_%s+%u\n", namestr(n->snum), v);
		return 1;
	case T_ARGUMENT:
		v += frame_len + argbase;
		/* Fall through */
	case T_LOCAL:
		v += sp;
		fprintf(fdo, "\tld hl,0x%x\n", v);
		fprintf(fdo, "\tadd hl,sp\n");
		return 1;
	case T_REG:
		if (nr)
			return 1;
		/* A register has no address.. we need to sort this out */
		error("rega");
		return 1;
	case T_CAST:
		/* TODO:Z280 has EXTS */
		return gen_cast(n);
	case T_BOOL:
		/* Rabbit has an boolify operator */
		if (IS_RABBIT) {
			fprintf(fdo, "\tbool hl\n");
			return 1;
		}
		break;
	case T_PLUS:
		if (size <= 2) {
			fprintf(fdo, "\tpop de\n\tadd hl,de\n");
			return 1;
		}
		break;
	case T_MINUS:
		if (size <= 2) {
			fprintf(fdo, "\tpop de\n\tor a\n\tex de,hl\n\tsbc hl,de\n");
			return 1;
		}
		break;
	case T_BANG:
		/* Logic via jumping always uses CC in our case */
		if (r->op == T_ANDAND || r->op == T_OROR)
			r->flags |= USECC;
		/* Always use the helper if we need the actual value */
		if (!(n->flags & CCONLY)) {
			if (r->flags & USECC) {
				/* Should never happen */
				fprintf(fdo, ";BOTCH %04x:%04X\n", r->op, r->flags);
				fprintf(fdo, "\tcall __cctonot\n");
				return 1;
			}
			return 0;
		}
		/* Already CC format */
		if (r->flags & USECC) {
			if (!(n->flags & CCFIXED)) {
				/* Will need work when we add > >= and friends TODO */
				if (ccflags == ccinvert)
					ccflags = ccnormal;
				else if (ccflags == ccnormal)
					ccflags = ccinvert;
				else
					error("ccf");
				n->flags |= USECC;
				return 1;
			}
			/*  We have a CC we want to invert it but we are not
			    allowed to */
			fprintf(fdo, "\tcall __cctonot\n");
			n->flags |= ISBOOL;
			return 1;
		}
		if (size == 4)
			return 0;
		if (!(n->flags & CCFIXED)) {
			/* Turn into CC format */
			if (size == 1)
				fprintf(fdo, "\tld a,l\n\tor a\n");
			else if (size == 2)
				fprintf(fdo, "\tld a,h\n\tor l\n");
			ccflags = ccinvert;
			n->flags |= USECC;
			return 1;
		}
		return 0;
	case T_BTST:
		/* Always CCONLY */
		if (v < 8)
			fprintf(fdo, "\tbit %u, l\n", v);
		else
			fprintf(fdo, "\tbit %u, h\n", v - 8);
		n->flags |= USECC;
		return 1;
	case T_BYTEEQ:
		v = n->value;
		/* Value is in L, just compare byte if we can */
		fprintf(fdo, "\tld a,0x%x\n", v & 0xFF);
		if ((n->flags & CCONLY) && !(n->flags & CCFIXED)) {
			fprintf(fdo, "\tcp l\n");
			ccflags = ccinvert;
			n->flags |= USECC;
			return 1;
		}
		/* Couldn't short cut so do it direct */
		fprintf(fdo, "\tcall __cmpeqla\n");
		n->flags |= ISBOOL;
		return 1;
	case T_BYTENE:
		v = n->value;
		/* Value is in L, just compare byte if we can */
		fprintf(fdo, "\tld a,0x%x\n", v & 0xFF);
		if (n->flags & CCONLY) {
			fprintf(fdo, "\tcp l\n");
			n->flags |= USECC;
			return 1;
		}
		fprintf(fdo, "\tcall __cmpnela\n");
		n->flags |= ISBOOL;
		return 1;
	}
	return 0;
}
