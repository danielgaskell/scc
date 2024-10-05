.z80
.code

.export _tanhf
_tanhf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32tanh
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32tanh
f32tanh:
;tanh(x) = (e^(2x)-1)/(e^(2x)+1)
;        = 1-2/(e^(2x)+1)
;
  call __pushpopf
  push hl
  push de
;e^(2x)+1
  call f32mul2
  ld h,b
  ld l,c
  call f32exp
  ld de,f32_const_1
  call f32add

;1-2/ans
  ex de,hl
  call f32div
  ex de,hl
  call f32mul2
  jp f32rsub
  