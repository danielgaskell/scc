
	.export __ldword24
	.code
__ldword24:
	ld hl,24
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
