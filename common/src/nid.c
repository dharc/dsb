#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/specials.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

NID_t local_base;

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

struct NID *dsb_nid(enum NIDType type, unsigned long long ll, struct NID *nid)
{
	nid->type = type;
	nid->ll = ll;
	return nid;
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

void dsb_iton(int i, struct NID *n)
{
	n->type = NID_INTEGER;
	n->ll = i;
}

int dsb_ntoi(const struct NID *n)
{
	if (n->type == NID_INTEGER)
	{
		return n->ll;
	}
	else
	{
		return 0;
	}
}

int dsb_nid_fromStr(const char *str, struct NID *nid)
{
	if (str == 0) return ERR_NIDSTR;

	//Explicit OID
	if ((str[0] == '['))
	{
		int i = 1;
		int a;
		long long b;

		//Read Integer
		a = 0;
		while (str[i] != 0 && str[i] != ':') {
			a *= 10;
			a += str[i] - '0';
			i++;
		}

		if (str[i] == 0) return ERR_NIDSTR;

		//Read :
		i++;

		//Read Integer
		b = 0;
		while (str[i] != 0 && str[i] != ']') {
			b *= 10;
			b += str[i] - '0';
			i++;
		}

		if (str[i] == 0) return ERR_NIDSTR;

		//Read ]

		nid->type = a;
		nid->ll = b;
		return SUCCESS;
	}
	else if ((str[0] == '-') || (str[0] >= '0' && str[0] <= '9'))
	{
			if (strchr(str, '.'))
			{
				nid->dbl = atof(str);
				nid->type = NID_REAL;
				return SUCCESS;
			}
			else
			{
				int i=0;
				int num=0;
				int neg = 0;

				if (str[0] == '-') {
					i++;
					neg = 1;
				}

				if (str[i] == 0) return ERR_NIDSTR;

				while (str[i] != 0) {
					num *= 10;
					num += str[i] - '0';
					i++;
				}
				if (neg == 1) num = -num;
				nid->type = NID_INTEGER;
				nid->ll = num;
				return SUCCESS;
			}
		}
		//else if (v[0] == '\'') {
		//	m_a = 0;
		//	m_b = 3;
		//	m_c = 0;
		//	m_d = v[1];
		//} else {
		//	*this = names->lookup(v);
		//}

		return ERR_NIDSTR;
}

int dsb_nid_toStr(const struct NID *nid, char *str, int len)
{
	int i = 0;
	const char *temp;


	if (nid->type == NID_SPECIAL)
	{
		switch(nid->ll)
		{
		case SPECIAL_NULL:		strcpy(str, "null"); return SUCCESS;
		case SPECIAL_TRUE:		strcpy(str, "true"); return SUCCESS;
		case SPECIAL_FALSE:		strcpy(str, "false"); return SUCCESS;
		default: break;
		}
	}
	else if (nid->type == NID_INTEGER)
	{
		sprintf(str, "%d", (unsigned int)nid->ll);
		return SUCCESS;
	}
	else if (nid->type == NID_REAL)
	{
		sprintf(str, "%0.4f", (float)nid->dbl);
		return SUCCESS;
	}

	sprintf(str,"[%d:%d]",nid->type,(unsigned int)nid->ll);
	return SUCCESS;
}

