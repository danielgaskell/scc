
	.export __ldbyte23
	.code
__ldbyte23:
	ld hl,23
	add hl,sp
	ld l,(hl)
	ret
