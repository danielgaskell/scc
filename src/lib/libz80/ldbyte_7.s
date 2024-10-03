
	.export __ldbyte7
	.code
__ldbyte7:
	ld hl,7
	add hl,sp
	ld l,(hl)
	ret
