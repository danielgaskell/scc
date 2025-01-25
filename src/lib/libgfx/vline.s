.z80
.code

.export __gfx_vline4
__gfx_vline4:
	push bc
	push ix
	push iy
	ld ix,#0x04
	add ix,sp
	call __gfx_xy4
	call __gfx_coff4
	ld hl,__pixtab      	; IY = mask byte address
	add hl,bc
	push hl
	pop iy
	ld a,(ix+8)             ; IX = color byte address (with color AND 3 to downsample)
	and 3
	ld c,a
	inc bc
	add hl,bc
	push hl
	ld hl,(__gfx_activebw)  ; DE = row width in bytes
	ex de,hl
	ld b,(ix+6)             ; B = height remaining in pixels
	pop ix
vline4loop:
	ld c,(iy+0)				; a = byte AND mask
	ld a,(hl)
	and c
	ld c,(ix+0)				; a = byte OR color
	or c
	ld (hl),a				; write byte back
	add hl,de
	dec b
	jr nz,vline4loop
	pop iy
	pop ix
	pop bc
	ret

.export __gfx_vline16
__gfx_vline16:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	call __gfx_xy16
	ld hl,(__gfx_activebw)  ; DE = row width in bytes
	ex de,hl
	ld b,(ix+6)             ; B = height remaining in pixels
	ld a,(ix+2)
	and 1
	jr z,vline16left
	ld c,(ix+8)				; C = color
vline16loop1:
	ld a,(hl)
	and #0xF0				; a = byte AND mask
	or c					; a = byte OR color
	ld (hl),a				; write byte back
	add hl,de				; go down a line
	djnz vline16loop1
	pop ix
	pop bc
	ret
vline16left:
	ld c,(ix+8)				; C = color
	sla c
	sla c
	sla c
	sla c
vline16loop2:
	ld a,(hl)
	and #0x0F				; a = byte AND mask
	or c					; a = byte OR color
	ld (hl),a				; write byte back
	add hl,de				; go down a line
	djnz vline16loop2
	pop ix
	pop bc
	ret

.export _Gfx_VLine
_Gfx_VLine:
	ld a,(__gfx_16)
	or a
	jp z,__gfx_vline4
	jp __gfx_vline16
