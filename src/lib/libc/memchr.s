.z80
.code

.export _memchr
_memchr:
	push bc
	push ix
	ld ix,6
	add ix,sp
	ld c,(ix+4)
	ld b,(ix+5)
	ld a,b
	or c
	jr z,_memchrfail
	ld l,(ix+0)
	ld h,(ix+1)
	ld a,(ix+2)
	cpir
	jr nz,_memchrfail
	dec hl
	pop ix
	pop bc
	ret
_memchrfail:
	ld hl,0
	pop ix
	pop bc
	ret
	