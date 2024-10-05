.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32pow2
f32pow2:
; if x is on [0,1):
;  2^x = 1.000000001752 + x * (0.693146989552 + x * (0.2402298085906 + x *
;  (5.54833215071e-2 + x * (9.67907584392e-3 + x * (1.243632065103e-3 + x *
;  2.171671843714e-4)))))
; Please note that usually I like to reduce to [-.5,.5] as the extra overhead is usually worth it.
; In this case, our polynomial is the same degree, with error different by less
; than 1 bit, so it's just a waste to range-reduce in this way.
  call __pushpopf
  push bc
  ld de,scrap
  ldi
  ldi
  ld a,(hl)
  add a,a
  ldi
  ld a,(hl)
  ld c,a
  res 7,a
  ld (de),a
  ld a,c
  adc a,a
  pop de
  ; carry is the sign, A is the exponent
  jp z,f32pow2_ret_1
  inc a
  jp z,f32pow2_ret_infnan
;A-1 is the exponent, carry is the sign
; if the exponent >= 7 and carry is set, return 0
; if the exponent >= 7 and carry is reset, return +inf
  rl b  ;save the carry
  cp 135
  jp nc,f32pow2_overflow
  cp 121
  jp c,f32pow2_ret_0
;

; now we need to extract the integer part from x while performing mod1 on it
; extract into C
  ld c,0
  sub 128
  jr c,int_exted
  push bc ; save the sign
  ld b,a
  ld hl,(scrap)
  ld a,(scrap+2)
  or #0x80
  inc b
int_ext_loop:
  add hl,hl
  adc a,a
  rl c
  djnz int_ext_loop
; Now normalize the significand
; make sure the significand isn't zero
  ld b,a
  or h
  or l
  jr z,int_ext_0
  ld a,b
; shift the significand up until there is a 1 in the top bit
  ld b,#0x7E  ; exponent of the fractional portion
  or a
  jr int_ext_norm
int_ext_norm_loop:
  dec b
  add hl,hl
  adc a,a
int_ext_norm:
  jp p,int_ext_norm_loop
int_ext_0:
  ld (scrap),hl
  add a,a
  srl b
  rra
  ld l,a
  ld h,b
  ld (scrap+2),hl
  pop af  ; LSB is sign
  ld b,a
int_exted:

; if the sign is negative, then perform 1-X and -c-1 ==> c
  rr b
  jr nc,jr1
  ld a,c
  cpl
  push af
  push de
  ld hl,f32_const_1
  ld de,scrap
  ld b,d
  ld c,e
  call f32sub
  pop de
  pop af
  ld c,a
jr1:
  ; sign-extend the exponent
  ld a,c
  add a,a
  sbc a,a
  ld b,a
  push bc ; save the exponent

;x is now on [0, 1)

;for 0 <= x <= 1
;2^x = 1 + x * (0.69314706325531005859375 + x * (0.240229070186614990234375 + x *
; (5.54862879216670989990234375e-2 + x * (9.67352092266082763671875e-3 + x *
; (1.248489017598330974578857421875e-3 + x * 2.15564403333701193332672119140625e-4)))))

; DE points to our output (accumulator)
  ld hl,f32pow2_a7
  push de
  ldi
  ldi
  ldi
  ld a,(hl)
  ld (de),a
  pop bc          ; points to accumulator
  ld de,scrap ; points to x

  ld hl,f32pow2_a6
  call f32horner_step
  ld hl,f32pow2_a5
  call f32horner_step
  ld hl,f32pow2_a4
  call f32horner_step
  ld hl,f32pow2_a3
  call f32horner_step
  ld hl,f32pow2_a2
  call f32horner_step
  ld hl,f32pow2_a1
  call f32horner_step
  ld h,b
  ld l,c
  pop bc
  ; need to add BC to the exponent
  inc hl
  inc hl
  ld a,(hl)
  add a,a
  ld (hl),a
  inc hl
  ld a,(hl)
  adc a,a
  add a,c
  jr nc,jr2
  or a  ; reset the carry flag if it is set
  inc b
jr2:
  ;if b is -1, then underflow
  ;elsif b is 0, then good
  ;else overflow
  inc b
  jr z,f32pow2_exp_overflow
  djnz f32pow2_exp_underflow
  .db #0x06 ; start of `ld b,*` to eat the next byte
f32pow2_exp_underflow:
  xor a
  rra
  ld (hl),a
  dec hl
  rr (hl)
  ret

f32pow2_exp_overflow:
  ld (hl),#0x7F
  dec hl
  ld (hl),#0x80
  xor a
  dec hl
  ld (hl),a
  dec hl
  ld (hl),a
  ret

f32pow2_overflow:
  rr b
  jr c,f32pow2_ret_0
f32pow2_ret_inf:
  ld hl,f32_const_inf
  jr f32pow2_ret
f32pow2_ret_0:
  ld hl,f32_const_0
  jr f32pow2_ret
f32pow2_ret_1:
;2^0 ==> 1
  ld hl,f32_const_1
f32pow2_ret:
  jp mov4

f32pow2_ret_infnan:
;2^inf ==> inf
;2^-inf ==> 0
;2^NaN ==> NaN

  ld hl,scrap
  ;if carry is reset, return OP1
  jp nc,mov4
  ;else if OP1 is NaN, return Nan
  ;else return 0
  ld a,(hl)
  ldi
  or (hl)
  ldi
  ld c,a
  ld a,(hl)
  add a,a
  or c
  ; if zero, return 0, else top bit needs to be set
  add a,255
  sbc a,a
  ld (de),a
  inc de
  ld (de),a
  ret

mov4:
  ldi
  ldi
  ldi
  ldi
  ret
  
f32pow2_a1:
 .db #0x00,#0x00,#0x80,#0x3F  ;1.000000001752
f32pow2_a2:
.db #0x16,#0x72,#0x31,#0x3F  ;0.69314706325531005859375
f32pow2_a3:
.db #0x9C,#0xFE,#0x75,#0x3E  ;0.240229070186614990234375
f32pow2_a4:
.db #0x97,#0x45,#0x63,#0x3D  ;5.54862879216670989990234375e-2
f32pow2_a5:
.db #0xB0,#0x7D,#0x1E,#0x3C  ;9.67352092266082763671875e-3
f32pow2_a6:
.db #0x57,#0xA4,#0xA3,#0x3A  ;1.248489017598330974578857421875e-3
f32pow2_a7:
.db #0x21,#0x09,#0x62,#0x39  ;2.15564403333701193332672119140625e-4
