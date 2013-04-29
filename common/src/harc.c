#include "dsb/harc.h"

struct NID *dsb_harc_major(const struct HARC *harc)
{
	if (dsb_nid_compare(harc->a,harc->b) >= 0)
	{
		return &harc->a;
	}
	else
	{
		return &harc->b;
	}
}

