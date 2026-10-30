/*
GAME_ENGINE_CTF.C

symbols in this file:
0009EB60 0060:
	_create_the_flag_at_position (0000)
0009EBC0 0010:
	_ctf_engine_dispose (0000)
0009EBD0 0060:
	_create_the_flag (0000)
0009EC30 0040:
	_ctf_single_flag_what_is_up_message (0000)
0009EC70 0010:
	_ctf_engine_dispose_from_old_map (0000)
0009EC80 0020:
	_ctf_engine_player_added (0000)
0009ECA0 0010:
	_ctf_engine_game_ending (0000)
0009ECB0 0010:
	_ctf_engine_game_starting (0000)
0009ECC0 0010:
	_ctf_engine_statistics_append (0000)
0009ECD0 0010:
	_ctf_engine_handle_client_message (0000)
0009ECE0 0010:
	_ctf_engine_handle_server_message (0000)
0009ECF0 0010:
	_ctf_engine_pregame_post_rasterize (0000)
0009ED00 0010:
	_ctf_engine_post_rasterize (0000)
0009ED10 00c0:
	_player_score (0000)
0009EDD0 0040:
	_ctf_flag_failure_sound (0000)
0009EE10 0070:
	_get_player_with_this_flag (0000)
0009EE80 0080:
	_ctf_engine_allow_pick_up (0000)
0009EF00 0010:
	_ctf_engine_player_damaged_player (0000)
0009EF10 0010:
	_ctf_engine_player_killed_player (0000)
0009EF20 03a0:
	_ctf_engine_display_score (0000)
0009F2C0 0010:
	_ctf_engine_prespawn_player_update (0000)
0009F2D0 0010:
	_ctf_state_message_update_warning (0000)
0009F2E0 0040:
	_ctf_sound_update_warning (0000)
0009F320 0020:
	_ctf_set_flag_warning (0000)
0009F340 0040:
	_ctf_weapon_drop (0000)
0009F380 0040:
	_ctf_get_score (0000)
0009F3C0 0010:
	_ctf_test_flag (0000)
0009F3D0 0040:
	_ctf_get_score_string (0000)
0009F410 0060:
	_ctf_get_score_header_string (0000)
0009F470 0030:
	_ctf_get_team_score_string (0000)
0009F4A0 03d0:
	_ctf_engine_initialize_for_new_map (0000)
0009F870 0050:
	_weapon_reset_flag (0000)
0009F8C0 0090:
	_player_reset_flag (0000)
0009F950 0050:
	_in_scoring_range (0000)
0009F9A0 0150:
	_ctf_engine_player_update (0000)
0009FAF0 0240:
	_ctf_engine_weapon_update (0000)
0009FD30 0080:
	_ctf_engine_update (0000)
0009FDB0 0160:
	_ctf_weapon_pickup (0000)
0009FF10 0120:
	_ctf_engine_starting_location_rating (0000)
0025BBC4 000f:
	??_C@_0P@HPMPGFLN@created?5a?5flag?$AA@ (0000)
0025BBD4 001a:
	??_C@_0BK@JCPFMGLB@failed?5to?5create?5the?5flag?$AA@ (0000)
0025BBF0 0015:
	??_C@_0BF@KKINPGPB@NONE?5?$CB?$DN?5weapon_index?$AA@ (0000)
0025BC08 0026:
	??_C@_0CG@KLNGAFMF@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BC30 0013:
	??_C@_0BD@MAOGAFHI@NONE?5?$CB?$DN?5team_index?$AA@ (0000)
0025BC44 0018:
	??_C@_0BI@HLCGDKNJ@game_engine_can_score?$CI?$CJ?$AA@ (0000)
0025BC60 004b:
	??_C@_0EL@BLLOIBND@NETGAME_FLAG_WARNING?5starting?5lo@ (0000)
0025BCAC 003c:
	??_C@_0DM@LOHOHMDO@NETGAME_FLAG_WARNING?5starting?5lo@ (0000)
0025BCE8 002f:
	??_C@_0CP@KMKIMAED@?$CIflag_to_create?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIflag_t@ (0000)
0025BD18 0024:
	??_C@_0CE@FKOBANOL@failed?5to?5find?5one?5of?5the?5ctf?5fl@ (0000)
0025BD3C 001d:
	??_C@_0BN@PNGPDONM@ctf?5started?5up?5without?5teams?$AA@ (0000)
0025BD5C 0013:
	??_C@_0BD@CBKEKJFH@NONE?5?$CB?$DN?5unit_index?$AA@ (0000)
0025BD70 0036:
	??_C@_0DG@PMEHEDDI@weapon?9?$DOobject?4owner_team_index?5@ (0000)
0025BDA8 000a:
	??_C@_09NOBKAKN@flag_blue?$AA@ (0000)
0025BDB8 0008:
	__real@3fd51eb860000000 (0000)
002DE400 0088:
	_ctf_engine (0000)
0043E914 0030:
	_bss_0043e914 (0000)
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
#include "text_group.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "main.h"
#include "weapons.h"
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
	ctf_message_score = 30,
	ctf_message_you_scored,
	ctf_message_enemy_scored,
	ctf_message_good_guys_scored,
	ctf_message_you_have_the_flag,
	ctf_message_you_returned_the_flag,
	ctf_message_enemy_has_the_flag,
	ctf_message_enemy_has_the_flag_tick,
	ctf_message_enemy_returned_the_flag,
	ctf_message_good_guys_have_the_flag,
	ctf_message_good_guys_returned_the_flag,
	ctf_message_your_flag_was_returned,
	ctf_message_enemy_flag_was_returned,
	ctf_message_time_expired,
	ctf_message_you_are_on_attack,
	ctf_message_you_are_on_defense
};

enum
{
	NUMBER_OF_CTF_TEAMS = 2 /* fake name */
};

/* ---------- macros */

#define OTHER_TEAM(team_index) (((team_index) + 1) % NUMBER_OF_CTF_TEAMS) /* fake name */

/* ---------- structures */

struct ctf_globals_type
{
	struct scenario_netgame_flag *flags[NUMBER_OF_CTF_TEAMS];
	long weapon_index[NUMBER_OF_CTF_TEAMS];
	long score[NUMBER_OF_CTF_TEAMS];
	long score_to_win;
	boolean flag_warning[NUMBER_OF_CTF_TEAMS];
	long flag_warning_time_in_ticks[NUMBER_OF_CTF_TEAMS];
	long flag_swap_timer;
	long non_deterministic_next_flag_failure_time;
};

/* ---------- prototypes */

static long create_the_flag_at_position(real_point3d *position);
static void ctf_engine_dispose(void);
static void create_the_flag(long team_index);
static void ctf_single_flag_what_is_up_message(long flag_team_index);
static void ctf_engine_dispose_from_old_map(void);
static void ctf_engine_player_added(long player_index);
static void ctf_engine_game_ending(void);
static void ctf_engine_game_starting(void);
static void ctf_engine_statistics_append(struct game_statistics *permanent_statistics, struct game_statistics *game_statistics);
static void ctf_engine_handle_client_message(long player_index, void *encoded_message, short encoded_message_size);
static void ctf_engine_handle_server_message(void *encoded_message, short encoded_message_size);
static void ctf_engine_pregame_post_rasterize(void);
static void ctf_engine_post_rasterize(void);
static void player_score(long player_index, long team_index);
static void ctf_flag_failure_sound(long player_index);
static long get_player_with_this_flag(long weapon_index);
static boolean ctf_engine_allow_pick_up(long unit_index, long item_index);
static void ctf_engine_player_damaged_player(long killing_player_index, long dead_player_index, boolean friendly_fire);
static void ctf_engine_player_killed_player(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);
static boolean ctf_engine_display_score(long player_index, long message, long message_data, word *buffer, long buffer_size);
static void ctf_engine_prespawn_player_update(long player_index);
void ctf_state_message_update_warning(long team_index);
static void ctf_sound_update_warning(long team_index);
static void ctf_set_flag_warning(long team_index, boolean warning);
static void ctf_weapon_drop(long weapon_index);
static long ctf_get_score(long player_index, enum get_score_type get_score_type);
static boolean ctf_test_flag(long flag);
static word *ctf_get_score_string(long player_index, word *buffer);
static word *ctf_get_score_header_string(word *buffer);
static word *ctf_get_team_score_string(long team_index, word *buffer);
static boolean ctf_engine_initialize_for_new_map(void);
static void weapon_reset_flag(long weapon_index);
static void player_reset_flag(long player_index, long weapon_index);
static boolean in_scoring_range(real_point3d *location, real distance, long team_index);
static void ctf_engine_player_update(long player_index);
static void ctf_engine_weapon_update(long weapon_index, struct weapon_datum *weapon);
static void ctf_engine_update(void);
static boolean ctf_weapon_pickup(long weapon_index, long player_index);
static real ctf_engine_starting_location_rating(long player_index, struct scenario_player *starting_location);

/* ---------- globals */

struct game_engine ctf_engine =
{
	"ctf",
	_game_engine_ctf,
	ctf_engine_dispose,
	ctf_engine_initialize_for_new_map,
	ctf_engine_dispose_from_old_map,
	ctf_engine_player_added,
	ctf_engine_game_ending,
	ctf_engine_game_starting,
	ctf_engine_statistics_append,
	ctf_engine_handle_client_message,
	ctf_engine_handle_server_message,
	ctf_engine_pregame_post_rasterize,
	ctf_engine_post_rasterize,
	ctf_engine_player_update,
	ctf_engine_weapon_update,
	ctf_weapon_pickup,
	ctf_weapon_drop,
	ctf_engine_update,
	ctf_get_score,
	ctf_get_score_string,
	ctf_get_score_header_string,
	ctf_get_team_score_string,
	ctf_engine_allow_pick_up,
	ctf_engine_player_damaged_player,
	ctf_engine_player_killed_player,
	ctf_engine_display_score,
	ctf_engine_starting_location_rating,
	ctf_engine_prespawn_player_update,
	NULL, // postspawn_player_update
	NULL, // game_engine_player_get_team_index
	NULL, // goal_matches_player
	ctf_test_flag,
	NULL, // game_engine_test_trait
	NULL, // game_engine_did_player_win
};

static struct ctf_globals_type ctf_globals;

/* ---------- private code */

static long create_the_flag_at_position(
	real_point3d *position)
{
	struct object_placement_data placement_data;
	long definition_index = get_flag_definition_index();
	long weapon_index = NONE;

	object_placement_data_new(&placement_data, definition_index, NONE);
	placement_data.position = *position;
	weapon_index = object_new(&placement_data);
	object_set_automatic_deactivation(weapon_index, FALSE);
	OutputDebugString("created a flag");

	return weapon_index;
}

static void ctf_engine_dispose(
	void)
{
	return;
}

static void create_the_flag(
	long team_index)
{
	if (ctf_globals.flags[team_index])
	{
		long weapon_index = create_the_flag_at_position(&ctf_globals.flags[team_index]->position);

		match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 189, NONE != weapon_index);

		if (weapon_index == NONE)
		{
			error(_error_silent, "failed to create the flag");
		}
		else
		{
			weapon_get(weapon_index)->object.owner_team_index = (short)team_index;
			ctf_globals.weapon_index[team_index] = weapon_index;
		}
	}

	return;
}

static void ctf_single_flag_what_is_up_message(
	long flag_team_index)
{
	game_show_score_team(flag_team_index % NUMBER_OF_CTF_TEAMS, ctf_message_you_are_on_defense);
	game_show_score_team(OTHER_TEAM(flag_team_index), ctf_message_you_are_on_attack);

	return;
}

static boolean ctf_engine_initialize_for_new_map(
	void)
{
	struct scenario *scenario;
	long team_index;
	short starting_location_count;
	short starting_location_index;

	if (!game_engine_get_variant()->universal_variant.teams)
	{
		error(_error_silent, "ctf started up without teams");
	}

	memset(&ctf_globals, 0, sizeof(ctf_globals));
	ctf_globals.weapon_index[0] = NONE;
	ctf_globals.weapon_index[1] = NONE;
	timeout_for_endgame_sound = 2 * TICKS_PER_SECOND;
	scenario = global_scenario_get();

	for (team_index = 0; team_index < NUMBER_OF_CTF_TEAMS; team_index++)
	{
		long flag_team_index;
		long flag_index = find_netgame_flag(NULL, 0.f, 0.f, _netgame_flag_ctf_flag, (short)team_index);

		ctf_globals.score[team_index] = 0;
		flag_team_index = team_index;

		if (game_engine_get_variant()->game_engine_variant.ctf.assault)
		{
			flag_team_index = OTHER_TEAM(team_index);
		}

		ctf_globals.flags[flag_team_index] = NULL;

		if (flag_index != NONE)
		{
			ctf_globals.flags[flag_team_index] = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);
		}

		if (!ctf_globals.flags[flag_team_index])
		{
			error(_error_silent, "failed to find one of the ctf flags");
		}
	}

	if (game_engine_get_variant()->game_engine_variant.ctf.single_flag_time > 0)
	{
		long flag_to_create = random_range(0, NUMBER_OF_CTF_TEAMS);

		match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 263, (flag_to_create >= 0) && (flag_to_create <= 1));

		create_the_flag(flag_to_create);
		ctf_single_flag_what_is_up_message(flag_to_create);
		ctf_globals.flag_swap_timer = game_engine_get_variant()->game_engine_variant.ctf.single_flag_time;
	}
	else
	{
		for (team_index = 0; team_index < NUMBER_OF_CTF_TEAMS; team_index++)
		{
			create_the_flag(team_index);
		}
	}

	ctf_globals.score_to_win = game_engine_get_variant()->universal_variant.score_to_win;
	starting_location_count = player_get_starting_location_count();

	for (starting_location_index = 0; starting_location_index < starting_location_count; starting_location_index++)
	{
		struct scenario_player *starting_location = player_get_starting_location(starting_location_index);

		if (starting_location->team_index != 0 && starting_location->team_index != 1)
		{
			error(_error_silent, "NETGAME_FLAG_WARNING starting location %d bad team index %d", starting_location_index, starting_location->team_index);
			continue;
		}

		if (match_game_type(_game_engine_ctf, NUMBEROF(starting_location->game_type), starting_location->game_type))
		{
			boolean assault = game_engine_get_variant()->game_engine_variant.ctf.assault > 0;
			boolean too_close = FALSE;
			long own_team_index = starting_location->team_index % NUMBER_OF_CTF_TEAMS;
			long enemy_team_index = OTHER_TEAM(own_team_index);
			real own_flag_distance = distance_squared3d(&ctf_globals.flags[own_team_index]->position, &starting_location->position);
			real enemy_flag_distance = distance_squared3d(&ctf_globals.flags[enemy_team_index]->position, &starting_location->position);

			if (assault)
			{
				too_close = own_flag_distance < enemy_flag_distance;
			}
			else
			{
				too_close = own_flag_distance > enemy_flag_distance;
			}

			if (too_close)
			{
				error(_error_silent, "NETGAME_FLAG_WARNING starting location %d team %d, too close to enemy flag", starting_location_index, starting_location->team_index);
				starting_location->team_index = 3;
			}
		}
	}

	return TRUE;
}

static void ctf_engine_dispose_from_old_map(
	void)
{
	return;
}

static void ctf_engine_player_added(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	return;
}

static void ctf_engine_game_ending(
	void)
{
	return;
}

static void ctf_engine_game_starting(
	void)
{
	game_engine_play_multiplayer_sound(_multiplayer_sound_ctf);

	return;
}

static void ctf_engine_statistics_append(
	struct game_statistics *permanent_statistics,
	struct game_statistics *game_statistics)
{
	return;
}

static void ctf_engine_handle_client_message(
	long player_index,
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void ctf_engine_handle_server_message(
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void ctf_engine_pregame_post_rasterize(
	void)
{
	return;
}

static void ctf_engine_post_rasterize(
	void)
{
	return;
}

static void weapon_reset_flag(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	ctf_set_flag_warning(weapon->object.owner_team_index, FALSE);

	if (ctf_globals.flags[weapon->object.owner_team_index])
	{
		game_engine_flag_reset(weapon_index, &ctf_globals.flags[weapon->object.owner_team_index]->position);
		SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag, FALSE);
	}

	return;
}

static void player_reset_flag(
	long player_index,
	long weapon_index)
{
	struct player_datum *player = player_get(player_index);
	long unit_index = player->unit_index;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 423, NONE != unit_index);

	unit_drop_current_weapon(unit_index, TRUE);
	weapon_reset_flag(weapon_index);

	return;
}

static void player_score(
	long player_index,
	long team_index)
{
	struct player_datum *player = player_get(player_index);

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 436, game_engine_can_score());
	match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 437, NONE != player_index);
	match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 438, NONE != team_index);

	ctf_globals.score[team_index]++;
	player->statistics.multiplayer_statistics.ctf_statistics.flag_scores++;
	game_engine_play_multiplayer_sound(player->team_index ? _multiplayer_sound_ctf_blue_captured_flag : _multiplayer_sound_ctf_red_captured_flag);
	game_show_score_you_ally_enemy(player_index, ctf_message_you_scored, ctf_message_good_guys_scored, ctf_message_enemy_scored, player_index);

	return;
}

static boolean in_scoring_range(
	real_point3d *location,
	real distance,
	long team_index)
{
	boolean result = FALSE;

	if (location && ctf_globals.flags[team_index])
	{
		if (distance_squared3d(location, &ctf_globals.flags[team_index]->position) < distance * distance)
		{
			result = TRUE;
		}
	}

	return result;
}

static void ctf_flag_failure_sound(
	long player_index)
{
	long const flag_failure_sound_delay = 4 * TICKS_PER_SECOND;

	if (game_time_get() > ctf_globals.non_deterministic_next_flag_failure_time &&
		player_index != NONE &&
		player_get(player_index)->local_player_index != NONE)
	{
		game_engine_play_multiplayer_sound(_multiplayer_sound_flag_failure);
		ctf_globals.non_deterministic_next_flag_failure_time = game_time_get() + flag_failure_sound_delay;
	}

	return;
}

static void ctf_engine_player_update(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	long unit_index;
	struct unit_datum *unit;
	long weapon_index;
	struct weapon_datum *weapon;

	if (game_engine_player_has_flag(player_index))
	{
		game_engine_player_depower_active_camo(player_index);
	}

	unit_index = player->unit_index;
	if (unit_index == NONE)
	{
		return;
	}

	unit = unit_get(unit_index);
	if (unit->unit.current_weapon_index == NONE)
	{
		return;
	}

	weapon_index = unit->unit.weapon_object_indices[unit->unit.current_weapon_index];
	if (weapon_index == NONE)
	{
		return;
	}

	weapon = weapon_get(weapon_index);
	if (game_engine_can_score() && weapon_is_flag(weapon_index))
	{
		real scoring_distance = 1.f;

		match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 532, weapon->object.owner_team_index != player->team_index);

		if (in_scoring_range(&unit->object.position, scoring_distance, player->team_index))
		{
			boolean can_score = TRUE;

			if (game_engine_get_variant()->game_engine_variant.ctf.flag_at_home_to_score)
			{
				if (game_engine_get_variant()->game_engine_variant.ctf.single_flag_time)
				{
					can_score = TRUE;
				}
				else
				{
					struct weapon_datum *flag = weapon_get(ctf_globals.weapon_index[player->team_index]);

					can_score = !TEST_FLAG(flag->weapon.flags, _weapon_multiplayer_flag);
				}
			}

			if (can_score)
			{
				player_score(player_index, player->team_index);
				player_reset_flag(player_index, weapon_index);
				game_engine_get_variant();
			}
			else
			{
				ctf_flag_failure_sound(player_index);
			}
		}
	}

	return;
}

static long get_player_with_this_flag(
	long weapon_index)
{
	struct data_iterator player_iterator;
	struct player_datum *player;
	long player_index = NONE;

	data_iterator_new(&player_iterator, player_data);

	while (player = (struct player_datum *)data_iterator_next(&player_iterator))
	{
		if (player->unit_index != NONE && unit_has_weapon(player->unit_index, weapon_index))
		{
			player_index = player_iterator.index;
			break;
		}
	}

	return player_index;
}

static void ctf_engine_weapon_update(
	long weapon_index,
	struct weapon_datum *weapon)
{
	long flag_reset_delay = 17 * TICKS_PER_SECOND;
	long player_index;
	long team_index;
	long other_team_index;
	real_point3d position;
	real vertical_offset;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 609, weapon_is_flag(weapon_index));

	if (game_engine_get_variant()->game_engine_variant.ctf.single_flag_time > 0)
	{
		if (ctf_globals.flag_swap_timer > 0)
		{
			ctf_globals.flag_swap_timer--;
		}

		if (ctf_globals.flag_swap_timer == 0 && !TEST_FLAG(weapon->item.flags, _item_in_unit_inventory_bit))
		{
			game_show_score(NONE, ctf_message_time_expired);
			ctf_set_flag_warning(0, FALSE);
			ctf_set_flag_warning(1, FALSE);
			ctf_globals.weapon_index[weapon->object.owner_team_index] = NONE;
			game_engine_clear_goal_position(0);
			game_engine_clear_goal_position(1);
			weapon->object.owner_team_index = OTHER_TEAM(weapon->object.owner_team_index);
			game_engine_play_multiplayer_sound(weapon->object.owner_team_index ? _multiplayer_sound_red_team_ctf : _multiplayer_sound_blue_team_ctf);
			weapon_reset_flag(weapon_index);
			game_engine_clear_goal_position(2);
			game_engine_clear_goal_position(3);
			ctf_globals.flag_swap_timer = game_engine_get_variant()->game_engine_variant.ctf.single_flag_time;
			ctf_single_flag_what_is_up_message(weapon->object.owner_team_index);
		}
	}

	if ((unsigned long)(game_time_get() - weapon->item.last_owned_time) > flag_reset_delay && weapon_is_flag(weapon_index))
	{
		boolean connected = TEST_FLAG(weapon->object.flags, _object_connected_to_map_bit);
		boolean attached = weapon->object.parent_object_index != NONE;

		if (connected && !attached)
		{
			long flag_team_index = weapon->object.owner_team_index;
			long enemy_team_index = OTHER_TEAM(weapon->object.owner_team_index);

			if (TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag))
			{
				game_engine_play_multiplayer_sound(flag_team_index ? _multiplayer_sound_ctf_blue_returned_flag : _multiplayer_sound_ctf_red_returned_flag);
				ctf_set_flag_warning(weapon->object.owner_team_index, FALSE);
				game_show_score_team(flag_team_index, ctf_message_your_flag_was_returned);
				game_show_score_team(enemy_team_index, ctf_message_enemy_flag_was_returned);
			}

			weapon_reset_flag(weapon_index);
		}
	}

	player_index = get_player_with_this_flag(weapon_index);
	team_index = weapon->object.owner_team_index;
	other_team_index = OTHER_TEAM(team_index);
	vertical_offset = 0.f;
	item_get_position_even_if_in_inventory(weapon_index, &position);
	game_engine_set_goal_position((short)team_index, &position, vertical_offset, "flag_blue", NONE, (short)other_team_index, player_index);

	if (player_index != NONE)
	{
		position = ctf_globals.flags[other_team_index]->position;
		position.y += 0.5f;
		game_engine_set_goal_position((short)(team_index + 2), &position, vertical_offset, "default", player_index, NONE, NONE);
	}
	else
	{
		game_engine_clear_goal_position((short)(team_index + 2));
	}

	return;
}

static void ctf_engine_update(
	void)
{
	boolean game_over = FALSE;
	long team_index;

	for (team_index = 0; team_index < NUMBER_OF_CTF_TEAMS; team_index++)
	{
		if (ctf_globals.score[team_index] >= ctf_globals.score_to_win)
		{
			game_over = TRUE;
		}
	}

	if (game_over)
	{
		game_engine_end_game();
	}

	ctf_state_message_update_warning(0);
	ctf_state_message_update_warning(1);
	ctf_sound_update_warning(0);
	ctf_sound_update_warning(1);

	return;
}

static boolean ctf_engine_allow_pick_up(
	long unit_index,
	long item_index)
{
	boolean allow = TRUE;
	long player_index = player_index_from_unit_index(unit_index);

	if (player_index != NONE && item_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		struct weapon_datum *weapon = weapon_try_and_get(item_index);

		if (weapon &&
			weapon_is_flag(item_index) &&
			!TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag) &&
			weapon->object.owner_team_index == player->team_index)
		{
			allow = FALSE;
		}
	}

	return allow;
}

static void ctf_engine_player_damaged_player(
	long killing_player_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

static void ctf_engine_player_killed_player(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

static boolean ctf_engine_display_score(
	long player_index,
	long message,
	long message_data,
	word *buffer,
	long buffer_size)
{
	long team_index = player_get(player_index)->team_index;
	long other_team_index = OTHER_TEAM(team_index);
	boolean result = TRUE;
	wchar_t *string;

	switch (message)
	{
	case ctf_message_score:
		multiplayer_game_text_get_string(string, _string_red_team_score_blue_team_score);
		usnprintf(buffer, buffer_size, string, ctf_globals.score[0], ctf_globals.score[1]);
		break;
	case ctf_message_you_scored:
		multiplayer_game_text_get_string(string, _string_you_scored_score_to_score);
		usnprintf(buffer, buffer_size, string, ctf_globals.score[team_index], ctf_globals.score[other_team_index]);
		break;
	case ctf_message_enemy_scored:
		multiplayer_game_text_get_string(string, _string_enemy_scored_score_to_score);
		usnprintf(buffer, buffer_size, string, ctf_globals.score[team_index], ctf_globals.score[other_team_index]);
		break;
	case ctf_message_good_guys_scored:
		multiplayer_game_text_get_string(string, _string_your_ally_scored_score_to_score);
		usnprintf(buffer, buffer_size, string, ctf_globals.score[team_index], ctf_globals.score[other_team_index]);
		break;
	case ctf_message_you_have_the_flag:
		usnprintf(buffer, buffer_size, L"");
		break;
	case ctf_message_you_returned_the_flag:
		multiplayer_game_text_get_string(string, _string_you_returned_the_flag);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_enemy_has_the_flag:
	case ctf_message_enemy_has_the_flag_tick:
		multiplayer_game_text_get_string(string, _string_the_enemy_has_your_flag);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_enemy_returned_the_flag:
		multiplayer_game_text_get_string(string, _string_the_enemy_returned_the_flag);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_good_guys_have_the_flag:
		multiplayer_game_text_get_string(string, _string_your_ally_has_the_flag);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_good_guys_returned_the_flag:
		multiplayer_game_text_get_string(string, _string_your_ally_returned_the_flag);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_your_flag_was_returned:
		multiplayer_game_text_get_string(string, _string_your_flag_was_returned);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_enemy_flag_was_returned:
		multiplayer_game_text_get_string(string, _string_the_enemys_flag_was_returned);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_time_expired:
		multiplayer_game_text_get_string(string, _string_time_expired);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_you_are_on_attack:
		multiplayer_game_text_get_string(string, _string_you_are_on_offense);
		ustrncpy(buffer, string, buffer_size);
		break;
	case ctf_message_you_are_on_defense:
		multiplayer_game_text_get_string(string, _string_you_are_on_defense);
		ustrncpy(buffer, string, buffer_size);
		break;
	default:
		result = FALSE;
		break;
	}

	return result;
}

static void ctf_engine_prespawn_player_update(
	long player_index)
{
	return;
}

void ctf_state_message_update_warning(
	long team_index)
{
	return;
}

static void ctf_sound_update_warning(
	long team_index)
{
	if (ctf_globals.flag_warning[team_index])
	{
		long flag_warning_delay = 20 * TICKS_PER_SECOND;

		if (ctf_globals.flag_warning_time_in_ticks[team_index] > flag_warning_delay)
		{
			game_engine_play_multiplayer_sound(team_index ? _multiplayer_sound_ctf_red_took_flag : _multiplayer_sound_ctf_blue_took_flag);
			ctf_globals.flag_warning_time_in_ticks[team_index] = 0;
		}

		ctf_globals.flag_warning_time_in_ticks[team_index]++;
	}

	return;
}

static void ctf_set_flag_warning(
	long team_index,
	boolean warning)
{
	ctf_globals.flag_warning[team_index] = warning;
	ctf_globals.flag_warning_time_in_ticks[team_index] = 0;
	ctf_state_message_update_warning(team_index);

	return;
}

static boolean ctf_weapon_pickup(
	long weapon_index,
	long player_index)
{
	boolean result = TRUE;
	struct weapon_datum *weapon = weapon_get(weapon_index);

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 974, weapon_is_flag(weapon_index));

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (weapon->object.owner_team_index == player->team_index)
		{
			if (!game_engine_get_variant()->game_engine_variant.ctf.flag_must_reset)
			{
				if (TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag) && game_engine_can_score())
				{
					ctf_set_flag_warning(weapon->object.owner_team_index, FALSE);
					player->statistics.multiplayer_statistics.ctf_statistics.flag_returns++;
					game_show_score_you_ally_enemy(player_index, ctf_message_you_returned_the_flag, ctf_message_good_guys_returned_the_flag, ctf_message_enemy_returned_the_flag, player_index);
					game_engine_play_multiplayer_sound(player->team_index ? _multiplayer_sound_ctf_blue_returned_flag : _multiplayer_sound_ctf_red_returned_flag);
				}

				weapon_reset_flag(weapon_index);
			}
			else if (TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag))
			{
				ctf_flag_failure_sound(player_index);
			}

			result = FALSE;
		}
		else
		{
			if (TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag))
			{
			}
			else if (game_engine_can_score())
			{
				player->statistics.multiplayer_statistics.ctf_statistics.flag_grabs++;

				if (game_engine_get_variant()->game_engine_variant.ctf.assault)
				{
				}
				else
				{
					game_engine_play_multiplayer_sound(player->team_index ? _multiplayer_sound_ctf_blue_took_flag : _multiplayer_sound_ctf_red_took_flag);
					ctf_set_flag_warning(weapon->object.owner_team_index, TRUE);
					game_show_score_you_ally_enemy(player_index, NONE, ctf_message_good_guys_have_the_flag, ctf_message_enemy_has_the_flag, player_index);
				}
			}

			SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_flag, TRUE);
		}
	}

	return result;
}

static void ctf_weapon_drop(
	long weapon_index)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_engine_ctf.c", 1050, weapon_is_flag(weapon_index));

	return;
}

static long ctf_get_score(
	long player_index,
	enum get_score_type get_score_type)
{
	struct player_datum *player = player_get(player_index);
	long score;

	if (get_score_type == _get_score_individual)
	{
		score = player->statistics.multiplayer_statistics.ctf_statistics.flag_scores;
	}
	else
	{
		score = ctf_globals.score[player->team_index];
	}

	return score;
}

static boolean ctf_test_flag(
	long flag)
{
	boolean result = FALSE;

	switch (flag)
	{
	case _game_engine_spawn_point_obeys_teams_bit:
		result = TRUE;
		break;
	}

	return result;
}

static word *ctf_get_score_string(
	long player_index,
	word *buffer)
{
	struct player_datum *player = player_get(player_index);

	usprintf(buffer, L"%d", player->statistics.multiplayer_statistics.ctf_statistics.flag_scores);

	return buffer;
}

static word *ctf_get_score_header_string(
	word *buffer)
{
	wchar_t *string;

	multiplayer_game_text_get_string(string, _string_score);
	ustrcpy(buffer, string);

	return buffer;
}

static word *ctf_get_team_score_string(
	long team_index,
	word *buffer)
{
	usprintf(buffer, L"%d", ctf_globals.score[team_index]);

	return buffer;
}

static real ctf_engine_starting_location_rating(
	long player_index,
	struct scenario_player *starting_location)
{
	real rating = 1.f;

	if (game_engine_get_variant()->game_engine_variant.ctf.assault)
	{
		struct player_datum *player = player_get(player_index);
		long team_index = player->team_index;
		long other_team_index = OTHER_TEAM(team_index);
		real distance = distance_squared3d(&starting_location->position, &ctf_globals.flags[other_team_index]->position);

		distance = PIN(distance, 0.5f, 10.f);
		rating = 1.f / distance;

		if (game_time_get() > TICKS_PER_SECOND)
		{
			if (distance > 1.f)
			{
				rating = (real)pow(rating, 0.33f);
			}

			rating = PIN(rating, 0.5f, 2.f);
		}
	}

	return rating;
}
