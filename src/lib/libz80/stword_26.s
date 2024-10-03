
	.export __stword26
	.code
__stword26:
	ex de,hl
	ld hl,26
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
