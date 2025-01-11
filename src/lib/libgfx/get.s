.z80
.code

; equivalent to ldir, but reads from HL shifted left by the number of pixels in IX+2 (CPC Mode 1).
; Uses IX+4 as an incrementor and IYL for carry.
ldir_gshift4cpc:
	ld a,(ix+2)
	or a
	jr z,gshift4cpc_fast
	ld (ix+4),c
gshift4cpc_loop1:
	ld a,(hl)
	ld b,(ix+2)
	inc hl
	; shift byte left by nibble
gshift4cpc_loop2:
	and #0x77
	sla a
	djnz gshift4cpc_loop2
	.byte #0xFD
	ld l,a
	; shift borrow byte right (4 - shift) bits by nibble
	ld b,(ix+2)
	ld a,4
	sub b
	ld b,a
	ld a,(hl)
gshift4cpc_loop3:
	and #0xEE
	srl a
	djnz gshift4cpc_loop3
	; OR byte with borrow byte
	.byte #0xFD
	ld c,l
	or c
	; done shifting, write
	ld (de),a
	inc de
	dec (ix+4)
	jr nz,gshift4cpc_loop1
	ret
	
gshift4cpc_fast:
	ldir
	ret
	
.export __gfx_get4
__gfx_get4:
	push bc
	push ix
	push iy
	ld ix,#0x08
	add ix,sp
	call __gfx_xy4  ; DE = read
	ld l,(ix+0)		; HL = write address
	ld h,(ix+1)
	; build header
	ld b,0			; BC = byte width (w >> 2)
	ld c,(ix+6)
	srl c
    srl c
	push bc
	ld (hl),c
	inc hl
	ld a,(ix+6)		; A = pixel width (w)
	ld (hl),a
	inc hl
	ld a,(ix+8)		; A = pixel height (h)
	ld (hl),a
	inc hl
	ex de,hl		; write = DE, read = HL, wbytes = BC
	; get loop parameters
	ld a,(ix+2)
	and 3
	ld (ix+2),a				; IX+2 = shift
	push hl
get4loop:
	call ldir_gshift4cpc
	dec (ix+8)
	jr z,get4done
	pop hl					; reset HL and move down one row
	ld bc,(__gfx_activebw)
	add hl,bc
	pop bc					; reset BC
	push bc
	push hl
	jr get4loop
get4done:
	pop de					; clean up stack
	pop de
	pop iy
	pop ix
	pop bc
	ret
	
; shifts the BC bytes prior to DE left by 1 pixel (16-color mode), preserving BC, DE, and HL.
shiftleft16:
	push bc
	push de
	push hl
	ld b,c
	ex de,hl
	ld a,(hl)
	dec hl
shiftleft16loop:
	rld						; huzzah for weird opcodes(!?)
	dec hl
	djnz shiftleft16loop
	pop hl
	pop de
	pop bc
	ret
	
.export __gfx_get16
__gfx_get16:
	push bc
	push ix
	push iy
	ld ix,#0x08
	add ix,sp
	call __gfx_xy16 ; DE = read
	ld l,(ix+0)		; HL = write address
	ld h,(ix+1)
	; build header
	ld b,0			; BC = byte width (w >> 1)
	ld c,(ix+6)
	srl c
	push bc
	ld (hl),c
	inc hl
	ld a,(ix+6)		; A = pixel width (w)
	ld (hl),a
	inc hl
	ld a,(ix+8)		; A = pixel height (h)
	ld (hl),a
	inc hl
	ex de,hl		; write = DE, read = HL, wbytes = BC
	; get loop parameters
	push hl
	ld a,(ix+2)
	and 1
	jr z,get16fastloop
get16loop:
	inc bc					; need one more byte per ldir for left shift
	ldir
	pop hl					; reset HL and move down one row
	ld bc,(__gfx_activebw)
	add hl,bc
	pop bc					; reset BC
	push bc
	push hl
	call shiftleft16
	dec de					; write one less byte per ldir for left shift
	dec (ix+8)
	jr nz,get16loop
	pop de					; clean up stack
	pop de
	pop iy
	pop ix
	pop bc
	ret
get16fastloop:
	ldir
	dec (ix+8)
	jr z,get16fastdone
	pop hl					; reset HL and move down one row
	ld bc,(__gfx_activebw)
	add hl,bc
	pop bc					; reset BC
	push bc
	push hl
	jr get16fastloop
get16fastdone:
	pop de					; clean up stack
	pop de
	pop iy
	pop ix
	pop bc
	ret
	
.export _Gfx_Get
_Gfx_Get:
	ld a,(__gfx_16)
	or a
	jp z,__gfx_get4
	jp __gfx_get16
	
