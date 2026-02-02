/*
PROFILE.H
*/

#ifndef __PROFILE_H
#define __PROFILE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

#define profile_enter(objects_update_section)							\
if (profile_global_enable && objects_update_section.profile_active)	\
{														\
	profile_enter_private(&objects_update_section);					\
}														\

#define profile_exit()									\
if (profile_global_enable && objects_update_section.profile_active)	\
{														\
	profile_exit_private(&objects_update_section);						\
}														\

/* ---------- structures */

struct profile_section
{
	const char *name;
	long section_index;
	boolean profile_active;
	short stack_depth;
	long field_C;
	unsigned __int64 field_10;
	char __unknown18[1460];
	long field_5CC;
	char __unknown5D0[40];
};

/* ---------- prototypes/PROFILE.C */

void profile_initialize(void);

void profile_enter_private(struct profile_section *objects_update_section);

void profile_exit_private(struct profile_section *objects_update_section);

/* ---------- globals */

/* comm */
boolean profile_global_enable;

/* ---------- public code */

#endif // __PROFILE_H
