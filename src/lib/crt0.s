	.code
__codestart:
	
; EXE header (256 bytes)
.export __symheader
.export __segcodelen
.export __segdatalen
.export __segtranslen
__symheader:
__segcodelen:		.word 0
__segdatalen:		.word 0
__segtranslen:		.word 0
__exehead_origin:
__exehead_dataaddr:	.word 0
__exehead_reloc:
__exehead_transaddr:.word 0
__exehead_stack:	.word __symstack-__segtrans
					.word 0
.export __symbank
__symbank:			.byte 0
__exehead_name:		.byte 65	; App
					.byte 112
					.byte 112
					.ds 22
__exehead_flags:	.byte 0
__exehead_icon16:	.word 0
					.ds 5
__exehead_exeid:	.byte 83	; SymExe10
					.byte 121
					.byte 109
					.byte 69
					.byte 120
					.byte 101
					.byte 49
					.byte 48
__exehead_extcode:	.word 4352
__exehead_extdata:	.word 0
__exehead_exttrans:	.word 0
					.ds 26
.export __symappid
__symappid:
__exehead_sversion:	.byte 0
__exehead_pid:
__exehead_bversion:	.byte 2
__exehead_iconsm:	.ds 19
__exehead_iconlg:
	.byte #0x06, #0x18, #0x18, #0xF0, #0xF0, #0xF0, #0xF0, #0xF0, #0xF0, #0xD7
	.byte #0xFF, #0xFF, #0xFF, #0xFF, #0xB4, #0x87, #0xFA, #0xF5, #0xF7, #0xFF
	.byte #0x5A, #0xA7, #0xFF, #0xFF, #0xFF, #0xFF, #0xB4, #0xF0, #0xF0, #0xF0
	.byte #0xF0, #0xF0, #0xF0, #0x87, #0x0F, #0x0F, #0x0F, #0x0F, #0x1E, #0xB4
	.byte #0xF0, #0xF0, #0xF0, #0xF0, #0xD2, #0xB4, #0xF0, #0xF0, #0xF0, #0xF0
	.byte #0xD2, #0xB4, #0xF0, #0xF0, #0xF0, #0xF0, #0xD2, #0xB4, #0xF0, #0xF0
	.byte #0xF0, #0xF0, #0xD2, #0xB4, #0x00, #0xF0, #0xB0, #0xF0, #0xD2, #0xA4
	.byte #0x60, #0x70, #0xB0, #0xB0, #0xD2, #0xA4, #0x60, #0x40, #0x90, #0x90
	.byte #0xD2, #0xA4, #0x00, #0x40, #0xD0, #0xC0, #0xD2, #0xA4, #0x60, #0x70
	.byte #0xC0, #0xC0, #0xD2, #0xA4, #0x60, #0x40, #0xE0, #0x90, #0xD2, #0xA4
	.byte #0x60, #0x40, #0xE0, #0xB0, #0xD2, #0xB4, #0xF0, #0xF0, #0xF0, #0xF0
	.byte #0xD2, #0xB4, #0xF0, #0xF0, #0xF0, #0xF0, #0xD2, #0xB4, #0xF0, #0xF0
	.byte #0xF0, #0xF0, #0xD2, #0xB4, #0xF0, #0xF0, #0xF0, #0xF0, #0xD2, #0xB4
	.byte #0xF0, #0xF0, #0xF0, #0xF0, #0xD2, #0x87, #0x0F, #0x0F, #0x0F, #0x0F
	.byte #0x1E, #0xF0, #0xF0, #0xF0, #0xF0, #0xF0, #0xF0

.export __segcode
__segcode:
	.word __codestart
__heapsize:			; note: must be at byte 258! written by linker
	.word 4096
.export __debugtrace
__debugtrace:       ; note: must be at byte 260!
	.word 0
.export __debugstack
__debugstack:       ; note: must be at byte 262!
	.word 0

; entry point
start2:
	; initialize malloc heap
	ld hl,__codestart
	ld de,(__segcodelen)
	add hl,de
	ld de,256 ; for EXE header
	or a
	sbc hl,de
	ld (__malloc_max),hl
	ld de,(__heapsize)
	or a
	sbc hl,de
	ld de,256 ; for path, which overwrites codeextra for some reason (?)
	add hl,de
	ld (__malloc_heap),hl
	ld (__malloc_top),hl
	
	; initialize __symversion
	ld e,7					; first get bank of version string -> IYL
	ld hl,#0x8103
	rst #0x28
	push iy
	ld e,8					; then get version string address -> IY
	ld hl,#0x8103
	rst #0x28
	push iy
	pop hl
	pop de
	ld a,e
	rst #0x20				; Banking_ReadWord
	.word #0x8124
	ld hl,0
	ld d,0
	ld e,c
	ld a,b
	ld b,10
mult10:
	add hl,de				; major version = tens digit
	djnz mult10
	ld e,a
	add hl,de				; minor version = ones digit
	ld (__symversion),hl
	
	; load argv and call main
	call __load_argv
	ld hl,__argv
	push hl
	ld hl,(__argc+0)
	push hl
	call _main		; go
	
	; exit on return
	push hl         ; pass return value to _exit
	call _exit

; exit code
.export _exit
_exit:
	; run any exit hooks that have been defined
_symexithooks:
	ld hl,(_exit_hook)
	ld c,(hl)
	inc hl
	ld b,(hl)
	inc hl
	ld (_exit_hook),hl
	ld a,b
	or c
	jr z,_symexit1
	ld hl,_symexithooks+0
	push hl
	push bc
	ret					; simulate 'call bc'
_symexit1:
	; if a shell is open, close it
	ld a,(__shellpid)
	and a
	jr z,_symexit2
	; send MSC_SHL_EXIT
	ld a,(__sympid)
	.byte #0xDD			; ld ixl,a
	ld l,a
	ld a,(__shellpid)
	.byte #0xDD			; ld ixh,a
	ld h,a
	ld iy,__symmsgbuf
	ld (iy+0),#0x44
	ld (iy+1),#0x00
	pop bc
	pop bc              ; get status from original _exit stack
	ld (iy+2),c			; status is passed back in unused bytes 2-3, for anything that wants it
	rst #0x10
_symexit2:
    ; send MSC_SYS_PRGEND and idle until killed
	ld a,(__sympid)
	.byte #0xDD			; ld ixl,a
	ld l,a
	.byte #0xDD			; ld ixh,3
	ld h,3
	ld iy,__symmsgbuf
    ld a,(__symappid)
	ld (iy+0),#0x11
	ld (iy+1),a
	rst #0x10
_symexit_loop:
	rst #0x30
	jr _symexit_loop

; Msg_Sleep
.export _Msg_Sleep
_Msg_Sleep:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld l,(ix+6)
	ld h,(ix+8)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x08
	push ix
	pop hl
	pop iy
	pop ix
	pop bc
	ret
	
; Msg_Send
.export _Msg_Send
_Msg_Send:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld l,(ix+6)
	ld h,(ix+8)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x10
	push ix
	pop hl
	pop iy
	pop ix
	pop bc
	ret

; Msg_Receive
.export _Msg_Receive
_Msg_Receive:
	push bc
	push ix
	push iy
	ld ix,#0x02
	add ix,sp
	ld l,(ix+6)
	ld h,(ix+8)
	push hl
	ld l,(ix+10)
	ld h,(ix+11)
	push hl
	pop iy
	pop ix
	rst #0x18
	push ix
	pop hl
	pop iy
	pop ix
	pop bc
	ret

; Multitasking_SoftInterrupt
.export _Idle
_Idle:
	rst #0x30
	ret
	
; _msgpid() - returns the process ID that represents "current thread" to
; Msg_Send(), etc. For SymbOS >4.0, this is -1. For SymbOS <=4.0, the actual
; current process ID is (HL' - 0x40) / 6. (This relies on knowing the legacy
; structure of the kernel process table, which may change in future versions,
; so for >4.0 the new -1 method is preferred.)
.export __msgpid
__msgpid:
	or a
	ld hl,(__symversion)
	ld de,41
	sbc hl,de
	jr nc,_msgpidneg
	; SymbOS <=4.0 case: actual process ID
	push bc
	di
	exx
	push hl
	exx
	ei
	pop hl
	ld de,#0x40
	or a
	sbc hl,de
	ld d,6
    ; HL = HL/6
	xor a
	ld b,16
_div8loop:
	add hl,hl
	rla
	cp d
	jp c,_div8next
	sub d
	inc l
_div8next:
	djnz _div8loop
	pop bc
	ret
_msgpidneg:
	; SymbOS >4.0 case: -1
	ld l,-1
	ret

; _msemaon(): _symmsg semaphore on
.export __msemaon
__msemaon:
	ld a,(__symmsgsema)
	or a
	jr nz,__msemaon
	ld a,1
	ld (__symmsgsema),a
	ret
	
; _msemaoff(): _symmsg semaphore off
.export __msemaoff
__msemaoff:
	ld a,0
	ld (__symmsgsema),a
	ret

; some data
.export __malloc_heap
__malloc_heap:
	.word 0
.export __malloc_top
__malloc_top:
	.word 0
.export __malloc_max
__malloc_max:
	.word 0
.export __symicon
__symicon:
	.word __exehead_iconlg
.export __exit_hooks
__exit_hooks:
	.ds 16
_exit_hook:
	.word __exit_hooks
.export __symversion
__symversion:
	.word 40

; start of SymbOS data area
	.symdata
.export __segdata
__segdata:
	.word __segdata

; start of SymbOS transfer area
	.symtrans
.export __segtrans
.export __symmsg
.export __symmsgbuf
__segtrans:
	.word __segtrans
__symmsg:		.word __symmsgbuf	; C pointer to actual message buffer
__symmsgbuf:	.ds 14
__symmsgsema:   .byte 0             ; multithreading semaphore for _symmsg

; program stack and initialization space
			.ds 1024
.export __symstack
__symstack:
			.ds 12			;register predefinition
			.word start2	;routine start address
.export __sympid
__sympid:	.byte 0
