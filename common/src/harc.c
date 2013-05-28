#include "dsb/harc.h"
#include "dsb/nid.h"
#include "dsb/errors.h"

HARC_t *dsb_harc(const NID_t *t1, const NID_t *t2, HARC_t *harc)
{
	harc->t1 = *t1;
	harc->t2 = *t2;
	harc->deps = 0;
	harc->e = 0;
	harc->def.type = 0;
	harc->def.ll = 0;
	harc->flags = HARC_OUTOFDATE;
	return harc;
}


