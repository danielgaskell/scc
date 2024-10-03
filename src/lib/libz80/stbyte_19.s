
	.export __stbyte19
	.code
__stbyte19:
	ld a,l
	ld hl,19
	add hl,sp
	ld (hl),a
	ld l,a
	ret
