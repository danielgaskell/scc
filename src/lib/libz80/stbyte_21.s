
	.export __stbyte21
	.code
__stbyte21:
	ld a,l
	ld hl,21
	add hl,sp
	ld (hl),a
	ld l,a
	ret
