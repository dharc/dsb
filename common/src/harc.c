#include "dsb/harc.h"
#include "dsb/nid.h"

int dsb_harc_gen(const struct NID *pa, const struct NID *pb, struct HARC *ph)
{
	int nidc = dsb_nid_compare(pa,pb);
	if (nidc < 0)
	{
		ph->a1 = pa->type;
		ph->a2 = pb->type;
		ph->b = pa->ll;
		ph->c = pb->ll;
	}
	else
	{
		ph->a2 = pa->type;
		ph->a1 = pb->type;
		ph->c = pa->ll;
		ph->b = pb->ll;
	}
	return 0;
}

int dsb_harc_compare(const struct HARC *pa, const struct HARC *pb)
{
	if (pa->a < pb->a) return -1;
	if (pa->a > pb->a) return 1;
	if (pa->b < pb->b) return -1;
	if (pa->b > pb->b) return 1;
	if (pa->c < pb->c) return -1;
	if (pa->c > pb->c) return 1;
	return 0;
}

