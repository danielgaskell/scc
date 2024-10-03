
	.export __stbyte13
	.code
__stbyte13:
	ld a,l
	ld hl,13
	add hl,sp
	ld (hl),a
	ld l,a
	ret
