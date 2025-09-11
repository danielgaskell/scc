	.code

; Int_Disable
.export _Int_Disable
_Int_Disable:
	di
	ret
	
; Int_Enable
.export _Int_Enable
_Int_Enable:
	ei
	ret
