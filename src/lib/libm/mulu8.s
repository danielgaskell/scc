.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export mulu8_dp2
;
; This is a special-purpose routine for multiplying an f32 float (x):
;   x * D*2^-E
; where D is a signed 8-bit integer.
mulu8_dp2:
; HL points to the float
; D is the multiplier
; E is the (signed) power of 2 to multiply by.
  call __pushpopf
  push bc
  push de
  ld c,d

  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld a,(hl)
  add a,a
  ld b,a
  inc hl
  ld a,(hl)
  adc a,a
  jr z,mulu8_dp2_zero
  inc a
  jr z,mulu8_dp2_infnan
  ; check if we are multiplying by 0!
  inc c
  dec c
  jr z,mulu8_dp2_zero

  pop hl  ;
  push af

  ;extend the sign of L into H
  ld a,l
  add a,a
  sbc a,a
  ld h,a
  push hl

  ; B is shifted over by 2; we still need to shift a 1 back in
  scf
  rr b
  call C_Times_BDE  ;CAHL
  ld b,a
  pop de  ;DE is how much to add to the exponent
  pop af  ; A is the current exponent, carry is sign
  push af ; save carry

;CBHL
  ;need to add E+7 to the exponent, and shift CBHL up
  add a,e
  jr nc,jr1
  inc d
jr1:

  add a,7
  jr nc,jr2
  inc d
jr2:

  inc d
  jr z,mulu8_dp2_zero2
  dec d
  jr nz,mulu8_dp2_inf2

  ld e,a
  inc c
  dec c
  jr mulu8_dp2_adjust
mulu8_dp2_loop:
  dec de
  add hl,hl
  rl b
  rl c
mulu8_dp2_adjust:
  jp p,mulu8_dp2_loop
  sla c
  pop af
  ld a,e
  ld e,h
  ld d,b
  ld b,c
  .db #0x21 ;start of `ld hl,**` to eatt the next 2 bytes (dec a \ pop de)
mulu8_dp2_infnan:
  dec a
mulu8_dp2_zero:
  pop de
mulu8_dp2_return:
; sign is in carry, A is the exponent, (B/2)DE is the significand
  rra
  rr b
  pop hl
  ld (hl),e
  inc hl
  ld (hl),d
  inc hl
  ld (hl),b
  inc hl
  ld (hl),a
  ret
mulu8_dp2_inf2:
  ld a,-1
  .db #0xFE
mulu8_dp2_zero2:
  xor a
  ld de,0
  ld b,d
  jr mulu8_dp2_return
  