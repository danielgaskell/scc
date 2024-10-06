.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

; IMPLEMENTED:
; int _castf_(uint32_t a1)
; unsigned _castf_u(uint32_t a1)
; signed char _castf_c(uint32_t a1)
; unsigned char _castf_uc(uint32_t a1)
; long _castf_l(uint32_t a1)
; unsigned long _castf_ul(uint32_t a1)

.export __castf_
__castf_:
	ld hl,2
	add hl,sp
	call f32toi16
	ret

.export __castf_u
__castf_u:
	ld hl,2
	add hl,sp
	call f32tou16
	ret
	
.export __castf_c
__castf_c:
	ld hl,2
	add hl,sp
	call f32toi8
	ld h,0
	ld l,a
	ret
	
.export __castf_uc
__castf_uc:
	ld hl,2
	add hl,sp
	call f32tou8
	ld h,0
	ld l,a
	ret
	
.export __castf_l
.export __castf_ul
__castf_l:
__castf_ul:
	ld ix,0
	add ix,sp
	ld l,(ix+2)
	ld h,(ix+3)
	ld e,(ix+4)
	ld d,(ix+5)
	call f32toi32
	ld (__hireg),de
	ret
	
.export f32toi16
f32toi16:
;Inputs: HL points to an f32 float
;Outputs: HL is the signed 16-bit integer part of the input (rounded down)
;Special cases:
;   NaN              ==> 0
;   greater than 127 ==> 32767
;   less than -128   ==> -32768
  push de
  push bc
  push af

  ld c,(hl)
  inc hl
  ld e,(hl)
  inc hl
  ld a,(hl)
  rlca
  scf
  rra
  ld d,a
  inc hl
  ld a,(hl)
  adc a,a
; carry flag is sign, DEC is the significand, A is the exponent
  call f32toi16_get_int
f32toi16_return:
  pop af
  pop bc
  pop de
  ret

f32toi16_infnan:
  ; if the exponent is 128, return 0 if NaN, else inf
  ld a,d
  add a,a
  or e
  or c
  ret nz
f32toi16_inf:
  ld hl,32767
  rr b
  ret nc
  inc hl
  ret

f32toi16_zero_ish:
  rr b
  ret nc
  ld a,d
  add a,a
  or e
  or c
  ret z
  dec hl
  ret

f32toi16_get_int:
  rl b    ; save the sign
  ld hl,0
  inc a
  jr z,f32toi16_infnan
  jp p,f32toi16_zero_ish
  sub 128
  cp 15
  jr nc,f32toi16_inf
  ex de,hl  ; significand is in HLC now, but we don't need to track C
  ;DE is 0
  cp 7
  jr c,jr1
  ;shift up by 8
  sub 8
  ld e,h
  ld h,l
  ld l,d    ; 0

jr1:
  ld d,b    ; save sign
  ld b,a
  xor a
  ;AE.HLC
  inc b
  jr z,jr2
djnz1:
  add hl,hl
  rl e
  rla
  djnz djnz1

jr2:
  ld b,d    ; save sign again
  ld d,a
  ex de,hl
  ; HL is the result, DEC has any fractional bits

  rrc b   ; if carry is reset, then we are done
  ret nc
  ; otherwise the number is negative, so if the fractional part is non-zero,
  ; need to round down

  xor a
  sub l
  ld l,a
  sbc a,a
  sub h
  ld h,a

  ld a,c
  or d
  or e
  ret z
  dec hl
  ret
  
.export f32tou16
f32tou16:
;Inputs: HL points to an f32 float
;Outputs: HL is the unsigned 16-bit integer part of the input (rounded down)
;Special cases:
;   NaN                ==> 0
;   greater than 65535 ==> 65535
;   less than 0        ==> 0

  push de
  push bc
  push af
  ld c,(hl)
  inc hl
  ld e,(hl)
  inc hl
  ld a,(hl)
  rlca
  scf
  rra
  ld d,a
  inc hl
  ld a,(hl)
  adc a,a
  ccf
  jr nc,f32tou16_return_carry
  or a
  jr z,f32tou16_return_carry
  inc a
  jr z,f32tou16_infnan

  add a,256-143
  jr c,f32tou16_return_carry
  add a,15
  jr nc,f32tou16_return_carry

  ld b,a
  inc b
  ex de,hl
  xor a
  ld e,a

djnz2:
  add hl,hl
  rl e
  rla
  djnz djnz2

  ld l,e
  ld h,a

  .db #0x01 ; start of `ld bc,**` to skip the next two bytes
f32tou16_return_carry:
  sbc hl,hl

f32tou16_return:
  pop af
  pop bc
  pop de
  ret
f32tou16_infnan:
  ld a,d
  add a,a
  or e
  or c
  sub 1
  jr f32tou16_return_carry
  
.export f32toi8
f32toi8:
;Inputs: HL points to an f32 float
;Outputs: A is the signed 8-bit integer part of the input (rounded down)
;Special cases:
;   NaN              ==> 0
;   greater than 127 ==> 127
;   less than -128   ==> -128

  push hl
  push de
  push bc
  ld a,(hl)
  inc hl
  or (hl)
  ld d,a    ; save the OR of the bottom two bytes of the significand
  inc hl
  ld a,(hl)
  rlca
  scf
  rra
  ld c,a
  inc hl
  ld a,(hl)
  adc a,a

  rr e    ; save the sign
; E has the sign, C is the portion of the significand that matters,
; A is the exponent

  call f32toi8_get_int

f32toi8_return:
  pop bc
  pop de
  pop hl
  ret

f32toi8_zero_ish:
  xor a
  rl e
  ret nc
  ld a,c
  add a,a
  or d
  add a,255
  sbc a,a
  ret

f32toi8_infnan:
  ; if the significand is non-zero, return NaN
  ld a,c
  add a,a
  or d
  sub 1
  sbc a,a
  ret z
f32toi8_inf:
  ld a,127
  rl e
  adc a,0
  ret

f32toi8_get_int:
  inc a
  jr z,f32toi8_infnan
  jp p,f32toi8_zero_ish
  sub 128
  cp 7
  jr nc,f32toi8_inf

  ld h,0
  ld l,c    ; upper 8 bits of the significand, H is 0
  ld b,a
  inc b
djnz3:
  add hl,hl
  djnz djnz3
  ld a,h
  rl e
  ret nc
  ld a,l
  or d
  add a,255
  sbc a,a
  sub h
  ld h,a
  ret

.export f32tou8
f32tou8:
;Inputs: HL points to an f32 float
;Outputs: A is the unsigned 8-bit integer part of the input (rounded down)
;Special cases:
;   NaN              ==> 0
;   greater than 255 ==> 255
;   less than 0      ==> 0

  push hl
  push de
  push bc
  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld a,(hl)
  rlca
  scf
  rra
  ld c,a
  inc hl
  ld a,(hl)
  adc a,a
  ccf
  jr nc,f32tou8_return_carry
  or a
  jr z,f32tou8_return_carry
  inc a
  jr z,f32tou8_infnan

  add a,256-135
  jr c,f32tou8_return_carry
  add a,7
  jr nc,f32tou8_return_carry

  ld b,a
  inc b
  xor a
djnz4:
  rl c
  rla
  djnz djnz4

  .db #0xFE
f32tou8_return_carry:
  sbc a,a
f32tou8_return:
  pop bc
  pop de
  pop hl
  ret

f32tou8_infnan:
  ld a,c
  add a,a
  or d
  or e
  sub 1
  jr f32tou8_return_carry

; Code below this point adapted from z88dk IEEE Floating Point Package
; (Clarified Artistic License)
f32toi32:
    ld b,d
    ld a,d                      ;Holds sign + 7bits of exponent
    rl e
    rla                         ;a = Exponent
    and a
    jp z,m32_fszero             ;exponent was 0, return 0
    cp #0x9E
    jp nc,m32_fsmax             ;number too large
    ; e register is rotated by bit, restore the hidden bit and rotate back
    scf
    rr  e
    ld d,e
    ld e,h
    ld h,l
    ld l,0
f32toi32loop:
    srl d                       ;fill with 0
    rr e
    rr h
	rr l
    inc a
    cp #0x9E
    jr nz,f32toi32loop
    rl b                        ;check sign bit
    call c,l_neg_dehl
    ret

; here to negate a number in dehl
m32_fsneg:
    ld a,d
    xor #0x80
    ld d,a
    ret

; here to return a legal zero of sign d in dehl
m32_fszero:
    ld a,d
    and #0x80
    ld d,a
    ld e,0
    ld h,e
    ld l,e
    ret

; here to change underflow to a error floating zero
m32_fsmin:
    call m32_fszero

m32_fseexit:
    scf                     ; C set for error
    ret

; here to change overflow to floating infinity of sign d in dehl
m32_fsmax:
    ld a,d
    or #0x7f                 ; max exponent
    ld d,a
    ld e,#0x80               ;floating infinity
    ld hl,0
    jr m32_fseexit

; here to change error to floating NaN of sign d in dehl
m32_fsnan:
    ld a,d
    or #0x7f                 ; max exponent
    ld d,a
    ld e,#0xff               ;floating NaN
    ld h,e
    ld l,e
    jr m32_fseexit

l_neg_dehl:
   ; negate dehl
   ; enter : dehl = long
   ; exit  : dehl = -long
   ; uses  : af, de, hl, carry unaffected
   ld a,l
   cpl
   ld l,a
   ld a,h
   cpl
   ld h,a
   ld a,e
   cpl
   ld e,a
   ld a,d
   cpl
   ld d,a
   inc l
   ret nz
   inc h
   ret nz
   inc de
   ret
