
	.export __stbyte29
	.code
__stbyte29:
	ld a,l
	ld hl,29
	add hl,sp
	ld (hl),a
	ld l,a
	ret
