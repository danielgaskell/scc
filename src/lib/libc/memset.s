; fast(er) Z80 memset

	.z80
	.code

	.export _memset
_memset:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld c,(ix+4)		; len -> BC
	ld b,(ix+5)
	ld a,b			; skip if BC = 0 (since ldir wraps)
	or c
	jr z,skip
	ld l,(ix+0)		; write address -> HL
	ld h,(ix+1)
	ld e,(ix+2)		; value -> E
	ld (hl),e		; copy value into first address
	dec bc			; skip if BC = 1 (since ldir wraps)
	ld a,b
	or c
	jr z,skip
	push hl
	ld e,l			; de = hl + 1
	ld d,h
	inc de
	ldir			; propagate value through
	pop hl
skip:
	pop ix
	pop bc
	ret
