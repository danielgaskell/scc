.z80
.code

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
