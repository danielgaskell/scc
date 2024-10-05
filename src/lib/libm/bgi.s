.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export f32bgi
f32bgi:
;1/BG(x,y) where bg(x,y) is the Borchardt-Gauss Mean of x and y
;
;This approximation computes 2835/(-a0 + 84*a1 - 1344*a2 + 4096*a3) where:
;   a0 = x, g0 = y
;   a_(n+1) = (a_n + g_n)/2
;   g_(n+1) = sqrt(a_(n+1) * g_n)
;
;Optimized a bit, this looks like:
;  b=a/2048
;  a=.5*(a+g)
;  c=21*a/512
;  c-=b
;  g=sqrt(a*g)
;  a=.5*(a+g)
;  g=sqrt(a*g)
;  a*=11/32
;  a+=c
;  a+=g
;  return 1.38427734375/a
;
  call __pushpopf
  push bc
  ;move x to A for safe keeping
  push de
  ld de,f32bgi_a
  ldi
  ldi
  ld a,(hl)
  add a,a
  ldi
  ld a,(hl)
  adc a,a
  ldi
  pop hl
  ; jp z,f32bgi_return_NaN
  inc a
  jr z,f32bgi_infnan_op2

  ld de,f32bgi_g
  ldi
  ldi
  ld a,(hl)
  add a,a
  ldi
  ld a,(hl)
  ldi
  adc a,a
  jp z,f32bgi_op1_0
  inc a
  jp z,f32bgi_op2_infnan

; b=a/2048
  ld a,-11
  ; ld bc,f32bgi_b
  ld b,d
  ld c,e
  ld hl,f32bgi_a
  call f32mulpow2

; a=.5*(a+g)
  ld hl,f32bgi_a
  ld de,f32bgi_g
  ld b,h
  ld c,l
  call f32amean

; c=21*a/512
  ld hl,f32bgi_a
  ld de,15F7h    ;D is 21, E is -9
  ld bc,f32bgi_c
  call mulu8_dp2

; c-=b
  ld h,b
  ld l,c
  ld de,f32bgi_b
  call f32sub

; g=sqrt(a*g)
  ld hl,f32bgi_a
  ld de,f32bgi_g
  ld b,d
  ld c,e
  call f32geomean

; a=.5*(a+g)
  ld b,h
  ld c,l
  call f32amean

; g=sqrt(a*g)
  ld b,d
  ld c,e
  call f32geomean

; a*=11/32
  ld de,0BFBh    ;D is 1, E is -5
  ld b,h
  ld c,l
  call mulu8_dp2

; a+=c
  ld de,f32bgi_c
  call f32add

; a+=g
  ld de,f32bgi_g
  call f32add

  ex de,hl
  ld hl,f32_cont_1p38427734375
  pop bc
  jp f32div

f32_cont_1p38427734375:
	.db #0x00,#0x30,#0xB1,#0x3F  ;1.38427734375



f32bgi_infnan_op2:
;1/BG(-inf, y) ==> NaN
  jr c,f32bgi_return_NaN
;1/BG(NaN, x) ==> NaN
  ex de,hl
  dec hl
  ld c,(hl)
  dec hl
  ld a,(hl)
  add a,a
  dec hl
  or (hl)
  dec hl
  or (hl)
  jr nz,f32bgi_return_NaN

;1/BG(+inf, 0)    ==> NaN
;1/BG(+inf, fin)  ==> 0
;1/BG(+inf, -inf) ==> NaN
;1/BG(+inf, NaN)  ==> NaN
;1/BG(+inf, +inf) ==> 0
  ex de,hl
  ld a,(hl)
  inc hl
  or (hl)
  inc hl
  ld b,a
  ld a,(hl)
  add a,a
  ld d,a
  inc hl
  ld a,(hl)
  adc a,a
  jr z,f32bgi_return_NaN
;1/BG(+inf, -x) ==> NaN
  jr c,f32bgi_return_NaN
  inc a
  jr nz,f32bgi_return_0
;1/BG(+inf, NaN)  ==> NaN
  ld a,d
  or b
  jr nz,f32bgi_return_NaN
f32bgi_return_0:
  ld hl,f32_const_0
  jr f32bgi_return

f32bgi_op2_infnan:
;1/BG(x,+-inf) ==> 0
;1/BG(x, NaN) ==> NaN
  dec hl
  dec hl
  ld a,(hl)
  add a,a
  dec hl
  or (hl)
  dec hl
  or (hl)
  jr z,f32bgi_return_0
f32bgi_op1_0:
; 1/BG(x, 0) ==> NaN
f32bgi_return_NaN:
  ld hl,f32_const_NaN
f32bgi_return:
  pop de
  jp mov4

mov4:
  ldi
  ldi
  ldi
  ldi
  ret
  