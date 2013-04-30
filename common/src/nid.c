#include "dsb/nid.h"
#include "dsb/errors.h"

struct NID local_base;

int dsb_nid_init()
{
	local_base.type = NID_USER;
	local_base.ll = 0;
	return SUCCESS;
}

int dsb_nid_final()
{
	return SUCCESS;
}

int dsb_nid_allocate(struct NID *nid)
{
	*nid = local_base;
	local_base.ll++;
	return SUCCESS;
}

int dsb_nid_free(const struct NID *nid)
{
	return ERR_NID_FREE;
}

int dsb_nid_compare(const struct NID *a, const struct NID *b)
{
	if (a->type < b->type) return -1;
	if (a->type > b->type) return 1;
	if (a->a < b->a) return -1;
	if (a->a > b->a) return 1;
	if (a->b < b->b) return -1;
	if (a->b > b->b) return 1;
	return 0;
}

