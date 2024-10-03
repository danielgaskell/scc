
	.export __stword25
	.code
__stword25:
	ex de,hl
	ld hl,25
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
