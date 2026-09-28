/*
RENDER.H

header included in hcex build.
*/

#ifndef __RENDER_H
#define __RENDER_H
#pragma once

/* ---------- headers */

#include "render_cameras.h"
#include "structure_bsp_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct render_screen_flash
{
	short type;
	real intensity;
	real_argb_color color;
};

struct render_screen_effect
{
	short type;
	real intensity;
};

struct rendered_cluster
{
	short cluster_index;
	real_rectangle2d clip_bounds;
	struct render_frustum frustum;
};

struct render_globals
{
	long frame_index;
	long scene_index;
	short local_player_index;
	short window_index;
	real time_delta_since_tick_sec;
	struct render_camera camera;
	struct render_frustum frustum;
	struct render_fog fog;
	long leaf_index;
	long cluster_index;
	boolean under_water;
	boolean visible_sky_model;
	short visible_sky_index;

	// Bitvector of all clusters where bit set means the cluster is visible
	unsigned long visible_cluster_flags[MAXIMUM_CLUSTERS_PER_STRUCTURE / LONG_BITS];
	struct rendered_cluster rendered_clusters[MAXIMUM_RENDERED_CLUSTERS];
	short rendered_cluster_count;
	unsigned long environment_surface_flags[MAXIMUM_SURFACES_PER_STRUCTURE];
	short environment_surface_count;
	long environment_surface_indices[MAXIMUM_RENDERED_ENVIRONMENT_SURFACES];
};

/* ---------- prototypes/RENDER.C */

void render_effects(boolean enable);
void render_initialize(void);
void render_initialize_for_new_map(void);
void render_dispose_from_old_map(void);
void render_dispose(void);

void render_frame_pregame(const struct render_window *window);
void render_frame_present(const point2d *screenshot_index, struct bitmap_data *bitmap);
boolean render_location_visible(struct location *location);
struct rendered_cluster *rendered_cluster_get(short rendered_cluster_index);

/* ---------- prototypes/RENDER_OBJECTS.C */

void render_objects_initialize(void);
void render_objects_initialize_for_new_map(void);
void render_objects_dispose_from_old_map(void);
void render_objects_dispose(void);

/* ---------- globals */

extern struct render_globals render;

/* ---------- public code */

#endif // __RENDER_H
