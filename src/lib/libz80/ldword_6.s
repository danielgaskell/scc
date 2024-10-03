
	.export __ldword6
	.code
__ldword6:
	ld hl,6
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
