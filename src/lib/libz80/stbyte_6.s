
	.export __stbyte6
	.code
__stbyte6:
	ld a,l
	ld hl,6
	add hl,sp
	ld (hl),a
	ld l,a
	ret
