	.code

; Banking_ReadWord
.export _Bank_ReadWord
_Bank_ReadWord:
	push ix
	ld ix,0
	add ix,sp
	ld a,(ix+4)
	ld l,(ix+6)
	ld h,(ix+7)
	rst #0x20
	.word #0x8124
	ld h,b
	ld l,c
	pop ix
	ret
	
; Banking_WriteWord
.export _Bank_WriteWord
_Bank_WriteWord:
	push ix
	ld ix,0
	add ix,sp
	ld a,(ix+4)
	ld l,(ix+6)
	ld h,(ix+7)
	ld c,(ix+8)
	ld b,(ix+9)
	rst #0x20
	.word #0x8127
	pop ix
	ret
	
; Banking_ReadByte
.export _Bank_ReadByte
_Bank_ReadByte:
	push ix
	ld ix,0
	add ix,sp
	ld a,(ix+4)
	ld l,(ix+6)
	ld h,(ix+7)
	rst #0x20
	.word #0x812A
	ld h,0
	ld l,b
	pop ix
	ret

; Banking_WriteByte
.export _Bank_WriteByte
_Bank_WriteByte:
	push ix
	ld ix,0
	add ix,sp
	ld a,(ix+4)
	ld l,(ix+6)
	ld h,(ix+7)
	ld b,(ix+8)
	rst #0x20
	.word #0x812D
	pop ix
	ret
	
; Banking_Copy
.export _Bank_Copy
_Bank_Copy:
	push ix
	ld ix,0
	add ix,sp
	ld a,(ix+4)		; A = (bankDst << 4) | bankSrc
	ld b,(ix+8)
	sla a
	sla a
	sla a
	sla a
	or b
	ld e,(ix+6)		; DE = addrDst
	ld d,(ix+7)
	ld l,(ix+10)	; HL = addrSrc
	ld h,(ix+11)
	ld c,(ix+12)	; BC = len
	ld b,(ix+13)
	rst #0x20
	.word #0x8130
	pop ix
	ret
	
; Banking_GetBank
.export _Bank_Get
_Bank_Get:
	rst #0x20
	.word #0x8133
	ld h,0
	ld l,a
	ret
	
