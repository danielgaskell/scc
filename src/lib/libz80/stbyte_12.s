
	.export __stbyte12
	.code
__stbyte12:
	ld a,l
	ld hl,12
	add hl,sp
	ld (hl),a
	ld l,a
	ret
