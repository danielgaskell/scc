
	.export __ldword30
	.code
__ldword30:
	ld hl,30
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
