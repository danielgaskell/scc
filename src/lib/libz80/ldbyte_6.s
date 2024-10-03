
	.export __ldbyte6
	.code
__ldbyte6:
	ld hl,6
	add hl,sp
	ld l,(hl)
	ret
