
	.export __stword9
	.code
__stword9:
	ex de,hl
	ld hl,9
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
