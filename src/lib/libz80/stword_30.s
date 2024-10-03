
	.export __stword30
	.code
__stword30:
	ex de,hl
	ld hl,30
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
