#include "dsb/core/harc.h"
#include "dsb/core/nid.h"
#include "dsb/errors.h"

HARC_t *dsb_harc(const NID_t *t1, const NID_t *t2, HARC_t *harc)
{
	harc->t1 = *t1;
	harc->t2 = *t2;
	harc->deps = 0;
	dsb_nid_null(&harc->def);
	harc->h = harc->def;
	harc->flags = 0;
	return harc;
}


/*
 * Convert a HARC structure into a single CSV line.
 */
int dsb_harc_serialize(FILE *fd, const HARC_t *harc)
{
	char buf1[100];
	char buf2[100];
	char buf3[100];

	dsb_nid_toRawStr(&harc->t1, buf1, 100);
	dsb_nid_toRawStr(&harc->t2, buf2, 100);
	dsb_nid_toRawStr(&harc->def, buf3, 100);

	fprintf(fd, "%s,%s,%s,%d,\n",buf1,buf2,buf3,harc->flags);

	return 0;
}

/*
 * Convert a CSV line into a HARC. Format is tail1,tail2,definition,evaluator.
 */
int dsb_harc_deserialize(FILE *fd, HARC_t *harc)
{
	char buf1[100];
	char buf2[100];
	char buf3[100];

	if (fscanf(fd, "%[^,],%[^,],%[^,],%d,\n",buf1,buf2,buf3,&harc->flags) != 4)
	{
		//End of file or invalid input.
		return -1;
	}

	DSB_ERROR(dsb_nid_fromStr(buf1,&harc->t1),0);
	DSB_ERROR(dsb_nid_fromStr(buf2,&harc->t2),0);
	DSB_ERROR(dsb_nid_fromStr(buf3,&harc->def),0);

	return 0;
}
