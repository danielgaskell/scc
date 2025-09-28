	.code

; Screen_TextLength (1)
.export _Text_Width
_Text_Width:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld a,(ix+6)
	ld l,(ix+8)
	ld h,(ix+9)
	ld e,(ix+10)
	ld d,(ix+11)
	push de
	pop iy
	rst #0x20
	.word #0x815d
	ld h,d
	ld l,e
	pop iy
	pop ix
	pop bc
	ret

; Screen_TextLength (2)
.export _Text_Height
_Text_Height:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld a,(ix+6)
	ld l,(ix+8)
	ld h,(ix+9)
	ld e,(ix+10)
	ld d,(ix+11)
	push de
	pop iy
	rst #0x20
	.word #0x815d
	ld h,0
	ld l,a
	pop iy
	pop ix
	pop bc
	ret
