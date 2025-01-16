; fast(er) Z80 memmove

	.z80
	.code

.byte 1,2,3,4,1,2,3,4
	.export _memmove
_memmove:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	ld l,(ix+4)			; read address -> HL
	ld h,(ix+5)
	ld c,(ix+6)			; len -> BC
	ld b,(ix+7)
	ld a,b				; skip if BC = 0 (ldir wraps rather than doing nothing)
	or c
	jr z,skip
	ld e,(ix+2)			; write address -> DE
	ld d,(ix+3)
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
	ld l,(ix+4)			; restore starting address
	ld h,(ix+5)
	pop ix
	pop bc
	ret

