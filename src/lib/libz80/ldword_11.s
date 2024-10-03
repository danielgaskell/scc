
	.export __ldword11
	.code
__ldword11:
	ld hl,11
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
