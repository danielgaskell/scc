/*
 *	General backend helpers for Z80 except for code generator
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/* Export the C symbol */
void gen_export(const char *name)
{
	fprintf(fdo, "	.export _%s\n", name);
}

void gen_segment(unsigned segment)
{
	switch(segment) {
	case A_CODE:
		fprintf(fdo, "\t.%s\n", codeseg);
		break;
	case A_DATA:
		fprintf(fdo, "\t.data\n");
		break;
	case A_BSS:
		fprintf(fdo, "\t.bss\n");
		break;
	case A_LITERAL:
		fprintf(fdo, "\t.literal\n");
		break;
	case A_SYMDATA:
		fprintf(fdo, "\t.symdata\n");
		break;
	case A_SYMTRANS:
		fprintf(fdo, "\t.symtrans\n");
		break;
	default:
		error("gseg");
	}
}

/* Generate the function prologue - may want to defer this until
   gen_frame for the most part */
void gen_prologue(const char *name)
{
	fprintf(fdo, "_%s:\n", name);
	unreachable = 0;
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size,  unsigned aframe)
{
	frame_len = size;
	sp = 0;
	use_fp = 0;

	if (size || (func_flags & (F_REG(1)|F_REG(2)|F_REG(3))))
		func_cleanup = 1;
	else
		func_cleanup = 0;

	argbase = ARGBASE;

	/* In banked mode the arguments are two bytes further out */
	if (cpufeat & 1)
		argbase += 2;

	if (func_flags & F_REG(1)) {
		fprintf(fdo, "\tpush bc\n");
		argbase += 2;
	}
	if (func_flags & F_REG(2)) {
		fprintf(fdo, "\tpush ix\n");
		argbase += 2;
	}
	if (func_flags & F_REG(3)) {
		fprintf(fdo, "\tpush iy\n");
		argbase += 2;
	} else {
		/* IY is free use it as a frame pointer ? */
		if (!optsize && size > 4) {
			argbase += 2;
			fprintf(fdo, "\tpush iy\n");
			/* Remember we need to restore IY */
			func_flags |= F_REG(3);
			use_fp = 1;
		}
	}
	/* If we are building a frame pointer we need to do this work anyway */
	if (use_fp) {
		fprintf(fdo, "\tld iy,0x%x\n", (uint16_t) - size);
		fprintf(fdo, "\tadd iy,sp\n");
		fprintf(fdo, "\tld sp,iy\n");
		return;
	}
	if (size > 10) {
		fprintf(fdo, "\tld hl,0x%x\n", (uint16_t) -size);
		fprintf(fdo, "\tadd hl,sp\n");
		fprintf(fdo, "\tld sp,hl\n");
		return;
	}
	if (size & 1) {
		fprintf(fdo, "\tdec sp\n");
		size--;
	}
	while(size) {
		fprintf(fdo, "\tpush hl\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != 0)
		error("sp");

	/* Return in HL, does need care on stack. TOOD: flag void functions
	   where we can burn the return */
	sp -= size;

	/* This can happen if the function never returns or the only return
	   is a by a ret directly (ie from a function without locals) */
	if (unreachable)
		return;

	if (size > 10) {
		unsigned x = func_flags & F_VOIDRET;
		if (!x)
			fprintf(fdo, "\tex de,hl\n");
		fprintf(fdo, "\tld hl,0x%x\n", (uint16_t)size);
		fprintf(fdo, "\tadd hl,sp\n");
		fprintf(fdo, "\tld sp,hl\n");
		if (!x)
			fprintf(fdo, "\tex de,hl\n");
	} else {
		if (size & 1) {
			fprintf(fdo, "\tinc sp\n");
			size--;
		}
		while (size) {
			fprintf(fdo, "\tpop de\n");
			size -= 2;
		}
	}
	if (func_flags & F_REG(3))
		fprintf(fdo, "\tpop iy\n");
	if (func_flags & F_REG(2))
		fprintf(fdo, "\tpop ix\n");
	if (func_flags & F_REG(1))
		fprintf(fdo, "\tpop bc\n");
	fprintf(fdo, "\tret\n");
	unreachable = 1;
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	fprintf(fdo, "L%u%s:\n", n, tail);
}

/* A return statement. We can sometimes shortcut this if we have
   no cleanup to do */
unsigned gen_exit(const char *tail, unsigned n)
{
	if (func_cleanup) {
		gen_jump(tail, n);
		return 0;
	} else {
		fprintf(fdo, "\tret\n");
		unreachable = 1;
		return 1;
	}
}

void gen_jump(const char *tail, unsigned n)
{
	fprintf(fdo, "\tjr L%u%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	fprintf(fdo, "\tjr %s,L%u%s\n", ccflags + 2, n, tail);
	ccflags = ccnormal;
}

void gen_jtrue(const char *tail, unsigned n)
{
	fprintf(fdo, "\tjr %c%c,L%u%s\n", ccflags[0], ccflags[1], n, tail);
	ccflags = ccnormal;
}

void gen_cleanup(unsigned v)
{
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	if (v > 10) {
		/* This is more expensive, but we don't often pass that many
		   arguments so it seems a win to stay in HL */
		/* TODO: spot void function and skip ex de,hl */
		fprintf(fdo, "\tex de,hl\n");
		fprintf(fdo, "\tld hl,0x%x\n", v);
		fprintf(fdo, "\tadd hl,sp\n");
		fprintf(fdo, "\tld sp,hl\n");
		fprintf(fdo, "\tex de,hl\n");
	} else {
		while(v >= 2) {
			fprintf(fdo, "\tpop de\n");
			v -= 2;
		}
		if (v)
			fprintf(fdo, "\tinc sp\n");
	}
}

/*
 *	Helper handlers. We use a tight format for integers but C
 *	style for float as we'll have C coded float support if any
 */

/* True if the helper is to be called C style */
static unsigned c_style(struct node *n)
{
	/* Assignment is done asm style */
	if (n->op == T_EQ)
		return 0;
	/* Float ops otherwise are C style */
	if (n->type == FLOAT)
		return 1;
	n = n->right;
	if (n && n->type == FLOAT)
		return 1;
	return 0;
}

void gen_helpcall(struct node *n)
{
	/* Check both N and right because we handle casts to/from float in
	   C call format */
	if (c_style(n))
		gen_push(n->right);
	fprintf(fdo, "\tcall __");
}

void gen_helptail(struct node *n)
{
}

void gen_helpclean(struct node *n)
{
	unsigned s;

	if (c_style(n)) {
		s = 0;
		if (n->left) {
			s += get_size(n->left->type);
			/* gen_node already accounted for removing this thinking
			   the helper did the work, adjust it back as we didn't */
			sp += s;
			}
		s += get_size(n->right->type);
		gen_cleanup(s);
		/* C style ops that are ISBOOL didn't set the bool flags */
		if (n->flags & ISBOOL)
			fprintf(fdo, "\txor a\n\tcp l\n");
	}
}

void gen_switch(unsigned n, unsigned type)
{
	fprintf(fdo, "\tld de,Sw%u\n", n);
	fprintf(fdo, "\tjp __switch");
	helper_type(type, 0);
	fprintf(fdo, "\n");
	unreachable = 1;
}

void gen_switchdata(unsigned n, unsigned size)
{
	fprintf(fdo, "Sw%u:\n", n);
	fprintf(fdo, "\t.word %u\n", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	fprintf(fdo, "Sw%u_%u:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	fprintf(fdo, "\t.word Sw%u_%u\n", tag, entry);
}

void gen_data_label(const char *name, unsigned align)
{
	fprintf(fdo, "_%s:\n", name);
}

void gen_space(unsigned value)
{
	fprintf(fdo, "\t.ds %u\n", value);
}

void gen_text_data(struct node *n)
{
	fprintf(fdo, "\t.word T%u\n", n->val2);
}

/* The label for a literal (currently only strings)
   TODO: if we add other literals we may need alignment here */

void gen_literal(unsigned n)
{
	if (n)
		fprintf(fdo, "T%u:\n", n);
}

void gen_name(struct node *n)
{
	fprintf(fdo, "\t.word _%s+%u\n", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	unsigned w = WORD(value);
	if (PTR(type)) {
		fprintf(fdo, "\t.word %u\n", w);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		fprintf(fdo, "\t.byte %u\n", BYTE(w));
		break;
	case CSHORT:
	case USHORT:
		fprintf(fdo, "\t.word %u\n", w);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		fprintf(fdo, "\t.word %u\n", w);
		fprintf(fdo, "\t.word %u\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	/* For now.. */
	fprintf(fdo, "\t.z80\n");
}

void gen_end(void)
{
}

