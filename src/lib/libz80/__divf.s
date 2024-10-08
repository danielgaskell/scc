.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export __divf
__divf:
	ld hl,2
	add hl,sp
	ex de,hl
	ld hl,6
	add hl,sp
	push bc
	ld bc,__floatd
	call f32div
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	pop bc
	ret

; uses stack and registers for all operations
; input: DE points to first operand
;        HL points to second operand
;        BC points to output location
.export f32div
f32div:
;x/y ==> z
  call __pushpopf
  push bc
  push de
  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld b,(hl)
  inc hl
  ld a,b
  add a,a
  ld a,(hl)
  adc a,a
  ;A is the exponent, carry is the sign
  pop hl
  jp z,f32div_0_op2
  inc a
  jp z,f32div_infnan_op2
  dec a
  push af
  push de



  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld c,(hl)
  inc hl
  ld a,c
  add a,a
  ld a,(hl)
  adc a,a
  ;A is the exponent, carry is the sign
  pop hl
  jp z,f32div_op1_0
  inc a
  jp z,f32div_op1_infnan
  push af

  set 7,c
  set 7,b
  ;CHL is the first operand, BDE is the second operand
  ; if CHL >= BDE then need to increment exponent and shift CHL>>1
  or a
  sbc hl,de
  ld a,b
  sbc a,c
  jr nc,jr3
  add hl,de
  adc a,c
jr3:
  ld b,a
  sbc a,a
  inc a
  ;A is 1 if we need to shift a 1 into the result of the division, else 0
  push af ;nc if we need to shift in a 1
  call div24_24
  pop hl

  rr h      ;shift out the bottom bit, shift round bit into H
  jr nc,jr4
  rr b
  rr d
  rr e
  pop af
  dec a
  .db #0x0E ;start of `ld c,*` to eat the next byte
jr4:
  pop af
  ld l,a
  rr h
  pop af
  rr h    ;top 2 bits of H are signs

  ;H.BDE, need to add carry
  bit 5,h
  jr z,jr5
  inc e
  jr nz,jr5
  inc d
  jr nz,jr5
  inc b
  jr nz,jr5
  dec l
jr5:

  sub l
  ld l,0
  jr nc,jr1
  dec l
jr1:
  add a,7Fh
  jr nc,jr2
  inc l
jr2:
  dec l
  jr z,f32div_ret_inf
  inc l
  jr nz,f32div_ret_zero

f32div_ret:
;BDE is the significand
  sla b
  ld l,a  ;exponent
  ld a,h
  and #0xC0
  jp pe,jp1
  scf
jp1:
  ld a,l
  rra
  rr b
  pop hl
  ld (hl),e
  inc hl
  ld (hl),d
  inc hl
  ld (hl),b
  inc hl
  ld (hl),a
  ret

f32div_0_op2:
; if op2 is 0 or NaN, return NaN, else 0
;if OP2 is 0, inf, or NaN, return NaN
  rr b       ;save the sign of OP1
  ld a,(hl)
  inc hl
  or (hl)
  inc hl
  ld c,(hl)
  sla c
  or c
  ld c,a    ;c is zero if the significand is zero
  ld a,(hl)
  add a,a
  inc hl
  ld a,(hl)
  adc a,a
  ld h,b
  jr z,f32div_ret_NaN
  rr h
  inc a
  ;If A is not zero, return 0
  ;If A is 0 and C is 0, then continue
  ;If A is 0 and C is not 0, NaN
  jr nz,f32div_ret_zero
  or c
  jr z,f32div_ret_zero
  jr f32div_ret_NaN2

f32div_infnan_op2:
;if OP2 is 0, inf, or NaN, return NaN
  rr c      ;save the sign of OP1
  inc hl
  inc hl
  ld a,(hl)
  add a,a
  inc hl
  ld a,(hl)
  adc a,a
  ld h,c
  jr z,f32div_ret_NaN
  inc a
  jr z,f32div_ret_NaN
  ;otherwise, return OP1, BDE ==> BDE
  rr h
  ld a,-1
  jr f32div_ret

f32div_op1_0:
  rr h
  pop af
f32div_ret_NaN:
  rr h
f32div_ret_NaN2:
  ld a,-1
  ld b,a  ;something > 1
  jr f32div_ret

f32div_op1_infnan:
;if the second op is NaN, return NaN, else 0
  rr h
  pop af
  rr h
  ld a,c
  add a,a
  or d
  or e
  jr z,f32div_ret_zero
  jr f32div_ret_NaN2

f32div_ret_inf:
  ld a,#0xFF
  .db #0xFE
f32div_ret_zero:
  xor a
  ld b,a
  jr f32div_ret

div24_24:
;BHL/CDE
;BHL<CDE
;return result as 0.BDE, with carry set to 1 if rounding should round up
;speed: 167+3*div24_24_sub_8 + div24_24_sub_1
;min: 1924
;max: 2499
;avg: 2167.75
; negate BDE
  xor a
  sub e
  ld e,a

  ld a,0
  sbc a,d
  ld d,a

  sbc a,a
  sub c
  ld c,a

;
  ld a,b
  call div24_24_sub_1 ; we need an extra iteration for rounding
  call div24_24_sub_8
  push bc
  call div24_24_sub_8
  push bc
  call div24_24_sub_8
  ;carry is for rounding
  pop de
  ld e,b
  pop bc
  ret


div24_24_sub_8:
; speed: 567+8*{8,{0,23}}
; min: 567
; max: 751
; avg: 645
  call div24_24_sub_4
div24_24_sub_4:
  call div24_24_sub_2
div24_24_sub_2:
  call div24_24_sub_1
div24_24_sub_1:
;56+{8,{0,23}}
  rl b
  add hl,hl
  adc a,a
  jr c,div24_24_sub_overflow
  add hl,de
  adc a,c
  ret c
  sbc hl,de
  sbc a,c
  ret
div24_24_sub_overflow:
  add hl,de
  adc a,c
  scf
  ret
 