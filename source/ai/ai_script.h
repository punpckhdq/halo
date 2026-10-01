/*
AI_SCRIPT.H

header included in hcex build.
*/

#ifndef __AI_SCRIPT_H
#define __AI_SCRIPT_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/AI_SCRIPT.C */

void ai_scripting_erase_all(void);
void ai_index_to_string(long ai_index, struct scenario *scenario, char *buffer, long bufsize);

/* ---------- globals */

/* ---------- public code */

#endif // __AI_SCRIPT_H
