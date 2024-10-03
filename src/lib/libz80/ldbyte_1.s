
	.export __ldbyte1
	.code
__ldbyte1:
	ld hl,1
	add hl,sp
	ld l,(hl)
	ret
