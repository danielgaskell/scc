
	.export __stword28
	.code
__stword28:
	ex de,hl
	ld hl,28
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
