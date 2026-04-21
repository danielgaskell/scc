.z80
.code

.export _isprint
_isprint:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	ld hl,0
	cp 32
	ret c
	cp 127
	ret nc
	ld hl,1
	ret
	