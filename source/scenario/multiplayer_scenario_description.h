/*
MULTIPLAYER_SCENARIO_DESCRIPTION.H
*/

#ifndef __MULTIPLAYER_SCENARIO_DESCRIPTION_H
#define __MULTIPLAYER_SCENARIO_DESCRIPTION_H
#pragma once

/* ---------- constants */

enum
{
	MULTIPLAYER_SCENARIO_DESCRIPTION_TAG = 'mply' /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct multiplayer_scenario_description_item /* fake name */
{
	struct tag_reference descriptive_bitmap;	// bitmap_group
	struct tag_reference displayed_map_name;	// unicode_string_list_group_header
	char scenario_tag_directory_path[TAG_STRING_LENGTH+1];
	long unused[1];
};

/* ---------- prototypes/MULTIPLAYER_SCENARIO_DESCRIPTION.C */

struct multiplayer_scenario_description_item *multiplayer_scenario_description_get_list(short *count);
boolean map_name_from_multiplayer_scenario_description_item(struct multiplayer_scenario_description_item const *item, char *buffer, long buffer_size);

/* ---------- globals */

#endif // __MULTIPLAYER_SCENARIO_DESCRIPTION_H
