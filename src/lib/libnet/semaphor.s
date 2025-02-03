.code

__netmsgsema:   .byte 0 ; multithreading semaphore for _netmsg

; _nsemaon(): _netmsg semaphore on
.export __nsemaon
__nsemaon:
	ld a,(__netmsgsema)
	or a
	jr nz,__nsemaon
	ld a,1
	ld (__netmsgsema),a
	ret
	
; _nsemaoff(): _netmsg semaphore off
.export __nsemaoff
__nsemaoff:
	ld a,0
	ld (__netmsgsema),a
	ret
