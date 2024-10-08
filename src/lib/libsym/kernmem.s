	.code

; Memory_Summary (1)
.export _Mem_Free
_Mem_Free:
	push bc
	push ix
	push iy
	ld hl,#0x8100
	rst #0x28
	ld d,0
	ld (__hireg),de
	push ix
	pop hl
	pop iy
	pop ix
	pop bc
	ret

; Memory_Summary (1)
.export _Mem_Banks
_Mem_Banks:
	push bc
	push ix
	push iy
	ld hl,#0x8100
	rst #0x28
	ld h,0
	ld l,d
	pop iy
	pop ix
	pop bc
	ret

; Memory_Information
.export _Mem_Longest
_Mem_Longest:
	push bc
	push ix
	push iy
	ld ix,0
	add ix,sp
	ld a,(ix+6)
	ld e,(ix+8)
	rst #0x20
	.word #0x8121
	ld h,b
	ld l,c
	pop iy
	pop ix
	pop bc
	ret
	
; Memory_Get
.export _Mem_Reserve
_Mem_Reserve:
	push bc
	push ix
	ld ix,#0x02
	add ix,sp
	ld a,(ix+4)
	ld e,(ix+6)
	ld c,(ix+8)
	ld b,(ix+9)
	rst #0x20
	.word #0x8118
	ld c,(ix+10);	; save bank to ptr1
	ld b,(ix+11);
	ld (bc),a
	ld c,(ix+12);	; save addr to ptr2
	ld b,(ix+13);
	ld a,l
	ld (bc),a
	inc bc
	ld a,h
	ld (bc),a
	ld hl,0
	adc hl,hl		; error is in CF
	pop ix
	pop bc
	ret
	
; Memory_Free
.export _Mem_Release
_Mem_Release:
	push bc
	push ix
	ld ix,#0x02
	add ix,sp
	ld a,(ix+4)
	ld l,(ix+6)
	ld h,(ix+7)
	ld c,(ix+8)
	ld b,(ix+9)
	rst #0x20
	.word #0x811B
	pop ix
	pop bc
	ret
	