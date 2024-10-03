
	.export __ldbyte4
	.code
__ldbyte4:
	ld hl,4
	add hl,sp
	ld l,(hl)
	ret
