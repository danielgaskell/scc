.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32amean
f32amean:
;(x+y)/2

  call f32add
  push hl
  push af
  ld h,b
  ld l,c
  inc hl
  inc hl
  ld a,(hl)
  add a,a
  inc hl
  ld a,(hl)
  adc a,a
  jr z,f32amean_done
  inc a
  jr z,f32amean_done
  dec a
  dec a
  rra
  ld (hl),a
  dec hl
  rl (hl)
  rrc (hl)
f32amean_done:
  pop af
  pop hl
  ret
  