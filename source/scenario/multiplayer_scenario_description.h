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

#define MULTIPLAYER_SCENARIO_DESCRIPTION_TAG_NAME "ui\\multiplayer_scenarios" /* fake name */
#define multiplayer_scenario_description_definition_get(index) ((struct multiplayer_scenario_description *)tag_get(MULTIPLAYER_SCENARIO_DESCRIPTION_TAG, index)) /* fake name */

/* ---------- structures */

struct scenario_description
{
	struct tag_reference descriptive_bitmap;	// bitmap_group
	struct tag_reference displayed_map_name;	// unicode_string_list_group_header
	char scenario_tag_directory_path[TAG_STRING_LENGTH+1];
	long unused[1];
};

struct multiplayer_scenario_description
{
	struct tag_block multiplayer_scenarios;	// scenario_description
};

/* ---------- prototypes/MULTIPLAYER_SCENARIO_DESCRIPTION.C */

struct scenario_description *multiplayer_scenario_description_get_list(short *count);
boolean map_name_from_multiplayer_scenario_description_item(struct scenario_description const *item, char *buffer, long buffer_size);

/* ---------- globals */

/* ---------- public code */

#endif // __MULTIPLAYER_SCENARIO_DESCRIPTION_H
