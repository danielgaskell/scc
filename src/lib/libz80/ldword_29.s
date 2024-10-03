
	.export __ldword29
	.code
__ldword29:
	ld hl,29
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
