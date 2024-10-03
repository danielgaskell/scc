
	.export __ldbyte2
	.code
__ldbyte2:
	ld hl,2
	add hl,sp
	ld l,(hl)
	ret
