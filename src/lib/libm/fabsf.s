.z80
.code

.export _fabsf
_fabsf:
	ld hl,5
	add hl,sp
	ld a,(hl)
	and #0x7F
	ld (hl),a
	ret
	