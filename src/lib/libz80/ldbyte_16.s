
	.export __ldbyte16
	.code
__ldbyte16:
	ld hl,16
	add hl,sp
	ld l,(hl)
	ret
