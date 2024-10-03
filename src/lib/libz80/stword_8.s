
	.export __stword8
	.code
__stword8:
	ex de,hl
	ld hl,8
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
