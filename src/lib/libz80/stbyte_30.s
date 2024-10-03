
	.export __stbyte30
	.code
__stbyte30:
	ld a,l
	ld hl,30
	add hl,sp
	ld (hl),a
	ld l,a
	ret
