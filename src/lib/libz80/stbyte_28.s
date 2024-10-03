
	.export __stbyte28
	.code
__stbyte28:
	ld a,l
	ld hl,28
	add hl,sp
	ld (hl),a
	ld l,a
	ret
