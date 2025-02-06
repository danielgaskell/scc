.code

; 0 = idle, -1 = busy, 1 = platform unsupported
.export _Print_Busy
_Print_Busy:
	push bc
	call _Sys_Type
	ld a,l
	cp 1
	jr z,_busy_cpc
	cp 4
	jr z,_busy_msx
	ld l,1	; 1 = platform unsupported
	pop bc
	ret
_busy_cpc:
	ld b,0xF5
	in a,(c)
	rla
	rla
	sbc a,a
	ld l,a
	pop bc
	ret
_busy_msx:
	in a,(0x90)
	rra
	rra
	sbc a,a
	ld l,a
	pop bc
	ret

; 0 = ok, 1 = platform unsupported, 2 = timeout
.export _Print_Char
_Print_Char:
	; A = machine type
	call _Sys_Type
	ld a,l
	; L = character
	pop de
	pop hl
	push hl
	push de
	push bc
	; DE = timeout
	ld de,0x6403 ; (this waiting approach is inspired by Quigs)
	; check platform
	cp 1
	jr z,_char_cpc
	cp 4
	jp z,_char_msx
	ld l,1	; 1 = platform unsupported
	pop bc
	ret
	
_char_cpc:
	ld b,0xF5
	in a,(c)
	rla
	rla
	jr nc,_char_cpcout	; not busy, go ahead
	dec e
	jr nz,_char_cpc		; try checking busy 3 times
	rst 0x30			; Idle()
	dec d
	jr z,_char_timeout	; hard timeout after 100 attempts
	ld e,3
	jr _char_cpc
_char_cpcout:
	; CPC requires strobe 0 - 1 - 0
	ld b,0xEF
	res 7,l
	out (c),l
	set 7,l
	out (c),l
	res 7,l
	out (c),l
	ld l,0
	pop bc
	ret
	
_char_msx:
	in a,(0x90)
	rra
	rra
	jr nc,_char_msxout	; not busy, go ahead
	dec e
	jr nz,_char_msx		; try checking busy 3 times
	rst 0x30			; Idle()
	dec d
	jr z,_char_timeout	; hard timeout after 100 attempts
	ld e,3
	jr _char_msx
_char_msxout:
	ld a,l
	out (0x91),a
	xor a
	out (0x90),a
	dec a
	out (0x90),a
	ld l,0
	pop bc
	ret
	
_char_timeout:
	ld l,2
	pop bc
	ret
