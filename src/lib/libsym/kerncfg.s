	.code

; Sys_GetConfig
.export _Sys_GetConfig
_Sys_GetConfig:
	push bc
	push ix
	push iy
	ld ix,8
	add ix,sp
	ld d,(ix+4)
	ld l,(ix+2)
	ld h,(ix+3)
	push hl
	pop iy
	ld l,(ix+0)
	ld h,(ix+1)
	push hl
	pop ix
	ld e,5
	ld hl,#0x8103
	rst #0x28
	pop iy
	pop ix
	pop bc
	ret
