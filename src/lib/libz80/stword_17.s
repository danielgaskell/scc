
	.export __stword17
	.code
__stword17:
	ex de,hl
	ld hl,17
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
