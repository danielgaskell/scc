; fast(er) Z80 memcpy

	.z80
	.code

	.export _memcpy
_memcpy:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld l,(ix+2)		; read address -> HL
	ld h,(ix+3)
	ld c,(ix+4)		; len -> BC
	ld b,(ix+5)
	ld a,b			; skip if BC = 0 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	ld e,(ix+0)		; write address -> DE
	ld d,(ix+1)
	push de
	ldir			; do copy
	pop hl
skip:
	pop ix
	pop bc
	ret
