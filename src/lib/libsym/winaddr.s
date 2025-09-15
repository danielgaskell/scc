.code
.z80

.export _Win_Addr
_Win_Addr:
	push bc
	push ix
	push iy
	
	; get window table address
	ld e,7					; bank of version string -> IYL
	ld hl,#0x8103
	rst #0x28				; System_Information
	push iy
	ld e,8					; version string address -> IY
	ld hl,#0x8103
	rst #0x28				; System_Information
	push iy
	pop hl
	pop de
	ld bc,32				; extended desktop data address is at IY+32
	add hl,bc
	ld a,e
	rst #0x20				; Banking_ReadWord -> BC
	.word #0x8124
	ld hl,11				; window table address is at byte 11 of extended desktop data
	add hl,bc
	ld a,0
	rst #0x20				; Banking_ReadWord -> BC
	.word #0x8124
	
	; retrieve values
	ld ix,8
	add ix,sp
	ld a,(ix+0)
	rlca
	rlca
	inc a
	ld l,a
	ld h,0
	add hl,bc
	ld a,0
	rst #0x20				; Banking_ReadByte, get bank -> B
	.word #0x812A
	ld d,b
	ld a,0
	rst #0x20				; Banking_ReadWord, get address -> BC
	.word #0x8124
	ld l,(ix+2)				; save bank
	ld h,(ix+3)
	ld (hl),d
	ld l,(ix+4)				; save address
	ld h,(ix+5)
	ld (hl),c
	inc hl
	ld (hl),b
	
	pop iy
	pop ix
	pop bc
	ret
	