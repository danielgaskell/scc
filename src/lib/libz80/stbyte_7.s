
	.export __stbyte7
	.code
__stbyte7:
	ld a,l
	ld hl,7
	add hl,sp
	ld (hl),a
	ld l,a
	ret
