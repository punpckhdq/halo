/*
GAME_ENGINE_ODDBALL.C

symbols in this file:
000A1630 0010:
	_oddball_engine_dispose (0000)
000A1640 0010:
	_oddball_engine_dispose_from_old_map (0000)
000A1650 0020:
	_oddball_engine_player_added (0000)
000A1670 0010:
	_oddball_engine_game_ending (0000)
000A1680 0010:
	_oddball_engine_game_starting (0000)
000A1690 0010:
	_oddball_engine_statistics_append (0000)
000A16A0 0010:
	_oddball_engine_handle_client_message (0000)
000A16B0 0010:
	_oddball_engine_handle_server_message (0000)
000A16C0 0010:
	_oddball_engine_pregame_post_rasterize (0000)
000A16D0 0010:
	_oddball_engine_post_rasterize (0000)
000A16E0 00d0:
	_oddball_add_score (0000)
000A17B0 0040:
	_oddball_add_time_with_ball (0000)
000A17F0 0030:
	_player_ball_count (0000)
000A1820 0010:
	_oddball_engine_player_damaged_player (0000)
000A1830 0030:
	_player_has_ball (0000)
000A1860 0040:
	_ball_available (0000)
000A18A0 02e0:
	_oddball_engine_display_score (0000)
000A1B80 0010:
	_oddball_engine_prespawn_player_update (0000)
000A1B90 0040:
	_oddball_weapon_drop (0000)
000A1BD0 0040:
	_oddball_get_score (0000)
000A1C10 0020:
	_oddball_ball_transfer_by_killing (0000)
000A1C30 0020:
	_accumulate_score_by_time (0000)
000A1C50 0020:
	_terminator_scoring_rules (0000)
000A1C70 0020:
	_oddball_test_flag (0000)
000A1C90 0050:
	_oddball_test_trait (0000)
000A1CE0 0050:
	_oddball_get_score_string (0000)
000A1D30 0070:
	_oddball_get_score_header_string (0000)
000A1DA0 0050:
	_oddball_get_team_score_string (0000)
000A1DF0 0140:
	_find_position_for_ball (0000)
000A1F30 0090:
	_create_the_ball (0000)
000A1FC0 0120:
	_oddball_engine_initialize_for_new_map (0000)
000A20E0 0070:
	_reset_ball (0000)
000A2150 0090:
	_update_ball_ownership (0000)
000A21E0 01c0:
	_oddball_engine_player_update (0000)
000A23A0 00c0:
	_oddball_engine_weapon_update (0000)
000A2460 00d0:
	_oddball_engine_update (0000)
000A2530 01c0:
	_oddball_engine_player_killed_player (0000)
000A26F0 00b0:
	_oddball_weapon_pickup (0000)
0025BE74 002a:
	??_C@_0CK@LKLFFONL@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BEA0 0034:
	??_C@_0DE@EJKHILOB@0?5?$CG?$CG?5?$CCthis?5map?5was?5not?5correctly@ (0000)
0025BED8 0040:
	??_C@_0EA@MPBOOMHB@?$CD?$CD?$CD?5failed?5to?5find?5any?5suitable?5@ (0000)
0025BF18 0023:
	??_C@_0CD@CKBFGHJI@?$CD?$CD?$CD?5failed?5to?5find?5the?5flag?5obje@ (0000)
0025BF3C 0028:
	??_C@_0CI@KJBBGHKH@?$CD?$CD?$CD?5failed?5to?5find?5ball?5spawn?5te@ (0000)
0025BF64 0028:
	??_C@_0CI@LAAKFGOG@?$CD?$CD?$CD?5failed?5to?5find?5ball?5spawn?5te@ (0000)
0025BF8C 000a:
	??_C@_09GMEHKGKL@ball_blue?$AA@ (0000)
0025BF98 000c:
	??_C@_0M@FNMEMKCB@target_blue?$AA@ (0000)
0025BFA4 002d:
	??_C@_0CN@DJFDFML@?$CBball_available?$CI?$CJ?5?$HM?$HM?5?$CIcapture_in@ (0000)
002DE560 0088:
	_oddball_engine (0000)
0043EBA8 0104:
	_bss_0043eba8 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "game_engine_list.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "text_group.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "game_sound.h"
#include "font_group.h"
#include "network_server_message_handler.h"
#include "scenery.h"
#include "game_engine.h"
#include "game_globals.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "items.h"
#include "unicode.h"
#include "game.h"
#include "objects.h"

/* ---------- constants */

enum
{
	oddball_message_you_have_the_ball = 30,
	oddball_message_ally_has_the_ball,
	oddball_message_enemy_has_the_ball,
	oddball_message_you_are_it,
	oddball_message_ally_is_it,
	oddball_message_enemy_is_it,
	oddball_message_ball_was_reset,
	oddball_message_enemy_has_the_ball_tick,
	oddball_message_ally_has_the_ball_tick,
	oddball_message_you_have_the_ball_tick
};

/* ---------- macros */

/* ---------- structures */

struct oddball_globals_type
{
	long score_to_win;
	long team_score[MULTIPLAYER_MAXIMUM_PLAYERS];
	long individual_score[MULTIPLAYER_MAXIMUM_PLAYERS];
	long ball_spawn_timer[MULTIPLAYER_MAXIMUM_PLAYERS];
	long current_ball_owner[MULTIPLAYER_MAXIMUM_PLAYERS];
};

/* ---------- prototypes */

static void oddball_engine_dispose(void);
static void oddball_engine_dispose_from_old_map(void);
static void oddball_engine_player_added(long player_index);
static void oddball_engine_game_ending(void);
static void oddball_engine_game_starting(void);
static void oddball_engine_statistics_append(struct game_statistics *permanent_statistics, struct game_statistics *game_statistics);
static void oddball_engine_handle_client_message(long player_index, void *encoded_message, short encoded_message_size);
static void oddball_engine_handle_server_message(void *encoded_message, short encoded_message_size);
static void oddball_engine_pregame_post_rasterize(void);
static void oddball_engine_post_rasterize(void);
static void oddball_add_score(long player_index);
static void oddball_add_time_with_ball(long player_index);
static long player_ball_count(long player_index);
static void oddball_engine_player_damaged_player(long killing_player_index, long dead_player_index, boolean friendly_fire);
static boolean oddball_engine_display_score(long player_index, long message, long message_data, word *buffer, long buffer_size);
static void oddball_engine_prespawn_player_update(long player_index);
static void oddball_weapon_drop(long weapon_index);
static long oddball_get_score(long player_index, enum get_score_type get_score_type);
static boolean oddball_ball_transfer_by_killing(void);
static boolean accumulate_score_by_time(void);
static boolean terminator_scoring_rules(void);
static boolean oddball_test_flag(long flag);
static boolean oddball_test_trait(long player_index, long trait);
static word *oddball_get_score_string(long player_index, word *buffer);
static word *oddball_get_score_header_string(word *buffer);
static word *oddball_get_team_score_string(long team_index, word *buffer);
static real_point3d find_position_for_ball(long ball_index);
static void create_the_ball(long ball_index);
static boolean oddball_engine_initialize_for_new_map(void);
static void reset_ball(long weapon_index);
static void update_ball_ownership(long player_index);
static void oddball_engine_player_update(long player_index);
static void oddball_engine_weapon_update(long weapon_index, struct weapon_datum *weapon);
static void oddball_engine_update(void);
static void oddball_engine_player_killed_player(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);
static boolean oddball_weapon_pickup(long weapon_index, long player_index);

/* ---------- globals */

struct game_engine oddball_engine =
{
	"oddball",
	_game_engine_oddball,
	oddball_engine_dispose,
	oddball_engine_initialize_for_new_map,
	oddball_engine_dispose_from_old_map,
	oddball_engine_player_added,
	oddball_engine_game_ending,
	oddball_engine_game_starting,
	oddball_engine_statistics_append,
	oddball_engine_handle_client_message,
	oddball_engine_handle_server_message,
	oddball_engine_pregame_post_rasterize,
	oddball_engine_post_rasterize,
	oddball_engine_player_update,
	oddball_engine_weapon_update,
	oddball_weapon_pickup,
	oddball_weapon_drop,
	oddball_engine_update,
	oddball_get_score,
	oddball_get_score_string,
	oddball_get_score_header_string,
	oddball_get_team_score_string,
	NULL, // allow_pick_up
	oddball_engine_player_damaged_player,
	oddball_engine_player_killed_player,
	oddball_engine_display_score,
	NULL, // starting_location_rating
	oddball_engine_prespawn_player_update,
	NULL, // postspawn_player_update
	NULL, // game_engine_player_get_team_index
	NULL, // goal_matches_player
	oddball_test_flag,
	oddball_test_trait,
	NULL, // game_engine_did_player_win
};

static struct oddball_globals_type oddball_globals;

/* ---------- private code */

static real_point3d find_position_for_ball(
	long ball_index)
{
	struct scenario *scenario = global_scenario_get();
	real_point3d position;
	long flag_index = NONE;

	if (!game_engine_get_variant()->game_engine_variant.oddball.random_start)
	{
		flag_index = find_netgame_flag(NULL, 0.f, 0.f, _netgame_flag_oddball_ball_spawn, (short)ball_index);
	}

	if (flag_index == NONE)
	{
		long spawn_count = 0;
		short index;

		for (index = 0; index < scenario->netgame_flags.count; index++)
		{
			struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, index, struct scenario_netgame_flag);

			if (flag->type == _netgame_flag_oddball_ball_spawn)
			{
				spawn_count++;
			}
		}

		if (!spawn_count)
		{
			error(_error_silent, "### failed to find any suitable netgame flag to create the ball");
			match_assert("c:\\halo\\SOURCE\\game\\game_engine_oddball.c", 173, 0 && "this map was not correctly setup for oddball");
		}
		else
		{
			spawn_count = random_range(0, (short)spawn_count);

			for (index = 0; index < scenario->netgame_flags.count; index++)
			{
				struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, index, struct scenario_netgame_flag);

				if (flag->type == _netgame_flag_oddball_ball_spawn)
				{
					if (spawn_count == 0)
					{
						flag_index = index;
						break;
					}

					spawn_count--;
				}
			}
		}
	}

	if (flag_index != NONE)
	{
		position = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag)->position;
	}

	return position;
}

static void create_the_ball(
	long ball_index)
{
	if (!oddball_ball_transfer_by_killing())
	{
		struct object_placement_data placement_data;
		long definition_index = get_ball_definition_index();
		long object_index = NONE;

		if (definition_index == NONE)
		{
			error(_error_silent, "### failed to find the flag object");
		}
		else
		{
			object_placement_data_new(&placement_data, definition_index, NONE);
			placement_data.position = find_position_for_ball(ball_index);
			object_index = object_new(&placement_data);
			weapon_get(object_index)->object.owner_team_index = (short)ball_index;
			object_set_automatic_deactivation(object_index, FALSE);
		}
	}

	return;
}

static void oddball_engine_dispose(
	void)
{
	return;
}

static boolean oddball_engine_initialize_for_new_map(
	void)
{
	struct scenario *scenario = global_scenario_get();
	long index;

	memset(&oddball_globals, 0, sizeof(oddball_globals));
	oddball_globals.score_to_win = game_engine_get_variant()->universal_variant.score_to_win;

	if (accumulate_score_by_time())
	{
		oddball_globals.score_to_win *= TICKS_PER_MINUTE;
	}

	if (find_netgame_flag(NULL, 0.f, 0.f, _netgame_flag_oddball_ball_spawn, 0) == NONE)
	{
		error(_error_silent, "### failed to find ball spawn team id 0");
	}

	if (find_netgame_flag(NULL, 0.f, 0.f, _netgame_flag_oddball_ball_spawn, 1) == NONE)
	{
		error(_error_silent, "### failed to find ball spawn team id 1");
	}

	for (index = 0; index < MULTIPLAYER_MAXIMUM_PLAYERS; index++)
	{
		oddball_globals.current_ball_owner[index] = NONE;
	}

	if (!oddball_ball_transfer_by_killing())
	{
		long ball_index;
		long ball_count = game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count;
		long spawn_timer = 0;
		long spawn_delay = 15 * TICKS_PER_SECOND;

		for (ball_index = 0; ball_index < ball_count; ball_index++)
		{
			spawn_timer += spawn_delay;
			oddball_globals.ball_spawn_timer[ball_index] = spawn_timer;
		}
	}
	else
	{
		long ball_index;
		long ball_count = game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count;

		for (ball_index = 0; ball_index < ball_count; ball_index++)
		{
			oddball_globals.ball_spawn_timer[ball_index] = 0;
			create_the_ball(ball_index);
		}
	}

	return TRUE;
}

static void oddball_engine_dispose_from_old_map(
	void)
{
	return;
}

static void oddball_engine_player_added(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	return;
}

static void oddball_engine_game_ending(
	void)
{
	return;
}

static void oddball_engine_game_starting(
	void)
{
	return;
}

static void oddball_engine_statistics_append(
	struct game_statistics *permanent_statistics,
	struct game_statistics *game_statistics)
{
	return;
}

static void oddball_engine_handle_client_message(
	long player_index,
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void oddball_engine_handle_server_message(
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void oddball_engine_pregame_post_rasterize(
	void)
{
	return;
}

static void oddball_engine_post_rasterize(
	void)
{
	return;
}

static void reset_ball(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	real_point3d position = find_position_for_ball(weapon->object.owner_team_index);

	if (game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count <= 2)
	{
		game_engine_play_multiplayer_sound(_multiplayer_sound_hill_move);
	}

	game_engine_flag_reset(weapon_index, &position);
	SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag, FALSE);

	return;
}

static void oddball_add_score(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	oddball_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)]++;
	oddball_globals.team_score[player->team_index]++;

	if (oddball_globals.score_to_win - oddball_globals.team_score[player->team_index] == 30 * TICKS_PER_SECOND)
	{
		if (game_engine_has_teams())
		{
			game_engine_play_multiplayer_sound(player->team_index ? _multiplayer_sound_blue_30_seconds : _multiplayer_sound_red_30_seconds);
		}
		else
		{
			game_engine_play_multiplayer_sound(_multiplayer_sound_30_seconds);
		}
	}

	if (oddball_globals.score_to_win - oddball_globals.team_score[player->team_index] == 60 * TICKS_PER_SECOND)
	{
		if (game_engine_has_teams())
		{
			game_engine_play_multiplayer_sound(player->team_index ? _multiplayer_sound_blue_60_seconds : _multiplayer_sound_red_60_seconds);
		}
		else
		{
			game_engine_play_multiplayer_sound(_multiplayer_sound_60_seconds);
		}
	}

	if (oddball_globals.team_score[player->team_index] >= oddball_globals.score_to_win)
	{
		game_engine_end_game();
	}

	return;
}

static void oddball_add_time_with_ball(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	if (game_engine_get_variant()->game_engine_variant.oddball.oddball_ball_type == _oddball_normal)
	{
		game_engine_state_message(player_index, oddball_message_you_have_the_ball_tick, player_index);
	}

	player->statistics.multiplayer_statistics.oddball_statistics.time_with_the_ball++;
	oddball_add_score(player_index);

	return;
}

static long player_ball_count(
	long player_index)
{
	long count = 0;
	long ball_count = game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count;
	long ball_index;

	for (ball_index = 0; ball_index < ball_count; ball_index++)
	{
		if (oddball_globals.current_ball_owner[ball_index] == player_index)
		{
			count++;
		}
	}

	return count;
}

static void update_ball_ownership(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	if (!oddball_ball_transfer_by_killing())
	{
		long ball_index;
		long unit_index;

		for (ball_index = 0; ball_index < MULTIPLAYER_MAXIMUM_PLAYERS; ball_index++)
		{
			if (oddball_globals.current_ball_owner[ball_index] == player_index)
			{
				oddball_globals.current_ball_owner[ball_index] = NONE;
			}
		}

		unit_index = player->unit_index;
		if (unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);

			if (unit->unit.current_weapon_index != NONE)
			{
				long weapon_index = unit->unit.weapon_object_indices[unit->unit.current_weapon_index];

				if (weapon_index != NONE && weapon_is_flag(weapon_index))
				{
					struct weapon_datum *weapon = weapon_get(weapon_index);

					oddball_globals.current_ball_owner[weapon->object.owner_team_index] = player_index;
				}
			}
		}
	}

	return;
}

static void oddball_engine_player_update(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	long ball_count;

	game_engine_state_message(player_index, NONE, NONE);
	update_ball_ownership(player_index);
	ball_count = player_ball_count(player_index);
	player->multiplayer.speed_multiplier = 1.f;

	if (ball_count > 0 && game_engine_get_variant()->game_engine_variant.oddball.trait_with_ball != _game_trait_invisible)
	{
		game_engine_player_depower_active_camo(player_index);
	}

	if (ball_count > 0)
	{
		switch (game_engine_get_variant()->game_engine_variant.oddball.speed_with_ball)
		{
		case _oddball_speed_normal:
			player->multiplayer.speed_multiplier = 1.f;
			break;
		case _oddball_speed_faster:
			player->multiplayer.speed_multiplier = 1.25f;
			break;
		default:
			player->multiplayer.speed_multiplier = 0.75f;
			break;
		}
	}

	if (game_engine_can_score() && accumulate_score_by_time() && ball_count > 0)
	{
		long ball_index;

		for (ball_index = 0; ball_index < ball_count; ball_index++)
		{
			oddball_add_time_with_ball(player_index);
		}
	}

	if (oddball_ball_transfer_by_killing() && ball_count > 0)
	{
		game_engine_state_message(player_index, oddball_message_you_are_it, player_index);
	}

	if (player->unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(player->unit_index);

		if (unit->unit.current_weapon_index != NONE)
		{
			long weapon_index = unit->unit.weapon_object_indices[unit->unit.current_weapon_index];

			if (weapon_index != NONE && weapon_is_flag(weapon_index))
			{
				long seconds = oddball_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)] / TICKS_PER_SECOND;
				struct weapon_datum *weapon = weapon_get(weapon_index);
				long score = oddball_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)];
				long countdown_interval = 5 * TICKS_PER_SECOND;

				if (score > 0 && score % countdown_interval == 0 && score < oddball_globals.score_to_win)
				{
					game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_timer_end);
				}

				weapon->weapon.magazines[0].rounds_loaded = (short)seconds;
			}
		}
	}

	return;
}

static void oddball_engine_weapon_update(
	long weapon_index,
	struct weapon_datum *weapon)
{
	long ball_reset_delay = 40 * TICKS_PER_SECOND;
	real vertical_offset = 0.f;
	real_point3d position;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_oddball.c", 595, weapon_is_flag(weapon_index));

	item_get_position_even_if_in_inventory(weapon_index, &position);
	game_engine_set_goal_position(weapon->object.owner_team_index, &position, vertical_offset, "ball_blue", NONE, NONE, oddball_globals.current_ball_owner[weapon->object.owner_team_index]);

	if ((unsigned long)(game_time_get() - weapon->item.last_owned_time) > ball_reset_delay && weapon_is_flag(weapon_index))
	{
		boolean connected_to_map = TEST_FLAG(weapon->object.flags, _object_connected_to_map_bit);
		boolean attached = weapon->object.parent_object_index != NONE;

		if (connected_to_map && !attached)
		{
			if (TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag))
			{
				game_show_score(NONE, oddball_message_ball_was_reset);
			}

			reset_ball(weapon_index);
		}
	}

	return;
}

static void oddball_engine_update(
	void)
{
	long ball_spawn_count;
	long ball_index;

	if (game_time_get() == 2 * TICKS_PER_SECOND)
	{
		game_engine_play_multiplayer_sound(game_engine_has_teams() ? _multiplayer_sound_team_oddball : _multiplayer_sound_oddball);
	}

	ball_spawn_count = game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count;

	for (ball_index = 0; ball_index < ball_spawn_count; ball_index++)
	{
		if (oddball_globals.ball_spawn_timer[ball_index] > 0 && --oddball_globals.ball_spawn_timer[ball_index] == 0)
		{
			game_engine_play_multiplayer_sound(_multiplayer_sound_oddball_spawn);
			create_the_ball(ball_index);
		}
	}

	if (oddball_ball_transfer_by_killing())
	{
		for (ball_index = 0; ball_index < ball_spawn_count; ball_index++)
		{
			long owner_index = oddball_globals.current_ball_owner[ball_index];

			if (owner_index == NONE)
			{
				game_engine_clear_goal_position((short)ball_index);
			}
			else
			{
				struct player_datum *player = player_get(owner_index);

				if (player->unit_index != NONE)
				{
					struct unit_datum *unit = unit_get(player->unit_index);
					real vertical_offset = 0.f;

					game_engine_set_goal_position((short)ball_index, &unit->object.bounding_sphere_center, vertical_offset, "target_blue", NONE, NONE, owner_index);
				}
			}
		}
	}

	return;
}

static void oddball_engine_player_damaged_player(
	long killing_player_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

boolean player_has_ball(
	long player_index)
{
	long ball_count = game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count;
	boolean result = FALSE;
	long ball_index;

	for (ball_index = 0; ball_index < ball_count; ball_index++)
	{
		if (oddball_globals.current_ball_owner[ball_index] == player_index)
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

boolean ball_available(
	void)
{
	long ball_count = game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count;
	boolean result = FALSE;
	long ball_index;

	for (ball_index = 0; ball_index < ball_count; ball_index++)
	{
		if (oddball_globals.ball_spawn_timer[ball_index] != 0)
		{
			continue;
		}

		if (oddball_globals.current_ball_owner[ball_index] == NONE)
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

static void oddball_engine_player_killed_player(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	if (oddball_ball_transfer_by_killing())
	{
		long ball_count = game_engine_get_variant()->game_engine_variant.oddball.ball_spawn_count;
		long capture_index = NONE;
		long ball_index;

		match_assert("c:\\halo\\SOURCE\\game\\game_engine_oddball.c", 755, dead_player_index != NONE);

		if (killing_player_index != NONE && !friendly_fire)
		{
			struct player_datum *killing_player = player_get(killing_player_index);

			if (player_has_ball(dead_player_index))
			{
				killing_player->statistics.multiplayer_statistics.oddball_statistics.ball_carrier_kills++;

				if (terminator_scoring_rules() && game_engine_can_score())
				{
					oddball_add_score(killing_player_index);
				}
			}
			else if (player_has_ball(killing_player_index))
			{
				killing_player->statistics.multiplayer_statistics.oddball_statistics.kills_as_ball_carrier++;

				if (terminator_scoring_rules() && game_engine_can_score())
				{
					oddball_add_score(killing_player_index);
				}
			}
			else if (ball_available())
			{
				oddball_add_score(killing_player_index);
			}

			if (killing_player->unit_index != NONE)
			{
				for (ball_index = 0; ball_index < ball_count; ball_index++)
				{
					if (oddball_globals.ball_spawn_timer[ball_index] == 0 && capture_index == NONE && oddball_globals.current_ball_owner[ball_index] == NONE)
					{
						capture_index = ball_index;
					}

					if (oddball_globals.current_ball_owner[ball_index] == dead_player_index)
					{
						capture_index = ball_index;
						break;
					}
				}

				match_assert("c:\\halo\\SOURCE\\game\\game_engine_oddball.c", 806, !ball_available() || (capture_index != NONE));

				if (capture_index != NONE)
				{
					game_show_score_you_ally_enemy(killing_player_index, oddball_ball_transfer_by_killing() ? NONE : oddball_message_you_are_it, oddball_message_ally_is_it, oddball_message_enemy_is_it, killing_player_index);
					oddball_globals.current_ball_owner[capture_index] = killing_player_index;
				}
			}
		}

		for (ball_index = 0; ball_index < ball_count; ball_index++)
		{
			if (oddball_globals.current_ball_owner[ball_index] == dead_player_index)
			{
				oddball_globals.current_ball_owner[ball_index] = NONE;
			}
		}
	}

	return;
}

static boolean oddball_engine_display_score(
	long player_index,
	long message,
	long message_data,
	word *buffer,
	long buffer_size)
{
	struct player_datum *player = player_get(player_index);
	boolean result = TRUE;
	wchar_t *string;

	switch (message)
	{
	case oddball_message_you_are_it:
		multiplayer_game_text_get_string(string, _string_you_are_it);
		ustrncpy(buffer, string, buffer_size);
		break;
	case oddball_message_ally_is_it:
		multiplayer_game_text_get_string(string, _string_ally_is_it);
		ustrncpy(buffer, string, buffer_size);
		break;
	case oddball_message_enemy_is_it:
	{
		struct player_datum *message_player = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_name_is_it);
		usnprintf(buffer, buffer_size, string, message_player->name);
		break;
	}
	case oddball_message_you_have_the_ball:
		multiplayer_game_text_get_string(string, _string_you_have_the_ball);
		ustrncpy(buffer, string, buffer_size);
		break;
	case oddball_message_ally_has_the_ball:
		multiplayer_game_text_get_string(string, _string_an_ally_has_the_ball);
		ustrncpy(buffer, string, buffer_size);
		break;
	case oddball_message_enemy_has_the_ball:
	{
		struct player_datum *message_player = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_name_has_the_ball);
		usnprintf(buffer, buffer_size, string, message_player->name);
		break;
	}
	case oddball_message_enemy_has_the_ball_tick:
	case oddball_message_ally_has_the_ball_tick:
	case oddball_message_you_have_the_ball_tick:
	{
		struct player_datum *message_player = player_get(message_data);
		long seconds = oddball_globals.team_score[message_player->team_index] / TICKS_PER_SECOND;

		if (message == oddball_message_you_have_the_ball_tick)
		{
			wchar_t *place_name = get_place_name(game_engine_get_place(player_index, _get_score_team));

			multiplayer_game_text_get_string(string, _string_place_score_seconds);
			usnprintf(buffer, buffer_size, string, place_name, seconds);
		}
		else if (message == oddball_message_ally_has_the_ball_tick)
		{
			multiplayer_game_text_get_string(string, _string_ally_name_has_the_ball_score_seconds);
			usnprintf(buffer, buffer_size, string, message_player->name, seconds);
		}
		else if (message == oddball_message_enemy_has_the_ball_tick)
		{
			multiplayer_game_text_get_string(string, _string_enemy_name_has_the_ball_score_seconds);
			usnprintf(buffer, buffer_size, string, message_player->name, seconds);
		}
		break;
	}
	default:
		result = FALSE;
		break;
	}

	return result;
}

static void oddball_engine_prespawn_player_update(
	long player_index)
{
	return;
}

static boolean oddball_weapon_pickup(
	long weapon_index,
	long player_index)
{
	boolean result = TRUE;
	struct weapon_datum *weapon = weapon_get(weapon_index);

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_oddball.c", 942, weapon_is_flag(weapon_index));

	if (oddball_ball_transfer_by_killing())
	{
		game_show_score_you_ally_enemy(player_index, oddball_message_you_have_the_ball, oddball_message_ally_has_the_ball, oddball_message_enemy_has_the_ball, player_index);
	}
	else
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index != NONE)
		{
			result = !unit_has_weapon_with_flag(player->unit_index, 3);

			if (result)
			{
				SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag, TRUE);
			}
		}
	}

	return result;
}

static void oddball_weapon_drop(
	long weapon_index)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_engine_oddball.c", 968, weapon_is_flag(weapon_index));

	return;
}

static long oddball_get_score(
	long player_index,
	enum get_score_type get_score_type)
{
	long score;
	struct player_datum *player = player_get(player_index);

	if (get_score_type == _get_score_team)
	{
		score = oddball_globals.team_score[player->team_index];
	}
	else
	{
		score = oddball_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)];
	}

	return score;
}

static boolean oddball_ball_transfer_by_killing(
	void)
{
	boolean result;

	switch (game_engine_get_variant()->game_engine_variant.oddball.oddball_ball_type)
	{
	case _oddball_magic:
	case _oddball_terminator:
		result = TRUE;
		break;
	default:
		result = FALSE;
		break;
	}

	return result;
}

static boolean accumulate_score_by_time(
	void)
{
	boolean result;

	switch (game_engine_get_variant()->game_engine_variant.oddball.oddball_ball_type)
	{
	case _oddball_terminator:
		result = FALSE;
		break;
	default:
		result = TRUE;
		break;
	}

	return result;
}

static boolean terminator_scoring_rules(
	void)
{
	boolean result;

	switch (game_engine_get_variant()->game_engine_variant.oddball.oddball_ball_type)
	{
	case _oddball_terminator:
		result = TRUE;
		break;
	default:
		result = FALSE;
		break;
	}

	return result;
}

static boolean oddball_test_flag(
	long flag)
{
	boolean result = FALSE;

	switch (flag)
	{
	case _game_engine_show_kills_with_score_bit:
		result = terminator_scoring_rules();
		break;
	}

	return result;
}

static boolean oddball_test_trait(
	long player_index,
	long trait)
{
	boolean result = FALSE;

	if (trait != _game_trait_none)
	{
		if (player_has_ball(player_index))
		{
			result = trait == game_engine_get_variant()->game_engine_variant.oddball.trait_with_ball;
		}
		else
		{
			result = trait == game_engine_get_variant()->game_engine_variant.oddball.trait_without_ball;
		}
	}

	return result;
}

static word *oddball_get_score_string(
	long player_index,
	word *buffer)
{
	long score = oddball_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)];

	if (terminator_scoring_rules())
	{
		usprintf(buffer, L"%d", score);
	}
	else
	{
		ticks_to_unicode_time_string(score, 256, buffer);
	}

	return buffer;
}

static word *oddball_get_score_header_string(
	word *buffer)
{
	short string_index;
	wchar_t *string;

	if (terminator_scoring_rules())
	{
		string_index = _string_score;
	}
	else
	{
		string_index = _string_time;
	}

	multiplayer_game_text_get_string(string, string_index);
	ustrcpy(buffer, string);

	return buffer;
}

static word *oddball_get_team_score_string(
	long team_index,
	word *buffer)
{
	long score = oddball_globals.team_score[team_index];

	if (terminator_scoring_rules())
	{
		usprintf(buffer, L"%d", score);
	}
	else
	{
		ticks_to_unicode_time_string(score, 256, buffer);
	}

	return buffer;
}

