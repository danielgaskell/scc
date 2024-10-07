/*
 * truncf(x)
 * Return x rounded toward 0 to integral value
 */

#include <math.h>
#include "libm.h"

float truncf(float x) {
    if (x > 0.0)
        return floorf(x);
    else
        return ceilf(x);
}
