.z80
.code

.export __cceqf
__cceqf:
	ld hl,2
	add hl,sp
	ex de,hl
	ld hl,6
	add hl,sp
	call f32cmp
	ld hl,0
	ret nz
	ld l,1
	ret
	