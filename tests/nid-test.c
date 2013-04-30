#include "dsb/test.h"
#include "dsb/nid.h"

void test_nid_compare()
{
	struct NID a;
	struct NID b;

	a.type = NID_INTEGER;
	a.ll = 555;
	b.type = NID_INTEGER;
	b.ll = 555;
	CHECK(dsb_nid_compare(&a,&b) == 0);

	a.ll = 556;
	CHECK(dsb_nid_compare(&a,&b) > 0);
	b.ll = 557;
	CHECK(dsb_nid_compare(&a,&b) < 0);

	a.ll = 557;
	a.type = NID_REAL;
	CHECK(dsb_nid_compare(&a,&b) != 0);

	DONE;
}

void test_nid_compareextreme()
{
	struct NID a;
	struct NID b;

	a.type = NID_INTEGER;
	a.ll = 55;
	b.type = NID_INTEGER;
	b.ll = 0x7FFFFFFFFFFFFFFF;
	CHECK(dsb_nid_compare(&a,&b) < 0);
	DONE;
}

void test_nid_allocate()
{
	struct NID a;

	dsb_nid_init();
	CHECK(dsb_nid_allocate(&a) == 0);
	CHECK(a.type == NID_USER);
	CHECK(a.ll == 0);
	CHECK(dsb_nid_allocate(&a) == 0);
	CHECK(a.ll == 1);
	dsb_nid_final();
	DONE;
}

int main(int argc, char *argv[])
{
	dsb_test(test_nid_compare);
	dsb_test(test_nid_compareextreme);
	dsb_test(test_nid_allocate);
	return 0;
}
