.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

; STILL NEEDED (FIXME):
; uint32_t _castul_f(unsigned long a1)
; uint32_t _castl_f(long a1)

; IMPLEMENTED:
; uint32_t _cast_f(int a1)
; uint32_t _castu_f(unsigned a1)
; uint32_t _castc_f(signed char a1)
; uint32_t _castuc_f(unsigned char a1)

ftemp:
	.word 0, 0
	
.export __cast_f
__cast_f:
;Inputs:
;   SP+2 holds a 16-bit signed integer, (-32768 to 32767)
;   BC points to where to write the float (now a buffer)
;Outputs:
;   Converts A to an f32 float at HL, (__hireg)
;
  pop de ; HL = SP+2
  pop hl
  push hl
  push de
  ld bc,ftemp ; BC = ftemp
  call __pushpopf

  xor a
  ld (bc),a
  inc bc

  ld a,l
  or h
  ld d,a
  jr z,i16tof32_finish

  ld a,h
  or a
  jp p,jp1
  xor a
  sub l
  ld l,a
  sbc a,a
  sub h
  ld h,a
  scf

jp1:
  rla         ; save the sign
  ld d,#0x95  ;Initial exponent

jr1:
  dec d
  add hl,hl
  jr nc,jr1

  rra   ; shift out the sign
  rr d  ; shift the exponent down, shifting in the sign
  rr h  ; shift the lsb of the exponent into the significand
  rr l

  ld a,l
i16tof32_finish:
  ld (bc),a
  inc bc
  ld a,h
  ld (bc),a
  inc bc
  ld a,d
  ld (bc),a
  
  ; copy ftemp to HL, (__hireg)
  ld hl,(ftemp+2)
  ld (__hireg),hl
  ld hl,(ftemp+0)
  ret
 
.export __castu_f
__castu_f:
;Inputs:
;   SP+2 holds a 16-bit unsigned integer, (0 to 65535)
;   BC points to where to write the float (now a buffer)
;Outputs:
;   Converts HL (unsigned) to an f32 float at HL, (__hireg)
;
  pop de ; HL = SP+2
  pop hl
  push hl
  push de
  ld bc,ftemp ; BC = ftemp
  call __pushpopf

  xor a
  ld (bc),a
  inc bc

  ld a,l
  or h
  ld d,a
  jr z,u16tof32_finish

  ld d,#0x95  ;Initial exponent

jr4:
  dec d
  add hl,hl
  jr nc,jr4

  srl d ; shift the exponent down, shifting in the sign
  rr h  ; shift the lsb of the exponent into the significand
  rr l

  ld a,l
u16tof32_finish:
  ld (bc),a
  inc bc
  ld a,h
  ld (bc),a
  inc bc
  ld a,d
  ld (bc),a
  ; copy ftemp to HL, (__hireg)
  ld hl,(ftemp+2)
  ld (__hireg),hl
  ld hl,(ftemp+0)
  ret

.export __castc_f
__castc_f:
;Inputs:
;   SP+2 holds a 8-bit signed integer, (-128 to 127)
;   BC points to where to write the float (now a buffer)
;Outputs:
;   Converts A to an f32 float at HL, (__hireg)
;
  pop de ; HL = SP+2
  pop hl
  push hl
  push de
  ld bc,ftemp ; BC = ftemp
  push hl
  push af

  or a
  jp p,jp2
  neg
  scf

jp2:
  ; Begin writing the float
  ld h,b
  ld l,c
  ld (hl),0
  inc hl
  ld (hl),0
  inc hl

  jr nz,jr2
  ld (hl),a
  inc hl
  ld (hl),a
  pop af
  pop hl
  ; copy ftemp to HL, (__hireg)
  ld hl,(ftemp+2)
  ld (__hireg),hl
  ld hl,(ftemp+0)
  ret

jr2:
  push bc
  rl c        ; save the sign
  ld b,#0x87  ;Initial exponent

jr3:
  dec b
  add a,a
  jr nc,jr3

  rr c  ; shift in a 1 and shift out the sign
  rr b  ; shift the exponent down, shifting in the sign
  rra   ; shift the lsb of the exponent into the significand
  ld (hl),a
  inc hl
  ld (hl),b

  pop bc
  pop af
  pop hl
  ; copy ftemp to HL, (__hireg)
  ld hl,(ftemp+2)
  ld (__hireg),hl
  ld hl,(ftemp+0)
  ret

.export __castuc_f
__castuc_f:
;Inputs:
;   SP+2 holds a 8-bit unsigned integer, (0 to 255)
;   BC points to where to write the float (now a buffer)
;Outputs:
;   Converts A to an f32 float at HL, (__hireg)
;
  pop de ; HL = SP+2
  pop hl
  push hl
  push de
  ld bc,ftemp ; BC = ftemp
  push hl
  push af

  ; Begin writing the float
  ld h,b
  ld l,c
  ld (hl),0
  inc hl
  ld (hl),0
  inc hl

  or a
  jr nz,jr5
  ld (hl),a
  inc hl
  ld (hl),a
  pop af
  pop hl
  ; copy ftemp to HL, (__hireg)
  ld hl,(ftemp+2)
  ld (__hireg),hl
  ld hl,(ftemp+0)
  ret

jr5:
  push bc
  rl c        ; save the sign
  ld b,#0x87  ;Initial exponent

jr6:
  dec b
  add a,a
  jr nc,jr6

  rr c  ; shift in a 1 and shift out the sign
  rr b  ; shift the exponent down, shifting in the sign
  rra   ; shift the lsb of the exponent into the significand
  ld (hl),a
  inc hl
  ld (hl),b

  pop bc
  pop af
  pop hl
  ; copy ftemp to HL, (__hireg)
  ld hl,(ftemp+2)
  ld (__hireg),hl
  ld hl,(ftemp+0)
  ret
  