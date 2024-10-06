.z80
.code

; IMPLEMENTED:
; uint32_t _castl_f(long a1)
; uint32_t _castul_f(unsigned long a1)

.export __castl_f
__castl_f:
	ld ix,0
	add ix,sp
	ld l,(ix+2)
	ld h,(ix+3)
	ld e,(ix+4)
	ld d,(ix+5)
	call i32tof32
	ld (__hireg),de
	ret

.export __castul_f
__castul_f:
	ld ix,0
	add ix,sp
	ld l,(ix+2)
	ld h,(ix+3)
	ld e,(ix+4)
	ld d,(ix+5)
	call u32tof32
	ld (__hireg),de
	ret

; Code below this point adapted from z88dk IEEE Floating Point Package
;  Copyright (c) 2015 Digi International Inc.
;
;  This Source Code Form is subject to the terms of the Mozilla Public
;  License, v. 2.0. If a copy of the MPL was not distributed with this
;  file, You can obtain one at http://mozilla.org/MPL/2.0/.
;
;  feilipu, 2019 April
;  adapted for z80, z180, and z80n
;  prevtenet, 2024 October
;  adapted for SCC
;-------------------------------------------------------------------------

; convert long in dehl to float in dehl
i32tof32:
    ex de,hl                    ; hlde
    ld b,h                      ; to hold the sign, put copy of ULSW into b
    bit 7,h                     ; test sign, negate if negative
    jr z,dldf0
    ld c,l                      ; LLSW into c
    ld hl,0
    or a                        ; clear C
    sbc hl,de                   ; least
    ex de,hl
    ld hl,0
    sbc hl,bc
    jp po,dldf0                 ; number in hlde, sign in b[7]

; here negation of 0x80000000 = -2^31 = 0xcf000000
    ld de,#0xcf00
    ld hl,0
    ret

; convert unsigned long in dehl to float in dehl
u32tof32:
    res 7,b                     ; ensure unsigned long's "sign" bit is reset
                                ; to hold the sign, put copy of MSB into b
                                ; continue, with unsigned long number in dehl
    ex de,hl

dldf0:
; number in hlde, sign in b[7]
    ld c,150                    ; exponent if no shift
    ld a,h
    or a
    jr nz,dldfright             ; go shift right
; exponent in c, sign in b[7]
    push af
    jp m32_fsnormalize          ; piggy back on existing code in _fsnormalize

; must shift right to make h = 0 and mantissa in lde
dldfright:
    ld a,h
    and #0xf0
    jr z,dldf4                  ; shift right only 1-4 bits

; here shift right 4-8
    srl h
    rr l
    rr d
	rr e
    srl h
    rr l
    rr d
	rr e
    srl h
    rr l
    rr d
	rr e
    srl h
    rr l
    rr d                        ; 4 for sure
	rr e
    ld c,154                    ; exponent for no more shifts
; here shift right 1-4 more
dldf4:
    ld a,h
    or a
    jr z,dldf8                  ; done right
    srl h
    rr l
    rr d
	rr e
    inc c
    ld a,h
    or a
    jr z,dldf8
    srl h
    rr l
    rr d
	rr e
    inc c
    ld a,h
    or a
    jr z,dldf8
    srl h
    rr l
    rr d
	rr e
    inc c
    ld a,h
    or a
    jr z,dldf8
    srl h
    rr l
    rr d
	rr e
    inc c
dldf8:                          ; pack up the floating point mantissa in lde, exponent in c, sign in b[7]
    sla l
    rl b                        ; get sign (if unsigned input, it was forced 0)
    rr c
    rr l
    ld h,c                      ; result in hlde
    ex de,hl 
    ret                         ; result in dehl

; enter here with af' carry clear for float functions m32_float32, m32_float32u
m32_fsnormalize:
    xor a
    or l
    jr z,fa8a
    and #0xf0
    jr z,S24L                   ; shift 24 bits, most significant in low nibble   
    jr S24H                     ; shift 24 bits in high
fa8a:
    or d
    jr z,fa8b
    and #0xf0
    jp z,S16L                   ; shift 16 bits, most significant in low nibble
    jp S16H                     ; shift 16 bits in high
fa8b:
    or e
    jp z,normzero               ;  all zeros
    and #0xf0
    jp z,S8L                    ; shift 8 bits, most significant in low nibble 
    jp S8H                      ; shift 8 bits in high

S24H:                           ; shift 24 bits 0 to 3 left, count in c
    rl e
	rl d
    rl l
    jr c,S24H1
    rl e
	rl d
    rl l
    jr c,S24H2
    rl e
	rl d
    rl l
    jr c,S24H3
    ld a,-3                     ; count
    jr normdone                ; from normalize

S24H1:
    rr l
    rr d                        ; reverse overshift
	rr e
    ld a,c                      ; zero adjust
    jr normdone0

S24H2:
    rr l
    rr d
	rr e
    ld a,-1
    jr normdone

S24H3:
    rr l
    rr d
	rr e
    ld a,-2
    jr normdone

S24L:                           ; shift 24 bits 4-7 left, count in C
    rl e
	rl d
    rl l
    rl e
	rl d
    rl l
    rl e
	rl d
    rl l
    ld a,#0xf0
    and l
    jp z,S24L4more               ; if still no bits in high nibble, total of 7 shifts
    rl e
	rl d
    rl l
; 0, 1 or 2 shifts possible here
    rl e
	rl d
    rl l
    jr c,S24Lover1
    rl e
	rl d
    rl l
    jr c,S24Lover2
; 6 shift case
    ld a,-6
    jr normdone

S24L4more:
    rl e
	rl d
    rl l
    rl e
	rl d
    rl l
    rl e
	rl d
    rl l
    rl e
	rl d
    rl l
    ld a,-7
    jr normdone

S24Lover1:                      ; total of 4 shifts
    rr l
    rr d                        ; correct overshift
	rr e
    ld a,-4
    jr normdone

S24Lover2:                      ; total of 5 shifts
    rr l
    rr d
	rr e
    ld a,-5                     ; this is the very worst case
                                ; drop through to .normdone

; enter here to continue after normalize
; this path only on subtraction
; a has left shift count, lde has mantissa, c has exponent before shift
; b has original sign of larger number
;
normdone:
    add a,c                     ; exponent of result
    jr nc,normzero              ; if underflow return zero
normdone0:                      ; case of zero shift
    rl l
    rl b                        ; sign
    rra
    rr l
    ld h,a                      ; exponent
    ex de,hl                    ; return DEHL
    pop af
    ret

normzero:                       ; return zero
    ld hl,0
    ld d,h
    ld e,l
    pop af
    ret

; all bits in lower 4 bits of e (bits 0-3 of mantissa)
; shift 8 bits 4-7 bits left
; e, l, d=zero
S8L:
    sla e
    sla e
    sla e
    ld a,#0xf0
    and e
    jp z,S8L4more               ; if total is 7
    sla e                       ; guaranteed
    sla e                       ; 5th shift
    jr c,S8Lover1               ; if overshift
    sla e                       ; the shift
    jr c,S8Lover2
; total of 6, case 7 already handled
    ld l,e
    ld e,d                      ; zero
    ld a,-22
    jr normdone

S8Lover1:                       ; total of 4
    rr e
    ld l,e
    ld e,d                      ; zero
    ld a,-20
    jr normdone

S8Lover2:                       ; total of 5
    rr e
    ld l,e
    ld e,d                      ; zero
    ld a,-21
    jr normdone

S8L4more:
    sla e
    sla e
    sla e
    sla e
    ld l,e
    ld e,d                      ; zero
    ld a,-23
    jr normdone

; shift 16 bit fraction by 4-7
; l is zero, 16 bits number in de
S16L:
    rl e
	rl d
    rl e
	rl d
    rl e                        ; 3 shifts
	rl d
    ld a,#0xf0
    and d
    jp z,S16L4more              ; if still not bits n upper after 3
    sla e
    rl d                        ; guaranteed shift 4
    jp m,S16L4                  ; complete at 4
    sla e
    rl d
    jp m,S16L5                  ; complete at 5
    rl e                        ; 6 shifts, case of 7 already taken care of must be good
	rl d
    ld l,d
    ld d,e
    ld e,0
    ld a,-14
    jp normdone

S16L4:
    ld l,d
    ld d,e
    ld e,0
    ld a,-12
    jp normdone

S16L5:                          ; for total of 5 shifts left
    ld l,d
    ld d,e
    ld e,0
    ld a,-13
    jp normdone

S16L4more:
    rl e
    rl d
    rl e
    rl d
    rl e
    rl d
    rl e
    rl d
    ld l,d
    ld d,e
    ld e,0
    ld a,-15
    jp normdone

; shift 0-3, l is zero
; 16 bits in de
S16H:
    sla e
    rl d
    jr c,S16H1                   ; if zero
    jp m,S16H2                   ; if 1 shift
    sla e
    rl d
    jp m,S16H3                   ; if 2 ok
; must be 3
    rl e
	rl d
    ld l,d
    ld d,e
    ld e,0
    ld a,-11
    jp normdone

S16H1:                          ; overshift
    rr d
	rr e
    ld l,d
    ld d,e
    ld e,0
    ld a,-8
    jp normdone

S16H2:                          ; one shift
    ld l,d
    ld d,e
    ld e,0
    ld a,-9
    jp normdone

S16H3:
    ld l,d
    ld d,e
    ld e,0
    ld a,-10
    jp normdone

; shift 8 left 0-3
; number in e, l, d==zero
S8H:
    sla e
    jr c,S8H1                   ; jump if bit found in data
    sla e
    jr c,S8H2
    sla e
    jr c,S8H3
; 3 good shifts, number in a shifted left 3 ok
    ld l,e
    ld e,d                      ; zero
    ld a,-19
    jp normdone

S8H1:
    rr e                        ; correct overshift
    ld l,e
    ld e,d
    ld a,-16                    ; zero shifts
    jp normdone

S8H2:
    rr e                        ; correct overshift
    ld l,e
    ld e,d
    ld a,-17                    ; one shift
    jp normdone

S8H3:
    rr e                        ; correct overshift
    ld l,e
    ld e,d
    ld a,-18
    jp normdone                ; worst case S8H
	