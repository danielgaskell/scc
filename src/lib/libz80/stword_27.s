
	.export __stword27
	.code
__stword27:
	ex de,hl
	ld hl,27
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
