
	.export __stbyte15
	.code
__stbyte15:
	ld a,l
	ld hl,15
	add hl,sp
	ld (hl),a
	ld l,a
	ret
