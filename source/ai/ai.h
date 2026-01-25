/*
AI.H

header included in hcex build.
*/

#ifndef __AI_H
#define __AI_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct ai_unit_information_look_data
{
	long unit_index;
};

struct ai_object_information_look_data
{
	long object_index;
};

struct ai_information_look_data
{
	union
	{
		struct ai_unit_information_look_data unit;
		struct ai_object_information_look_data object;
	};
};

struct ai_allegiance_information_data
{
	short team1_index;
	short team2_index;
	boolean broken;
};

struct ai_combat_stimulus_information_data
{
	word pad;
};

struct ai_target_knowledge_information_data
{
	long prop_index;
};

struct ai_information_data
{
	union
	{
		struct ai_allegiance_information_data allegiance;
		struct ai_combat_stimulus_information_data combat_stimulus;
		struct ai_target_knowledge_information_data target_knowledge;
	};
};


struct ai_information_packet
{
	long target_unit_index;
	short communication_type;
	short dialogue_type_index;
	short damage_category;
	boolean updated_dialogue_timers;
	short look_priority;
	short look_type;
	struct ai_information_look_data look_data;
	short information_type;
	struct ai_information_data information_data;
};

/* ---------- prototypes/AI.C */

boolean ai_release_inactive_swarms(char *result_description, unsigned char *more_to_release, unsigned char *working_memory, short working_memory_size);
void ai_find_inactive_encounters(unsigned char *working_memory, short working_memory_size);
boolean ai_release_inactive_encounters(char *result_description, unsigned char *more_to_release, unsigned char *working_memory, short working_memory_size);

/* ---------- globals */

/* ---------- public code */

#endif // __AI_H
