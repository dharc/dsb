#include "nid.h"

NID local_base;

int dsb_nid_init() {
	local_base.type = NID_USER;
	local_base.ll = 0;
	return 0;
}

int dsb_nid_final() {
	return 0;
}

int dsb_nid_allocate(NID *nid) {
	*nid = local_base;
	local_base.b++;
	return 0;
}

int dsb_nid_free(NID *nid) {
	return -1;
}

int dsb_nid_compare(const NID *a, const NID *b) {
	int res = a->type - b->type;
	if (res != 0) return res;
	res = a->a - b->a;
	if (res != 0) return res;
	res = a->b - b->b;
	return res;
}

