.z80
.code

.export _strlen
_strlen:
	pop de
	pop hl
	push bc
	xor a
	ld b,a
	ld c,a
	cpir
	ld a,b
	cpl
	ld h,a
	ld a,c
	cpl
	ld l,a
	pop bc
	push hl
	push de
	ret
	