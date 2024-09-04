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
__exehead_extcode:	.word 4096
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

; entry point
start2:
	; initialize malloc heap
	ld hl,__codestart
	ld de,(__segcodelen)
	add hl,de
	ld de,256 ; for EXE header
	or a ; reset carry
	sbc hl,de
	ld (__malloc_max),hl
	ld de,(__heapsize)
	or a ; reset carry
	sbc hl,de
	ld de,256 ; for path, which overwrites codeextra for some reason (?)
	add hl,de
	ld (__malloc_heap),hl
	ld (__malloc_top),hl
	
	; load argv and call main
	call __load_argv
	ld hl,__argv
	push hl
	ld hl,(__argc+0)
	push hl
	call _main		; go
	
	; exit on return
	call _exit

; exit code
.export _exit
_exit:
	; if fopen() has set __exit_stdio, close all open files
	; (this indirect method prevents stdio from being linked via crt0.s unless actually used)
	ld a,(__exit_stdio+0)
	ld b,a
	ld a,(__exit_stdio+1)
	or b
	jr z,_symexit1
	ld hl,(__exit_stdio+0)
	push hl
	push hl
	ret					; simulate 'call hl'
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

; Message_Sleep_And_Receive
.export _Msg_Sleep
_Msg_Sleep:
	push ix
	push iy
	ld ix,0
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
	ret
	
; Message_Send
.export _Msg_Send
_Msg_Send:
	push ix
	push iy
	ld ix,0
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
	ret

; Message_Receive
.export _Msg_Receive
_Msg_Receive:
	push ix
	push iy
	ld ix,0
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
	ret

; Multitasking_SoftInterrupt
.export _Idle
_Idle:
	rst #0x30
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
.export __exit_stdio
__exit_stdio:
	.word 0

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

; program stack and initialization space
			.ds 256
.export __symstack
__symstack:
			.ds 12			;register predefinition
			.word start2	;routine start address
.export __sympid
__sympid:	.byte 0
