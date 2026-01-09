#include <stdlib.h>

static inline void *
custom_mco_alloc(size_t size)
{
	void *ptr;

	if (!(ptr = calloc(1, size)))
		abort();

	return ptr;
}

static inline void
custom_mco_dealloc(void *ptr, size_t)
{
	free(ptr);
}

#define MCO_ALLOC custom_mco_alloc
#define MCO_DEALLOC custom_mco_dealloc
#define MCO_NO_MULTITHREAD

#define MINICORO_IMPL
#include "minicoro.h"
