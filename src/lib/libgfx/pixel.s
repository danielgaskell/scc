.z80
.code

; main 4-color pixel draw routine
.export __gfx_pix4
__gfx_pix4:
	ld ix,#0x0
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
	ret

; main 16-color pixel draw routine
.export __gfx_pix16
__gfx_pix16:
	ld ix,#0x0
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
	ret
pix16offset:
	ld a,(de)
	and #0xF0
	ld b,(ix+6)
	or b
	ld (de),a
	ret					

.export __gfx_pix
__gfx_pix:
	.word __gfx_pix4

.export _Gfx_Pixel
_Gfx_Pixel:
	pop bc
	ld hl,(__gfx_pix)
	push bc
	push hl
	ret ; redirect call to __gfx_pix
