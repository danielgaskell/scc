
	.export __stword6
	.code
__stword6:
	ex de,hl
	ld hl,6
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
