
	.export __ldbyte8
	.code
__ldbyte8:
	ld hl,8
	add hl,sp
	ld l,(hl)
	ret
