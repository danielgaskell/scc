.z80
.code

; fast 8-bit multiplication: HL=H*E, clearing D and L
mul8:
	ld d,0
	ld l,d
	ld b,8
mul8loop:
	add hl,hl
	jp nc,mul8skip
	add hl,de
mul8skip:
	djnz mul8loop
	ret
	
.export __gfx_active     ; active canvas
__gfx_active:
	.word 0
.export __gfx_activedat  ; active graphics data
__gfx_activedat:
	.word 0
.export __gfx_activebw   ; active byte width (note that 2nd byte is always 0)
__gfx_activebw:
	.word 0
.export __gfx_activew    ; active pixel width
__gfx_activew:
	.word 0
.export __gfx_activeh    ; active pixel height (note that 2nd byte is always 0)
__gfx_activeh:
	.word 0
	
; given X = IX+2,3 and Y = IX+4, sets DE = address of affected byte (4-color mode).
.export __gfx_xy4
__gfx_xy4:
	ld h,(ix+4)             ; h = byte y offset
	ld a,(__gfx_activebw)   ; e = byte width of a row
	ld e,a
	call mul8               ; hl = y byte offset within canvas
	ld e,(ix+2)             ; de = byte x offset (x >> 2)
	ld d,(ix+3)
	srl d
    rr e
	srl d
    rr e
	add hl,de               ; hl = x/y byte offset within canvas
	ld de,(__gfx_activedat)
	add hl,de               ; de = address of byte to change
	ex de,hl
	ret
	
; given X = IX+2,3 and Y = IX+4, sets DE = address of affected byte (16-color mode).
.export __gfx_xy16
__gfx_xy16:
	ld h,(ix+4)             ; h = byte y offset
	ld a,(__gfx_activebw)   ; e = byte width of a row
	ld e,a
	call mul8               ; hl = y byte offset within canvas
	ld e,(ix+2)             ; de = byte x offset (x >> 1)
	ld d,(ix+3)
	srl d
    rr e
	add hl,de               ; hl = x/y byte offset within canvas
	ld de,(__gfx_activedat)
	add hl,de               ; de = address of byte to change
	ex de,hl
	ret
	
; given X = IX+2,3, sets BC = table lookup offset (shift << 3).
; On exit B = 0. Zero flag will be set if shift is zero.
.export __gfx_coff4
__gfx_coff4:
	ld a,(ix+2)				; bc = shift within byte << 3
	and 3
	add a,a
	add a,a
	add a,a
	ld b,0
	ld c,a
	ret

; logical blit operators
.export __gfx_blitop
__gfx_blitop:
	ld b,(ix+6)
	sra b
	ret c ; PUT_SET
	sra b
	jr c,blitopand
	sra b
	jr c,blitopor
blitopxor:
	ld b,a
	ld a,(de)
	xor b
	ret
blitopand:
	ld b,a
	ld a,(de)
	and b
	ret
blitopor:
	ld b,a
	ld a,(de)
	or b
	ret

.export __pixtab
__pixtab:
	.byte #0x77, #0x00, #0x80, #0x08, #0x88, #0x00, #0x00, #0x00
	.byte #0xBB, #0x00, #0x40, #0x04, #0x44, #0x00, #0x00, #0x00
	.byte #0xDD, #0x00, #0x20, #0x02, #0x22, #0x00, #0x00, #0x00
	.byte #0xEE, #0x00, #0x10, #0x01, #0x11, #0x00, #0x00, #0x00
