.z80
.code

; Adapted from z80float by Zeda Thomas (Apache license).

.export scrap
scrap:
.export f32bgi_a
f32bgi_a:
	.word 0,0
.export f32bgi_g
f32bgi_g:
	.word 0,0
.export f32bgi_b
f32bgi_b:
	.word 0,0
.export f32bgi_c
f32bgi_c:
	.word 0,0

.export f32_const_pi
f32_const_pi:
  .db #0xDB,#0x0F,#0x49,#0x40  ;3.14159265
.export f32_const_2pi
f32_const_2pi:
  .db #0xDB,#0x0F,#0xC9,#0x40  ;6.28318530
.export f32_const_pi_div_2
f32_const_pi_div_2:
  .db #0xDB,#0x0F,#0xC9,#0x3F  ;6.28318530
.export f32_const_0
f32_const_0:
  .db #0x00,#0x00,#0x00,#0x00  ;0
.export f32_const_1
f32_const_1:
  .db #0x00,#0x00,#0x80,#0x3F  ;1
.export f32_const_n1
f32_const_n1:
  .db #0x00,#0x00,#0x80,#0xBF  ;-1
.export f32_const_NaN
f32_const_NaN:
  .db #0xFF,#0xFF,#0xFF,#0x7F  ;NaN
.export f32_const_inf
f32_const_inf:
  .db #0x00,#0x00,#0x80,#0x7F  ;inf
.export f32_const_ninf
f32_const_ninf:
  .db #0x00,#0x00,#0x80,#0xFF  ;-inf
.export f32_const_ln2
f32_const_ln2:
  .db #0x18,#0x72,#0x31,#0x3F  ;.6931471806
.export f32_const_lg_e
f32_const_lg_e:
  .db #0x3B,#0xAA,#0xB8,#0x3F  ;1.442695041
.export f32_const_log10_e
f32_const_log10_e:
  .db #0xD9,#0x5B,#0xDE,#0x3E  ;0.4342944819
.export f32_const_p5
f32_const_p5:
  .db #0x00,#0x00,#0x00,#0x3F  ;.5
.export f32_const_p25
f32_const_p25:
  .db #0x00,#0x00,#0x80,#0x3E  ;.25
.export f32_const_2pi_inv
f32_const_2pi_inv:
  .db #0x83,#0xF9,#0x22,#0x3E  ;.1591549431
.export f32_const_lg_10
f32_const_lg_10:
  .db #0x78,#0x9A,#0x54,#0x40  ;3.321928095
  