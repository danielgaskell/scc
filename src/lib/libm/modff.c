#include <math.h>
#include "libm.h"

float modff(float x, float *y) {
    *y = floorf(x);
    return x - *y;
}
