
	.export __ldword14
	.code
__ldword14:
	ld hl,14
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
