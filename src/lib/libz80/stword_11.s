
	.export __stword11
	.code
__stword11:
	ex de,hl
	ld hl,11
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
