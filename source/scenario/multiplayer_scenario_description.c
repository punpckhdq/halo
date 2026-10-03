/*
MULTIPLAYER_SCENARIO_DESCRIPTION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "multiplayer_scenario_description.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

struct multiplayer_scenario_description_item *multiplayer_scenario_description_get_list(
	short *count)
{
	struct multiplayer_scenario_description_item *result;
	long scenario_list_index;

	match_assert("c:\\halo\\SOURCE\\scenario\\multiplayer_scenario_description.c", 60, count);
	scenario_list_index = tag_loaded(MULTIPLAYER_SCENARIO_DESCRIPTION_TAG, "ui\\multiplayer_scenarios");
	if (scenario_list_index != NONE)
	{
		struct tag_block *scenario_list = tag_get(MULTIPLAYER_SCENARIO_DESCRIPTION_TAG, scenario_list_index);

		match_assert("c:\\halo\\SOURCE\\scenario\\multiplayer_scenario_description.c", 66, scenario_list);
		result = scenario_list->address;
		*count = scenario_list->count;
	}
	else
	{
		result = NULL;
		*count = 0;
	}

	return result;
}

boolean map_name_from_multiplayer_scenario_description_item(
	struct multiplayer_scenario_description_item const *item,
	char *buffer,
	long buffer_size)
{
	long i;

	match_assert("c:\\halo\\SOURCE\\scenario\\multiplayer_scenario_description.c", 88, item);
	buffer[0] = '\0';
	for (i = strlen(item->scenario_tag_directory_path); i >= 0; i--)
	{
		if (item->scenario_tag_directory_path[i] == '\\')
		{
			_snprintf(buffer, buffer_size, "%s\\%s", item->scenario_tag_directory_path, &item->scenario_tag_directory_path[i+1]);
			break;
		}
	}

	return TRUE;
}

/* ---------- private code */
