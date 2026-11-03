/*
GAME_ENGINE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game_engine.h"
#include "players.h"
#include "game_engine_list.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "actor_definitions.h"
#include "console.h"
#include "rasterizer_console_vars.h"
#include "network_messages.h"
#include "collisions.h"
#include "text_group.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "weapons.h"
#include "bipeds.h"
#include "editor_stubs.h"
#include "hud.h"
#include "network_client_manager.h"
#include "font_group.h"
#include "network_server_message_handler.h"
#include "terminal.h"
#include "network_server_manager.h"
#include "object_types.h"
#include "player_ui.h"
#include "damage_effect_definitions.h"
#include "player_effects.h"
#include "hud_messaging.h"
#include "equipment_definitions.h"
#include "equipment.h"
#include "playlist_profile.h"
#include "sound_classes.h"

/* ---------- constants */


#define STATISTIC_BUFFER_PLACE_TIED_FLAG 0x80000000 /* fake name */
#define MAXIMUM_TITLE_STRING_LENGTH 80 /* fake name */

enum
{
	TELEPORTER_COLLISION_TEST_FLAGS =	/* fake name */
		FLAG(_collision_test_objects_bit) |
		FLAG(_collision_test_objects_bipeds_bit) |
		FLAG(_collision_test_objects_vehicles_bit) |
		FLAG(_collision_test_skip_passthrough_bipeds_bit)
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */


static void initialize_player_multiplayer_data(long player_index);
static void game_engine_build_lighting(void);
static void multiplayer_message(long player_index, long message, long message_data);
static boolean game_engine_player_is_odd_man_out(long player_index);
static long adjust_score_for_ranking(long player_index, long score);
static boolean is_place_tied(struct statistic_buffer *buffer);
static long place_get_position(struct statistic_buffer *buffer);
static wchar_t *get_place_string(struct statistic_buffer *buffer);
static struct statistic_buffer game_engine_get_player_place(long player_index);
static void game_engine_generate_title_string(long player_index, wchar_t *title_string);
static long select_players_to_display(long player_index, struct statistic_buffer *out_statistic_buffer, enum postgame_statistic postgame_statistic, long maximum_count);
static void drawline(wchar_t const *text, int line, short justification);
static void get_selected_color(real_argb_color *selected_color);
static boolean find_closest_player_callback(long object_index, void *custom_data);
static long find_closest_player_index(long player_index);
static void internal_rasterize_target_name(long player_index);
static boolean internal_rasterize_score(long player_index, long message, long message_data, wchar_t *buffer, long buffer_size);
static void game_engine_post_rasterize_in_game(void);
static void game_engine_update_teleporter(long player_index);
static long game_engine_did_team_win(long team_index);
static void game_engine_verify_current_map(void);
static boolean test_any_gamepad_button(long button_index);
static boolean game_engine_infinite_grenades_internal(void);

static void rasterize_in_game_score_draw_line(wchar_t const *text, boolean hilite, real_argb_color *color, long line);
static void game_engine_rasterize_in_game_score(long player_index, real alpha);
static void game_engine_predict_resources(void);
/* ---------- globals */

struct _game_engine_globals game_engine_globals;
struct play_stage global_stage;

struct netgame_goal global_goal[MAXIMUM_NETGAME_GOALS] = {0};
struct game_variant global_variant = {0};
struct game_engine *game_engine = NULL;

short debug_player_color = NONE;

/* ---------- public code */

long game_engine_get_team_score(
	long team_index)
{
	struct data_iterator player_iterator;
	struct player_datum *player;

	data_iterator_new(&player_iterator, player_data);

	while (player = (struct player_datum *)data_iterator_next(&player_iterator))
	{
		if (player->team_index == team_index)
		{
			return game_engine->get_score(player_iterator.index, _get_score_team);
		}
	}

	return 0;
}

real linear_to_non_linear_alpha(
	real linear_alpha)
{
	real non_linear_alpha = (real)pow(linear_alpha, 1.9f);

	return non_linear_alpha;
}

void game_engine_initialize(
	struct game_variant *variant)
{
	memset(&game_engine_globals, 0, sizeof(game_engine_globals));
	game_engine_globals.mode = game_engine_mode_active;

	if (variant && variant->game_engine_index)
	{
		global_variant = *variant;
		game_engine_variant_cleanup(&global_variant);
		game_engine = game_engines[variant->game_engine_index];
	}

	return;
}

void game_engine_dispose(
	void)
{
	if (game_engine)
	{
		if (game_engine->dispose)
		{
			game_engine->dispose();
		}

		game_engine = NULL;
	}

	return;
}

static void initialize_player_multiplayer_data(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	player->multiplayer.state_message = NONE;
	player->multiplayer.state_message_data = NONE;
	player->multiplayer.speed_multiplier = 1.f;
	player->multiplayer.teleporter_index = NONE;
	player->multiplayer.player_display_index = NONE;
	memset(&player->statistics.multiplayer_statistics, 0, sizeof(long));

	return;
}

static void game_engine_build_lighting(
	void)
{
	long player_count = 0;
	long vehicle_count = 0;

	{
		struct data_iterator player_iterator;

		data_iterator_new(&player_iterator, player_data);

		while (data_iterator_next(&player_iterator))
		{
			player_count++;
		}
	}

	if (global_variant.universal_variant.vehicle_set != _game_engine_vehicles_none)
	{
		if (global_variant.game_engine_index == _game_engine_race)
		{
			short flag_index;
			struct scenario *scenario = global_scenario_get();

			for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
			{
				struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

				if (flag->type == _netgame_flag_race_vehicle)
				{
					vehicle_count++;
				}
			}

			if (vehicle_count > player_count)
			{
				vehicle_count = player_count;
			}
		}
		else
		{
			struct object_iterator item_iterator;

			object_iterator_new(&item_iterator, _object_mask_vehicle, 0);

			while (object_iterator_next(&item_iterator))
			{
				vehicle_count++;
			}
		}
	}

	if (player_count > 8 && vehicle_count >= 2 || player_count >= 13)
	{
		SET_FLAG(game_engine_globals.flags, _game_engine_disable_dynamic_light_bit, TRUE);
	}

	if (player_count > 4 || vehicle_count >= 4 || TEST_FLAG(game_engine_globals.flags, _game_engine_disable_dynamic_light_bit))
	{
		SET_FLAG(game_engine_globals.flags, _game_engine_disable_integrated_lights_bit, TRUE);
	}

	if (player_count >= 5)
	{
		SET_FLAG(game_engine_globals.flags, _game_engine_5_or_more_players_bit, TRUE);
	}

	if (player_count >= 9)
	{
		SET_FLAG(game_engine_globals.flags, _game_engine_9_or_more_players_bit, TRUE);
	}

	return;
}

void game_engine_initialize_for_new_map(
	void)
{
	if (game_engine)
	{
		game_engine_verify_current_map();
		game_engine_intialize_queued_sounds();
		memset(global_goal, 0, sizeof(global_goal));
		game_engine_globals.current_team_to_add = 0;
		timeout_for_endgame_sound = 0;

		if (game_engine->initialize && !game_engine->initialize())
		{
			error(_error_silent, "failed to initialize custome game engine for new map, reverting to default game engine");
			game_engine_dispose();
		}

		game_engine_predict_resources();
	}

	return;
}

void game_engine_dispose_from_old_map(
	void)
{
	if (game_engine && game_engine->dispose_from_old_map)
	{
		game_engine->dispose_from_old_map();
	}

	return;
}

void game_engine_player_added(
	long player_index)
{
	initialize_player_multiplayer_data(player_index);

	if (game_engine)
	{
		struct player_datum *player = player_get(player_index);

		if (global_variant.universal_variant.teams)
		{
			if (global_network_game_client_get())
			{
				player->team_index = player->network_player_data.team_index % 2;
			}
			else
			{
				player->network_player_data.team_index = (char)game_engine_globals.current_team_to_add;
				player->team_index = (char)game_engine_globals.current_team_to_add;
				game_engine_globals.current_team_to_add = (game_engine_globals.current_team_to_add + 1) % 2;
			}
		}
		else
		{
			player->network_player_data.team_index = (char)game_engine_globals.current_team_to_add;
			player->team_index = (char)game_engine_globals.current_team_to_add;
			game_engine_globals.current_team_to_add++;
		}

		game_show_score(player_index, game_engine_message_welcome);

		if (game_engine->player_added)
		{
			game_engine->player_added(player_index);
		}
	}

	return;
}

void game_engine_game_ending(
	void)
{
	if (game_engine && game_engine->game_ending)
	{
		game_engine->game_ending();
	}

	return;
}

void game_engine_game_starting(
	void)
{
	if (game_engine)
	{
		if (game_engine->game_starting)
		{
			game_engine->game_starting();
		}

		game_engine_build_lighting();
	}

	return;
}

long players_in_game(
	void)
{
	struct data_iterator player_iterator;
	long player_count = 0;

	data_iterator_new(&player_iterator, player_data);

	while (data_iterator_next(&player_iterator))
	{
		player_count++;
	}

	return player_count;
}

boolean multiple_teams_alive(
	void)
{
	boolean result = FALSE;

	if (players_in_game() <= 1)
	{
		return TRUE;
	}
	else
	{
		long team_index = NONE;
		struct data_iterator player_iterator;
		struct player_datum *player;

		data_iterator_new(&player_iterator, player_data);

		while (player = data_iterator_next(&player_iterator))
		{
			boolean participating;

			if (player->quit_out_of_game)
			{
				participating = FALSE;
			}
			else
			{
				participating = player->unit_index != NONE || (!game_engine_player_is_odd_man_out(player_iterator.index) && !game_engine_player_is_out_of_lives(player_iterator.index));
			}

			if (participating)
			{
				match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 446, player->team_index != NONE);

				if (player->team_index != team_index)
				{
					if (team_index == NONE)
					{
						team_index = player->team_index;
					}
					else
					{
						result = TRUE;
						goto exit;
					}
				}
			}
		}
	}

exit:
	return result;
}

boolean team_has_players(
	long team_index)
{
	boolean result = FALSE;

	if (players_in_game() <= 1)
	{
		result = TRUE;
	}
	else
	{
		struct data_iterator player_iterator;
		struct player_datum *player;

		data_iterator_new(&player_iterator, player_data);

		while (player = data_iterator_next(&player_iterator))
		{
			boolean participating;

			if (player->quit_out_of_game)
			{
				participating = FALSE;
			}
			else
			{
				participating = player->unit_index != NONE || (!game_engine_player_is_odd_man_out(player_iterator.index) && !game_engine_player_is_out_of_lives(player_iterator.index));
			}

			if (participating)
			{
				match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 503, player->team_index != NONE);

				if (player->team_index == team_index)
				{
					result = TRUE;
				}

				goto exit;
			}
		}
	}

exit:
	return result;
}

boolean game_engine_should_end_game(
	void)
{
	boolean result = FALSE;

	if (game_engine)
	{
		if (!multiple_teams_alive())
		{
			result = TRUE;
		}
	}

	return result;
}

void game_engine_statistics_append(
	struct game_statistics *permanent_statistics,
	struct game_statistics *game_statistics)
{
	if (game_engine && game_engine->statistics_append)
	{
		game_engine->statistics_append(permanent_statistics, game_statistics);
	}

	return;
}

void game_engine_handle_client_message(
	long player_index,
	void *encoded_message,
	short encoded_message_size)
{
	if (game_engine && game_engine->handle_client_message)
	{
		game_engine->handle_client_message(player_index, encoded_message, encoded_message_size);
	}

	return;
}

void game_engine_handle_server_message(
	void *encoded_message,
	short encoded_message_size)
{
	if (game_engine && game_engine->handle_server_message)
	{
		game_engine->handle_server_message(encoded_message, encoded_message_size);
	}

	return;
}

int sort_statistic_buffer(
	struct statistic_buffer const *buffer_1,
	struct statistic_buffer const *buffer_2)
{
	int result = 0;

	if (buffer_1->score > buffer_2->score)
	{
		result = -1;
	}
	else if (buffer_2->score > buffer_1->score)
	{
		result = 1;
	}

	return result;
}

int sort_statistic_buffer_ranking(
	struct statistic_buffer const *buffer_1,
	struct statistic_buffer const *buffer_2)
{
	int result = 0;

	if (buffer_1->custom > buffer_2->custom)
	{
		result = -1;
	}
	else if (buffer_2->custom > buffer_1->custom)
	{
		result = 1;
	}
	else if (buffer_1->kills > buffer_2->kills)
	{
		result = -1;
	}
	else if (buffer_2->kills > buffer_1->kills)
	{
		result = 1;
	}
	else if (buffer_1->deaths > buffer_2->deaths)
	{
		result = 1;
	}
	else if (buffer_2->deaths > buffer_1->deaths)
	{
		result = -1;
	}
	else if (buffer_1->assists > buffer_2->assists)
	{
		result = -1;
	}
	else if (buffer_2->assists > buffer_1->assists)
	{
		result = 1;
	}

	return result;
}

static long adjust_score_for_ranking(
	long player_index,
	long score)
{
	struct player_datum *player;
	long flags = 0;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 669, player_index!=NONE);
	player = player_get(player_index);

	if (score < -1000)
	{
		score = -1000;
	}

	score += 1000;

	{
		long alive_flag = FLAG(30);
		long in_game_flag = FLAG(29);

		if (player->statistics.deaths < game_engine_get_variant()->universal_variant.lives)
		{
			flags |= alive_flag;
		}

		if (!player->quit_out_of_game)
		{
			flags |= in_game_flag;
		}
	}

	return score | flags;
}

long populate_statistic_buffer(
	struct statistic_buffer *statistic_buffer,
	enum postgame_statistic statistic,
	boolean inverse)
{
	struct data_iterator player_iterator;
	long index;
	long player_count = 0;
	boolean invert = (statistic == _postgame_statistic_deaths) ? !inverse : inverse;

	data_iterator_new(&player_iterator, player_data);

	while (data_iterator_next(&player_iterator))
	{
		match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 712, player_count < MULTIPLAYER_MAXIMUM_PLAYERS);

		if (player_count < MULTIPLAYER_MAXIMUM_PLAYERS)
		{
			statistic_buffer[player_count++].player_index = player_iterator.index;
		}
	}

	for (index = 0; index < player_count; index++)
	{
		long player_index = statistic_buffer[index].player_index;
		struct player_datum *player = player_get(player_index);

		switch (statistic)
		{
		case _postgame_statistic_ranking:
			statistic_buffer[index].custom = 0;

			if (game_engine->get_score)
			{
				statistic_buffer[index].custom = adjust_score_for_ranking(player_index, game_engine->get_score(player_index, _get_score_individual));
			}

			statistic_buffer[index].kills = player->statistics.kills[0];
			statistic_buffer[index].assists = player->statistics.assists[0];
			statistic_buffer[index].deaths = player->statistics.deaths;
			break;
		case _postgame_statistic_custom:
			statistic_buffer[index].score = 0;

			if (game_engine->get_score)
			{
				statistic_buffer[index].score = adjust_score_for_ranking(player_index, game_engine->get_score(player_index, _get_score_individual));
			}
			break;
		case _postgame_statistic_kills:
			statistic_buffer[index].score = player->statistics.kills[0];
			break;
		case _postgame_statistic_assists:
			statistic_buffer[index].score = player->statistics.assists[0];
			break;
		case _postgame_statistic_deaths:
			statistic_buffer[index].score = player->statistics.deaths;
			break;
		default:
			match_unreachable("c:\\halo\\SOURCE\\game\\game_engine.c", 765);
			break;
		}

		if (invert)
		{
			statistic_buffer[index].score = -statistic_buffer[index].score;
		}
	}

	if (statistic == _postgame_statistic_ranking)
	{
		qsort(statistic_buffer, player_count, sizeof(*statistic_buffer), (int (__cdecl *)(void const *, void const *))sort_statistic_buffer_ranking);
	}
	else
	{
		qsort(statistic_buffer, player_count, sizeof(*statistic_buffer), (int (__cdecl *)(void const *, void const *))sort_statistic_buffer);
	}

	for (index = 0; index < player_count; index++)
	{
		if (!index || sort_statistic_buffer_ranking(&statistic_buffer[index - 1], &statistic_buffer[index]))
		{
			statistic_buffer[index].place = index;
		}
		else
		{
			statistic_buffer[index - 1].place |= STATISTIC_BUFFER_PLACE_TIED_FLAG;
			statistic_buffer[index].place = statistic_buffer[index - 1].place;
		}
	}

	return player_count;
}

static boolean is_place_tied(
	struct statistic_buffer *buffer)
{
	boolean tied = (buffer->place & STATISTIC_BUFFER_PLACE_TIED_FLAG) > 0;

	return tied;
}

static long place_get_position(
	struct statistic_buffer *buffer)
{
	long position = buffer->place & ~STATISTIC_BUFFER_PLACE_TIED_FLAG;

	return position;
}

static wchar_t *get_place_string(
	struct statistic_buffer *buffer)
{
	wchar_t *result;
	short place = PIN(buffer->place & MASK(7), 0, MULTIPLAYER_MAXIMUM_PLAYERS - 1);

	multiplayer_game_text_get_string(result, place + _string_1st);

	return result;
}

static struct statistic_buffer game_engine_get_player_place(
	long player_index)
{
	struct statistic_buffer statistic_buffer[MULTIPLAYER_MAXIMUM_PLAYERS];
	long place = 0;
	boolean inverse = FALSE;

	populate_statistic_buffer(statistic_buffer, _postgame_statistic_ranking, inverse);

	while (TRUE)
	{
		if (statistic_buffer[place].player_index == player_index)
		{
			break;
		}

		place++;
		match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 866, place<MULTIPLAYER_MAXIMUM_PLAYERS);
	}

	return statistic_buffer[place];
}

static void game_engine_generate_title_string(
	long player_index,
	wchar_t *title_string)
{
	wchar_t n_limited_lives_string[128];
	struct player_datum *player = player_get(player_index);
	wchar_t *lives_string = L"";

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 878, title_string);

	if (global_variant.universal_variant.lives > 0)
	{
		long lives_remaining = global_variant.universal_variant.lives - player_get(player_index)->statistics.deaths;

		switch (lives_remaining)
		{
		case 0:
			multiplayer_game_text_get_string(lives_string, _string_no_lives);
			break;
		case 1:
			multiplayer_game_text_get_string(lives_string, _string_1_life);
			break;
		default:
		{
			wchar_t *format;

			multiplayer_game_text_get_string(format, _string_n_lives);
			usnprintf(n_limited_lives_string, NUMBEROF(n_limited_lives_string), format, lives_remaining);
			n_limited_lives_string[NUMBEROF(n_limited_lives_string) - 1] = '\0';
			lives_string = n_limited_lives_string;
			break;
		}
		}
	}

	if (game_engine_globals.mode == game_engine_mode_postgame_delay)
	{
		wchar_t *string;
		long win = game_engine_did_player_win(player_index);
		boolean teams = game_engine_has_teams();

		switch (win)
		{
		case NONE:
			multiplayer_game_text_get_string(string, _string_game_ends_in_a_draw);
			ustrncpy(title_string, string, MAXIMUM_TITLE_STRING_LENGTH);
			break;
		case FALSE:
			if (teams)
			{
				multiplayer_game_text_get_string(string, _string_your_team_lost);
				ustrncpy(title_string, string, MAXIMUM_TITLE_STRING_LENGTH);
			}
			else
			{
				multiplayer_game_text_get_string(string, _string_you_lost);
				ustrncpy(title_string, string, MAXIMUM_TITLE_STRING_LENGTH);
			}
			break;
		case TRUE:
			if (teams)
			{
				multiplayer_game_text_get_string(string, _string_your_team_won);
				ustrncpy(title_string, string, MAXIMUM_TITLE_STRING_LENGTH);
			}
			else
			{
				multiplayer_game_text_get_string(string, _string_you_won);
				ustrncpy(title_string, string, MAXIMUM_TITLE_STRING_LENGTH);
			}
			break;
		}
	}
	else if (game_engine_has_teams())
	{
		wchar_t team0_score_string[8];
		wchar_t team1_score_string[8];
		wchar_t *string;
		long team0_score;
		long team1_score;

		game_engine->get_team_score_string(0, team0_score_string);
		game_engine->get_team_score_string(1, team1_score_string);
		team0_score = game_engine_get_team_score(0);
		team1_score = game_engine_get_team_score(1);

		if (team0_score > team1_score)
		{
			multiplayer_game_text_get_string(string, _string_red_leads_blue_score_to_score_lives);
			usnprintf(title_string, MAXIMUM_TITLE_STRING_LENGTH,string, team0_score_string, team1_score_string, lives_string);
		}
		else if (team0_score < team1_score)
		{
			multiplayer_game_text_get_string(string, _string_blue_leads_red_score_to_score_lives);
			usnprintf(title_string, MAXIMUM_TITLE_STRING_LENGTH,string, team1_score_string, team0_score_string, lives_string);
		}
		else
		{
			multiplayer_game_text_get_string(string, _string_teams_tied_lives);
			usnprintf(title_string, MAXIMUM_TITLE_STRING_LENGTH,string, team1_score_string, lives_string);
		}
	}
	else
	{
		struct statistic_buffer place;
		wchar_t personalScore[256];
		wchar_t *string;

		place = game_engine_get_player_place(player_index);
		game_engine->get_score_string(player_index, personalScore);

		if (is_place_tied(&place))
		{
			multiplayer_game_text_get_string(string, _string_tied_for_nth_place_with_score_lives);
			usnprintf(title_string, MAXIMUM_TITLE_STRING_LENGTH,string, get_place_string(&place), personalScore, lives_string);
		}
		else
		{
			multiplayer_game_text_get_string(string, _string_in_nth_place_with_score_lives);
			usnprintf(title_string, MAXIMUM_TITLE_STRING_LENGTH,string, get_place_string(&place), personalScore, lives_string);
		}
	}

	title_string[MAXIMUM_TITLE_STRING_LENGTH - 1] = '\0';

	return;
}

long postgame_statistic_get_rating(
	long player_index,
	enum postgame_statistic statistic,
	boolean inverse)
{
	struct statistic_buffer statistic_buffer[MULTIPLAYER_MAXIMUM_PLAYERS];
	long player_count = populate_statistic_buffer(statistic_buffer, statistic, inverse);
	long rating = 0;

	if (statistic_buffer[0].player_index != player_index)
	{
		long place;

		for (place = 1; place < player_count; place++)
		{
			if (statistic_buffer[place - 1].score != statistic_buffer[place].score)
			{
				rating++;
			}

			if (statistic_buffer[place].player_index == player_index)
			{
				break;
			}
		}
	}

	return rating;
}

static long select_players_to_display(
	long player_index,
	struct statistic_buffer *out_statistic_buffer,
	enum postgame_statistic postgame_statistic,
	long maximum_count)
{
	struct statistic_buffer statistic_buffer[MULTIPLAYER_MAXIMUM_PLAYERS];
	struct statistic_buffer outside_range[4];
	boolean inverse = FALSE;
	long index;
	long player_count = populate_statistic_buffer(statistic_buffer, postgame_statistic, inverse);
	boolean debug = rasterizer_debug_options.pad3 == 69;
	long local_player_count = 0;

	if (debug)
	{
		terminal_printf(
			global_real_argb_white,
			"player_count=%d, maxcount=%d",
			player_count,
			maximum_count);
	}

	for (index = 0; index < player_count; index++)
	{
		struct player_datum *player = player_get(statistic_buffer[index].player_index);

		if (player->local_player_index != NONE)
		{
			local_player_count++;
		}
	}

	if (player_count > maximum_count)
	{
		long outside_index;
		long outside_count = 0;

		for (index = maximum_count; index < player_count; index++)
		{
			struct player_datum *player = player_get(statistic_buffer[index].player_index);

			if (player && player->local_player_index != NONE)
			{
				if (debug)
				{
					terminal_printf(global_real_argb_white, "found local player");
				}

				outside_range[outside_count++] = statistic_buffer[index];
			}
		}

		for (outside_index = 0; outside_index < outside_count; outside_index++)
		{
			long buffer_index;

			for (buffer_index = maximum_count - 1; buffer_index >= 0; buffer_index--)
			{
				struct player_datum *player = player_get(statistic_buffer[buffer_index].player_index);

				if (player->local_player_index == NONE)
				{
					struct statistic_buffer *destination = &statistic_buffer[buffer_index];
					struct statistic_buffer *source = &statistic_buffer[buffer_index + 1];
					long size = (maximum_count - buffer_index - 1) * sizeof(*statistic_buffer);

					memmove(destination, source, size);
					statistic_buffer[maximum_count - 1] = outside_range[outside_index];
					break;
				}
			}
		}
	}

	memcpy(
		out_statistic_buffer,
		statistic_buffer,
		sizeof(*statistic_buffer) * MIN(maximum_count, player_count));

	return MIN(maximum_count, player_count);
}

static void drawline(
	wchar_t const *text,
	int line,
	short justification)
{
	rectangle2d bounds = render.camera.window_bounds;
	long line_height = 18;
	long line_spacing = 8;

	offset_rectangle2d(&bounds, (short)-render.camera.viewport_bounds.x0, (short)-render.camera.viewport_bounds.y0);
	bounds.y1 = bounds.y0;
	bounds.y0 = line * line_height;
	bounds.y1 = (line + 1) * line_height + line_spacing;
	draw_string_set_format(NONE, justification, 0);
	rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, text);

	return;
}

static void get_selected_color(
	real_argb_color *selected_color)
{
	selected_color->red = 0.98f;
	selected_color->green = 0.96f;
	selected_color->blue = 0.96f;
	selected_color->alpha = 1.f;

	return;
}

void get_postgame_hilite_colors(
	real_argb_color *normal,
	real_argb_color *column_hilite,
	real_argb_color *winner)
{
	normal->red = 117.f / 255.f;
	normal->green = 186.f / 255.f;
	normal->blue = 1.f;
	normal->alpha = 1.f;
	column_hilite->red = 1.f;
	column_hilite->green = 1.f;
	column_hilite->blue = 0.f;
	column_hilite->alpha = 1.f;
	get_selected_color(winner);

	return;
}

void game_engine_post_rasterize_post_game(
	void)
{
	if (game_engine)
	{
		wchar_t buffer[256];
		wchar_t custom_buffer[256];
		struct statistic_buffer statistic_buffer[MULTIPLAYER_MAXIMUM_PLAYERS];
		real_argb_color normal_color;
		real_argb_color hilite_color;
		real_argb_color winner_color;
		wchar_t *fields[5];
		long player_count;
		long index;
		short tab_stops[6] = {50, 125, 250, 350, 410, 500};
		long font_index = hud_globals->messaging.single_player_font.index;

		get_postgame_hilite_colors(&normal_color, &hilite_color, &winner_color);
		draw_string_set_draw_mode(font_index, NONE, 2, 8, &normal_color);
		draw_string_set_color(&normal_color);
		draw_string_set_format(NONE, 0, 0);

		{
			rectangle2d rect;
			struct hud_globals_definition *hud_globals_definition = hud_globals_definition_get(interface_get_tag_index(_interface_hud_globals));

			rect.x0 = 0;
			rect.y0 = 0;
			rect.x1 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
			rect.y1 = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;

			if (bitmap_group_try_and_get_bitmap(hud_globals_definition->carnage_report_bitmap.index, 0))
			{
				draw_bitmap_in_rect(
					bitmap_group_try_and_get_bitmap(hud_globals_definition->carnage_report_bitmap.index, 0),
					&rect,
					&rect,
					NULL,
					NONE,
					NULL,
					TRUE);
			}
		}

		if (global_variant.universal_variant.teams)
		{
			wchar_t *team_string[2];
			short team_tab_stops[6] = {50, 200, 300, 350, 410, 500};
			long team_order[2] = {0, 1};
			long team_won = game_engine_did_team_win(0);

			multiplayer_game_text_get_string(team_string[0], _string_tab_red_team_tab_string);
			multiplayer_game_text_get_string(team_string[1], _string_tab_blue_team_tab_string);

			if (!team_won)
			{
				team_order[0] = 1;
				team_order[1] = 0;
			}

			draw_string_set_tab_stops(team_tab_stops, NUMBEROF(team_tab_stops));

			for (index = 0; index < 2; index++)
			{
				long team_index = team_order[index];

				game_engine->get_team_score_string(team_index, custom_buffer);
				usnprintf(buffer, NUMBEROF(buffer), team_string[team_index], custom_buffer);
				buffer[NUMBEROF(buffer) - 1] = '\0';
				drawline(buffer, index + 4, 0);
			}
		}

		multiplayer_game_text_get_string(fields[0], _string_place);
		multiplayer_game_text_get_string(fields[1], _string_name);
		multiplayer_game_text_get_string(fields[2], _string_kills);
		multiplayer_game_text_get_string(fields[3], _string_assists);
		multiplayer_game_text_get_string(fields[4], _string_deaths);
		game_engine->get_score_header_string(custom_buffer);
		usnprintf(
			buffer,
			NUMBEROF(buffer),
			L"\t%s\t%s\t%s\t%s\t%s\t%s",
			fields[0],
			fields[1],
			custom_buffer,
			fields[2],
			fields[3],
			fields[4]);
		buffer[NUMBEROF(buffer) - 1] = '\0';
		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		drawline(buffer, 7, 0);
		player_count = select_players_to_display(NONE, statistic_buffer, _postgame_statistic_ranking, 12);

		for (index = 0; index < player_count; index++)
		{
			long player_index = statistic_buffer[index].player_index;
			struct player_datum *player = player_get(player_index);
			int line = index + 8;

			if (player->local_player_index != NONE)
			{
				draw_string_set_color(&hilite_color);
			}
			else
			{
				draw_string_set_color(&normal_color);
			}

			draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
			usnprintf(buffer, NUMBEROF(buffer), L" \t%s", get_place_string(&statistic_buffer[index]));
			buffer[NUMBEROF(buffer) - 1] = '\0';
			drawline(buffer, line, 0);
			draw_string_set_color(&normal_color);

			if (global_variant.universal_variant.teams)
			{
				real_argb_color team_colors[2];

				team_colors[0].red = 0.8f;
				team_colors[0].green = 0.4f;
				team_colors[0].blue = 0.4f;
				team_colors[0].alpha = 1.f;
				team_colors[1].red = 0.4f;
				team_colors[1].green = 0.4f;
				team_colors[1].blue = 0.8f;
				team_colors[1].alpha = 1.f;
				draw_string_set_color(&team_colors[PIN(player->team_index, 0, 1)]);
			}

			usnprintf(buffer, NUMBEROF(buffer), L" \t \t%s", player->name);
			buffer[NUMBEROF(buffer) - 1] = '\0';
			drawline(buffer, line, 0);
			draw_string_set_color(&normal_color);

			if (!postgame_statistic_get_rating(player_index, _postgame_statistic_custom, FALSE))
			{
				draw_string_set_color(&winner_color);
			}

			game_engine->get_score_string(player_index, custom_buffer);
			usnprintf(buffer, NUMBEROF(buffer), L" \t \t \t%s", custom_buffer);
			buffer[NUMBEROF(buffer) - 1] = '\0';
			drawline(buffer, line, 0);
			draw_string_set_color(&normal_color);

			if (!postgame_statistic_get_rating(player_index, _postgame_statistic_kills, FALSE))
			{
				draw_string_set_color(&winner_color);
			}

			usnprintf(buffer, NUMBEROF(buffer), L" \t \t \t \t%d", player->statistics.kills[0]);
			buffer[NUMBEROF(buffer) - 1] = '\0';
			drawline(buffer, line, 0);
			draw_string_set_color(&normal_color);

			if (!postgame_statistic_get_rating(player_index, _postgame_statistic_assists, FALSE))
			{
				draw_string_set_color(&winner_color);
			}

			usnprintf(buffer, NUMBEROF(buffer), L" \t \t \t \t \t%d", player->statistics.assists[0]);
			buffer[NUMBEROF(buffer) - 1] = '\0';
			drawline(buffer, line, 0);
			draw_string_set_color(&normal_color);

			if (!postgame_statistic_get_rating(player_index, _postgame_statistic_deaths, FALSE))
			{
				draw_string_set_color(&winner_color);
			}

			usnprintf(buffer, NUMBEROF(buffer), L" \t \t \t \t \t \t%d", player->statistics.deaths);
			buffer[NUMBEROF(buffer) - 1] = '\0';
			drawline(buffer, line, 0);
			draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		}

		{
			wchar_t *string;
			rectangle2d bounds = render.camera.window_bounds;
			real_argb_color options_color = normal_color;

			options_color.alpha = game_engine_globals.post_game_options_fade;
			bounds.y0 = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT - 70;
			bounds.x0 = 70;
			offset_rectangle2d(&bounds, (short)-render.camera.viewport_bounds.x0, (short)-render.camera.viewport_bounds.y0);
			draw_string_set_tab_stops(NULL, 0);
			draw_string_set_color(&options_color);

			if (global_network_game_server_get())
			{
				bounds.x0 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH - 260;
				multiplayer_game_text_get_string(string, _string_server_post_game_stats_footer);
				draw_string_and_hack_in_icons(&bounds, NULL, NULL, 0, string, FALSE);
			}
			else
			{
				bounds.x0 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH - 120;
				multiplayer_game_text_get_string(string, _string_client_post_game_stats_footer);
				draw_string_and_hack_in_icons(&bounds, NULL, NULL, 0, string, FALSE);
			}
		}
	}

	return;
}

static boolean find_closest_player_callback(
	long object_index,
	void *custom_data)
{
	long player_index = *(long *)custom_data;
	boolean result = FALSE;
	struct object_datum *object = object_get(object_index);

	if (!TEST_FLAG(object->object.flags, _object_invisible_bit) &&
		TEST_FLAG(_object_mask_biped, object->object.type) &&
		!TEST_FLAG(object->object.damage_flags, _object_dead_bit) &&
		player_index_from_unit_index(object_index) != player_index)
	{
		result = TRUE;
	}

	return result;
}

static long find_closest_player_index(
	long player_index)
{
	long buffer[32];
	real_vector3d direction;
	real_point3d position;
	real_point3d autoaim_target_position;
	real autoaim_target_distance;
	real autoaim_target_angle;
	real_vector3d autoaim_target_direction;
	struct player_datum *player = player_get(player_index);
	long target_object_index = NONE;

	if (player->local_player_index != NONE && player_control_get_autoaim_level(player->local_player_index) > 0.f)
	{
		target_object_index = player_control_get_target_object_index(player->local_player_index);
	}

	if (target_object_index == NONE)
	{
		long object_count;
		long index;

		unit_get_camera_position(player->unit_index, &position);
		player_control_get_facing_direction(player->local_player_index, &direction);
		object_count = find_objects_from_point_vector(
			&position,
			&direction,
			find_closest_player_callback,
			&player_index,
			NUMBEROF(buffer),
			buffer);

		for (index = 0; index < object_count; index++)
		{
			long object_index = buffer[index];
			struct unit_datum *unit = unit_get(object_index);
			real distance_squared = distance_squared3d(&position, &unit->object.position);

			if ((unit->unit.active_camouflage < 1.f || player->multiplayer.player_display_index == player_index_from_unit_index(object_index)) &&
				autoaim_compute_target(
					buffer[index],
					&position,
					&direction,
					player->unit_index,
					&autoaim_target_position,
					&autoaim_target_direction,
					&autoaim_target_distance,
					&autoaim_target_angle) &&
				fabs(autoaim_target_angle) < 0.13083334f &&
				distance_squared < 400.f &&
				distance_squared < 900.f)
			{
				target_object_index = buffer[index];
			}
		}
	}

	if (target_object_index != NONE)
	{
		target_object_index = player_index_from_unit_index(target_object_index);
	}

	return target_object_index;
}

static void internal_rasterize_target_name(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	long closest_player_index = NONE;
	long maximum_display_count = 15;
	long fade_display_count = 10;

	if (player->local_player_index != NONE && player->unit_index != NONE)
	{
		closest_player_index = find_closest_player_index(player_index);

		if (closest_player_index == player_index)
		{
			closest_player_index = NONE;
		}
	}

	if (player->multiplayer.player_display_index != closest_player_index)
	{
		if (player->multiplayer.player_display_count > 0)
		{
			player->multiplayer.player_display_count--;
		}

		if (!player->multiplayer.player_display_count)
		{
			player->multiplayer.player_display_index = closest_player_index;
		}
	}
	else if (player->multiplayer.player_display_count < maximum_display_count)
	{
		player->multiplayer.player_display_count++;
	}

	if (player->multiplayer.player_display_index != NONE)
	{
		struct player_datum *display_player = player_get(player->multiplayer.player_display_index);
		wchar_t buffer[12] = {0};
		real display_count = (real)MIN(fade_display_count, player->multiplayer.player_display_count);
		real fraction = display_count / fade_display_count;
		real alpha = linear_to_non_linear_alpha(fraction);

		alpha *= 0.5f;
		ustrncpy(buffer, display_player->name, 11);
		buffer[11] = '\0';
		game_engine_rasterize_message(buffer, alpha);
	}

	return;
}

static boolean internal_rasterize_score(
	long player_index,
	long message,
	long message_data,
	wchar_t *buffer,
	long buffer_size)
{
	wchar_t *string;
	boolean result = TRUE;
	struct player_datum *player = player_get(player_index);
	long team_score = 0;

	if (game_engine_test_flag(_game_engine_show_kills_with_score_bit))
	{
		switch (message)
		{
		case game_engine_message_multi_kill:
			message = game_engine_message_multi_kill_with_score;
			break;
		case game_engine_message_triple_kill:
			message = game_engine_message_triple_kill_with_score;
			break;
		case game_engine_message_double_kill:
			message = game_engine_message_double_kill_with_score;
			break;
		case game_engine_message_ten_kills_in_a_row:
			message = game_engine_message_ten_kills_in_a_row_with_score;
			break;
		case game_engine_message_five_kills_in_row:
			message = game_engine_message_five_kills_in_row_with_score;
			break;
		case game_engine_message_killed_enemy:
			message = game_engine_message_killed_enemy_with_score;
			break;
		}

		if (message >= game_engine_message_multi_kill_with_score && message <= game_engine_message_killed_enemy_with_score)
		{
			team_score = game_engine->get_score(player_index, _get_score_team);
		}
	}

	switch (message)
	{
	case game_engine_message_welcome:
		multiplayer_game_text_get_string(string, _string_welcome_player_name);
		usnprintf(buffer, buffer_size, string, player->name);
		break;
	case game_engine_message_killed_by_unknown:
		multiplayer_game_text_get_string(string, _string_player_name_died);
		usnprintf(buffer, buffer_size, string, player->name);
		break;
	case game_engine_message_killed_by_biped:
		multiplayer_game_text_get_string(string, _string_player_name_was_killed_by_the_guardians);
		usnprintf(buffer, buffer_size, string, player->name);
		break;
	case game_engine_message_killed_by_vehicle:
		multiplayer_game_text_get_string(string, _string_player_name_was_killed_by_a_vehicle);
		usnprintf(buffer, buffer_size, string, player->name);
		break;
	case game_engine_message_killed_by_player:
	{
		struct player_datum *killer = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_player_name_was_killed_by_player_name);
		usnprintf(buffer, buffer_size, string, player->name, killer->name);
		break;
	}
	case game_engine_message_killed_by_friendly_fire:
	{
		struct player_datum *killer = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_player_name_was_betrayed_by_player_name);
		usnprintf(buffer, buffer_size, string, player->name, killer->name);
		break;
	}
	case game_engine_message_quit:
	{
		struct player_datum *quitter = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_player_name_quit);
		usnprintf(buffer, buffer_size, string, quitter->name);
		break;
	}
	case game_engine_message_killed_by_self:
		multiplayer_game_text_get_string(string, _string_player_name_committed_suicide);
		usnprintf(buffer, buffer_size, string, player->name);
		break;
	case game_engine_message_killed_friendly:
	{
		struct player_datum *victim = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_you_betrayed_player_name);
		usnprintf(buffer, buffer_size, string, victim->name);
		break;
	}
	case game_engine_message_multi_kill:
		multiplayer_game_text_get_string(string, _string_killtacular);
		ustrncpy(buffer, string, buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killtacular_kill);
		break;
	case game_engine_message_triple_kill:
		multiplayer_game_text_get_string(string, _string_triple_kill);
		ustrncpy(buffer, string, buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_triple_kill);
		break;
	case game_engine_message_double_kill:
		multiplayer_game_text_get_string(string, _string_double_kill);
		ustrncpy(buffer, string, buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_double_kill);
		break;
	case game_engine_message_ten_kills_in_a_row:
		multiplayer_game_text_get_string(string, _string_running_riot);
		ustrncpy(buffer, string, buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_running_riot);
		break;
	case game_engine_message_five_kills_in_row:
		multiplayer_game_text_get_string(string, _string_you_are_on_a_killing_spree);
		ustrncpy(buffer, string, buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killing_spree);
		break;
	case game_engine_message_killed_enemy:
	{
		struct player_datum *victim = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_you_killed_player_name);
		usnprintf(buffer, buffer_size, string, victim->name);
		break;
	}
	case game_engine_message_multi_kill_with_score:
		multiplayer_game_text_get_string(string, _string_killtacular_score);
		usnprintf(buffer, buffer_size, string, team_score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killtacular_kill);
		break;
	case game_engine_message_triple_kill_with_score:
		multiplayer_game_text_get_string(string, _string_triple_kill_score);
		usnprintf(buffer, buffer_size, string, team_score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_triple_kill);
		break;
	case game_engine_message_double_kill_with_score:
		multiplayer_game_text_get_string(string, _string_double_kill_score);
		usnprintf(buffer, buffer_size, string, team_score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_double_kill);
		break;
	case game_engine_message_ten_kills_in_a_row_with_score:
		multiplayer_game_text_get_string(string, _string_running_riot_score);
		usnprintf(buffer, buffer_size, string, team_score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_running_riot);
		break;
	case game_engine_message_five_kills_in_row_with_score:
		multiplayer_game_text_get_string(string, _string_you_are_on_a_killing_spree_score);
		usnprintf(buffer, buffer_size, string, team_score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killing_spree);
		break;
	case game_engine_message_killed_enemy_with_score:
	{
		struct player_datum *victim = player_get(message_data);

		multiplayer_game_text_get_string(string, _string_you_killed_player_name_score);
		usnprintf(buffer, buffer_size, string, victim->name, team_score);
		break;
	}
	case game_engine_message_odd_man_out:
		multiplayer_game_text_get_string(string, _string_you_are_the_odd_man_out);
		ustrncpy(buffer, string, buffer_size);
		break;
	case game_engine_message_out_of_lives:
		multiplayer_game_text_get_string(string, _string_you_are_out_of_lives);
		ustrncpy(buffer, string, buffer_size);
		break;
	case game_engine_message_respawn_timer:
		multiplayer_game_text_get_string(string, _string_rejoin_in_seconds);
		usnprintf(buffer, buffer_size, string, message_data);
		break;
	case game_engine_message_waiting_for_space_to_clear:
		multiplayer_game_text_get_string(string, _string_waiting_for_space_to_clear);
		ustrncpy(buffer, string, buffer_size);
		break;
	case game_engine_message_player_quit_self:
		multiplayer_game_text_get_string(string, _string_you_quit_out_of_game);
		ustrncpy(buffer, string, buffer_size);
		break;
	case game_engine_message_press_back_for_score:
		multiplayer_game_text_get_string(string, _string_hold_back_for_score);
		ustrncpy(buffer, string, buffer_size);
		break;
	default:
		result = FALSE;
		break;
	}

	buffer[buffer_size - 1] = '\0';

	return result;
}

static void game_engine_post_rasterize_in_game(
	void)
{
	struct gamepad_state const *gamepad;
	real alpha;
	long local_player_index = render.local_player_index;
	long player_index = local_player_get_player_index(local_player_index);
	struct player_datum *player = player_get(player_index);

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 1904, NONE != local_player_index);
	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 1905, NULL != game_engine);

	if (game_engine && player)
	{
		internal_rasterize_target_name(player_index);
	}

	gamepad = input_get_gamepad_state(local_player_index);
	alpha = game_engine_globals.score_alpha[local_player_index];

	if ((!gamepad || !gamepad->buttons[_gamepad_binary_button_back]) && game_engine_globals.mode != game_engine_mode_postgame_delay)
	{
		alpha -= 1.f / 15.f;
	}
	else
	{
		alpha += 1.f / 15.f;
	}

	alpha = PIN(alpha, 0.f, 1.f);

	if (alpha > 0.f)
	{
		game_engine_rasterize_in_game_score(player_index, linear_to_non_linear_alpha(alpha));
	}

	game_engine_globals.score_alpha[local_player_index] = alpha;

	return;
}

void game_engine_post_rasterize_objects(
	void)
{
	if (game_engine && game_engine->post_rasterize)
	{
		game_engine->post_rasterize();
	}

	return;
}

void game_engine_post_rasterize(
	void)
{
	if (game_engine)
	{
		switch (game_engine_globals.mode)
		{
		case game_engine_mode_active:
		case game_engine_mode_postgame_delay:
			game_engine_post_rasterize_in_game();
			break;
		case game_engine_mode_postgame_rasterize_delay:
		case game_engine_mode_postgame_rasterize:
			game_engine_post_rasterize_post_game();
			break;
		default:
			match_unreachable("c:\\halo\\SOURCE\\game\\game_engine.c", 1975);
			break;
		}
	}

	return;
}

static boolean can_delete_item(
	long item_index)
{
	struct weapon_datum *weapon = weapon_try_and_get(item_index);
	boolean result = TRUE;

	if (weapon && weapon_is_flag(item_index))
	{
		result = FALSE;
	}

	return result;
}

static void game_engine_update_purge(
	void)
{
	long purge_time = game_time_get() - 30 * TICKS_PER_SECOND;

	{
		struct object_iterator item_iterator;

		object_iterator_new(&item_iterator, _object_mask_item, 0);

		while (object_iterator_next(&item_iterator))
		{
			struct item_datum *item = item_get(item_iterator.index);

			if (item->item.last_owned_time >= purge_time)
			{
				continue;
			}

			if (TEST_FLAG(item->item.flags, _item_in_unit_inventory_bit))
			{
				continue;
			}

			if (!can_delete_item(item_iterator.index))
			{
				continue;
			}

			object_delete(item_iterator.index);
		}
	}

	{
		struct object_iterator biped_iterator;

		object_iterator_new(&biped_iterator, _object_mask_biped, 0);

		while (object_iterator_next(&biped_iterator))
		{
			struct object_datum *biped = object_get(biped_iterator.index);

			if (biped->object.idle_ticks > 30 * TICKS_PER_SECOND && TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				object_delete(biped_iterator.index);
			}
		}
	}

	return;
}

static void update_weapon_inventory(
	long weapon_index)
{
	struct weapon_datum *weapon;
	boolean attached;
	boolean in_inventory;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2058, weapon_index != NONE);
	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2059, weapon_is_flag(weapon_index));

	weapon = weapon_get(weapon_index);
	attached = weapon->object.parent_object_index != NONE;
	in_inventory = TEST_FLAG(weapon->item.flags, _item_in_unit_inventory_bit);

	if (!attached && !in_inventory)
	{
		if (TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_inventory_flag))
		{
			SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_inventory_flag, FALSE);

			if (game_engine->weapon_drop)
			{
				game_engine->weapon_drop(weapon_index);
			}
		}
	}

	return;
}

static void game_engine_update_weapons(
	void)
{
	struct object_iterator item_iterator;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2088, NULL != game_engine);

	object_iterator_new(&item_iterator, _object_mask_item, 0);

	while (object_iterator_next(&item_iterator))
	{
		struct item_datum *item = item_get(item_iterator.index);

		if (TEST_FLAG(item->item.flags, _item_in_unit_inventory_bit))
		{
			item->object.scale = 1.f;
		}
		else
		{
			struct item_definition *definition = item_definition_get(item->definition_index);

			item->object.scale = definition->item.scale != 0.f ? definition->item.scale : 1.f;
			match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2110, (item->object.scale >= 0.5f) && (item->object.scale <= 3.f));
		}

		if (game_engine->weapon_update)
		{
			struct weapon_datum *weapon = weapon_try_and_get(item_iterator.index);

			if (weapon && weapon_is_flag(item_iterator.index))
			{
				update_weapon_inventory(item_iterator.index);
				game_engine->weapon_update(item_iterator.index, weapon);
			}
		}
	}

	return;
}

static long item_collection_get_total(
	struct item_collection_definition *item_collection)
{
	long permutation_index;
	struct item_permutation_definition *permutations = (struct item_permutation_definition *)item_collection->permutations.address;
	long permutation_count = item_collection->permutations.count;
	long total = 0;

	for (permutation_index = 0; permutation_index < permutation_count; permutation_index++)
	{
		total = (long)(total + permutations[permutation_index].weight);
	}

	return total;
}

static long random_item(
	long item_collection_index)
{
	struct item_collection_definition *item_collection = item_collection_definition_get(item_collection_index);
	long permutation_index;
	long permutation_count = item_collection->permutations.count;
	long item_index = NONE;
	long random_index = random_range(0, (short)item_collection_get_total(item_collection));
	long random_value = random_index;
	struct item_permutation_definition *permutations = (struct item_permutation_definition *)item_collection->permutations.address;

	for (permutation_index = 0; permutation_index < permutation_count; permutation_index++)
	{
		random_value = (long)(random_value - permutations[permutation_index].weight);

		if (random_value < 0)
		{
			item_index = permutations[permutation_index].item.index;
			break;
		}
	}

	return item_index;
}

static long game_engine_get_type(
	void)
{
	long type = NONE;

	if (game_engine_running())
	{
		type = game_engine->type;
	}

	return type;
}

boolean match_game_type(
	long game_engine_type,
	long count,
	short const *game_type)
{
	boolean result;
	long game_type_index;

	if (game_engine_running())
	{
		result = FALSE;

		for (game_type_index = 0; game_type_index < count; game_type_index++)
		{
			result |= game_type[game_type_index] == game_engine_type;

			if (game_type[game_type_index] == _game_engine_all)
			{
				result |= TRUE;
			}
			else if (game_type[game_type_index] == _game_engine_all_non_team)
			{
				result |= game_engine_type != _game_engine_ctf;
			}
			else if (game_type[game_type_index] == _game_engine_all_normal)
			{
				result |= game_engine_type != _game_engine_ctf && game_engine_type != _game_engine_race;
			}
		}
	}
	else
	{
		result = TRUE;

		for (game_type_index = 0; game_type_index < count; game_type_index++)
		{
			result &= game_type[game_type_index] == _game_engine_none;
		}
	}

	return result;
}

static void game_engine_update_item_spawn(
	void)
{
	short equipment_index;
	struct scenario *scenario = global_scenario_get();

	for (equipment_index = 0; equipment_index < scenario->netgame_equipment.count; equipment_index++)
	{
		struct scenario_netgame_equipment *equipment = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_equipment, equipment_index, struct scenario_netgame_equipment);

		if (match_game_type(game_engine_get_type(), NUMBEROF(equipment->game_type), equipment->game_type))
		{
			short spawn_time = equipment->spawn_time;
			long spawn_ticks = 30 * TICKS_PER_SECOND;

			if (!spawn_time && equipment->item_collection.index != NONE)
			{
				spawn_time = item_collection_definition_get(equipment->item_collection.index)->spawn_time;
			}

			if (spawn_time)
			{
				spawn_ticks = spawn_time * TICKS_PER_SECOND;
			}

			if (!(game_time_get() % spawn_ticks))
			{
				long item_definition_index = random_item(equipment->item_collection.index);
				long item_index;
				struct object_placement_data placement_data;

				object_placement_data_new(&placement_data, item_definition_index, NONE);
				placement_data.position = equipment->position;
				item_index = object_new(&placement_data);

				if (item_index != NONE)
				{
					struct item_datum *item = item_get(item_index);

					object_set_garbage(item_index, FALSE);

					if (TEST_FLAG(equipment->flags, _netgame_equipment_flag_float))
					{
						SET_FLAG(item->object.flags, _object_at_rest_bit, TRUE);
					}

					item->item.last_owned_time += spawn_ticks - 30 * TICKS_PER_SECOND;
				}
			}
		}
	}

	return;
}

void game_engine_load_stage(
	char const *map_name)
{
	if (!map_name || strcmp(global_stage.map_name, map_name))
	{
		main_set_multiplayer_map_name(global_stage.map_name);
	}

	game_set_game_variant(&global_stage.game_variant);

	if (!network_game_is_active())
	{
		main_reset_map();
	}

	return;
}

void game_engine_playlist_begin(
	void)
{
	main_set_multiplayer_map_name(global_stage.map_name);
	game_set_game_variant(&global_stage.game_variant);

	if (!network_game_is_active())
	{
		main_reset_map();
	}

	return;
}

boolean game_engine_get_current_stage(
	struct game_variant *variant,
	char *map_name)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2328, variant && map_name);

	memcpy(variant, &global_stage.game_variant, sizeof(*variant));
	strncpy(map_name, global_stage.map_name, sizeof(global_stage.map_name) - 1);
	map_name[sizeof(global_stage.map_name) - 1] = '\0';

	return TRUE;
}

static void game_engine_update_player_no_shield(
	long player_index)
{
	if (!game_engine_has_shield(player_index))
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);

			unit->object.shield_vitality = 0.f;
			unit->object.maximum_shield_vitality = 0.f;
		}
	}

	return;
}

void game_engine_update_player_always_invis(
	long player_index)
{
	if (game_engine_running())
	{
		boolean invisible = FALSE;

		if (TEST_FLAG(global_variant.universal_variant.flags, _always_invisible_bit))
		{
			invisible = TRUE;
		}
		else if (game_engine->game_engine_test_trait)
		{
			invisible = game_engine->game_engine_test_trait(player_index, _game_trait_invisible);
		}

		if (invisible)
		{
			if (player_get(player_index)->unit_index != NONE)
			{
				player_handle_powerup_minor(player_index, _player_powerup_active_camouflage, 15);
			}
		}
	}

	return;
}

void game_engine_end_game(
	void)
{
	if (game_engine_globals.mode == game_engine_mode_active)
	{
		game_engine_globals.mode = game_engine_mode_postgame_delay;
		game_engine_globals.game_ending_timer = 7.f;
		game_engine_play_multiplayer_sound(_multiplayer_sound_game_over);
		ui_widgets_close_all();
	}

	return;
}

void game_engine_update_non_deterministic(
	real seconds_elapsed)
{
	if (game_engine)
	{
		switch (game_engine_globals.mode)
		{
		case game_engine_mode_postgame_rasterize_delay:
			rumble_clear_all_now();
			game_engine_globals.game_ending_timer -= seconds_elapsed;

			if (game_engine_globals.game_ending_timer <= 0.f)
			{
				game_engine_globals.mode = game_engine_mode_postgame_rasterize;
			}
			break;
		case game_engine_mode_postgame_rasterize:
			rumble_clear_all_now();
			game_engine_globals.post_game_options_fade += seconds_elapsed;

			if (game_engine_globals.post_game_options_fade > 1.f)
			{
				game_engine_globals.post_game_options_fade = 1.f;
			}

			if (test_any_gamepad_button(_gamepad_analog_button_a) || test_any_gamepad_button(_gamepad_binary_button_start))
			{
				if (global_network_game_server_get())
				{
					network_game_server_reset_to_pregame(global_network_game_server_get());
				}
			}
			else if (test_any_gamepad_button(_gamepad_analog_button_b) || test_any_gamepad_button(_gamepad_binary_button_back))
			{
				network_game_abort();
			}
			break;
		}
	}

	return;
}

void game_engine_update(
	void)
{
	if (game_engine)
	{
		game_engine_update_multiplayer_sound();
		game_engine_update_purge();
		game_engine_update_weapons();
		game_engine_update_item_spawn();

		{
			struct data_iterator player_iterator;

			data_iterator_new(&player_iterator, player_data);

			while (data_iterator_next(&player_iterator))
			{
				game_engine_update_player_no_shield(player_iterator.index);
				game_engine_update_player_always_invis(player_iterator.index);
				game_engine_update_teleporter(player_iterator.index);

				if (game_engine->player_update)
				{
					game_engine->player_update(player_iterator.index);
				}
			}
		}

		if (game_engine->update)
		{
			game_engine->update();
		}

		switch (game_engine_globals.mode)
		{
		case game_engine_mode_active:
			if (game_engine_should_end_game())
			{
				game_engine_end_game();
			}
			break;
		case game_engine_mode_postgame_delay:
			if (game_engine_globals.game_ending_timer <= 2.f && !TEST_FLAG(game_engine_globals.flags, _game_engine_game_over_sound_disabled_bit))
			{
				sound_class_set_gain("", 0.f, TICKS_PER_SECOND);
				sound_class_set_gain("ambient_nature", 0.2f, TICKS_PER_SECOND);
				sound_class_set_gain("ambient_machinery", 0.2f, TICKS_PER_SECOND);
				sound_class_set_gain("ambient_computers", 0.2f, TICKS_PER_SECOND);
				SET_FLAG(game_engine_globals.flags, _game_engine_game_over_sound_disabled_bit, TRUE);
			}

			game_engine_globals.game_ending_timer -= 1.f / TICKS_PER_SECOND;

			if (game_engine_globals.game_ending_timer <= 0.f)
			{
				game_engine_globals.post_game_options_fade = 0.f;
				game_engine_globals.mode = game_engine_mode_postgame_rasterize_delay;
				game_engine_globals.game_ending_timer = 5.f;

				{
					struct data_iterator player_iterator;
					struct player_datum *player;

					data_iterator_new(&player_iterator, player_data);

					while (player = (struct player_datum *)data_iterator_next(&player_iterator))
					{
						if (player->unit_index != NONE)
						{
							unit_kill(player->unit_index);
						}

						if (player->local_player_index != NONE)
						{
							rumble_player_clear(player->local_player_index);
						}
					}
				}

				{
					struct object_iterator vehicle_iterator;

					object_iterator_new(&vehicle_iterator, _object_mask_vehicle, 0);

					while (object_iterator_next(&vehicle_iterator))
					{
						object_delete(vehicle_iterator.index);
					}
				}

				if (global_network_game_server_get())
				{
					network_game_server_switch_to_postgame(global_network_game_server_get());
				}
			}
			break;
		case game_engine_mode_postgame_rasterize_delay:
		case game_engine_mode_postgame_rasterize:
			break;
		default:
			match_unreachable("c:\\halo\\SOURCE\\game\\game_engine.c", 2563);
		}
	}

	return;
}

boolean game_engine_allow_pick_up(
	long unit_index,
	long item_index)
{
	boolean result = TRUE;

	if (game_engine && game_engine->allow_pick_up)
	{
		result = game_engine->allow_pick_up(unit_index, item_index);
	}

	return result;
}

void game_engine_player_damaged_player(
	long killing_player_index,
	long dead_player_index,
	boolean friendly_fire)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2592, dead_player_index != NONE);

	if (game_engine && game_engine->player_damaged_player)
	{
		game_engine->player_damaged_player(killing_player_index, dead_player_index, friendly_fire);
	}

	return;
}

void game_engine_player_killed(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	struct player_datum *dead_player = player_get(dead_player_index);
	boolean suicide;
	boolean both_players_valid;
	boolean legitimate_kill;
	long message = NONE;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2609, dead_player_index != NONE);

	if (!game_engine)
	{
		return;
	}

	dead_player->multiplayer.time_of_death = game_time_get();

	if (game_engine->player_killed_player)
	{
		game_engine->player_killed_player(killing_player_index, killing_object_index, dead_player_index, friendly_fire);
	}

	suicide = killing_player_index == dead_player_index;
	both_players_valid = killing_player_index != NONE && dead_player_index != NONE;
	legitimate_kill = !friendly_fire && both_players_valid && !suicide;
	dead_player->respawn_timer = dead_player->respawn_penalty + global_variant.universal_variant.respawn_time;

	if (global_variant.universal_variant.respawn_time_growth > 0)
	{
		dead_player->respawn_penalty += global_variant.universal_variant.respawn_time_growth;
		dead_player->respawn_penalty = MIN(dead_player->respawn_penalty, 5 * global_variant.universal_variant.respawn_time_growth);

		if (legitimate_kill && killing_player_index != NONE)
		{
			struct player_datum *killing_player = player_get(killing_player_index);

			killing_player->respawn_penalty -= global_variant.universal_variant.respawn_time_growth;
			killing_player->respawn_penalty = MAX(killing_player->respawn_penalty, 0);
		}
	}

	if (!legitimate_kill)
	{
		dead_player->respawn_timer += global_variant.universal_variant.suicide_penalty;
	}

	dead_player->respawn_timer = MAX(dead_player->respawn_timer, 3 * TICKS_PER_SECOND);

	if (!player_get(dead_player_index)->quit_out_of_game)
	{
		if (killing_player_index == NONE)
		{
			if (killing_object_index == NONE)
			{
				message = game_engine_message_killed_by_unknown;
			}
			else
			{
				struct object_datum *killing_object = object_get(killing_object_index);
				struct unit_datum *killing_unit = unit_try_and_get(killing_object_index);

				switch (killing_object->object.type)
				{
				case _object_type_biped:
					message = game_engine_message_killed_by_biped;
					break;
				case _object_type_vehicle:
					message = game_engine_message_killed_by_vehicle;
					break;
				default:
					message = game_engine_message_killed_by_unknown;
					break;
				}
			}
		}
		else if (killing_player_index == dead_player_index)
		{
			message = game_engine_message_killed_by_self;
		}
		else
		{
			message = friendly_fire ? game_engine_message_killed_by_friendly_fire : game_engine_message_killed_by_player;
		}

		game_show_score_extended(dead_player_index, message, killing_player_index);
	}
	else
	{
		game_show_score_extended(NONE, game_engine_message_quit, dead_player_index);
	}

	do
	{
		if (message == game_engine_message_killed_by_friendly_fire)
		{
			game_show_score_extended(killing_player_index, game_engine_message_killed_friendly, dead_player_index);
		}
		else if (message == game_engine_message_killed_by_player)
		{
			long killing_message;
			struct player_datum *killing_player = player_get(killing_player_index);

			if (killing_player->statistics.multiple_kills >= 4)
			{
				killing_message = game_engine_message_multi_kill;
			}
			else if (killing_player->statistics.multiple_kills == 3)
			{
				killing_message = game_engine_message_triple_kill;
			}
			else if (killing_player->statistics.multiple_kills == 2)
			{
				killing_message = game_engine_message_double_kill;
			}
			else if (killing_player->statistics.kills_in_a_row == 5)
			{
				killing_message = game_engine_message_five_kills_in_row;
			}
			else
			{
				killing_message = killing_player->statistics.kills_in_a_row % 5 ? game_engine_message_killed_enemy : game_engine_message_ten_kills_in_a_row;
			}

			game_show_score_extended(killing_player_index, killing_message, dead_player_index);
		}

		message = NONE;
	}
	while (message != NONE);

	return;
}

static boolean multiplayer_message_internal(
	long player_index,
	long message,
	long message_data,
	wchar_t *buffer,
	long buffer_length)
{
	boolean result = FALSE;

	if (game_engine->rasterize_score)
	{
		result = game_engine->rasterize_score(player_index, message, message_data, buffer, buffer_length);
	}

	if (!result)
	{
		result = internal_rasterize_score(player_index, message, message_data, buffer, buffer_length);
	}

	return result;
}

static void multiplayer_message(
	long player_index,
	long message,
	long message_data)
{
	struct player_datum *player = player_get(player_index);

	if (player->local_player_index != NONE)
	{
		wchar_t buffer[1024];
		long buffer_length = NUMBEROF(buffer);
		boolean success = FALSE;

		success = multiplayer_message_internal(player_index, message, message_data, buffer, buffer_length);

		if (success)
		{
			buffer[buffer_length - 1] = '\0';
			hud_print_message(player->local_player_index, buffer);
		}
	}

	return;
}

static void game_show_score_one_player(
	long player_index,
	long message,
	long message_data)
{
	if (message != NONE)
	{
		multiplayer_message(player_index, message, message_data);
	}

	return;
}

void game_show_score_team(
	long team_index,
	long message)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = (struct player_datum *)data_iterator_next(&iterator))
	{
		if (player->team_index == team_index)
		{
			game_show_score_one_player(iterator.index, message, NONE);
		}
	}

	return;
}

void game_show_score_you_ally_enemy(
	long player_index,
	long you_message,
	long ally_message,
	long enemy_message,
	long message_data)
{
	struct data_iterator iterator;
	struct player_datum *other_player;
	struct player_datum *player = player_get(player_index);

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 2842, NONE != player_index);

	data_iterator_new(&iterator, player_data);

	while (other_player = (struct player_datum *)data_iterator_next(&iterator))
	{
		if (iterator.index == player_index)
		{
			game_show_score_one_player(iterator.index, you_message, message_data);
		}
		else
		{
			game_show_score_one_player(iterator.index, game_team_is_enemy((short)player->team_index, (short)other_player->team_index) ? enemy_message : ally_message, message_data);
		}
	}

	return;
}

void game_show_score_extended(
	long player_index,
	long message,
	long message_data)
{
	if (player_index != NONE)
	{
		game_show_score_one_player(player_index, message, message_data);
	}
	else
	{
		struct data_iterator iterator;

		data_iterator_new(&iterator, player_data);

		while (data_iterator_next(&iterator))
		{
			game_show_score_one_player(iterator.index, message, message_data);
		}
	}

	return;
}

void game_show_score(
	long player_index,
	long message)
{
	game_show_score_extended(player_index, message, NONE);

	return;
}

static boolean game_engine_player_is_odd_man_out(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	boolean result = FALSE;

	if (global_variant.universal_variant.odd_man_out && player->unit_index == NONE)
	{
		struct data_iterator player_iterator;
		struct player_datum *other_player;

		result = TRUE;
		data_iterator_new(&player_iterator, player_data);

		while (other_player = (struct player_datum *)data_iterator_next(&player_iterator))
		{
			if (other_player->unit_index == NONE && other_player != player)
			{
				if (other_player->multiplayer.time_of_death > player->multiplayer.time_of_death ||
					other_player->multiplayer.time_of_death == player->multiplayer.time_of_death &&
					DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) < DATUM_INDEX_TO_ABSOLUTE_INDEX(player_iterator.index))
				{
					result = FALSE;
				}
			}
		}
	}

	return result;
}

boolean game_engine_player_is_out_of_lives(
	long player_index)
{
	boolean result = FALSE;

	if (global_variant.universal_variant.lives > 0)
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index == NONE && player->statistics.deaths >= global_variant.universal_variant.lives)
		{
			result = TRUE;
		}
	}

	return result;
}

boolean game_engine_should_spawn_player(
	long player_index)
{
	boolean result = FALSE;

	if (game_engine)
	{
		struct player_datum *player = player_get(player_index);

		if (player->quit_out_of_game == TRUE)
		{
			result = FALSE;
		}
		else if (!player->statistics.deaths)
		{
			result = TRUE;
		}
		else if (game_engine_player_is_out_of_lives(player_index))
		{
			result = FALSE;
		}
		else if (game_engine_player_is_odd_man_out(player_index))
		{
			result = FALSE;
		}
		else if (game_engine_globals.mode == game_engine_mode_postgame_rasterize || game_engine_globals.mode == game_engine_mode_postgame_rasterize_delay)
		{
			result = FALSE;
		}
		else if (player->respawn_timer <= 0)
		{
			result = TRUE;
		}
		else
		{
			if (player->local_player_index != NONE)
			{
				if (player->respawn_timer == 3 * TICKS_PER_SECOND)
				{
					game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_for_respawn);
				}
				else if (player->respawn_timer == 2 * TICKS_PER_SECOND)
				{
					game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_for_respawn);
				}
				else if (player->respawn_timer == 1 * TICKS_PER_SECOND)
				{
					game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_for_respawn);
				}
				else if (player->respawn_timer == 1)
				{
					game_engine_play_multiplayer_sound(_multiplayer_sound_respawn);
				}
			}

			result = --player->respawn_timer == 0;
		}

		if (result && game_time_get() > 3 && game_time_get() % 32 != DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) % 32)
		{
			result = FALSE;
		}
	}

	return result;
}

long find_netgame_flags(
	real_point3d const *location,
	real distance,
	real height_delta,
	short type,
	short team,
	long max,
	long *flags)
{
	long count = 0;
	real distance_squared = distance * distance;
	struct scenario *scenario = global_scenario_get();
	short flag_index;

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		if (type != NONE && type != flag->type)
		{
			continue;
		}

		if (team != NONE && team != flag->team_index)
		{
			continue;
		}

		if (location && distance >= 0.f && distance_squared3d(&flag->position, location) > distance_squared)
		{
			continue;
		}

		if (location && height_delta > 0.f && fabs(flag->position.z - location->z) > height_delta)
		{
			continue;
		}

		if (count < max)
		{
			flags[count++] = flag_index;
		}
	}

	return count;
}

long find_netgame_flag(
	real_point3d const *location,
	real distance,
	real height_delta,
	short type,
	short team)
{
	long result = NONE;

	find_netgame_flags(location, distance, height_delta, type, team, 1, &result);

	return result;
}

long game_engine_player_get_team_index(
	long player_index)
{
	long team_index = 1;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 3089, game_engine);

	if (game_engine->game_engine_player_get_team_index)
	{
	}
	else
	{
		struct player_datum *player = player_get(player_index);

		team_index = player->local_player_index % 2;
	}

	return team_index;
}

void game_engine_prespawn_player_update(
	long player_index)
{
	if (game_engine)
	{
		if (game_engine->prespawn_player_update)
		{
			game_engine->prespawn_player_update(player_index);
		}
		else
		{
			struct player_datum *player = player_get(player_index);

			player->team_index = player->local_player_index % 2;
		}
	}

	return;
}

static void handle_custom_starting_equipment(
	long unit_index,
	long *frag_count,
	long *plasma_count)
{
	struct scenario_starting_equipment *starting_equipment;
	long equipment_index;
	struct scenario *scenario = global_scenario_get();

	for (equipment_index = 0; equipment_index < scenario->scenario_starting_equipment.count; equipment_index++)
	{
		starting_equipment = TAG_BLOCK_GET_ELEMENT(&scenario->scenario_starting_equipment, equipment_index, struct scenario_starting_equipment);

		if (match_game_type(game_engine_get_type(), NUMBEROF(starting_equipment->game_type), starting_equipment->game_type))
		{
			long item_collection_index;
			boolean first_weapon = TRUE;

			for (item_collection_index = 0; item_collection_index < NUMBEROF(starting_equipment->item_collection); item_collection_index++)
			{
				if (starting_equipment->item_collection[item_collection_index].index != NONE)
				{
					long weapon_definition_index = random_item(starting_equipment->item_collection[item_collection_index].index);
					long weapon_index;
					struct object_placement_data placement_data;

					object_placement_data_new(&placement_data, weapon_definition_index, NONE);
					weapon_index = object_new(&placement_data);

					if (weapon_index != NONE)
					{
						struct item_datum *item = item_get(weapon_index);

						if (first_weapon || !unit_has_weapon_definition_index(unit_index, item->definition_index))
						{
							unit_add_weapon_to_inventory(unit_index, weapon_index, first_weapon ? _unit_add_weapon_as_only_weapon : _unit_add_weapon_normally);
							first_weapon = FALSE;
						}
						else
						{
							object_delete(weapon_index);
						}
					}
				}
			}

			if (TEST_FLAG(starting_equipment->flags, _netgame_starting_equipment_flag_no_grenades_bit))
			{
				*frag_count = 0;
				*plasma_count = 0;
			}

			if (TEST_FLAG(starting_equipment->flags, _netgame_starting_equipment_flag_plasma_greandes_bit))
			{
				*plasma_count += *frag_count;
				*frag_count = 0;
			}

			break;
		}
	}

	return;
}

void game_engine_postspawn_player_update(
	long player_index)
{
	if (game_engine)
	{
		if (game_engine->postspawn_player_update)
		{
			game_engine->postspawn_player_update(player_index);
		}
		else
		{
			struct player_datum *player = player_get(player_index);
			long unit_index = player->unit_index;
			long frag_count;
			long plasma_count;
			struct game_globals_grenade *frag_grenade = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, _unit_grenade_human_fragmentation, struct game_globals_grenade);
			struct game_globals_grenade *plasma_grenade = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, _unit_grenade_covenant_plasma, struct game_globals_grenade);
			long frag_maximum = frag_grenade->maximum_count;
			long plasma_maximum = plasma_grenade->maximum_count;

			if (TEST_FLAG(game_engine_globals.flags, _game_engine_9_or_more_players_bit))
			{
				frag_maximum = 1;
				plasma_maximum = 1;
			}
			else if (TEST_FLAG(game_engine_globals.flags, _game_engine_5_or_more_players_bit))
			{
				frag_maximum = 2;
				plasma_maximum = 2;
			}

			frag_count = frag_maximum;
			plasma_count = 0;

			if (!TEST_FLAG(global_variant.universal_variant.flags, _generic_starting_equipment_bit))
			{
				handle_custom_starting_equipment(unit_index, &frag_count, &plasma_count);
			}

			if (game_engine_infinite_grenades_internal())
			{
				frag_count = frag_maximum;
				plasma_count = plasma_maximum;
			}

			if (unit_index != NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);

				switch (global_variant.universal_variant.weapon_set)
				{
				case _game_engine_weapons_plasma_weapons:
					plasma_count += frag_count;
					frag_count = 0;
					break;
				case _game_engine_weapons_human:
					frag_count += plasma_count;
					plasma_count = 0;
					break;
				case _game_engine_weapons_no_grenades:
					if (!game_engine_infinite_grenades_internal())
					{
						frag_count = 0;
						plasma_count = 0;
					}
					break;
				}

				frag_count = MIN(frag_count, frag_maximum);
				plasma_count = MIN(plasma_count, plasma_maximum);

				unit->unit.grenade_counts[_unit_grenade_human_fragmentation] = (char)frag_count;
				unit->unit.grenade_counts[_unit_grenade_covenant_plasma] = (char)plasma_count;
			}
		}
	}

	return;
}

boolean game_engine_running(
	void)
{
	boolean running = game_engine != NULL;

	return running;
}

real game_engine_get_damage_multiplier(
	long attacker_index,
	long defender_index)
{
	real multiplier = 1.f;

	if (game_engine)
	{
		multiplier = 1.f / PIN(global_variant.universal_variant.health, 0.25f, 4.f);
	}

	if (attacker_index != NONE && defender_index != NONE && game_engine)
	{
		if (game_engine_test_trait(attacker_index, _game_trait_extra_damage))
		{
			multiplier *= 1.5f;
		}

		if (game_engine_test_trait(defender_index, _game_trait_damage_resistant))
		{
			multiplier *= 0.5f;
		}
	}

	return multiplier;
}

boolean game_engine_can_score(
	void)
{
	boolean result = TRUE;

	if (game_engine)
	{
		result = game_engine_globals.mode == game_engine_mode_active;
	}

	return result;
}

boolean game_engine_force_single_screen(
	void)
{
	boolean result = FALSE;

	if (game_engine &&
		game_engine_globals.mode >= game_engine_mode_postgame_rasterize_delay &&
		game_engine_globals.mode <= game_engine_mode_postgame_rasterize)
	{
		result = TRUE;
	}

	return result;
}

static void game_engine_update_teleporter(
	long player_index)
{
	static long blocked_message_delay = 0;
	static short fade_function = 0;
	static short screen_flash_type = _screen_flash_type_tint;
	static real max_intensity = 1.f;
	static real alpha = 0.5f;
	static real red = 0.35f;
	static real green = 1.f;
	static real blue = 0.35f;
	static real duration = 1.f;
	struct scenario *scenario = global_scenario_get();
	struct player_datum *player = player_get(player_index);
	real trigger_radius = 0.5f;
	real reset_radius = 1.f;

	if (player->unit_index == NONE)
	{
		goto exit;
	}

	{
		long source_flag_index;
		struct unit_datum *unit = unit_get(player->unit_index);

		if (player->multiplayer.teleporter_index != NONE)
		{
			struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, player->multiplayer.teleporter_index, struct scenario_netgame_flag);

			if (distance_squared3d(&flag->position, &unit->object.position) > reset_radius * reset_radius)
			{
				player->multiplayer.teleporter_index = NONE;
			}
		}

		source_flag_index = find_netgame_flag(&unit->object.position, trigger_radius, 0.f, _netgame_flag_teleporter_source, NONE);

		if (source_flag_index != NONE && source_flag_index != player->multiplayer.teleporter_index)
		{
			struct scenario_netgame_flag *source_flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, source_flag_index, struct scenario_netgame_flag);
			long destination_flag_index = find_netgame_flag(NULL, 0.f, 0.f, _netgame_flag_teleporter_target, source_flag->team_index);

			if (destination_flag_index != NONE)
			{
				struct scenario_netgame_flag *destination_flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, destination_flag_index, struct scenario_netgame_flag);
				struct unit_datum *teleported_unit = unit_get(player->unit_index);
				real_vector3d forward = teleported_unit->object.forward;
				struct player_datum *teleported_player = player_get(player_index);

				{
					struct collision_feature_list features;
					real our_width;
					real our_height;
					real_point3d our_pos;

					biped_get_physics_pill(teleported_player->unit_index, &our_pos, &our_height, &our_width);
					our_pos = destination_flag->position;

					if (collision_get_features_in_sphere(TELEPORTER_COLLISION_TEST_FLAGS, &our_pos, 2.f * our_width + our_height, our_height, our_width, NONE, &features))
					{
						struct collision_plane coll_plane;

						if (collision_features_test_point(&features, &our_pos, &coll_plane))
						{
							if (coll_plane.object_index != NONE && TEST_FLAG(_object_mask_unit, object_get(coll_plane.object_index)->object.type))
							{
								long blocking_player_index = unit_get(coll_plane.object_index)->unit.player_index;

								if (blocking_player_index != NONE)
								{
									struct player_datum *blocking_player = player_get(blocking_player_index);

									blocking_player->is_blocking_teleporter = TRUE;
									blocking_player->telefrag_timeout++;
								}
							}

							if (blocked_message_delay > 0)
							{
								blocked_message_delay--;
							}
							else
							{
								wchar_t *string;

								blocked_message_delay = 120;
								multiplayer_game_text_get_string(string, _string_teleporter_is_blocked);
								hud_print_message(unit_get_local_player_index(teleported_player->unit_index), string);
							}

							goto exit;
						}
					}
				}

				{
					real angle;

					if (player->local_player_index != NONE)
					{
						game_engine_play_multiplayer_sound(_multiplayer_sound_teleporter_activate);
					}

					if (player->local_player_index != NONE)
					{
						struct screen_flash_definition screen_flash = { 0 };

						screen_flash.fade_function = fade_function;
						screen_flash.type = screen_flash_type;
						screen_flash.priority = _screen_flash_high_priority;
						screen_flash.duration = duration;
						screen_flash.max_intensity = max_intensity;
						screen_flash.zero_scale_factor = 0.f;
						screen_flash.screen_flash_color.alpha = alpha;
						screen_flash.screen_flash_color.red = red;
						screen_flash.screen_flash_color.green = green;
						screen_flash.screen_flash_color.blue = blue;
						player_effect_screen_flash(player_index, &screen_flash, 1.f);
					}

					angle = arctangent(forward.j, forward.i) + destination_flag->facing - source_flag->facing;
					forward.i = cosine(angle);
					forward.j = sine(angle);
					normalize3d(&forward);
					object_set_position(player->unit_index, &destination_flag->position, &forward, NULL);

					if (player->local_player_index != NONE)
					{
						player_control_set_facing(player->local_player_index, &forward);
					}

					player->multiplayer.teleporter_index = find_netgame_flag(&teleported_unit->object.position, reset_radius, 0.f, _netgame_flag_teleporter_source, NONE);
				}
			}
			else
			{
				console_printf(FALSE, "failed to teleport %d", source_flag->team_index);
			}
		}
	}

exit:
	return;
}

real get_blink_alpha(
	void)
{
	long period = 2700;
	long time = system_milliseconds() % period;
	real angle = (real)(time * 3.14159265358979 * (1.f / period)); // M_PI?
	real alpha = (real)sin(angle);

	return alpha;
}

static void game_engine_press_start_to_begin(
	void)
{
	return;
}

void game_engine_nonplayer_post_rasterize(
	void)
{
	if (game_engine)
	{
		switch (game_engine_globals.mode)
		{
		case game_engine_mode_active:
		case game_engine_mode_postgame_delay:
			game_engine_press_start_to_begin();
			break;
		case game_engine_mode_postgame_rasterize_delay:
		case game_engine_mode_postgame_rasterize:
		{
			rectangle2d window;
			long local_player_index;

			game_engine_post_rasterize_post_game();
			window.x0 = 0;
			window.x1 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
			window.y0 = 0;
			window.y1 = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;

			for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
			{
				render_ui_widgets_postgame((short)local_player_index, &window);
				rumble_player_clear((short)local_player_index);
			}
			break;
		}
		default:
			match_unreachable("c:\\halo\\SOURCE\\game\\game_engine.c", 3643);
			break;
		}
	}

	return;
}

real game_engine_get_distance_rating_for_spawn(
	long player_index,
	struct scenario_player *starting_location)
{
	boolean teams = game_engine_running() ? global_variant.universal_variant.teams : FALSE;
	struct player_datum *player = player_get(player_index);
	real rating = 1.f;
	real minimum_distance = 0.25f;
	real nearby_distance = 1.f;
	real minimum_enemy_distance = 2.f;
	real maximum_enemy_distance = 5.f;
	struct data_iterator iterator;
	struct player_datum *other_player;

	data_iterator_new(&iterator, player_data);

	while (other_player = (struct player_datum *)data_iterator_next(&iterator))
	{
		if (other_player->unit_index != NONE)
		{
			real_point3d origin;
			real distance;

			object_get_origin(other_player->unit_index, &origin);
			distance = distance3d(&origin, &starting_location->position);

			if (teams && other_player->team_index == player->team_index && distance > minimum_distance)
			{
				continue;
			}

			if (distance < minimum_distance)
			{
				rating = 0.f;
			}
			else if (distance < nearby_distance)
			{
				rating *= 0.1f;
			}

			if (other_player->team_index != player->team_index)
			{
				if (distance < minimum_enemy_distance)
				{
					rating = 0.f;
				}
				else if (distance > maximum_enemy_distance)
				{
					rating *= 1.f;
				}
				else
				{
					rating = (distance - minimum_enemy_distance) / (maximum_enemy_distance - minimum_enemy_distance) * rating;
				}
			}
		}
	}

	return rating;
}

static real game_engine_get_friendly_bonus(
	long player_index,
	struct scenario_player *starting_location)
{
	struct player_datum *player = player_get(player_index);
	real rating = 1.f;
	struct data_iterator iterator;
	struct player_datum *other_player;
	real bonus = 0.f;

	data_iterator_new(&iterator, player_data);

	while (other_player = (struct player_datum *)data_iterator_next(&iterator))
	{
		if (player->team_index == other_player->team_index && other_player->unit_index != NONE)
		{
			real minimum_distance = 1.f;
			real maximum_distance = 6.f;
			real_point3d origin;
			real distance;

			object_get_origin(other_player->unit_index, &origin);
			distance = distance3d(&origin, &starting_location->position);

			if (distance >= minimum_distance && distance <= maximum_distance)
			{
				bonus += (real)pow(1.f - (distance - minimum_distance) / (maximum_distance - minimum_distance), 0.6f);
			}
		}
	}

	bonus = MIN(bonus, 3.f);
	bonus *= 3.f;
	bonus += 1.f;
	rating *= bonus;

	return rating;
}

static real default_starting_location_rate_function(
	long player_index,
	struct scenario_player *starting_location)
{
	struct player_datum *player = player_get(player_index);
	real rating = 1.f;

	if (game_engine_running() && game_engine_test_flag(_game_engine_spawn_point_obeys_teams_bit) && player->team_index != starting_location->team_index)
	{
		rating = 0.f;
	}

	if (rating > 0.f)
	{
		rating *= game_engine_get_distance_rating_for_spawn(player_index, starting_location);
	}

	if (game_engine_running() && rating > 0.f && game_engine_has_teams())
	{
		rating *= game_engine_get_friendly_bonus(player_index, starting_location);
	}

	if (game_engine_running() && game_engine->starting_location_rating)
	{
		rating *= game_engine->starting_location_rating(player_index, starting_location);
	}

	return rating;
}

static boolean nearby_vehicle(
	long player_index,
	struct scenario_player *starting_location)
{
	long object_indices[16];
	struct location location;
	short object_count;
	short object_index;
	struct player_datum *player = player_get(player_index);
	unsigned long object_mask = _object_mask_unit | _object_mask_item | _object_mask_control;

	scenario_location_from_point(&location, &starting_location->position);
	object_count = objects_in_sphere(0, object_mask, &location, &starting_location->position, 0.1f, object_indices, NUMBEROF(object_indices));

	for (object_index = 0; object_index < object_count; object_index++)
	{
		struct object_datum *object = object_get(object_indices[object_index]);

		match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 3817, object);

		if (object->object.type == _object_type_vehicle)
		{
			struct vehicle_datum *vehicle = vehicle_get(object_indices[object_index]);

			match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 3822, vehicle);

			if (vehicle)
			{
				return TRUE;
			}

			break;
		}
	}

	return FALSE;
}

real game_engine_get_starting_location_rating(
	long player_index,
	struct scenario_player *starting_location)
{
	real rating;
	boolean valid_game_type = match_game_type(game_engine_get_type(), NUMBEROF(starting_location->game_type), starting_location->game_type);

	if (!valid_game_type)
	{
		rating = 0.f;
	}
	else if (nearby_vehicle(player_index, starting_location))
	{
		rating = 0.f;
	}
	else
	{
		rating = default_starting_location_rate_function(player_index, starting_location);
	}

	return rating;
}

void game_engine_rasterize_message(
	wchar_t const *string,
	real alpha)
{
	real_argb_color color;
	long font_index = local_player_count() ? hud_globals->messaging.multi_player_font.index : hud_globals->messaging.single_player_font.index;
	rectangle2d bounds = render.camera.window_bounds;
	long terminal_font_index = interface_get_tag_index(_interface_font_terminal);

	draw_string_set_draw_mode(terminal_font_index, NONE, 0, 0, global_real_argb_white);
	color.alpha = alpha;
	color.red = 117.f / 255.f;
	color.green = 186.f / 255.f;
	color.blue = 1.f;
	offset_rectangle2d(&bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
	bounds.y1 = (5 * bounds.y0 + bounds.y1) / 6 + 9;
	bounds.y0 = bounds.y1 - 15;
	draw_string_set_draw_mode(font_index, NONE, 2, 8, &color);
	draw_string_set_color(&color);
	rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, string);
	draw_string_set_format(NONE, 0, 0);
	draw_string_set_tab_stops(NULL, 0);

	return;
}

boolean game_engine_picking_up(
	long unit_index,
	long item_index)
{
	boolean allow_pick_up = TRUE;

	if (game_engine)
	{
		struct weapon_datum *weapon = weapon_try_and_get(item_index);

		if (weapon && weapon_is_flag(item_index))
		{
			if (TEST_FLAG(weapon->weapon.flags, _weapon_multiplayer_inventory_flag))
			{
				SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_inventory_flag, FALSE);

				if (game_engine->weapon_drop)
				{
					game_engine->weapon_drop(item_index);
				}
			}

			SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_inventory_flag, TRUE);

			if (game_engine->weapon_pickup)
			{
				long player_index = player_index_from_unit_index(unit_index);

				allow_pick_up = game_engine->weapon_pickup(item_index, player_index);
			}

			match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 3928, !allow_pick_up || !TEST_FLAG(weapon->weapon.flags, _weapon_must_be_readied_bit) || !unit_has_weapon_with_flag(unit_index, _weapon_must_be_readied_bit));
		}
	}

	return allow_pick_up;
}

static boolean goal_matches_player(
	struct player_datum *player,
	long player_index,
	long goal_index)
{
	boolean result = FALSE;

	if (game_engine_running())
	{
		struct netgame_goal *goal = &global_goal[goal_index];

		if (game_engine->goal_matches_player)
		{
			if (goal->in_use)
			{
				result = game_engine->goal_matches_player(player_index, goal_index);
			}
		}
		else if (goal->in_use)
		{
			if ((goal->player_index == NONE || player_index == goal->player_index) &&
				(goal->team_index == NONE || player->team_index == goal->team_index) &&
				(goal->ignore_player_index == NONE || player_index != goal->ignore_player_index))
			{
				result = TRUE;
			}
		}
	}

	return result;
}

short game_engine_player_get_custom_motion_sensor_positions(
	long player_index,
	real_point2d *points,
	char *goal_indices,
	short max_points)
{
	short count = 0;

	if (game_engine && global_variant.universal_variant.goal_radar == _radar_motion_tracker && player_index != NONE)
	{
		long goal_index;
		struct player_datum *player = player_get(player_index);

		for (goal_index = 0; goal_index < MAXIMUM_NETGAME_GOALS; goal_index++)
		{
			if (goal_matches_player(player, player_index, goal_index) && count < max_points)
			{
				goal_indices[count] = (char)goal_index;
				points[count] = *(real_point2d *)&global_goal[goal_index].position;
				count++;
			}
		}
	}

	return count;
}

long get_flag_definition_index(
	void)
{
	struct scenario *scenario = global_scenario_get();
	struct game_globals *globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *multiplayer = TAG_BLOCK_GET_ELEMENT(&globals->multiplayer_information, 0, struct game_globals_multiplayer_information);

	return multiplayer->flag.index;
}

long get_ball_definition_index(
	void)
{
	struct scenario *scenario = global_scenario_get();
	struct game_globals *globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *multiplayer = TAG_BLOCK_GET_ELEMENT(&globals->multiplayer_information, 0, struct game_globals_multiplayer_information);

	return multiplayer->ball.index;
}

void game_engine_switch_to_postgame(
	void)
{
	if (game_engine_globals.mode == game_engine_mode_active)
	{
		if (global_network_game_server_get())
		{
			game_engine_globals.mode = game_engine_mode_postgame_delay;
			game_engine_globals.game_ending_timer = 7.f;
		}
		else
		{
			game_engine_globals.mode = game_engine_mode_postgame_rasterize;
		}
	}

	return;
}

struct game_variant *game_engine_get_variant(
	void)
{
	return &global_variant;
}

boolean game_engine_get_goal_in_use(
	short index)
{
	return global_goal[index].in_use;
}

real_point3d game_engine_get_goal_position(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 4066, global_goal[index].in_use);

	return global_goal[index].position;
}

void game_engine_set_goal_position(
	short index,
	real_point3d const *position,
	real vertical_offset,
	char const *descriptor,
	long player_index,
	short team_index,
	long ignore_player_index)
{
	real height_offset = 0.63f;

	global_goal[index].ignore_player_index = ignore_player_index;
	global_goal[index].nav_index = find_nav_point(descriptor);
	global_goal[index].in_use = TRUE;
	global_goal[index].position = *position;
	global_goal[index].position.z = height_offset + vertical_offset + global_goal[index].position.z;
	global_goal[index].team_index = team_index;
	global_goal[index].player_index = player_index;

	return;
}

void game_engine_clear_goal_position(
	short index)
{
	memset(&global_goal[index], 0, sizeof(global_goal[index]));

	return;
}

struct game_variant *global_autogenerate_list = NULL;
long global_autogenerate_count = 0;

struct game_variant game_engine_get_variant_by_name(
	char const *name)
{
	struct game_variant result;

	memset(&result, 0, sizeof(result));

	if (!strcmp(name, "race"))
	{
		result = build_game_variant_race();
	}
	else if (!strcmp(name, "team_race"))
	{
		result = build_game_variant_team_race();
	}
	else if (!strcmp(name, "rally"))
	{
		result = build_game_variant_rally();
	}
	else if (!strcmp(name, "slayer"))
	{
		result = build_game_variant_slayer();
	}
	else if (!strcmp(name, "team_slayer"))
	{
		result = build_game_variant_team_slayer();
	}
	else if (!strcmp(name, "elimination"))
	{
		result = build_game_variant_elimination();
	}
	else if (!strcmp(name, "stalker"))
	{
		result = build_game_variant_stalker();
	}
	else if (!strcmp(name, "team_oddball"))
	{
		result = build_game_variant_team_oddball();
	}
	else if (!strcmp(name, "accumulation"))
	{
		result = build_game_variant_accumulation();
	}
	else if (!strcmp(name, "oddball"))
	{
		result = build_game_variant_oddball();
	}
	else if (!strcmp(name, "ctf"))
	{
		result = build_game_variant_ctf();
	}
	else if (!strcmp(name, "ironctf"))
	{
		result = build_game_variant_iron_ctf();
	}
	else if (!strcmp(name, "king"))
	{
		result = build_game_variant_king();
	}
	else if (!strcmp(name, "team_king"))
	{
		result = build_game_variant_team_king();
	}

	return result;
}

void game_engine_render_nav_points(
	short local_player_index)
{
	if (game_engine &&
		global_variant.universal_variant.goal_radar == _radar_nav_point &&
		local_player_index != NONE)
	{
		long player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE)
		{
			struct player_datum *player = player_get(player_index);

			if (player->unit_index != NONE)
			{
				real_point3d head_position;
				long goal_index;

				unit_get_head_position(player->unit_index, &head_position);

				for (goal_index = 0; goal_index < MAXIMUM_NETGAME_GOALS; goal_index++)
				{
					if (goal_matches_player(player, player_index, goal_index))
					{
						long reference_object_index = NONE;

						custom_render_nav_point(
							local_player_index,
							&global_goal[goal_index].position,
							global_goal[goal_index].nav_index,
							hud_get_nav_point_render_type(local_player_index, &head_position, &global_goal[goal_index].position, reference_object_index));
					}
				}
			}
		}
	}

	return;
}

static boolean game_engine_infinite_grenades_internal(
	void)
{
	boolean result = FALSE;

	if (!TEST_FLAG(game_engine_globals.flags, _game_engine_5_or_more_players_bit))
	{
		result = TEST_FLAG(global_variant.universal_variant.flags, _infinite_grenades_bit);
	}

	return result;
}

boolean game_engine_infinite_grenades(
	long player_index)
{
	boolean result = FALSE;

	if (game_engine_running() && player_index != NONE)
	{
		result = game_engine_infinite_grenades_internal();
	}

	return result;
}

boolean game_engine_has_teams(
	void)
{
	boolean result = FALSE;

	if (game_engine_running())
	{
		result = global_variant.universal_variant.teams;
	}

	return result;
}

boolean game_engine_display_team_indicators(
	void)
{
	boolean result = FALSE;

	if (game_engine_running())
	{
		result = TEST_FLAG(global_variant.universal_variant.flags, _allow_friendly_navpoints_bit) && global_variant.universal_variant.teams;
	}

	return result;
}

boolean game_engine_has_shield(
	long player_index)
{
	boolean result = TRUE;

	if (game_engine_running() && player_index != NONE)
	{
		result = !TEST_FLAG(global_variant.universal_variant.flags, _no_shields_bit);
	}

	return result;
}

long weapon_definition_index_to_list_index(
	long definition_index)
{
	long list_index;
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_tag_reference *weapons = game_globals->weapon_list.count ? TAG_BLOCK_GET_ELEMENT(&game_globals->weapon_list, 0, struct game_globals_tag_reference) : NULL;
	long result = NONE;

	for (list_index = 0; list_index < game_globals->weapon_list.count; list_index++)
	{
		if (definition_index == weapons[list_index].reference.index)
		{
			result = list_index;
			break;
		}
	}

	return result;
}

long list_index_to_weapon_definition_index(
	long list_index)
{
	long result = NONE;

	if (list_index != NONE)
	{
		result = game_globals_get_weapon(scenario_get_game_globals(), list_index);
	}

	return result;
}

long game_engine_remap_vehicle(
	long vehicle_definition_index)
{
	long result = vehicle_definition_index;

	if (game_engine)
	{
		struct tag_reference *vehicle;
		struct game_globals *game_globals = scenario_get_game_globals();
		struct game_globals_multiplayer_information *multiplayer_information = TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information);
		struct tag_reference *warthog = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_warthog, struct tag_reference);
		struct tag_reference *ghost = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_ghost, struct tag_reference);
		struct tag_reference *scorpion = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_scorpion, struct tag_reference);

		if (vehicle_definition_index != warthog->index && vehicle_definition_index != ghost->index && vehicle_definition_index != scorpion->index)
		{
			result = NONE;
		}

		switch (global_variant.universal_variant.vehicle_set)
		{
		case _game_engine_vehicles_none:
			result = NONE;
			break;
		case _game_engine_vehicles_warthog:
			vehicle = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_warthog, struct tag_reference);

			if (vehicle->index != result)
			{
				result = NONE;
			}
			break;
		case _game_engine_vehicles_ghost:
			vehicle = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_ghost, struct tag_reference);

			if (vehicle->index != result)
			{
				result = NONE;
			}
			break;
		case _game_engine_vehicles_tank:
			vehicle = TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, _multiplayer_vehicle_scorpion, struct tag_reference);

			if (vehicle->index != result)
			{
				result = NONE;
			}
			break;
		}
	}

	return result;
}

long game_engine_remap_equipment(
	long equipment_definition_index)
{
	long result;
	struct equipment_definition *definition = equipment_definition_index == NONE ? NULL : equipment_definition_get(equipment_definition_index);
	long list_index = weapon_definition_index_to_list_index(equipment_definition_index);

	if (list_index == _weapon_list_frag_grenade || list_index == _weapon_list_plasma_grenade)
	{
		switch (global_variant.universal_variant.weapon_set)
		{
		case _game_engine_weapons_plasma_weapons:
			list_index = _weapon_list_plasma_grenade;
			break;
		case _game_engine_weapons_human:
			list_index = _weapon_list_frag_grenade;
			break;
		case _game_engine_weapons_no_grenades:
			list_index = NONE;
			break;
		}

		if (game_engine_infinite_grenades_internal())
		{
			list_index = NONE;
		}

		if (TEST_FLAG(game_engine_globals.flags, _game_engine_9_or_more_players_bit))
		{
			if (real_random() > 0.3f)
			{
				list_index = NONE;
			}
		}
		else if (TEST_FLAG(game_engine_globals.flags, _game_engine_5_or_more_players_bit))
		{
			if (real_random() > 0.55f)
			{
				list_index = NONE;
			}
		}

		result = list_index_to_weapon_definition_index(list_index);
	}
	else
	{
		result = equipment_definition_index;

		if (definition)
		{
			if (definition->equipment.powerup_type == _equipment_powerup_over_shield)
			{
				if (TEST_FLAG(global_variant.universal_variant.flags, _no_shields_bit))
				{
					result = NONE;
				}
			}
			else if (definition->equipment.powerup_type == _equipment_powerup_active_camouflage)
			{
				if (TEST_FLAG(global_variant.universal_variant.flags, _always_invisible_bit))
				{
					result = NONE;
				}
			}
		}
	}

	return result;
}

long game_engine_remap_weapon(
	long weapon_definition_index)
{
	long result;
	long list_index = weapon_definition_index_to_list_index(weapon_definition_index);

	if (list_index == _weapon_list_ball || list_index == _weapon_list_flag || list_index == NONE)
	{
		result = weapon_definition_index;
	}
	else
	{
		if (list_index == _weapon_list_flamethrower)
		{
			list_index = _weapon_list_rocket_launcher;
		}

		switch (global_variant.universal_variant.weapon_set)
		{
		case _game_engine_weapons_pistols:
			switch (list_index)
			{
			case _weapon_list_needler:
			case _weapon_list_pistol:
			case _weapon_list_plasma_rifle:
			case _weapon_list_rocket_launcher:
				list_index = _weapon_list_plasma_pistol;
				break;
			default:
				list_index = _weapon_list_pistol;
				break;
			}
			break;
		case _game_engine_weapons_assault_rifles:
			switch (list_index)
			{
			case _weapon_list_needler:
			case _weapon_list_pistol:
			case _weapon_list_plasma_rifle:
			case _weapon_list_rocket_launcher:
				list_index = _weapon_list_plasma_rifle;
				break;
			default:
				list_index = _weapon_list_assault_rifle;
				break;
			}
			break;
		case _game_engine_weapons_plasma_weapons:
			if (list_index < _weapon_list_needler || list_index > _weapon_list_plasma_pistol)
			{
				list_index = _weapon_list_plasma_rifle;
			}
			else
			{
				list_index = _weapon_list_plasma_pistol;
			}
			break;
		case _game_engine_weapons_sniping:
			if (list_index != _weapon_list_pistol && list_index != _weapon_list_sniper_rifle)
			{
				list_index = _weapon_list_sniper_rifle;
			}
			break;
		case _game_engine_weapons_no_sniping:
			if (list_index == _weapon_list_pistol)
			{
				list_index = _weapon_list_assault_rifle;
			}
			else if (list_index == _weapon_list_sniper_rifle)
			{
				list_index = _weapon_list_shotgun;
			}
			break;
		case _game_engine_weapons_rocket_launchers:
			list_index = _weapon_list_rocket_launcher;
			break;
		case _game_engine_weapons_shotguns:
			list_index = _weapon_list_shotgun;
			break;
		case _game_engine_weapons_short_range:
			switch (list_index)
			{
			case _weapon_list_assault_rifle:
			case _weapon_list_needler:
			case _weapon_list_pistol:
			case _weapon_list_sniper_rifle:
				list_index = _weapon_list_shotgun;
				break;
			}
			break;
		case _game_engine_weapons_human:
			switch (list_index)
			{
			case _weapon_list_needler:
				list_index = _weapon_list_assault_rifle;
				break;
			case _weapon_list_plasma_pistol:
				list_index = _weapon_list_pistol;
				break;
			case _weapon_list_plasma_rifle:
				list_index = _weapon_list_assault_rifle;
				break;
			}
			break;
		default:
			break;
		}

		result = list_index_to_weapon_definition_index(list_index);
	}

	return result;
}

long game_engine_remap_object_definition(
	long definition_index)
{
	long result = definition_index;

	if (game_engine_running() && definition_index != NONE)
	{
		short object_type = object_definition_get(definition_index)->object.type;

		if (object_type == _object_type_vehicle)
		{
			result = game_engine_remap_vehicle(definition_index);
		}
		else if (object_type == _object_type_weapon)
		{
			result = game_engine_remap_weapon(definition_index);
		}
		else if (object_type == _object_type_equipment)
		{
			result = game_engine_remap_equipment(definition_index);
		}
	}

	return result;
}

boolean game_engine_man_out(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	boolean result = player->quit_out_of_game ||
		game_engine_player_is_out_of_lives(player_index) ||
		game_engine_player_is_odd_man_out(player_index);

	return result;
}

boolean game_engine_get_state_message(
	long player_index,
	wchar_t *buffer,
	long buffer_length)
{
	boolean result = FALSE;

	if (game_engine_running())
	{
		struct player_datum *player = player_get(player_index);

		switch (player->multiplayer.state_message)
		{
		case game_engine_message_odd_man_out:
		case game_engine_message_out_of_lives:
		case game_engine_message_respawn_timer:
		case game_engine_message_waiting_for_space_to_clear:
			player->multiplayer.state_message = NONE;
			break;
		}

		if (player->unit_index == NONE)
		{
			long message;
			long message_data = 0;

			if (player->quit_out_of_game == TRUE)
			{
				message = game_engine_message_player_quit_self;
			}
			else if (game_engine_player_is_out_of_lives(player_index))
			{
				message = game_engine_message_out_of_lives;
			}
			else if (game_engine_player_is_odd_man_out(player_index))
			{
				message = game_engine_message_odd_man_out;
			}
			else if (player->respawn_timer > 0)
			{
				message = game_engine_message_respawn_timer;
				message_data = player->respawn_timer / TICKS_PER_SECOND;
			}
			else
			{
				message = game_engine_message_waiting_for_space_to_clear;
			}

			result = multiplayer_message_internal(player_index, message, message_data, buffer, buffer_length);
		}
		else if (game_time_get() < 15 * TICKS_PER_SECOND)
		{
			result = multiplayer_message_internal(player_index, game_engine_message_press_back_for_score, NONE, buffer, buffer_length);
		}
		else if (player->multiplayer.state_message != NONE)
		{
			result = multiplayer_message_internal(player_index, player->multiplayer.state_message, player->multiplayer.state_message_data, buffer, buffer_length);
		}
	}

	return result;
}

void game_engine_state_message(
	long player_index,
	long message,
	long message_data)
{
	struct player_datum *player = player_get(player_index);

	player->multiplayer.state_message = message;
	player->multiplayer.state_message_data = message_data;

	return;
}

real_rgb_color game_engine_player_get_change_color(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	real_rgb_color color;

	if (global_variant.universal_variant.teams)
	{
		if (player->team_index == 0)
		{
			color = *global_real_rgb_red;
		}
		else
		{
			color = *global_real_rgb_blue;
		}
	}
	else
	{
		long color_index = player->network_player_data.primary_color_index;

		if (debug_player_color != NONE)
		{
			color_index = debug_player_color;
		}

		color = player_profile_get_rgb_color(color_index);
	}

	return color;
}

boolean game_engine_player_has_flag(
	long player_index)
{
	boolean result = FALSE;

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		long unit_index = player->unit_index;

		if (unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);
			long index;

			for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
			{
				long weapon_index = unit->unit.weapon_object_indices[index];

				if (weapon_index != NONE && weapon_is_flag(weapon_index))
				{
					result = TRUE;
					break;
				}
			}
		}
	}

	return result;
}

void game_engine_player_depower_active_camo(
	long player_index)
{
	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);

			if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
			{
				unit->unit.active_camouflage = 0.5f;
			}
		}
	}

	return;
}

wchar_t *get_place_name(
	struct game_engine_place place)
{
	enum
	{
		maximum_places = 16,
		_place_name_even = 2 * maximum_places,
		_place_name_winning,
		_place_name_losing,
		_place_name_tied
	};

	wchar_t *string;
	long lookup_index;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 4886, place.place < maximum_places);

	if (TEST_FLAG(place.flags, _place_two_groups) && TEST_FLAG(place.flags, _place_tied))
	{
		lookup_index = _place_name_tied;
	}
	else if (TEST_FLAG(place.flags, _place_two_groups) && place.place == 0)
	{
		lookup_index = _place_name_winning;
	}
	else if (TEST_FLAG(place.flags, _place_two_groups) && place.place == 1)
	{
		lookup_index = _place_name_losing;
	}
	else if (TEST_FLAG(place.flags, _place_all_tied))
	{
		lookup_index = _place_name_even;
	}
	else
	{
		lookup_index = place.place;

		if (TEST_FLAG(place.flags, _place_tied))
		{
			lookup_index += maximum_places;
		}
	}

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 4913, NONE != lookup_index);

	multiplayer_game_text_get_string(string, lookup_index + _string_first_place);

	return string;
}

struct game_engine_place game_engine_get_place(
	long player_index,
	enum get_score_type get_score_type)
{
	struct game_engine_place place;
	struct player_datum *player = player_get(player_index);
	boolean all_tied = TRUE;
	boolean tied = FALSE;
	long group_count = 1;

	place.place = 0;
	place.flags = 0;

	if (game_engine->get_score)
	{
		struct data_iterator player_iterator;
		struct player_datum *other_player;
		unsigned long team_mask = 0;
		long score = game_engine->get_score(player_index, get_score_type);

		data_iterator_new(&player_iterator, player_data);

		while (other_player = (struct player_datum *)data_iterator_next(&player_iterator))
		{
			boolean different_group;

			if (get_score_type == _get_score_team)
			{
				different_group = other_player->team_index != player->team_index;
			}
			else
			{
				different_group = player_iterator.index != player_index;
			}

			if (different_group && get_score_type == _get_score_team)
			{
				if (!TEST_FLAG(team_mask, other_player->team_index))
				{
					SET_FLAG(team_mask, other_player->team_index, TRUE);
					different_group = TRUE;
				}
				else
				{
					different_group = FALSE;
				}
			}

			if (different_group)
			{
				long other_score = game_engine->get_score(player_iterator.index, get_score_type);

				group_count++;

				if (other_score != score)
				{
					all_tied = FALSE;
				}

				if (other_score > score)
				{
					place.place++;
				}
				else if (other_score == score)
				{
					tied = TRUE;
				}
			}
		}
	}

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 4983, (!all_tied || (tied)) || (1 == group_count));

	all_tied &= tied;
	SET_FLAG(place.flags, _place_team, get_score_type == _get_score_team);
	SET_FLAG(place.flags, _place_tied, tied);
	SET_FLAG(place.flags, _place_all_tied, all_tied);
	SET_FLAG(place.flags, _place_two_groups, group_count == 2);

	return place;
}

boolean game_engine_hud_draw_motion_sensor(
	long player_index)
{
	boolean result = TRUE;

	if (game_engine_running())
	{
		boolean goal_radar;

		result = TEST_FLAG(global_variant.universal_variant.flags, _allow_motion_tracker_bit);
		goal_radar = global_variant.universal_variant.goal_radar == _radar_motion_tracker;

		if (global_variant.game_engine_index == _game_engine_slayer && !global_variant.game_engine_variant.slayer.kill_in_order)
		{
			goal_radar = FALSE;
		}

		result |= goal_radar;
	}

	return result;
}

boolean game_engine_test_flag(
	long flag)
{
	boolean result = FALSE;

	if (game_engine_running() && game_engine->game_engine_test_flag)
	{
		result = game_engine->game_engine_test_flag(flag);
	}

	return result;
}

boolean game_engine_test_trait(
	long player_index,
	long trait)
{
	boolean result = FALSE;

	if (game_engine_running() && game_engine->game_engine_test_trait)
	{
		result = game_engine->game_engine_test_trait(player_index, trait);
	}

	return result;
}

long game_engine_did_player_win_default(
	long player_index)
{
	long result = 0;

	if (global_variant.universal_variant.teams)
	{
		long winning_team_index;
		long red_score = game_engine_get_team_score(0);
		long blue_score = game_engine_get_team_score(1);
		struct player_datum *player = player_get(player_index);

		if (multiple_teams_alive())
		{
			winning_team_index = (red_score == blue_score) ? NONE : (red_score <= blue_score);
		}
		else
		{
			winning_team_index = !team_has_players(0);
		}

		if (winning_team_index != NONE)
		{
			result = player->team_index == winning_team_index;
		}
		else
		{
			result = NONE;
		}
	}
	else
	{
		struct statistic_buffer place = game_engine_get_player_place(player_index);

		if (!is_place_tied(&place) || place_get_position(&place) != 0)
		{
			result = place_get_position(&place) == 0;
		}
		else
		{
			result = NONE;
		}
	}

	return result;
}

long game_engine_did_player_win(
	long player_index)
{
	long result = 0;

	if (game_engine_running())
	{
		if (game_engine->game_engine_did_player_win)
		{
			result = game_engine->game_engine_did_player_win(player_index);
		}
		else
		{
			result = game_engine_did_player_win_default(player_index);
		}
	}

	return result;
}

static long game_engine_did_team_win(
	long team_index)
{
	struct data_iterator player_iterator;
	struct player_datum *player;
	long result = 0;

	data_iterator_new(&player_iterator, player_data);

	while (player = data_iterator_next(&player_iterator))
	{
		if (player->team_index == team_index)
		{
			result = game_engine_did_player_win(player_iterator.index);
			break;
		}
	}

	return result;
}

static void netgame_flag_verify_team_exists(
	short type,
	short team,
	char *string)
{
	long flag_index = find_netgame_flag(NULL, 0.f, 0.f, type, team);

	if (flag_index == NONE)
	{
		error(_error_silent, string, team);
	}

	return;
}

static void netgame_flag_verify_no_team_duplicates(
	short type,
	char *string)
{
	short flag_index;
	struct scenario *scenario = global_scenario_get();

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		short other_flag_index;

		if (type != flag->type)
		{
			continue;
		}

		for (other_flag_index = flag_index + 1; other_flag_index < scenario->netgame_flags.count; other_flag_index++)
		{
			struct scenario_netgame_flag *other_flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, other_flag_index, struct scenario_netgame_flag);

			if (type != other_flag->type)
			{
				continue;
			}

			if (other_flag->team_index == flag->team_index)
			{
				error(_error_silent, string, other_flag->team_index);
			}
		}
	}

	return;
}

static void netgame_flag_verify_team_range(
	short type,
	short min,
	short max,
	char *string)
{
	short flag_index;
	struct scenario *scenario = global_scenario_get();

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		if (type != flag->type)
		{
			continue;
		}

		if (flag->team_index < min || flag->team_index > max)
		{
			error(_error_silent, string, flag->team_index);
		}
	}

	return;
}

static void netgame_verify_spawn_points(
	short game_engine,
	short team,
	short count,
	char *string)
{
	short starting_location_index;
	short starting_location_count = player_get_starting_location_count();
	long found_count = 0;

	for (starting_location_index = 0; starting_location_index < starting_location_count; starting_location_index++)
	{
		struct scenario_player *starting_location = player_get_starting_location(starting_location_index);

		if (match_game_type(game_engine, NUMBEROF(starting_location->game_type), starting_location->game_type))
		{
			found_count++;
		}
	}

	if (found_count < count)
	{
		error(_error_silent, string, found_count, count);
	}

	return;
}

static void netgame_verify_equipment(
	short game_type,
	char *string)
{
	short equipment_index;
	long found_count = 0;
	struct scenario *scenario = global_scenario_get();

	for (equipment_index = 0; equipment_index < scenario->netgame_equipment.count; equipment_index++)
	{
		struct scenario_netgame_equipment *equipment = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_equipment, equipment_index, struct scenario_netgame_equipment);

		if (match_game_type(game_type, NUMBEROF(equipment->game_type), equipment->game_type))
		{
			found_count++;
		}
	}

	if (!found_count)
	{
		error(_error_silent, string);
	}

	return;
}

static void game_engine_verify_current_map(
	void)
{
	netgame_flag_verify_team_exists(_netgame_flag_ctf_flag, 0, "NETGAME MAP FAILURE: missing ctf flag [team %d]");
	netgame_flag_verify_team_exists(_netgame_flag_ctf_flag, 1, "NETGAME MAP FAILURE: missing ctf flag [team %d]");
	netgame_flag_verify_no_team_duplicates(_netgame_flag_ctf_flag, "NETGAME MAP FAILURE: duplicate ctf flag [team %d]");
	netgame_flag_verify_team_range(_netgame_flag_ctf_flag, 0, 1, "NETGAME MAP FAILURE: ctf flag out of range [team %d]");
	netgame_flag_verify_team_exists(_netgame_flag_hill, 0, "NETGAME MAP FAILURE: missing hill flag [team %d]");
	netgame_flag_verify_team_exists(_netgame_flag_hill, 1, "NETGAME MAP FAILURE: missing hill flag [team %d]");
	netgame_flag_verify_team_exists(_netgame_flag_oddball_ball_spawn, 0, "NETGAME MAP FAILURE: missing oddball flag [team %d]");
	netgame_flag_verify_team_exists(_netgame_flag_oddball_ball_spawn, 1, "NETGAME MAP FAILURE: missing oddball flag [team %d]");
	netgame_flag_verify_team_exists(_netgame_flag_race_track, 0, "NETGAME MAP FAILURE: missing race flag [team %d]");
	netgame_flag_verify_team_exists(_netgame_flag_race_track, 1, "NETGAME MAP FAILURE: missing race flag [team %d]");
	netgame_flag_verify_no_team_duplicates(_netgame_flag_race_track, "NETGAME MAP FAILURE: duplicate race track flag [team %d]");
	netgame_verify_spawn_points(_game_engine_ctf, 0, 4, "NETGAME MAP FAILURE: failed to find enough spawn points for ctf team 0 (%d/%d)");
	netgame_verify_spawn_points(_game_engine_ctf, 0, 4, "NETGAME MAP FAILURE: failed to find enough spawn points for ctf team 1 (%d/%d)");
	netgame_verify_spawn_points(_game_engine_slayer, 0, 4, "NETGAME MAP FAILURE: failed to find enough spawn points for slayer %d/%d");
	netgame_verify_spawn_points(_game_engine_oddball, 0, 4, "NETGAME MAP FAILURE: failed to find enough spawn points for oddball %d/%d");
	netgame_verify_spawn_points(_game_engine_king, 0, 4, "NETGAME MAP FAILURE: failed to find enough spawn points for king %d/%d");
	netgame_verify_spawn_points(_game_engine_race, 0, 4, "NETGAME MAP FAILURE: failed to find enough spawn points for race %d/%d");
	netgame_verify_equipment(_game_engine_ctf, "NETGAME MAP FAILURE: failed to find any equipment for ctf");
	netgame_verify_equipment(_game_engine_slayer, "NETGAME MAP FAILURE: failed to find any equipment for slayer");
	netgame_verify_equipment(_game_engine_oddball, "NETGAME MAP FAILURE: failed to find any equipment for oddball");
	netgame_verify_equipment(_game_engine_king, "NETGAME MAP FAILURE: failed to find any equipment for king");
	netgame_verify_equipment(_game_engine_race, "NETGAME MAP FAILURE: failed to find any equipment for race");

	return;
}

void game_engine_playlist_initialize(
	void)
{
	game_engine_playlist_next(0, 0, 2);

	return;
}

void game_engine_playlist_next(
	long unused1,
	long unused2,
	long unused3)
{
	struct game_variant user_selected_variant;
	char *map_name;

	strcpy(global_stage.map_name, "levels\\test\\carousel\\carousel");
	map_name = main_get_multiplayer_map_name();

	if (map_name && *map_name)
	{
		strncpy(global_stage.map_name, map_name, NUMBEROF(global_stage.map_name) - 1);
		global_stage.map_name[NUMBEROF(global_stage.map_name) - 1] = '\0';
	}

	if (player_ui_game_variant_specified(&user_selected_variant))
	{
		memcpy(&global_stage.game_variant, &user_selected_variant, sizeof(global_stage.game_variant));
	}

	return;
}

struct game_variant build_game_variant_slayer(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_motion_tracker;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 15;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = FALSE;
	variant.game_engine_variant.slayer.no_kill_penalty = FALSE;
	variant.game_engine_variant.slayer.kill_in_order = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_slayer_pro(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, TRUE);
	variant.universal_variant.goal_radar = _radar_motion_tracker;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 25;
	variant.universal_variant.suicide_penalty = 450;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = TRUE;
	variant.game_engine_variant.slayer.no_kill_penalty = TRUE;
	variant.game_engine_variant.slayer.kill_in_order = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_elimination(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_motion_tracker;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 1;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 25;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = FALSE;
	variant.game_engine_variant.slayer.no_kill_penalty = FALSE;
	variant.game_engine_variant.slayer.kill_in_order = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_phantoms(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 150;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 10;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = TRUE;
	variant.game_engine_variant.slayer.no_kill_penalty = TRUE;
	variant.game_engine_variant.slayer.kill_in_order = TRUE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_endurance(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_motion_tracker;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 5;
	variant.universal_variant.odd_man_out = TRUE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 300;
	variant.universal_variant.score_to_win = 10;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = FALSE;
	variant.game_engine_variant.slayer.no_kill_penalty = FALSE;
	variant.game_engine_variant.slayer.kill_in_order = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_rockets(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, TRUE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 25;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_rocket_launchers;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = FALSE;
	variant.game_engine_variant.slayer.no_kill_penalty = FALSE;
	variant.game_engine_variant.slayer.kill_in_order = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_snipers(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, TRUE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 150;
	variant.universal_variant.score_to_win = 15;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_sniping;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = FALSE;
	variant.game_engine_variant.slayer.no_kill_penalty = FALSE;
	variant.game_engine_variant.slayer.kill_in_order = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_team_slayer(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_slayer;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_motion_tracker;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 300;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 50;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.slayer.no_death_bonus = FALSE;
	variant.game_engine_variant.slayer.no_kill_penalty = FALSE;
	variant.game_engine_variant.slayer.kill_in_order = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_oddball(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_oddball;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 150;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 2;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_none;
	variant.game_engine_variant.oddball.ball_spawn_count = 1;
	variant.game_engine_variant.oddball.ball_spawn_delay = TRUE;
	variant.game_engine_variant.oddball.random_start = FALSE;
	variant.game_engine_variant.oddball.oddball_ball_type = _oddball_normal;
	variant.game_engine_variant.oddball.trait_with_ball = _game_trait_none;
	variant.game_engine_variant.oddball.trait_without_ball = _game_trait_none;
	variant.game_engine_variant.oddball.speed_with_ball = _oddball_speed_slow;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_team_oddball(
	void)
{
	struct game_variant variant;

	variant.game_engine_index = _game_engine_oddball;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, TRUE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 300;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 2;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_none;
	variant.game_engine_variant.oddball.ball_spawn_count = 1;
	variant.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	variant.game_engine_variant.oddball.random_start = FALSE;
	variant.game_engine_variant.oddball.oddball_ball_type = _oddball_normal;
	variant.game_engine_variant.oddball.trait_with_ball = _game_trait_none;
	variant.game_engine_variant.oddball.trait_without_ball = _game_trait_none;
	variant.game_engine_variant.oddball.speed_with_ball = _oddball_speed_slow;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_reverse_tag(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_oddball;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 150;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 2;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_none;
	variant.game_engine_variant.oddball.ball_spawn_count = 1;
	variant.game_engine_variant.oddball.ball_spawn_delay = TRUE;
	variant.game_engine_variant.oddball.random_start = FALSE;
	variant.game_engine_variant.oddball.oddball_ball_type = _oddball_magic;
	variant.game_engine_variant.oddball.trait_with_ball = _game_trait_none;
	variant.game_engine_variant.oddball.trait_without_ball = _game_trait_none;
	variant.game_engine_variant.oddball.speed_with_ball = _oddball_speed_slow;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_accumulation(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_oddball;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_none;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 150;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 5;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_none;
	variant.game_engine_variant.oddball.ball_spawn_count = 16;
	variant.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	variant.game_engine_variant.oddball.random_start = FALSE;
	variant.game_engine_variant.oddball.oddball_ball_type = _oddball_magic;
	variant.game_engine_variant.oddball.trait_with_ball = _game_trait_none;
	variant.game_engine_variant.oddball.trait_without_ball = _game_trait_none;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_juggernaut(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_oddball;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 150;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 10;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.oddball.ball_spawn_count = 1;
	variant.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	variant.game_engine_variant.oddball.random_start = FALSE;
	variant.game_engine_variant.oddball.trait_with_ball = _game_trait_extra_damage;
	variant.game_engine_variant.oddball.trait_without_ball = _game_trait_none;
	variant.game_engine_variant.oddball.oddball_ball_type = _oddball_terminator;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_stalker(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_oddball;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_motion_tracker;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 150;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 10;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.oddball.ball_spawn_count = 1;
	variant.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	variant.game_engine_variant.oddball.random_start = FALSE;
	variant.game_engine_variant.oddball.oddball_ball_type = _oddball_terminator;
	variant.game_engine_variant.oddball.trait_with_ball = _game_trait_invisible;
	variant.game_engine_variant.oddball.trait_without_ball = _game_trait_damage_resistant;
	variant.game_engine_variant.oddball.speed_with_ball = _oddball_speed_faster;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_king(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_king;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 150;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 2;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.king.moving_hill = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_king_pro(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_king;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, TRUE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 300;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 2;
	variant.universal_variant.suicide_penalty = 450;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.king.moving_hill = FALSE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_crazy_king(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_king;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 2;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.king.moving_hill = TRUE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_team_king(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_king;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 300;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 2;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.king.moving_hill = TRUE;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_ctf(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_ctf;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 300;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 3;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.ctf.assault = FALSE;
	variant.game_engine_variant.ctf.flag_at_home_to_score = FALSE;
	variant.game_engine_variant.ctf.flag_must_reset = FALSE;
	variant.game_engine_variant.ctf.reset_on_capture = FALSE;
	variant.game_engine_variant.ctf.single_flag_time = 0;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_ctf_pro(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_ctf;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, TRUE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 300;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 3;
	variant.universal_variant.suicide_penalty = 450;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.ctf.assault = FALSE;
	variant.game_engine_variant.ctf.flag_at_home_to_score = TRUE;
	variant.game_engine_variant.ctf.flag_must_reset = FALSE;
	variant.game_engine_variant.ctf.reset_on_capture = FALSE;
	variant.game_engine_variant.ctf.single_flag_time = 0;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_invasion(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_ctf;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 5;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 3;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.ctf.assault = TRUE;
	variant.game_engine_variant.ctf.flag_at_home_to_score = FALSE;
	variant.game_engine_variant.ctf.flag_must_reset = FALSE;
	variant.game_engine_variant.ctf.reset_on_capture = FALSE;
	variant.game_engine_variant.ctf.single_flag_time = 0;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_iron_ctf(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_ctf;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 2.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 450;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 3;
	variant.universal_variant.suicide_penalty = 150;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_tank;
	variant.game_engine_variant.ctf.assault = FALSE;
	variant.game_engine_variant.ctf.flag_at_home_to_score = FALSE;
	variant.game_engine_variant.ctf.flag_must_reset = TRUE;
	variant.game_engine_variant.ctf.reset_on_capture = FALSE;
	variant.game_engine_variant.ctf.single_flag_time = 0;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_race(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_race;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 3;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.race.race_type = _race_type_normal;
	variant.game_engine_variant.race.team_scoring = _race_team_scoring_min;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_rally(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_race;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 15;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = FALSE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.race.race_type = _race_type_flag_rally;
	variant.game_engine_variant.race.team_scoring = _race_team_scoring_min;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_team_race(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_race;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 3;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.race.race_type = _race_type_normal;
	variant.game_engine_variant.race.team_scoring = _race_team_scoring_min;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

struct game_variant build_game_variant_team_rally(
	void)
{
	struct game_variant variant = {0};

	variant.game_engine_index = _game_engine_race;
	SET_FLAG(variant.universal_variant.flags, _allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _infinite_grenades_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _no_shields_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _always_invisible_bit, FALSE);
	SET_FLAG(variant.universal_variant.flags, _allow_motion_tracker_bit, TRUE);
	SET_FLAG(variant.universal_variant.flags, _generic_starting_equipment_bit, FALSE);
	variant.universal_variant.goal_radar = _radar_nav_point;
	variant.universal_variant.health = 1.0f;
	variant.universal_variant.lives = 0;
	variant.universal_variant.odd_man_out = FALSE;
	variant.universal_variant.respawn_time = 0;
	variant.universal_variant.respawn_time_growth = 0;
	variant.universal_variant.score_to_win = 5;
	variant.universal_variant.suicide_penalty = 300;
	variant.universal_variant.teams = TRUE;
	variant.universal_variant.weapon_set = _game_engine_weapons_normal;
	variant.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	variant.game_engine_variant.race.race_type = _race_type_flag_rally;
	variant.game_engine_variant.race.team_scoring = _race_team_scoring_min;
	variant.flags = FLAG(_variant_is_system_default_bit);

	return variant;
}

void game_engine_override_map_name(
	char const *map_name)
{
	if (map_name && *map_name)
	{
		strncpy(global_stage.map_name, map_name, NUMBEROF(global_stage.map_name) - 1);
	}

	return;
}

void game_engine_override_game_variant(
	struct game_variant const *variant)
{
	if (variant)
	{
		memcpy(&global_stage.game_variant, variant, sizeof(global_stage.game_variant));
	}

	return;
}

static void rasterize_in_game_score_draw_line(
	wchar_t const *text,
	boolean hilite,
	real_argb_color *color,
	long line)
{
	short split_screen_tab_stops[3];
	short mp_tab_stops[3];
	short const *tab_stops;
	rectangle2d bounds = render.camera.window_bounds;
	boolean split_screen = local_player_count() > 1;
	long font_index = hud_get_font_index();

	split_screen_tab_stops[0] = 80;
	split_screen_tab_stops[1] = 125;
	split_screen_tab_stops[2] = 200;
	mp_tab_stops[0] = 130;
	mp_tab_stops[1] = 195;
	mp_tab_stops[2] = 315;
	tab_stops = bounds.x1 - bounds.x0 > 320 ? mp_tab_stops : split_screen_tab_stops;

	if (line)
	{
		draw_string_set_tab_stops(tab_stops, NUMBEROF(mp_tab_stops));
	}
	else
	{
		draw_string_set_tab_stops(NULL, 0);
	}

	offset_rectangle2d(&bounds, (short)-render.camera.viewport_bounds.x0, (short)-render.camera.viewport_bounds.y0);

	if (font_index != NONE)
	{
		long line_height;
		struct font_header const *font = font_definition_get(font_index);
		long first_line = split_screen ? 4 : 8;

		if (split_screen)
		{
			line_height = font->leading_height + font->ascending_height;
		}
		else
		{
			line_height = font->leading_height + font->descending_height + font->ascending_height;
		}

		if (hilite)
		{
			color->red += 0.4f;
			color->green += 0.4f;
			color->blue += 0.4f;

			if (color->red > 1.f)
			{
				color->red = 1.f;
			}

			if (color->green > 1.f)
			{
				color->green = 1.f;
			}

			if (color->blue > 1.f)
			{
				color->blue = 1.f;
			}
		}

		line += first_line;
		bounds.y0 = (short)(line * line_height);
		bounds.y1 = (short)((line + 1) * line_height);
		draw_string_set_draw_mode(font_index, NONE, 0, 0, color);
		rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, text);
	}

	draw_string_set_tab_stops(NULL, 0);

	return;
}

static void game_engine_rasterize_in_game_score(
	long player_index,
	real alpha)
{
	wchar_t title_string[MAXIMUM_TITLE_STRING_LENGTH];
	struct statistic_buffer statistic_buffer[6];
	long count;
	real_argb_color color;
	wchar_t buffer[256];
	wchar_t custom_buffer[256];
	real_argb_color team_colors[2];
	real_argb_color normal_color;
	wchar_t *place_string;
	wchar_t *name_string;
	long statistic_index;
	boolean teams = game_engine_has_teams();

	game_engine_generate_title_string(player_index, title_string);
	count = select_players_to_display(player_index, statistic_buffer, _postgame_statistic_ranking, NUMBEROF(statistic_buffer));
	color.alpha = alpha;
	color.red = 0.7f;
	color.green = 0.7f;
	color.blue = 0.7f;
	rasterize_in_game_score_draw_line(title_string, FALSE, &color, 0);
	color.red = 0.5f;
	color.green = 0.5f;
	color.blue = 0.5f;
	color.alpha = alpha;
	multiplayer_game_text_get_string(place_string, _string_place);
	multiplayer_game_text_get_string(name_string, _string_name);

	game_engine->get_score_header_string(custom_buffer);
	usprintf(
		buffer,
		L"\t%s\t%s\t%s",
		place_string,
		name_string,
		custom_buffer);
	rasterize_in_game_score_draw_line(buffer, FALSE, &color, 1);

	for (statistic_index = 0; statistic_index < count; statistic_index++)
	{
		long statistic_player_index = statistic_buffer[statistic_index].player_index;
		boolean hilite = player_index == statistic_player_index;

		if (player_try_and_get(statistic_player_index))
		{
			struct player_datum const *player;
			wchar_t *status_string;
			real_argb_color *line_color;

			color = *hud_get_text_color(&normal_color);
			player = player_get(statistic_player_index);
			team_colors[0].alpha = alpha;
			team_colors[0].red = 0.6f;
			team_colors[0].green = 0.3f;
			team_colors[0].blue = 0.3f;
			team_colors[1].alpha = alpha;
			team_colors[1].red = 0.3f;
			team_colors[1].green = 0.3f;
			team_colors[1].blue = 0.6f;
			color.alpha = alpha;
			game_engine->get_score_string(statistic_player_index, custom_buffer);

			if (game_engine_player_is_out_of_lives(statistic_player_index))
			{
				multiplayer_game_text_get_string(status_string, _string_dead);
			}
			else if (player->quit_out_of_game)
			{
				multiplayer_game_text_get_string(status_string, _string_quit);
			}
			else
			{
				status_string = custom_buffer;
			}

			usprintf(
				buffer,
				L"\t%s\t%s\t%s",
				get_place_string(&statistic_buffer[statistic_index]),
				player->name,
				status_string);

			if (teams)
			{
				line_color = &team_colors[PIN(player->team_index, 0, 1)];
			}
			else
			{
				line_color = &color;
			}

			rasterize_in_game_score_draw_line(buffer, hilite, line_color, statistic_index + 2);
		}
	}

	return;
}

boolean game_engine_hud_draw_messages(
	long player_index)
{
	boolean result = TRUE;

	if (game_engine_running() && player_index != NONE)
	{
		struct player_datum const *player = player_get(player_index);

		if (player->local_player_index != NONE)
		{
			real alpha = game_engine_globals.score_alpha[player->local_player_index];

			if (alpha > 0.f)
			{
				result = FALSE;
			}
		}
	}

	return result;
}

boolean game_engine_player_has_stealth_weapon(
	long player_index)
{
	boolean result = FALSE;

	if (player_index != NONE)
	{
		struct player_datum const *player = player_get(player_index);
		long unit_index = player->unit_index;

		if (unit_index != NONE)
		{
			struct unit_datum const *unit = unit_get(unit_index);
			long current_weapon_index = unit->unit.current_weapon_index;

			if (current_weapon_index != NONE)
			{
				long weapon_index = unit->unit.weapon_object_indices[current_weapon_index];

				if (weapon_index != NONE)
				{
					struct weapon_datum const *weapon = weapon_get(weapon_index);

					if (weapon->definition_index != NONE)
					{
						struct weapon_definition const *definition = weapon_definition_get(weapon->definition_index);

						result = TEST_FLAG(definition->weapon.flags, _weapon_does_not_depower_active_camo_bit);
					}
				}
			}
		}
	}

	return result;
}

void game_engine_weapon_fired(
	long player_index)
{
	if (game_engine_running() && player_index != NONE)
	{
		struct player_datum const *player = player_get(player_index);
		long unit_index = player->unit_index;

		if (unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);
			long weapon_index = unit_inventory_get_weapon(unit_index, unit_get(unit_index)->unit.current_weapon_index);
			real minimum_active_camouflage = 0.05f;
			real active_camo_ding = 0.1f;

			if (game_engine_player_has_stealth_weapon(player_index))
			{
				active_camo_ding = 0.f;
			}
			else if (weapon_index != NONE)
			{
				struct weapon_datum *weapon = weapon_get(weapon_index);
				struct weapon_definition const *definition = weapon_definition_get(weapon->definition_index);

				if (0.f != definition->weapon.active_camo_ding)
				{
					active_camo_ding = definition->weapon.active_camo_ding;
				}
			}

			if (unit->unit.active_camouflage >= minimum_active_camouflage)
			{
				unit->unit.cause_for_camo_regrowth = cause_for_camo_regrowth_shot_fired;
				unit->unit.active_camouflage -= active_camo_ding;
				unit->unit.active_camouflage = MAX(minimum_active_camouflage, unit->unit.active_camouflage);
			}
		}
	}

	return;
}

static boolean test_any_gamepad_button(
	long button_index)
{
	long gamepad_index;
	boolean result = FALSE;

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
	{
		struct gamepad_state const *gamepad_state = input_get_gamepad_state((short)gamepad_index);

		if (gamepad_state && gamepad_state->buttons[button_index])
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

void ticks_to_unicode_time_string(
	long ticks,
	long count,
	wchar_t *buffer)
{
	wchar_t minutes_buffer[64];
	wchar_t seconds_buffer[64];
	long total_seconds = ticks / TICKS_PER_SECOND;
	long minutes = total_seconds / SECONDS_PER_MINUTE;
	long seconds = total_seconds - minutes * SECONDS_PER_MINUTE;

	if (minutes == 0)
	{
		usnprintf(minutes_buffer, NUMBEROF(minutes_buffer), L" ");
	}
	else
	{
		usnprintf(minutes_buffer, NUMBEROF(minutes_buffer), L"%d", minutes);
	}

	if (seconds <= 9)
	{
		usnprintf(seconds_buffer, NUMBEROF(seconds_buffer), L"0%d", seconds);
	}
	else
	{
		usnprintf(seconds_buffer, NUMBEROF(seconds_buffer), L"%d", seconds);
	}

	usnprintf(buffer, count, L"%s:%s", minutes_buffer, seconds_buffer);

	return;
}

void game_engine_flag_reset(
	long weapon_index,
	real_point3d const *position)
{
	if (weapon_index != NONE)
	{
		struct weapon_datum *weapon = weapon_get(weapon_index);

		object_set_position(weapon_index, position, global_forward3d, global_up3d);
		object_reset(weapon_index);
		SET_FLAG(weapon->weapon.flags, _weapon_multiplayer_inventory_flag, FALSE);
		weapon->item.last_owned_time = game_time_get();
		weapon->item.ignore_object_index = NONE;
	}

	return;
}

void game_engine_variant_cleanup(
	struct game_variant *variant)
{
	struct game_variant old_variant = *variant;

	variant->human_readable_game_description[NUMBEROF(variant->human_readable_game_description) - 1] = '\0';
	variant->game_engine_index = PIN(variant->game_engine_index, FIRST_USABLE_GAME_ENGINE_INDEX, LAST_USABLE_GAME_ENGINE_INDEX);
	variant->universal_variant.teams = variant->universal_variant.teams ? TRUE : FALSE;
	variant->universal_variant.odd_man_out = variant->universal_variant.odd_man_out ? TRUE : FALSE;
	variant->universal_variant.respawn_time_growth = MAX(variant->universal_variant.respawn_time_growth, 0);
	variant->universal_variant.respawn_time = MAX(variant->universal_variant.respawn_time, 0);
	variant->universal_variant.suicide_penalty = MAX(variant->universal_variant.suicide_penalty, 0);
	variant->universal_variant.lives = MAX(variant->universal_variant.lives, 0);
	variant->universal_variant.health = PIN(variant->universal_variant.health, 0.25f, 4.f);
	variant->universal_variant.weapon_set = PIN(variant->universal_variant.weapon_set, 0, NUMBER_OF_GAME_ENGINE_WEAPON_SETS - 1);
	variant->universal_variant.vehicle_set = PIN(variant->universal_variant.vehicle_set, 0, NUMBER_OF_GAME_ENGINE_VEHICLE_SETS - 1);

	switch (variant->game_engine_index)
	{
	case _game_engine_ctf:
		variant->universal_variant.teams = TRUE;
		variant->game_engine_variant.ctf.assault = variant->game_engine_variant.ctf.assault ? TRUE : FALSE;
		variant->game_engine_variant.ctf.reset_on_capture = variant->game_engine_variant.ctf.reset_on_capture ? TRUE : FALSE;
		variant->game_engine_variant.ctf.flag_must_reset = variant->game_engine_variant.ctf.flag_must_reset ? TRUE : FALSE;
		variant->game_engine_variant.ctf.flag_at_home_to_score = variant->game_engine_variant.ctf.flag_at_home_to_score ? TRUE : FALSE;
		variant->game_engine_variant.ctf.single_flag_time = FLOOR(variant->game_engine_variant.ctf.single_flag_time, 0);
		break;
	case _game_engine_slayer:
		variant->game_engine_variant.slayer.no_death_bonus = variant->game_engine_variant.slayer.no_death_bonus ? TRUE : FALSE;
		variant->game_engine_variant.slayer.no_kill_penalty = variant->game_engine_variant.slayer.no_kill_penalty ? TRUE : FALSE;
		variant->game_engine_variant.slayer.kill_in_order = variant->game_engine_variant.slayer.kill_in_order ? TRUE : FALSE;
		break;
	}

	if (memcmp(&old_variant, variant, sizeof(*variant)))
	{
		struct game_variant test;

		memcpy(&test, &old_variant, sizeof(test));
		memcpy(&test, variant, sizeof(test));
		error(_error_silent, "NETGAME CODE FAILURE: game_engine_variant_cleanup changed the variant");
	}

	return;
}

boolean game_engine_allow_pause(
	void)
{
	return game_engine_globals.mode == game_engine_mode_active;
}

static void game_engine_predict_resources(
	void)
{
	long weapon_definitions[10];
	long weapon_index;
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information const *multiplayer_information =TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information);

	switch (global_variant.universal_variant.vehicle_set)
	{
	case _game_engine_vehicles_warthog:
		object_definition_predict(TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, 0, struct tag_reference)->index);
		break;
	case _game_engine_vehicles_ghost:
		object_definition_predict(TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, 1, struct tag_reference)->index);
		break;
	case _game_engine_vehicles_tank:
		object_definition_predict(TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, 2, struct tag_reference)->index);
		break;
	default:
		object_definition_predict(TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, 0, struct tag_reference)->index);
		object_definition_predict(TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, 1, struct tag_reference)->index);
		object_definition_predict(TAG_BLOCK_GET_ELEMENT(&multiplayer_information->vehicles, 2, struct tag_reference)->index);
		break;
	}

	object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_frag_grenade));
	object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_plasma_grenade));

	if (global_variant.game_engine_index == _game_engine_oddball)
	{
		object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_ball));
	}

	if (global_variant.game_engine_index == _game_engine_ctf)
	{
		object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_flag));
	}

	weapon_definitions[0] = list_index_to_weapon_definition_index(_weapon_list_assault_rifle);
	weapon_definitions[1] = list_index_to_weapon_definition_index(_weapon_list_flamethrower);
	weapon_definitions[2] = list_index_to_weapon_definition_index(_weapon_list_gravity_rifle);
	weapon_definitions[3] = list_index_to_weapon_definition_index(_weapon_list_needler);
	weapon_definitions[4] = list_index_to_weapon_definition_index(_weapon_list_pistol);
	weapon_definitions[5] = list_index_to_weapon_definition_index(_weapon_list_plasma_pistol);
	weapon_definitions[6] = list_index_to_weapon_definition_index(_weapon_list_plasma_rifle);
	weapon_definitions[7] = list_index_to_weapon_definition_index(_weapon_list_rocket_launcher);
	weapon_definitions[8] = list_index_to_weapon_definition_index(_weapon_list_shotgun);
	weapon_definitions[9] = list_index_to_weapon_definition_index(_weapon_list_sniper_rifle);

	for (weapon_index = 0; weapon_index < (long)NUMBEROF(weapon_definitions); weapon_index++)
	{
		object_definition_predict(game_engine_remap_weapon(weapon_definitions[weapon_index]));
	}

	return;
}

boolean game_engine_draw_object_in_motion_sensor(
	long unit_index)
{
	boolean result = TRUE;

	if (game_engine_running())
	{
		result = TEST_FLAG(global_variant.universal_variant.flags, _allow_motion_tracker_bit);
	}

	return result;
}

boolean game_engine_allow_dynamic_lighting(
	void)
{
	boolean result = TRUE;

	if (game_engine_running())
	{
		result = !TEST_FLAG(game_engine_globals.flags, _game_engine_disable_dynamic_light_bit);
	}

	return result;
}

boolean game_engine_allow_integrated_lights(
	long object_index)
{
	boolean result = TRUE;

	if (game_engine_running())
	{
		result = !TEST_FLAG(game_engine_globals.flags, _game_engine_disable_integrated_lights_bit);
	}

	return result;
}

boolean game_engine_force_autopickup(
	long unit_index,
	long weapon_index)
{
	boolean result = FALSE;

	if (game_engine_running() && global_variant.game_engine_index == _game_engine_ctf && weapon_is_flag(weapon_index))
	{
		result = TRUE;
	}

	return result;
}

/* ---------- private code */
