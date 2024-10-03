
	.export __stbyte20
	.code
__stbyte20:
	ld a,l
	ld hl,20
	add hl,sp
	ld (hl),a
	ld l,a
	ret
