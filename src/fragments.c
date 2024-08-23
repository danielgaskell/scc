/*
 *	Write out the target file including any necessary headers if we are
 *	writing out a relocatable object. At this point we don't have any
 *	support for writing out a nice loader friendly format, and that
 *	may want to be a separate tool that consumes a resolved relocatable
 *	binary and generates a separate relocation block in the start of
 *	BSS.
 */
static void write_binary(FILE * op, FILE *mp)
{
	static struct objhdr hdr;
	register uint_fast8_t i;

	hdr.o_arch = arch;
	hdr.o_cpuflags = arch_flags;
	hdr.o_flags = obj_flags;
	hdr.o_segbase[0] = sizeof(hdr);
	hdr.o_size[0] = size[0];
	hdr.o_size[1] = size[1];
	hdr.o_size[2] = size[2];
	hdr.o_size[3] = size[3];
	hdr.o_size[7] = size[7];

	rewind(op);

	if (verbose)
		printf("Writing binary\n");
	if (!rawstream)
		fwrite(&hdr, sizeof(hdr), 1, op);
	/* For LD_RFLAG number the symbols for output, for othe forms
	   check for unknowmn symbols and error them out */
	if (!rawstream)
		renumber_symbols();
	if ((ldmode == LD_FUZIX || ldmode == LD_RFLAG) && size[0]) {
		fprintf(stderr, "Cannot build a SymbOS or relocatable binary including absolute segments.\n");
		exit(1);
	}
	if (ldmode != LD_FUZIX)
		write_stream(op, ABSOLUTE);
	write_stream(op, CODE);
	if (ldmode == LD_FUZIX)
		write_stream(op, LITERAL);
	hdr.o_segbase[1] = ftell(op);
	write_stream(op, DATA);
	/* Absolute images may contain things other than code/data/bss */
	if (ldmode == LD_ABSOLUTE) {
		for (i = 4; i < OSEG; i++) {
			if (target_has_regzp()) {
				if (i == ZP)
					continue;
			}
			write_stream(op, i);
		}
	}
	else {
		/* ZP is ok in Fuzix but is not initialized in a defined way */
		for (i = ldmode == LD_FUZIX ? 5 : 4; i < OSEG; i++) {
			if (i != LITERAL && size[i]) {
				fprintf(stderr, "Unsupported data in non-standard segment %d.\n", i);
				break;
			}
		}
		if (!rawstream && !strip) {
			hdr.o_symbase = ftell(op);
			write_symbols(op);
		}
	}
	hdr.o_dbgbase = ftell(op);
	hdr.o_magic = MAGIC_OBJ;
	/* TODO: needs a special pass
	if (dbgsyms )
		copy_debug_all(op, mp);*/
	if (err == 0) {
		if (!rawstream) {
			xfseek(op, 0);
			fwrite(&hdr, sizeof(hdr), 1, op);
		}
		//} else	/* FIXME: honour umask! */
		//	fchmod (fileno(op), 0755);
	}
}