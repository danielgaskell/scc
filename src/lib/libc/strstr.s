.z80
.code

.export _strstr
_strstr:
    push bc
    push ix
    ld ix,6
    add ix,sp
    ld l,(ix+0)
    ld h,(ix+1)
    ld e,(ix+2)
    ld d,(ix+3)
    ld a,(de)
    or a
    jr z,_strstrempty
    ld c,a
_strstrloop:
    ld a,(hl)
    cp c
    jr z,_strstrcheck
    or a
    jr z,_strstrnotfound
    inc hl
    jp _strstrloop
_strstrcheck:
    push hl
    push bc
    ld e,(ix+2)
    ld d,(ix+3)
    inc de
    inc hl
_strstrinner:
    ld a,(de)
    or a
    jr z,_strstrfound
    cp (hl)
    jr nz,_strstrfail
    inc de
    inc hl
    jp _strstrinner
_strstrfail:
    pop bc
    pop hl
    inc hl
    jp _strstrloop
_strstrfound:
    pop bc
    pop hl
_strstrempty:
    pop ix
    pop bc
    ret
_strstrnotfound:
    ld hl,0
    pop ix
    pop bc
    ret
