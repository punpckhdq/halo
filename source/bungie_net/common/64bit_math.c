/*
64BIT_MATH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "64bit_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void negate64(struct qword_value const *a, struct qword_value *result);

/* ---------- globals */

/* ---------- public code */

void add64(
	struct qword_value const *a,
	struct qword_value const *b,
	struct qword_value *result)
{
	long i;
	long sum;
	long carry = 0;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 33, a && b && result);

	for (i = 0; i < 4; i++)
	{
		sum = a->words[i] + b->words[i] + carry;

		if (sum > UNSIGNED_SHORT_MAX)
		{
			carry = 1;
		}
		else
		{
			carry = 0;
		}

		result->words[i] = (word)sum;
	}

	return;
}

void subtract64(
	struct qword_value const *a,
	struct qword_value const *b,
	struct qword_value *result)
{
	struct qword_value negative_b;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 79, a && b && result);

	negate64(b, &negative_b);
	add64(a, &negative_b, result);

	return;
}

void multiply64(
	struct qword_value const *a,
	struct qword_value const *b,
	struct qword_value *result)
{
	unsigned long i;
	unsigned long j;
	unsigned long products[7] = {0, 1, 2, 3, 4, 5, 6};

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 95, a && b && result);

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			unsigned long product = a->words[i] * b->words[j];

			products[i + j] += product & UNSIGNED_SHORT_MAX;
			products[i + j + 1] += product >> SHORT_BITS;
		}
	}

	for (i = 0; i < 4; i++)
	{
		result->words[i] = (word)products[i];
	}

	return;
}

void divide64(
	struct qword_value const *numerator,
	struct qword_value const *denominator,
	struct qword_value *quotient,
	struct qword_value *remainder)
{
	word division[8];
	struct qword_value difference;
	struct qword_value high;
	unsigned long i;
	unsigned long bit;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 124, numerator && denominator);

	for (i = 0; i < 4; i++)
	{
		division[i] = denominator->words[i];
		division[i + 4] = 0;
	}

	for (bit = 0; bit < 64; bit++)
	{
		unsigned long carry = 0;

		for (i = 0; i < 8; i++)
		{
			carry += division[i] * 2;
			division[i] = (word)carry;
			carry >>= SHORT_BITS;
		}

		high = *(struct qword_value *)&division[4];
		subtract64(&high, numerator, &difference);

		if (!(difference.words[3] & FLAG(SHORT_BITS - 1)))
		{
			*(struct qword_value *)&division[4] = difference;
			division[0]++;
		}
	}

	if (quotient)
	{
		*quotient = *(struct qword_value *)&division[0];
	}

	if (remainder)
	{
		*remainder = *(struct qword_value *)&division[4];
	}

	return;
}

/* ---------- private code */

static void negate64(
	struct qword_value const *a,
	struct qword_value *result)
{
	long i;
	word carry = 0;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 58, a && result);

	for (i = 0; i < 4; i++)
	{
		result->words[i] = (word)-(a->words[i] + carry);

		if (a->words[i])
		{
			carry = 1;
		}
	}

	return;
}
