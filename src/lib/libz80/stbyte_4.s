
	.export __stbyte4
	.code
__stbyte4:
	ld a,l
	ld hl,4
	add hl,sp
	ld (hl),a
	ld l,a
	ret
