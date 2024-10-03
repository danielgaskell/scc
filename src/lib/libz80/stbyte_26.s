
	.export __stbyte26
	.code
__stbyte26:
	ld a,l
	ld hl,26
	add hl,sp
	ld (hl),a
	ld l,a
	ret
