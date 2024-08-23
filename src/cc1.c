/*
 *	Compiler pass main loop
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

#include "compiler.h"

int fdi = -1;
int fdo = -1;

#include "body.c"
#include "declaration.c"
#include "enum.c"
#include "error.c"
#include "expression.c"
#include "header.c"
#include "idxdata.c"
#include "initializer.c"
#include "label.c"
#include "lex.c"
#include "primary.c"
#include "storage.c"
#include "stackframe.c"
#include "struct.c"
#include "switch.c"
#include "symbol.c"
#include "target.c"
#include "token.h"
#include "tree.c"
#include "type.c"
#include "type_iterator.c"

FILE *debug;

unsigned deffunctype;		/* The type of an undeclared function */
unsigned funcbody;		/* Parser global for function body */
unsigned voltrack;		/* Track possible volatiles */
unsigned in_sizeof;		/* Set if we are in sizeof() */
unsigned cputype;		/* So the target specific code can make decisions */
unsigned long cpufeat;		/* CPU feature flags from user for target specific code */

/*
 *	A C program consists of a series of declarations that by default
 *	are external definitions.
 */
static void toplevel(void)
{
	if (token == T_TYPEDEF) {
		next_token();
		dotypedef();
	} else {
		funcbody = 0;
		voltrack = 0;
		target_reginit();
		declaration(S_EXTDEF);
	}
}

/* A function defined by use is taken to be int f(); */
static unsigned functype[2] = {
	1, ELLIPSIS
};

int main(int argc, char *argv[])
{
	if (argc > 2) {
		error("usage: cc1 [cpufeatures]");
		exit(1);
	}
	cputype = 80;
	if (argv[1] != NULL)
        cpufeat = atol(argv[1]);
    else
        cpufeat = 0;
    fdi = open("$stream1", O_RDONLY | O_BINARY, 0600);
    if (fdi == -1)
        fatal("error opening input stream");
    fdo = open("$stream2", O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0600);
    if (fdo == -1)
        fatal("error opening output stream");

	next_token();
	init_nodes();
	/* A function with no type info returning INT */
	deffunctype = make_function(CINT, functype);
#ifdef DEBUG
	if (argv[1]) {
		debug = fopen(argv[1], "w");
		if (debug == NULL) {
			perror(argv[1]);
			return 255;
		}
	}
#endif
	while (token != T_EOF)
		toplevel();
	/* No write out any uninitialized variables */
	write_bss();
	out_write();
	close_files();
	return errors;
}
