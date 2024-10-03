
	.export __ldbyte5
	.code
__ldbyte5:
	ld hl,5
	add hl,sp
	ld l,(hl)
	ret
