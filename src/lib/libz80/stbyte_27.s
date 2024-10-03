
	.export __stbyte27
	.code
__stbyte27:
	ld a,l
	ld hl,27
	add hl,sp
	ld (hl),a
	ld l,a
	ret
