.z80
.code

.export _isupper
_isupper:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	ld hl,0
	cp 65
	ret c
	cp 91
	ret nc
	ld hl,1
	ret
	