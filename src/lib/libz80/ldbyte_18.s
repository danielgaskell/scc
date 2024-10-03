
	.export __ldbyte18
	.code
__ldbyte18:
	ld hl,18
	add hl,sp
	ld l,(hl)
	ret
