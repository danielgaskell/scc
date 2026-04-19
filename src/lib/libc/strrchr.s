.z80
.code

.export _strrchr
_strrchr:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld l,(ix+0)
	ld h,(ix+1)
	ld a,(ix+2)
	or a
	jr nz,_strrchrfind
	; fall through = find null terminator
	ld b,a
	ld c,a
	cpir
	dec hl
	pop ix
	pop bc
	ret
_strrchrfind:
	ld d,a
	xor a
	ld b,a
	ld c,a
	cpir
	ld a,b
	cpl
	ld b,a
	ld a,c
	cpl
	ld c,a
	or b		; empty string?
	jr z,_strrchrnone
	dec hl
	dec hl
	ld a,d
	cpdr
	jp z,_strrchrfound
_strrchrnone:
	ld hl,0
	pop ix
	pop iy
	ret
_strrchrfound:
	inc hl
	pop ix
	pop iy
	ret
