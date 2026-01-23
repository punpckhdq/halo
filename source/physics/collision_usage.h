/*
COLLISION_USAGE.H

header included in hcex build.
*/

#ifndef __COLLISION_USAGE_H
#define __COLLISION_USAGE_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_COLLISION_USER_STACK_DEPTH= 32,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

extern short global_current_collision_user_depth;
extern short global_current_collision_users[MAXIMUM_COLLISION_USER_STACK_DEPTH];

/* ---------- public code */

#endif // __COLLISION_USAGE_H
