#include "dsb/test.h"
#include "dsb/nid.h"
#include "dsb/errors.h"
#include <string.h>
#include <stdio.h>

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

void test_nid_fromstr()
{
	NID_t t;

	CHECK(dsb_nid_fromStr("55",&t) == 0);
	CHECK(t.ll == 55);
	CHECK(t.t == NID_INTEGER);

	CHECK(dsb_nid_fromStr("66\n",&t) == 0);
	CHECK(t.ll == 66);
	CHECK(t.t == NID_INTEGER);

	CHECK(dsb_nid_fromStr("[00:0001:000000000000014d]",&t) == 0);
	CHECK(t.ll == 333);
	CHECK(t.header == 0);
	CHECK(t.t == 1);

	CHECK(dsb_nid_fromStr("[03:55:66:77:88:99:00:0000000444]", &t) == 0);
	CHECK(t.hasMac == 1);
	CHECK(t.persist == 1);
	CHECK(t.mac[0] == 0x55);
	CHECK(t.mac[4] == 0x99);
	CHECK(t.n == 0x444);

	//Check various invalid strings.
	CHECK(dsb_nid_fromStr("[0]",&t) == ERR_NIDSTR);
	CHECK(dsb_nid_fromStr("[03:]",&t) == ERR_NIDSTR);
	CHECK(dsb_nid_fromStr("[03:00:00:0:00]",&t) == ERR_NIDSTR);
	CHECK(dsb_nid_fromStr("[03:00:00:00:00:00:00:4545]",&t) == ERR_NIDSTR);
	CHECK(dsb_nid_fromStr("[03:55:66:77:88:99:00:0000000444", &t) == ERR_NIDSTR);
	CHECK(dsb_nid_fromStr("[0:0:0]", &t) == ERR_NIDSTR);
	CHECK(dsb_nid_fromStr("[00:1:333:777]", &t) == ERR_NIDSTR);

	DONE;
}

void test_nid_toStr()
{
	char buf[100];
	NID_t t;

	t.header = 0;
	t.t = NID_INTEGER;
	t.ll = 4545;
	CHECK(dsb_nid_toStr(&t,buf,100) == 0);
	CHECK(strcmp(buf,"4545") == 0);

	t.t = NID_REAL;
	t.dbl = 34.78;
	CHECK(dsb_nid_toStr(&t,buf,100) == 0);
	CHECK(strcmp(buf,"34.7800") == 0);

	t.t = 45;
	t.ll = 78;
	CHECK(dsb_nid_toStr(&t,buf,100) == 0);
	CHECK(strcmp(buf,"[00:002d:000000000000004e]") == 0);

	t.hasMac = 1;
	t.persist = 1;
	t.r1 = 0;
	t.mac[0] = 0xaa;
	t.mac[1] = 0;
	t.mac[2] = 0;
	t.mac[3] = 0;
	t.mac[4] = 0;
	t.mac[5] = 0xcc;
	t.n = 343434;
	CHECK(dsb_nid_toStr(&t,buf,100) == 0);
	CHECK(strcmp(buf,"[03:aa:00:00:00:00:cc:0000053d8a]") == 0);

	DONE;
}

int main(int argc, char *argv[])
{
	dsb_test(test_nid_iton);
	dsb_test(test_nid_cton);
	dsb_test(test_nid_null);
	dsb_test(test_nid_nid);
	dsb_test(test_nid_eq);
	dsb_test(test_nid_leq);
	dsb_test(test_nid_geq);
	dsb_test(test_nid_fromstr);
	dsb_test(test_nid_toStr);
	return 0;
}
