
	.export __stword13
	.code
__stword13:
	ex de,hl
	ld hl,13
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
