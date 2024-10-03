#include "libfp.h"

uint32_t _muleqf(uint32_t a1, uint32_t* var)
{
	*var = _mulf(a1, *var);
	return *var;
}
