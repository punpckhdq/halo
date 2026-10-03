/*
RANDOM_NUMBERS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "random_numbers.h"
#include <time.h>

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static boolean random_numbers_initialized; /* fake name */

/* ---------- public code */

unsigned long randomrange(
	unsigned long min,
	unsigned long max)
{
	if (!random_numbers_initialized)
	{
		srand(time(NULL));
		random_numbers_initialized = TRUE;
	}

	return min + (unsigned long)((double)rand() * max / (min + (double)RAND_MAX));
}

void randomrange64(
	struct qword_value const *min,
	struct qword_value const *max,
	struct qword_value *result)
{
	struct qword_value random;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\random_numbers.c", 46, min && max && result);

	if (!random_numbers_initialized)
	{
		srand(time(NULL));
		random_numbers_initialized = TRUE;
	}

	random.qword = (unsigned __int64)((double)rand() * max->qword / (min->qword + (double)RAND_MAX));
	add64(min, &random, result);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\random_numbers.c", 58, result->qword >= min->qword);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\random_numbers.c", 59, result->qword <= max->qword);

	return;
}
