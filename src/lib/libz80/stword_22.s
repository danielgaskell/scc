
	.export __stword22
	.code
__stword22:
	ex de,hl
	ld hl,22
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
