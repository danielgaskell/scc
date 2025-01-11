.z80
.code

val4tab:
	.byte #0x08, #0x04, #0x02, #0x01

; 4-color pixel read
.export __gfx_val4
__gfx_val4:
	ld ix,#0x0
	add ix,sp
	call __gfx_xy4
	ld a,(ix+2)
	and 3
	ld b,0
	ld c,a
	ld hl,val4tab
	add hl,bc
	ld a,(de)
	ld b,(hl)
	and b
	ld h,0
	ld l,0
	jr z,val4skip
	ld l,2
val4skip:
	ld a,(de)
	srl a
	srl a
	srl a
	srl a
	and b
	ret z
	ld a,l
	or 1
	ld l,a
	ret

; 16-color pixel read
.export __gfx_val16
__gfx_val16:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	call __gfx_xy16
	ld a,(ix+2)
	and 1
	jr nz,val16offset
	ld a,(de)
	srl a
	srl a
	srl a
	srl a
	ld h,0
	ld l,a
	pop ix
	pop bc
	ret
val16offset:
	ld a,(de)
	and #0x0F
	ld h,0
	ld l,a
	pop ix
	pop bc
	ret

.export _Gfx_Value
_Gfx_Value:
	ld a,(__gfx_16)
	or a
	jp z,__gfx_val4
	jp __gfx_val16
	