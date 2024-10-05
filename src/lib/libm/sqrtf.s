.z80
.code

.export _sqrtf
_sqrtf:
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32sqrt
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32sqrt
f32sqrt:
;sqrt(x) ==> z

;return NaN if the input is negative, except:
;"IEEE 754 defines sqrt(-0.) as -0."
;   - https://stackoverflow.com/a/19232238/3303651

  call __pushpopf
  push bc
  ld b,(hl)
  inc hl
  ld e,(hl)
  inc hl
  ld d,(hl)
  ld a,d
  add a,a
  inc hl
  ld a,(hl)
  adc a,a
  jr c,f32sqrt_neg
  jr z,f32sqrt_zero
  inc a
  jr z,f32sqrt_infnan

;Now adjust the significand for our square-root routine
  set 7,d

;if the exponent is odd, need to shift right again
;also need to compute the new exponent as (A-1)>>1 + 0x3E
  ld c,0
  rra
  jr c,jr1
  srl d
  rr e
  rr b
  rr c
  ; dec a
jr1:
  add a,#0x3F

;save the exponent
  push af

  call sqrt24_mant

;DEL is the significand
  ld b,l
  pop af
  sla d
  srl a
  rr d
f32sqrt_zero:
  pop hl
  ld (hl),b
  inc hl
  ld (hl),e
  inc hl
  ld (hl),d
  inc hl
  ld (hl),a
  ret
f32sqrt_neg:
  jr z,f32sqrt_neg0
  ld a,-1
f32sqrt_neg0:
  ; either writing a
  pop hl
  ld (hl),a
  inc hl
  ld (hl),a
  inc hl
  ld (hl),a
  inc hl
  ;If A is FF, then upper bit is fine either way as we wrote NaN, but if it is
  ;00, we need to set the top bit to 1 so that sqrt(-0) => -0
  xor #0x80
  ld (hl),a
  ret

f32sqrt_infnan:
  pop hl
  ;If inf, then the significand is 0, else it is NaN
  ld a,d
  and #0x7F ;mask out the exponent bit
  or e
  or b
;If A is 0, then write +inf (significand is 0), else write non-zero for NaN
  ld (hl),a
  inc hl
  ld (hl),a
  inc hl
  or #0x80
  ld (hl),a
  inc hl
  ld (hl), #0x7F
  ret

sqrt24_mant:
;DEB.C is the 24-bit mantissa with C as any extra bits used for rounding.
;Output: DEL
  push bc
  pop ix
  ex de,hl
;HLIX
  call sqrtHLIX
;AHL is the new remainder
;Need to divide by 2, then divide by DE
  rra
  ld a,h
;HL/DE to 8 bits
;We are just going to approximate it
  res 0,l
  jr c,jr2
  cp d
  jr c,jr3
jr2:
  sub d
  inc l
jr3:
  sla l
  rla
  jr c,jr4
  cp d
  jr c,jr5
jr4:
  sub d
  inc l
jr5:
  sla l
  rla
  jr c,jr6
  cp d
  jr c,jr7
jr6:
  sub d
  inc l
jr7:
  sla l
  rla
  jr c,jr8
  cp d
  jr c,jr9
jr8:
  sub d
  inc l
jr9:
  sla l
  rla
  jr c,jr10
  cp d
  jr c,jr11
jr10:
  sub d
  inc l
jr11:
  sla l
  rla
  jr c,jr12
  cp d
  jr c,jr13
jr12:
  sub d
  inc l
jr13:
  sla l
  rla
  jr c,jr14
  cp d
  jr c,jr15
jr14:
  sub d
  inc l
jr15:
  sla l
  rla
  jr c,jr16
  cp d
  ret c   
  sub d
jr16:
  inc l
  ret
  
sqrtHLIX:
;Input: HLIX
;Output: DE is the sqrt, AHL is the remainder
;speed: 751+6{0,6}+{0,3+{0,18}}+{0,38}+sqrtHL
;min: 1103
;max: 1237
;avg: 1165.5
;166 bytes

  call sqrtHL   ;expects returns A as sqrt, HL as remainder, D = 0
  add a,a
  ld e,a
  rl d

  .byte #0xDD
  ld a,h
  sla e
  set 0,e
  rl d
  add a,a
  adc hl,hl
  add a,a
  adc hl,hl
  sbc hl,de
  jr nc,ur1
  add hl,de
  dec e
  .db #0xFE     ;start of `cp *`
ur1:
  inc e

  sla e
  set 0,e
  rl d
  add a,a
  adc hl,hl
  add a,a
  adc hl,hl
  sbc hl,de
  jr nc,ur2
  add hl,de
  dec e
  .db #0xFE     ;start of `cp *`
ur2:
  inc e

  sla e
  set 0,e
  rl d
  add a,a
  adc hl,hl
  add a,a
  adc hl,hl
  sbc hl,de
  jr nc,ur3
  add hl,de
  dec e
  .db #0xFE     ;start of `cp *`
ur3:
  inc e

  sla e
  set 0,e
  rl d
  add a,a
  adc hl,hl
  add a,a
  adc hl,hl
  sbc hl,de
  jr nc,ur4
  add hl,de
  dec e
  .db #0xFE     ;start of `cp *`
ur4:
  inc e

;Now we have four more iterations
;The first two are no problem
  .byte #0xDD
  ld a,l
  sla e
  set 0,e
  rl d
  add a,a
  adc hl,hl
  add a,a
  adc hl,hl
  sbc hl,de
  jr nc,ur5
  add hl,de
  dec e
  .db #0xFE     ;start of `cp *`
ur5:
  inc e

  sla e
  set 0,e
  rl d
  add a,a
  adc hl,hl
  add a,a
  adc hl,hl
  sbc hl,de
  jr nc,ur6
  add hl,de
  dec e
  .db #0xFE     ;start of `cp *`
ur6:
  inc e

sqrt32_iter15:
;On the next iteration, HL might temporarily overflow by 1 bit
  sla e
  set 0,e
  rl d      ;sla e
  rl d
  inc e
  add a,a
  adc hl,hl
  add a,a
  adc hl,hl       ;This might overflow!
  jr c,sqrt32_iter15_br0
;
  sbc hl,de
  jr nc,ur7
  add hl,de
  dec e
  jr sqrt32_iter16
sqrt32_iter15_br0:
  or a
  sbc hl,de
ur7:
  inc e

;On the next iteration, HL is allowed to overflow, DE could overflow with our current routine,
;but it needs to be shifted right at the end, anyways
sqrt32_iter16:
  add a,a
  ld b,a        ;either 0x00 or 0x80
  adc hl,hl
  rla
  adc hl,hl
  rla
;AHL - (DE+DE+1)
  sbc hl,de
  sbc a,b
  inc e
  or a
  sbc hl,de
  sbc a,b
  ret p
  add hl,de
  adc a,b
  dec e
  add hl,de
  adc a,b
  ret
  
sqrtHL:
;returns A as the sqrt, HL as the remainder, D = 0
;min: 352cc
;max: 391cc
;avg: 371.5cc


  ld de,#0x5040  ; 10
  ld a,h        ; 4
  sub e         ; 4
  jr nc,sq7     ;\
  add a,e       ; | branch 1: 12cc
  ld d,16       ; | branch 2: 18cc
sq7:            ;/

; ----------

  cp d          ; 4
  jr c,sq6      ;\
  sub d         ; | branch 1: 12cc
  set 5,d       ; | branch 2: 19cc
sq6:            ;/

; ----------
  res 4,d       ; 8
  srl d         ; 8
  set 2,d       ; 8
  cp d          ; 4
  jr c,sq5      ;\
  sub d         ; | branch 1: 12cc
  set 3,d       ; | branch 2: 19cc
sq5:            ;/
  srl d         ; 8

; ----------

  inc a         ; 4
  sub d         ; 4
  jr nc,sq4     ;\
  dec d         ; | branch 1: 12cc
  add a,d       ; | branch 2: 19cc
  dec d         ; | <-- this resets the low bit of D, so `srl d` resets carry.
sq4:            ;/
  srl d         ; 8
  ld h,a        ; 4

; ----------

  ld a,e        ; 4
  sbc hl,de     ; 15
  jr nc,sq3     ;\
  add hl,de     ; | 12cc or 18cc
sq3:            ;/
  ccf           ; 4
  rra           ; 4
  srl d         ; 8
  rra           ; 4

; ----------

  ld e,a        ; 4
  sbc hl,de     ; 15
  jr c,sq2      ;\
  or #0x20      ; | branch 1: 23cc
  .db 254        ; |   <-- start of `cp *` which is 7cc to skip the next byte.
sq2:            ; | branch 2: 21cc
  add hl,de     ;/

  xor #0x18     ; 7
  srl d         ; 8
  rra           ; 4

; ----------

  ld e,a        ; 4
  sbc hl,de     ; 15
  jr c,sq1      ;\
  or 8          ; | branch 1: 23cc
  .db 254        ; |   <-- start of `cp *` which is 7cc to skip the next byte.
sq1:            ; | branch 2: 21cc
  add hl,de     ;/

  xor 6         ; 7
  srl d         ; 8
  rra           ; 4

; ----------

  ld e,a        ; 4
  sbc hl,de     ; 15
  jr nc,ur8     ;    \
  add hl,de     ; 15  |
  srl d         ; 8   |
  rra           ; 4   | branch 1: 38cc
  ret           ; 10  | branch 2: 40cc
ur8:            ;     |
  inc a         ; 4   |
  srl d         ; 8   |
  rra           ; 4   |
  ret           ; 10 /
  
