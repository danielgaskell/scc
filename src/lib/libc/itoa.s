.export _itoa
.export _uitoa

_itoa:
    push ix
    ld ix,0
    add ix,sp
    push bc             ; preserve BC

    ; fetch arguments
    ld e,(ix+4)
    ld d,(ix+5)         ; DE = value
    ld l,(ix+6)
    ld h,(ix+7)         ; HL = dest
    ld c,(ix+8)         ; C = radix

    ; check for negative (only on radix=10)
    ld a,c
    cp 10
    jr nz,itoanotneg    ; skip if base != 10
    bit 7,d
    jr z,itoanotneg     ; skip if positive

    ld a,'-'            ; output '-' and negate DE
    ld (hl),a
    inc hl
    xor a
    sub e
    ld e,a
    sbc a,a
    sub d
    ld d,a
    jr itoanotneg

_uitoa:
    push ix
    ld ix,0
    add ix,sp
    push bc             ; preserve BC

    ; fetch arguments
    ld e,(ix+4)
    ld d,(ix+5)         ; DE = value
    ld l,(ix+6)
    ld h,(ix+7)         ; HL = dest
    ld c,(ix+8)         ; C = radix

itoanotneg:
    ex de,hl            ; HL = value (unsigned), DE = dest
    xor a
    push af             ; push 0 as sentinel

itoaloop:
    ; HL / C -> quotient in HL, remainder in A
    xor a
    ld b,16
itoadiv:
    add hl,hl
    rla
    cp c
    jr c,itoadivnosub
    sub c
    inc l
itoadivnosub:
    djnz itoadiv
    
    ; A -> ASCII character
    cp 10
    jr c,itoaisdigit
    add a,39            ; offset for hex digits 'a' through 'f'
itoaisdigit:
    add a,'0'
    push af             ; push ASCII digit
    ld a,h              ; check if quotient in HL is zero
    or l
    jr nz,itoaloop

itoawrite:
    pop af
    or a
    jr z,itoadone       ; hit sentinel 0
    ld (de),a
    inc de
    jr itoawrite

itoadone:
    xor a
    ld (de),a          ; null-terminate string
    ld l,(ix+6)         ; return original dest pointer in HL
    ld h,(ix+7)
    pop bc
    pop ix
    ret
