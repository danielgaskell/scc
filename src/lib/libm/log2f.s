.z80
.code

.export _log2f
_log2f:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32log2
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32log2
f32log2:
;log2(x)
  call f32log
  push hl
  push de
  ld h,b
  ld l,c
  ld de,f32_const_lg_e
  call f32mul
  pop de
  pop hl
  ret