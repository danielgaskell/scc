.z80
.code

.export _logf
_logf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32log
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32log
f32log:
;log(x) (natural logarithm)
;(x-1)/BG(.5(1+x), sqrt(x))
  call __pushpopf
  push bc

; copy to f32bgi_c+4, check special values
  ld de,f32bgi_c+4
  ldi
  ldi
  ld a,(hl)
  add a,a
  ldi
  ld a,(hl)
  ld (de),a
  adc a,a
  ;ln(-x) == NaN
  jr c,f32log_ret_NaN
  ;ln(0) == -inf
  jr z,f32log_ret_ninf
  inc a
  jr z,f32log_ret_x
; A-1 is the exponent. We'll hold onto this for later.
  push af
; now set the exponent to 0
  ex de,hl
  ld (hl),#0x3F
  dec hl
  set 7,(hl)

;(x-1)/BG(.5(1+x), sqrt(x))
  ld hl,f32bgi_c+4
  ld bc,f32bgi_g
  call f32sqrt
  ld de,f32_const_1
  ld bc,f32bgi_a
  call f32amean
  ld bc,f32bgi_c+4
  call f32sub

  ld hl,f32bgi_a
  ld de,f32bgi_g
  ld b,h
  ld c,l
  call f32bgi

  ld de,f32bgi_c+4
  call f32mul

  pop af
;(A-128)*log(2) + f32bgi_a
  sub 128
  ld hl,f32_const_ln2
  ld bc,f32bgi_g
  call f32_muli8
  ld h,b
  ld l,c
  ld de,f32bgi_a
  pop bc
  jp f32add

f32log_ret_NaN:
  .db $3E ; start of `ld a,*`
f32log_ret_ninf:
  xor a
  pop hl
  ld (hl),a
  inc hl
  ld (hl),a
  inc hl
  or #0x80
  ld (hl),a
  inc hl
  ld (hl),-1
  ret

f32log_ret_x:
  pop de
  ld hl,f32bgi_c+4
  jp mov4
  
mov4:
  ldi
  ldi
  ldi
  ldi
  ret
 