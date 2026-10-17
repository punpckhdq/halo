/*
RENDER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "render_debug.h"
#include "objects.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "input.h"
#include "rasterizer_console_vars.h"
#include "collisions.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "director.h"
#include "ui_widget.h"
#include "structures.h"
#include "editor_stubs.h"
#include "object_lights.h"
#include "cinematics.h"
#include "player_effects.h"
#include "decals.h"
#include "bink_playback.h"
#include "progress_bar.h"
#include "structure_render.h"
#include "game_engine.h"
#include "interface.h"
#include "first_person_weapons.h"
#include "particle_systems.h"
#include "weather_particle_systems.h"

/* ---------- constants */

enum
{
	_draw_fullscreen_elements = 0,
	_draw_game_engine_elements
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void render_player_frame(struct render_window *window, point2d const *screenshot_combined_index);
static void render_nonplayer_frame(struct render_window const *window, long type);
static void render_window(short local_player_index, struct render_camera const *render_camera, struct render_frustum const *render_frustum, struct render_camera const *rasterizer_camera, struct render_frustum const *rasterizer_frustum, short rasterizer_target, boolean has_mirror);

/* ---------- globals */

struct render_globals render;

boolean render_contrails_enabled = TRUE;
boolean render_particles_enabled = TRUE;
boolean render_particle_systems_enabled = TRUE;
boolean render_weather_particle_systems_enabled = TRUE;

/* ---------- public code */

void render_effects(
	boolean enable)
{
	render_weather_particle_systems_enabled = enable;
	render_particle_systems_enabled = enable;
	render_particles_enabled = enable;
	render_contrails_enabled = enable;

	return;
}

void render_initialize(
	void)
{
	render_objects_initialize();

	return;
}

void render_initialize_for_new_map(
	void)
{
	render_objects_initialize_for_new_map();

	return;
}

void render_dispose_from_old_map(
	void)
{
	render_objects_dispose_from_old_map();

	return;
}

void render_dispose(
	void)
{
	render_objects_dispose();

	return;
}

static void render_player_frame(
	struct render_window *window,
	point2d const *screenshot_combined_index)
{
	static boolean fog_error_displayed;
	real_rectangle2d frustum_bounds;
	struct render_frustum render_frustum;
	struct render_frustum rasterizer_frustum;
	struct render_mirror mirror;
	struct render_camera *camera = &window->render_camera;
	boolean has_mirror = FALSE;

	structure_visibility_find_camera(camera);
	render.fog.runtime_flags = 0;
	scenario_get_atmospheric_fog(window->local_player_index, render.visible_sky_index, &camera->position, &render.fog);
	structure_get_planar_fog((short)render.cluster_index, &render.fog);

	if (render.fog.atmospheric_maximum_distance != 0.f &&
		render.visible_sky_index == NONE &&
		render.fog.planar_maximum_distance > render.fog.atmospheric_maximum_distance)
	{
		render.fog.planar_maximum_distance = render.fog.atmospheric_maximum_distance;
	}

	if (render.fog.atmospheric_maximum_density == 1.f && render.fog.atmospheric_maximum_distance != 0.f)
	{
		window->render_camera.z_far = MIN(window->render_camera.z_far, render.fog.atmospheric_maximum_distance);
	}

	if (render.fog.planar_mode == _render_planar_fog_mode_fully_fogged && render.fog.planar_maximum_distance != 0.f)
	{
		window->render_camera.z_far = MIN(window->render_camera.z_far, render.fog.planar_maximum_distance);
	}

	if (window->render_camera.z_far <= window->render_camera.z_near)
	{
		if (!fog_error_displayed)
		{
			error(_error_silent, "### ERROR something is wrong with the fog in the sky tag or the fog tag");
			fog_error_displayed = TRUE;
		}

		window->render_camera.z_far = window->render_camera.z_near + 0.01f;
	}

	match_assert(
		"c:\\halo\\SOURCE\\render\\render.c",
		187,
		!memcmp(&window->render_camera.viewport_bounds, &window->rasterizer_camera.viewport_bounds, sizeof(rectangle2d)));
	match_assert(
		"c:\\halo\\SOURCE\\render\\render.c",
		188,
		!memcmp(&window->render_camera.window_bounds, &window->rasterizer_camera.window_bounds, sizeof(rectangle2d)));

	render_camera_build_frustum_bounds(camera, &frustum_bounds);

	if (screenshot_combined_index)
	{
		long screenshot_tile_count = global_screenshot_count * global_screenshot_size;

		if (screenshot_tile_count > 0)
		{
			real_rectangle2d adjusted_frustum_bounds;
			real tile_width = (frustum_bounds.x1 - frustum_bounds.x0) / screenshot_tile_count;
			real tile_height = (frustum_bounds.y1 - frustum_bounds.y0) / screenshot_tile_count;

			adjusted_frustum_bounds.x0 = screenshot_combined_index->x * tile_width + frustum_bounds.x0;
			adjusted_frustum_bounds.y0 = (screenshot_tile_count - screenshot_combined_index->y - 1) * tile_height + frustum_bounds.y0;
			adjusted_frustum_bounds.x1 = adjusted_frustum_bounds.x0 + tile_width;
			adjusted_frustum_bounds.y1 = adjusted_frustum_bounds.y0 + tile_height;
			frustum_bounds = adjusted_frustum_bounds;
		}
	}

	render_camera_build_frustum(camera, &frustum_bounds, &render_frustum, TRUE);
	render_camera_build_frustum(&window->rasterizer_camera, &frustum_bounds, &rasterizer_frustum, TRUE);

	if (main_get_window_count() == 1 && structure_visibility_find_mirror(camera, &render_frustum, &mirror))
	{
		struct render_camera mirror_camera;
		struct render_frustum mirror_frustum;
		short cluster_index = (short)render.cluster_index;

		match_assert("c:\\halo\\SOURCE\\render\\render.c", 225, window->render_camera.viewport_bounds.x0==0);
		match_assert("c:\\halo\\SOURCE\\render\\render.c", 226, window->render_camera.viewport_bounds.y0==0);
		match_assert(
			"c:\\halo\\SOURCE\\render\\render.c",
			227,
			window->render_camera.viewport_bounds.x1==RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH);
		match_assert(
			"c:\\halo\\SOURCE\\render\\render.c",
			228,
			window->render_camera.viewport_bounds.y1==RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT);

		render_camera_mirror(camera, &mirror, &mirror_camera);
		render_camera_build_frustum(&mirror_camera, &frustum_bounds, &mirror_frustum, TRUE);
		rasterizer_profile_enable(FALSE);
		render.cluster_index = mirror.cluster_index;
		render_window(
			NONE,
			&mirror_camera,
			&mirror_frustum,
			&mirror_camera,
			&mirror_frustum,
			_rasterizer_target_render_secondary,
			FALSE);
		render.cluster_index = cluster_index;
		rasterizer_profile_enable(TRUE);
		has_mirror = TRUE;
	}

	render_window(
		window->local_player_index,
		camera,
		&render_frustum,
		&window->rasterizer_camera,
		&rasterizer_frustum,
		_rasterizer_target_render_primary,
		has_mirror);

	return;
}

static void render_nonplayer_frame(
	struct render_window const *window,
	long type)
{
	struct rasterizer_window_begin_parameters rasterizer_parameters;

	profile_render_window_start(FALSE);
	memset(&rasterizer_parameters, 0, sizeof(rasterizer_parameters));
	render.camera = window->render_camera;
	render_camera_build_frustum(&render.camera, NULL, &render.frustum, TRUE);
	rasterizer_parameters.camera = window->rasterizer_camera;
	render_camera_build_frustum(&rasterizer_parameters.camera, NULL, &rasterizer_parameters.frustum, TRUE);
	rasterizer_parameters.rasterizer_target = _rasterizer_target_render_primary;
	rasterizer_parameters.suppress_clear = type == _draw_fullscreen_elements;
	rasterizer_parameters.window_index = NONE;
	rasterizer_window_begin(&rasterizer_parameters);

	switch (type)
	{
	case _draw_fullscreen_elements:
		interface_draw_fullscreen_overlays();
		rasterizer_debug_draw();
		break;
	case _draw_game_engine_elements:
		game_engine_nonplayer_post_rasterize();
		break;
	default:
		match_unreachable("c:\\halo\\SOURCE\\render\\render.c", 287);
		break;
	}

	rasterizer_window_end();
	profile_render_window_end();

	return;
}

void render_frame(
	struct render_window *windoze,
	short window_count,
	point2d const *screenshot_page_index,
	point2d const *screenshot_index,
	struct bitmap_data *screenshot_bitmap,
	real time_delta_since_tick_sec)
{
	struct rasterizer_frame_begin_parameters parameters;
	short window_index;

	render.frame_index++;
	render.time_delta_since_tick_sec = time_delta_since_tick_sec;
	memset(&parameters, 0, sizeof(parameters));
	parameters.game_time_sec = game_time_get() * (1.f / TICKS_PER_SECOND);
	rasterizer_frame_begin(&parameters);
	rasterizer_windows_begin();

	for (window_index = 0; window_index < window_count; window_index++)
	{
		struct render_window *window = &windoze[window_index];

		render.window_index = window_index;

		if (window->console_window)
		{
			render_nonplayer_frame(window, _draw_fullscreen_elements);
		}
		else if (window->local_player_index != NONE)
		{
			point2d screenshot_combined_index;

			if (screenshot_index && screenshot_page_index)
			{
				screenshot_combined_index.x = screenshot_page_index->x * global_screenshot_size + screenshot_index->x;
				screenshot_combined_index.y = screenshot_page_index->y * global_screenshot_size + screenshot_index->y;
			}

			render_player_frame(window, screenshot_index ? &screenshot_combined_index : NULL);
		}
		else
		{
			render_nonplayer_frame(window, _draw_game_engine_elements);
		}
	}

	progress_bar_eachframe();
	rasterizer_windows_end();
	rasterizer_frame_end();

	return;
}

static void render_window(
	short local_player_index,
	struct render_camera const *render_camera,
	struct render_frustum const *render_frustum,
	struct render_camera const *rasterizer_camera,
	struct render_frustum const *rasterizer_frustum,
	short rasterizer_target,
	boolean has_mirror)
{
	struct rasterizer_window_begin_parameters parameters;

	profile_render_window_start(TRUE);
	render.scene_index++;
	memset(&parameters, 0, sizeof(parameters));
	render.local_player_index = local_player_index;
	render.camera = *render_camera;
	render.frustum = *render_frustum;
	parameters.camera = *rasterizer_camera;
	parameters.frustum = *rasterizer_frustum;
	parameters.rasterizer_target = rasterizer_target;
	parameters.has_mirror = has_mirror;
	parameters.window_index = render.window_index;
	parameters.fog = render.fog;
	structure_visibility_compute();
	player_effect_get_screen_flash(local_player_index, &parameters.screen_flash);
	rasterizer_window_begin(&parameters);

	if (!bink_playback_in_progress())
	{
		short rendered_cluster_index;

		build_sprite_prepare_for_window();
		render_sky();
		first_person_weapon_render_update();
		lights_preprocess_scene();
		render_objects();
		structure_render_preprocess();
		structure_render_lightmaps();
		rasterizer_lens_flares_submit_occlusion_tests();
		render_object_shadows();
		lights_render_diffuse();

		rasterizer_decals_begin(_decal_layer_light);

		for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
		{
			rasterizer_decals_draw(rendered_cluster_get(rendered_cluster_index)->cluster_index);
		}

		rasterizer_decals_end();

		rasterizer_decals_begin(_decal_layer_alpha_tested);

		for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
		{
			rasterizer_decals_draw(rendered_cluster_get(rendered_cluster_index)->cluster_index);
		}

		rasterizer_decals_end();

		structure_render_diffuse_texture();

		rasterizer_decals_begin(_decal_layer_primary);

		for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
		{
			rasterizer_decals_draw(rendered_cluster_get(rendered_cluster_index)->cluster_index);
		}

		rasterizer_decals_end();

		rasterizer_decals_begin(_decal_layer_secondary);

		for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
		{
			rasterizer_decals_draw(rendered_cluster_get(rendered_cluster_index)->cluster_index);
		}

		rasterizer_decals_end();

		lights_render_specular();
		structure_render_specular_lightmaps();
		structure_render_reflection_lightmap_masks();
		structure_render_reflection_mirrors();
		structure_render_reflections();
		structure_render_transparent_geometry();
		structure_render_fog();
		game_engine_post_rasterize_objects();
		weather_particle_systems_render();
		render_particles();
		particle_systems_render();
		render_contrails_normal();
		rasterizer_transparent_geometry_draw(TRUE);

		rasterizer_decals_begin(_decal_layer_water);

		for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
		{
			rasterizer_decals_draw(rendered_cluster_get(rendered_cluster_index)->cluster_index);
		}

		rasterizer_decals_end();

		structure_render_detail_objects();
		rasterizer_transparent_geometry_draw(FALSE);
		rasterizer_transparent_geometry_stop();
		structure_render_fog_screen();
		rasterizer_lens_flares_draw();
		interface_draw_screen();
		rasterizer_screen_flash();
		render_ui_widgets(local_player_index, &rasterizer_camera->viewport_bounds);
	}

	bink_playback_render();
	render_camera_debug_frustum(&render.camera, &render.frustum);
	render_debug();
	editor_render();
	rasterizer_debug_draw();
	rasterizer_window_end();
	profile_render_window_end();

	return;
}

void render_frame_pregame(
	struct render_window const *window,
	struct bitmap_data *screenshot_bitmap)
{
	struct rasterizer_frame_begin_parameters parameters;
	struct rasterizer_window_begin_parameters rasterizer_parameters;

	render.frame_index++;
	rasterizer_frame_begin(&parameters);
	rasterizer_windows_begin();
	profile_render_window_start(FALSE);
	memset(&rasterizer_parameters, 0, sizeof(rasterizer_parameters));
	render.camera = window->render_camera;
	render_camera_build_frustum(&render.camera, NULL, &render.frustum, TRUE);
	rasterizer_parameters.camera = window->rasterizer_camera;
	render_camera_build_frustum(&rasterizer_parameters.camera, NULL, &rasterizer_parameters.frustum, TRUE);
	rasterizer_parameters.rasterizer_target = _rasterizer_target_render_primary;
	rasterizer_window_begin(&rasterizer_parameters);
	render_ui_widgets(0, &window->rasterizer_camera.viewport_bounds);
	bink_playback_render();

	{
		real progress;

		if (game_map_loading_in_progress(&progress))
		{
			progress_bar_display(progress);
		}
	}

	rasterizer_window_end();
	profile_render_window_end();
	rasterizer_windows_end();
	rasterizer_frame_end();

	return;
}

void render_frame_present(
	point2d const *screenshot_index,
	struct bitmap_data *bitmap)
{
	rasterizer_present(bitmap, screenshot_index);

	return;
}

boolean render_location_visible(
	struct location *location)
{
	match_assert(
		"c:\\halo\\SOURCE\\render\\render.c",
		584,
		location->cluster_index>=0 && location->cluster_index<global_structure_bsp_get()->clusters.count);

	return BIT_VECTOR_TEST_FLAG(render.visible_cluster_flags, location->cluster_index);
}

struct rendered_cluster *rendered_cluster_get(
	short rendered_cluster_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\render\\render.c",
		592,
		rendered_cluster_index>=0 && rendered_cluster_index<render.rendered_cluster_count);

	return &render.rendered_clusters[rendered_cluster_index];
}

/* ---------- private code */
