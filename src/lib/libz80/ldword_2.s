
	.export __ldword2
	.code
__ldword2:
	ld hl,2
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
