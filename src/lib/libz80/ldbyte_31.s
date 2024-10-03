
	.export __ldbyte31
	.code
__ldbyte31:
	ld hl,31
	add hl,sp
	ld l,(hl)
	ret
