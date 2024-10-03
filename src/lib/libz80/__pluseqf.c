#include "libfp.h"

uint32_t _pluseqf(uint32_t a1, uint32_t* var)
{
	*var = _plusf(a1, *var);
	return *var;
}
