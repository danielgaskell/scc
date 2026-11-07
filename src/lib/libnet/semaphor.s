.code

__netmsgsema:   .byte 0 ; multithreading semaphore for _netmsg

; _nsemaon(): _netmsg semaphore on
.export __nsemaon
__nsemaon:
	di
	ld a,(__netmsgsema)
	or a
	jr z,__nsemaon1
	ei
	rst 0x30
	jr __nsemaon
__nsemaon1:
	inc a
	ld (__netmsgsema),a
	ei
	ret
	
; _nsemaoff(): _netmsg semaphore off
.export __nsemaoff
__nsemaoff:
	xor a
	ld (__netmsgsema),a
	ret

__packsema:		.byte 0 ; multithreading semaphore for _netpacket

; _packsemaon(): _netpacket semaphore on
.export __packsemaon
__packsemaon:
	di
	ld a,(__packsema)
	or a
	jr z,__packsemaon1
	ei
	rst 0x30
	jr __packsemaon
__packsemaon1:
	inc a
	ld (__packsema),a
	ei
	ret
	
; _packsemaoff(): _netpacket semaphore off
.export __packsemaoff
__packsemaoff:
	xor a
	ld (__packsema),a
	ret

