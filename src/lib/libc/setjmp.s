	.code
.export _setjmp
_setjmp:
	pop bc     ; return address
	pop de     ; save pointer
	ld hl,0
	add hl,sp
	ex de,hl   ; hl = save pointer, de = stack pointer
	ld (hl),c
	inc hl
	ld (hl),b
	inc hl
	ld (hl),e
	inc hl
	ld (hl),d
	ld hl,0
	push hl
	push bc
	ret
	