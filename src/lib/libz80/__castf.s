.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

; STILL NEEDED (FIXME):
; unsigned long _castf_ul(uint32_t a1)
; long _castf_l(uint32_t a1)

; IMPLEMENTED:
; unsigned _castf_u(uint32_t a1)
; unsigned char _castf_uc(uint32_t a1)
; int _castf_(uint32_t a1)
; signed char _castf_c(uint32_t a1)
; float _castf_f(uint32_t a1)

.export __castf_
__castf_:
;Inputs: SP+2 is an f32 float
;Outputs: HL is the signed 16-bit integer part of the input (rounded down)
;Special cases:
;   NaN              ==> 0
;   greater than 127 ==> 32767
;   less than -128   ==> -32768
  push de
  push bc
  push af
  
  ld hl,8
  add hl,sp
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
  
.export __castf_u
__castf_u:
;Inputs: SP+2 is an f32 float
;Outputs: HL is the unsigned 16-bit integer part of the input (rounded down)
;Special cases:
;   NaN                ==> 0
;   greater than 65535 ==> 65535
;   less than 0        ==> 0

  push de
  push bc
  push af
  ld hl,8
  add hl,sp
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

  add a,113
  jr c,f32tou16_return_carry
  add a,15
  jr nc,f32tou16_return_carry

  ld b,a
  inc b
  ex de,hl
  xor a
  ld e,a

djnz3:
  add hl,hl
  rl e
  rla
  djnz djnz3

  ld l,e
  ld h,a

  .byte #0x01 ; start of `ld bc,**` to skip the next two bytes
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

.export __castf_c
__castf_c:
;Inputs: SP+2 is an f32 float
;Outputs: L is the signed 8-bit integer part of the input (rounded down)
;Special cases:
;   NaN              ==> 0
;   greater than 127 ==> 127
;   less than -128   ==> -128

  push de
  push bc
  push af
  ld hl,10
  add hl,sp
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
  ld h,0
  ld l,a
  pop af
  pop bc
  pop de
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
djnz2:
  add hl,hl
  djnz djnz2
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
  
.export __castf_uc
__castf_uc:
;Inputs: SP+2 is an f32 float
;Outputs: L is the unsigned 8-bit integer part of the input (rounded down)
;Special cases:
;   NaN              ==> 0
;   greater than 255 ==> 255
;   less than 0      ==> 0

  push de
  push bc
  push af
  ld hl,8
  add hl,sp
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

  add a,121
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

  .byte #0xFE
f32tou8_return_carry:
  sbc a,a
f32tou8_return:
  ld h,0
  ld l,a
  pop af
  pop bc
  pop de
  ret

f32tou8_infnan:
  ld a,c
  add a,a
  or d
  or e
  sub 1
  jr f32tou8_return_carry
  