
	.export __ldbyte11
	.code
__ldbyte11:
	ld hl,11
	add hl,sp
	ld l,(hl)
	ret
