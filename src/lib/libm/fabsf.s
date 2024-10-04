.z80
.code

.export _fabs
_fabs:
	ld hl,5
	add hl,sp
	ld a,(hl)
	and #0x7F
	ld (hl),a
	ret
	