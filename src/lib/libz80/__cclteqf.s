.z80
.code

.export __cclteqf
__cclteqf:
	ld hl,2
	add hl,sp
	ex de,hl
	ld hl,6
	add hl,sp
	call f32cmp
	ld l,1
	ret z
	ret c
	ld hl,0
	ret
	