
	.export __stword12
	.code
__stword12:
	ex de,hl
	ld hl,12
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
