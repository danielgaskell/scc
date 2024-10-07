/* origin: FreeBSD /usr/src/lib/msun/src/s_tanhf.c */
/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */


#include <math.h>
#include "libm.h"

float tiny = 1.0e-30;
float one = 1.0, two = 2.0, huge = 1.0e30;

float tanhf(float x)
{
	float t, z;
	int32_t jx, ix;

	GET_FLOAT_WORD(jx, x);
	ix = jx & 0x7fffffffL;

    /* x is INF or NaN */
	if (ix >= 0x7f800000L) {
	    if (jx >= 0) return one/x+one;    /* tanh(+-inf)=+-1 */
	    else         return one/x-one;    /* tanh(NaN) = NaN */
	}

    /* |x| < 9 */
	if (ix < 0x41100000L) {		 /* |x|<9 */
	    if (ix < 0x39800000L) {	 /* |x|<2**-12 */
            if (huge + x > one) return x; /* tanh(tiny) = tiny with inexact */
	    }
	    if (ix >= 0x3f800000L) {	 /* |x|>=1  */
            t = expf(two*fabsf(x)) - 1.0;
            z = one - two/(t+two);
	    } else {
	        t = expf(-two*fabsf(x)) - 1.0;
	        z = -t/(t+two);
	    }
    /* |x| >= 9, return +-1 */
	} else {
	    z = one - tiny;		/* raise inexact flag */
	}
	return (jx >= 0) ? z : -z;
}
