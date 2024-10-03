
	.export __ldword17
	.code
__ldword17:
	ld hl,17
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
