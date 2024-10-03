
	.export __stbyte5
	.code
__stbyte5:
	ld a,l
	ld hl,5
	add hl,sp
	ld (hl),a
	ld l,a
	ret
