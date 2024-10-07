/* Round float to integer away from zero. */

#include "libm.h"

float roundf(float x) {
    if (x > 0.0)
        return floorf(x + 0.5);
    else
        return ceilf(x - 0.5);
}
