
	.export __ldword13
	.code
__ldword13:
	ld hl,13
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
