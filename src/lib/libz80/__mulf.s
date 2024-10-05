.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export __mulf
__mulf:
	ld hl,6
	add hl,sp
	ex de,hl
	ld hl,2
	add hl,sp
	ld bc,__floatd
	call f32mul
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	ret

; uses stack and registers for all operations
; input: DE points to first operand
;        HL points to second operand
;        BC points to output location
.export f32mul
f32mul:
;x * y ==> z
;
  call __pushpopf
  push bc
  push de
  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld b,(hl)
  ld a,b
  add a,a
  inc hl
  ld a,(hl)
  ld c,a
  adc a,a
  pop hl
  jp z,f32mul_0_op2
  inc a
  jp z,f32mul_infnan_op2
  dec a
  push af   ;exponent of the first operand

  push de
  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld c,(hl)
  ld a,c
  add a,a
  inc hl
  ld a,(hl)
  adc a,a
  jr z,f32mul_op1_0
  inc a
  jr z,f32mul_op1_infnan
  pop hl
  ex de,hl
  push af   ;exponent of the second operand
;BDE*CHL
  set 7,b
  set 7,c

  call mul24
;BHLDEA
  pop af
  rr e
  bit 7,b
  jr nz,jr3
  dec a
  sla d
  adc hl,hl
  rl b
jr3:

  sla d
  ex de,hl
  jr nc,jr4
  inc e
  jr nz,jr4
  inc d
  jr nz,jr4
  inc b
  jr nz,jr4
  inc a
jr4:
  ld h,a  ; first exponent
  pop af  ; A is the second exponent
  rr l    ; top 2 bits are the signs
  ; ultimately need H+A-0x7F
  add a,h
  ld h,0
  rl h
  sub #0x7E
  dec a
  jr nc,jr1
  dec h

jr1:
  ; If H is 1, we have an inf, -1 we have 0
  dec h
  jp z,f32mul_inf_l
  inc h
  jr z,jr2
  xor a

jr2:
  sla b

  ld c,a  ;the exponent
  ld a,l
  and #0xC0
  jp pe,f32mul_return2
  scf

f32mul_return2:
  rr c
  rr b

f32mul_return:
  pop hl
  ld (hl),e
  inc hl
  ld (hl),d
  inc hl
  ld (hl),b
  inc hl
  ld (hl),c
  ret

f32mul_op1_0:
  pop bc
  rra
  ld c,a
  pop af
  rra
  xor c
  and #0x80
  ld c,a
  ld b,0
  jr f32mul_return

f32mul_op1_infnan:
  pop hl
  rra
  ld b,a
  pop af
  rra
  xor b ;top bit is sign
  ld h,a

  ld a,c
  add a,a
  or d
  or e
  ld b,a
  add hl,hl
  ld c,-1
  jr f32mul_return2

f32mul_0_op2:
;if OP2 is inf or NaN, retun NaN, else return 0
  rra
  ld b,a     ;save the sign
  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld a,(hl)
  add a,a
  ld c,a
  inc hl
  ld a,(hl)
  ld h,a
  adc a,a
  inc a
  jr z,f32mul_return_NaN
  rra
  xor b
  and #0x80
  ld c,a
  ld b,0
  jr f32mul_return
f32mul_return_NaN:
  ld bc,#0xFF7F
  jr f32mul_return

f32mul_infnan_op2:
;if OP2 is 0 or NaN, return NaN, else return OP1 with adjusted sign
  rra
  ld c,a  ;save the sign
  ld a,b
  add a,a
  or d
  or e
  jr nz,f32mul_return_NaN
f32mul_inf_op2:
  inc hl
  inc hl
  ld a,(hl)
  inc hl
  add a,a
  ld a,(hl)
  adc a,a
  jr z,f32mul_return_NaN
  inc a
  jr z,f32mul_check_NaN
  rra
  xor c
  and #0x80
  or #0x7F
  ld c,a
  jr f32mul_return
f32mul_check_NaN:
  rra
  xor c
  and #0x80
  or #0x7F
  ld c,a
  dec hl
  ld a,(hl)
  add a,a
  dec hl
  or (hl)
  dec hl
  or (hl)
  jr nz,f32mul_return_NaN
  jr f32mul_return



f32mul_inf_l:
  ld a,l
  and #0xC0
  jp pe,f32mul_inf
  scf
f32mul_inf:
;carry is sign
  pop hl
  ld (hl),0
  inc hl
  ld (hl),0
  inc hl
  ld (hl),#0x80
  inc hl
  ld a,-1
  rra
  ld (hl),a
  ret

mul24:
;BDE*CHL ->  BHLDEA
;510+{0,39}+{0,8}+3*C_Times_BDE
;61 bytes
    push hl
    pop ix
    call C_Times_BDE  ; CAHL
    push hl
    ld l,a
    ld h,c
    ex (sp),hl
    push hl

    ld a,b
    push ix
    pop bc
    ld b,a
    call C_Times_BDE  ; CAHL
    ;CA + stack without destroying BDE
    ex (sp),hl
    add a,l
    ld l,a
    ld a,c
    adc a,h
    ld h,a
 ;   {1}HL{2}
    ld a,b
    push ix
    pop bc
    ld c,b
    ld b,a
    pop ix
  ;  IXHL{1}
    jr nc,jr5
    ex (sp),hl
    inc hl
    ex (sp),hl
jr5:
    push hl
    push ix
;    {3}{2}{1}

    call C_Times_BDE
    dec sp
    ;CAHL
    pop de    ;D is the bottom byte
    ld b,d    ;B is the bottom byte
    pop de
    add hl,de ;HL is bytes 1 and 2
    ld d,c
    ld e,a
    ex (sp),hl
    adc hl,de ;HL is bytes 3 and 4
    pop de    ;DE is bytes 1 and 2
    ld a,b    ;A is the bottom byte
    dec sp
    pop bc    ;B is the top byte
    ret nc
    inc b
;    BHLDEA
    ret
	
.export C_Times_BDE
C_Times_BDE:
;C*BDE => CAHL
;C>=128    155+6*(20+{0,21+{0,1}}) + {0,20+{0,8}}
;C>=64     155+5*(20+{0,21+{0,1}}) + {0,20+{0,8}}
;C>=32     155+4*(20+{0,21+{0,1}}) + {0,20+{0,8}}
;C>=16     155+3*(20+{0,21+{0,1}}) + {0,20+{0,8}}
;C>=8      155+2*(20+{0,21+{0,1}}) + {0,20+{0,8}}
;C>=4      155+1*(20+{0,21+{0,1}}) + {0,20+{0,8}}
;C>=2      155+0*(20+{0,21+{0,1}}) + {0,20+{0,8}}
;C==1      136
;C==0      152
;max: 435cc
;min: 136cc
;avg: ~320.87cc (320+223/256)
;114 bytes
;
  ld a,b
  ld h,d
  ld l,e
  sla c
  jr c,mul8_24_1
  sla c
  jr c,mul8_24_2
  sla c
  jr c,mul8_24_3
  sla c
  jr c,mul8_24_4
  sla c
  jr c,mul8_24_5
  sla c
  jr c,mul8_24_6
  sla c
  jr c,mul8_24_7
  sla c
  ret c
  ld a,c
  ld h,c
  ld l,c
  ret
mul8_24_1:
    add hl,hl
  rla
  rl c
  jr nc,mul8_24_2
  add hl,de
  adc a,b
  jr nc,mul8_24_2
  inc c
mul8_24_2:
    add hl,hl
  rla
  rl c
  jr nc,mul8_24_3
  add hl,de
  adc a,b
  jr nc,mul8_24_3
  inc c
mul8_24_3:
    add hl,hl
  rla
  rl c
  jr nc,mul8_24_4
  add hl,de
  adc a,b
  jr nc,mul8_24_4
  inc c
mul8_24_4:
    add hl,hl
  rla
  rl c
  jr nc,mul8_24_5
  add hl,de
  adc a,b
  jr nc,mul8_24_5
  inc c
mul8_24_5:
    add hl,hl
  rla
  rl c
  jr nc,mul8_24_6
  add hl,de
  adc a,b
  jr nc,mul8_24_6
  inc c
mul8_24_6:
    add hl,hl
  rla
  rl c
  jr nc,mul8_24_7
  add hl,de
  adc a,b
  jr nc,mul8_24_7
  inc c
mul8_24_7:
    add hl,hl
  rla
  rl c
  ret nc
  add hl,de
  adc a,b
  ret nc
  inc c
  ret
 