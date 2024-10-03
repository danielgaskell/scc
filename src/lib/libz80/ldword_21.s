
	.export __ldword21
	.code
__ldword21:
	ld hl,21
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
