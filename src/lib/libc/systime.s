.code

; Device_TimeGet
.export _Time_Get
_Time_Get:
	push bc
	push ix
	push iy
	ld iy,2
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
	pop bc
	ret

; Device_TimeSet
.export _Time_Set
_Time_Set:
	push bc
	push ix
	push iy
	ld ix,2
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
	pop bc
	ret
	