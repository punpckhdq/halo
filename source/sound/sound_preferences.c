/*
SOUND_PREFERENCES.C

symbols in this file:
001BF310 0010:
	_read_sound_preferences (0000)
001BF320 0010:
	_write_sound_preferences (0000)
00317A84 001c:
	_data_00317a84 (0000)
	_sound_channel_type_flags (0014)
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "object_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "sound_environment_definitions.h"
#include "sound_manager.h"
#include "sound_definitions.h"
#include "platform_sound.h"
#include "sound_preferences.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
