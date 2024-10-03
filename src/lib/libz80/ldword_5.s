
	.export __ldword5
	.code
__ldword5:
	ld hl,5
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
