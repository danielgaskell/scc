
	.export __stbyte9
	.code
__stbyte9:
	ld a,l
	ld hl,9
	add hl,sp
	ld (hl),a
	ld l,a
	ret
