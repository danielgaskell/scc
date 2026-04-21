.z80
.code

.export _isspace
_isspace:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	ld hl,1
	cp 32
	ret z
	cp 9
	ret z
	cp 10
	ret z
	cp 11
	ret z
	cp 12
	ret z
	cp 13
	ret z
	ld hl,0
	ret
	