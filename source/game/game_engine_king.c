/*
GAME_ENGINE_KING.C

symbols in this file:
000A0030 0010:
	_king_engine_dispose (0000)
000A0040 0020:
	_point3d_to_point2d (0000)
000A0060 0350:
	_find_hill (0000)
000A03B0 0010:
	_king_engine_dispose_from_old_map (0000)
000A03C0 0020:
	_king_engine_player_added (0000)
000A03E0 0010:
	_king_engine_game_ending (0000)
000A03F0 0020:
	_king_engine_game_starting (0000)
000A0410 0010:
	_king_engine_statistics_append (0000)
000A0420 0010:
	_king_engine_handle_client_message (0000)
000A0430 0010:
	_king_engine_handle_server_message (0000)
000A0440 0010:
	_king_engine_pregame_post_rasterize (0000)
000A0450 0090:
	_player_inside_hill (0000)
000A04E0 0160:
	_king_engine_player_update (0000)
000A0640 01c0:
	_king_calculate_hill_state (0000)
000A0800 0010:
	_king_engine_player_damaged_player (0000)
000A0810 0010:
	_king_engine_player_killed_player (0000)
000A0820 0190:
	_king_engine_display_score (0000)
000A09B0 0010:
	_king_engine_prespawn_player_update (0000)
000A09C0 0040:
	_king_get_score (0000)
000A0A00 0090:
	_render_dynamic_quad_initialize (0000)
000A0A90 02b0:
	_render_dynamic_quad (0000)
000A0D40 0040:
	_king_get_score_string (0000)
000A0D80 0060:
	_king_get_score_header_string (0000)
000A0DE0 0030:
	_king_get_team_score_string (0000)
000A0E10 0020:
	_king_engine_goal_matches_player (0000)
000A0E30 0070:
	_find_next_hill (0000)
000A0EA0 0110:
	_king_engine_initialize_for_new_map (0000)
000A0FB0 03a0:
	_king_engine_post_rasterize (0000)
000A1350 0110:
	_king_engine_update (0000)
0025BDC0 000d:
	??_C@_0N@DGPCNCJC@NULL?5?$CB?$DN?5flag?$AA@ (0000)
0025BDD0 0027:
	??_C@_0CH@IHPMMFJJ@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BDF8 0023:
	??_C@_0CD@DHKICDEA@king_globals?4hill_point_count?5?$CB?$DN@ (0000)
0025BE1C 0014:
	??_C@_0BE@EEOONBNI@FAILED?5TO?5FIND?5HILL?$AA@ (0000)
0025BE30 000b:
	??_C@_0L@HEFOOJCG@crown_blue?$AA@ (0000)
0025BE3C 0038:
	??_C@_0DI@GPEOKAOO@failed?5to?5find?5hill?5?$CD?$CFd?5most?5lik@ (0000)
002DE488 0088:
	_king_engine (0000)
0043E948 0230:
	_bss_0043e948 (0000)
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
#include "shaders.h"
#include "network_messages.h"
#include "text_group.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
#include "weapons.h"
#include "font_group.h"
#include "network_server_message_handler.h"
#include "scenery.h"
#include "game_engine.h"
#include "game_globals.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "geometry.h"
#include "rasterizer_geometry.h"
#include "unicode.h"
#include "game.h"

/* ---------- constants */

enum
{
	king_message_enemy_on_the_hill = 30,
	king_message_ally_on_the_hill,
	king_message_you_are_on_the_hill
};

enum
{
	king_hill_uncontrolled = 0,
	king_hill_controlled,
	king_hill_controlled_red,
	king_hill_controlled_blue,
	king_hill_contested
};

enum
{
	MAXIMUM_HILL_POINTS = 12, /* fake name */
	MAXIMUM_KING_HILLS = 64 /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct king_globals_type
{
	long score[MULTIPLAYER_MAXIMUM_PLAYERS];
	long score_tick[MULTIPLAYER_MAXIMUM_PLAYERS];
	boolean on_the_hill[MULTIPLAYER_MAXIMUM_PLAYERS];
	long hill_point_count;
	real_point3d hill_points[MAXIMUM_HILL_POINTS];
	real_point2d convex_hull[MAXIMUM_HILL_POINTS];
	real_point3d hill_center;
	long hill_state;
	long hill_controlled_count;
	long hill_previous_controller;
	real hill_top;
	real hill_bottom;
	long hill_id;
	long hill_timer;
};

/* ---------- prototypes */

static void king_engine_dispose(void);
static void point3d_to_point2d(long count, real_point3d *src, real_point2d *dst);
static void find_hill(void);
static void king_engine_dispose_from_old_map(void);
static void king_engine_player_added(long player_index);
static void king_engine_game_ending(void);
static void king_engine_game_starting(void);
static void king_engine_statistics_append(struct game_statistics *permanent_statistics, struct game_statistics *game_statistics);
static void king_engine_handle_client_message(long player_index, void *encoded_message, short encoded_message_size);
static void king_engine_handle_server_message(void *encoded_message, short encoded_message_size);
static void king_engine_pregame_post_rasterize(void);
static boolean player_inside_hill(long player_index);
static void king_engine_player_update(long player_index);
static void king_calculate_hill_state(void);
static void king_engine_player_damaged_player(long killing_player_index, long dead_player_index, boolean friendly_fire);
static void king_engine_player_killed_player(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);
static boolean king_engine_display_score(long player_index, long message, long message_data, word *buffer, long buffer_size);
static void king_engine_prespawn_player_update(long player_index);
static long king_get_score(long player_index, enum get_score_type get_score_type);
void render_dynamic_quad_initialize(void);
void render_dynamic_quad(struct model_vertex_uncompressed *verts, long shader_index, struct render_lighting const *lighting, struct render_animation const *animation, real u_scale, real v_scale);
static word *king_get_score_string(long player_index, word *buffer);
static word *king_get_score_header_string(word *buffer);
static word *king_get_team_score_string(long team_index, word *buffer);
static boolean king_engine_goal_matches_player(long player_index, long goal_index);
static long find_next_hill(long hill_id);
static boolean king_engine_initialize_for_new_map(void);
static void king_engine_post_rasterize(void);
static void king_engine_update(void);

/* ---------- globals */

struct game_engine king_engine =
{
	"king",
	_game_engine_king,
	king_engine_dispose,
	king_engine_initialize_for_new_map,
	king_engine_dispose_from_old_map,
	king_engine_player_added,
	king_engine_game_ending,
	king_engine_game_starting,
	king_engine_statistics_append,
	king_engine_handle_client_message,
	king_engine_handle_server_message,
	king_engine_pregame_post_rasterize,
	king_engine_post_rasterize,
	king_engine_player_update,
	NULL, // weapon_update
	NULL, // weapon_pickup
	NULL, // weapon_drop
	king_engine_update,
	king_get_score,
	king_get_score_string,
	king_get_score_header_string,
	king_get_team_score_string,
	NULL, // allow_pick_up
	king_engine_player_damaged_player,
	king_engine_player_killed_player,
	king_engine_display_score,
	NULL, // starting_location_rating
	king_engine_prespawn_player_update,
	NULL, // postspawn_player_update
	NULL, // game_engine_player_get_team_index
	king_engine_goal_matches_player,
	NULL, // game_engine_test_flag
	NULL, // game_engine_test_trait
	NULL, // game_engine_did_player_win
};

real_rgb_color global_default_animation_colors[4];
real global_default_animation_values[4];

static short king_engine_hills[MAXIMUM_KING_HILLS];
static short king_engine_hill_count; /* fake name */
static struct king_globals_type king_globals;

/* ---------- private code */

static void king_engine_dispose(
	void)
{
	return;
}

static void point3d_to_point2d(
	long count,
	real_point3d *src,
	real_point2d *dst)
{
	long index;

	for (index = 0; index < count; index++)
	{
		dst[index].x = src[index].x;
		dst[index].y = src[index].y;
	}

	return;
}

static long find_next_hill(
	long hill_id)
{
	short start = random_range(0, king_engine_hill_count);
	long next_hill;
	short index;

	for (index = 0; index < king_engine_hill_count; index++)
	{
		short hill_index = (index + start) % king_engine_hill_count;

		if (hill_id != king_engine_hills[hill_index])
		{
			next_hill = king_engine_hills[hill_index];
			break;
		}
	}

	return next_hill;
}

static void find_hill(
	void)
{
	struct scenario *scenario = global_scenario_get();
	long hill_flag_indicies[MAXIMUM_HILL_POINTS];
	real_point2d hill_points_2d[MAXIMUM_HILL_POINTS];
	real_point3d hill_points_3d[MAXIMUM_HILL_POINTS];
	short hull_indices[MAXIMUM_HILL_POINTS];
	real_point3d min_point;
	real_point3d max_point;
	long point_count;

	king_globals.hill_point_count = point_count = find_netgame_flags(NULL, 0.f, 0.f, _netgame_flag_hill, (short)king_globals.hill_id, MAXIMUM_HILL_POINTS, hill_flag_indicies);

	if (point_count == 0)
	{
	}
	else
	{
		long index;
		long hull_count;

		for (index = 0; index < point_count; index++)
		{
			struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, hill_flag_indicies[index], struct scenario_netgame_flag);

			match_assert("c:\\halo\\SOURCE\\game\\game_engine_king.c", 222, NULL != flag);

			hill_points_3d[index] = flag->position;
		}

		if (point_count == 1)
		{
			hill_points_3d[1] = hill_points_3d[0];
			hill_points_3d[2] = hill_points_3d[0];
			hill_points_3d[3] = hill_points_3d[0];
			hill_points_3d[0].x += -1.f;
			hill_points_3d[0].y += -1.f;
			hill_points_3d[1].x += 1.f;
			hill_points_3d[1].y += -1.f;
			hill_points_3d[2].x += -1.f;
			hill_points_3d[2].y += 1.f;
			hill_points_3d[3].x += 1.f;
			hill_points_3d[3].y += 1.f;
			point_count = 4;
		}

		point3d_to_point2d(point_count, hill_points_3d, hill_points_2d);
		hull_count = convex_hull2d((short)point_count, hill_points_2d, hull_indices);

		for (index = 0; index < hull_count; index++)
		{
			short hull_index = hull_indices[index];

			king_globals.hill_points[index] = hill_points_3d[hull_index];
			king_globals.convex_hull[index] = hill_points_2d[hull_index];
		}

		king_globals.hill_point_count = hull_count;
		min_point = king_globals.hill_points[0];
		max_point = king_globals.hill_points[0];

		for (index = 0; index < king_globals.hill_point_count; index++)
		{
			min_point.x = MIN(min_point.x, king_globals.hill_points[index].x);
			min_point.y = MIN(min_point.y, king_globals.hill_points[index].y);
			min_point.z = MIN(min_point.z, king_globals.hill_points[index].z);
			max_point.x = MAX(max_point.x, king_globals.hill_points[index].x);
			max_point.y = MAX(max_point.y, king_globals.hill_points[index].y);
			max_point.z = MAX(max_point.z, king_globals.hill_points[index].z);
		}

		king_globals.hill_bottom = min_point.z - 0.1f;
		king_globals.hill_top = max_point.z + 0.8f;
		king_globals.hill_center.x = (max_point.x + min_point.x) / 2.f;
		king_globals.hill_center.y = (max_point.y + min_point.y) / 2.f;
		king_globals.hill_center.z = (max_point.z + min_point.z) / 2.f;
	}

	return;
}

static boolean king_engine_initialize_for_new_map(
	void)
{
	struct scenario *scenario = global_scenario_get();
	short flag_index;

	memset(&king_globals, 0, sizeof(king_globals));
	king_engine_hill_count = 0;

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(&scenario->netgame_flags, flag_index, struct scenario_netgame_flag);

		if (flag->type == _netgame_flag_hill)
		{
			short hill_index;
			boolean found = FALSE;

			for (hill_index = 0; hill_index < king_engine_hill_count; hill_index++)
			{
				if (king_engine_hills[hill_index] == flag->team_index)
				{
					found = TRUE;
					break;
				}
			}

			if (!found)
			{
				king_engine_hills[king_engine_hill_count++] = flag->team_index;
			}
		}
	}

	king_globals.hill_id = 0;
	king_globals.hill_timer = TICKS_PER_MINUTE;
	king_globals.hill_previous_controller = NONE;
	king_globals.hill_state = king_hill_uncontrolled;
	find_hill();

	match_assert("c:\\halo\\SOURCE\\game\\game_engine_king.c", 340, king_globals.hill_point_count != 0);

	render_dynamic_quad_initialize();

	return TRUE;
}

static void king_engine_dispose_from_old_map(
	void)
{
	return;
}

static void king_engine_player_added(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	return;
}

static void king_engine_game_ending(
	void)
{
	return;
}

static void king_engine_game_starting(
	void)
{
	game_engine_play_multiplayer_sound(game_engine_has_teams() ? _multiplayer_sound_team_king : _multiplayer_sound_king);

	return;
}

static void king_engine_statistics_append(
	struct game_statistics *permanent_statistics,
	struct game_statistics *game_statistics)
{
	return;
}

static void king_engine_handle_client_message(
	long player_index,
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void king_engine_handle_server_message(
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void king_engine_pregame_post_rasterize(
	void)
{
	return;
}

static void king_engine_post_rasterize(
	void)
{
	long point_count;
	real_point3d *points;
	long index;
	struct scenario *scenario;
	struct game_globals *game_globals;
	struct game_globals_multiplayer_information *multiplayer_information;
	long shader_index;
	real perimeter;
	real length;
	real u0;
	real u1;
	real inverse_perimeter;
	real texture_scale;

	scenario = global_scenario_get();
	game_globals = scenario_get_game_globals();
	multiplayer_information = TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information);
	shader_index = multiplayer_information->hill_shader.index;
	perimeter = 0.f;
	point_count = king_globals.hill_point_count;
	points = king_globals.hill_points;

	for (index = 0; index < point_count; index++)
	{
		perimeter += distance3d(&points[index], &points[index + 1 == point_count ? 0 : index + 1]);
	}

	texture_scale = 1.f / floor(perimeter + 0.5f);
	inverse_perimeter = 1.f / (perimeter * texture_scale);
	u0 = 0.f;
	length = 0.f;

	for (index = 0; index < point_count; index++)
	{
		struct model_vertex_uncompressed render_points[4];
		real distance;
		real_vector3d v[2];
		real_vector3d n;

		distance = distance3d(&points[index], &points[index + 1 == point_count ? 0 : index + 1]);
		u1 = inverse_perimeter * (length + distance);
		length += distance;
		memset(render_points, 0, sizeof(render_points));
		render_points[1].position = points[index];
		render_points[0].position = render_points[1].position;
		render_points[3].position = points[index + 1 == point_count ? 0 : index + 1];
		render_points[2].position = render_points[3].position;
		render_points[1].position.z += 0.8f;
		render_points[2].position.z += 0.8f;
		vector_from_points3d(&render_points[0].position, &render_points[1].position, &v[0]);
		vector_from_points3d(&render_points[1].position, &render_points[2].position, &v[1]);
		cross_product3d(&v[0], &v[1], &n);
		normalize3d(&n);
		render_points[3].normal = n;
		render_points[2].normal = render_points[3].normal;
		render_points[1].normal = render_points[2].normal;
		render_points[0].normal = render_points[1].normal;
		render_points[0].texcoord.x = u0 * texture_scale;
		render_points[0].texcoord.y = 1.f;
		render_points[1].texcoord.x = u0 * texture_scale;
		render_points[1].texcoord.y = 0.2f;
		render_points[2].texcoord.x = u1 * texture_scale;
		render_points[2].texcoord.y = 0.2f;
		render_points[3].texcoord.x = u1 * texture_scale;
		render_points[3].texcoord.y = 1.f;
		u0 = u1;
		render_dynamic_quad(render_points, shader_index, NULL, NULL, 1.f / texture_scale, 1.f);
	}

	return;
}

static boolean player_inside_hill(
	long player_index)
{
	boolean inside = FALSE;

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);
			real_point3d *position = &unit->object.bounding_sphere_center;

			if (position->z >= king_globals.hill_bottom && position->z <= king_globals.hill_top)
			{
				real_point2d point2d;

				point2d.x = position->x;
				point2d.y = position->y;
				inside = convex_hull2d_test_point((short)king_globals.hill_point_count, king_globals.convex_hull, &point2d, 0.f);
			}
		}
	}

	return inside;
}

static void king_engine_player_update(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	long unit_index;
	struct unit_datum *unit;

	game_engine_state_message(player_index, NONE, NONE);
	king_globals.on_the_hill[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)] = FALSE;

	unit_index = player->unit_index;
	if (unit_index == NONE)
	{
		return;
	}

	unit = unit_get(unit_index);
	if (game_engine_can_score() && player_inside_hill(player_index))
	{
		king_globals.on_the_hill[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)] = TRUE;
		player->statistics.multiplayer_statistics.king_statistics.time_on_hill++;

		if (king_globals.score_tick[player->team_index] < game_time_get())
		{
			long maximum_score;
			long score;

			king_globals.score[player->team_index]++;
			king_globals.score_tick[player->team_index] = game_time_get();
			maximum_score = game_engine_get_variant()->universal_variant.score_to_win * TICKS_PER_MINUTE;
			score = king_globals.score[player->team_index];

			if (maximum_score - score == 30 * TICKS_PER_SECOND)
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

			if (maximum_score - score == 60 * TICKS_PER_SECOND)
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

			if (king_globals.score[player->team_index] > 0 && king_globals.score[player->team_index] % (5 * TICKS_PER_SECOND) == 0 && king_globals.score[player->team_index] < maximum_score)
			{
				game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_timer_end);
			}

			if (score >= maximum_score)
			{
				game_engine_end_game();
			}
		}

		game_engine_state_message(player_index, king_message_you_are_on_the_hill, player_index);
	}

	return;
}

static void king_calculate_hill_state(
	void)
{
	if (game_engine_has_teams())
	{
		long red_count = 0;
		long blue_count = 0;
		struct data_iterator player_iterator;
		struct player_datum *player;

		data_iterator_new(&player_iterator, player_data);

		while (player = (struct player_datum *)data_iterator_next(&player_iterator))
		{
			if (king_globals.on_the_hill[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_iterator.index)])
			{
				if (player->team_index)
				{
					blue_count++;
				}
				else
				{
					red_count++;
				}
			}
		}

		if (blue_count && red_count)
		{
			king_globals.hill_state = king_hill_contested;

			if (king_globals.hill_controlled_count > 10 * TICKS_PER_SECOND)
			{
				game_engine_play_multiplayer_sound(_multiplayer_sound_hill_contested);
			}

			king_globals.hill_controlled_count = 0;
		}
		else if (blue_count)
		{
			if (king_globals.hill_state == king_hill_controlled_blue)
			{
				king_globals.hill_controlled_count++;
			}
			else
			{
				king_globals.hill_controlled_count = 0;
			}

			king_globals.hill_state = king_hill_controlled_blue;
		}
		else if (red_count)
		{
			if (king_globals.hill_state == king_hill_controlled_red)
			{
				king_globals.hill_controlled_count++;
			}
			else
			{
				king_globals.hill_controlled_count = 0;
			}

			king_globals.hill_state = king_hill_controlled_red;
		}
		else
		{
			king_globals.hill_state = king_hill_uncontrolled;
			king_globals.hill_controlled_count = 0;
		}

		if (king_globals.hill_controlled_count == 10 * TICKS_PER_SECOND)
		{
			game_engine_play_multiplayer_sound(_multiplayer_sound_hill_controlled);
		}
	}
	else
	{
		long count = 0;
		long controller;
		struct data_iterator player_iterator;
		struct player_datum *player;

		data_iterator_new(&player_iterator, player_data);

		while (player = (struct player_datum *)data_iterator_next(&player_iterator))
		{
			if (king_globals.on_the_hill[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_iterator.index)])
			{
				controller = player_iterator.index;
				count++;
			}
		}

		if (count > 1)
		{
			king_globals.hill_state = king_hill_contested;

			if (king_globals.hill_controlled_count > 10 * TICKS_PER_SECOND)
			{
				game_engine_play_multiplayer_sound(_multiplayer_sound_hill_contested);
			}

			king_globals.hill_controlled_count = 0;
			king_globals.hill_previous_controller = NONE;
		}
		else if (count)
		{
			if (king_globals.hill_state == king_hill_controlled && controller == king_globals.hill_previous_controller)
			{
				king_globals.hill_controlled_count++;
			}
			else
			{
				king_globals.hill_controlled_count = 0;
				king_globals.hill_previous_controller = controller;
			}

			king_globals.hill_state = king_hill_controlled;
		}
		else
		{
			king_globals.hill_state = king_hill_uncontrolled;
			king_globals.hill_controlled_count = 0;
			king_globals.hill_previous_controller = NONE;
		}

		if (king_globals.hill_controlled_count == 10 * TICKS_PER_SECOND)
		{
			game_engine_play_multiplayer_sound(_multiplayer_sound_hill_controlled);
		}
	}

	return;
}

static void king_engine_update(
	void)
{
	if (game_engine_can_score() && game_engine_get_variant()->game_engine_variant.king.moving_hill && --king_globals.hill_timer == 0)
	{
		king_globals.hill_timer = TICKS_PER_MINUTE;

		while (TRUE)
		{
			king_globals.hill_id = find_next_hill(king_globals.hill_id);
			find_hill();
			game_engine_play_multiplayer_sound(_multiplayer_sound_hill_move);

			if (king_globals.hill_point_count)
			{
				break;
			}

			error(_error_silent, "failed to find hill #%d most likely bad point placement", king_globals.hill_id);

			if (king_globals.hill_id == 0)
			{
				break;
			}
		}
	}

	if (king_globals.hill_point_count > 0)
	{
		real_point3d position = king_globals.hill_center;
		real vertical_offset = 0.f;

		game_engine_set_goal_position(0, &position, vertical_offset, "crown_blue", NONE, NONE, NONE);
	}
	else
	{
		console_printf(FALSE, "FAILED TO FIND HILL");
	}

	king_calculate_hill_state();

	return;
}

static void king_engine_player_damaged_player(
	long killing_player_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

static void king_engine_player_killed_player(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

static boolean king_engine_display_score(
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
	case king_message_enemy_on_the_hill:
	case king_message_ally_on_the_hill:
	case king_message_you_are_on_the_hill:
	{
		struct player_datum *message_player = player_get(message_data);
		long seconds = king_globals.score[message_player->team_index] / TICKS_PER_SECOND;

		if (message == king_message_you_are_on_the_hill)
		{
			wchar_t *place_name = get_place_name(game_engine_get_place(player_index, _get_score_team));

			multiplayer_game_text_get_string(string, _string_place_score_seconds);
			usnprintf(buffer, buffer_size, string, place_name, seconds);
		}
		else if (message == king_message_ally_on_the_hill)
		{
			multiplayer_game_text_get_string(string, _string_ally_name_is_on_the_hill_score_seconds);
			usnprintf(buffer, buffer_size, string, message_player->name, seconds);
		}
		else if (message == king_message_enemy_on_the_hill)
		{
			multiplayer_game_text_get_string(string, _string_enemy_name_is_on_the_hill_score_seconds);
			usnprintf(buffer, buffer_size, string, message_player->name, seconds);
		}
		else
		{
			match_unreachable("c:\\halo\\SOURCE\\game\\game_engine_king.c", 930);
		}
		break;
	}
	default:
		result = FALSE;
		break;
	}

	return result;
}

static void king_engine_prespawn_player_update(
	long player_index)
{
	return;
}

static long king_get_score(
	long player_index,
	enum get_score_type get_score_type)
{
	struct player_datum *player = player_get(player_index);
	long score;

	if (get_score_type == _get_score_individual)
	{
		score = player->statistics.multiplayer_statistics.king_statistics.time_on_hill;
	}
	else
	{
		score = king_globals.score[player->team_index];
	}

	return score;
}

void render_dynamic_quad_initialize(
	void)
{
	long index;

	for (index = 0; index < NUMBEROF(global_default_animation_colors); index++)
	{
		global_default_animation_colors[index] = *global_real_rgb_white;
	}

	for (index = 0; index < NUMBEROF(global_default_animation_values); index++)
	{
		global_default_animation_values[index] = 0.f;
	}

	return;
}

void render_dynamic_quad(
	struct model_vertex_uncompressed *verts,
	long shader_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation,
	real u_scale,
	real v_scale)
{
	long dynamic_triangle_buffer_index;
	long dynamic_vertex_buffer_index;

	rasterizer_globals.current_lock_operation = _rasterizer_lock_koth;
	dynamic_triangle_buffer_index = rasterizer_dynamic_triangles_new(2);
	dynamic_vertex_buffer_index = rasterizer_dynamic_vertices_new(_rasterizer_vertex_type_model_compressed, 4);

	if (dynamic_triangle_buffer_index != NONE && dynamic_vertex_buffer_index != NONE)
	{
		struct model_vertex_compressed *vertices = rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);
		struct rasterizer_triangle *triangles = rasterizer_dynamic_triangles_lock(dynamic_triangle_buffer_index);
		struct shader *shader;
		struct rasterizer_model_begin_parameters parameters;
		real_point3d centroid;

		rasterizer_geometry_compress_vertices(_rasterizer_vertex_type_model_uncompressed, 4, vertices, 4 * sizeof(*vertices), verts, 4 * sizeof(*verts));
		triangles[0].vertex_indices[0] = 0;
		triangles[0].vertex_indices[1] = 1;
		triangles[0].vertex_indices[2] = 2;
		triangles[1].vertex_indices[0] = 2;
		triangles[1].vertex_indices[1] = 3;
		triangles[1].vertex_indices[2] = 0;
		rasterizer_dynamic_triangles_unlock(dynamic_triangle_buffer_index);
		rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);
		shader = tag_get(SHADER_DEFINITION_TAG, shader_index);
		centroid.x = (verts[0].position.x + verts[1].position.x + verts[2].position.x + verts[3].position.x) * 0.25f;
		centroid.y = (verts[0].position.y + verts[1].position.y + verts[2].position.y + verts[3].position.y) * 0.25f;
		centroid.z = (verts[0].position.z + verts[1].position.z + verts[2].position.z + verts[3].position.z) * 0.25f;
		memset(&parameters, 0, sizeof(parameters));
		parameters.unique_id = 1;
		parameters.skinning.node_matrix_count = 1;
		parameters.skinning.node_matrices = (real_matrix4x3 *)global_identity4x3;

		if (lighting)
		{
			parameters.lighting = *lighting;
		}
		else
		{
			parameters.lighting.ambient_color = *global_real_rgb_white;
			parameters.lighting.distant_light_count = 0;
			parameters.lighting.point_light_count = 0;
			parameters.lighting.reflection_tint_color = *global_real_argb_white;
			parameters.lighting.shadow_vector.i = 0.f;
			parameters.lighting.shadow_vector.j = 1.f;
			parameters.lighting.shadow_vector.k = 0.f;
			parameters.lighting.shadow_color = *global_real_rgb_black;
		}

		if (animation)
		{
			parameters.animation = *animation;
		}
		else
		{
			parameters.animation.colors = global_default_animation_colors;
			parameters.animation.values = global_default_animation_values;
		}

		parameters.centroid = centroid;
		parameters.base_map_scale.i = u_scale;
		parameters.base_map_scale.j = v_scale;
		rasterizer_profile_enable(FALSE);
		rasterizer_models_begin(FALSE);
		rasterizer_model_begin(&parameters, TRUE);

		if (shader_type_is_transparent(shader->base.type))
		{
			rasterizer_model_transparent_geometry_submit(shader, 0, NULL, dynamic_triangle_buffer_index, 2, NULL, dynamic_vertex_buffer_index, &centroid, NULL);
		}
		else
		{
			rasterizer_model_draw(shader, 0, NULL, dynamic_triangle_buffer_index, 2, NULL, dynamic_vertex_buffer_index);
		}

		rasterizer_model_end();
		rasterizer_models_end();
		rasterizer_profile_enable(TRUE);
		rasterizer_dynamic_triangles_delete(dynamic_triangle_buffer_index);
		rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);
	}

	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}

static word *king_get_score_string(
	long player_index,
	word *buffer)
{
	struct player_datum *player = player_get(player_index);

	ticks_to_unicode_time_string(player->statistics.multiplayer_statistics.king_statistics.time_on_hill, 256, buffer);

	return buffer;
}

static word *king_get_score_header_string(
	word *buffer)
{
	wchar_t *string;

	multiplayer_game_text_get_string(string, _string_time);
	ustrcpy(buffer, string);

	return buffer;
}

static word *king_get_team_score_string(
	long team_index,
	word *buffer)
{
	ticks_to_unicode_time_string(king_globals.score[team_index], 256, buffer);

	return buffer;
}

static boolean king_engine_goal_matches_player(
	long player_index,
	long goal_index)
{
	return (boolean)!king_globals.on_the_hill[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)];
}

