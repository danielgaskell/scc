.z80
.code

.export _stricmp
_stricmp:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld l,(ix+0)
	ld h,(ix+1)
	ld e,(ix+2)
	ld d,(ix+3)
_stricmploop:
	ld a,(de)
	ld c,a
	ld a,(hl)
	cp c
	jr z,_stricmpcont
	cp 65
	jr c,_stricmpconv
	cp 91
	jr nc,_stricmpconv
	or #0x20
_stricmpconv:
	ld b,a
	ld a,c
	cp 65
	jr c,_stricmpcomp
	cp 91
	jr nc,_stricmpcomp
	or #0x20
_stricmpcomp:
	cp b
	jr nz,_stricmpdiff
_stricmpcont:
	or a
	jr z,_stricmpmatch
	inc hl
	inc de
	jp _stricmploop
_stricmpmatch:
	ld hl,0
	pop ix
	pop bc
	ret
_stricmpdiff:
	jr c,_stricmppos
	ld hl,#0xFFFF
	pop ix
	pop bc
	ret
_stricmppos:
	ld hl,1
	pop ix
	pop bc
	ret
