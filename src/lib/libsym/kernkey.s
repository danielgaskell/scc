	.code

; Device_KeyTest
.export _Key_Down
_Key_Down:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld e,(ix+6)
	ld hl,#0x8145
	rst #0x28
	ld h,0
	ld l,e
	pop iy
	pop ix
	pop bc
	ret

; Device_KeyStatus
.export _Key_Status
_Key_Status:
	push bc
	push ix
	push iy
	ld hl,#0x8148
	rst #0x28
	ld h,d
	ld l,e
	pop iy
	pop ix
	pop bc
	ret

; Device_KeyPut
.export _Key_Put
_Key_Put:
	push bc
	push ix
	ld ix,#0x02
	add ix,sp
	ld a,(ix+4)
	rst #0x20
	.word #0x814B
	ld hl,0
	adc hl,hl		; result is in CF
	pop ix
	pop bc
	ret

; Device_KeyMulti
.export _Key_Multi
_Key_Multi:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld e,(ix+8)
	ld d,(ix+10)
    push de
	ld e,(ix+6)
	ld d,(ix+16)
	ld a,(ix+12)
	.byte 0xED
	ld l,a
	ld a,(ix+14)
	.byte 0xED
	ld h,a
    pop ix
	ld hl,#0x8160
	rst #0x28
	ld h,0
	ld l,e
	pop iy
	pop ix
	pop bc
	ret
