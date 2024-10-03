
	.export __ldword12
	.code
__ldword12:
	ld hl,12
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
