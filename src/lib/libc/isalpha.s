.z80
.code

.export _isalpha
_isalpha:
	pop de
	pop hl
	push hl
	push de
	ld a,l
	and 0xDF
	sub 0x41
	cp 0x1A
	ld hl,0
	rl l
	ret
	