.z80
.code

rndseed:
	.word 441

; Trivial 7-9-8 16-bit XORshift with a period of 65535.
.export _rand
_rand:
	ld hl,(rndseed)
	ld a,h
	rra
	ld a,l
	rra
	xor h
	ld h,a
	ld a,l
	rra
	ld a,h
	rra
	xor l
	ld l,a
	xor h
	ld h,a
	ld (rndseed),hl
	ret
	
.export _srand
_srand:
	pop hl
	pop de
	ld a,d			; cannot be 0
	or e
	jp nz,setseed
	ld de,441
setseed:
	ld (rndseed),de
	push de
	push hl
	ret
