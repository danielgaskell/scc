
	.export __stword31
	.code
__stword31:
	ex de,hl
	ld hl,31
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
