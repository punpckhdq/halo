/*
GAME_ENGINE_SLAYER.C

symbols in this file:
000A3CF0 0050:
	_target_is_valid (0000)
000A3D40 0010:
	_slayer_engine_dispose (0000)
000A3D50 0030:
	_slayer_engine_initialize_for_new_map (0000)
000A3D80 0010:
	_slayer_engine_dispose_from_old_map (0000)
000A3D90 0030:
	_slayer_engine_player_added (0000)
000A3DC0 0010:
	_slayer_engine_game_ending (0000)
000A3DD0 0020:
	_slayer_engine_game_starting (0000)
000A3DF0 0010:
	_slayer_engine_statistics_append (0000)
000A3E00 0010:
	_slayer_engine_handle_client_message (0000)
000A3E10 0010:
	_slayer_engine_handle_server_message (0000)
000A3E20 0010:
	_slayer_engine_pregame_post_rasterize (0000)
000A3E30 0010:
	_slayer_engine_post_rasterize (0000)
000A3E40 0010:
	_slayer_engine_update (0000)
000A3E50 0010:
	_slayer_engine_allow_pick_up (0000)
000A3E60 0010:
	_slayer_engine_player_damaged_player (0000)
000A3E70 0110:
	_update_speed_for_score (0000)
000A3F80 0040:
	_slayer_engine_adjust_score (0000)
000A3FC0 0010:
	_slayer_engine_prespawn_player_update (0000)
000A3FD0 0040:
	_slayer_get_score (0000)
000A4010 0010:
	_slayer_test_flag (0000)
000A4020 0030:
	_slayer_get_score_string (0000)
000A4050 0060:
	_slayer_get_score_header_string (0000)
000A40B0 0030:
	_slayer_get_team_score_string (0000)
000A40E0 0190:
	_find_next_target (0000)
000A4270 0090:
	_slayer_engine_player_killed_player (0000)
000A4300 0280:
	_slayer_engine_display_score (0000)
000A4580 0190:
	_slayer_player_update (0000)
0025C190 0014:
	??_C@_0BE@HDNMKCGO@next_target?5?$CB?$DN?5NONE?$AA@ (0000)
0025C1A4 0029:
	??_C@_0CJ@GAGEHHLD@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025C1D0 0036:
	??_C@_0DG@HCMPGGBD@?$CIindex?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIindex?5?$DM?5MULTIPL@ (0000)
0025C208 0004:
	__real@373a69dc (0000)
0025C20C 0004:
	__real@38e90453 (0000)
002DE670 0088:
	_slayer_engine (0000)
0043ED80 0080:
	_bss_0043ed80 (0000)
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
#include "console.h"
#include "network_messages.h"
#include "text_group.h"
#include "font_group.h"
#include "network_server_message_handler.h"
#include "scenery.h"
#include "game_engine.h"
#include "game_globals.h"
#include "text_group.h"
#include "unicode.h"
#include "tag_groups.h"

/* ---------- constants */

enum
{
	slayer_message_new_target = 30
};

/* ---------- macros */

#define KILL_PENALTY_RECOVERY_PER_TICK (1.f/(5*TICKS_PER_MINUTE)) /* fake name */
#define DEATH_BONUS_RECOVERY_PER_TICK (1.f/(50*TICKS_PER_MINUTE)) /* fake name */

/* ---------- structures */

struct slayer_globals_type
{
	long team_score[MULTIPLAYER_MAXIMUM_PLAYERS];
	long individual_score[MULTIPLAYER_MAXIMUM_PLAYERS];
};

/* ---------- prototypes */

static boolean target_is_valid(long my_index, long old_target_index, long new_target_index);
static void find_next_target(long my_index);
static void slayer_engine_dispose(void);
static boolean slayer_engine_initialize_for_new_map(void);
static void slayer_engine_dispose_from_old_map(void);
static void slayer_engine_player_added(long player_index);
static void slayer_engine_game_ending(void);
static void slayer_engine_game_starting(void);
static void slayer_engine_statistics_append(struct game_statistics *permanent_statistics, struct game_statistics *game_statistics);
static void slayer_engine_handle_client_message(long player_index, void *encoded_message, short encoded_message_size);
static void slayer_engine_handle_server_message(void *encoded_message, short encoded_message_size);
static void slayer_engine_pregame_post_rasterize(void);
static void slayer_engine_post_rasterize(void);
static void slayer_engine_update(void);
static boolean slayer_engine_allow_pick_up(long unit_index, long item_index);
static void slayer_engine_player_damaged_player(long killing_player_index, long dead_player_index, boolean friendly_fire);
static void slayer_engine_adjust_score(long player_index, long adjustment);
static void slayer_engine_player_killed_player(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);
static boolean slayer_engine_display_score(long player_index, long message, long message_data, word *buffer, long buffer_size);
static void slayer_engine_prespawn_player_update(long player_index);
static void slayer_player_update(long player_index);
static long slayer_get_score(long player_index, enum get_score_type get_score_type);
static boolean slayer_test_flag(long flag);
static word *slayer_get_score_string(long player_index, word *buffer);
static word *slayer_get_score_header_string(word *buffer);
static word *slayer_get_team_score_string(long team_index, word *buffer);

/* ---------- globals */

struct game_engine slayer_engine =
{
	"slayer",
	_game_engine_slayer,
	slayer_engine_dispose,
	slayer_engine_initialize_for_new_map,
	slayer_engine_dispose_from_old_map,
	slayer_engine_player_added,
	slayer_engine_game_ending,
	slayer_engine_game_starting,
	slayer_engine_statistics_append,
	slayer_engine_handle_client_message,
	slayer_engine_handle_server_message,
	slayer_engine_pregame_post_rasterize,
	slayer_engine_post_rasterize,
	slayer_player_update,
	NULL, // weapon_update
	NULL, // weapon_pickup
	NULL, // weapon_drop
	slayer_engine_update,
	slayer_get_score,
	slayer_get_score_string,
	slayer_get_score_header_string,
	slayer_get_team_score_string,
	slayer_engine_allow_pick_up,
	slayer_engine_player_damaged_player,
	slayer_engine_player_killed_player,
	slayer_engine_display_score,
	NULL, // starting_location_rating
	slayer_engine_prespawn_player_update,
	NULL, // postspawn_player_update
	NULL, // game_engine_player_get_team_index
	NULL, // goal_matches_player
	slayer_test_flag,
	NULL, // game_engine_test_trait
	NULL, // game_engine_did_player_win
};

static struct slayer_globals_type slayer_globals;

/* ---------- private code */

static boolean target_is_valid(
	long my_index,
	long old_target_index,
	long new_target_index)
{
	boolean valid = FALSE;
	struct player_datum *my_player = player_get(my_index);
	struct player_datum *new_target = player_get(new_target_index);

	if (new_target_index != my_index &&
		new_target_index != old_target_index &&
		new_target->team_index != my_player->team_index &&
		new_target->unit_index != NONE)
	{
		valid = TRUE;
	}

	return valid;
}

static void find_next_target(
	long my_index)
{
	struct player_datum *player = player_get(my_index);
	long old_target_index = player->multiplayer.special;
	long next_target = NONE;
	struct data_iterator player_iterator;
	struct player_datum *other_player;
	long valid_target_count = 0;

	data_iterator_new(&player_iterator, player_data);

	while (other_player = (struct player_datum *)data_iterator_next(&player_iterator))
	{
		if (target_is_valid(my_index, old_target_index, player_iterator.index))
		{
			valid_target_count++;
		}
	}

	if (valid_target_count > 0)
	{
		valid_target_count = random_range(0, (short)valid_target_count);
		data_iterator_new(&player_iterator, player_data);

		while (other_player = (struct player_datum *)data_iterator_next(&player_iterator))
		{
			if (target_is_valid(my_index, old_target_index, player_iterator.index))
			{
				if (valid_target_count == 0)
				{
					next_target = player_iterator.index;
					break;
				}

				valid_target_count--;
			}
		}

		match_assert("c:\\halo\\SOURCE\\game\\game_engine_slayer.c", 195, next_target != NONE);
	}

	player->multiplayer.special = next_target;

	if (player->multiplayer.special != NONE)
	{
		game_show_score_extended(my_index, slayer_message_new_target, player->multiplayer.special);
	}

	return;
}

static void slayer_engine_dispose(
	void)
{
	return;
}

static boolean slayer_engine_initialize_for_new_map(
	void)
{
	memset(slayer_globals.team_score, 0, sizeof(slayer_globals.team_score));
	memset(slayer_globals.individual_score, 0, sizeof(slayer_globals.individual_score));

	return TRUE;
}

static void slayer_engine_dispose_from_old_map(
	void)
{
	return;
}

static void slayer_engine_player_added(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	player->multiplayer.special = NONE;

	return;
}

static void slayer_engine_game_ending(
	void)
{
	return;
}

static void slayer_engine_game_starting(
	void)
{
	game_engine_play_multiplayer_sound(game_engine_has_teams() ? _multiplayer_sound_team_slayer : _multiplayer_sound_slayer);

	return;
}

static void slayer_engine_statistics_append(
	struct game_statistics *permanent_statistics,
	struct game_statistics *game_statistics)
{
	return;
}

static void slayer_engine_handle_client_message(
	long player_index,
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void slayer_engine_handle_server_message(
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void slayer_engine_pregame_post_rasterize(
	void)
{
	return;
}

static void slayer_engine_post_rasterize(
	void)
{
	return;
}

static void slayer_engine_update(
	void)
{
	return;
}

static boolean slayer_engine_allow_pick_up(
	long unit_index,
	long item_index)
{
	return TRUE;
}

static void slayer_engine_player_damaged_player(
	long killing_player_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

void update_speed_for_score(
	long dead_player_index,
	long killing_player_index)
{
	real kill_penalty = 0.02f;
	real fast_kill_penalty = 0.15f;
	real minimum_speed = 0.9f;
	real death_bonus = 0.1f;
	real slow_death_bonus = 0.1f;
	real maximum_speed = 1.5f;
	struct player_datum *killing_player = player_get(killing_player_index);
	struct player_datum *dead_player = player_get(dead_player_index);

	if (!game_engine_get_variant()->game_engine_variant.slayer.no_kill_penalty)
	{
		killing_player->multiplayer.speed_multiplier -= kill_penalty;

		if (killing_player->multiplayer.speed_multiplier >= 1.f)
		{
			killing_player->multiplayer.speed_multiplier -= fast_kill_penalty;
			killing_player->multiplayer.speed_multiplier = MAX(killing_player->multiplayer.speed_multiplier, 1.f);
		}

		killing_player->multiplayer.speed_multiplier = MAX(killing_player->multiplayer.speed_multiplier, minimum_speed);
	}

	if (!game_engine_get_variant()->game_engine_variant.slayer.no_death_bonus)
	{
		dead_player->multiplayer.speed_multiplier += death_bonus;

		if (dead_player->multiplayer.speed_multiplier <= 1.f)
		{
			dead_player->multiplayer.speed_multiplier += slow_death_bonus;
			dead_player->multiplayer.speed_multiplier = MIN(dead_player->multiplayer.speed_multiplier, 1.f);
		}

		dead_player->multiplayer.speed_multiplier = MIN(dead_player->multiplayer.speed_multiplier, maximum_speed);
	}

	return;
}

static void slayer_engine_adjust_score(
	long player_index,
	long adjustment)
{
	struct player_datum *player = player_get(player_index);

	slayer_globals.team_score[player->team_index] += adjustment;
	slayer_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)] += adjustment;

	return;
}

static void slayer_engine_player_killed_player(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	struct player_datum *dead_player = player_get(dead_player_index);

	if (dead_player->quit_out_of_game)
	{
		return;
	}

	if (killing_player_index != NONE)
	{
		struct player_datum *killing_player = player_get(killing_player_index);

		if (!friendly_fire)
		{
			update_speed_for_score(dead_player_index, killing_player_index);

			if (game_engine_get_variant()->game_engine_variant.slayer.kill_in_order)
			{
				if (killing_player->multiplayer.special == dead_player_index)
				{
					find_next_target(killing_player_index);
					slayer_engine_adjust_score(killing_player_index, 1);
				}
			}
			else
			{
				slayer_engine_adjust_score(killing_player_index, 1);
			}
		}
		else
		{
			slayer_engine_adjust_score(killing_player_index, -1);
		}
	}

	return;
}

static boolean slayer_engine_display_score(
	long player_index,
	long message,
	long message_data,
	word *buffer,
	long buffer_size)
{
	struct player_datum *player = player_get(player_index);
	struct player_datum *target = NULL;
	wchar_t score_string[128];
	boolean result = TRUE;

	if (message == slayer_message_new_target)
	{
		if (game_engine_get_variant()->universal_variant.teams)
		{
			wchar_t *format;

			multiplayer_game_text_get_string(format, _string_n_team_n);
			usnprintf(score_string, NUMBEROF(score_string), format, slayer_get_score(player_index, _get_score_individual), slayer_get_score(player_index, _get_score_team));
		}
		else
		{
			usnprintf(score_string, NUMBEROF(score_string), L"%d", slayer_get_score(player_index, _get_score_individual));
		}

		target = player_get(message_data);
	}

	switch (message)
	{
	case slayer_message_new_target:
	{
		wchar_t *format;

		multiplayer_game_text_get_string(format, _string_new_target_name);
		usnprintf(buffer, buffer_size, format, target->name);
		break;
	}
	case game_engine_message_show_score:
	{
		wchar_t *place_name;
		wchar_t *format;

		if (game_engine_get_variant()->universal_variant.teams)
		{
			place_name = get_place_name(game_engine_get_place(player_index, _get_score_team));
			multiplayer_game_text_get_string(format, _string_name_kills_score_n_team_score_of_max);
			usnprintf(buffer, buffer_size, format, place_name, slayer_get_score(player_index, _get_score_individual), slayer_get_score(player_index, _get_score_team), game_engine_get_variant()->universal_variant.score_to_win);
		}
		else
		{
			place_name = get_place_name(game_engine_get_place(player_index, _get_score_team));
			multiplayer_game_text_get_string(format, _string_name_kills_score_of_max);
			usnprintf(buffer, buffer_size, format, place_name, slayer_get_score(player_index, _get_score_team), game_engine_get_variant()->universal_variant.score_to_win);
		}
		break;
	}
	default:
		result = FALSE;
		break;
	}

	return result;
}

static void slayer_engine_prespawn_player_update(
	long player_index)
{
	return;
}

static void slayer_player_update(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	if (game_engine_get_variant()->game_engine_variant.slayer.no_kill_penalty && player->multiplayer.speed_multiplier > 1.f)
	{
		player->multiplayer.speed_multiplier -= KILL_PENALTY_RECOVERY_PER_TICK;
		player->multiplayer.speed_multiplier = MAX(player->multiplayer.speed_multiplier, 1.f);
	}

	if (game_engine_get_variant()->game_engine_variant.slayer.no_death_bonus && player->multiplayer.speed_multiplier < 1.f)
	{
		player->multiplayer.speed_multiplier += DEATH_BONUS_RECOVERY_PER_TICK;
		player->multiplayer.speed_multiplier = MIN(player->multiplayer.speed_multiplier, 1.f);
	}

	if (game_engine_get_variant()->game_engine_variant.slayer.kill_in_order)
	{
		short index = (short)player_index;

		match_assert("c:\\halo\\SOURCE\\game\\game_engine_slayer.c", 513, (index >= 0) && (index < MULTIPLAYER_MAXIMUM_PLAYERS));

		game_engine_clear_goal_position(index);

		if (player->multiplayer.special != NONE)
		{
			struct player_datum *target_player = player_get(player->multiplayer.special);

			if (target_player->unit_index != NONE)
			{
				struct unit_datum *target_unit = unit_get(target_player->unit_index);
				real vertical_offset = 0.f;

				game_engine_set_goal_position(index, &target_unit->object.bounding_sphere_center, vertical_offset, "target_blue", player_index, NONE, NONE);
			}
		}

		if (player->unit_index != NONE && player->multiplayer.special == NONE)
		{
			find_next_target(player_index);
		}

		if (player->multiplayer.special != NONE && game_engine_man_out(player->multiplayer.special))
		{
			find_next_target(player_index);
		}
	}

	if (slayer_get_score(player_index, _get_score_team) >= game_engine_get_variant()->universal_variant.score_to_win)
	{
		game_engine_end_game();
	}

	return;
}

static long slayer_get_score(
	long player_index,
	enum get_score_type get_score_type)
{
	struct player_datum *player = player_get(player_index);
	long score;

	if (get_score_type == _get_score_team)
	{
		score = slayer_globals.team_score[player->team_index];
	}
	else
	{
		score = slayer_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)];
	}

	return score;
}

static boolean slayer_test_flag(
	long flag)
{
	boolean result = FALSE;

	switch (flag)
	{
	case _game_engine_show_kills_with_score_bit:
		result = TRUE;
		break;
	}

	return result;
}

static word *slayer_get_score_string(
	long player_index,
	word *buffer)
{
	usprintf(buffer, L"%d", slayer_globals.individual_score[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)]);

	return buffer;
}

static word *slayer_get_score_header_string(
	word *buffer)
{
	wchar_t *string;

	multiplayer_game_text_get_string(string, _string_score);
	ustrcpy(buffer, string);

	return buffer;
}

static word *slayer_get_team_score_string(
	long team_index,
	word *buffer)
{
	usprintf(buffer, L"%d", slayer_globals.team_score[team_index]);

	return buffer;
}

