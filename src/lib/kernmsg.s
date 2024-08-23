	.code

; Message_Sleep_And_Receive
.export _Msg_Sleep
_Msg_Sleep:
	push ix
	push iy
	ld ix,0
	add ix,sp
	ld l,(ix+6)
	ld h,(ix+8)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x08
	push ix
	pop hl
	pop iy
	pop ix
	ret
	
; Message_Send
.export _Msg_Send
_Msg_Send:
	push ix
	push iy
	ld ix,0
	add ix,sp
	ld l,(ix+6)
	ld h,(ix+8)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x10
	push ix
	pop hl
	pop iy
	pop ix
	ret

; Message_Receive
.export _Msg_Receive
_Msg_Receive:
	push ix
	push iy
	ld ix,0
	add ix,sp
	ld l,(ix+6)
	ld h,(ix+8)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x18
	push ix
	pop hl
	pop iy
	pop ix
	ret

; Multitasking_SoftInterrupt
.export _Idle
_Idle:
	rst #0x30
	ret

