
	.export __stbyte11
	.code
__stbyte11:
	ld a,l
	ld hl,11
	add hl,sp
	ld (hl),a
	ld l,a
	ret
