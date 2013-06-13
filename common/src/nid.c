#include "dsb/nid.h"
#include "dsb/errors.h"
#include "dsb/specials.h"
#include "dsb/names.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

NID_t local_base;
static unsigned char macaddr[6];

NID_t Root;
NID_t PRoot;
NID_t Null;
NID_t True;
NID_t False;
NID_t Names;
NID_t Size;
NID_t Keys;

int dsb_nid_init()
{
	int netf;
	char buf[18];
	char *serial;
	int lmac[6];

	serial = getenv("DSB_SERIAL");

	//If not serial number then use mac address
	if (serial == 0)
	{
		netf = open("/sys/class/net/eth0/address", O_RDONLY);
		if (netf != -1)
		{
			netf = read(netf, buf, 17);

			if (netf != 17)
			{
				serial = "00:00:00:00:00:01";
			}
			else
			{
				buf[17] = 0;
				serial = buf;
				close(netf);
			}
		}
		else
		{
			//Worst case, there is no way to get a serial number.
			serial = "00:00:00:00:00:01";
		}
	}

	//Decode serial number.
	sscanf(serial, "%x:%x:%x:%x:%x:%x", &(lmac[0]), &(lmac[1]), &(lmac[2]), &(lmac[3]), &(lmac[4]), &(lmac[5]));
	macaddr[0] = (unsigned char)lmac[0];
	macaddr[1] = (unsigned char)lmac[1];
	macaddr[2] = (unsigned char)lmac[2];
	macaddr[3] = (unsigned char)lmac[3];
	macaddr[4] = (unsigned char)lmac[4];
	macaddr[5] = (unsigned char)lmac[5];
	printf("Serial: %s\n",serial);

	//Initialise global NIDs.
	dsb_nid_local(0,&Root);
	dsb_nid_local(1,&PRoot);
	dsb_nid_null(&Null);

	dsb_nid(NID_SPECIAL,SPECIAL_TRUE,&True);
	dsb_nid(NID_SPECIAL,SPECIAL_FALSE,&False);
	dsb_nid(NID_SPECIAL,SPECIAL_SIZE,&Size);
	dsb_nid(NID_SPECIAL,SPECIAL_NAMES,&Names);
	dsb_nid(NID_SPECIAL,SPECIAL_KEYS,&Keys);

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

	if (n->hasMac == 0)
	{
		*buf = n->r2;
		buf++;
		//*((short*)buf) = n->t;
		memcpy(buf,&n->t,sizeof(short));
		buf += sizeof(short);
		//*((long long*)buf) = n->ll;
		memcpy(buf,&n->ll,sizeof(long long));
		return 2+sizeof(short)+sizeof(long long);
	}
	else
	{
		*buf = n->mac[0];
		buf++;
		*buf = n->mac[1];
		buf++;
		*buf = n->mac[2];
		buf++;
		*buf = n->mac[3];
		buf++;
		*buf = n->mac[4];
		buf++;
		*buf = n->mac[5];
		buf++;
		//*((long long*)buf) = n->n;
		memcpy(buf,&n->n,sizeof(int));
		return 7+sizeof(long long);
	}
}

int dsb_nid_unpack(const char *buf, NID_t *n)
{
	n->header = *buf;
	buf++;

	if (n->hasMac == 0)
	{
		n->r2 = *buf;
		buf++;
		//n->t = *((short*)buf);
		memcpy(&n->t,buf,sizeof(short));
		buf += sizeof(short);
		//n->ll = *((long long*)buf);
		memcpy(&n->ll,buf,sizeof(long long));
		return 2+sizeof(short)+sizeof(long long);
	}
	else
	{
		n->mac[0] = *buf;
		buf++;
		n->mac[1] = *buf;
		buf++;
		n->mac[2] = *buf;
		buf++;
		n->mac[3] = *buf;
		buf++;
		n->mac[4] = *buf;
		buf++;
		n->mac[5] = *buf;
		buf++;
		//n->n = *((long long*)buf);
		memcpy(&n->n,buf,sizeof(int));
		return 7+sizeof(long long);
	}
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
	if (n1->hasMac == 1)
	{
		return ((n1->header == n2->header) && (n1->n == n2->n) && (memcmp(n1->mac,n2->mac,6) == 0));
	}
	else
	{
		return ((n1->header == n2->header) && (n1->t == n2->t) && (n1->ll == n2->ll));
	}
}

int dsb_nid_leq(const NID_t *n1, const NID_t *n2)
{
	return ((n1->header <= n2->header) && (n1->t <= n2->t) && (n1->ll <= n2->ll));
}

int dsb_nid_geq(const NID_t *n1, const NID_t *n2)
{
	return ((n1->header >= n2->header) && (n1->t >= n2->t) && (n1->ll >= n2->ll));
}

int dsb_nid_op(unsigned int op, NID_t *n)
{
	n->header = 0;
	n->t = NID_VMOP;
	n->ll = op;
	return 0;
}

int dsb_nid_isLocal(const NID_t *n)
{
	if (n->hasMac == 0) return 1;
	if (memcmp(n->mac,macaddr,6) == 0) return 1;
	return 0;
}

int dsb_nid_local(int persistent, NID_t *n)
{
	n->persist = persistent;
	n->hasMac = 1;
	n->r1 = 0;
	memcpy(n->mac,macaddr,6);
	n->mac[6] = 0;
	n->n = 0;
	return SUCCESS;
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

/*
 * Take two hex digits and return their value.
 */
static int readHex(char a, char b)
{
	int res = 0;

	res = ((a >= '0') && (a <= '9')) ? (a-'0') << 4 : ((a-'a')+10) << 4;
	res +=  ((b >= '0') && (b <= '9')) ? (b-'0') : (b-'a')+10;

	return res;
}

int dsb_nid_fromStr(const char *str, struct NID *nid)
{
	if (str == 0) return ERR_NIDSTR;

	//Explicit OID
	if ((str[0] == '['))
	{
		int i = 1;
		int j;
		long long a;

		if (str[i] == 0 || str[i+1] == 0 || str[i+2] != ':') return ERR_NIDSTR;
		nid->header = readHex(str[i],str[i+1]);
		i += 3;

		if (nid->hasMac == 1)
		{
			//Replace a * with local mac
			if (str[i] == '*')
			{
				i += 2;
				for (j=0; j<6; j++)
				{
					nid->mac[j] = macaddr[j];
				}
			}
			else
			{
				//Parse 6 bytes of MAC address.
				for (j=0; j<6; j++)
				{
					if (str[i] == 0 || str[i+1] == 0 || str[i+2] != ':') return ERR_NIDSTR;
					nid->mac[j] = readHex(str[i],str[i+1]);
					i += 3;
				}
			}

			nid->mac[6] = 0;

			//Parse 40bit number
			a = 0;
			for (j=0; j<5; j++)
			{
				if (str[i] == 0 || str[i+1] == 0) return ERR_NIDSTR;
				a |= readHex(str[i],str[i+1]) << ((4-j) * 8);
				i += 2;
			}

			nid->n = a;
			if (str[i] != ']') return ERR_NIDSTR;
		}
		else
		{
			int j;

			//Read Type
			a = 0;
			if (str[i] == 0 || str[i+1] == 0) return ERR_NIDSTR;
			a |= readHex(str[i],str[i+1]) << 8;
			i += 2;
			if (str[i] == 0 || str[i+1] == 0 || str[i+2] != ':') return ERR_NIDSTR;
			a |= readHex(str[i],str[i+1]);
			i += 3;

			nid->t = (unsigned short)a;

			//Read type value (long long)
			a = 0;
			for (j=0; j<8; j++)
			{
				if (str[i] == 0 || str[i+1] == 0) return ERR_NIDSTR;
				a |= readHex(str[i],str[i+1]) << ((7-j) * 8);
				i += 2;
			}

			nid->ll = a;
			if (str[i] != ']') return ERR_NIDSTR;
		}
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

			while (str[i] >= '0' && str[i] <= '9') {
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

	//Otherwise, do a name lookup.
	dsb_names_lookup(str,nid);

	return ERR_NIDSTR;
}

int dsb_nid_toStr(const struct NID *nid, char *str, int len)
{
	if (nid->header == 0)
	{
		if (nid->t == NID_INTEGER)
		{
			sprintf(str, "%d", (unsigned int)nid->ll);
			return SUCCESS;
		}
		else if (nid->t == NID_REAL)
		{
			sprintf(str, "%0.4f", (float)nid->dbl);
			return SUCCESS;
		}
		else if (nid->t == NID_CHARACTER)
		{
			str[0] = '\'';
			str[1] = nid->chr;
			str[2] = '\'';
			str[3] = 0;
			return SUCCESS;
		}
	}

	if (dsb_names_revlookup(nid,str,len) == 0) return SUCCESS;

	if (nid->hasMac == 1)
	{
		if (dsb_nid_isLocal(nid) == 1)
		{
			sprintf(str,"[%02x:*:%010x]",nid->header,(unsigned int)nid->n);
		}
		else
		{
			sprintf(str,"[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%010x]",nid->header,nid->mac[0],nid->mac[1],nid->mac[2],nid->mac[3],nid->mac[4],nid->mac[5],(unsigned int)nid->n);
		}
	}
	else
	{
		sprintf(str,"[%02x:%04x:%016x]",nid->header,nid->t,(unsigned int)nid->ll);
	}
	return SUCCESS;
}

int dsb_nid_toRawStr(const struct NID *nid, char *str, int len)
{
	if (nid->hasMac == 1)
	{
		if (dsb_nid_isLocal(nid) == 1)
		{
			sprintf(str,"[%02x:*:%010x]",nid->header,(unsigned int)nid->n);
		}
		else
		{
			sprintf(str,"[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%010x]",nid->header,nid->mac[0],nid->mac[1],nid->mac[2],nid->mac[3],nid->mac[4],nid->mac[5],(unsigned int)nid->n);
		}
	}
	else
	{
		sprintf(str,"[%02x:%04x:%016x]",nid->header,nid->t,(unsigned int)nid->ll);
	}
	return SUCCESS;
}

