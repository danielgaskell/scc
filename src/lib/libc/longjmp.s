	.code
.export _longjmp
_longjmp:
	pop bc      ; return address
	pop ix      ; save pointer
	pop de      ; status
	ld c,(ix+0) ; BC = old return address
	ld b,(ix+1)
	ld l,(ix+2)
	ld h,(ix+3)
	ld sp,hl    ; SP = old SP 
	ex de,hl    ; HL = status
	push bc
	ret         ; PC = old PC
	