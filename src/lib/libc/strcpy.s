.z80
.code

.export _strcpy
_strcpy:
	push ix
	ld ix,4
	add ix,sp
	ld e,(ix+0)
	ld d,(ix+1)
	ld l,(ix+2)
	ld h,(ix+3)
	push de
_strcpyloop:
	ld a,(hl)
	ld (de),a
	inc hl
	inc de
	or a
	jp nz,_strcpyloop
	pop hl
	pop ix
	ret
	