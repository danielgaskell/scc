
	.export __stword15
	.code
__stword15:
	ex de,hl
	ld hl,15
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
