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
	char __unknown02[547626];
};

/* ---------- prototypes/AI_DEBUG.C */

void ai_debug_initialize_for_new_map(void);

void ai_debug_change_selected_encounter(boolean a1);

void ai_debug_change_selected_actor(boolean a1);

char *ai_debug_describe_actor(long actor_index, long unit_index, boolean include_squad, char *buffer, long bufsize);

/* ---------- globals */

/* comm */
struct ai_debug_state ai_debug;

/* ---------- public code */

#endif // __AI_DEBUG_H
