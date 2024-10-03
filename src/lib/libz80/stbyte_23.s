
	.export __stbyte23
	.code
__stbyte23:
	ld a,l
	ld hl,23
	add hl,sp
	ld (hl),a
	ld l,a
	ret
