#include "libfp.h"

uint32_t _diveqf(uint32_t a1, uint32_t* var)
{
	*var = _divf(a1, *var);
	return *var;
}
