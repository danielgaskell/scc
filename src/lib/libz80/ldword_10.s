
	.export __ldword10
	.code
__ldword10:
	ld hl,10
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
