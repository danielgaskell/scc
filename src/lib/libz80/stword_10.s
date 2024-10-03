
	.export __stword10
	.code
__stword10:
	ex de,hl
	ld hl,10
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
