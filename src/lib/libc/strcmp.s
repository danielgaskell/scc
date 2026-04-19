.z80
.code

.export _strcmp
_strcmp:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld l,(ix+0)
	ld h,(ix+1)
	ld e,(ix+2)
	ld d,(ix+3)
_strcmploop:
	ld a,(de)
	cp (hl)
	jr nz,_strcmpdiff
	or a
	jr z,_strcmpmatch
	inc hl
	inc de
	jp _strcmploop
_strcmpmatch:
	ld hl,0
	pop ix
	pop bc
	ret
_strcmpdiff:
	jr c,_strcmppos
	ld hl,#0xFFFF
	pop ix
	pop bc
	ret
_strcmppos:
	ld hl,1
	pop ix
	pop bc
	ret
