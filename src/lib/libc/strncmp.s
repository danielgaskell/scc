.z80
.code

.export _strncmp
_strncmp:
    push bc
    push ix
    ld ix,6
    add ix,sp
    ld l,(ix+0)
    ld h,(ix+1)
    ld e,(ix+2)
    ld d,(ix+3)
    ld c,(ix+4)
    ld b,(ix+5)
    ld a,b
    or c
    jr z,_strncmpmatch
_strncmploop:
    ld a,(de)
    cp (hl)
    jr nz,_strncmpdiff
    or a
    jr z,_strncmpmatch
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jp nz,_strncmploop
_strncmpmatch:
    ld hl,0
    pop ix
    pop bc
    ret
_strncmpdiff:
    jr c,_strncmppos
    ld hl,#0xFFFF
    pop ix
    pop bc
    ret
_strncmppos:
    ld hl,1
    pop ix
    pop bc
    ret
