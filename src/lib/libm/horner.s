.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32horner_step
f32horner_step:
;c + acc*x ==> acc
;HL points to our c
;DE points to our x
;BC points to the accumulator
;This routine is useful for evaluating polynomials.
  push hl
  ld h,b
  ld l,c
  call f32mul
  ex de,hl      ;HL is x, DE is acc
  ex (sp),hl    ;HL is c, stack is x
  call f32add
  pop de        ;HL is c, DE is x
  ret
  