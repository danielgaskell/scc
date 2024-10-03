
	.export __ldword8
	.code
__ldword8:
	ld hl,8
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
