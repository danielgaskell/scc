; fast(er) Z80 memset

	.z80
	.code

	.export _memset
_memset:
	pop af
	pop hl
	pop de
	pop bc
	push af
	push hl			; save starting address
	ld a,b			; skip if BC = 0 (since ldir wraps)
	or c
	jr z,skip
	ld (hl),e		; copy value into first address
	dec bc			; skip if BC = 1 (since ldir wraps)
	ld a,b
	or c
	jr z,skip
	ld e,l			; de = hl + 1
	ld d,h
	inc de
	ldir			; propagate value through
skip:
	pop hl
	pop af
	push bc
	push de
	push hl
	push af
	ret
