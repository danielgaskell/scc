
	.export __ldword27
	.code
__ldword27:
	ld hl,27
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
