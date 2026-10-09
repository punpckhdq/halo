/*
AI_SCRIPT.H

header included in hcex build.
*/

#ifndef __AI_SCRIPT_H
#define __AI_SCRIPT_H
#pragma once

/* ---------- headers */

#include "encounters.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct ai_index_actor_iterator
{
	long encounter_index;
	long squad_index;
	long platoon_index;
	struct encounter_actor_iterator iterator;
};

/* ---------- prototypes/AI_SCRIPT.C */

void ai_scripting_erase_all(void);
void ai_index_to_string(long ai_index, struct scenario *scenario, char *buffer, long bufsize);
void ai_index_actor_iterator_new(long ai_index, struct ai_index_actor_iterator *iterator);
struct actor_datum *ai_index_actor_iterator_next(struct ai_index_actor_iterator *iterator);

/* ---------- globals */

/* ---------- public code */

#endif // __AI_SCRIPT_H
