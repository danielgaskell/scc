.z80
.code

__coltab:
	.byte #0x00, #0xF0, #0x0F, #0xFF
	.byte #0x00, #0xF0, #0x0F, #0xFF
	.byte #0x00, #0xF0, #0x0F, #0xFF
	.byte #0x00, #0xF0, #0x0F, #0xFF

.byte 1, 2, 3, 4, 1, 2, 3, 4

.export __gfx_text4
__gfx_text4:
	push bc
	push ix
	ld ix,#0x04
	add ix,sp
	ld hl,-22	; 22 bytes of storage (on stack for thread safety):
	add hl,sp	; IX-5,6 = working 16-bit field
	ld sp,hl	; IX-7 = current bit shift, IX-8 = font bank
				; IX-9 = saved font height, IX-10 = saved font first char
				; IX-26 = 16-byte cache for system font character
	; get system font address -> IX+10,11 if needed
	ld a,(__symbank)
	ld (ix-8),a			; assume font is in _symbank until proven otherwise
	ld a,(ix+10)
	or (ix+11)
	jp nz,skipfont
	push ix
	push iy
	ld e,8
	ld hl,#0x8103		; System_Information request 8
	rst #0x28
	pop iy
	pop ix
	ld (ix+10),e
	ld (ix+11),d
	ld a,0
	ld (ix-8),a			; system font is always in bank 0
skipfont:
	; cache font header to IX-9,10 and increment IX+10,11 past header
	ld l,(ix+10)
	ld h,(ix+11)
	ld a,(ix-8)
	rst #0x20
	.word #0x8124		; Banking_ReadWord
	ld (ix-9),c
	ld (ix-10),b
	ld (ix+10),l		; ReadWord advances HL +2
	ld (ix+11),h
	; convert color to colorfield -> IX+8
	ld d,0
	ld e,(ix+8)
	ld hl,__coltab
	add hl,de
	ld a,(hl)
	ld (ix+8),a
	call __gfx_xy4		; DE = write address
	; initial bit shift -> IX-7
	ld a,(ix+2)
	and 3
	ld (ix-7),a
text4loop:
	; HL = font data address, A = font data bank
	ld l,(ix+6)
	ld h,(ix+7)
	ld a,(hl)
	or a
	jp z,text4done
	sub (ix-10)			; offset char to first in font
	ld h,0
	ld l,a
	add hl,hl
	add hl,hl
	add hl,hl
	add hl,hl
	ld c,(ix+10)
	ld b,(ix+11)
	add hl,bc
	; set HL = actual character data address, caching system font if needed (when page == 0)
	ld a,(ix-8)
	or a
	call z,text4cache
	ld b,(hl)
	inc hl
	push de				; save write for later
	push bc				; save width for later
	ld b,(ix-9)			; height -> B
char4loop:
	push bc
	ld b,(hl)
	inc hl
	; shift B into 2 bytes -> IX-5,6
	ld c,0
	ld a,(ix-7)
	or a
	jp z,shift4done
shift4loop:
	srl b
	rr c
	dec a
	jr nz,shift4loop
shift4done:
	ld (ix-5),b
	ld (ix-6),c
	ex de,hl			; HL temporarily becomes write
text4nib1:
	; plot first nibble
	ld a,b
	and #0xF0
	jp z,text4nib2		; short-circuit if empty
	ld b,a
	rrca
	rrca
	rrca
	rrca
	or b
	call text4nibble
text4nib2:
	; plot second nibble
	inc hl
	ld a,(ix-5)
	and #0x0F
	jp z,text4nib3		; short-circuit if empty
	ld b,a
	rlca
	rlca
	rlca
	rlca
	or b
	call text4nibble
text4nib3:
	; plot third nibble
	inc hl
	ld a,(ix-6)
	or a				; no AND #0xF0 is needed because IX-6 should never have low bits.
	jp z,text4nextrow	; short-circuit if empty
	ld b,a
	rrca
	rrca
	rrca
	rrca
	or b
	call text4nibble
text4nextrow:
	; increment write by one row - 2
	ld bc,(__gfx_activebw)
	add hl,bc
	dec hl
	dec hl
	ex de,hl			; restore write to DE
	pop bc
	djnz char4loop
	; increment IX+6,7 (string pointer) and loop
	inc (ix+6)
	jp nz,text4nextchar
	inc (ix+7)
text4nextchar:
	; restore write and increment by width
	pop bc
	pop de
	ld a,(ix-7)
	add a,b
	cp 4
	jp c,text4noinc	; if shift < 4, no write increment
	inc de
	cp 8
	jp c,text4noinc
	inc de
text4noinc:
	and 3
	ld (ix-7),a
	jp text4loop
text4done:
	ld hl,22
	add hl,sp
	ld sp,hl
	pop ix
	pop bc
	ret
	
; takes: HL = write address, A = nibblemask, IX+8 = colorfield
; destroys: A, BC
text4nibble:
	ld b,a
	and (ix+8)			; colorfield &= nibblemask
	ld c,a
	ld a,b
	cpl					; background &= !nibblemask
	and (hl)
	or c				; background |= colorfield
	ld (hl),a			; write result
	ret
	
; caches the 16 bytes at HL, bank 0 -> IX-26 and sets HL to IX-26
; destroys AF, BC
text4cache:
	ld a,(__symbank)
	rlca
	rlca
	rlca
	rlca
	push de
	ex de,hl
	ld hl,-26
	push ix
	pop bc
	add hl,bc
	ex de,hl
	ld bc,16
	push de
	rst #0x20
	.word #0x8130		; Banking_Copy
	pop hl
	pop de
	ret
	
.export __gfx_text16
__gfx_text16:
	ret
	
.export _Gfx_Text
_Gfx_Text:
	ld a,(__gfx_16)
	or a
	jp z,__gfx_text4
	jp __gfx_text16
