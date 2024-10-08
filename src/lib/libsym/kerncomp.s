	.code

; Banking_Decompress
.export _Bank_Decompress
_Bank_Decompress:
	push bc
	push ix
	push iy
	ld ix,#0x04
	add ix,sp
_BD_Start:
	ld b,(ix+4)
	ld e,(ix+6)
	ld d,(ix+7)
	ld l,(ix+8)
	ld h,(ix+9)
	ld ix,#0xff06
	ld iy,#0xfff4
	call #0xff09
	ld a,255
	sub c
	jr nz,_BD_End
	rst #0x30
	jr _BD_Start
_BD_End:
	pop iy
	pop ix
	pop bc
	ret
