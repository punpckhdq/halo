/*
AI_DEBUG.H

header included in hcex build.
*/

#ifndef __AI_DEBUG_H
#define __AI_DEBUG_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct ai_debug_state
{
	boolean field_0;
	boolean select_actor;
	char gap[547626];
};

/* ---------- prototypes/AI_DEBUG.C */

void ai_debug_initialize_for_new_map(void);

/* ---------- globals */

struct ai_debug_state ai_debug;

/* ---------- public code */

#endif // __AI_DEBUG_H
