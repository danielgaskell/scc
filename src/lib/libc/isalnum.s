.z80
.code

.export _isalnum
_isalnum:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	and 0xDF
	sub 0x41
	cp 26
	jr c,_isalnumdone
	ld a,l
	sub 0x30
	cp 10
_isalnumdone:
	ld hl,0
	rl l
	ret
