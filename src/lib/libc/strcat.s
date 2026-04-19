.z80
.code

.export _strcat
_strcat:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld l,(ix+0)
	ld h,(ix+1)
	ld e,(ix+2)
	ld d,(ix+3)
	push hl
	ld bc,0
	xor a
	cpir
	dec hl
_strcatloop:
	ld a,(de)
	ld (hl),a
	inc hl
	inc de
	or a
	jp nz,_strcatloop
	pop hl
	pop ix
	pop bc
	ret
	