
	.export __stword5
	.code
__stword5:
	ex de,hl
	ld hl,5
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
