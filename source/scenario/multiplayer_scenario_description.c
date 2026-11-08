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

struct scenario_description *multiplayer_scenario_description_get_list(
	short *count)
{
	struct scenario_description *result;
	long scenario_list_index;

	match_assert("c:\\halo\\SOURCE\\scenario\\multiplayer_scenario_description.c", MATCH_LINE_NTSC(61, 60), count);
	scenario_list_index = tag_loaded(MULTIPLAYER_SCENARIO_DESCRIPTION_TAG, MULTIPLAYER_SCENARIO_DESCRIPTION_TAG_NAME);

	if (scenario_list_index != NONE)
	{
		struct multiplayer_scenario_description *scenario_list = multiplayer_scenario_description_definition_get(scenario_list_index);

		match_assert("c:\\halo\\SOURCE\\scenario\\multiplayer_scenario_description.c", MATCH_LINE_NTSC(67, 66), scenario_list);
		result = (struct scenario_description *)scenario_list->multiplayer_scenarios.address;
		*count = scenario_list->multiplayer_scenarios.count;
	}
	else
	{
		result = NULL;
		*count = 0;
	}

	return result;
}

boolean map_name_from_multiplayer_scenario_description_item(
	struct scenario_description const *item,
	char *buffer,
	long buffer_size)
{
	long i;

	match_assert("c:\\halo\\SOURCE\\scenario\\multiplayer_scenario_description.c", MATCH_LINE_NTSC(89, 88), item);
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
