/*
RENDER.H

header included in hcex build.
*/

#ifndef __RENDER_H
#define __RENDER_H
#pragma once

/* ---------- headers */

#include "render_cameras.h"
#include "structures/structures.h"
#include "structures/structure_render.h"

/* ---------- constants */

enum
{
	MAXIMUM_RENDERED_DISTANT_LIGHTS= 2,
	MAXIMUM_RENDERED_POINT_LIGHTS= 2,
	MAXIMUM_RENDERED_ENVIRONMENT_SURFACES= 16384,
	MAXIMUM_RENDERED_CLUSTERS= 128,
	MAXIMUM_SURFACES_PER_STRUCTURE= 0x20000,
	MAXIMUM_RENDERED_LIGHTS= 128,
	MAXIMUM_LIGHTS_PER_MAP= 896,
	MAXIMUM_LENS_FLARES_PER_LIGHT= 8,
	MAXIMUM_QUEUED_LENS_FLARES= 8,
};

/* ---------- macros */

/* ---------- structures */

struct render_distant_light
{
	real_rgb_color color;
	real_vector3d direction;
};

struct render_lighting
{
	real_rgb_color ambient_color;
	short distant_light_count;
	word pad;
	struct render_distant_light distant_lights[MAXIMUM_RENDERED_DISTANT_LIGHTS];
	short point_light_count;
	word pad1;
	long point_light_indices[MAXIMUM_RENDERED_POINT_LIGHTS];
	real_argb_color reflection_tint_color;
	real_vector3d shadow_vector;
	real_rgb_color shadow_color;
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


/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* comm */
struct render_globals render;

/* ---------- public code */

#endif // __RENDER_H
