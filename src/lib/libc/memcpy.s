; fast(er) Z80 memcpy

	.z80
	.code

	.export _memcpy
_memcpy:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	ld l,(ix+4)		; read address -> HL
	ld h,(ix+5)
	ld c,(ix+6)		; len -> BC
	ld b,(ix+7)
	ld a,b			; skip if BC = 0 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	ld e,(ix+2)		; write address -> DE
	ld d,(ix+3)
	ldir			; do copy
	ld l,(ix+4)		; restore starting address
	ld h,(ix+5)
skip:
	pop ix
	pop bc
	ret

