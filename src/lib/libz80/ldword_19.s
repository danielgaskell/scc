
	.export __ldword19
	.code
__ldword19:
	ld hl,19
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
