; fast(er) Z80 memmove

	.z80
	.code

	.export _memmove
_memmove:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld l,(ix+2)			; read address -> HL
	ld h,(ix+3)
	ld c,(ix+4)			; len -> BC
	ld b,(ix+5)
	ld a,b				; skip if BC = 0 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	ld e,(ix+0)			; write address -> DE
	ld d,(ix+1)
	push hl
	sbc hl,de
	pop hl
	jr z,skip			; skip if addresses are equal
	jr nc,copyforward
copybackward:
	add hl,bc
	dec hl
	ex de,hl
	add hl,bc
	dec hl
	ex de,hl
	lddr
	jp skip
copyforward:
	ldir				; do copy
skip:
	ld l,(ix+0)			; return write address
	ld h,(ix+1)
	pop ix
	pop bc
	ret
