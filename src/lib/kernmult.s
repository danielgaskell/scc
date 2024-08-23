	.code

; Multitasking_GetCounter (1)
.export _Sys_Counter
_Sys_Counter:
	push ix
	push iy
	ld hl,#0x8109
	rst #0x28
	ld (__hireg),iy
	push ix
	pop hl
	pop iy
	pop ix
	ret

; Multitasking_GetCounter (2)
.export _Sys_IdleCount
_Sys_IdleCount:
	push ix
	push iy
	ld hl,#0x8109
	rst #0x28
	ld h,d
	ld l,e
	pop iy
	pop ix
	ret

