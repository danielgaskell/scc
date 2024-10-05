.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32rsub
f32rsub:
;-x + y ==> z
;
  call __pushpopf
  ;save the location of the output
  push bc

  ; read and save the bottom 2 bytes of the first operand
  ld c,(hl)
  inc hl
  ld b,(hl)
  inc hl
  push bc

  ; read and save upper 2 bytes of the first operand
  ld c,(hl)
  inc hl
  ld a,(hl)
  xor #0x80
  ld b,a
  jp f32add_part2
  