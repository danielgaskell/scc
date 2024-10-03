
	.export __ldbyte24
	.code
__ldbyte24:
	ld hl,24
	add hl,sp
	ld l,(hl)
	ret
