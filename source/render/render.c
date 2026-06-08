/*
RENDER.C

symbols in this file:
001743B0 0020:
	_render_effects (0000)
001743D0 0010:
	_render_initialize (0000)
001743E0 0010:
	_render_initialize_for_new_map (0000)
001743F0 0010:
	_render_dispose_from_old_map (0000)
00174400 0010:
	_render_dispose (0000)
00174410 0110:
	_code_00174410 (0000)
00174520 00f0:
	_render_frame_pregame (0000)
00174610 0020:
	_render_frame_present (0000)
00174630 0070:
	_render_location_visible (0000)
001746A0 0050:
	_rendered_cluster_get (0000)
001746F0 03f0:
	_code_001746f0 (0000)
00174AE0 03f0:
	_code_00174ae0 (0000)
00174ED0 00f0:
	_render_frame (0000)
0029F44C 001f:
	??_C@_0BP@JNDFGHKA@c?3?2halo?2SOURCE?2render?2render?4c?$AA@ (0000)
0029F470 0061:
	??_C@_0GB@NHJDHKGO@location?9?$DOcluster_index?$DO?$DN0?5?$CG?$CG?5lo@ (0000)
0029F4D8 0052:
	??_C@_0FC@DLFICDJP@rendered_cluster_index?$DO?$DN0?5?$CG?$CG?5ren@ (0000)
0029F530 0052:
	??_C@_0FC@NBMIPGBN@window?9?$DOrender_camera?4viewport_b@ (0000)
0029F588 0051:
	??_C@_0FB@IDPOBHAJ@window?9?$DOrender_camera?4viewport_b@ (0000)
0029F5DC 002c:
	??_C@_0CM@LKHLGDKF@window?9?$DOrender_camera?4viewport_b@ (0000)
0029F608 002c:
	??_C@_0CM@HBCHLAAA@window?9?$DOrender_camera?4viewport_b@ (0000)
0029F638 006d:
	??_C@_0GN@PKELCNNH@?$CBmemcmp?$CI?$CGwindow?9?$DOrender_camera?4w@ (0000)
0029F6A8 0071:
	??_C@_0HB@BCLIBPGM@?$CBmemcmp?$CI?$CGwindow?9?$DOrender_camera?4v@ (0000)
0029F720 0048:
	??_C@_0EI@NGOLLMDP@?$CD?$CD?$CD?5ERROR?5something?5is?5wrong?5wit@ (0000)
0030D4D2 0004:
	_render_contrails_enabled (0000)
	_render_particles_enabled (0001)
	_render_particle_systems_enabled (0002)
	_render_weather_particle_systems_enabled (0003)
004B8B22 0001:
	_bss_004b8b22 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "objects.h"
#include "scenario.h"
#include "structure_bsp_definitions.h"
#include "rasterizer.h"
#include "profile.h"
#include "progress_bar.h"
#include "ui_widget.h"
#include "bink_playback.h"
#include "game.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

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
}

void render_initialize(
	void)
{
	render_objects_initialize();
}

void render_initialize_for_new_map(
	void)
{
	render_objects_initialize_for_new_map();
}

void render_dispose_from_old_map(
	void)
{
	render_objects_dispose_from_old_map();
}

void render_dispose(
	void)
{
	render_objects_dispose();
}

static void code_00174410(
	void)
{
	rasterizer_window_end();
	profile_render_window_end();
	rasterizer_windows_end();
	rasterizer_frame_end();
}

void render_frame_pregame(
	const struct render_window *window)
{
	struct rasterizer_frame_begin_parameters parameters;
	struct rasterizer_window_begin_parameters rasterizer_parameters;

	render.frame_index++;

	rasterizer_frame_begin(&parameters);
	rasterizer_windows_begin();
	profile_render_window_start(NULL);

	memset(&rasterizer_parameters, 0, sizeof(rasterizer_parameters));

	render.camera = window->render_camera;
	render_camera_build_frustum(&render.camera, NULL, &render.frustum, TRUE);

	rasterizer_parameters.camera = window->rasterizer_camera;
	render_camera_build_frustum(&rasterizer_parameters.camera, NULL, &rasterizer_parameters.frustum, TRUE);

	rasterizer_parameters.rasterizer_target = 0;
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
}

void render_frame_present(
	const point2d *screenshot_index,
	struct bitmap_data *bitmap)
{
	rasterizer_present(bitmap, screenshot_index);
}

boolean render_location_visible(
	struct location *location)
{
	match_assert("c:\\halo\\SOURCE\\render\\render.c", 584, location->cluster_index>=0 && location->cluster_index<global_structure_bsp_get()->clusters.count);
	return TEST_FLAG(render.visible_cluster_flags[location->cluster_index>>5], location->cluster_index&31);
}

struct rendered_cluster *rendered_cluster_get(
	short rendered_cluster_index)
{
	match_assert("c:\\halo\\SOURCE\\render\\render.c", 592, rendered_cluster_index>=0 && rendered_cluster_index<render.rendered_cluster_count);
	return &render.rendered_clusters[rendered_cluster_index];
}

/* ---------- private code */
