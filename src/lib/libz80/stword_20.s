
	.export __stword20
	.code
__stword20:
	ex de,hl
	ld hl,20
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
