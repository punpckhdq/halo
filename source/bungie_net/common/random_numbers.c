/*
RANDOM_NUMBERS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "random_numbers.h"

#include <stdlib.h>
#include <time.h>

/* ---------- globals */

static boolean random_numbers_initialized;

/* ---------- public code */

long randomrange(
	long min,
	long max)
{
	if (!random_numbers_initialized)
	{
		srand(time(NULL));
		random_numbers_initialized= TRUE;
	}

	return min+(long)(rand()*(double)(unsigned long)max/((double)(unsigned long)min+RAND_MAX));
}

void randomrange64(
	struct qword_value const *min,
	struct qword_value const *max,
	struct qword_value *result)
{
	struct qword_value range;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\random_numbers.c", 46, min && max && result);

	if (!random_numbers_initialized)
	{
		srand(time(NULL));
		random_numbers_initialized= TRUE;
	}

	range.qword= (unsigned __int64)(rand()*(double)max->qword/
		((double)min->qword+RAND_MAX));
	add64(min, &range, result);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\random_numbers.c", 58, result->qword >= min->qword);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\random_numbers.c", 59, result->qword <= max->qword);
}
