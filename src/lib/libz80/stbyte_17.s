
	.export __stbyte17
	.code
__stbyte17:
	ld a,l
	ld hl,17
	add hl,sp
	ld (hl),a
	ld l,a
	ret
