
	.export __ldword25
	.code
__ldword25:
	ld hl,25
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
