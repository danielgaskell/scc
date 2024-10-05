.z80
.code

.export _expf
_expf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32exp
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32exp
f32exp:
;  e^x = 2^(x*lg(e))
  push de
  ld de,f32_const_lg_e
  call f32mul
  pop de
  push hl
  ld h,b
  ld l,c
  call f32pow2
  pop hl
  ret
