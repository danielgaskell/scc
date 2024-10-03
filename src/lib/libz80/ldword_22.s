
	.export __ldword22
	.code
__ldword22:
	ld hl,22
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
