; fast(er) Z80 memset

	.z80
	.code

	.export _memset
_memset:
	pop iy
	pop hl
	pop de
	pop bc
	ld (addr),de	; save starting address
	ld a,b			; skip if BC < 2 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	dec bc
	ld a,b
	or c
	jr z,skip
	ld (hl),e		; copy value into first address
	ld e,l			; de = hl + 1
	ld d,h
	inc de
	ldir			; propagate value through
skip:
	push bc
	push de
	push hl
	push iy
	ld hl,(addr)
	ret

addr:
	.word 0
