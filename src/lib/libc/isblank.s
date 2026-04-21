.z80
.code

.export _isblank
_isblank:
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
	ld hl,0
	ret
	