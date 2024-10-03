
	.export __ldword20
	.code
__ldword20:
	ld hl,20
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
