; fast(er) Z80 memcpy

	.z80
	.code

	.export _memcpy
_memcpy:
	pop iy
	pop de
	pop hl
	pop bc
	ld (addr),de	; save starting address
	ld a,b			; skip if BC = 0 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	ldir
skip:
	push bc
	push hl
	push de
	push iy
	ld hl,(addr)
	ret

addr:
	.word 0
