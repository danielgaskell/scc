
	.export __stword29
	.code
__stword29:
	ex de,hl
	ld hl,29
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
