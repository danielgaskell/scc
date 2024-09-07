; fast(er) Z80 memcpy

	.z80
	.code

	.export _memcpy
_memcpy:
	pop af
	pop de
	pop hl
	pop bc
	push af         ; push return address back to stack
	push hl			; save starting address
	ld a,b			; skip if BC = 0 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	ldir
skip:
	pop hl
	pop af
	push bc
	push hl
	push de
	push af
	ret
