/*
OBJECT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __OBJECT_DEFINITIONS_H
#define __OBJECT_DEFINITIONS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	OBJECT_DEFINITION_TAG = 'obje',
	OBJECT_DEFINITION_VERSION = 1,
	NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS = 4,
	NUMBER_OF_INCOMING_OBJECT_FUNCTIONS = 4,
	MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT = 8,
	MAXIMUM_REGIONS_PER_OBJECT = 8,
};

enum
{
	_object_does_not_cast_shadow_bit = 0,
	_object_transparency_self_occludes_bit,
	_object_artificially_bright_bit,
	_object_not_pathfinding_obstacle_bit,
	NUMBER_OF_OBJECT_DEFINITION_FLAGS,
};

enum
{
	_object_runtime_scaled_change_colors_bit = 0,
	NUMBER_OF_OBJECT_DEFINITION_RUNTIME_FLAGS,
};

enum
{
	_object_function_invert_function_bit = 0,
	_object_function_additive_bit,
	_object_function_does_not_deactivate_below_lower_bound_bit,
	NUMBER_OF_OBJECT_FUNCTION_DEFINITION_FLAGS,
};

enum
{
	_object_function_clip_to_bounds = 0,
	_object_function_clip_to_bounds_and_normalize,
	_object_function_scale_to_fit_bounds,
	NUMBER_OF_OBJECT_FUNCTION_BOUNDS_MODES,
};

enum
{
	_object_function_none = 0,
	_object_function_body_vitality,
	_object_function_shield_vitality,
	_object_function_recent_body_damage,
	_object_function_recent_shield_damage,
	_object_function_random_constant,
	_object_function_umbrella_shield_vitality,
	_object_function_shield_stun,
	_object_function_recent_umbrella_shield_vitality,
	_object_function_umbrella_shield_stun,
	_object_function_first_region_damage,
	_object_function_last_region_damage = 17,
	_object_function_alive,
	_object_function_compass,
	NUMBER_OF_OBJECT_FUNCTION_MODES,
};

enum
{
	_object_type_biped = 0,
	_object_type_vehicle,
	_object_type_weapon,
	_object_type_equipment,
	_object_type_garbage,
	_object_type_projectile,
	_object_type_scenery,
	_object_type_machine,
	_object_type_control,
	_object_type_light_fixture,
	_object_type_placeholder,
	_object_type_sound_scenery,
	NUMBER_OF_OBJECT_TYPES,

	_object_mask_all =						UNSIGNED_LONG_MAX,
	_object_mask_unit =						(FLAG(_object_type_biped)|FLAG(_object_type_vehicle)),
	_object_mask_biped =					FLAG(_object_type_biped),
	_object_mask_vehicle =					FLAG(_object_type_vehicle),
	_object_mask_item =						(FLAG(_object_type_weapon)|FLAG(_object_type_equipment)|FLAG(_object_type_garbage)),
	_object_mask_weapon =					FLAG(_object_type_weapon),
	_object_mask_equipment =				FLAG(_object_type_equipment),
	_object_mask_garbage =					FLAG(_object_type_garbage),
	_object_mask_projectile =				FLAG(_object_type_projectile),
	_object_mask_scenery =					FLAG(_object_type_scenery),
	_object_mask_sound_scenery =			FLAG(_object_type_sound_scenery),
	_object_mask_device =					(FLAG(_object_type_machine)|FLAG(_object_type_control)|FLAG(_object_type_light_fixture)),
	_object_mask_machine =					FLAG(_object_type_machine),
	_object_mask_control =					FLAG(_object_type_control),
	_object_mask_light_fixture =			FLAG(_object_type_light_fixture),
	_object_mask_placeholder =				FLAG(_object_type_placeholder),

	_object_mask_sightblocking =			(FLAG(_object_type_vehicle)|FLAG(_object_type_scenery)|FLAG(_object_type_machine)),
	_object_mask_cannot_interpolate =		(FLAG(_object_type_projectile)|FLAG(_object_type_scenery)|FLAG(_object_type_machine)|FLAG(_object_type_control)|
											FLAG(_object_type_light_fixture)|FLAG(_object_type_placeholder)|FLAG(_object_type_sound_scenery)),
	_object_mask_remove_on_bsp_switch =		(FLAG(_object_type_scenery)|FLAG(_object_type_light_fixture))
};

enum
{
	_object_change_color_a = 0,
	_object_change_color_b,
	_object_change_color_c,
	_object_change_color_d,
	NUMBER_OF_OBJECT_CHANGE_COLORS,
};

/* ---------- macros */

#define object_definition_get(index) ((struct object_definition *)tag_get(OBJECT_DEFINITION_TAG, (index)))

/* ---------- structures */

struct object_attachment_definition
{
	struct tag_reference type;
	char marker_name[TAG_STRING_LENGTH+1];
	short primary_scale_function_reference;
	short secondary_scale_function_reference;
	short change_color_reference;
	word pad;
	long unused[4];
};

struct object_function_definition
{
	unsigned long flags;
	real period;
	short scale_period_by_function_index;
	short function_type;
	short scale_function_by_function_index;
	short wobble_function_type;
	real wobble_period;
	real wobble_magnitude;
	real square_wave_threshold;
	short step_count;
	short map_result_to_transition_function;
	short sawtooth_count;
	short add_function_index;
	short scale_result_by_function_index;
	short bounds_mode;
	real lower_bound;
	real upper_bound;
	real pad2;
	word pad3;
	short turn_off_with_function_index;
	real scale_by;
	int unused1[63];
	real runtime_reciprocal_bounds_range;
	real runtime_reciprocal_sawtooth_count;
	real runtime_reciprocal_step_count;
	real runtime_one_over_period;
	char usage[TAG_STRING_LENGTH+1];
};

struct object_change_color_permutation
{
	real weight;
	real_rgb_color color_lower_bound;
	real_rgb_color color_upper_bound;
};

struct object_change_color_definition
{
	short darken_by;
	short scaled_by;
	unsigned long scale_flags;
	real_rgb_color color_lower_bound;
	real_rgb_color color_upper_bound;
	struct tag_block permutations;		// object_change_color_permutation
};

struct _object_definition
{
	short type;
	word flags;
	real bounding_radius;
	real_point3d bounding_offset;
	real_point3d origin_offset;
	real acceleration_scale;
	unsigned long runtime_flags;
	struct tag_reference model;
	struct tag_reference animation_graph;
	long unused3[10];
	struct tag_reference collision_model;
	struct tag_reference physics;
	struct tag_reference modifier_shader;
	struct tag_reference creation_effect;
	long unused1[21];
	real render_bounding_radius;
	short function_modes[4];
	long unused2[11];
	short icon_text_index;
	short forced_shader_permutation_index;
	struct tag_block attachments;			// object_attachment_definition
	struct tag_block widgets;
	struct tag_block functions;				// object_function_definition
	struct tag_block change_colors;			// object_change_color_definition
	struct tag_block predicted_resources;
};

struct object_definition
{
	struct _object_definition object;
};

/* ---------- prototypes/OBJECT_DEFINITIONS.C */

/* ---------- globals */

/* ---------- public code */

#endif // __OBJECT_DEFINITIONS_H
