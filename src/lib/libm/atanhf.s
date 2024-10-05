.z80
.code

.export _atanhf
_atanhf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32atanh
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32atanh
f32atanh:
;atanh(x) = log((1+x)/(1-x))/2
;
  call __pushpopf
  push bc
  ld de,f32_const_1
  ld bc,f32bgi_g
  call f32rsub
  ld bc,f32bgi_a
  call f32add
  ld h,b
  ld l,c
  ld de,f32bgi_g
  call f32div
  pop bc
  call f32log
  ld h,b
  ld l,c

  inc hl
  inc hl
  ld a,(hl)
  add a,a
  inc hl
  ld a,(hl)
  adc a,a
  ret z
  inc a
  ret z
  dec a
  dec a
  rra
  ld (hl),a
  dec hl
  rl (hl)
  rrc (hl)
  ret
 