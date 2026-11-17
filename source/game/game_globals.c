/*
GAME_GLOBALS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game_globals.h"
#include "game.h"
#include "scenario.h"
#include "game_engine.h"
#include "actor_definitions.h"
#include "text_group.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "sound_definitions.h"
#include "hud_definitions.h"
#include "effect_definitions.h"
#include "physics_constants.h"
#include "font_group.h"
#include "damage_effect_definitions.h"
#include "equipment_definitions.h"
#include "projectile_definitions.h"
#include "weapon_hud_interface_definition.h"
#include "particle_definitions.h"
#include "camera_track_definitions.h"
#include "color_table_group.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static real game_difficulty_get_value_by_difficulty(short value_type, short difficulty);

/* ---------- globals */

short const global_difficulty_friend_settings[NUMBER_OF_GAME_DIFFICULTY_VALUES] =
{
	_game_difficulty_friend_damage_scale,
	_game_difficulty_friend_vitality_scale,
	_game_difficulty_friend_shield_scale,
	_game_difficulty_friend_recharge_scale,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE
};

char *global_material_type_strings[NUMBER_OF_MATERIAL_TYPES] =
{
	"dirt",
	"sand",
	"stone",
	"snow",
	"wood",
	"metal (hollow)",
	"metal (thin)",
	"metal (thick)",
	"rubber",
	"glass",
	"force field",
	"grunt",
	"hunter armor",
	"hunter skin",
	"elite",
	"jackal",
	"jackal energy shield",
	"engineer skin",
	"engineer force field",
	"flood combat form",
	"flood carrier form",
	"cyborg armor",
	"cyborg energy shield",
	"human armor",
	"human skin",
	"sentinel",
	"monitor",
	"plastic",
	"water",
	"leaves",
	"elite energy shield",
	"ice",
	"hunter shield"
};

/* ---------- public code */

char const *material_get_name(
	short material_type)
{
	if (material_type != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\game\\game_globals.c", 905, material_type>=0 && material_type<NUMBER_OF_MATERIAL_TYPES);

		return global_material_type_strings[material_type];
	}

	return "NONE";
}

static real game_difficulty_get_value_by_difficulty(
	short value_type,
	short difficulty)
{
	real result = 1.f;
	struct game_globals *game_globals = scenario_get_game_globals();

	match_assert("c:\\halo\\SOURCE\\game\\game_globals.c", 922, (value_type >= 0) && (value_type < NUMBER_OF_GAME_DIFFICULTY_VALUES));

	if (game_globals)
	{
		struct game_globals_difficulty_information *difficulty_information = game_globals->difficulty_information.count ? TAG_BLOCK_GET_ELEMENT(&game_globals->difficulty_information, 0, struct game_globals_difficulty_information) : NULL;

		if (difficulty_information)
		{
			difficulty = PIN(difficulty, 0, NUMBER_OF_GAME_DIFFICULTY_LEVELS - 1);
			result = difficulty_information->game_difficulty_values[value_type][difficulty];
		}
	}

	return result;
}

real game_difficulty_get_value(
	short value_type)
{
	return game_difficulty_get_value_by_difficulty(value_type, game_difficulty_level_get());
}

real game_difficulty_get_team_value(
	short value_type,
	short team_index)
{
	short difficulty = game_difficulty_level_get();

	if (game_engine_running())
	{
		difficulty = _game_difficulty_level_normal;
	}
	else if (!game_team_is_enemy(_game_team_player, team_index))
	{
		match_assert("c:\\halo\\SOURCE\\game\\game_globals.c", 957, (value_type >= 0) && (value_type < NUMBER_OF_GAME_DIFFICULTY_VALUES));

		if (global_difficulty_friend_settings[value_type] == NONE)
		{
			difficulty = _game_difficulty_level_normal;
		}
		else
		{
			value_type = global_difficulty_friend_settings[value_type];
		}
	}

	return game_difficulty_get_value_by_difficulty(value_type, difficulty);
}
