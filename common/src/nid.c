#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/specials.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

NID_t local_base;
static unsigned char macaddr[6];

int dsb_nid_init()
{
	int netf;
	char buf[18];

	netf = open("/sys/class/net/eth0/address", O_RDONLY);
	if (netf != -1)
	{
		int lmac[6];
		read(netf, buf, 17);
		buf[17] = 0;
		sscanf(buf, "%x:%x:%x:%x:%x:%x", &(lmac[0]), &(lmac[1]), &(lmac[2]), &(lmac[3]), &(lmac[4]), &(lmac[5]));
		close(netf);
		macaddr[0] = (unsigned char)lmac[0];
		macaddr[1] = (unsigned char)lmac[1];
		macaddr[2] = (unsigned char)lmac[2];
		macaddr[3] = (unsigned char)lmac[3];
		macaddr[4] = (unsigned char)lmac[4];
		macaddr[5] = (unsigned char)lmac[5];
		printf("Mac Addr: %s\n",buf);
	}

	return SUCCESS;
}

int dsb_nid_final()
{
	return SUCCESS;
}

int dsb_nid_pack(const NID_t *n, char *buf, int max)
{
	*buf = n->header;
	buf++;
	*buf = n->r2;
	buf++;
	*((short*)buf) = n->t;
	buf += sizeof(short);
	*((long long*)buf) = n->ll;
	return 2+sizeof(short)+sizeof(long long);
}

int dsb_nid_unpack(const char *buf, NID_t *n)
{
	n->header = *buf;
	buf++;
	n->r2 = *buf;
	buf++;
	n->t = *((short*)buf);
	buf += sizeof(short);
	n->ll = *((long long*)buf);
	return 2+sizeof(short)+sizeof(long long);
}

struct NID *dsb_nid(enum NIDType type, unsigned long long ll, struct NID *nid)
{
	nid->header = 0;
	nid->t = type;
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

int dsb_nid_eq(const NID_t *n1, const NID_t *n2)
{
	return ((n1->header == n2->header) && (n1->t == n2->t) && (n1->ll == n2->ll));
}

int dsb_nid_leq(const NID_t *n1, const NID_t *n2)
{
	return ((n1->header <= n2->header) && (n1->t <= n2->t) && (n1->ll <= n2->ll));
}

int dsb_nid_geq(const NID_t *n1, const NID_t *n2)
{
	return ((n1->header >= n2->header) && (n1->t >= n2->t) && (n1->ll >= n2->ll));
}

int dsb_nid_isLocal(const NID_t *n)
{
	if (n->hasMac == 0) return 1;
	if (memcmp(n->mac,macaddr,6) == 0) return 1;
	return 0;
}

void dsb_iton(int i, struct NID *n)
{
	n->header = 0;
	n->t = NID_INTEGER;
	n->ll = i;
}

int dsb_ntoi(const struct NID *n)
{
	if ((n->header == 0) && (n->t == NID_INTEGER))
	{
		return n->ll;
	}
	else
	{
		return 0;
	}
}

void dsb_cton(char chr, NID_t *n)
{
	n->header = 0;
	n->t = NID_CHARACTER;
	n->chr = chr;
}

char dsb_ntoc(const NID_t *n)
{
	if ((n->header == 0) && (n->t == NID_CHARACTER))
	{
		return n->chr;
	}
	else
	{
		return 0;
	}
}

void dsb_nid_null(NID_t *n)
{
	memset(n,0,sizeof(NID_t));
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

		nid->header = 0;
		nid->t = a;
		nid->ll = b;
		return SUCCESS;
	}
	else if ((str[0] == '-') || (str[0] >= '0' && str[0] <= '9'))
	{
			if (strchr(str, '.'))
			{
				nid->header = 0;
				nid->dbl = atof(str);
				nid->t = NID_REAL;
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
				nid->header = 0;
				nid->t = NID_INTEGER;
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
	if (nid->header == 0)
	{
		if (nid->t == NID_SPECIAL)
		{
			switch(nid->ll)
			{
			case SPECIAL_NULL:		strcpy(str, "null"); return SUCCESS;
			case SPECIAL_TRUE:		strcpy(str, "true"); return SUCCESS;
			case SPECIAL_FALSE:		strcpy(str, "false"); return SUCCESS;
			default: break;
			}
		}
		else if (nid->t == NID_INTEGER)
		{
			sprintf(str, "%d", (unsigned int)nid->ll);
			return SUCCESS;
		}
		else if (nid->t == NID_REAL)
		{
			sprintf(str, "%0.4f", (float)nid->dbl);
			return SUCCESS;
		}
	}

	sprintf(str,"[%d:%d:%d]",nid->header,nid->t,(unsigned int)nid->ll);
	return SUCCESS;
}

