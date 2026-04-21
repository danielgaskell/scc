.z80
.code

.export _isascii
_isascii:
	pop de
	pop hl
	push hl
	push de
	bit 7,l
	ld hl,1
	ret z
	ld hl,0
	ret
	