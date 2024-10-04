.z80
.code

.export __negatef
__negatef:
	ld hl,5
	add hl,sp
	ld a,(hl)
	xor #0x80
	ld (hl),a
	ret
	