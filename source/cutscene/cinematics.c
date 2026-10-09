/*
CINEMATICS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cinematics.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "ai_communication.h"
#include "rasterizer.h"
#include "players.h"
#include "bitmap_macros.h"
#include "input.h"
#include "game_state.h"
#include "sound_manager.h"
#include "items.h"
#include "text_group.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "draw_string.h"
#include "texture_cache.h"
#include "editor_stubs.h"
#include "hud.h"
#include "projectiles.h"
#include "ai_globals.h"
#include "rasterizer_cinematics.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct cinematic_globals_definition *cinematic_globals = NULL;

/* ---------- public code */

void cinematic_initialize(
	void)
{
	cinematic_globals = game_state_malloc(
		"cinematic globals",
		NULL,
		sizeof(struct cinematic_globals_definition));
	match_assert("c:\\halo\\SOURCE\\cutscene\\cinematics.c", 24, cinematic_globals);

	return;
}

void cinematic_dispose(
	void)
{
	return;
}

void cinematic_initialize_for_new_map(
	void)
{
	memset(cinematic_globals, 0, sizeof(*cinematic_globals));
	memset(cinematic_globals->active_titles, NONE, sizeof(cinematic_globals->active_titles));

	return;
}

void cinematic_dispose_from_old_map(
	void)
{
	cinematic_globals->letter_box = FALSE;
	cinematic_globals->cinematic_in_progress = FALSE;

	return;
}

void cinematic_start(
	void)
{
	player_input_enable(FALSE);
	ai_globals_dialogue_triggers_enabled(FALSE);
	cinematic_globals->letter_box = TRUE;
	cinematic_globals->letter_box_last_game_time = game_time_get();
	cinematic_globals->cinematic_in_progress = TRUE;
	projectiles_delete_all();

	return;
}

boolean cinematic_can_be_skipped(
	void)
{
	return cinematic_globals->cinematic_skip_in_progress;
}

void cinematic_skip_start(
	void)
{
	cinematic_globals->cinematic_skip_in_progress = TRUE;

	return;
}

void cinematic_skip_stop(
	void)
{
	cinematic_globals->cinematic_skip_in_progress = FALSE;

	return;
}

void cinematic_show_letterbox(
	boolean show)
{
	cinematic_globals->letter_box = show;

	if (show)
	{
		cinematic_globals->letter_box_last_game_time = game_time_get();
	}

	return;
}

void draw_quad(
	rectangle2d *rect,
	pixel32 color)
{
	struct dynamic_screen_vertex vertices[4];
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	real_point2d points[4];
	long vertex_index;
	struct scenario *scenario = global_scenario_get();
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_rasterizer_data *rasterizer_data = game_globals->rasterizer_data.count ? TAG_BLOCK_GET_ELEMENT(&game_globals->rasterizer_data, 0, struct game_globals_rasterizer_data) : NULL;
	struct bitmap_data *bitmap = TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(rasterizer_data->default_textures[0].index)->bitmaps, 1, struct bitmap_data);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_cinematics;

	points[0].x = rect->x0;
	points[0].y = rect->y0;
	points[1].x = rect->x1;
	points[1].y = rect->y0;
	points[2].x = rect->x1;
	points[2].y = rect->y1;
	points[3].x = rect->x0;
	points[3].y = rect->y1;

	for (vertex_index = 0; vertex_index < NUMBEROF(vertices); vertex_index++)
	{
		vertices[vertex_index].color = color;
		vertices[vertex_index].texcoord.x = 0.f;
		vertices[vertex_index].texcoord.y = 0.f;
		vertices[vertex_index].position = points[vertex_index];
	}

	memset(&parameters, 0, sizeof(parameters));
	parameters.framebuffer_blend_function = _shader_framebuffer_blend_function_alpha_blend;
	parameters.map_texture_scale[0].j = 1.f;
	parameters.map_texture_scale[0].i = 1.f;
	parameters.map_scale[0].j = 1.f;
	parameters.map_scale[0].i = 1.f;
	parameters.meter_parameters = NULL;
	parameters.point_sampled = FALSE;
	parameters.map[0] = bitmap;
	rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}

void cinematic_set_title(
	short index)
{
	cinematic_set_title_delayed(index, 0.f);

	return;
}

void cinematic_set_title_delayed(
	short index,
	real delay)
{
	short active_title_index;

	for (active_title_index = 0;
		active_title_index < MAXIMUM_ACTIVE_CINEMATIC_TITLES && cinematic_globals->active_titles[active_title_index].title_index != NONE;
		active_title_index++)
	{
	}

	if (active_title_index < MAXIMUM_ACTIVE_CINEMATIC_TITLES)
	{
		cinematic_globals->active_titles[active_title_index].title_index = index;
		cinematic_globals->active_titles[active_title_index].title_timer = (short)-fast_ftol(delay * TICKS_PER_SECOND);
	}
	else
	{
		error(
			_error_silent,
			"no free chapter title slots to display title '%s'",
			TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->cutscene_chapter_titles, index, struct scenario_cutscene_title)->name);
	}

	return;
}

void cinematic_force_title(
	short index)
{
	cinematic_globals->active_titles[0].title_index = index;
	cinematic_globals->active_titles[0].title_timer = 0;

	return;
}

void cinematic_suppress_bsp_object_creation(
	boolean suppress)
{
	cinematic_globals->cinematic_suppress_bsp_object_creation = suppress;

	return;
}

void cinematic_render(
	void)
{
	if ((cinematic_globals->letter_box || cinematic_globals->letter_box_amount > 0.f) && !ui_widgets_active())
	{
		real const seconds_per_tick = 1.f / TICKS_PER_SECOND;
		long game_time = game_time_get();
		long elapsed_ticks = game_time - cinematic_globals->letter_box_last_game_time;

		cinematic_globals->letter_box_last_game_time = game_time;

		if (cinematic_globals->letter_box)
		{
			cinematic_globals->letter_box_amount += elapsed_ticks * seconds_per_tick;
			cinematic_globals->letter_box_amount = MIN(cinematic_globals->letter_box_amount, 1.f);
		}
		else
		{
			cinematic_globals->letter_box_amount -= elapsed_ticks * seconds_per_tick;
			cinematic_globals->letter_box_amount = MAX(cinematic_globals->letter_box_amount, 0.f);
		}

		if (cinematic_globals->letter_box_amount > 0.f)
		{
			rectangle2d bounds;
			real letter_box_scale = 0.125f * cinematic_globals->letter_box_amount;
			real viewport_height = render.camera.viewport_bounds.y1 - render.camera.viewport_bounds.y0;

			bounds.x0 = fast_ftol(render.camera.viewport_bounds.x0);
			bounds.x1 = fast_ftol(render.camera.viewport_bounds.x1);
			bounds.y0 = fast_ftol(render.camera.viewport_bounds.y0);
			bounds.y1 = fast_ftol(render.camera.viewport_bounds.y0 + viewport_height * letter_box_scale);
			draw_quad(&bounds, 0xff000000);

			bounds.x0 = fast_ftol(render.camera.viewport_bounds.x0);
			bounds.x1 = fast_ftol(render.camera.viewport_bounds.x1);
			bounds.y0 = fast_ftol(render.camera.viewport_bounds.y1 - viewport_height * letter_box_scale);
			bounds.y1 = fast_ftol(render.camera.viewport_bounds.y1);
			draw_quad(&bounds, 0xff000000);
		}
	}

	{
		short active_title_index;

		for (active_title_index = 0; active_title_index < MAXIMUM_ACTIVE_CINEMATIC_TITLES; active_title_index++)
		{
			struct cinematic_title_datum *title_datum = &cinematic_globals->active_titles[active_title_index];

			if (title_datum->title_index != NONE)
			{
				long font_index = hud_globals->messaging.single_player_font.index;

				if (font_index != NONE)
				{
					struct scenario_cutscene_title *title = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->cutscene_chapter_titles, title_datum->title_index, struct scenario_cutscene_title);
					long string_list_index = global_scenario_get()->ingame_help_text.index;

					if (string_list_index != NONE)
					{
						struct unicode_string_list_group_header *string_list = unicode_string_list_definition_get(string_list_index);

						if (title->text_index >= 0 && title->text_index < string_list->string_references.count)
						{
							real_argb_color text_color;
							rectangle2d *bounds = &title->bounds;
							real fade = 1.f;

							if (bounds->x1 == bounds->x0 || bounds->y1 == bounds->y0)
							{
								bounds = &hud_globals->defaults.default_title_bounds;
							}

							if (!game_in_editor())
							{
								if (title_datum->title_timer < title->fade_in_time)
								{
									fade = title_datum->title_timer / title->fade_in_time;
								}
								else if (title_datum->title_timer > title->up_time)
								{
									fade = 1.f - ((title_datum->title_timer - title->up_time) / title->fade_out_time);
								}

								fade = PIN(fade, 0.f, 1.f);
							}

							pixel32_to_real_argb_color(title->foreground_color, &text_color);
							text_color.alpha *= fade;

							if (fabs(text_color.red - 1.f) < _real_epsilon &&
								fabs(text_color.green - 1.f) < _real_epsilon &&
								fabs(text_color.blue - 1.f) < _real_epsilon)
							{
								text_color.red = MIN(text_color.red, 0.8f);
								text_color.green = MIN(text_color.green, 0.8f);
								text_color.blue = MIN(text_color.blue, 0.8f);
							}

							draw_string_set_draw_mode(
								font_index,
								title->style - 1,
								title->justification,
								title->text_flags,
								&text_color);
							rasterizer_text_set_shadow_color((PIN(fast_ftol((title->shadow_color >> 24) * fade), 0, UNSIGNED_CHAR_MAX) << 24) | (title->shadow_color & MASK(24)));
							rasterizer_draw_unicode_string(
								bounds,
								NULL,
								NULL,
								0,
								unicode_string_list_get_string(string_list_index, title->text_index));
							rasterizer_text_set_shadow_color(0);

							title_datum->title_timer += game_time_get_paused() ? 0 : game_time_get_elapsed();

							if (!game_in_editor() && title_datum->title_timer >= title->up_time + title->fade_out_time)
							{
								title_datum->title_index = NONE;
								title_datum->title_timer = NONE;
							}
						}
					}
				}
			}
		}
	}

	return;
}

void cinematic_stop(
	void)
{
	cinematic_globals->letter_box = FALSE;
	player_input_enable(TRUE);
	ai_globals_dialogue_triggers_enabled(TRUE);
	cinematic_globals->cinematic_in_progress = FALSE;
	rasterizer_screen_effects_initialize_for_new_map();

	if (global_rasterizer_model_ambient_reflection_tint)
	{
		memset(global_rasterizer_model_ambient_reflection_tint, 0, sizeof(*global_rasterizer_model_ambient_reflection_tint));
	}

	rasterizer_set_near_clip_distance(0.f);
	display_errors_deferred_until_cinematic_stop();

	return;
}

boolean cinematic_in_progress(
	void)
{
	return cinematic_globals->cinematic_in_progress;
}
