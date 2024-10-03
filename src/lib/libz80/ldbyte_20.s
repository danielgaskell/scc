
	.export __ldbyte20
	.code
__ldbyte20:
	ld hl,20
	add hl,sp
	ld l,(hl)
	ret
