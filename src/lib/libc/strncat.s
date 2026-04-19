.z80
.code

.export _strncat
_strncat:
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
    jr z,_strncatend
_strncatscan:
    ld a,(hl)
    or a
    jr z,_strncatcopy
    inc hl
    jp _strncatscan
_strncatcopy:
    ld a,(de)
    or a
    jr z,_strncatnull
    ld (hl),a
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jp nz,_strncatcopy
_strncatnull:
    ld (hl),0
_strncatend:
    ld l,(ix+0)
    ld h,(ix+1)
    pop ix
    pop bc
    ret
