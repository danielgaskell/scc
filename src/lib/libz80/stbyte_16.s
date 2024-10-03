
	.export __stbyte16
	.code
__stbyte16:
	ld a,l
	ld hl,16
	add hl,sp
	ld (hl),a
	ld l,a
	ret
