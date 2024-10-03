
	.export __stword3
	.code
__stword3:
	ex de,hl
	ld hl,3
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
