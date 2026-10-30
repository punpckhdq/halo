/*
GAME_GLOBALS.C

symbols in this file:
000A4800 0050:
	_material_get_name (0000)
000A4850 00b0:
	_game_difficulty_get_value_by_difficulty (0000)
000A4900 0020:
	_game_difficulty_get_value (0000)
000A4920 0090:
	_game_difficulty_get_team_value (0000)
0025C218 0046:
	_global_difficulty_friend_settings (0000)
0025C260 000e:
	??_C@_0O@EIMPMJIN@hunter?5shield?$AA@ (0000)
0025C270 0004:
	??_C@_03MKIOHCPB@ice?$AA@ (0000)
0025C274 0014:
	??_C@_0BE@CBHAJPBP@elite?5energy?5shield?$AA@ (0000)
0025C288 0007:
	??_C@_06NGLEBKLK@leaves?$AA@ (0000)
0025C290 0006:
	??_C@_05ELAABIKM@water?$AA@ (0000)
0025C298 0008:
	??_C@_07OKMKCOHH@plastic?$AA@ (0000)
0025C2A0 000b:
	??_C@_0L@ONGKNPHB@human?5skin?$AA@ (0000)
0025C2AC 000c:
	??_C@_0M@JHKIJLAC@human?5armor?$AA@ (0000)
0025C2B8 0015:
	??_C@_0BF@POPDIFGF@cyborg?5energy?5shield?$AA@ (0000)
0025C2D0 000d:
	??_C@_0N@MOLHGIAG@cyborg?5armor?$AA@ (0000)
0025C2E0 0013:
	??_C@_0BD@JNMCHJMD@flood?5carrier?5form?$AA@ (0000)
0025C2F4 0012:
	??_C@_0BC@CLALDLJP@flood?5combat?5form?$AA@ (0000)
0025C308 0015:
	??_C@_0BF@CCOECBII@engineer?5force?5field?$AA@ (0000)
0025C320 000e:
	??_C@_0O@HGNLFKPA@engineer?5skin?$AA@ (0000)
0025C330 0015:
	??_C@_0BF@PLJMMMAA@jackal?5energy?5shield?$AA@ (0000)
0025C348 000c:
	??_C@_0M@ENBNENLN@hunter?5skin?$AA@ (0000)
0025C354 000d:
	??_C@_0N@FNKGCAL@hunter?5armor?$AA@ (0000)
0025C364 000c:
	??_C@_0M@EGKGIPKM@force?5field?$AA@ (0000)
0025C370 0006:
	??_C@_05NGDBJHKI@glass?$AA@ (0000)
0025C378 0007:
	??_C@_06GDEDHEOL@rubber?$AA@ (0000)
0025C380 000e:
	??_C@_0O@JCOFHDDO@metal?5?$CIthick?$CJ?$AA@ (0000)
0025C390 000d:
	??_C@_0N@LPNIIDHE@metal?5?$CIthin?$CJ?$AA@ (0000)
0025C3A0 000f:
	??_C@_0P@FANLOPFG@metal?5?$CIhollow?$CJ?$AA@ (0000)
0025C3B0 0005:
	??_C@_04FDHAPBNG@wood?$AA@ (0000)
0025C3B8 0005:
	??_C@_04HPKDHBOB@snow?$AA@ (0000)
0025C3C0 0006:
	??_C@_05PGFPPJKB@stone?$AA@ (0000)
0025C3C8 0005:
	??_C@_04EHONEKBC@sand?$AA@ (0000)
0025C3D0 0005:
	??_C@_04PGHGBCK@dirt?$AA@ (0000)
0025C3D8 003b:
	??_C@_0DL@CDBBBJKO@material_type?$DO?$DN0?5?$CG?$CG?5material_typ@ (0000)
0025C414 0023:
	??_C@_0CD@GJEDGBCH@c?3?2halo?2SOURCE?2game?2game_globals@ (0000)
0025C438 0045:
	??_C@_0EF@IDAEGPKL@?$CIvalue_type?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIvalue_type@ (0000)
002DE780 0084:
	_global_material_type_strings (0000)
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
