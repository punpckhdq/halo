/*
GAME_ENGINE_RACE.C

symbols in this file:
000A27A0 0010:
	_race_engine_dispose (0000)
000A27B0 0080:
	_delete_all_vehicles (0000)
000A2830 00f0:
	_race_get_vehicle_to_spawn (0000)
000A2920 0090:
	_race_flags_make_unique (0000)
000A29B0 0010:
	_race_engine_dispose_from_old_map (0000)
000A29C0 0030:
	_race_engine_player_added (0000)
000A29F0 0010:
	_race_engine_game_ending (0000)
000A2A00 0010:
	_race_engine_game_starting (0000)
000A2A10 0010:
	_race_engine_statistics_append (0000)
000A2A20 0010:
	_race_engine_handle_client_message (0000)
000A2A30 0010:
	_race_engine_handle_server_message (0000)
000A2A40 0010:
	_race_engine_pregame_post_rasterize (0000)
000A2A50 0010:
	_race_engine_post_rasterize (0000)
000A2A60 0190:
	_race_complete_lap (0000)
000A2BF0 0120:
	_can_touch_team (0000)
000A2D10 0010:
	_race_engine_weapon_update (0000)
000A2D20 0090:
	_race_team_can_win_game (0000)
000A2DB0 00e0:
	_build_player_speeds (0000)
000A2E90 0010:
	_race_engine_player_damaged_player (0000)
000A2EA0 0010:
	_race_engine_player_killed_player (0000)
000A2EB0 0480:
	_race_engine_display_score (0000)
000A3330 0010:
	_race_engine_prespawn_player_update (0000)
000A3340 0060:
	_race_goal_matches_player (0000)
000A33A0 0020:
	_count_bits_32 (0000)
000A33C0 00e0:
	_race_engine_get_score (0000)
000A34A0 0040:
	_race_get_score_string (0000)
000A34E0 0070:
	_race_get_score_header_string (0000)
000A3550 0030:
	_race_get_team_score_string (0000)
000A3580 00b0:
	_race_engine_did_player_win (0000)
000A3630 00e0:
	_find_closest_vehicle (0000)
000A3710 0130:
	_create_race_vehicles (0000)
000A3840 00f0:
	_new_rally_flag (0000)
000A3930 0150:
	_race_touch_flag (0000)
000A3A80 00a0:
	_race_engine_player_update (0000)
000A3B20 00c0:
	_race_engine_update (0000)
000A3BE0 0110:
	_race_engine_initialize_for_new_map (0000)
0025BFD4 0027:
	??_C@_0CH@MLHAJCND@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BFFC 0019:
	??_C@_0BJ@BIAKIHCM@itr?5?$DM?5MAXIMUM_RACE_FLAGS?$AA@ (0000)
0025C018 0036:
	??_C@_0DG@CFFMMMPN@?$CB?$CCrace?5goal?5matches?5player?5calle@ (0000)
0025C050 0011:
	??_C@_0BB@BCKDFNEI@?$CBcan_team_win?$FL1?$FN?$AA@ (0000)
0025C064 0011:
	??_C@_0BB@CAKPLNGL@new_flag?5?$CB?$DN?5NONE?$AA@ (0000)
0025C078 000a:
	??_C@_09DMMLFBJB@count?5?$DO?50?$AA@ (0000)
0025C084 0037:
	??_C@_0DH@EODBECII@?$CB?$CI?$CKlap_bit_vector?5?$CG?5?$HOrace_global@ (0000)
0025C0BC 0028:
	??_C@_0CI@ONDKDHNM@?$CBTEST_FLAG?$CI?$CKlap_bit_vector?0?5team@ (0000)
0025C0E8 0053:
	??_C@_0FD@CPKHPGHJ@_race_type_normal?5?$CB?$DN?5game_engine@ (0000)
0025C140 0050:
	??_C@_0FA@IHEEDPIP@one?5of?5the?5netgameflags?5that?5def@ (0000)
002DE5E8 0088:
	_race_engine (0000)
0043ECB0 00d0:
	_bss_0043ecb0 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "game_engine_list.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "network_messages.h"
#include "text_group.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "font_group.h"
#include "network_server_message_handler.h"
#include "scenery.h"
#include "game_engine.h"
#include "game_globals.h"
#include "game.h"
#include "unicode.h"
#include "tag_groups.h"
#include "units.h"
#include "objects.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "real_math.h"

/* ---------- constants */

enum
{
	MAXIMUM_RACE_FLAGS = 32,
	MAXIMUM_SPAWNED_RACE_VEHICLES = 8, /* fake name */
	MAXIMUM_DELETED_RACE_VEHICLES = 32 /* fake name */
};

enum
{
	race_message_you_completed_a_lap = 30,
	race_message_ally_completed_a_lap,
	race_message_enemy_completed_a_lap,
	race_message_you_touched_a_flag_rally,
	race_message_ally_touched_a_flag_rally,
	race_message_enemy_touched_a_flag_rally,
	race_message_new_best_lap_time
};

/* ---------- macros */

/* ---------- structures */

struct race_globals_type
{
	unsigned long lap_completed_value;
	long first_flag[MULTIPLAYER_MAXIMUM_PLAYERS];
	unsigned long lap_bit_vector[MULTIPLAYER_MAXIMUM_PLAYERS];
	long rally_flag;
	long team_laps[MULTIPLAYER_MAXIMUM_PLAYERS];
	long vehicle_sound_timeout;
	boolean vehicles_have_been_added;
};

/* ---------- prototypes */

static void race_engine_dispose(void);
static void delete_all_vehicles(void);
static long find_closest_vehicle(real_point3d const *point, long *ignore_these, long ignore_these_count);
static long race_get_vehicle_to_spawn(long vehicle_number);
static void create_race_vehicles(void);
void race_flags_make_unique(void);
static boolean race_engine_initialize_for_new_map(void);
static void race_engine_dispose_from_old_map(void);
static void race_engine_player_added(long player_index);
static void race_engine_game_ending(void);
static void race_engine_game_starting(void);
static void race_engine_statistics_append(struct game_statistics *permanent_statistics, struct game_statistics *game_statistics);
static void race_engine_handle_client_message(long player_index, void *encoded_message, short encoded_message_size);
static void race_engine_handle_server_message(void *encoded_message, short encoded_message_size);
static void race_engine_pregame_post_rasterize(void);
static void race_engine_post_rasterize(void);
static void race_complete_lap(long player_index);
static boolean can_touch_team(long player_index, long team_index);
static long new_rally_flag(long old_flag);
static void race_touch_flag(long player_index, long netgame_flag_index);
static void race_engine_player_update(long player_index);
static void race_engine_weapon_update(long weapon_index, struct weapon_datum *weapon);
static boolean race_team_can_win_game(long team_index);
static void build_player_speeds(void);
static void race_engine_update(void);
static void race_engine_player_damaged_player(long killing_player_index, long dead_player_index, boolean friendly_fire);
static void race_engine_player_killed_player(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);
static boolean race_engine_display_score(long player_index, long message, long message_data, word *buffer, long buffer_size);
static void race_engine_prespawn_player_update(long player_index);
static boolean race_goal_matches_player(long player_index, long goal_index);
static long count_bits_32(unsigned long value);
static long race_engine_get_score(long player_index, enum get_score_type get_score_type);
static word *race_get_score_string(long player_index, word *buffer);
static word *race_get_score_header_string(word *buffer);
static word *race_get_team_score_string(long team_index, word *buffer);
static long race_engine_did_player_win(long player_index);

/* ---------- globals */

struct game_engine race_engine =
{
	"race",
	_game_engine_race,
	race_engine_dispose,
	race_engine_initialize_for_new_map,
	race_engine_dispose_from_old_map,
	race_engine_player_added,
	race_engine_game_ending,
	race_engine_game_starting,
	race_engine_statistics_append,
	race_engine_handle_client_message,
	race_engine_handle_server_message,
	race_engine_pregame_post_rasterize,
	race_engine_post_rasterize,
	race_engine_player_update,
	race_engine_weapon_update,
	NULL, // weapon_pickup
	NULL, // weapon_drop
	race_engine_update,
	race_engine_get_score,
	race_get_score_string,
	race_get_score_header_string,
	race_get_team_score_string,
	NULL, // allow_pick_up
	race_engine_player_damaged_player,
	race_engine_player_killed_player,
	race_engine_display_score,
	NULL, // starting_location_rating
	race_engine_prespawn_player_update,
	NULL, // postspawn_player_update
	NULL, // game_engine_player_get_team_index
	race_goal_matches_player,
	NULL, // game_engine_test_flag
	NULL, // game_engine_test_trait
	race_engine_did_player_win,
};

static struct race_globals_type race_globals;

/* ---------- private code */

static void race_engine_dispose(
	void)
{
	return;
}

static void delete_all_vehicles(
	void)
{
	struct object_iterator iterator;
	long vehicle_indices[MAXIMUM_DELETED_RACE_VEHICLES];
	long vehicle_count = 0;
	long vehicle_index;

	object_iterator_new(&iterator, _object_mask_vehicle, 0);
	while (object_iterator_next(&iterator))
	{
		if (vehicle_count < MAXIMUM_DELETED_RACE_VEHICLES)
		{
			vehicle_indices[vehicle_count++] = iterator.index;
		}
	}

	for (vehicle_index = 0; vehicle_index < vehicle_count; vehicle_index++)
	{
		object_delete(vehicle_indices[vehicle_index]);
	}

	return;
}

static long find_closest_vehicle(
	real_point3d const *point,
	long *ignore_these,
	long ignore_these_count)
{
	real closest_distance = 1000000.f;
	long closest_index = NONE;
	long ignore_index;
	struct scenario *scenario = global_scenario_get();
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *multiplayer_information = TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information);
	long flag_index;

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		if (flag->type == _netgame_flag_race_vehicle)
		{
			boolean ignore = FALSE;
			real distance;

			for (ignore_index = 0; ignore_index < ignore_these_count; ignore_index++)
			{
				if (flag_index == ignore_these[ignore_index])
				{
					ignore = TRUE;
					break;
				}
			}

			if (ignore)
			{
				continue;
			}

			if (!point)
			{
				return flag_index;
			}

			distance = distance_squared3d(point, &flag->position);
			if (distance < closest_distance)
			{
				closest_distance = distance;
				closest_index = flag_index;
			}
		}
	}

	return closest_index;
}

static long race_get_vehicle_to_spawn(
	long vehicle_number)
{
	struct scenario *scenario = global_scenario_get();
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *multiplayer_information = TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information);
	struct tag_reference *warthog = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_warthog, struct tag_reference);
	struct tag_reference *ghost = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_ghost, struct tag_reference);
	struct tag_reference *scorpion = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_scorpion, struct tag_reference);
	long result = NONE;

	switch (game_engine_get_variant()->universal_variant.vehicle_set)
	{
	case _game_engine_vehicles_default:
		if (vehicle_number == 0)
		{
			result = warthog->index;
		}
		else if (vehicle_number == 1)
		{
			result = scorpion->index;
		}
		else if (vehicle_number < 6)
		{
			result = ghost->index;
		}
		break;
	case _game_engine_vehicles_warthog:
		result = vehicle_number < 4 ? warthog->index : NONE;
		break;
	case _game_engine_vehicles_ghost:
		result = vehicle_number < 8 ? ghost->index : NONE;
		break;
	case _game_engine_vehicles_tank:
		result = vehicle_number < 4 ? scorpion->index : NONE;
		break;
	}

	return result;
}

static void create_race_vehicles(
	void)
{
	struct scenario *scenario = global_scenario_get();
	long spawned_vehicles[MAXIMUM_SPAWNED_RACE_VEHICLES] = { NONE };
	struct data_iterator iterator;
	struct player_datum *player;
	long vehicle_count = 0;
	long vehicle_index;

	data_iterator_new(&iterator, player_data);
	while (player = (struct player_datum *)data_iterator_next(&iterator))
	{
		struct unit_datum *unit = player->unit_index != NONE ? (struct unit_datum *)object_get_and_verify_type(player->unit_index, _object_mask_unit) : NULL;
		long flag_index;

		if (vehicle_count == NUMBEROF(spawned_vehicles))
		{
			break;
		}

		if (unit)
		{
			flag_index = find_closest_vehicle(&unit->object.position, spawned_vehicles, vehicle_count);
		}
		else
		{
			flag_index = find_closest_vehicle(NULL, spawned_vehicles, vehicle_count);
		}

		if (flag_index == NONE)
		{
			break;
		}

		spawned_vehicles[vehicle_count++] = flag_index;
	}

	for (vehicle_index = 0; vehicle_index < vehicle_count; vehicle_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, spawned_vehicles[vehicle_index], struct scenario_netgame_flag);
		long definition_index = race_get_vehicle_to_spawn(vehicle_index);
		struct object_placement_data placement_data;

		if (definition_index != NONE)
		{
			object_placement_data_new(&placement_data, definition_index, NONE);
			placement_data.position = flag->position;
			vector3d_from_angle(&placement_data.forward, flag->facing);
			object_new(&placement_data);
			race_globals.vehicles_have_been_added = TRUE;
		}
	}

	return;
}

void race_flags_make_unique(
	void)
{
	struct scenario *scenario = global_scenario_get();
	unsigned long used_flags = 0;
	short flag_index;

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		if (flag->type == _netgame_flag_race_track && flag->team_index >= 0 && flag->team_index < MAXIMUM_RACE_FLAGS)
		{
			if (TEST_FLAG(used_flags, flag->team_index))
			{
				long itr;

				for (itr = 0; itr < MAXIMUM_RACE_FLAGS; itr++)
				{
					if (!TEST_FLAG(used_flags, itr))
					{
						SET_FLAG(used_flags, flag->team_index, TRUE);
						break;
					}
				}

				flag->team_index = (short)itr;
			}
			else
			{
				SET_FLAG(used_flags, flag->team_index, TRUE);
			}
		}
	}

	return;
}

static boolean race_engine_initialize_for_new_map(
	void)
{
	long first_flag = MAXIMUM_RACE_FLAGS;
	struct scenario *scenario = global_scenario_get();
	long flag_index;

	race_flags_make_unique();
	race_globals.vehicles_have_been_added = FALSE;
	memset(&race_globals, 0, sizeof(race_globals));
	timeout_for_endgame_sound = TICKS_PER_SECOND;

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		if (flag->type == _netgame_flag_race_track)
		{
			if (flag->team_index >= MAXIMUM_RACE_FLAGS)
			{
				error(_error_silent, "one of the netgameflags that defines the track was out of the legal range 0..%d", MAXIMUM_RACE_FLAGS);
			}
			else
			{
				real vertical_offset = 0.f;

				if (first_flag > flag->team_index)
				{
					first_flag = flag->team_index;
				}

				SET_FLAG(race_globals.lap_completed_value, flag->team_index, TRUE);
				game_engine_set_goal_position(flag->team_index, &flag->position, vertical_offset, "flag_blue", NONE, NONE, NONE);
			}
		}
	}

	if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_flag_rally)
	{
		race_globals.rally_flag = new_rally_flag(NONE);
	}
	else
	{
		long player_index;

		if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_normal)
		{
			for (player_index = 0; player_index < MULTIPLAYER_MAXIMUM_PLAYERS; player_index++)
			{
				race_globals.first_flag[player_index] = first_flag;
			}
		}
		else
		{
			for (player_index = 0; player_index < MULTIPLAYER_MAXIMUM_PLAYERS; player_index++)
			{
				race_globals.first_flag[player_index] = NONE;
			}
		}
	}

	return TRUE;
}

static void race_engine_dispose_from_old_map(
	void)
{
	return;
}

static void race_engine_player_added(
	long player_index)
{
	player_get(player_index)->multiplayer.special = 0;

	return;
}

static void race_engine_game_ending(
	void)
{
	return;
}

static void race_engine_game_starting(
	void)
{
	return;
}

static void race_engine_statistics_append(
	struct game_statistics *permanent_statistics,
	struct game_statistics *game_statistics)
{
	return;
}

static void race_engine_handle_client_message(
	long player_index,
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void race_engine_handle_server_message(
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void race_engine_pregame_post_rasterize(
	void)
{
	return;
}

static void race_engine_post_rasterize(
	void)
{
	return;
}

static void race_complete_lap(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	long absolute_player_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
	long lap_time = game_time_get() - player->multiplayer.special;
	struct data_iterator iterator;
	struct player_datum *other_player;
	long team_laps;

	race_globals.lap_bit_vector[absolute_player_index] = 0;
	game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_timer_end);
	player->statistics.multiplayer_statistics.race_statistics.last_lap_time = (short)lap_time;

	if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_flag_rally)
	{
		game_show_score_you_ally_enemy(player_index, race_message_you_touched_a_flag_rally, race_message_ally_touched_a_flag_rally, race_message_enemy_completed_a_lap, player_index);
	}
	else
	{
		game_show_score_you_ally_enemy(player_index, race_message_you_completed_a_lap, race_message_ally_completed_a_lap, race_message_enemy_completed_a_lap, player_index);
	}

	if (player->statistics.multiplayer_statistics.race_statistics.laps == 0)
	{
		player->statistics.multiplayer_statistics.race_statistics.best_lap_time = (short)lap_time;
	}
	else if (lap_time < player->statistics.multiplayer_statistics.race_statistics.best_lap_time)
	{
		player->statistics.multiplayer_statistics.race_statistics.best_lap_time = (short)lap_time;
		if (game_engine_get_variant()->game_engine_variant.race.race_type != _race_type_flag_rally)
		{
			game_show_score_extended(player_index, race_message_new_best_lap_time, player_index);
		}
	}

	player->statistics.multiplayer_statistics.race_statistics.laps++;
	player->multiplayer.special = game_time_get();
	team_laps = player->statistics.multiplayer_statistics.race_statistics.laps;

	data_iterator_new(&iterator, player_data);
	if (game_engine_get_variant()->game_engine_variant.race.team_scoring == _race_team_scoring_accumulate)
	{
		team_laps = 0;
	}

	while (other_player = (struct player_datum *)data_iterator_next(&iterator))
	{
		if (player->team_index == other_player->team_index)
		{
			long laps = other_player->statistics.multiplayer_statistics.race_statistics.laps;

			switch (game_engine_get_variant()->game_engine_variant.race.team_scoring)
			{
			case _race_team_scoring_min:
				team_laps = MIN(team_laps, laps);
				break;
			case _race_team_scoring_max:
				team_laps = MAX(team_laps, laps);
				break;
			case _race_team_scoring_accumulate:
				team_laps += laps;
				break;
			default:
				match_unreachable("c:\\halo\\SOURCE\\game\\game_engine_race.c", 584);
				break;
			}
		}
	}

	if (team_laps > race_globals.team_laps[player->team_index])
	{
		race_globals.team_laps[player->team_index] = team_laps;
	}

	if (race_globals.team_laps[player->team_index] >= game_engine_get_variant()->universal_variant.score_to_win)
	{
		game_engine_end_game();
	}

	return;
}

static boolean can_touch_team(
	long player_index,
	long team_index)
{
	struct player_datum *player = player_get(player_index);
	long absolute_player_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
	boolean result = TRUE;
	unsigned long remaining_flags = ~race_globals.lap_bit_vector[absolute_player_index] & race_globals.lap_completed_value;

	if (team_index >= MAXIMUM_RACE_FLAGS)
	{
		result = FALSE;
	}
	else if (player->statistics.multiplayer_statistics.race_statistics.laps >= game_engine_get_variant()->universal_variant.score_to_win)
	{
		result = FALSE;
	}
	else if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_flag_rally)
	{
		result = race_globals.rally_flag == team_index;
	}
	else if (race_globals.lap_bit_vector[absolute_player_index] == race_globals.lap_completed_value)
	{
		result = team_index == race_globals.first_flag[absolute_player_index];
	}
	else if (!TEST_FLAG(remaining_flags, team_index))
	{
		result = FALSE;
	}
	else if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_normal)
	{
		long itr;

		for (itr = 0; itr < MAXIMUM_RACE_FLAGS; itr++)
		{
			if (itr == team_index)
			{
				result = TRUE;
				break;
			}

			if (TEST_FLAG(remaining_flags, itr))
			{
				result = FALSE;
				break;
			}
		}

		match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 650, itr < MAXIMUM_RACE_FLAGS);
	}
	else
	{
		result = TRUE;
	}

	return result;
}

static long new_rally_flag(
	long old_flag)
{
	long new_flag = NONE;
	struct scenario *scenario = global_scenario_get();
	long count = count_bits_32(race_globals.lap_completed_value);
	long random_index;
	long flag_index;

	if (old_flag != NONE)
	{
		count--;
	}

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 680, count > 0);
	random_index = random_range(0, (short)count);

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		if (flag->type == _netgame_flag_race_track && flag->team_index != old_flag)
		{
			if (random_index == 0)
			{
				new_flag = flag->team_index;
				break;
			}

			random_index--;
		}
	}

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 701, new_flag != NONE);

	return new_flag;
}

static void race_touch_flag(
	long player_index,
	long netgame_flag_index)
{
	struct scenario *scenario = global_scenario_get();
	struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, netgame_flag_index, struct scenario_netgame_flag);
	long team_index = flag->team_index;

	if (can_touch_team(player_index, flag->team_index))
	{
		long absolute_player_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
		unsigned long *lap_bit_vector = &race_globals.lap_bit_vector[absolute_player_index];

		game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_timer);

		if (race_globals.first_flag[absolute_player_index] == NONE)
		{
			match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 724, _race_type_normal != game_engine_get_variant()->game_engine_variant.race.race_type);
			race_globals.first_flag[absolute_player_index] = flag->team_index;
		}

		if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_flag_rally)
		{
			race_complete_lap(player_index);
			race_globals.rally_flag = new_rally_flag(race_globals.rally_flag);
		}
		else if (*lap_bit_vector == race_globals.lap_completed_value)
		{
			race_complete_lap(player_index);
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 746, !TEST_FLAG(*lap_bit_vector, team_index));
			SET_FLAG(*lap_bit_vector, team_index, TRUE);
			match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 750, !(*lap_bit_vector & ~race_globals.lap_completed_value));
		}
	}

	return;
}

static void race_engine_player_update(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	game_engine_state_message(player_index, game_engine_message_show_score, player_index);

	if (player->unit_index != NONE && game_engine_can_score())
	{
		struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(player->unit_index, _object_mask_unit);
		long flag_index;

		if (unit->object.parent_object_index != NONE)
		{
			struct unit_datum *vehicle = (struct unit_datum *)object_get_and_verify_type(unit->object.parent_object_index, _object_mask_unit);
			real distance = 2.5f;

			flag_index = find_netgame_flag(&vehicle->object.bounding_sphere_center, distance, 0.f, _netgame_flag_race_track, NONE);
		}
		else
		{
			real distance = 1.5f;
			real height_delta = 0.6f;

			flag_index = find_netgame_flag(&unit->object.bounding_sphere_center, distance, height_delta, _netgame_flag_race_track, NONE);
		}

		if (flag_index != NONE)
		{
			race_touch_flag(player_index, flag_index);
		}
	}

	return;
}

static void race_engine_weapon_update(
	long weapon_index,
	struct weapon_datum *weapon)
{
	return;
}

static boolean race_team_can_win_game(
	long team_index)
{
	boolean can_win = TRUE;

	if (game_engine_get_variant()->game_engine_variant.race.team_scoring == _race_team_scoring_min)
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);
		while (player = (struct player_datum *)data_iterator_next(&iterator))
		{
			if (player->team_index == team_index &&
				player->statistics.multiplayer_statistics.race_statistics.laps < game_engine_get_variant()->universal_variant.score_to_win &&
				(game_engine_player_is_out_of_lives(iterator.index) || player->quit_out_of_game))
			{
				can_win = FALSE;
			}
		}
	}

	return can_win;
}

static void build_player_speeds(
	void)
{
	struct data_iterator player_iterator;
	struct player_datum *player;
	long maximum_laps = 0;

	data_iterator_new(&player_iterator, player_data);
	while (player = (struct player_datum *)data_iterator_next(&player_iterator))
	{
		maximum_laps = MAX(maximum_laps, player->statistics.multiplayer_statistics.race_statistics.laps);
	}

	data_iterator_new(&player_iterator, player_data);
	while (player = (struct player_datum *)data_iterator_next(&player_iterator))
	{
		real speed_multiplier = 1.f;
		long laps_behind = maximum_laps - player->statistics.multiplayer_statistics.race_statistics.laps;

		if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_flag_rally)
		{
			laps_behind /= 3;
		}

		if (laps_behind >= 2)
		{
			speed_multiplier = 1.2f;
		}
		else if (laps_behind >= 1)
		{
			speed_multiplier = 1.1f;
		}

		player->multiplayer.speed_multiplier = speed_multiplier;
	}

	return;
}

static void race_engine_update(
	void)
{
	if (game_time_get() == 2)
	{
		delete_all_vehicles();
		create_race_vehicles();
		game_engine_play_multiplayer_sound(game_engine_has_teams() ? _multiplayer_sound_team_race : _multiplayer_sound_race);

		if (race_globals.vehicles_have_been_added)
		{
			switch (game_engine_get_variant()->universal_variant.vehicle_set)
			{
			case _game_engine_vehicles_default:
				game_engine_play_multiplayer_sound(_multiplayer_sound_warthog);
				game_engine_play_multiplayer_sound(_multiplayer_sound_ghost);
				game_engine_play_multiplayer_sound(_multiplayer_sound_scorpion);
				break;
			case _game_engine_vehicles_warthog:
				game_engine_play_multiplayer_sound(_multiplayer_sound_warthog);
				break;
			case _game_engine_vehicles_ghost:
				game_engine_play_multiplayer_sound(_multiplayer_sound_ghost);
				break;
			case _game_engine_vehicles_tank:
				game_engine_play_multiplayer_sound(_multiplayer_sound_scorpion);
				break;
			}
		}
	}

	if (game_engine_has_teams())
	{
		if (!race_team_can_win_game(0))
		{
			game_engine_end_game();
		}

		if (!race_team_can_win_game(1))
		{
			game_engine_end_game();
		}
	}

	build_player_speeds();

	return;
}

static void race_engine_player_damaged_player(
	long killing_player_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

static void race_engine_player_killed_player(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

static boolean race_engine_display_score(
	long player_index,
	long message,
	long message_data,
	word *buffer,
	long buffer_size)
{
	struct player_datum *message_player = NULL;
	boolean result = TRUE;
	wchar_t *string;

	switch (message)
	{
	case game_engine_message_show_score:
	case race_message_you_completed_a_lap:
	case race_message_ally_completed_a_lap:
	case race_message_enemy_completed_a_lap:
	case race_message_ally_touched_a_flag_rally:
	case race_message_enemy_touched_a_flag_rally:
	case race_message_new_best_lap_time:
		message_player = player_get(message_data);
		break;
	}

	switch (message)
	{
	case race_message_you_touched_a_flag_rally:
		multiplayer_game_text_get_string(string, _string_you_scored_a_flag);
		ustrncpy(buffer, string, buffer_size);
		break;
	case race_message_ally_touched_a_flag_rally:
		multiplayer_game_text_get_string(string, _string_ally_name_scored_a_flag);
		usnprintf(buffer, buffer_size, string, message_player->name);
		break;
	case race_message_enemy_touched_a_flag_rally:
		multiplayer_game_text_get_string(string, _string_enemy_name_scored_a_flag);
		usnprintf(buffer, buffer_size, string, message_player->name);
		break;
	case race_message_you_completed_a_lap:
	{
		real seconds = (real)player_get(message_data)->statistics.multiplayer_statistics.race_statistics.last_lap_time * (1.f / TICKS_PER_SECOND);

		multiplayer_game_text_get_string(string, _string_you_completed_lap_n_in_x_seconds);
		usnprintf(buffer, buffer_size, string, message_player->statistics.multiplayer_statistics.race_statistics.laps + 1, seconds);
		break;
	}
	case race_message_ally_completed_a_lap:
		multiplayer_game_text_get_string(string, _string_ally_name_completed_a_lap_n);
		usnprintf(buffer, buffer_size, string, message_player->name, message_player->statistics.multiplayer_statistics.race_statistics.laps + 1);
		break;
	case race_message_enemy_completed_a_lap:
		multiplayer_game_text_get_string(string, _string_enemy_name_completed_a_lap);
		usnprintf(buffer, buffer_size, string, message_player->name, message_player->statistics.multiplayer_statistics.race_statistics.laps);
		break;
	case race_message_new_best_lap_time:
	{
		real seconds = (real)player_get(message_data)->statistics.multiplayer_statistics.race_statistics.best_lap_time * (1.f / TICKS_PER_SECOND);

		multiplayer_game_text_get_string(string, _string_new_best_lap_time_x);
		usnprintf(buffer, buffer_size, string, seconds);
		break;
	}
	case game_engine_message_show_score:
		if (game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_flag_rally)
		{
			if (message_player->statistics.multiplayer_statistics.race_statistics.laps == 1)
			{
				multiplayer_game_text_get_string(string, _string_name_1_flag);
				usnprintf(buffer, buffer_size, string, get_place_name(game_engine_get_place(player_index, _get_score_team)));
			}
			else
			{
				multiplayer_game_text_get_string(string, _string_name_n_flags);
				usnprintf(buffer, buffer_size, string, get_place_name(game_engine_get_place(player_index, _get_score_team)), message_player->statistics.multiplayer_statistics.race_statistics.laps);
			}
		}
		else
		{
			if (message_player->statistics.multiplayer_statistics.race_statistics.laps + 1 > game_engine_get_variant()->universal_variant.score_to_win)
			{
				multiplayer_game_text_get_string(string, _string_name_all_laps_complete);
				usnprintf(buffer, buffer_size, string, get_place_name(game_engine_get_place(player_index, _get_score_team)));
			}
			else
			{
				multiplayer_game_text_get_string(string, _string_name_lap_n_of_total);
				usnprintf(buffer, buffer_size, string, get_place_name(game_engine_get_place(player_index, _get_score_team)), message_player->statistics.multiplayer_statistics.race_statistics.laps + 1, game_engine_get_variant()->universal_variant.score_to_win);
			}
		}
		break;
	default:
		result = FALSE;
		break;
	}

	return result;
}

static void race_engine_prespawn_player_update(
	long player_index)
{
	return;
}

static boolean race_goal_matches_player(
	long player_index,
	long goal_index)
{
	struct player_datum *player = player_get(player_index);
	boolean result = FALSE;

	if (!TEST_FLAG(race_globals.lap_completed_value, goal_index))
	{
		match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 1086, !"race goal matches player called on an invalid goal");
	}
	else
	{
		result = can_touch_team(player_index, goal_index);
	}

	return result;
}

static long count_bits_32(
	unsigned long value)
{
	long count = 0;
	long bit;

	for (bit = 0; bit < 32; bit++)
	{
		if (TEST_FLAG(value, bit))
		{
			count++;
		}
	}

	return count;
}

static long race_engine_get_score(
	long player_index,
	enum get_score_type get_score_type)
{
	struct player_datum *player = player_get(player_index);
	long score;

	if (get_score_type == _get_score_team)
	{
		score = race_globals.team_laps[player->team_index];
	}
	else
	{
		long team_index = player->team_index;
		long laps = player->statistics.multiplayer_statistics.race_statistics.laps;

		score = laps * (MAXIMUM_RACE_FLAGS + 1) + count_bits_32(race_globals.lap_bit_vector[team_index]);
	}

	return score;
}

static word *race_get_score_string(
	long player_index,
	word *buffer)
{
	usprintf(buffer, L"%d", player_get(player_index)->statistics.multiplayer_statistics.race_statistics.laps);

	return buffer;
}

static word *race_get_score_header_string(
	word *buffer)
{
	long string_index = game_engine_get_variant()->game_engine_variant.race.race_type == _race_type_flag_rally ? _string_flags : _string_laps;
	wchar_t *string;

	multiplayer_game_text_get_string(string, string_index);
	ustrcpy(buffer, string);

	return buffer;
}

static word *race_get_team_score_string(
	long team_index,
	word *buffer)
{
	usprintf(buffer, L"%d", race_globals.team_laps[team_index]);

	return buffer;
}

static long race_engine_did_player_win(
	long player_index)
{
	long result;

	if (game_engine_has_teams())
	{
		struct player_datum *player = player_get(player_index);
		boolean can_team_win[2];

		can_team_win[0] = race_team_can_win_game(0);
		can_team_win[1] = race_team_can_win_game(1);

		if (can_team_win[0] != can_team_win[1])
		{
			result = can_team_win[player->team_index] ? TRUE : FALSE;
		}
		else if (!can_team_win[0])
		{
			match_assert("c:\\halo\\SOURCE\\game\\game_engine_race.c", 1196, !can_team_win[1]);
			result = NONE;
		}
		else
		{
			result = game_engine_did_player_win_default(player_index);
		}
	}
	else
	{
		result = game_engine_did_player_win_default(player_index);
	}

	return result;
}
