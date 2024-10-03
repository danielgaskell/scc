
	.export __stword23
	.code
__stword23:
	ex de,hl
	ld hl,23
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
