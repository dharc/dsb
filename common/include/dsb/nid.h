#ifndef _NID_H_
#define _NID_H_

enum NIDType {
	NID_SPECIAL=0,
	NID_INTEGER,
	NID_REAL,
	NID_CHARACTER,
	NID_LABEL,
	NID_USER=1000
};

struct NID {
	NIDType type;

	union {
		struct {
			unsigned int a;
			unsigned int b;
		};
		unsigned long long ll;
		double dbl;
		unsigned short chr;
	};
};

int dsb_nid_init();
int dsb_nid_final();
int dsb_nid_allocate(NID *nid);
int dsb_nid_free(const NID *nid);
int dsb_nid_compare(const NID *a, const NID *b);
int dsb_nid_toStr(const NID *nid, char *str, int len);
int dsb_nid_fromStr(const char *str, NID *nid);

#endif //_NID_H_
