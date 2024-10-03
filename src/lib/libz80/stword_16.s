
	.export __stword16
	.code
__stword16:
	ex de,hl
	ld hl,16
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
