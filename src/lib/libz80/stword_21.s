
	.export __stword21
	.code
__stword21:
	ex de,hl
	ld hl,21
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
