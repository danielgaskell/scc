.z80
.code

.export _isgraph
_isgraph:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	ld hl,0
	cp 33
	ret c
	cp 127
	ret nc
	ld hl,1
	ret
	