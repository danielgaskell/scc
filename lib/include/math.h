#ifndef _MATH_H
#define _MATH_H

#include <stdint.h>

/* Large elements of this are drawn from MUSL */

extern int signgam;

/* We have double defined as float .. so fix up the support routines */
#define acos(a)		acosf(a)
#define asin(a)		asinf(a)
#define atan(a)		atanf(a)
#define atan2(a,b)	atan2f(a,b)
#define cbrt(a)		cbrtf(a)
#define ceil(a)		ceilf(a)
#define copysign(a,b)	copysignf(a,b)
#define cos(a)		cosf(a)
#define cosh(a)		coshf(a)
#define erf(a)		erff(a)
#define exp(a)		expf(a)
#define expm1(a)	expm1f(a)
#define fabs(a)		fabsf(a)
#define fdim(a,b)	fdimf(a,b)
#define floor(a)	floorf(a)
#define fmax(a,b)	fmaxf(a,b)
#define fmin(a,b)	fminf(a,b)
#define fmod(a,b)	fmodf(a,b)
#define frexp(a,b)	frexpf(a,b)
#define hypot(a,b)	hypotf(a,b)
#define ilogb(a)	ilogbf(a)
#define j0(a)		j0f(a)
#define j1(a)		j1f(a)
#define jn(a,b)		jnf(a,b)
#define ldexp(a,b)	ldexpf(a,b)
#define lgamma(a)	lgammaf(a)
#define lgamma_r(a,b)	lgammaf_r(a,b)
#define log(a)		logf(a)
#define log10(a)	log10f(a)
#define log1p(a)	log1pf(a)
#define log2(a)		log2f(a)
#define logb(a)		logbf(a)
#define lrint(a)	lrintf(a)
#define lround(a)	lroundf(a)
#define modf(a,b)	modff(a,b)
#define nan(a)		nanf(a)
#define nearbyint(a)	nearbyintf(a)
#define nextafter(a,b)	nextafterf(a,b)
#define pow(a,b)	powf(a,b)
#define remainder(a,b)	remainderf(a,b)
#define remquo(a,b,c)	remquof(a,b,c)
#define rint(a)		ceil(a)
#define rintf(a)	ceil(a)
#define round(a)	roundf(a)
#define scalbln(a,b)	scalblnf(a,b)
#define scalbn(a,b)	scalblf(a,b)
#define sin(a)		sinf(a)
#define sinh(a)		sinhf(a)
#define sqrt(a)		sqrtf(a)
#define tan(a)		tanf(a)
#define tanh(a)		tanhf(a)
#define tgamma(a)	tgammaf(a)
#define trunc(a)	truncf(a)
#define y0(a)		y0f(a)
#define y1(a)		y1f(a)
#define yn(a,b)		ynf(a,b)

#define isinf(x) ((__float_bits(x) & 0x7fffffff) == 0x7f800000)
#define isnan(x) ((__float_bits(x) & 0x7fffffff) > 0x7f800000)
#define isnormal(x) (((__float_bits(x)+0x00800000) & 0x7fffffff) >= 0x0100000)
#define isfinite(x) ((__float_bits(x) & 0x7fffffff) < 0x7f800000)

#define signbit(x) ((int)(__float_bits(x)>>31))

extern float acosf(float);
extern float acoshf(float);
extern float asinf(float);
extern float asinhf(float);
extern float atanf(float);
extern float atan2f(float, float);
extern float atanhf(float);
extern float atof(const char *);
extern float cbrtf(float);
extern float ceilf(float);
extern float copysignf(float, float);
extern float cosf(float);
extern float coshf(float);
extern float erff(float);
extern float expf(float);
extern float expm1f(float);
extern float fabsf(float);
extern float fdimf(float, float);
extern float floorf(float);
extern float fmaxf(float, float);
extern float fminf(float, float);
extern float fmodf(float, float);
extern float frexpf(float, int *);
extern float hypotf(float, float);
extern int ilogbf(float);
extern float j0f(float);
extern float j1f(float);
extern float jnf(int, float);
extern float ldexpf(float, int);
extern float lgammaf(float);
extern float lgammaf_r(float, int *);
extern float logf(float);
extern float log10f(float);
extern float log1pf(float);
extern float log2f(float);
extern float logbf(float);
extern long lrintf(float);
extern long lroundf(float);
extern float modff(float, float *);
extern float nanf(const char *__tagp);
extern float nearbyintf(float);
extern float nextafterf(float, float);
extern float powf(float, float);
extern float remainderf(float, float);
extern float remquof(float, float, int *);
extern float roundf(float);
extern float scalblf(float, long);
extern float scalbnf(float, int);
extern float sinf(float);
extern float sinhf(float);
extern float sqrtf(float);
extern float tanf(float);
extern float tanhf(float);
extern float tgammaf(float);
extern float truncf(float);
extern float y0f(float);
extern float y1f(float);
extern float ynf(int, float);
extern unsigned int __float_bits(float);

/* FIXME: sort out the right NaN's */
#define __sNaN       0x1.fffff0p128
#define __NaN        0x1.fffff0p128

#define __FINFINITY 1e40f

#define INFINITY	__FINFINITY
#define NAN		__NaN

#define M_E		2.7182818284590452354
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		0.69314718055994530942
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define __1M_PI_2	1.57079632679489661923
#define __2M_PI_2	3.14159265358979323846
#define __3M_PI_2	4.71238898038468985769
#define __4M_PI_2	6.28318530717958647692

#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT_1_2	0.70710678118654752440

#define HUGE		3.40282347e+38F
#define HUGE_VALF	INFINITY
#define HUGE_VAL	((double)INFINITY)

#define MAXFLOAT	3.40282347e+38F

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#define FP_ILOGBNAN	(-1-(int)(((unsigned)-1)>>1))
#define FP_ILOGB0	FP_ILOGBNAN


#endif
