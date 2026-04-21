.z80
.code

.export _isdigit
_isdigit:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	ld hl,0
	cp 48
	ret c
	cp 58
	ret nc
	ld hl,1
	ret
	