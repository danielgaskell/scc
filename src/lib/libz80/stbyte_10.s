
	.export __stbyte10
	.code
__stbyte10:
	ld a,l
	ld hl,10
	add hl,sp
	ld (hl),a
	ld l,a
	ret
