.z80
.code

.export _acosf
_acosf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32acos
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32acos
f32acos:
;sqrt(1-x*x)*bgi(x,1)
  call __pushpopf
  push bc


; copy x to f32bgi_a
  ld de,f32bgi_a
  call mov4

; compute x^2
  ld de,f32bgi_a
  ld h,d
  ld l,e
  ld bc,f32bgi_c+4
  call f32mul

; now 1-x^2
  ld de,f32_const_1
  ld h,b
  ld l,c
  call f32rsub

; sqrt(1-x^2)
  call f32sqrt

; now DE points to 1.0, we need to compute 1/BG(x, 1)
  ld hl,f32bgi_a
  ld b,h
  ld c,l
  call f32bgi

  ld de,f32bgi_c+4
  pop bc
  jp f32mul

mov4:
  ldi
  ldi
  ldi
  ldi
  ret
  