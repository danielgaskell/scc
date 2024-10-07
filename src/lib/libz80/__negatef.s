.z80
.code

.export __negatef
__negatef:
	ld hl,2
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld a,(hl)
	ld (__hireg+0),a
	inc hl
	ld a,(hl)
	xor #0x80
	ld (__hireg+1),a
	ex de,hl
	ret
	