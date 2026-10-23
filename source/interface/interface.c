/*
INTERFACE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "interface.h"
#include "meter_definitions.h"
#include "ui_widget.h"
#include "hud.h"
#include "terminal.h"
#include "first_person_weapons.h"
#include "weapon_hud_interface_definition.h"
#include "motion_sensor.h"
#include "interface_panels.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "network_messages.h"
#include "rasterizer/common/rasterizer_common.h"
#include "collision_usage.h"
#include "weapon_interface_definitions.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "director.h"
#include "weapons.h"
#include "font_group.h"
#include "effects.h"
#include "cinematics.h"
#include "ai_profile.h"
#include "particles.h"
#include "rasterizer_cinematics.h"
#include "color_table_group.h"
#include "dialogs.h"

/* ---------- constants */

enum
{
	MAXIMUM_PROFILE_VALUES = 64,					/* fake name */
	MAXIMUM_PROFILE_GRAPH_POINTS = 512,				/* fake name */
	MAXIMUM_PROFILE_GRAPH_MESSAGES = 48,			/* fake name */
	NUMBER_OF_RASTERIZER_STALL_TYPES = 27,			/* fake name */
	PROFILE_DISPLAY_BUFFER_SIZE = 8192				/* fake name */
};

/* ---------- macros */

#define PROFILE_GRAPH_SECONDS 10.f	/* fake name */
#define PROFILE_GRAPH_SCALE 0.1f	/* fake name */

/* ---------- structures */

struct profile_value /* fake name */
{
	char name[256];
	char label[256];
	real_argb_color const **color;
	short frame_value;
	short section_index;
	boolean subtract_previous;
	boolean enabled;
};

/* ---------- prototypes */

static long interface_get_weapon_hud_index(real *flashlight_power);
static void render_debug_profile_stall_tick(short stall_type, real_rectangle2d const *bounds, real *below_position, real x, real scale);
static void render_debug_profile(void);
static void interface_splitscreen_render(void);

/* ---------- globals */

short profile_game_value_count = 3;
struct profile_value profile_game_values[MAXIMUM_PROFILE_VALUES] =
{
	{ "game", "game", &global_real_argb_yellow, NONE, NONE, FALSE, TRUE },
	{ "objects_update", "objects", &global_real_argb_green, NONE, NONE, FALSE, TRUE },
	{ "ai_update", "ai", &global_real_argb_blue, NONE, NONE, FALSE, TRUE }
};

short profile_frame_value_count = 1;
struct profile_value profile_frame_values[MAXIMUM_PROFILE_VALUES] =
{
	{ "frame", "frame", &global_real_argb_white, NONE, NONE, FALSE, TRUE }
};

short profile_graph_value_count = 14;
struct profile_value profile_graph_values[MAXIMUM_PROFILE_VALUES] =
{
	{ "stall", "stall", &global_real_argb_red, NONE, NONE, FALSE, TRUE },
	{ "texture", "texture", &global_real_argb_orange, NONE, NONE, FALSE, TRUE },
	{ "render0", "window0", &global_real_argb_blue, NONE, NONE, FALSE, TRUE },
	{ "render0_1", "window1", &global_real_argb_lightblue, NONE, NONE, TRUE, TRUE },
	{ "render0_2", "window2", &global_real_argb_cyan, NONE, NONE, TRUE, TRUE },
	{ "render0_3", "window3", &global_real_argb_purple, NONE, NONE, TRUE, TRUE },
	{ "render0_3np", "overlay", &global_real_argb_salmon, NONE, NONE, TRUE, TRUE },
	{ "render", "render", &global_real_argb_violet, NONE, NONE, TRUE, TRUE },
	{ "game_render", "game", &global_real_argb_yellow, NONE, NONE, TRUE, TRUE },
	{ "load", "load", &global_real_argb_magenta, NONE, NONE, TRUE, TRUE },
	{ "frame", "time", &global_real_argb_white, NONE, NONE, TRUE, TRUE },
	{ "gpu", "gpu", &global_real_argb_green, NONE, NONE, FALSE, TRUE },
	{ "pushbuffer", "pushbuffer", &global_real_argb_darkgreen, NONE, NONE, FALSE, TRUE },
	{ "dt", "dt", &global_real_argb_grey, NONE, NONE, FALSE, TRUE }
};

/* ---------- public code */

void interface_initialize(
	void)
{
	terminal_initialize();
	hud_initialize();
	draw_string_initialize();
	first_person_weapons_initialize();

	return;
}

void interface_initialize_for_new_map(
	void)
{
	hud_initialize_for_new_map();
	draw_string_initialize_for_new_map();
	first_person_weapons_initialize_for_new_map();
	draw_string_set_draw_mode(
		interface_get_tag_index(_interface_font_terminal),
		NONE,
		0,
		0,
		global_real_argb_white);

	return;
}

void interface_dispose_from_old_map(
	void)
{
	draw_string_dispose_from_old_map();
	hud_dispose_from_old_map();
	first_person_weapons_dispose_from_old_map();

	return;
}

void interface_dispose(
	void)
{
	draw_string_dispose();
	terminal_dispose();
	hud_dispose();
	first_person_weapons_dispose();

	return;
}

long interface_get_tag_index(
	short interface_tag_index)
{
	struct game_globals_interface_tag_references *interface_tag_references;

	match_assert("c:\\halo\\SOURCE\\interface\\interface.c", 109, interface_tag_index>=0 && interface_tag_index<NUMBER_OF_INTERFACE_TAGS);

	interface_tag_references = scenario_get_game_globals()->interface_tag_references.count ?
		TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->interface_tag_references, 0, struct game_globals_interface_tag_references) :
		NULL;

	return interface_tag_references->interface_tag_references[interface_tag_index].index;
}

argb_color *interface_get_rgb_color(
	short interface_color_table_index,
	short color_index,
	argb_color *color)
{
	real_argb_color real_color;

	interface_get_real_argb_color(interface_color_table_index, color_index, &real_color);
	color->n[0] = (word)(real_color.n[0] * UNSIGNED_SHORT_MAX);
	color->n[1] = (word)(real_color.n[1] * UNSIGNED_SHORT_MAX);
	color->n[2] = (word)(real_color.n[2] * UNSIGNED_SHORT_MAX);
	color->n[3] = (word)(real_color.n[3] * UNSIGNED_SHORT_MAX);

	return color;
}

real_argb_color *interface_get_real_argb_color(
	short interface_color_table_index,
	short color_index,
	real_argb_color *color)
{
	long color_table_index = interface_get_tag_index(interface_color_table_index);

	color->blue = 1.f;
	color->green = 1.f;
	color->red = 1.f;
	color->alpha = 1.f;

	if (color_table_index != NONE)
	{
		struct color_table *color_table = color_table_definition_get(color_table_index);

		if (color_table->colors.count)
		{
			color_index %= color_table->colors.count;
			*color = TAG_BLOCK_GET_ELEMENT(&color_table->colors, color_index, struct color_table_color)->real_color;
		}
	}

	return color;
}

void interface_set_bitmap_text_draw_mode(
	short interface_font_index,
	short style,
	short justification,
	unsigned long flags,
	short color_table_index,
	short color_index)
{
	real_argb_color color;
	long font_index = interface_get_tag_index(interface_font_index);

	interface_get_real_argb_color(color_table_index, color_index, &color);
	draw_string_set_draw_mode(font_index, style, justification, flags, &color);

	return;
}

static long interface_get_weapon_hud_index(
	real *flashlight_power)
{
	long player_index = local_player_get_player_index(render.local_player_index);
	long weapon_hud_index = NONE;
	real flashlight = 0.f;

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		short perspective = director_get_perspective(render.local_player_index);

		if (hud_scripted_globals &&
			hud_scripted_globals->show_hud &&
			perspective != _director_perspective_neutral &&
			perspective != _director_perspective_scripted &&
			player->unit_index != NONE)
		{
			long weapon_index = unit_inventory_get_weapon(player->unit_index, unit_get(player->unit_index)->unit.current_weapon_index);

			if (weapon_index == NONE)
			{
				struct unit_datum *unit = unit_get(player->unit_index);

				if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
				{
					struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
					struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition_get(parent_unit->definition_index)->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

					if (TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit))
					{
						weapon_index = unit_inventory_get_weapon(unit->object.parent_object_index, unit_get(unit->object.parent_object_index)->unit.current_weapon_index);
					}
				}
			}
			else
			{
				flashlight = unit_get(player->unit_index)->unit.integrated_night_vision_power;
			}

			if (weapon_index != NONE)
			{
				long hud_interface_index = weapon_definition_get(weapon_get(weapon_index)->definition_index)->weapon.interface_definition.hud_interface.index;

				if (hud_interface_index != NONE)
				{
					weapon_hud_index = hud_interface_index;
				}
				else if (!unit_get_weapon_count(player->unit_index))
				{
					weapon_hud_index = hud_globals->defaults.default_weapon_hud.index;
				}
			}
		}
	}

	*flashlight_power = flashlight;

	return weapon_hud_index;
}

void interface_draw_screen(
	void)
{
	if (render.local_player_index != NONE)
	{
		real flashlight_power;
		long weapon_hud_index = interface_get_weapon_hud_index(&flashlight_power);
		boolean screen_effect_set = FALSE;

		if (weapon_hud_index != NONE)
		{
			struct weapon_hud_interface_definition *weapon_hud_interface = weapon_hud_interface_definition_get(weapon_hud_index);

			if (weapon_hud_interface->screen_effects.count > 0)
			{
				struct rasterizer_screen_effect_parameters screen_effect;
				struct hud_screen_effect_definition *screen_effect_definition = TAG_BLOCK_GET_ELEMENT(&weapon_hud_interface->screen_effects, 0, struct hud_screen_effect_definition);
				long mask_index = NONE;
				boolean zoomed = player_control_get_zoom_level(render.local_player_index) != NONE;

				memset(&screen_effect, 0, sizeof(screen_effect));

				if (zoomed || !TEST_FLAG(screen_effect_definition->mask_flags, _hud_screen_effect_mask_only_when_zoomed_bit))
				{
					mask_index = main_get_window_count() <= 1 ?
						screen_effect_definition->mask_fullscreen.index :
						screen_effect_definition->mask_splitscreen.index;

					if (mask_index != NONE)
					{
						screen_effect.convolution_mask = TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(mask_index)->bitmaps, 0, struct bitmap_data);
						screen_effect.filter_light_enhancement_uses_convolution_mask = TEST_FLAG(screen_effect_definition->light_enhancement_flags, _hud_screen_effect_light_enhancement_uses_convolution_mask_bit);
						screen_effect.filter_desaturation_uses_convolution_mask = TEST_FLAG(screen_effect_definition->desaturation_flags, _hud_screen_effect_desaturation_uses_convolution_mask_bit);
					}
				}

				if (main_get_window_count() <= 1 &&
					(zoomed || !TEST_FLAG(screen_effect_definition->convolution_flags, _hud_screen_effect_convolution_only_when_zoomed_bit)))
				{
					real radius = 0.f;

					if (screen_effect_definition->convolution_radius_in_bounds[0] != screen_effect_definition->convolution_radius_in_bounds[1])
					{
						scalars_interpolate(
							screen_effect_definition->convolution_radius_out_bounds[0],
							screen_effect_definition->convolution_radius_out_bounds[1],
							PIN((render.camera.vertical_field_of_view - screen_effect_definition->convolution_radius_in_bounds[0]) / (screen_effect_definition->convolution_radius_in_bounds[1] - screen_effect_definition->convolution_radius_in_bounds[0]), 0.f, 1.f),
							&radius);
					}
					else
					{
						radius = screen_effect_definition->convolution_radius_out_bounds[1];
					}

					if (radius > 0.f)
					{
						screen_effect.convolution_type = _rasterizer_screen_effect_convolution_type_warp;
						screen_effect.convolution_radius = radius;
					}
				}

				if (zoomed || !TEST_FLAG(screen_effect_definition->light_enhancement_flags, _hud_screen_effect_light_enhancement_only_when_zoomed_bit))
				{
					real intensity = screen_effect_definition->light_enhancement_intensity;

					if (TEST_FLAG(screen_effect_definition->light_enhancement_flags, _hud_screen_effect_light_enhancement_connect_to_flashlight_bit))
					{
						intensity *= PIN(flashlight_power, 0.f, 1.f);
					}

					intensity *= PIN(rasterizer_script_screen_effect_get_value(screen_effect_definition->light_enhancement_script_source), 0.f, 1.f);

					if (intensity > 0.f)
					{
						screen_effect.filter_light_enhancement_intensity = intensity;
					}
				}

				if (zoomed || !TEST_FLAG(screen_effect_definition->desaturation_flags, _hud_screen_effect_desaturation_only_when_zoomed_bit))
				{
					real intensity = screen_effect_definition->desaturation_intensity;

					if (TEST_FLAG(screen_effect_definition->desaturation_flags, _hud_screen_effect_desaturation_connect_to_flashlight_bit))
					{
						intensity *= PIN(flashlight_power, 0.f, 1.f);
					}

					intensity *= PIN(rasterizer_script_screen_effect_get_value(screen_effect_definition->desaturation_script_source), 0.f, 1.f);

					if (intensity > 0.f)
					{
						screen_effect.filter_desaturation_intensity = intensity;
						screen_effect.filter_desaturation_is_additive = TEST_FLAG(screen_effect_definition->desaturation_flags, _hud_screen_effect_desaturation_is_additive_bit);
						screen_effect.filter_desaturation_tint = screen_effect_definition->desaturation_tint;
					}
				}

				rasterizer_screen_effect(&screen_effect);
				screen_effect_set = TRUE;
			}
		}

		if (!screen_effect_set)
		{
			rasterizer_screen_effect(NULL);
		}

		hud_draw_screen();
		game_engine_post_rasterize();
	}

	return;
}

void profile_graph_toggle(
	char const *graph_name)
{
	short graph_value_index;

	for (graph_value_index = 0; graph_value_index < profile_graph_value_count; graph_value_index++)
	{
		struct profile_value *graph_value = &profile_graph_values[graph_value_index];

		if (!_stricmp(graph_value->name, graph_name) || !_stricmp(graph_value->label, graph_name))
		{
			graph_value->enabled = !graph_value->enabled;
		}
	}

	return;
}

static void render_debug_profile_stall_tick(
	short stall_type,
	real_rectangle2d const *bounds,
	real *below_position,
	real x,
	real scale)
{
	real_argb_color const *color;
	real_point3d p0;
	real_point3d p1;

	switch (stall_type)
	{
	case 1:
		color = global_real_argb_blue;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
		color = global_real_argb_yellow;
		break;
	case 21:
		color = global_real_argb_green;
		break;
	case 26:
		color = global_real_argb_pink;
		break;
	default:
		color = global_real_argb_white;
		break;
	}

	p0.x = x * scale;
	p0.y = (bounds->y0 - (bounds->y1 - bounds->y0) * *below_position) * scale;
	p0.z = -scale;
	*below_position += 0.03f;
	p1.x = x * scale;
	p1.y = (bounds->y0 - (bounds->y1 - bounds->y0) * *below_position) * scale;
	p1.z = -scale;
	*below_position += 0.01f;
	matrix4x3_transform_point(&render.frustum.view_to_world, &p0, &p0);
	matrix4x3_transform_point(&render.frustum.view_to_world, &p1, &p1);
	rasterizer_debug_immediate_line(&p0, &p1, &color->rgb, NULL);

	return;
}

static void render_debug_profile(
	void)
{
	if (profile_display)
	{
		char buffer[PROFILE_DISPLAY_BUFFER_SIZE];

		interface_set_bitmap_text_draw_mode(_interface_font_terminal, NONE, 0, 0, _interface_color_table_dialog, 0);

		{
			struct objects_information objects_information;
			struct system_memory_information memory_information;
			struct effects_information effects_information;

			objects_information_get(&objects_information);
			system_memory_information_get(&memory_information);
			effects_information_get(&effects_information);
			strcpy(buffer, "");
			sprintf(
				buffer + strlen(buffer),
				"% 6.1fk free of % 6.1fk total|n",
				memory_information.free / 1024.f,
				memory_information.total / 1024.f);
			sprintf(
				buffer + strlen(buffer),
				"% 5d active of % 5d objects (% 3.1f%% used)|n",
				objects_information.active_object_count,
				objects_information.object_count,
				objects_information.used_memory * 100.f);
			sprintf(
				buffer + strlen(buffer),
				"% 5d active of % 5d effects (%5d locations)|n",
				effects_information.active_effect_count,
				effects_information.effect_count,
				effects_information.location_count);
			sprintf(
				buffer + strlen(buffer),
				"% 5d particles|n",
				particle_data->actual_count);
			ai_profile_display(buffer);
			collision_log_display(buffer);
			rasterizer_draw_string(NULL, NULL, NULL, 0, buffer);
		}

		{
			short tab_stops[4];

			tab_stops[0] = 300;
			tab_stops[1] = 380;
			tab_stops[2] = 460;
			tab_stops[3] = 540;
			sprintf(buffer, "|n|n|n|n|n|n");
			profile_dump(NULL, _profile_sort_total_time, _profile_dump_format_table, 10, buffer + strlen(buffer));
			draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
			rasterizer_draw_string(NULL, NULL, NULL, 0, buffer);
			draw_string_set_tab_stops(tab_stops, 0);
		}
	}

	if (profile_graph)
	{
		real_rectangle2d screen_clip_bounds;
		real_rectangle2d graph_bounds;
		real_rectangle2d graph_screen_bounds;

		render_frustum_get_projection_bounds(&render.frustum, &screen_clip_bounds);
		graph_bounds.x0 = screen_clip_bounds.x0 - PROFILE_GRAPH_SCALE;
		graph_bounds.x1 = screen_clip_bounds.x1 + PROFILE_GRAPH_SCALE;
		graph_bounds.y0 = screen_clip_bounds.y1 + PROFILE_GRAPH_SCALE;
		graph_bounds.y1 = screen_clip_bounds.y0 - PROFILE_GRAPH_SCALE;
		graph_screen_bounds.x0 = rasterizer_globals.screen_bounds.x1 - (rasterizer_globals.screen_bounds.x1 - rasterizer_globals.screen_bounds.x0) * PROFILE_GRAPH_SCALE;
		graph_screen_bounds.x1 = rasterizer_globals.screen_bounds.x0 + (rasterizer_globals.screen_bounds.x1 - rasterizer_globals.screen_bounds.x0) * PROFILE_GRAPH_SCALE;
		graph_screen_bounds.y0 = rasterizer_globals.screen_bounds.y1 - (rasterizer_globals.screen_bounds.y1 - rasterizer_globals.screen_bounds.y0) * PROFILE_GRAPH_SCALE;
		graph_screen_bounds.y1 = rasterizer_globals.screen_bounds.y0 + (rasterizer_globals.screen_bounds.y1 - rasterizer_globals.screen_bounds.y0) * PROFILE_GRAPH_SCALE;

		if (profile_graph)
		{
			short index;

			for (index = 0; index < profile_graph_value_count; index++)
			{
				struct profile_value *graph_value = &profile_graph_values[index];

				graph_value->frame_value = profile_find_frame_value(graph_value->name, &graph_value->section_index);
			}
		}

		if (profile_graph)
		{
			struct profile_frame_iterator iterator;
			__int64 vertical_blank_index;
			__int64 current_vertical_blank_index = rasterizer_globals.vblank_index;
			boolean first_frame = TRUE;

			{
				real_point3d p0;
				real_point3d p1;

				p0.x = graph_bounds.x0 * PROFILE_GRAPH_SCALE;
				p0.y = graph_bounds.y0 * PROFILE_GRAPH_SCALE;
				p0.z = -PROFILE_GRAPH_SCALE;
				p1.x = graph_bounds.x1 * PROFILE_GRAPH_SCALE;
				p1.y = graph_bounds.y0 * PROFILE_GRAPH_SCALE;
				p1.z = -PROFILE_GRAPH_SCALE;
				rasterizer_debug_immediate_line(&p0, &p1, &global_real_argb_white->rgb, NULL);
			}

			{
				short drawingbuf_counts[MAXIMUM_PROFILE_VALUES];
				point2d drawingbuf[MAXIMUM_PROFILE_VALUES][MAXIMUM_PROFILE_GRAPH_POINTS];
				real current_values[MAXIMUM_PROFILE_VALUES];
				point2d current_screen_points[MAXIMUM_PROFILE_VALUES];
				real_point3d current_world_points[MAXIMUM_PROFILE_VALUES];
				real last_values[MAXIMUM_PROFILE_VALUES];
				point2d last_screen_points[MAXIMUM_PROFILE_VALUES];
				real_point3d last_world_points[MAXIMUM_PROFILE_VALUES];

				memset(drawingbuf_counts, 0, sizeof(drawingbuf_counts));
				rasterizer_debug_immediate_begin_screenspace();
				profile_frame_iterator_new(&iterator);

				while (profile_frame_iterator_next(&iterator, &vertical_blank_index))
				{
					real frame_time = (real)(current_vertical_blank_index - vertical_blank_index) * (1.f / VBLANKS_PER_SECOND);

					if (frame_time < PROFILE_GRAPH_SECONDS)
					{
						short index;
						real screen_x = graph_screen_bounds.x0 + (graph_screen_bounds.x1 - graph_screen_bounds.x0) * frame_time * PROFILE_GRAPH_SCALE;
						real x = graph_bounds.x0 + (graph_bounds.x1 - graph_bounds.x0) * frame_time * PROFILE_GRAPH_SCALE;

						for (index = 0; index < profile_graph_value_count; index++)
						{
							struct profile_value *graph_value = &profile_graph_values[index];

							if (graph_value->frame_value != NONE)
							{
								point2d screenspace_point;
								real_point3d view_point;
								real_point3d world_point;
								real value = profile_frame_get_value(&iterator, graph_value->frame_value, graph_value->section_index);
								boolean overlapping = FALSE;

								value = PIN(value, 0.f, 100.f);

								if (graph_value->subtract_previous &&
									index > 0 &&
									profile_graph_values[index - 1].enabled &&
									profile_graph_values[index - 1].frame_value != NONE &&
									!first_frame &&
									fabs(value - current_values[index - 1]) < 0.1f &&
									fabs(last_values[index] - last_values[index - 1]) < 0.1f)
								{
									overlapping = TRUE;
								}

								screenspace_point.x = (short)screen_x;
								screenspace_point.y = (short)(graph_screen_bounds.y0 + (graph_screen_bounds.y1 - graph_screen_bounds.y0) * (value * 0.01f));
								view_point.x = x * PROFILE_GRAPH_SCALE;
								view_point.y = (graph_bounds.y0 + (graph_bounds.y1 - graph_bounds.y0) * (value * 0.01f)) * PROFILE_GRAPH_SCALE;
								view_point.z = -PROFILE_GRAPH_SCALE;
								matrix4x3_transform_point(&render.frustum.view_to_world, &view_point, &world_point);

								if (graph_value->enabled && !first_frame && !overlapping)
								{
									if (!drawingbuf_counts[index])
									{
										drawingbuf[index][0] = last_screen_points[index];
										drawingbuf_counts[index] = 1;
									}

									match_assert("c:\\halo\\SOURCE\\interface\\interface.c", 704, drawingbuf_counts[index] < 512);
									drawingbuf[index][drawingbuf_counts[index]++] = screenspace_point;
								}
								else if (drawingbuf_counts[index] > 0)
								{
									rasterizer_debug_immediate_linestrip_screenspace(
										drawingbuf[index],
										drawingbuf_counts[index],
										&(*graph_value->color)->rgb);
									drawingbuf_counts[index] = 0;
								}

								current_values[index] = value;
								current_screen_points[index] = screenspace_point;
								current_world_points[index] = world_point;
							}
						}
					}

					memcpy(last_values, current_values, sizeof(last_values));
					memcpy(last_screen_points, current_screen_points, sizeof(last_screen_points));
					memcpy(last_world_points, current_world_points, sizeof(last_world_points));
					first_frame = FALSE;
				}

				{
					short index;

					for (index = 0; index < profile_graph_value_count; index++)
					{
						if (drawingbuf_counts[index] > 0)
						{
							rasterizer_debug_immediate_linestrip_screenspace(
								drawingbuf[index],
								drawingbuf_counts[index],
								&(*profile_graph_values[index].color)->rgb);
							drawingbuf_counts[index] = 0;
						}
					}
				}

				rasterizer_debug_immediate_end_screenspace();
			}

			rasterizer_debug_immediate_begin();
			profile_frame_iterator_new(&iterator);

			while (profile_frame_iterator_next(&iterator, &vertical_blank_index))
			{
				real frame_time = (real)(current_vertical_blank_index - vertical_blank_index) * (1.f / VBLANKS_PER_SECOND);

				if (frame_time < PROFILE_GRAPH_SECONDS)
				{
					char graph_display_message_strings[MAXIMUM_PROFILE_GRAPH_MESSAGES][256];
					char *graph_display_message_stringptrs[MAXIMUM_PROFILE_GRAPH_MESSAGES];
					point2d graph_display_message_locations[MAXIMUM_PROFILE_GRAPH_MESSAGES];
					real_argb_color const *graph_display_message_colors[MAXIMUM_PROFILE_GRAPH_MESSAGES];
					short graph_display_messages;
					short largest_stall_type;
					real largest_stall_msec;
					long stalls;
					short message_index;
					real x = graph_bounds.x0 + (graph_bounds.x1 - graph_bounds.x0) * frame_time * PROFILE_GRAPH_SCALE;

					for (message_index = 0; message_index < (long)NUMBEROF(graph_display_message_stringptrs); message_index++)
					{
						graph_display_message_stringptrs[message_index] = graph_display_message_strings[message_index];
					}

					graph_display_messages = 0;
					profile_frame_get_messages(
						&iterator,
						&graph_display_messages,
						MAXIMUM_PROFILE_GRAPH_MESSAGES,
						graph_display_message_stringptrs,
						graph_display_message_locations,
						graph_display_message_colors);
					largest_stall_type = 0;
					largest_stall_msec = 0.f;
					stalls = profile_frame_get_stalls(&iterator, &largest_stall_type, &largest_stall_msec);

					if (stalls)
					{
						short stall_type;
						real below_position = 0.01f;

						if (TEST_FLAG(stalls, largest_stall_type))
						{
							render_debug_profile_stall_tick(
								largest_stall_type,
								&graph_bounds,
								&below_position,
								x,
								PROFILE_GRAPH_SCALE);
							SET_FLAG(stalls, largest_stall_type, FALSE);
						}

						for (stall_type = 0; stall_type < NUMBER_OF_RASTERIZER_STALL_TYPES; stall_type++)
						{
							if (TEST_FLAG(stalls, stall_type))
							{
								render_debug_profile_stall_tick(
									stall_type,
									&graph_bounds,
									&below_position,
									x,
									PROFILE_GRAPH_SCALE);
							}
						}
					}
				}
			}

			rasterizer_debug_immediate_end();
		}
	}

	return;
}

void interface_draw_fullscreen_overlays(
	void)
{
	cinematic_render();
	interface_splitscreen_render();
	hud_render_timer();
	terminal_draw();
	main_framerate_render();
	render_debug_profile();

	return;
}

static void interface_splitscreen_render(
	void)
{
	if (!game_engine_force_single_screen() && !cinematic_in_progress())
	{
		short window_count = local_player_count();

		if (window_count > 1)
		{
			rectangle2d bounds;

			bounds.y0 = 239;
			bounds.x0 = 0;
			bounds.y1 = 241;
			bounds.x1 = 640;
			draw_quad(&bounds, 0xFF000000);

			if (window_count > 2)
			{
				if (window_count == 3)
				{
					bounds.y0 = 240;
					bounds.x0 = 319;
					bounds.y1 = 480;
					bounds.x1 = 321;
					draw_quad(&bounds, 0xFF000000);
				}
				else
				{
					bounds.y0 = 0;
					bounds.x0 = 319;
					bounds.y1 = 480;
					bounds.x1 = 321;
					match_assert("c:\\halo\\SOURCE\\interface\\interface.c", 884, window_count==4);
					draw_quad(&bounds, 0xFF000000);
				}
			}
		}
	}

	return;
}

void interface_draw_bitmap(
	struct bitmap_data const *bitmap,
	point2d const *point,
	real_rectangle2d const *clip,
	real scale,
	real theta,
	real fade)
{
	struct dynamic_screen_vertex vertices[NUMBER_OF_POINTS_PER_RECTANGLE];
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	pixel32 color;
	short vertex_index;
	real_rectangle2d default_clip = { 0.f, 1.f, 0.f, 1.f };
	real sine_theta = sine(theta);
	real cosine_theta = cosine(theta);

	if (!clip)
	{
		clip = &default_clip;
	}

	color = PIXEL32_FROM_ARGB((pixel32)(fade * 255.f), UNSIGNED_CHAR_MAX, UNSIGNED_CHAR_MAX, UNSIGNED_CHAR_MAX);

	for (vertex_index = 0; vertex_index < NUMBER_OF_POINTS_PER_RECTANGLE; vertex_index++)
	{
		real u = ((vertex_index + 1) & 2) ? clip->x1 : clip->x0;
		real v = vertex_index > 1 ? clip->y1 : clip->y0;
		real x = (bitmap->width * u - bitmap->registration_point.x) * scale;
		real y = (bitmap->height * v - bitmap->registration_point.y) * scale;

		vertices[vertex_index].position.x = point->x + x * cosine_theta - y * sine_theta;
		vertices[vertex_index].position.y = point->y + x * sine_theta + y * cosine_theta;
		vertices[vertex_index].texcoord.x = u;
		vertices[vertex_index].texcoord.y = v;
		vertices[vertex_index].color = color;
	}

	memset(&parameters, 0, sizeof(parameters));
	parameters.map_texture_scale[0].j = 1.f;
	parameters.map_texture_scale[0].i = 1.f;
	parameters.map_scale[0].j = 1.f;
	parameters.map_scale[0].i = 1.f;
	parameters.meter_parameters = NULL;
	parameters.point_sampled = FALSE;
	parameters.framebuffer_blend_function = _shader_framebuffer_blend_function_alpha_multiply_add;
	parameters.map[0] = (struct bitmap_data *)bitmap;
	rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);

	return;
}

void interface_draw_bitmap_modulated(
	struct bitmap_data const *bitmap,
	point2d const *point,
	real_rectangle2d const *clip,
	real scale,
	real theta,
	real_argb_color const *modulated_color,
	short shader_type)
{
	struct dynamic_screen_vertex vertices[NUMBER_OF_POINTS_PER_RECTANGLE];
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	pixel32 color;
	short vertex_index;
	real_rectangle2d default_clip = { 0.f, (real)bitmap->width, 0.f, (real)bitmap->height };
	real sine_theta = sine(theta);
	real cosine_theta = cosine(theta);

	if (!clip)
	{
		clip = &default_clip;
	}

	color = REAL_ARGB_COLOR_TO_PIXEL32(modulated_color);

	for (vertex_index = 0; vertex_index < NUMBER_OF_POINTS_PER_RECTANGLE; vertex_index++)
	{
		real u = ((vertex_index + 1) & 2) ? clip->x1 : clip->x0;
		real v = vertex_index > 1 ? clip->y1 : clip->y0;
		real x = (u - bitmap->registration_point.x) * scale;
		real y = (v - bitmap->registration_point.y) * scale;

		vertices[vertex_index].position.x = point->x + x * cosine_theta - y * sine_theta;
		vertices[vertex_index].position.y = point->y + x * sine_theta + y * cosine_theta;
		vertices[vertex_index].texcoord.x = u;
		vertices[vertex_index].texcoord.y = v;
		vertices[vertex_index].color = color;
	}

	memset(&parameters, 0, sizeof(parameters));
	parameters.map_texture_scale[0].j = 1.f;
	parameters.map_texture_scale[0].i = 1.f;
	parameters.map_scale[0].j = 1.f;
	parameters.map_scale[0].i = 1.f;
	parameters.meter_parameters = NULL;
	parameters.point_sampled = FALSE;
	parameters.framebuffer_blend_function = shader_type;
	parameters.map[0] = (struct bitmap_data *)bitmap;
	rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);

	return;
}

void interface_draw_bitmap_modulated_p32(
	struct bitmap_data const *bitmap,
	point2d const *point,
	real_rectangle2d const *clip,
	real scale,
	real theta,
	pixel32 modulated_color,
	short shader_type)
{
	struct dynamic_screen_vertex vertices[NUMBER_OF_POINTS_PER_RECTANGLE];
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	short vertex_index;
	real_rectangle2d default_clip = { 0.f, (real)bitmap->width, 0.f, (real)bitmap->height };
	real sine_theta = sine(theta);
	real cosine_theta = cosine(theta);

	if (!clip)
	{
		clip = &default_clip;
	}

	for (vertex_index = 0; vertex_index < NUMBER_OF_POINTS_PER_RECTANGLE; vertex_index++)
	{
		real u = ((vertex_index + 1) & 2) ? clip->x1 : clip->x0;
		real v = vertex_index > 1 ? clip->y1 : clip->y0;
		real x = (u - bitmap->registration_point.x) * scale;
		real y = (v - bitmap->registration_point.y) * scale;

		vertices[vertex_index].position.x = point->x + x * cosine_theta - y * sine_theta;
		vertices[vertex_index].position.y = point->y + x * sine_theta + y * cosine_theta;
		vertices[vertex_index].texcoord.x = u;
		vertices[vertex_index].texcoord.y = v;
		vertices[vertex_index].color = modulated_color;
	}

	memset(&parameters, 0, sizeof(parameters));
	parameters.map_texture_scale[0].j = 1.f;
	parameters.map_texture_scale[0].i = 1.f;
	parameters.map_scale[0].j = 1.f;
	parameters.map_scale[0].i = 1.f;
	parameters.meter_parameters = NULL;
	parameters.point_sampled = FALSE;
	parameters.framebuffer_blend_function = shader_type;
	parameters.map[0] = (struct bitmap_data *)bitmap;
	rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);

	return;
}
