.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).
;
; float atof(const char *str)
;   Parses the C float (f32) from the NUL-terminated string at (str).

.export _atof
_atof:
	pop de
	pop hl
	push hl
	push de
	push bc
	ld bc,__floatd
	call atof32
	ld hl,(__floatd+2)
	ld (__hireg),hl
	ld hl,(__floatd+0)
	pop bc
	ret

atof32:
;Inputs:
;  HL points to the string
;  BC points to where to write the float
;Output:
;  The f32 float is written to (BC)
;Destroys:
;  BC, DE
;
  call __pushpopf
  push bc ; save the pointer to the output
;Check for a negative sign, save for later, and advance the pointer if so.
;   Advance ptr
  ld a,(hl)
  sub '-'
  sub 1
  push af
  jr nc,af_digit
af_leadzero:
  inc hl
;Skip all leading zeroes
af_digit:
  ld a,(hl)
  cp '0'
  jr z,af_leadzero

;Set exponent to 0
  ld b,0
;Check if the next char is '.'
  sub '.'
  or a      ;to reset the carry flag
  jr nz,af_skip_1
  .db #0xFE  ;start of cp *
;Get rid of zeroes
af_decexp:
  dec b
  inc hl
  ld a,(hl)
  cp '0'
  jr z,af_decexp
  scf
af_skip_1:
; at this point, we ought to pointing at our first non-zero digit, unless all
; digits were 0, in which case we might be pointing to 'e'
  rl c
  ld a,(hl)
  sub '0'
  cp 10
  jp nc,af_zchk_eng
  srl c

;Now we read in the next 8 digits. Ideally, we want 27 bits worth, but 8 digits
; gives us about 26.5 bits
  call a2bcd
  ld d,a
  call a2bcd
  ld e,a
  push de
  call a2bcd
  ld d,a
  call a2bcd
  ld e,a

;Now DEHL holds the 4-digit base-100 number (little-endian)
;b is the exponent
;if carry flag is set, just need to get rid of remaining digits
;Otherwise, need to get rid of remaining digits, while incrementing the exponent

  sbc a,a
  ld c,a
af_loop_1:
  ld a,(hl)
  cp '.'
  jr nz,af_subdigit
  dec c
  jp pe,af_skip_7 ; we've reached our first decimal
  jr af_skip_2    ; else C was already FF, then a decimal was already encountered
af_subdigit:
  sub '0'
  cp 10
  jr nc,af_skip_6
; carry flag is set
af_skip_7:
  inc hl
  ld a,b
  adc a,c
  ; if carry flag is set, then we added 0
  jr c,af_loop_1
  jp z,af_inf2
  ld b,a
  jr af_loop_1

af_skip_6:
  ld a,(hl)
;Now check for engineering `E` to modify the exponent
  and #0xDF
  cp 'E'
  call z,af_str_eng_exp

af_skip_2:
  pop hl
  ex de,hl        ; DEHL is the number

  push bc         ; B is the base-10 exponent
  call af_times_256
  ld b,c
  call af_times_256
  push bc         ; BC is the upper 16 bits of the result
  call af_times_256
  ld b,c
  call af_times_256
  pop de
  ld h,b
  ld l,c
  ; DEHL is 32 bits of the significand
  pop bc  ; B is the base-10 exponent

;HLC holds the 3-digit significand, need to normalize it
; make sure it isn't zero
  ld a,e
  or d
  or l
  or h
  jp z,af_zero


  ld c,#0x7E
  ld a,d
  or a
  jr af_loop_3_start
af_loop_3:
  dec c
  add hl,hl
  rl e
  adc a,a
af_loop_3_start:
  jp p,af_loop_3

  add a,a
  ld d,a
  pop af
  rr c
  rr d

; round
  sla l
  jr nc,af_rounded
  inc h
  jr nz,af_rounded
  inc e
  jr nz,af_rounded
  inc d
  jr nz,af_rounded
  inc c
af_rounded:
;CDEH * 10^B

; write the float in CDEH to the output
  ld a,h
  pop hl
  ld (hl),a
  inc hl
  ld (hl),e
  inc hl
  ld (hl),d
  inc hl
  ld (hl),c
  dec hl
  dec hl
  dec hl

; now multiply the float at HL by 10^B
  xor a
  sub b
  ld de,f32_pown10_LUT
  jp p,af_cpx
  ld a,b
  ld de,f32_pow10_LUT

af_cpx:
  cp 38
  jr c,af_ldbh
  ld a,-1

af_ldbh:
  ld b,h
  ld c,l
  call af_mul_first
  call af_mul_twice
af_mul_twice:
  call af_mul_once
af_mul_once:
  inc de
  inc de
  inc de
  inc de
af_mul_first:
  rra
  call c,f32mul
  ret

af_str_eng_exp:
; HL points to the string, B is the current exponent, returns adjusted exponent in B
  push de   ; save lower digits of input
  ld e,0
  inc hl
  ld a,(hl)
  sub '-'   ;negative exponent?
  ld c,a    ;backup result
  jr nz,af_skip_5
  jr af_skip_3
af_loop_2:
  ld d,a  ; save the digit to add

; make sure E doesn't exceed 3 (else the exponent is over 40)
  ld a,e
  cp 4
  jr nc,af_eng_overflow

  add a,a
  add a,a
  add a,e
  add a,a ; E*10
  add a,d ; E*10+D
  ld e,a

af_skip_3:
  inc hl
af_skip_5:
  ld a,(hl)
  sub '0'
  cp 10
  jr c,af_loop_2

  ld a,e
  cp 60
  jr nc,af_eng_overflow
  ld a,c
  or a
  ld a,b
  jr nz,af_skip_4
  sub e
  .db #0xFE  ; start of `cp *` to skip the next byte
af_skip_4:
  add a,e
  ld b,a
  pop de
  ret

af_times_256:
  call af_times_16
af_times_16:
  call af_times_4
af_times_4:
  call af_times_2
af_times_2:
;DEHL 8 BCD digits, need to multiply by 2
  ld a,l
  add a,a
  daa
  ld l,a

  ld a,h
  adc a,a
  daa
  ld h,a

  ld a,e
  adc a,a
  daa
  ld e,a

  ld a,d
  adc a,a
  daa
  ld d,a

  rl c
  ret

af_inf2:
  ld c,-1
af_eng_overflow:
  pop de  ; pop off saved digits
  pop af  ; pop off the return address for the call
  pop hl  ; pop off the other saved digits
  ld a,c
  or a
  jr z,af_zero
af_inf:
;return inf
  pop af
  pop hl
  ld a,0
  ld (hl),a
  inc hl
  ld (hl),a
  inc hl
  ld (hl),a
  dec a
  rra
  rr (hl)
  inc hl
  ld (hl),a
  ret

af_zchk_eng:
  and #0xDF
  cp 'E'-'0'
  jr nz,af_zwr_hl
  ; otherwise, we need to read through the engineering exponent
af_enloop:
  inc hl
  ld a,(hl)
  cp '-'
  jr nz,af_engread
af_engread:
  ld a,(hl)
  sub '0'
  cp 10
  jr c,af_enloop
af_zwr_hl:
af_zero:
  pop af
  pop hl
  ld a,0
  ld (hl),a
  inc hl
  ld (hl),a
  inc hl
  ld (hl),a
  inc hl
  rra
  ld (hl),a
  ret

a2bcd:
;Inputs:
;   HL points to the next digit to read in
;   B is the base-10 exponent
;   carry flag is set if we shouldn't increment the exponent
;Outputs:
;   HL is advanced as needed (not necessarily by 2)
;   A is the next BCD byte
;   carry is set if it was already set initially, or if a decimal was reached,
;   or if a non-numeric character was reached, otherwise, carry remains reset.
;Destroys:
;   C
  jr c,a2bcd_noexp
  ld a,(hl)
  cp '.'
  jr z,a2bcd_dec_fnd1
  cp '0'
  jr c,a2bcd_done1
  cp '9'+1
  jr nc,a2bcd_done1
  inc hl
  inc b
  add a,a
  add a,a
  add a,a
  add a,a
  ld c,a

  ld a,(hl)
  cp '.'
  jr z,a2bcd_dec_fnd2
  cp '0'
  jr c,a2bcd_done2
  cp '9'+1
  jr nc,a2bcd_done2
  inc hl
  inc b
  and #0x0F
  or c
  ret

a2bcd_done1:
  xor a
  scf
  ret
a2bcd_done2:
  ld a,c
  scf
  ret

a2bcd_dec_fnd1:
  inc hl
a2bcd_noexp:
  ld a,(hl)
  cp '0'
  jr c,a2bcd_done1
  cp '9'+1
  jr nc,a2bcd_done1
  add a,a
  add a,a
  add a,a
  add a,a
  ld c,a

a2bcd_dec_fnd2:
  inc hl
  ld a,(hl)
  cp '0'
  jr c,a2bcd_done2
  cp '9'+1
  jr nc,a2bcd_done2
  inc hl
  and #0x0F
  or c
  scf
  ret
