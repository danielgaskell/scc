
	.export __ldword9
	.code
__ldword9:
	ld hl,9
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
