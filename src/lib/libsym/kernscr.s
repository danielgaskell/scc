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
