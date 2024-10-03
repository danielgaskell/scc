
	.export __ldword16
	.code
__ldword16:
	ld hl,16
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
