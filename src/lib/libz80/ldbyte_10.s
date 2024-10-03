
	.export __ldbyte10
	.code
__ldbyte10:
	ld hl,10
	add hl,sp
	ld l,(hl)
	ret
