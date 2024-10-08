.z80
.code

; alias for _gfx_pixel, but checks bounds first (note that x, y will never be negative by definition)
.export _Gfx_Safe_Pixel
_Gfx_Safe_Pixel:
	push ix
	ld ix,#0x02
	add ix,sp
	ld hl,(__gfx_activew)
	ld e,(ix+2)
	ld d,(ix+3)
	or a
	sbc hl,de
	jr c,GSP_done
	ld hl,(__gfx_activeh)
	ld e,(ix+4)
	ld d,(ix+5)
	or a
	sbc hl,de
	jr c,GSP_done
	pop ix
	ld hl,(__gfx_pix)
	push hl  ; redirect call to __gfx_pix
	ret
GSP_done:
	pop ix
	ret
	