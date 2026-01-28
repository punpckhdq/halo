/*
MODEL_ANIMATION_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __MODEL_ANIMATION_DEFINITIONS_H
#define __MODEL_ANIMATION_DEFINITIONS_H
#pragma once


/* ---------- headers */

#include "math/real_math.h"
#include "tag_files/tag_files.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	ANIMATION_GRAPH_TAG= 'antr',
	ANIMATION_GRAPH_VERSION= 4,
	MAXIMUM_FRAMES_PER_ANIMATION= 2048,
	MAXIMUM_NODES_PER_ANIMATION= 64,
	MAXIMUM_ANIMATIONS_PER_GRAPH= 256,
	MAXIMUM_SOUND_REFERENCES_PER_ANIMATION_GRAPH= 257,
	MINIMUM_COMPRESSED_ANIMATION_FRAME_COUNT= 6,
};

enum
{
	_animation_compressed_bit= 0,
	_animation_world_relative_bit,
	_animation_25Hz_bit,
	NUMBER_OF_ANIMATION_FLAGS,
};

/* ---------- macros */

#define animation_graph_definition_get(index) ((struct animation_graph *)tag_get(ANIMATION_GRAPH_TAG, index))

/* ---------- structures */

struct animation
{
	char name[TAG_STRING_LENGTH+1];
	short type;
	short frame_count;
	short frame_size;
	short frame_info_type;
	unsigned long node_list_checksum;
	short node_count;
	short private_loop_frame_index;
	real weight;
	short private_key_frame_index;
	short private_second_key_frame_index;
	short next_animation_index;
	word flags;
	short sound_index;
	short private_sound_frame_index;
	byte private_left_foot_frame_index;
	byte private_right_foot_frame_index;
	short runtime_parent_animation_index;
	real runtime_normalized_weight;
	struct tag_data frame_info;
	unsigned long nodes_with_translation_flags[2];
	long unused_translation[2];
	unsigned long nodes_with_rotation_flags[2];
	long unused_rotation[2];
	unsigned int nodes_with_scale_flags[2];
	long unused_scale[1];
	long compressed_data_offset;
	struct tag_data default_data;
	struct tag_data data;
};


struct animation_graph
{
	struct tag_block object_overlays;
	struct tag_block unit_seats;
	struct tag_block weapon_animations;
	struct tag_block vehicle_animations;
	struct tag_block device_animations;
	struct tag_block unit_damage_animations;
	struct tag_block first_person_weapon_animations;
	struct tag_block sound_references;
	real limp_body_node_collision_radius;
	word flags;
	word pad;
	struct tag_block nodes;
	struct tag_block animations;						// animation
};

/* ---------- prototypes/MODEL_ANIMATION_DEFINITIONS.C */

/* ---------- prototypes/MODEL_ANIMATIONS.C */

void animation_get_node_orientations(
	struct model const *model,
	struct animation const *animation,
	short frame_index, 
	struct real_orientation *node_orientations);

void inverse_kinematics_adjust_matrices(
	struct real_matrix4x3 *desired_hand_matrix,
	struct real_matrix4x3 *shoulder_matrix,
	struct real_matrix4x3 *elbow_matrix,
	struct real_matrix4x3 *hand_matrix);

void interpolate_node_orientations(
	short node_count,
	struct real_orientation *original_node_orientations,
	struct real_orientation *target_node_orientations,
	short frame_index,
	short frame_count);

/* ---------- globals */

/* ---------- public code */

#endif // __MODEL_ANIMATION_DEFINITIONS_H
