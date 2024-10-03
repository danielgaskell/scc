
	.export __ldbyte13
	.code
__ldbyte13:
	ld hl,13
	add hl,sp
	ld l,(hl)
	ret
