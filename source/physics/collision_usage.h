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
	MAXIMUM_COLLISION_USER_STACK_DEPTH = 32,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/EXAMPLE.C */

void collision_log_initialize();
void collision_log_dispose();
void collision_log_enable(bool);
void collision_log_render();
void collision_log_start_time(__int64 *start_time);
void collision_log_end_time(short collision_function, __int64 end_time);
void collision_log_usage(short);
void collision_log_display(char *);
void collision_log_begin_period(short);
void collision_log_end_period();

/* ---------- globals */

extern short global_current_collision_user_depth;
extern short global_current_collision_users[MAXIMUM_COLLISION_USER_STACK_DEPTH];

/* ---------- public code */

#endif // __COLLISION_USAGE_H
