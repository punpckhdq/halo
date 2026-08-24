/*
PRIME_NUMBERS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "prime_numbers.h"

#include "64bit_math.h"
#include "random_numbers.h"

#include <math.h>
#include <stdlib.h>

/* ---------- constants */

#define PROBABLE_PRIME64_FACTOR_COUNT 4
#define PROBABLE_PRIME64_FACTOR_MAXIMUM 0xffff

/* ---------- prototypes */

static int compare_ulongs_descending(void const *a, void const *b);
static unsigned long *primegen(unsigned long max, unsigned long *num_primes);

/* ---------- private code */

static int compare_ulongs_descending(
	void const *a,
	void const *b)
{
	unsigned long value_a= *(unsigned long const *)a;
	unsigned long value_b= *(unsigned long const *)b;

	return (value_b>value_a) ? 1 : ((value_b<value_a) ? -1 : 0);
}

// returns every prime below max, largest first, and the count through num_primes;
// the caller owns the returned block
static unsigned long *primegen(
	unsigned long max,
	unsigned long *num_primes)
{
	unsigned long odd_count= max>>1;
	unsigned long index= 0;
	unsigned long sieve_count= 0;
	unsigned long *primes;
	unsigned long total;
	unsigned long root;
	unsigned long value;

	if (!(max&1)) odd_count--;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 61, num_primes);

	if (max<2)
	{
		*num_primes= 0;
		return NULL;
	}

	total= odd_count+1;
	*num_primes= total;

	primes= match_malloc("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 71,
		odd_count*sizeof(unsigned long)+sizeof(unsigned long));

	if (primes)
	{
		value= 3;
		root= (unsigned long)sqrt((double)max);

		if (odd_count)
		{
			do
			{
				primes[index++]= value;
				value+= 2;
			}
			while (index<odd_count);

			do
			{
				if (primes[sieve_count]>root) break;
				sieve_count++;
			}
			while (sieve_count<odd_count);
		}
		else
		{
			sieve_count= 0;
		}

		if (sieve_count)
		{
			unsigned long *factor= primes;
			unsigned long factor_index= 0;

			do
			{
				if (*factor)
				{
					unsigned long multiple_index= factor_index+1;

					if (multiple_index<odd_count)
					{
						do
						{
							unsigned long multiple= primes[multiple_index];

							if (multiple && multiple%*factor==0)
							{
								primes[multiple_index]= 0;
								(*num_primes)--;
							}
							multiple_index++;
						}
						while (multiple_index<odd_count);
					}
				}
				factor_index++;
				factor++;
			}
			while (--sieve_count);
		}

		primes[odd_count]= 2;
		qsort(primes, total, sizeof(unsigned long), compare_ulongs_descending);

		if (*num_primes<total)
		{
			primes= match_realloc("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 117,
				primes, *num_primes*sizeof(unsigned long));
		}
	}

	return primes;
}

/* ---------- public code */

unsigned long randomprime(
	unsigned long max)
{
	unsigned long result= 0;
	unsigned long num_primes;
	unsigned long *primes= primegen(max, &num_primes);

	if (primes)
	{
		result= primes[randomrange(0, num_primes-1)];
		match_free("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 137, primes);
	}

	return result;
}

void probable_prime64(
	struct qword_value *result)
{
	struct qword_value two;
	struct qword_value factor;
	long index;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\prime_numbers.c", 150, result);

	result->qword= 1;
	two.qword= 2;

	for (index= PROBABLE_PRIME64_FACTOR_COUNT; index; index--)
	{
		factor.qword= randomprime(PROBABLE_PRIME64_FACTOR_MAXIMUM);
		multiply64(result, &factor, result);
	}

	add64(result, &two, result);

	return;
}
