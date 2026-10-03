/*
PRIME_NUMBERS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "prime_numbers.h"
#include "random_numbers.h"

/* ---------- constants */

enum
{
	NUMBER_OF_PROBABLE_PRIME_FACTORS = 4, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static int __cdecl compare_ulongs_descending(void const *a, void const *b);
static unsigned long *primegen(unsigned long maximum, unsigned long *num_primes);

/* ---------- globals */

/* ---------- public code */

unsigned long randomprime(
	unsigned long maximum)
{
	unsigned long num_primes;
	unsigned long result = 0;
	unsigned long *primes = primegen(maximum, &num_primes);

	if (primes)
	{
		result = primes[randomrange(0, num_primes - 1)];
		match_free("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 137, primes);
	}

	return result;
}

void probable_prime64(
	struct qword_value *result)
{
	struct qword_value two;
	struct qword_value prime;
	long i;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 150, result);

	result->qword = 1;
	two.qword = 2;

	for (i = 0; i < NUMBER_OF_PROBABLE_PRIME_FACTORS; i++)
	{
		prime.qword = randomprime(UNSIGNED_SHORT_MAX);
		multiply64(result, &prime, result);
	}

	add64(result, &two, result);

	return;
}

/* ---------- private code */

static int __cdecl compare_ulongs_descending(
	void const *a,
	void const *b)
{
	unsigned long left = *(unsigned long const *)a;
	unsigned long right = *(unsigned long const *)b;

	return right > left ? 1 : (right < left ? -1 : 0);
}

static unsigned long *primegen(
	unsigned long maximum,
	unsigned long *num_primes)
{
	unsigned long *primes;
	unsigned long i;
	unsigned long limit;
	unsigned long count;
	unsigned long odd_count = maximum >> 1;

	if (!(maximum & 1))
	{
		odd_count--;
	}

	limit = 0;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 61, num_primes);

	if (maximum < 2)
	{
		*num_primes = 0;
		return NULL;
	}

	count = odd_count + 1;

	*num_primes = count;
	primes = (unsigned long *)match_malloc("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 71, (odd_count + 1) * sizeof(unsigned long));

	if (primes)
	{
		unsigned long square_root;
		unsigned long odd = 3;

		i = 0;
		square_root = (unsigned long)sqrt(maximum);

		while (i < odd_count)
		{
			primes[i++] = odd;
			odd += 2;
		}

		while (limit < odd_count && primes[limit] <= square_root)
		{
			limit++;
		}

		for (i = 0; i < limit; i++)
		{
			if (primes[i])
			{
				unsigned long j;

				for (j = i + 1; j < odd_count; j++)
				{
					if (primes[j] && !(primes[j] % primes[i]))
					{
						primes[j] = 0;
						(*num_primes)--;
					}
				}
			}
		}

		primes[odd_count] = 2;
		qsort(primes, count, sizeof(unsigned long), compare_ulongs_descending);

		if (*num_primes < count)
		{
			primes = (unsigned long *)match_realloc("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 117, primes, *num_primes * sizeof(unsigned long));
		}
	}

	return primes;
}
