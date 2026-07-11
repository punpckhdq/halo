/*
AI_COMMUNICATION.H

header included in hcex build.
*/

#ifndef __AI_COMMUNICATION_H
#define __AI_COMMUNICATION_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/AI_COMMUNICATION.C */

real ai_communication_get_player_rating(real unit_index, boolean test_line_of_sight, long *unit_index_reference, real *distance_reference);

/* ---------- globals */

/* ---------- public code */

#endif // __AI_COMMUNICATION_H
