.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export __minusf
__minusf:
	ld hl,2
	add hl,sp
	ex de,hl
	ld hl,6
	add hl,sp
	push bc
	ld bc,__floatd
	call f32sub
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	pop bc
	ret

; uses stack and registers for all operations
; input: DE points to first operand
;        HL points to second operand
;        BC points to output location
.export f32sub
f32sub:
;x - y ==> z
;
  call __pushpopf
  ;save the location of the output
  push bc
  ex de,hl

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
  