	.code

; Clipboard_Put
.export _Clip_Put
_Clip_Put:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld e,(ix+6)
	ld d,(ix+12)
	ld l,(ix+8)
	ld h,(ix+9)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x20		; BNKSCL
	.word #0x814E
	ld hl,0
	adc hl,hl		; result is in CF
	pop iy
	pop ix
	pop bc
	ret
	
; Clipboard_Get
.export _Clip_Get
_Clip_Get:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld e,(ix+6)
	ld d,(ix+12)
	ld l,(ix+8)
	ld h,(ix+9)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x20		; BNKSCL
	.word #0x8151
	push iy
	pop hl			; FIXME return error
	pop iy
	pop ix
	pop bc
	ret

; Clipboard_Status (1)
.export _Clip_Type
_Clip_Type:
	push ix
	push iy
	rst #0x20
	.word #0x8154
	ld h,0
	ld l,d
	pop iy
	pop ix
	ret

; Clipboard_Status (2)
.export _Clip_Len
_Clip_Len:
	push ix
	push iy
	rst #0x20
	.word #0x8154
	push iy
	pop hl
	pop iy
	pop ix
	ret
	
