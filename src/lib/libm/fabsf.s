.z80
.code

.export _fabsf
_fabsf:
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
	and #0x7F
	ld (__hireg+1),a
	ex de,hl
	ret
	