/*
PRIME_NUMBERS.H
*/

#ifndef __PRIME_NUMBERS_H
#define __PRIME_NUMBERS_H
#pragma once

/* ---------- headers */

#include "64bit_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PRIME_NUMBERS.C */

unsigned long randomprime(unsigned long maximum);
void probable_prime64(struct qword_value *result);

/* ---------- globals */

/* ---------- public code */

#endif // __PRIME_NUMBERS_H
