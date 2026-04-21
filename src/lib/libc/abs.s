.z80
.code

.export _abs
_abs:
	pop de
	pop hl
	push hl
	push de
	bit 7,h
	ret z
	xor a
	sub l
	ld l,a
	sbc a,a
	sub h
	ld h,a
	ret
