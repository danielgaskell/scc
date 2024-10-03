
	.export __ldbyte3
	.code
__ldbyte3:
	ld hl,3
	add hl,sp
	ld l,(hl)
	ret
