#include "libfp.h"

uint32_t _minuseqf(uint32_t a1, uint32_t* var)
{
	*var = _minusf(a1, *var);
	return *var;
}
