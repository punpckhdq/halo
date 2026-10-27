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

void hs_runtime_dispose_from_old_map(void);
void hs_runtime_initialize_for_new_map(void);
char const *hs_runtime_get_executing_thread_name(void);

boolean hs_compile_and_evaluate(const char *expression);

short hs_tokens_enumerate(char const *substring, long type_flags, char const **results, short maximum_count);

void render_debug_scripting(void);
void render_debug_trigger_volumes(void);

/* ---------- globals */

/* ---------- public code */

#endif // __HS_H
