.z80
.code

.export _strchr
_strchr:
	push ix
	ld ix,4
	add ix,sp
	ld l,(ix+0)
	ld h,(ix+1)
	ld d,(ix+2)
	pop ix
	dec hl
_strchrloop:
	inc hl
	ld a,(hl)
	cp d
	ret z
	or a
	jp nz,_strchrloop
	ld hl,0		; not found
	ret
	