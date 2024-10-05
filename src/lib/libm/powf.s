.z80
.code

.export _powf
_powf:
	ld hl,6
	add hl,sp
	ex de,hl
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32pow
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32pow
f32pow:
; x^y = 2^(y*lg(x))
  push hl
  call f32log2
  ld h,b
  ld l,c
  call f32mul
  call f32pow2
  pop hl
  ret
  