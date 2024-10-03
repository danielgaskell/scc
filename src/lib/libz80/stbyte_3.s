
	.export __stbyte3
	.code
__stbyte3:
	ld a,l
	ld hl,3
	add hl,sp
	ld (hl),a
	ld l,a
	ret
