
	.export __stword2
	.code
__stword2:
	ex de,hl
	ld hl,2
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
