
	.export __ldword31
	.code
__ldword31:
	ld hl,31
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
