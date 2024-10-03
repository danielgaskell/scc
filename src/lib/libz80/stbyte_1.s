
	.export __stbyte1
	.code
__stbyte1:
	ld a,l
	ld hl,1
	add hl,sp
	ld (hl),a
	ld l,a
	ret
