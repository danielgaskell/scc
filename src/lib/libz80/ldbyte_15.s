
	.export __ldbyte15
	.code
__ldbyte15:
	ld hl,15
	add hl,sp
	ld l,(hl)
	ret
