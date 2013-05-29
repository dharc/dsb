#include "dsb/test.h"
#include "dsb/nid.h"

struct Event;

int dsb_send(struct Event *evt)
{
	return 0;
}

void test_nid_iton()
{
	NID_t n;

	dsb_iton(6767,&n);
	CHECK(dsb_ntoi(&n) == 6767);
	CHECK(n.header == 0);
	CHECK(n.t == NID_INTEGER);
	CHECK(n.ll == 6767);

	DONE;
}

void test_nid_cton()
{
	NID_t n;

	dsb_cton('a',&n);
	CHECK(dsb_ntoc(&n) == 'a');
	CHECK(n.header == 0);
	CHECK(n.t == NID_CHARACTER);
	CHECK(n.chr == 'a');

	DONE;
}

void test_nid_null()
{
	NID_t n;

	dsb_nid_null(&n);
	CHECK(n.header == 0);
	CHECK(n.t == 0);
	CHECK(n.ll == 0);

	DONE;
}

void test_nid_nid()
{
	NID_t n;

	dsb_nid(67,67,&n);
	CHECK(n.header == 0);
	CHECK(n.t == 67);
	CHECK(n.ll == 67);

	DONE;
}

void test_nid_eq()
{
	NID_t a;
	NID_t b;

	a.header = 5;
	b.header = 5;
	a.t = 89;
	b.t = 89;
	a.ll = 4545;
	b.ll = 4545;

	CHECK(dsb_nid_eq(&a,&b) == 1);
	b.ll = 4546;
	CHECK(dsb_nid_eq(&a,&b) == 0);
	b.t = 90;
	b.ll = 4545;
	CHECK(dsb_nid_eq(&a,&b) == 0);
	b.header = 6;
	b.t = 89;
	CHECK(dsb_nid_eq(&a,&b) == 0);

	DONE;
}

void test_nid_leq()
{
	NID_t a;
	NID_t b;

	a.header = 5;
	b.header = 5;
	a.t = 89;
	b.t = 89;
	a.ll = 4545;
	b.ll = 4545;

	CHECK(dsb_nid_leq(&a,&b) == 1);
	b.ll = 4546;
	CHECK(dsb_nid_leq(&a,&b) == 1);
	b.t = 90;
	b.ll = 4545;
	CHECK(dsb_nid_leq(&a,&b) == 1);
	b.header = 6;
	b.t = 89;
	CHECK(dsb_nid_leq(&a,&b) == 1);
	b.header = 5;
	b.ll = 4544;
	CHECK(dsb_nid_leq(&a,&b) == 0);
	b.t = 88;
	b.ll = 4545;
	CHECK(dsb_nid_leq(&a,&b) == 0);
	b.header = 4;
	b.t = 89;
	CHECK(dsb_nid_leq(&a,&b) == 0);

	DONE;
}

void test_nid_geq()
{
	NID_t a;
	NID_t b;

	a.header = 5;
	b.header = 5;
	a.t = 89;
	b.t = 89;
	a.ll = 4545;
	b.ll = 4545;

	CHECK(dsb_nid_geq(&a,&b) == 1);
	b.ll = 4546;
	CHECK(dsb_nid_geq(&a,&b) == 0);
	b.t = 90;
	b.ll = 4545;
	CHECK(dsb_nid_geq(&a,&b) == 0);
	b.header = 6;
	b.t = 89;
	CHECK(dsb_nid_geq(&a,&b) == 0);
	b.header = 5;
	b.ll = 4544;
	CHECK(dsb_nid_geq(&a,&b) == 1);
	b.t = 88;
	b.ll = 4545;
	CHECK(dsb_nid_geq(&a,&b) == 1);
	b.header = 4;
	b.t = 89;
	CHECK(dsb_nid_geq(&a,&b) == 1);

	DONE;
}

/*void test_nid_allocate()
{
	struct NID a;

	dsb_nid_init();
	CHECK(dsb_nid_allocate(&a) == 0);
	CHECK(a.t == NID_USER);
	CHECK(a.ll == 0);
	CHECK(dsb_nid_allocate(&a) == 0);
	CHECK(a.ll == 1);
	dsb_nid_final();
	DONE;
}*/

int main(int argc, char *argv[])
{
	//dsb_test(test_nid_compare);
	//dsb_test(test_nid_compareextreme);
	dsb_test(test_nid_iton);
	dsb_test(test_nid_cton);
	dsb_test(test_nid_null);
	dsb_test(test_nid_nid);
	dsb_test(test_nid_eq);
	dsb_test(test_nid_leq);
	dsb_test(test_nid_geq);
	//dsb_test(test_nid_allocate);
	return 0;
}
