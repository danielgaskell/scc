
	.export __ldword1
	.code
__ldword1:
	ld hl,1
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ret
