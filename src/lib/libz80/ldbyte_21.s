
	.export __ldbyte21
	.code
__ldbyte21:
	ld hl,21
	add hl,sp
	ld l,(hl)
	ret
