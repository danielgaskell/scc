
	.export __stbyte31
	.code
__stbyte31:
	ld a,l
	ld hl,31
	add hl,sp
	ld (hl),a
	ld l,a
	ret
