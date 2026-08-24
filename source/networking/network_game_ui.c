/*
NETWORK_GAME_UI.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_game_manager.h"
#include "network_game_ui.h"
#include "tag_groups.h"
#include "text_group.h"
#include "real_math.h"

/* ---------- public code */

wchar *network_game_get_random_player_name(void)
{
	wchar *name= L"";
	long tag_index= tag_loaded(UNICODE_STRING_LIST_TAG, "ui\\random_player_names");

	if (tag_index!=NONE)
	{
		struct tag_block *names= (struct tag_block *)tag_get(UNICODE_STRING_LIST_TAG, tag_index);

		if (names)
		{
			name= unicode_string_list_get_string(tag_index, seed_random_range(get_global_local_random_seed_address(), 0, (short)(names->count-1)));
		}
	}

	return name;
}
