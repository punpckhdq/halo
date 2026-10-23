/*
MODELS.H

header included in hcex build.
*/

#ifndef __MODELS_H
#define __MODELS_H
#pragma once

/* ---------- headers */

#include "model_definitions.h"
#include "rasterizer_geometry.h"
#include "object_definitions.h"

/* ---------- constants */

enum animation_update_kind
{
	animation_update_kind_render_only = 0,
	animation_update_kind_affects_game_state,
};

enum
{
	_animation_running = 0,
	_animation_key_frame,
	_animation_will_restart_on_next_frame,
	_animation_restarted,
	_animation_looped,
	NUMBER_OF_ANIMATION_UPDATE_RESULTS,
};

enum
{
	_render_model_immediate_bit = 0,
	_render_model_shadow_bit,
	_render_model_no_planar_fog_bit,
	_render_model_first_person_bit,
};

/* ---------- structures */

struct animation_state
{
	short index;
	short frame_index;
};

struct animation_state_continuous
{
	short index;
	word pad;
	real frame_index;
};

/* ---------- prototypes/MODELS.C */

void render_model(
	long model_index,
	real level_of_detail_pixels,
	real_matrix4x3 const *node_matrices,
	char const *region_permutation_indices,
	real_rgb_color const *change_colors,
	real const *function_values,
	struct render_lighting const *lighting,
	real_point3d const *centroid,
	real radius,
	struct render_model_effect const *model_effect,
	long object_index,
	short forced_shader_permutation_index,
	unsigned long flags);

void model_interpolate_node_orientations(struct model *model, real_orientation *original_node_orientations, real_orientation *target_node_orientations, short frame_index, short frame_count);
void model_get_node_matrices(struct model const *model, real_matrix4x3 *node_matrices, real_point3d const *origin, real_vector3d const *forward, real_vector3d const *up);
void model_node_matrices_from_orientations(struct model const *model, real_matrix4x3 *node_matrices, real_orientation const *node_orientations, real_point3d const *origin, real_vector3d const *forward, real_vector3d const *up);
real_matrix4x3 const *model_get_default_inverse_matrix(struct model const *model, short node_index);
short model_find_node(long model_index, char const *name);
void model_build_tangent_matrices(struct model *model);

void model_get_node_orientations(struct model const *model, struct real_orientation *node_orientations);

short model_find_marker(long model_index, char const *name);
short model_get_marker_by_name(
	long model_index,
	char const *name,
	byte const *region_permutations,
	short const *node_remapping_table,
	short node_count, 
	struct real_matrix4x3 const *node_matrices,
	boolean mirrored_flag,
	struct object_marker *markers,
	short maximum_marker_count);

/* ---------- prototypes/MODEL_ANIMATIONS.C */

short animation_graph_get_animation_by_name(long animation_graph_index, char const *animation_name);
short animation_choose_random_permutation_internal(enum animation_update_kind render_or_affects_game_state, long animation_graph_index, short animation_index);
short animation_update_internal(enum animation_update_kind render_or_affects_game_state, long animation_graph_index, struct animation_state *state, long *triggered_sound_index);

/* ---------- globals */

extern boolean render_model_no_geometry;
extern boolean render_model_markers;
extern boolean render_model_index_counts;
extern boolean render_model_vertex_counts;
extern boolean render_model_nodes;

/* ---------- public code */

__inline short animation_update(
	long animation_graph_index,
	struct animation_state *state,
	long *triggered_sound_index)
{
	return animation_update_internal(animation_update_kind_affects_game_state, animation_graph_index, state, triggered_sound_index);
}

__inline short animation_update_render_only(
	long animation_graph_index,
	struct animation_state *state,
	long *triggered_sound_index)
{
	return animation_update_internal(animation_update_kind_render_only, animation_graph_index, state, triggered_sound_index);
}

__inline short animation_choose_random_permutation(
	long animation_graph_index,
	short animation_index)
{
	return animation_choose_random_permutation_internal(animation_update_kind_affects_game_state, animation_graph_index, animation_index);
}

#endif // __MODELS_H
