.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32_muli8
;
; This is a special-purpose routine for multiplying an f32 float (x):
;   x * D*2^-E
; where D is a signed 8-bit integer.
f32_muli8:
; HL points to the float
; A is the (signed) multiplier
; BC is the output
  call __pushpopf
  push bc
  ld c,a    ;multiplier
  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld a,(hl)
  ld b,a
  add a,a
  inc hl
  ld a,(hl)
  adc a,a
  jr z,f32_muli8_ret_op1
  inc a
  jr z,f32_muli8_ret_op1
  rl h  ; bottom bit of h has the sign
; if the multiplier is 0, return 0
  inc c
  dec c
  jr z,f32_muli8_ret_0

; if C is negative, need to toggle sign
  jp p,f32_muli8_sign_set
  inc h ; toggle the bottom bit of h (sign)
  ld l,a
  xor a ; negate C
  sub c
  ld c,a
  ld a,l
f32_muli8_sign_set:
  push hl
  push af ; save the exponent of the output
  set 7,b
  call C_Times_BDE  ;CAHL
  ld b,a
  pop af
  ;CBHL is significand, A+7 is the exponent
  ; we'll add an extra 1 to make it easier to detect overflow later
  add a,8
  ld e,a
  ; move carry into low bit of D
  rl d

f32_muli8_significand_loop:
  dec de
  add hl,hl
  rl b
  rl c
  jp p,f32_muli8_significand_loop
; the low 9 bits of DE is the exponent+1
; CBH is the adjusted significand, need to round
  sla l
  jr nc,jr1
  inc h
  jr nz,jr1
  inc b
  jr nz,jr1
  inc c
  jr nz,jr1
  inc de
jr1:
; if LSB of D holds any overflow, return inf
  rr d
  jr c,f32_muli8_ret_inf
; adjust E
  dec e
  sla c
  pop af  ; low bit is sign
  rra
  rr e
  rr c

  ; ECBH
  ld d,b
  ld b,c
  ld a,e
  ld e,h
  jr f32_muli8_ret_op1

f32_muli8_ret_inf:
  pop af
  rrca
  ld de,0
  ld b,#0x80
  or #0x7F
  .db #0x21 ; start of `ld hl,**` to eat the next two bytes
f32_muli8_ret_0:
  xor a
  ld b,a
f32_muli8_ret_op1:
  pop hl
  ld (hl),e
  inc hl
  ld (hl),d
  inc hl
  ld (hl),b
  inc hl
  ld (hl),a
  ret
 