/*
64BIT_MATH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "64bit_math.h"

/* ---------- prototypes */

static void negate64(struct qword_value const *a, struct qword_value *result);

/* ---------- public code */

void add64(
	struct qword_value const *a,
	struct qword_value const *b,
	struct qword_value *result)
{
	long sum;
	long carry;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 33, a && b && result);

	sum= a->words[0]+b->words[0];
	result->words[0]= (word)sum;
	carry= sum>0xFFFF;
	sum= carry+a->words[1]+b->words[1];
	result->words[1]= (word)sum;
	carry= sum>0xFFFF;
	sum= carry+a->words[2]+b->words[2];
	result->words[2]= (word)sum;
	carry= sum>0xFFFF;
	sum= carry+a->words[3]+b->words[3];
	result->words[3]= (word)sum;
	carry= sum>0xFFFF;
}

static void negate64(
	struct qword_value const *a,
	struct qword_value *result)
{
	long borrow= 0;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 58, a && result);

	result->words[0]= -a->words[0];
	if (a->words[0]) borrow= 1;
	result->words[1]= -(word)(a->words[1]+borrow);
	if (a->words[1]) borrow= 1;
	result->words[2]= -(word)(a->words[2]+borrow);
	if (a->words[2]) borrow= 1;
	result->words[3]= -(word)(a->words[3]+borrow);
}

void subtract64(
	struct qword_value const *a,
	struct qword_value const *b,
	struct qword_value *result)
{
	struct qword_value negated_b;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 79, a && b && result);

	negate64(b, &negated_b);
	add64(a, &negated_b, result);
}

void multiply64(
	struct qword_value const *a,
	struct qword_value const *b,
	struct qword_value *result)
{
	unsigned long partial[7]= {0, 1, 2, 3, 4, 5, 6};
	unsigned long i, j;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 95, a && b && result);

	for (i= 0; i<4; i++)
	{
		for (j= 0; j<4; j++)
		{
			unsigned long product= a->words[i]*b->words[j];

			partial[i+j]+= product&0xFFFF;
			partial[i+j+1]+= product>>16;
		}
	}

	result->words[0]= (word)partial[0];
	result->words[1]= (word)partial[1];
	result->words[2]= (word)partial[2];
	result->words[3]= (word)partial[3];
}

void divide64(
	struct qword_value const *numerator,
	struct qword_value const *denominator,
	struct qword_value *quotient,
	struct qword_value *remainder)
{
	struct qword_value value[2];
	struct qword_value difference;
	struct qword_value high;
	unsigned long i;
	long count;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\64bit_math.c", 124, numerator && denominator);

	for (i= 0; i<4; i++)
	{
		value[0].words[i]= denominator->words[i];
		value[1].words[i]= 0;
	}

	for (count= 64; count; count--)
	{
		unsigned long carry= 0;

		for (i= 0; i<8; i++)
		{
			carry+= ((word *)value)[i]<<1;
			((word *)value)[i]= (word)carry;
			carry>>= 16;
		}
		high= value[1];
		subtract64(&high, numerator, &difference);
		if (!(difference.words[3]&0x8000))
		{
			value[1]= difference;
			value[0].words[0]++;
		}
	}

	if (quotient) *quotient= value[0];
	if (remainder) *remainder= value[1];
}
