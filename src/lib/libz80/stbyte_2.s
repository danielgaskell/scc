
	.export __stbyte2
	.code
__stbyte2:
	ld a,l
	ld hl,2
	add hl,sp
	ld (hl),a
	ld l,a
	ret
