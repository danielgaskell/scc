
	.export __stbyte22
	.code
__stbyte22:
	ld a,l
	ld hl,22
	add hl,sp
	ld (hl),a
	ld l,a
	ret
