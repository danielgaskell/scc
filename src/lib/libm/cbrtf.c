/* cbrtf(x)
 * Return cube root of x
 */

#include <math.h>
#include "libm.h"

float cbrtf(float x) {
    if (x == 0.0)
        return 0.0;
    else if (x > 0.0)
        return powf(x, 0.33333333);
    else
        return powf(-x, 0.33333333);
}
