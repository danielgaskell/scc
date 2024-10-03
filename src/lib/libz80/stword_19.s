
	.export __stword19
	.code
__stword19:
	ex de,hl
	ld hl,19
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
