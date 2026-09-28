/*
HUD_NAV_POINTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hud_definitions.h"
#include "hud.h"
#include "game_state.h"
#include "game.h"
#include "players.h"
#include "network_connection.h"
#include "game_engine.h"
#include "ai.h"
#include "object_definitions.h"
#include "object_types.h"
#include "cluster_partitions.h"
#include "objects.h"
#include "units.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "render_cameras.h"
#include "structures.h"
#include "structure_render.h"
#include "render.h"
#include "collisions.h"
#include "collision_usage.h"
#include "bitmaps_inlines.h"
#include "texture_cache.h"

/* ---------- constants */

enum
{
	_nav_point_flag,
	_nav_point_object,
	_nav_point_game_engine_flag,
	NUMBER_OF_NAV_POINT_TYPES,
};

enum
{
	MAXIMUM_ACTIVE_NAV_POINTS = 4,
};

/* ---------- structures */

struct hud_nav_point_datum
{
	short nav_index;
	short type : 4;
	short screen_type : 4;
	real z_offset;
	long reference_index;
};

struct hud_nav_point_player_datum
{
	struct hud_nav_point_datum nav_points[MAXIMUM_ACTIVE_NAV_POINTS];
};

/* ---------- prototypes */

struct hud_nav_point_player_datum *get_nav_point_datum(short local_player_index);
static void hud_activate_nav_point(short nav_index, long player_index, short type, long reference_index, real vertical_offset);
static void hud_activate_team_nav_point(short nav_index, short team_index, short type, long reference_index, real vertical_offset);
static void hud_activate_global_nav_point(short nav_index, short type, short reference_index, real vertical_offset);
static void hud_deactivate_nav_point(long player_index, short type, long reference_index);
static void hud_deactivate_team_nav_point(short team_index, short type, long reference_index);
static void hud_update_nav_point_local_player(short local_player_index);

/* ---------- globals */

static struct hud_nav_point_player_datum *nav_point_data;

/* ---------- public code */

short find_nav_point(
	char const *name)
{
	short nav_index = NONE;

	if (hud_globals)
	{
		short index;
		for (index = 0; index < hud_globals->waypoint.arrows.count; index++)
		{
			struct hud_waypoint_arrow *arrow = TAG_BLOCK_GET_ELEMENT(&hud_globals->waypoint.arrows, index, struct hud_waypoint_arrow);
			if (!_stricmp(name, arrow->name))
			{
				nav_index = index;
				break;
			}
		}
	}

	if (nav_index == NONE)
	{
		error(_error_silent, "could not find nav point");
	}

	return nav_index;
}

struct hud_nav_point_player_datum *get_nav_point_datum(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 95, local_player_index>=0&&local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 96, nav_point_data);

	return &nav_point_data[local_player_index];
}

void hud_initialize_nav_points(
	void)
{
	nav_point_data = game_state_malloc("hud nav points", NULL, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS * sizeof(struct hud_nav_point_player_datum));

	match_assert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 106, nav_point_data);

	return;
}

void hud_initialize_nav_points_for_new_map(
	void)
{
	memset(nav_point_data, NONE, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS * sizeof(struct hud_nav_point_player_datum));

	return;
}

void hud_dispose_nav_points_from_old_map(
	void)
{
	return;
}

void hud_dispose_nav_points(
	void)
{
	return;
}

void hud_activate_nav_point_with_game_engine_flag(
	short nav_index,
	long player_index,
	short flag_index,
	real vertical_offset)
{
	hud_activate_nav_point(nav_index, player_index, _nav_point_game_engine_flag, flag_index, vertical_offset);

	return;
}

void hud_activate_nav_point_with_flag(
	short nav_index,
	long player_index,
	short flag_index,
	real vertical_offset)
{
	hud_activate_nav_point(nav_index, player_index, _nav_point_flag, flag_index, vertical_offset);

	return;
}

void hud_activate_nav_point_with_object(
	short nav_index,
	long player_index,
	long object_index,
	real vertical_offset)
{
	hud_activate_nav_point(nav_index, player_index, _nav_point_object, object_index, vertical_offset);

	return;
}

void hud_activate_team_nav_point_with_game_engine_flag(
	short nav_index,
	short team_index,
	short flag_index,
	real vertical_offset)
{
	hud_activate_team_nav_point(nav_index, team_index, _nav_point_game_engine_flag, flag_index, vertical_offset);

	return;
}

void hud_activate_team_nav_point_with_flag(
	short nav_index,
	short team_index,
	short flag_index,
	real vertical_offset)
{
	hud_activate_team_nav_point(nav_index, team_index, _nav_point_flag, flag_index, vertical_offset);

	return;
}

void hud_activate_team_nav_point_with_object(
	short nav_index,
	short team_index,
	long object_index,
	real vertical_offset)
{
	hud_activate_team_nav_point(nav_index, team_index, _nav_point_object, object_index, vertical_offset);

	return;
}

void hud_activate_global_nav_point_with_game_engine_flag(
	short nav_index,
	short flag_index,
	real vertical_offset)
{
	hud_activate_global_nav_point(nav_index, _nav_point_game_engine_flag, flag_index, vertical_offset);

	return;
}

void hud_deactivate_nav_point_with_game_engine_flag(
	long player_index,
	short flag_index)
{
	hud_deactivate_nav_point(player_index, _nav_point_game_engine_flag, flag_index);

	return;
}

void hud_deactivate_nav_point_with_flag(
	long player_index,
	short flag_index)
{
	hud_deactivate_nav_point(player_index, _nav_point_flag, flag_index);

	return;
}

void hud_deactivate_nav_point_with_object(
	long player_index,
	long object_index)
{
	hud_deactivate_nav_point(player_index, _nav_point_object, object_index);

	return;
}

void hud_deactivate_team_nav_point_with_flag(
	short team_index,
	short flag_index)
{
	hud_deactivate_team_nav_point(team_index, _nav_point_flag, flag_index);

	return;
}

void hud_deactivate_team_nav_point_with_object(
	short team_index,
	long object_index)
{
	hud_deactivate_team_nav_point(team_index, _nav_point_object, object_index);

	return;
}

void hud_update_nav_points(
	void)
{
	short local_player_index;

	for (local_player_index = local_player_get_next(NONE);
		local_player_index != NONE;
		local_player_index = local_player_get_next(local_player_index))
	{
		hud_update_nav_point_local_player(local_player_index);
	}
	return;
}

short hud_get_nav_point_render_type(
	short local_player_index,
	real_point3d const *head,
	real_point3d const *position,
	long reference_object_index)
{
	struct collision_result collision;
	short result;

	match_collision_log_begin_user("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 510, _collision_user_ui);

	if (!collision_test_line(
		_collision_test_for_line_of_sight_flags,
		head,
		position,
		local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index,
		&collision) ||
		(collision.type == _collision_result_object && collision.object_index == reference_object_index))
	{
		result = _waypoint_on_screen;
	}
	else
	{
		result = _waypoint_occluded;
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 528);

	return result;
}

void custom_render_nav_point(
	short local_player_index,
	real_point3d const *position_pointer,
	short nav_index,
	short waypoint_type)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct hud_waypoint_arrow *arrow;
	real_point3d position;
	real distance;
	real scale;
	real_point2d screen_position;
	real a, b, ab, bx, ay;
	real theta;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));
	arrow = TAG_BLOCK_GET_ELEMENT(&hud_globals->waypoint.arrows, nav_index, struct hud_waypoint_arrow);
	position = *position_pointer;

	{
		real_point3d cam_pos;

		unit_get_camera_position(local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index, &cam_pos);
		distance = distance3d(&cam_pos, position_pointer);
	}

	if (distance > 15.f)
	{
		scale = 0.5f;
	}
	else
	{
		scale = (real)pow(1.f - distance / 15.f, 0.7) * 1.f + 0.5f;
	}

	matrix4x3_transform_point(&render.frustum.world_to_view, &position, &position);
	if (waypoint_type == _waypoint_off_screen || !render_camera_view_to_screen(&render.camera, &render.frustum, &position, &screen_position))
	{
		screen_position.x = position.x;
		screen_position.y = -position.y;
		waypoint_type = _waypoint_off_screen;
	}
	else
	{
		screen_position.x -= render.camera.viewport_bounds.x0 + (render.camera.viewport_bounds.x1 - render.camera.viewport_bounds.x0) / 2;
		screen_position.y -= render.camera.viewport_bounds.y0 + (render.camera.viewport_bounds.y1 - render.camera.viewport_bounds.y0) / 2;
	}

	a = ((render.camera.window_bounds.x1 - render.camera.window_bounds.x0) - (hud_globals->waypoint.right_offset + hud_globals->waypoint.left_offset)) / 2.f;
	b = ((render.camera.window_bounds.y1 - render.camera.window_bounds.y0) - (hud_globals->waypoint.bottom_offset + hud_globals->waypoint.top_offset)) / 2.f;
	ab = a * b;
	bx = b * screen_position.x;
	ay = a * screen_position.y;
	theta = 0.f;

	if (waypoint_type == _waypoint_off_screen || ab * ab <= bx * bx + ay * ay)
	{
		real factor = square_root(ab * ab / (bx * bx + ay * ay));

		screen_position.x *= factor;
		screen_position.y *= factor;
		waypoint_type = _waypoint_off_screen;

		if (!TEST_FLAG(arrow->flags, _hud_waypoint_dont_rotate_offscreen))
		{
			theta = -arctangent(screen_position.x, screen_position.y);
		}
	}

	screen_position.x += (render.camera.viewport_bounds.x1 - render.camera.viewport_bounds.x0) / 2;
	screen_position.y += (render.camera.viewport_bounds.y1 - render.camera.viewport_bounds.y0) / 2;
	match_assert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 615, waypoint_type!=NONE);
	{
		long bitmap_index = hud_globals->waypoint.arrow_bitmap.index;
		struct bitmap_data const *bitmap = NULL;
		real_rectangle2d const *clip = NULL;

		hud_retrieve_bitmap_and_bounding_rect(bitmap_index, arrow->sequence_indices[waypoint_type], 0, &bitmap, &clip);
		if (bitmap && _texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
		{
			point2d corner;
			byte alpha;
			real_rgb_color rgb_temp;

			corner.x = (short)screen_position.x;
			corner.y = (short)screen_position.y;
			alpha = PIN(255 * fast_ftol_C(arrow->opacity), 0, 255);
			pixel32_to_real_rgb_color(arrow->color, &rgb_temp);
			rgb_temp.red *= PIN(1.f - arrow->fade, 0.f, 1.f);
			rgb_temp.green *= PIN(1.f - arrow->fade, 0.f, 1.f);
			rgb_temp.blue *= PIN(1.f - arrow->fade, 0.f, 1.f);
			hud_draw_bitmap_direct(bitmap, _hud_center, &corner, clip, scale, theta, (alpha << 24) | real_rgb_color_to_pixel32(&rgb_temp), FALSE);

			if (waypoint_type != _waypoint_off_screen)
			{
				struct hud_absolute_placement_definition placement;
				struct number_hud_element_definition numbers;
				real fractional_value;

				distance *= 3.048f;
				memset(&placement, 0, sizeof(struct hud_absolute_placement_definition));
				memset(&numbers, 0, sizeof(struct number_hud_element_definition));
				placement.corner = _hud_top_left;
				numbers.colors.color = (alpha << 24) | real_rgb_color_to_pixel32(&rgb_temp);
				numbers.colors.flash_color = (alpha << 24) | real_rgb_color_to_pixel32(&rgb_temp);
				numbers.digits = 3;
				numbers.fractional_digits = 1;
				numbers.number_flags = FLAG(_hud_number_show_all_leading_zeros_bit) | FLAG(_hud_number_show_trailing_m);

				numbers.placement.offset.x = corner.x + (bitmap->width * (clip->x1 - clip->x0) / 2.f) * scale * 0.33f;
				numbers.placement.offset.y = corner.y + (bitmap->height * (clip->y1 - clip->y0) / 2.f) * scale * 0.66f;
				numbers.placement.offset.x -= render.camera.window_bounds.x0 - render.camera.viewport_bounds.x0;
				numbers.placement.offset.y -= render.camera.window_bounds.y0 - render.camera.viewport_bounds.y0;

				fractional_value = power(10.f, 4.f);
				hud_draw_numbers(local_player_index, &placement, &numbers,
					fast_ftol_C(distance), fast_ftol(fmod(fabs((real)(fractional_value * distance)), fractional_value)),
					0, 0.f, 0.f);
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);
		match_vassert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 675, return_eip == get_return_eip(), "corrupt return address!");
		match_vassert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 675, corrupt_index == NONE, csprintf(temporary, "corrupt stack at %d!", corrupt_index));
	}

	return;
}

void hud_render_nav_points(
	short local_player_index)
{
	real radius;

	if (local_player_index != NONE &&
		(local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index) != NONE &&
		hud_globals->waypoint.arrow_bitmap.index != NONE)
	{
		struct hud_nav_point_player_datum *data = get_nav_point_datum(local_player_index);
		short index;

		for (index = 0; index < MAXIMUM_ACTIVE_NAV_POINTS; index++)
		{
			struct hud_nav_point_datum *nav = &data->nav_points[index];

			if (nav->nav_index == NONE || nav->reference_index == NONE || nav->type == NONE)
			{
				nav->type = NONE;
			}
			else
			{
				real_point3d position;

				switch (nav->type)
				{
				case _nav_point_flag:
					position = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->cutscene_flags, nav->reference_index, struct scenario_cutscene_flag)->position;
					break;
				case _nav_point_object:
					if (object_try_and_get(nav->reference_index))
					{
						object_get_bounding_sphere(nav->reference_index, &position, &radius);
					}
					else
					{
						continue;
					}
					break;
				case _nav_point_game_engine_flag:
					position = game_engine_get_goal_position((short)nav->reference_index);
					break;
				default:
					match_unreachable("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 725);
					break;
				}
				position.z += nav->z_offset;
				custom_render_nav_point(local_player_index, &position, nav->nav_index, nav->screen_type);
			}
		}
	}

	game_engine_render_nav_points(local_player_index);

	return;
}

void hud_unit_activate_nav_point_with_flag(
	short nav_index,
	long unit_index,
	short flag_index,
	real vertical_offset)
{
	long player_index = player_index_from_unit_index(unit_index);

	if (player_index != NONE)
	{
		hud_activate_nav_point_with_flag(nav_index, player_index, flag_index, vertical_offset);
	}

	return;
}

void hud_unit_activate_nav_point_with_object(
	short nav_index,
	long unit_index,
	long object_index,
	real vertical_offset)
{
	long player_index = player_index_from_unit_index(unit_index);

	if (player_index != NONE)
	{
		hud_activate_nav_point_with_object(nav_index, player_index, object_index, vertical_offset);
	}

	return;
}

void hud_unit_deactivate_nav_point_with_flag(
	long unit_index,
	short flag_index)
{
	long player_index = player_index_from_unit_index(unit_index);

	if (player_index != NONE)
	{
		hud_deactivate_nav_point_with_flag(player_index, flag_index);
	}

	return;
}

void hud_unit_deactivate_nav_point_with_object(
	long unit_index,
	long object_index)
{
	long player_index = player_index_from_unit_index(unit_index);

	if (player_index != NONE)
	{
		hud_deactivate_nav_point_with_object(player_index, object_index);
	}

	return;
}

/* ---------- private code */

static void hud_activate_nav_point(
	short nav_index,
	long player_index,
	short type,
	long reference_index,
	real vertical_offset)
{
	if (player_index != NONE)
	{
		short local_player_index = player_get(player_index)->local_player_index;

		if (VALID_INDEX(local_player_index, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS) && reference_index != NONE && nav_index != NONE)
		{
			struct hud_nav_point_player_datum *data = get_nav_point_datum(local_player_index);
			short index;
			short free_index = NONE;

			for (index = 0; index < MAXIMUM_ACTIVE_NAV_POINTS; index++)
			{
				struct hud_nav_point_datum *nav = &data->nav_points[index];

				if (nav->type == type && nav->reference_index == reference_index)
				{
					nav->nav_index = nav_index;
					nav->z_offset = vertical_offset;
					return;
				}
				if (nav->type == NONE)
				{
					free_index = index;
				}
			}

			if (free_index != NONE)
			{
				struct hud_nav_point_datum *nav = &data->nav_points[free_index];
				nav->type = type;
				nav->reference_index = reference_index;
				nav->nav_index = nav_index;
				nav->z_offset = vertical_offset;
			}
			else
			{
				error(_error_silent, "Could not add another nav point");
			}
		}
	}

	return;
}

static void hud_activate_team_nav_point(
	short nav_index,
	short team_index,
	short type,
	long reference_index,
	real vertical_offset)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->local_player_index != NONE && team_index == player->team_index)
		{
			hud_activate_nav_point(nav_index, iterator.index, type, reference_index, vertical_offset);
		}
	}

	return;
}

static void hud_activate_global_nav_point(
	short nav_index,
	short type,
	short reference_index,
	real vertical_offset)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->local_player_index != NONE)
		{
			hud_activate_nav_point(nav_index, iterator.index, type, reference_index, vertical_offset);
		}
	}

	return;
}

static void hud_deactivate_nav_point(
	long player_index,
	short type,
	long reference_index)
{
	if (player_index != NONE)
	{
		short local_player_index = player_get(player_index)->local_player_index;

		if (VALID_INDEX(local_player_index, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS) && reference_index != NONE)
		{
			struct hud_nav_point_player_datum *data = get_nav_point_datum(local_player_index);
			short index;

			for (index = 0; index < MAXIMUM_ACTIVE_NAV_POINTS; index++)
			{
				struct hud_nav_point_datum *nav = &data->nav_points[index];

				if (nav->type == type && nav->reference_index == reference_index)
				{
					nav->type = NONE;
					nav->reference_index = NONE;
					nav->nav_index = NONE;
					break;
				}
			}
		}
	}

	return;
}

static void hud_deactivate_team_nav_point(
	short team_index,
	short type,
	long reference_index)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->local_player_index != NONE && team_index == player->team_index)
		{
			hud_deactivate_nav_point(iterator.index, type, reference_index);
		}
	}

	return;
}

static void hud_update_nav_point_local_player(
	short local_player_index)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct hud_nav_point_player_datum *data;
	long unit_index;
	short index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));
	data = get_nav_point_datum(local_player_index);
	unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;
	for (index = 0; index < MAXIMUM_ACTIVE_NAV_POINTS; index++)
	{
		struct hud_nav_point_datum *nav = &data->nav_points[index];

		if (nav->nav_index == NONE || nav->reference_index == NONE || nav->type == NONE)
		{
			nav->type = NONE;
		}
		else if (unit_index != NONE)
		{
			real_point3d head, position;
			long reference_object_index = NONE;

			unit_get_head_position(unit_index, &head);
			switch (nav->type)
			{
			case _nav_point_flag:
				position = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->cutscene_flags, nav->reference_index, struct scenario_cutscene_flag)->position;
				break;
			case _nav_point_object:
				{
					struct object_datum *object = object_try_and_get(nav->reference_index);
					real radius;

					reference_object_index = nav->reference_index;
					if (!object || TEST_FLAG(object->object.damage_flags, _object_dead_bit))
					{
						nav->type = NONE;
						nav->reference_index = NONE;
						nav->nav_index = NONE;
						continue;
					}
					object_get_bounding_sphere(reference_object_index, &position, &radius);
				}
				break;
			case _nav_point_game_engine_flag:
				position = game_engine_get_goal_position((short)nav->reference_index);
				break;
			}

			position.z += nav->z_offset;
			nav->screen_type = hud_get_nav_point_render_type(local_player_index, &head, &position, reference_object_index);
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);
		match_vassert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 496, return_eip == get_return_eip(), "corrupt return address!");
		match_vassert("c:\\halo\\SOURCE\\interface\\hud_nav_points.c", 496, corrupt_index == NONE, csprintf(temporary, "corrupt stack at %d!", corrupt_index));
	}

	return;
}
