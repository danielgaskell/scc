
	.export __ldword23
	.code
__ldword23:
	ld hl,23
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
