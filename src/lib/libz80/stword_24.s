
	.export __stword24
	.code
__stword24:
	ex de,hl
	ld hl,24
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
