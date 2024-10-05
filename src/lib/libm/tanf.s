.z80
.code

.export _tanf
_tanf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32tan
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32tan
f32tan:
;tan(x)
  call __pushpopf
  push bc
  call f32cos
  ld bc,scrap+4
  call f32sin
  ld h,b
  ld l,c
  pop bc
  ld d,b
  ld e,c
  jp f32div
  