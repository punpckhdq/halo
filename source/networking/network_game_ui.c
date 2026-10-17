/*
NETWORK_GAME_UI.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_game_ui.h"
#include "network_game_globals.h"
#include "network_messages.h"
#include "network_client_manager.h"
#include "units.h"
#include "players.h"
#include "text_group.h"
#include "game_engine_list.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

wchar_t const *network_game_get_random_player_name(
	void)
{
	wchar_t const *name = L"";
	long string_list_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\random_player_names");

	if (string_list_index != NONE)
	{
		struct unicode_string_list_group_header *string_list = unicode_string_list_definition_get(string_list_index);

		if (string_list)
		{
			name = unicode_string_list_get_string(
				string_list_index,
				local_random_range(0, string_list->string_references.count - 1));
		}
	}

	return name;
}

/* ---------- private code */
