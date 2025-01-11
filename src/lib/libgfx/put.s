.z80
.code

shift4_mask:
	.byte #0x00, #0x88, #0xCC, #0xEE

; equivalent to ldir, but shifts by the number of pixels in IX+2 (CPC Mode 1).
; Uses IX+4 as an incrementor and IYL for carry.
ldir_shift4cpc:
	ld (ix+4),c
	ld a,(ix+2)
	or a
	jr z,shift4cpc0
	; load background into carry, depending on mode
	ld b,(ix+6)
	sra b
	jr c,shift4cpcset
	sra b
	jr c,shift4cpcand
shift4cpcor:
	; case PUT_OR or PUT_XOR, set to 0x00
	.byte #0xFD
	ld l,#0x00
	jr shift4cpcplot
shift4cpcand:
	; case PUT_AND, set to 0xFF and mask
	ld a,#0xFF
	push hl
	ld hl,shift4_mask
	ld b,0
	ld c,(ix+2)
	add hl,bc
	ld b,(hl)
	pop hl
	and b
	.byte #0xFD
	ld l,a
	jr shift4cpcplot
shift4cpcset:
	; case PUT_SET, set to background and mask
	ld a,(de)
	push hl
	ld hl,shift4_mask
	ld b,0
	ld c,(ix+2)
	add hl,bc
	ld b,(hl)
	pop hl
	and b
	.byte #0xFD
	ld l,a
shift4cpcplot:
	ld a,(ix+2)
	dec a
	jr z,shift4cpc1
	dec a
	jr z,shift4cpc2
shift4cpc3:
	ld a,(hl)
	and #0x77
	sla a
	ld c,a
	ld a,(hl)
	and #0x88
	srl a
	srl a
	srl a
	.byte #0xFD
	or l
	call __gfx_blitop
	ld (de),a
	.byte #0xFD
	ld l,c
	inc hl
	inc de
	dec (ix+4)
	jr nz,shift4cpc3
	jr shift4cpcdone
shift4cpc2:
	ld a,(hl)
	and #0x33
	sla a
	sla a
	ld c,a
	ld a,(hl)
	and #0xCC
	srl a
	srl a
	.byte #0xFD
	or l
	call __gfx_blitop
	ld (de),a
	.byte #0xFD
	ld l,c
	inc hl
	inc de
	dec (ix+4)
	jr nz,shift4cpc2
	jr shift4cpcdone
shift4cpc1:
	ld a,(hl)
	and #0x11
	sla a
	sla a
	sla a
	ld c,a
	ld a,(hl)
	and #0xEE
	srl a
	.byte #0xFD
	or l
	call __gfx_blitop
	ld (de),a
	.byte #0xFD
	ld l,c
	inc hl
	inc de
	dec (ix+4)
	jr nz,shift4cpc1
	jr shift4cpcdone
shift4cpc0:
	ld a,(hl)
	call __gfx_blitop
	ld (de),a
	inc hl
	inc de
	dec (ix+4)
	jr nz,shift4cpc0
	ret
shift4cpcdone:
	; merge remaining carry with background
	.byte #0xFD
	ld a,l
	call __gfx_blitop
	ld b,0
	ld c,(ix+2)
	push hl
	ld hl,shift4_mask
	add hl,bc
	ld b,(hl)
	pop hl
	and b
	ld c,a
	ld a,b
	cpl
	ld b,a
	ld a,(de)
	and b
	or c
	ld (de),a
	ret

.export __gfx_put4
__gfx_put4:
	push bc
	push ix
	push iy
	ld ix,#0x08
	add ix,sp
	call __gfx_xy4			; DE = write
	ld l,(ix+0)
	ld h,(ix+1)
	ld b,0
	ld c,(hl)
	push bc
	inc hl
	inc hl
	ld a,(hl)
	ld (ix+3),a				; IX+3 = rows
	inc hl
	push de
	ld a,(ix+2)
	and 3
	ld (ix+2),a				; IX+2 = shift
	jr nz,put4loop
	ld a,(ix+6)
	sra a
	jr nc,put4loop
put4fast:
	ldir
	dec (ix+3)
	jr z,put4done
	pop de					; reset DE and move down one row
	ex de,hl
	ld bc,(__gfx_activebw)
	add hl,bc
	ex de,hl
	pop bc					; reset BC
	push bc
	push de
	jr put4fast
put4loop:
	call ldir_shift4cpc
	dec (ix+3)
	jr z,put4done
	pop de					; reset DE and move down one row
	ex de,hl
	ld bc,(__gfx_activebw)
	add hl,bc
	ex de,hl
	pop bc					; reset BC
	push bc
	push de
	jr put4loop
put4done:
	pop de					; clean up stack
	pop de
	pop iy
	pop ix
	pop bc
	ret

; equivalent to ldir, but shifts by the number of pixels in IX+2 (16-color mode).
; Uses IX+4 as an incrementor and IYL for carry.
ldir_shift16:
	ld (ix+4),c
	ld a,(ix+2)
	or a
	jr z,shift16none
	; load background into carry, depending on mode
	ld b,(ix+6)
	sra b
	jr c,shift16set
	sra b
	jr c,shift16and
shift16or:
	; case PUT_OR or PUT_XOR, set to 0x00
	.byte #0xFD
	ld l,#0x00
	jr shift16plot
shift16and:
	; case PUT_AND, set to 0xF0
	.byte #0xFD
	ld l,#0xF0
	jr shift16plot
shift16set:
	; case PUT_SET, set to background and mask
	ld a,(de)
	and #0xF0
	.byte #0xFD
	ld l,a
shift16plot:
	ld a,(ix+2)
	and 1
	jr nz,shift16shifted
shift16none:
	ld a,(hl)
	call __gfx_blitop
	ld (de),a
	inc hl
	inc de
	dec (ix+4)
	jr nz,shift16none
	ret
shift16shifted:
	; FIXME this would be faster if I could figure out how to do it with RRD.
	ld a,(hl)
	and #0x0F
	sla a
	sla a
	sla a
	sla a
	ld c,a
	ld a,(hl)
	and #0xF0
	srl a
	srl a
	srl a
	srl a
	.byte #0xFD
	or l
	call __gfx_blitop
	ld (de),a
	.byte #0xFD
	ld l,c
	inc hl
	inc de
	dec (ix+4)
	jr nz,shift16shifted
shift16done:
	; merge remaining carry with background
	.byte #0xFD
	ld a,l
	call __gfx_blitop
	and #0xF0
	ld b,a
	ld a,(de)
	and #0x0F
	or b
	ld (de),a
	ret

.export __gfx_put16
__gfx_put16:
	push bc
	push ix
	push iy
	ld ix,#0x08
	add ix,sp
	call __gfx_xy16			; DE = write
	ld l,(ix+0)
	ld h,(ix+1)
	ld b,0
	ld c,(hl)
	push bc
	inc hl
	inc hl
	ld a,(hl)
	ld (ix+3),a				; IX+3 = rows
	inc hl
	push de
	ld a,(ix+2)
	and 1
	ld (ix+2),a				; IX+2 = shift
	jr nz,put16loop
	ld a,(ix+6)
	sra a
	jr nc,put16loop
put16fast:
	ldir
	dec (ix+3)
	jr z,put16done
	pop de					; reset DE and move down one row
	ex de,hl
	ld bc,(__gfx_activebw)
	add hl,bc
	ex de,hl
	pop bc					; reset BC
	push bc
	push de
	jr put16fast
put16loop:
	call ldir_shift16
	dec (ix+3)
	jr z,put16done
	pop de					; reset DE and move down one row
	ex de,hl
	ld bc,(__gfx_activebw)
	add hl,bc
	ex de,hl
	pop bc					; reset BC
	push bc
	push de
	jr put16loop
put16done:
	pop de					; clean up stack
	pop de
	pop iy
	pop ix
	pop bc
	ret
	
.export _Gfx_Put
_Gfx_Put:
	ld a,(__gfx_16)
	or a
	jp z,__gfx_put4
	jp __gfx_put16
