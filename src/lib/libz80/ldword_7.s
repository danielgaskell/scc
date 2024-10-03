
	.export __ldword7
	.code
__ldword7:
	ld hl,7
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
