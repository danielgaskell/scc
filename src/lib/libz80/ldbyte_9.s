
	.export __ldbyte9
	.code
__ldbyte9:
	ld hl,9
	add hl,sp
	ld l,(hl)
	ret
