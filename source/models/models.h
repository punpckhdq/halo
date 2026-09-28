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

/* ---------- macros */

/* ---------- structures */

struct animation_state
{
	short index;
	short frame_index;
};

/* ---------- prototypes/MODELS.C */

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

/* ---------- public code */

__inline short animation_choose_random_permutation(
	long animation_graph_index,
	short animation_index)
{
	return animation_choose_random_permutation_internal(animation_update_kind_affects_game_state, animation_graph_index, animation_index);
}

__inline short animation_update(
	long animation_graph_index,
	struct animation_state *state,
	long *triggered_sound_index)
{
	return animation_update_internal(animation_update_kind_affects_game_state, animation_graph_index, state, triggered_sound_index);
}

#endif // __MODELS_H
