/*
64BIT_MATH.H

header included in hcex build.
*/

#ifndef __64BIT_MATH_H
#define __64BIT_MATH_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct qword_value /* fake name */
{
	union
	{
		unsigned __int64 qword;
		word words[4];
	};
};

/* ---------- prototypes/64BIT_MATH.C */

void add64(struct qword_value const *a, struct qword_value const *b, struct qword_value *result);
void subtract64(struct qword_value const *a, struct qword_value const *b, struct qword_value *result);
void multiply64(struct qword_value const *a, struct qword_value const *b, struct qword_value *result);
void divide64(struct qword_value const *numerator, struct qword_value const *denominator, struct qword_value *quotient, struct qword_value *remainder);

/* ---------- globals */

/* ---------- public code */

#endif // __64BIT_MATH_H
