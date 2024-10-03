
	.export __stword4
	.code
__stword4:
	ex de,hl
	ld hl,4
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
