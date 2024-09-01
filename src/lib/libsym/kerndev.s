	.code

; Device_ScreenMode (1)
.export _Screen_Mode
_Screen_Mode:
	push ix
	push iy
	ld hl,#0x8139
	rst #0x28
	ld h,0
	ld l,e
	pop iy
	pop ix
	ret
	
; Device_ScreenMode (2)
.export _Screen_Colors
_Screen_Colors:
	push ix
	push iy
	ld hl,#0x8139
	rst #0x28
	ld h,0
	ld l,d
	pop iy
	pop ix
	ret
	
; Device_ScreenMode (2)
.export _Screen_Width
_Screen_Width:
	push ix
	push iy
	ld hl,#0x8139
	rst #0x28
	push ix
	pop hl
	pop iy
	pop ix
	ret
	
; Device_ScreenMode (2)
.export _Screen_Height
_Screen_Height:
	push ix
	push iy
	ld hl,#0x8139
	rst #0x28
	push iy
	pop hl
	pop iy
	pop ix
	ret

; Device_MousePosition (1)
.export _Mouse_X
_Mouse_X:
	rst #0x20
	.word #0x813C
	ld d,h
	ld e,l
	ret

; Device_MousePosition (2)
.export _Mouse_Y
_Mouse_Y:
	rst #0x20
	.word #0x813C
	ret

; Device_MouseKeyStatus
.export _Mouse_Buttons
_Mouse_Buttons:
	rst #0x20
	.word #0x813F
	ld h,0
	ld l,a
	ret

; Device_KeyTest
.export _Key_Down
_Key_Down:
	push ix
	push iy
	ld ix,0
	add ix,sp
	ld e,(ix+6)
	ld hl,#0x8145
	rst #0x28
	ld h,0
	ld l,e
	pop iy
	pop ix
	ret

; Device_KeyStatus
.export _Key_Status
_Key_Status:
	push ix
	push iy
	ld hl,#0x8148
	rst #0x28
	ld h,d
	ld l,e
	pop iy
	pop ix
	ret

; Device_KeyPut
.export _Key_Put
_Key_Put:
	push ix
	ld ix,0
	add ix,sp
	ld a,(ix+4)
	rst #0x20
	.word #0x814B
	ld hl,0
	adc hl,hl		; result is in CF
	pop ix
	ret

; Device_KeyMulti
.export _Key_Multi
_Key_Multi:
	push ix
	push iy
	ld ix,0
	add ix,sp
	ld e,(ix+6)
	ld d,(ix+8)
	ld a,(ix+10)
	.byte 0xDD
	ld l,a
	ld a,(ix+12)
	.byte 0xDD
	ld h,a
	ld a,(ix+14)
	.byte 0xED
	ld l,a
	ld a,(ix+16)
	.byte 0xED
	ld h,a
	ld hl,#0x8160
	rst #0x28
	ld h,0
	ld l,e
	pop iy
	pop ix
	ret

; Screen_TextLength (1)
.export _Text_Width
_Text_Width:
	push ix
	push iy
	ld ix,0
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
	ret

; Screen_TextLength (2)
.export _Text_Height
_Text_Height:
	push ix
	push iy
	ld ix,0
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
	ret

; Device_TimeGet
.export _Time_Get
_Time_Get:
	push ix
	push iy
	ld iy,0
	add iy,sp
	ld l,(iy+6)
	ld h,(iy+7)
	push hl
	rst #0x20
	.word #0x810C
	pop iy
	ld (iy+0),a
	ld (iy+1),b
	ld (iy+2),c
	ld (iy+3),d
	ld (iy+4),e
	ld (iy+5),l
	ld (iy+6),h
	.byte #0xDD
	ld a,l          ; ld a,ixl
	ld (iy+7),a
	pop iy
	pop ix
	ret

; Device_TimeSet
.export _Time_Set
_Time_Set:
	push ix
	push iy
	ld ix,0
	add ix,sp
	ld b,(ix+6)
	ld c,(ix+7)
	push bc
	pop ix
	ld a,(ix+0)
	ld b,(ix+1)
	ld c,(ix+2)
	ld d,(ix+3)
	ld e,(ix+4)
	ld l,(ix+5)
	ld h,(ix+6)
	ld a,(ix+7)
	.byte #0xDD
	ld l,a          ; ld ixl,a
	rst #0x20
	.word #0x810F
	pop iy
	pop ix
	ret
	