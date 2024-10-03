
	.export __stbyte8
	.code
__stbyte8:
	ld a,l
	ld hl,8
	add hl,sp
	ld (hl),a
	ld l,a
	ret
