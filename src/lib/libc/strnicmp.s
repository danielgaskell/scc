.z80
.code

.export _strnicmp
_strnicmp:
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
    jr z,_strnicmpmatch
_strnicmploop:
    ld a,(de)
    cp (hl)
    jr z,_strnicmpcont
    push bc
    ld c,a
    ld a,(hl)
    cp 65
    jr c,_strnicmpconv
    cp 91
    jr nc,_strnicmpconv
    or 0x20
_strnicmpconv:
    ld b,a
    ld a,c
    cp 65
    jr c,_strnicmpcomp
    cp 91
    jr nc,_strnicmpcomp
    or 0x20
_strnicmpcomp:
    cp b
    jr nz,_strnicmpdiff
    pop bc
    jr _strnicmpadvance
_strnicmpcont:
    or a
    jr z,_strnicmpmatch    
_strnicmpadvance:
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jp nz,_strnicmploop
_strnicmpmatch:
    ld hl,0
    pop ix
    pop bc
    ret
_strnicmpdiff:
    pop bc
    jr c,_strnicmppos      
    ld hl,#0xFFFF
    pop ix
    pop bc
    ret
_strnicmppos:
    ld hl,1
    pop ix
    pop bc
    ret
