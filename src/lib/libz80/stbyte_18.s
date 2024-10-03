
	.export __stbyte18
	.code
__stbyte18:
	ld a,l
	ld hl,18
	add hl,sp
	ld (hl),a
	ld l,a
	ret
