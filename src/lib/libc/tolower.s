.z80
.code

.export _tolower
_tolower:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	cp 65
	ret c
	cp 91
	ret nc
	set 5,l
	ret
	