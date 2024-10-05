.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32abs
f32abs:
;abs(x) ==> z
  call __pushpopf
  ld d,b
  ld e,c
  ldi
  ldi
  ldi
  ld a,(hl)
  and #0x7F
  ld (de),a
  ret
  