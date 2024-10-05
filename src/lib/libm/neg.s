.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32neg
f32neg:
;-x ==> z
  push hl
  push de
  push bc
  push af
  ld d,b
  ld e,c
  ldi
  ldi
  ldi
  ld a,(hl)
  xor #0x80
  ld (de),a
  pop af
  pop bc
  pop de
  pop hl
  ret
  