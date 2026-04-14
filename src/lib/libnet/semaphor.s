.code

__netmsgsema:   .byte 0 ; multithreading semaphore for _netmsg

; _nsemaon(): _netmsg semaphore on
.export __nsemaon
__nsemaon:
	ld a,(__netmsgsema)
	or a
	jr z,__nsemaon1
	rst 0x30
	jr __nsemaon
__nsemaon1:
	ld a,1
	ld (__netmsgsema),a
	ret
	
; _nsemaoff(): _netmsg semaphore off
.export __nsemaoff
__nsemaoff:
	ld a,0
	ld (__netmsgsema),a
	ret

__packsema:		.byte 0 ; multithreading semaphore for _netpacket

; _packsemaon(): _netpacket semaphore on
.export __packsemaon
__packsemaon:
	ld a,(__packsema)
	or a
	jr z,__packsemaon1
	rst 0x30
	jr __packsemaon
__packsemaon1:
	ld a,1
	ld (__packsema),a
	ret
	
; _packsemaoff(): _netpacket semaphore off
.export __packsemaoff
__packsemaoff:
	ld a,0
	ld (__packsema),a
	ret

