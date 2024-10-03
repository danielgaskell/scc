
	.export __stbyte25
	.code
__stbyte25:
	ld a,l
	ld hl,25
	add hl,sp
	ld (hl),a
	ld l,a
	ret
