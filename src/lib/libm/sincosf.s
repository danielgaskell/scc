.z80
.code

.export _cosf
_cosf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32cos
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

.export _sinf
_sinf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32sin
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32sin
f32sin:
;sin(x) = cos(pi/2 - x)
  call __pushpopf
  ld de,f32_const_pi_div_2
  call f32rsub
  ld h,b
  ld l,c
  jp f32cos_nopushpop

.export f32sin_range_reduced
f32sin_range_reduced:
;multiply by 2pi
  ld de,f32_const_2pi
  call f32mul

;sine(-pi/4<=x<pi/4)
;y=x*x
;a1=2^-3 * 11184804/2^23
;a2=2^-7 * 8946604/2^23
;a3=2^-13 * 13408017/2^23
;x(1-y(a1-y(a2-y*a3)))

  ld bc,scrap+4
  ld d,h
  ld e,l
;-x^2 ==> y
  call f32mul
  ld hl,scrap+4+3
  set 7,(hl)

  pop de
  ld hl,f32sin_a3
  ldi
  ldi
  ldi
  ld a,(hl)
  ld (de),a
  ld b,d
  ld c,e
  dec bc
  dec bc
  dec bc
  ld de,scrap+4
  ld hl,f32sin_a2
  call f32horner_step
  ld hl,f32sin_a1
  call f32horner_step
  ld hl,f32_const_1
  call f32horner_step
  ld hl,scrap
  ld d,b
  ld e,c
  jp f32mul

f32sin_a3:
.db #0x11,#0x97,#0x4C,#0x39  ;1.951123268e-4
f32sin_a2:
.db #0xAC,#0x83,#0x08,#0x3C  ;.0083321742713
f32sin_a1:
.db #0xA4,#0xAA,#0x2A,#0x3E  ;.1666665673

.export f32cos
f32cos:
;cos(x)
  call __pushpopf
.export f32cos_nopushpop
f32cos_nopushpop:
  push bc
  ld de,scrap
  ldi
  ldi
  ld a,(hl)
  add a,a
  ldi
  ld a,(hl)
; we can drop the sign since cos(-x) = cos(x)
  adc a,a
  rra
  ld (de),a
  jp z,f32cos_ret_1
  adc a,a
  inc a
  jp z,f32cos_ret_NaN

  ld hl,scrap
  ld b,h
  ld c,l
  ld de,f32_const_2pi_inv
  call f32mul

; Add .5
  ld de,f32_const_p5
  call f32add

;Now grab mod 1
  call f32mod1

f32cos_stepin:
;subtract off the .5
  ld de,f32_const_p5
  call f32sub

;  cos(-x)=cos(x)
  ld hl,scrap+2
  ld a,(hl)
  add a,a
  inc hl
  res 7,(hl)
  ld a,(hl)
  adc a,a   ; A is a copy of the exponent
  jr z,f32cos_ret_1

;  cos(x-pi)=-cos(x)
;   if our x is now on [.25,.5], then subtract x from .5 absolute value,
;   and return the negative result.
  pop bc
  cp #0x7D
  jr c,jr1
  ; do (.5 - x)
  ; ld de,f32_const_p5
  push bc
  ld bc,scrap
  ld h,b
  ld l,c
  call f32rsub
  ld hl,scrap+2
  ld a,(hl)
  add a,a
  inc hl
  ld a,(hl)
  adc a,a   ; A is a copy of the exponent
  pop bc
  call jr1
  ld h,b
  ld l,c
  jp f32neg
jr1:
  push bc
  ld bc,scrap

;  cos(pi/2-x)=sin(x)
;    if our x is now on [.125,.25], subtract it from .25 and feed it to the sine routine.
  cp #0x7C
  jr c,f32cos_range_reduced
  ;(.25-x)
  ld hl,scrap
  ld de,f32_const_p25
  call f32rsub
  jp f32sin_range_reduced

f32cos_range_reduced:
;multiply by 2pi
  ld hl,scrap
  ld de,f32_const_2pi
  call f32mul

;cos(-pi/4<=x<pi/4)
;y=x*x
;1-y(.49999887-y(.041655882-y.0013591743))
;1-y(a1-y(a2-y*a3))

  ld bc,scrap+4
  ld d,h
  ld e,l
;-x^2 ==> y
  call f32mul
  ld hl,scrap+4+3
  set 7,(hl)

  pop de
  ld hl,f32cos_a3
  ldi
  ldi
  ldi
  ld a,(hl)
  ld (de),a

  ld b,d
  ld c,e
  dec bc
  dec bc
  dec bc
  ld de,scrap+4
  ld hl,f32cos_a2
  call f32horner_step
  ld hl,f32cos_a1
  call f32horner_step
  ld hl,f32_const_1
  jp f32horner_step

f32cos_ret_NaN:
  ld hl,f32_const_NaN
  jr f32cos_ret
f32cos_ret_1:
  ld hl,f32_const_1
f32cos_ret:
  pop de
  jp mov4

f32cos_a3:
.db #0x52,#0x26,#0xB2,#0x3A  ;.0013591743
f32cos_a2:
.db #0x5C,#0x9F,#0x2A,#0x3D  ;.041655882
f32cos_a1:
.db #0xDA,#0xFF,#0xFF,#0x3E  ;.49999887

mov4:
  ldi
  ldi
  ldi
  ldi
  ret
  