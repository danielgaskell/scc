
	.export __stword14
	.code
__stword14:
	ex de,hl
	ld hl,14
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
