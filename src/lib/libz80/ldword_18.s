
	.export __ldword18
	.code
__ldword18:
	ld hl,18
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
