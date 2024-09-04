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
	ld (addr),de	; save starting address
	ld a,b			; skip if BC = 0 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	ldir
skip:
	pop af
	push bc
	push hl
	push de
	push af
	ld hl,(addr)
	ret

addr:
	.word 0

.byte 1
.byte 2
.byte 3
.byte 4
.byte 1
.byte 2
.byte 3
.byte 4
