
	.export __stbyte24
	.code
__stbyte24:
	ld a,l
	ld hl,24
	add hl,sp
	ld (hl),a
	ld l,a
	ret
