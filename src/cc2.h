/* Until we do more testing */
#define IS_EZ80		0	/* EZ80 has ld de,(hl) and friends but not offset */
#define IS_RABBIT	0	/* Has ld hl,(rr + n) and vice versa but only 16bit */
#define HAS_LDHLSP	0	/* Can ld hl,(sp + n) and vice versa */
#define HAS_LDASP	0	/* Can ld a,(sp + n) and vice versa */
#define HAS_LDHLHL	0	/* Can ld hl,(hl) or hl,(hl + 0) */

#define ARGBASE	2	/* Bytes between arguments and locals if no reg saves */

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)
/*
 *	Upper node flag fields are ours
 */

#define USECC	0x0100

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)
#define T_RREF		(T_USER+7)
#define T_RSTORE	(T_USER+8)
#define T_RDEREF	(T_USER+9)		/* *regptr */
#define T_REQ		(T_USER+10)		/* *regptr */
#define T_BTST		(T_USER+11)		/* Use bit n, for and bit conditionals */
#define T_BYTEEQ	(T_USER+12)		/* Until we handle 8bit better */
#define T_BYTENE	(T_USER+13)


extern unsigned get_size(unsigned t);
extern unsigned get_stack_size(unsigned t);
extern int bitcheckb1(uint8_t n);
extern int bitcheck1(unsigned n, unsigned s);
extern int bitcheck0(unsigned n, unsigned s);
extern void gen_cleanup(unsigned v);

extern unsigned frame_len;	/* Number of bytes of stack frame */
extern unsigned sp;		/* Stack pointer offset tracking */
extern unsigned argbase;	/* Argument offset in current function */
extern unsigned unreachable;	/* Code after an unconditional jump */
extern unsigned func_cleanup;	/* Zero if we can just ret out */
extern unsigned use_fp;		/* Using a frame pointer this function */

extern const char ccnormal[];
extern const char ccinvert[];

extern const char *ccflags;	/* True, False flags */

extern unsigned cpu;
extern unsigned opt;
extern unsigned optsize;
extern const char *codeseg;
extern unsigned long cpufeat;

extern void error(const char *p);

extern char *namestr(unsigned n);
extern struct node *new_node(void);
extern void free_node(struct node *n);
extern void init_nodes(void);

extern void helper(struct node *n, const char *h);
extern void helper_s(struct node *n, const char *h);
extern void helper_type(unsigned t, unsigned s);
extern void codegen_lr(struct node *n);

extern struct node *gen_rewrite_node(struct node *n);
extern struct node *gen_rewrite(struct node *n);

extern void gen_segment(unsigned segment);
extern void gen_export(const char *name);
extern void gen_prologue(const char *name);
extern void gen_frame(unsigned size, unsigned asize);
extern void gen_epilogue(unsigned size, unsigned argsize);
extern unsigned gen_exit(const char *t, unsigned n);
extern void gen_label(const char *t, unsigned n);
extern void gen_jump(const char *t, unsigned n);
extern void gen_jfalse(const char *t, unsigned n);
extern void gen_jtrue(const char *t, unsigned n);

extern void gen_switch(unsigned n, unsigned type);
extern void gen_switchdata(unsigned n, unsigned size);
extern void gen_case(unsigned tag, unsigned entry);
extern void gen_case_data(unsigned tag, unsigned entry);
extern void gen_case_label(unsigned tag, unsigned entry);

extern void gen_data_label(const char *t, unsigned align);

extern void gen_space(unsigned value);
extern void gen_text_data(struct node *n);
extern void gen_value(unsigned type, unsigned long value);
extern void gen_name(struct node *n);
extern void gen_literal(unsigned value);

extern void gen_helpcall(struct node *n);
extern void gen_helptail(struct node *n);
extern void gen_helpclean(struct node *n);

extern void gen_start(void);
extern void gen_end(void);

extern void gen_tree(struct node *n);

/* Provide if using codgen_lr */
extern unsigned gen_push(struct node *n);
extern unsigned gen_node(struct node *n);
extern unsigned gen_direct(struct node *n);
extern unsigned gen_uni_direct(struct node *n);
extern unsigned gen_shortcut(struct node *n);

/* Build a node */
extern void make_node(struct node *n);

#define A_CODE		1
#define A_DATA		2
#define A_BSS		3
#define A_LITERAL	4
#define A_SYMDATA   5
#define A_SYMTRANS  6

#define MAX_SEG		3

extern unsigned func_flags;
