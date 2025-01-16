; fast(er) Z80 memset

	.z80
	.code

	.export _memset
_memset:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	ld c,(ix+6)		; len -> BC
	ld b,(ix+7)
	ld a,b			; skip if BC = 0 (since ldir wraps)
	or c
	jr z,skip
	ld l,(ix+2)		; write address -> HL
	ld h,(ix+3)
	ld e,(ix+4)		; value -> E
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
	pop ix
	pop bc
	ret
