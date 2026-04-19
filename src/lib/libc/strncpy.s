.z80
.code

.export _strncpy
_strncpy:
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
    jr z,_strncpyend
_strncpyloop:
    ld a,(de)
    ld (hl),a
    inc hl
    inc de
    dec bc
    or a
    jr z,_strncpypadcheck
    ld a,b
    or c
    jp nz,_strncpyloop
    jr _strncpyend
_strncpypadcheck:
    ld a,b
    or c
    jr z,_strncpyend
_strncpypadloop:
    ld (hl),0
    inc hl
    dec bc
    ld a,b
    or c
    jp nz,_strncpypadloop
_strncpyend:
    ld l,(ix+0)
    ld h,(ix+1)
    pop ix
    pop bc
    ret
