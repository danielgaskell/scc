.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

; char* _fnum(double val, char fmt, int prec);

fnumbuf:
	.ds 32
	
EXT_FMT_LEN:
	.byte 6
	
EXT_FMT_MAX_ENG:
	.byte 5

EXT_FMT_MIN_ENG:
	.byte -5

.export __fnum
__fnum:
	ld ix,0
	add ix,sp
	ld a,(ix+8)
	ld (EXT_FMT_LEN),a
	ld a,(ix+6)
	cp 'f'
	jr nz,fnumelse
	; 'f' mode
	ld a,5
	ld (EXT_FMT_MAX_ENG),a
	ld a,-5
	ld (EXT_FMT_MIN_ENG),a
	jr fnumend
fnumelse:
	; 'e/E/g/G' mode: exponential where preci = digits after decimal (total is preci + 1)
	; (this is not exactly what the standard says, but as close as we can easily get)
	ld a,(EXT_FMT_LEN)
	inc a
	ld (EXT_FMT_LEN),a
	ld a,1
	ld (EXT_FMT_MAX_ENG),a
	ld a,-1
	ld (EXT_FMT_MIN_ENG),a
fnumend:
	ld hl,2
	add hl,sp
	ld bc,fnumbuf
	call f32toa
	ld hl,fnumbuf
	ret

; Set a hard upper limit to the number of digits used
; This routine uses at most MAX_FMT_LEN+8 bytes of space

MAX_FMT_LEN equ 8

; Set the default number of digits used. If EXT_FMT_LEN is defined and
; that value is 0 or greater than MAX_FMT_LEN, then FMT_LEN is used.

FMT_LEN equ 6

; set the char to use for `e` notation
TOK_ENG equ 'e'

; set the char to use for negatives
TOK_NEG equ '-'

; set the char to use for decimal points
TOK_DECIMAL equ '.'

FMT_MIN_ENG equ -5

scrap:
	.word 0,0

.export f32toa
f32toa:
;Inputs:
;   HL points to the input float
;   BC points to where the string gets written.
  call __pushpopf
  ld e,(hl)
  inc hl
  ld d,(hl)
  inc hl
  ld (scrap),de
  ld a,(hl)
  ld e,a
  add a,a
  inc hl
  ld a,(hl)
  ld d,a
  adc a,a
  ld h,b
  ld l,c
  inc a
  jp z,f32toa_return_infnan
  res 7,d
  ld (scrap+2),de
  jr nc,jr1
  ld (hl),TOK_NEG    ;negative sign
  inc hl
jr1:
;DEBC is the float, A is a copy of the exponent, HL points to the next byte to write
  dec a
  jp z,f32toa_return_0

  push hl

  ld (hl),'0' ; pad one more '0' to make room for rounding (i.e. 9.999999=>10.000000)
  inc hl

  push hl

; approximate the base-10 exponent as floor((A-127)*log10(2))
; which is approximately floor((A-127)*77/256)
  ld h,0
  ld l,a
  ld b,h
  ld c,a
  add hl,hl ;2
  add hl,hl ;4
  add hl,hl ;8
  add hl,bc ;9
  add hl,hl ;18
  add hl,bc ;19
  add hl,hl ;38
  add hl,hl ;76
  add hl,bc ;77
  ; now subtract 127*77 from HL and keep the upper 8 bits of the result
  ld a,205
  add a,l
  ld a,217
  adc a,h
  ; A is the approximate base-10 exponent
;scrap needs to be multipled by 10^-A
  push af ; save the exponent
  call nz,f32toa_adjust
;the float is now on [1, 20)
;Let's no work on the float as a 0.24 fixed-point value
;we'll first need to extract the integer component
  ld hl,(scrap)
  ld de,(scrap+2)
  ld a,e
  rlca
  scf
  rra
  ld e,a
  ld a,d
  adc a,a
  sub 126
; A is how many bits to shift out of HLE
  ld b,a
  xor a
f32toa_fdig_loop:
  add hl,hl
  rl e
  rla
  djnz f32toa_fdig_loop

; A is the first digit
  pop bc  ; B is the base-10 exponent
  ex (sp),hl
  ld d,0
  cp 10
  jr c,f32toa_fdig_fixed
  inc d
  dec hl
  ld (hl),'1'
  inc hl
  sub 10
f32toa_fdig_fixed:
  add a,'0'
  ld (hl),a

  ld a,b
  ex (sp),hl
  pop bc
;A is the base-10 exponent
;BC is where to write the digits
;EHL is the 0.24 fixed-point number
;D is 1 if we have 2 digits already, else D is 0
  push af
  call f32toa_digits

;BC points to the last digit. We'll want to round!
  ld h,b
  ld l,c
  ld a,(hl)
  ld (hl),0
  add a,5
  jr f32toa_round_start
f32toa_round_loop:
  dec hl
  inc (hl)
  ld a,(hl)
f32toa_round_start:
  cp '9'+1
  jr nc,f32toa_round_loop
; the string is rounded
  inc hl
  ld (hl),0

  ; pop the exponent off the stack and sign-extend
  pop af
  ld e,a
  add a,a
  sbc a,a
  ld d,a
  pop hl
  ; check if rounding caused overflow; increment exponent if so
  ld a,(hl)
  cp '0'
  jp z,formatstr
  inc de
  jp formatstr


f32toa_digits:
; How many digits do we need?
  ; we define the number of digits externally
  ld a,(EXT_FMT_LEN)

  ; if A is 0, use FMT_LEN
  or a
  jr nz,jr5
  ld a,FMT_LEN
jr5:

  ; if A > MAX_FMT_LEN, set A to MAX_FMT_LEN
  cp MAX_FMT_LEN
  jr c,jr6
  ld a,MAX_FMT_LEN
jr6:

  ; the first digit is written.
  ; if we only wanted 1 digit, then A is 0 and we should stop
  or a
  ret z

; we want D more digits (an extra one for rounding)
f32toa_digits_loop:
  push af
  call f32toa_next_digit
  pop af
f32toa_digits_start:
  dec a
  jr nz,f32toa_digits_loop
  ret

f32toa_next_digit:
; multiply 0.EBC by 10
  push bc
  ld b,h
  ld c,l
  ld a,e
  ld d,6  ;overflow digit. We shift d 3 times, that 6 turns into a 0x30 == '0'

  add hl,hl
  adc a,a
  rl d

  add hl,hl
  adc a,a
  rl d

  add hl,bc
  adc a,e
  jr nc,jr7
  inc d
jr7:

  add hl,hl
  adc a,a
  ld e,a
  ld a,d
  adc a,a

  pop bc
  inc bc
  ld (bc),a
  ret


f32toa_adjust:
  ld hl,f32_pown10_LUT-4
  jr c,pow10_mul
  neg
  ld hl,f32_pow10_LUT-4
pow10_mul:
;HL points to the first entry of the LUT
;(scrap) is the accumulator
;bottom 6 bits of A control which terms to multiply by
  ld de,scrap
  ld b,d
  ld c,e
; process pow10_mul_sub 6 times
  call pow10_mul_sub3
pow10_mul_sub3:
  call pow10_mul_sub
  call pow10_mul_sub
pow10_mul_sub:
  inc hl
  inc hl
  inc hl
  inc hl
  rra
  jp c,f32mul
  ret

f32toa_return_0:
  ld (hl),'0'
  inc hl
  ld (hl),0
  ret

f32toa_return_infnan:
  rl b  ; save the sign
  ld a,e
  add a,a
  ld de,(scrap)
  or d
  or e
  ex de,hl
  ld hl,str_NaN
  jr nz,f32toa_return
  ld hl,str_inf
  rr b
  jr nc,f32toa_return
  ld a,'-'
  ld (de),a
  inc de
f32toa_return:
  jp mov4
  
; This routine for taking a base-10 exponent and a string of digits and (without
; a decimal) and inserting a decimal, any leading zeros, stripping trailing
; zeros, and appending an exponent if needed.



; Define this to use an external reference for the max exponent before switching
; to enginieering mode.
;#define EXT_FMT_MAX_ENG  <pointer>

; Define the max exponent to use before switching to ENG format
; If none is defined, this uses FMT_LEN.
; If EXT_FMT_MAX_ENG is used and is 0 or larger than FMT_LEN,
; then this will be used.
;#define FMT_MAX_ENG


; Define this to use an external reference for the min exponent before switching
; to enginieering mode.
;#define EXT_FMT_MIN_ENG  <pointer>

; Define the largest negative exponent to use ENG format
; If EXT_FMT_MIN_ENG is used and is 0 or larger than FMT_LEN,
; then this will be used.
FMT_MIN_ENG equ -5  ; causes exponent of -5 to be enginierring

formatstr:
;Inputs:
;   HL points to the null-terminated string of digits
;   DE is the signed exponent.
;Outputs:
;   The string has leading and trailing zeros stripped, a decimal is placed
;   (if needed), and an exponent field is appended (if needed).
;Destroys:
;   HL, DE, BC, AF
;Notes:
;   This routine operates in-place. It assumes that there is enough space
;   allocated for the string. At most MAX_FMT_LEN+10 bytes is needed,
;   assuming the exponent can be up to 5 digits long.
;
; Skip over the negative sign, if any
  ld a,(hl)
  cp TOK_NEG
  jr nz,jr8
  inc hl
jr8:
  push de ; save the exponent
  push hl

;Strip leading zeros
  ld d,h
  ld e,l
  ld a,'0'
  cp (hl)                           ;These two lines can be commented-out to save three
  jr nz,formatstr_no_leading_zeros  ;bytes at the expense of redundant processing.
formatstr_strip_leading_zeros:
  cpi
  jr z,formatstr_strip_leading_zeros
  dec hl

; HL points to the first non-'0' digit
; DE points to the first digit
; Copy bytes from HL to DE until 0x00 is reached at HL or FMT_LEN digits are copied
  ld a,(EXT_FMT_LEN)
  or a
  jr nz,jr9
  ld a,FMT_LEN
jr9:
  cp MAX_FMT_LEN
  jr c,jr10
  ld a,MAX_FMT_LEN
jr10:
  ld c,a

  xor a
  ld b,a
formatstr_copy_digits:
  cp (hl)
  ldi
  jp po,jr2
  jr nz,formatstr_copy_digits
jr2:
  dec hl
  ld (hl),0

formatstr_no_leading_zeros:
; there are no more leading zeros
; Truncate the number of digits if necessary based on FMT_LEN

  call formatstr_remove_trailing_zeros
  pop hl  ; points to the first digit
  pop de  ; exponent

; Make sure the first digit isn't 0x00
  ld a,(hl)
  or a
  jr nz,jr3
  ld (hl),'0'
  inc hl
  ld (hl),a
  ret
jr3:

  call formatstr_ceng
  jr c,formatstr_eng
  inc de
  bit 7,d
  jr nz,formatstr_neg_exp
  ; Otherwise, we need to insert a decimal after DE digits (D is 0, though)
  ld b,d
  ld c,e
  xor a
  cp e
  jr z,formatstr_idec
  cpir
  ; If we have reached a 0x00, we may need to pad with zeros
  jr z,formatstr_pad_right
  ; otherwise, let's insert the decimal
formatstr_idec:
  ld a,(hl)
  or a
  ret z
  ld a,TOK_DECIMAL
formatstr_idec_loop:
  ld c,(hl) ; back up digit
  ld (hl),a
  inc hl
  ld a,c
  or a
  jr nz,formatstr_idec_loop
  ld (hl),a
  ret

formatstr_neg_exp:
; Need to pad -DE 0s to the left
  xor a
  ld c,a
  cpir
  ld b,a
  sub c
  ld c,a
  ;HL-1 is where to start reading bytes
  ;HL-DE is where to start writing bytes
  xor a
  ld d,a
  sub e     ; A is the number of 0s to write
  ld e,a
  ex de,hl
  add hl,de
  ex de,hl
  dec hl
  ; DE points to where to write the last byte
  ; HL points to where to read it from
  ; BC is the number of bytes to copy (it will be non-zero)
  ; A is the number of zeros to insert
  lddr
  ;now from DE backwards, write A '0's
  ex de,hl
  ld b,a
djnz1:
  ld (hl),'0'
  dec hl
  djnz djnz1
  ; finally, write a '.'
  ld (hl),TOK_DECIMAL
  ret

formatstr_pad_right:
  ; append BC+1 0s
  dec hl
  inc bc
jr17:
  ld (hl),'0'
  cpi
  jp pe,jr17
  ld (hl),0
  ret

formatstr_eng:
; need to insert a decimal after the first digit
  inc hl
  call formatstr_idec
  ld (hl),TOK_ENG
  inc hl
  bit 7,d
  jr z,formatstr_exp_to_str
  ld (hl),TOK_NEG
  inc hl
  xor a
  sub e
  ld e,a
  sbc a,a
  sub d
  ld d,a
formatstr_exp_to_str:
  ex de,hl
  ld a,'0'-1
  ld bc,-10000
jr18:
  inc a
  add hl,bc
  jr c,jr18
  cp '0'
  jr z,jr12
  ld (de),a
  inc de
jr12:

  ld a,'9'+1
  ld bc,1000
jr19:
  dec a
  add hl,bc
  jr nc,jr19
  cp '0'
  jr z,jr13
  ld (de),a
  inc de
jr13:

  ld a,'0'-1
  ld bc,-100
jr20:
  inc a
  add hl,bc
  jr c,jr20
  cp '0'
  jr z,jr14
  ld (de),a
  inc de
jr14:

  ld b,10
  ld a,l
jr21:
  add a,10
  dec b
  jr nc,jr21
  ex de,hl
  jr z,formatstr_eng_last_digit
  set 4,b
  set 5,b
  ld (hl),b
  inc hl
formatstr_eng_last_digit:
  add a,'0'
  ld (hl),a
  inc hl
  ld (hl),0
  ret

formatstr_remove_trailing_zeros:
  ; first, seek the end of the string
  xor a
  ld c,a
  ld b,a
  cpir
  dec hl
  ld a,'0'
jr22:
  dec hl
  cp (hl)
  jr z,jr22
  inc hl
  ld (hl),0
  ret

formatstr_ceng:
; Return carry flag set if ENG format is required, else nc
;
; If the exponent is greater than FMT_MAX_ENG, then use enginieering
; notation. Note that FMT_MAX_ENG < 256, so check that D = -1 or 0
  ld a,d
  inc a
  jr z,formatstr_ceng_neg
  add a,254
  ret c   ;the exponent is too big in magnitude, ENG format is required.
; The exponent is positive and less than 256

  ld a,(EXT_FMT_MAX_ENG)
  or a
  jr nz,jr4
  ld a,(EXT_FMT_LEN)
  or a
  jr nz,jr4
  ld a,FMT_LEN

jr4:
  cp e
  ret

formatstr_ceng_neg:
;The exponent is negative and greater than or equal to -256

  ld a,(EXT_FMT_MIN_ENG)
  or a
  jr nz,jr15
  ld a,FMT_MIN_ENG
jr15:
  cp e
  ccf
  ret
  
mov4:
  ldi
  ldi
  ldi
  ldi
  ret
  
f32_pown10_LUT:
.db #0xCD,#0xCC,#0xCC,#0x3D  ;1e-1
.db #0x0A,#0xD7,#0x23,#0x3C  ;1e-2
.db #0x17,#0xB7,#0xD1,#0x38  ;1e-4
.db #0x77,#0xCC,#0x2B,#0x32  ;1e-8
.db #0x95,#0x95,#0xE6,#0x24  ;1e-16
.db #0x1F,#0xB1,#0x4F,#0x0A  ;1e-32

f32_pow10_LUT:
.db #0x00,#0x00,#0x20,#0x41  ;1e1
.db #0x00,#0x00,#0xC8,#0x42  ;1e2
.db #0x00,#0x40,#0x1C,#0x46  ;1e4
.db #0x20,#0xBC,#0xBE,#0x4C  ;1e8
.db #0xCA,#0x1B,#0x0E,#0x5A  ;1e16
.db #0xAE,#0xC5,#0x9D,#0x74  ;1e32

s_neginf:
str_neginf:
  .byte TOK_NEG
  .byte 0
s_inf:
str_inf:
  .ascii "inf"
  .byte 0
s_NaN:
str_NaN:
  .ascii "NaN"
  .byte 0
str_Zero:
  .ascii "0"
  .byte 0
  