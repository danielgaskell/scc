
	.export __ldbyte25
	.code
__ldbyte25:
	ld hl,25
	add hl,sp
	ld l,(hl)
	ret
