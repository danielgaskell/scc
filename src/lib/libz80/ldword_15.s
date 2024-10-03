
	.export __ldword15
	.code
__ldword15:
	ld hl,15
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
