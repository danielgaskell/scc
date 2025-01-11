.z80
.code

__lin1tab:
	.byte #0x00, #0x00, #0xF0, #0x0F, #0xFF, #0x00, #0x00, #0x00
	.byte #0x88, #0x00, #0x70, #0x07, #0x77, #0x00, #0x00, #0x00
	.byte #0xCC, #0x00, #0x30, #0x03, #0x33, #0x00, #0x00, #0x00
	.byte #0xEE, #0x00, #0x10, #0x01, #0x11, #0x00, #0x00, #0x00
	
__lin2tab:
	.byte #0x00, #0xF0, #0x0F, #0xFF

__lin3tab:
	.byte #0xFF, #0x00, #0x00, #0x00, #0x00, #0x00, #0x00, #0x00
	.byte #0x77, #0x00, #0x80, #0x08, #0x88, #0x00, #0x00, #0x00
	.byte #0x33, #0x00, #0xC0, #0x0C, #0xCC, #0x00, #0x00, #0x00
	.byte #0x11, #0x00, #0xE0, #0x0E, #0xEE, #0x00, #0x00, #0x00
	
.export __gfx_hline4
__gfx_hline4:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	; case where w < 4
	ld l,(ix+6)
	ld h,(ix+7)
	ld bc,4
	or a
	sbc hl,bc
	jp c,hline4short
	; color = color AND 3 (downsamples reasonably well)
	ld a,(ix+8)
	and 3
	ld (ix+8),a
	call __gfx_xy4
	call __gfx_coff4
	jr z,hline4middle
	; draw left edge
	ld hl,__lin1tab         ; hl = mask data
	add hl,bc
	ld c,(hl)				; a = byte AND mask
	ld a,(de)
	and c
	ld c,(ix+8)             ; bc = color
	inc bc
	add hl,bc               ; hl = color data
	ld c,(hl)
	or c					; a = byte OR color
	ld (de),a				; write byte back
	inc de
	ld a,(ix+2)             ; w -= (4 - (x AND 3)) [inefficient, fix]
	and 3
	ld c,a
	ld a,4
	sub c
	ld c,a
	ld l,(ix+6)
	ld h,(ix+7)
	or a
	sbc hl,bc
	ld (ix+6),l
	ld (ix+7),h
	; draw middle
hline4middle:
	ld hl,__lin2tab         ; a = color byte
	ld c,(ix+8)
	add hl,bc
	ld a,(hl)
	ld b,(ix+6)             ; b = bytes left (w >> 2)
	ld c,(ix+7)
	srl c
    rr b
	srl c
    rr b
	jr z,hline4right
hline4loop:
	ld (de),a
	inc de
	djnz hline4loop
hline4right:
	; draw right edge
	ld a,(ix+6)
	and 3
	jp z,hline4done
	add a,a
	add a,a
	add a,a
	ld c,a
	ld hl,__lin3tab         ; hl = mask data
	add hl,bc
	ld c,(hl)				; a = byte AND mask
	ld a,(de)
	and c
	ld c,(ix+8)             ; bc = color
	inc bc
	add hl,bc               ; hl = color data
	ld c,(hl)
	or c					; a = byte OR color
	ld (de),a				; write byte back
	jr hline4done
hline4short: ; 1-3 pixel case - not very efficient currently
	ld l,(ix+8)
	ld h,(ix+9)
	push hl
	ld l,(ix+4)
	ld h,(ix+5)
	push hl
	ld l,(ix+2)
	ld h,(ix+3)
	push hl
	call __gfx_pix4
	pop hl
	pop hl
	pop hl
	ld ix,#0x0
	add ix,sp
	ld l,(ix+2)
	ld h,(ix+3)
	inc hl
	ld (ix+2),l
	ld (ix+3),h
	dec (ix+6)
	jr nz,hline4short
hline4done:
	pop ix
	pop bc
	ret

.export __gfx_hline16
__gfx_hline16:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	; case w < 2
	ld a,(ix+6)
	or a
	jr nz,hline16long
	ld a,(ix+7)
	cp 2
	jp c,hline16short
hline16long:
	call __gfx_xy16			; DE = write address
	ld c,(ix+8)				; C = color (low nibble)
	ld a,(ix+2)
	and 1
	jr z,hline16middle
	ld a,(de)
	and #0xF0
	or c
	ld (de),a
	inc de
	dec (ix+6)
	; draw middle
hline16middle:
	ld b,(ix+6)             ; b = bytes left (w >> 1)
	ld l,(ix+7)
	sra l
    rr b
	ld a,c					; A = color (in both nibbles)
	sla c
	sla c
	sla c
	sla c
	or c
hline16loop:
	ld (de),a
	inc de
	djnz hline16loop
; draw right edge (never more than 1 pixel by definition)
	ld a,(ix+6)
	and 1
	jr z,hline16done
	ld a,(de)
	and #0x0F
	or c					; C = color << 4
	ld (de),a
	jr hline16done
	; 1 pixel case
hline16short:
	ld l,(ix+8)
	ld h,(ix+9)
	push hl
	ld l,(ix+4)
	ld h,(ix+5)
	push hl
	ld l,(ix+2)
	ld h,(ix+3)
	push hl
	call __gfx_pix16
	pop hl
	pop hl
	pop hl
hline16done:
	pop ix
	pop bc
	ret
	
.export _Gfx_HLine
_Gfx_HLine:
	ld a,(__gfx_16)
	or a
	jp z,__gfx_hline4
	jp __gfx_hline16
