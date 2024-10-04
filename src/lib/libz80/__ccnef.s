.z80
.code

.export __ccnef
__ccnef:
	ld hl,2
	add hl,sp
	ex de,hl
	ld hl,6
	add hl,sp
	call f32cmp
	ld hl,0
	ret z
	ld l,1
	ret
	