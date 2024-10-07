#include <math.h>
#include "libm.h"

float hypotf(float x, float y) {
	return sqrtf(x*x + y*y);
}
