.z80
.code

; main 4-color pixel draw routine
.export __gfx_pix4
__gfx_pix4:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	; color = color AND 3 (downsamples reasonably well)
	ld a,(ix+6)
	and 3
	ld (ix+6),a
	call __gfx_xy4
	call __gfx_coff4
	ld hl,__pixtab      	; hl = mask data
	add hl,bc
	ld c,(hl)				; a = byte AND mask
	ld a,(de)
	and c
	ld c,(ix+6)             ; bc = color
	inc bc
	add hl,bc               ; hl = color data
	ld b,(hl)
	or b					; a = byte OR color
	ld (de),a				; write byte back
	pop ix
	pop bc
	ret

; main 16-color pixel draw routine
.export __gfx_pix16
__gfx_pix16:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	call __gfx_xy16
	ld a,(ix+2)
	and 1
	jr nz,pix16offset
	ld a,(de)
	and #0x0F
	ld b,(ix+6)
	sla b
	sla b
	sla b
	sla b
	or b
	ld (de),a
	pop ix
	pop bc
	ret
pix16offset:
	ld a,(de)
	and #0xF0
	ld b,(ix+6)
	or b
	ld (de),a
	pop ix
	pop bc
	ret

.export _Gfx_Pixel
_Gfx_Pixel:
	ld a,(__gfx_16)
	or a
	jp z,__gfx_pix4
	jp __gfx_pix16
