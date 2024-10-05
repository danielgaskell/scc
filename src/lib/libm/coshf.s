.z80
.code

.export _coshf
_coshf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32cosh
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32cosh
f32cosh:
;cosh(x) = (e^x+e^-x))/2
  call __pushpopf
  push bc
  ld bc,scrap+4
  call f32exp
  ld d,b
  ld e,c
  pop bc
  ld hl,f32_const_1
  call f32div
  ld h,b
  ld l,c
  jp f32amean
