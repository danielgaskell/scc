.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export __pushpopf
__pushpopf:
;26 bytes, adds 118cc to the traditional routine
  ex (sp),hl
  push de
  push bc
  push af
  push hl
  ld hl,pushpopret
  ex (sp),hl
  push hl
  push af
  ld hl,12
  add hl,sp
  ld a,(hl)
  inc hl
  ld h,(hl)
  ld l,a
  pop af
  ret
pushpopret:
  pop af
  pop bc
  pop de
  pop hl
  ret
  