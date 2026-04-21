.z80
.code

.export _islower
_islower:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	ld hl,0
	cp 97
	ret c
	cp 123
	ret nc
	ld hl,1
	ret
	