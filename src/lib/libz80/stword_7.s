
	.export __stword7
	.code
__stword7:
	ex de,hl
	ld hl,7
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
