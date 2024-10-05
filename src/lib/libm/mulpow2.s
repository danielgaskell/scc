.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32mulpow2
f32mulpow2:
  call __pushpopf
  ld d,b
  ld e,c
  ld c,a  ;copy A to BC, extending the sign
  add a,a
  sbc a,a
  ld b,a
  ldi
  ldi
  inc bc
  inc bc
  ld a,(hl)
  add a,a
  ld (de),a
  inc hl
  ld a,(hl)
  adc a,a
  jr z,f32mulpow2_ret
  inc a
  jr z,f32mulpow2_ret_infnan
  dec a
  rl l  ; save the sign
  add a,c
  jr nc,jr1
  inc b
jr1:

  inc b
  jr z,f32mulpow2_zero
  djnz f32mulpow2_ret_infnan
  .db #0x06 ; start of `ld b,*` to eat the next byte
f32mulpow2_ret_infnan:
  dec a
f32mulpow2_ret:
  rr l  ; shift out the sign
  rra
  ex de,hl
  rr (hl)
  inc hl
  ld (hl),a
  ret

f32mulpow2_inf:
  ld d,-1
  .db 1 ;start of `ld bc,**` to eat the next two bytes
f32mulpow2_zero:
  ld d,0
  rr d
  ld (hl),a
  dec hl
  ld a,0
  rra
  ld (hl),a
  dec hl
  xor a
  ld (hl),a
  dec hl
  ld (hl),a
  ret
  