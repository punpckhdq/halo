/*
HS.H

header included in hcex build.
*/

#ifndef __HS_H
#define __HS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/HS.C */

boolean hs_compile_and_evaluate(const char *expression);

short hs_tokens_enumerate(char const *substring, long type_flags, char const **results, short maximum_count);

/* ---------- globals */

/* ---------- public code */

#endif // __HS_H
