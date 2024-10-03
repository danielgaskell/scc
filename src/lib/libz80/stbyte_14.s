
	.export __stbyte14
	.code
__stbyte14:
	ld a,l
	ld hl,14
	add hl,sp
	ld (hl),a
	ld l,a
	ret
