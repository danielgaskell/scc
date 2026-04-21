.z80
.code

.export _toupper
_toupper:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	cp 97
	ret c
	cp 123
	ret nc
	res 5,l
	ret
	