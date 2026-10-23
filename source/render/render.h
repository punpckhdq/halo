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

enum
{
	MAXIMUM_NUMBER_OF_BUILD_SPRITE_BITMAPS = 8,
};

enum
{
	_build_sprite_normal = 0,
	_build_sprite_parallel,
	_build_sprite_perpendicular,
	NUMBER_OF_BUILD_SPRITE_ORIENTATIONS,
};

enum
{
	_render_model_effect_type_none = 0,
	_render_model_effect_type_active_camouflage
};

/* ---------- macros */

/* ---------- structures */

struct render_animation
{
	real_rgb_color const *colors;
	real const *values;
};

struct render_skinning
{
	real_matrix4x3 *node_matrices;
	short node_matrix_count;
};

struct render_sort_filth
{
	short *prev_group_presorted_index_reference;
	short *next_group_presorted_index_reference;
	short group_index;
	short next_part_index;
	short part_index;
	word pad;
};

struct render_model_effect
{
	short type;
	real intensity;
	real parameter;
	long source_object_index;
	real_point3d source_object_centroid;
	struct shader *modifier_shader;
	struct render_animation modifier_animation;
};

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

struct build_sprite_group
{
	long vertex_buffer_index;
	void *vertices;
	short sprite_count;
	struct bitmap_data *bitmap;
};

struct build_sprite_data
{
	long bitmap_group_index;
	short maximum_sprite_count;
	short screen_space;
	struct shader_effect const *shader;
	short sprite_count;
	unsigned long flags;
	real_point3d centroid;
	short group_count;
	struct build_sprite_group groups[MAXIMUM_NUMBER_OF_BUILD_SPRITE_BITMAPS];
};

/* ---------- prototypes/RENDER.C */

void render_frame(struct render_window const *windoze, short window_count, point2d const *screenshot_page_index, point2d const *screenshot_index, struct bitmap_data *screenshot_bitmap, real time_delta_since_tick_sec);

void render_effects(boolean enable);
void render_initialize(void);
void render_initialize_for_new_map(void);
void render_dispose_from_old_map(void);
void render_dispose(void);

void render_frame_pregame(struct render_window const *window, struct bitmap_data *screenshot_bitmap);
void render_frame_present(point2d const *screenshot_index, struct bitmap_data *bitmap);
boolean render_location_visible(struct location *location);
struct rendered_cluster *rendered_cluster_get(short rendered_cluster_index);

/* ---------- prototypes/RENDER_SPRITE.C */

void build_sprites_begin(struct build_sprite_data *data, short maximum_sprite_count, long bitmap_group_index, struct shader_effect const *shader, unsigned long flags);
void build_sprites_end(struct build_sprite_data *data);
void build_sprite(struct build_sprite_data *data, short mode, short sequence_index, short sprite_index, real_point3d const *untransformed_origin, real_vector3d const *untransformed_direction, real rotation, real scale, real_argb_color const *color, real fade, unsigned long flags);

/* ---------- prototypes/RENDER_OBJECTS.C */

void render_objects_initialize(void);
void render_objects_initialize_for_new_map(void);
void render_objects_dispose_from_old_map(void);
void render_objects_dispose(void);
struct render_lighting const *object_get_cached_render_lighting(long object_index, real level_of_detail_pixels);

/* ---------- globals */

extern struct render_globals render;
extern boolean debug_render_freeze;

/* ---------- public code */

#endif // __RENDER_H
