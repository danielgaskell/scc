.export _atoi

_atoi:
	pop de
	pop hl
	push hl
	push de
	ld a,(hl)
	cp '-'
	jr nz,atoinotneg
	inc hl			; advance past -
atoinotneg:
	push af
	push bc
	ex de,hl
	ld hl,0

atoiloop:
	ld a,(de)
	inc de
	sub '0'
	cp 10
	jr nc,atoidone	; exit on non-digit or null

	; HL = (HL * 10) + A
	add hl,hl
	ld c,l
	ld b,h
	add hl,hl
	add hl,hl
	add hl,bc
	ld c,a
	ld b,0
	add hl,bc
	jr atoiloop

atoidone:
	pop bc
	pop af			; restore saved sign
	jr nz,atoipos
	; negate HL
	xor a
	sub l
	ld l,a
	sbc a,a
	sub h
	ld h,a

atoipos:
	ret
	