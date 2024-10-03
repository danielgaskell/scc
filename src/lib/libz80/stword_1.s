
	.export __stword1
	.code
__stword1:
	ex de,hl
	ld hl,1
	add hl,sp
	ld (hl),e
	inc hl
	ld (hl),d
	ex de,hl
	ret
