
	.export __stword18
	.code
__stword18:
	ex de,hl
	ld hl,18
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
