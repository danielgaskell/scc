	.code

; Device_MousePosition (1)
.export _Mouse_X
_Mouse_X:
	rst #0x20
	.word #0x813C
	ld h,d
	ld l,e
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
	ret
	