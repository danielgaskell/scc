
	.export __ldword26
	.code
__ldword26:
	ld hl,26
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
