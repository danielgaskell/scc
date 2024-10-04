.z80
.code

.export __ccgtf
__ccgtf:
	ld hl,2
	add hl,sp
	ex de,hl
	ld hl,6
	add hl,sp
	call f32cmp
	ld hl,0
	ret z
	ret c
	ld l,1
	ret
	