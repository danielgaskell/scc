
	.export __ldbyte12
	.code
__ldbyte12:
	ld hl,12
	add hl,sp
	ld l,(hl)
	ret
