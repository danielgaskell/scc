
	.export __ldword4
	.code
__ldword4:
	ld hl,4
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
