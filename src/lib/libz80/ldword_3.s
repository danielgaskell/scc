
	.export __ldword3
	.code
__ldword3:
	ld hl,3
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
