/*
RANDOM_NUMBERS.H

header included in hcex build.
*/

#ifndef __RANDOM_NUMBERS_H
#define __RANDOM_NUMBERS_H
#pragma once

/* ---------- headers */

#include "64bit_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RANDOM_NUMBERS.C */

long randomrange(long min, long max);
void randomrange64(struct qword_value const *min, struct qword_value const *max, struct qword_value *result);

/* ---------- globals */

/* ---------- public code */

#endif // __RANDOM_NUMBERS_H
