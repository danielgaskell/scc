
	.export __ldword28
	.code
__ldword28:
	ld hl,28
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
