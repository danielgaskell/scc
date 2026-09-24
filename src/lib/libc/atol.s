.export _atol

_atol:
    push ix
    ld ix,0
    add ix,sp
    push bc
    ld l,(ix+4)
    ld h,(ix+5)         ; HL = str

    ; skip leading whitespace, per standard
atolws:
    ld a,(hl)
    cp ' '
    jr z,atolwsinc
    cp 9
    jr c,atolwsdone     ; < 9
    cp 14
    jr nc,atolwsdone    ; >= 14
atolwsinc:
    inc hl
    jr atolws

atolwsdone:
    ; parse optional sign (+ or -)
    ld b,0              ; B = sign flag (0 = positive, 1 = negative)
    ld a,(hl)
    cp '-'
    jr nz,atolchkplus
    inc hl
    inc b
    jr atolsigndone
atolchkplus:
    cp '+'
    jr nz,atolsigndone
    inc hl
atolsigndone:
    push bc             ; preserve sign flag B on stack
    ex de,hl            ; DE = string pointer

    ; initialize 32-bit accumulator to 0
    ld hl,0
    ld (__hireg),hl

atollloop:
    ld a,(de)
    inc de
    sub '0'
    cp 10
    jr nc,atolldone     ; exit loop on non-digit or null

    push de             ; save string pointer DE
    push af             ; save converted digit A

    ; __mull expects operand 1 in __hireg+HL and operand 2 on stack (high word then low word)
    ld de,0
    push de             ; push high word of 10
    ld de,10
    push de             ; push low word of 10
    call __mull         ; result returned in __hireg+HL

    pop af              ; restore digit A
    pop de              ; restore string pointer DE

    ; add digit (A) to 32-bit accumulator (__hireg+HL)
    add a,l
    ld l,a
    ld a,0
    adc a,h
    ld h,a
    jr nc,atollnoinc

    ; propagate carry into high word (__hireg)
    push hl
    ld hl,(__hireg)
    inc hl
    ld (__hireg),hl
    pop hl

atollnoinc:
    jr atollloop

atolldone:
    pop bc              ; restore sign flag B
    ld a,b
    or a
    jr z,atollpos       ; skip negation if positive (B = 0)

    ; 32-bit negation of __hireg+HL
    xor a
    sub l
    ld l,a
    ld a,0
    sbc a,h
    ld h,a
    push hl
    ld hl,0
    ld de,(__hireg)
    sbc hl,de
    ld (__hireg),hl
    pop hl

atollpos:
    pop bc
    pop ix
    ret
	