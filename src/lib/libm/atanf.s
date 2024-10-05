.z80
.code

.export _atanf
_atanf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32atan
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32atan
f32atan:
;atan(x)
;Computed as x/BG(1.0,sqrt(1+x^2))
;

  call __pushpopf
  push bc


; copy x to f32bgi_c+4
  ld de,f32bgi_c+4
  call mov4

; compute x^2
  ld de,f32bgi_c+4
  ld h,d
  ld l,e
  ld bc,f32bgi_g
  call f32mul

; now x^2 + 1
  ld de,f32_const_1
  ld h,b
  ld l,c
  call f32add

; sqrt(1+x^2)
  call f32sqrt

; now DE points to 1.0, we need to compute 1/BG(1, sqrt(1+x^2))
  ex de,hl
  call f32bgi

  ld hl,f32bgi_c+4
  pop bc
  jp f32mul
 
mov4:
  ldi
  ldi
  ldi
  ldi
  ret
  