/*
SHADER_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __SHADER_DEFINITIONS_H
#define __SHADER_DEFINITIONS_H
#pragma once

/* ---------- headers */

/* ---------- constants */

enum
{
	SHADER_DEFINITION_TAG = 'shdr',
};

enum
{
	_shader_type_screen = 0,
	_shader_type_effect,
	_shader_type_decal,
	_shader_type_environment,
	_shader_type_model,
	_shader_type_transparent_generic,
	_shader_type_transparent_chicago,
	_shader_type_transparent_water,
	_shader_type_transparent_glass,
	_shader_type_transparent_meter,
	_shader_type_transparent_plasma,
	NUMBER_OF_SHADER_TYPES
};

enum
{
	_shader_model_detail_after_reflection_bit = 0,
	_shader_model_two_sided_bit,
	_shader_model_not_alpha_tested_bit,
	_shader_model_alpha_blended_decal_bit,
	_shader_model_true_atmospheric_fog_bit,
	_shader_model_nocull_two_sided_bit,
	NUMBER_OF_SHADER_MODEL_FLAGS,
};

enum
{
	_shader_radiosity_simple_parameterization_bit = 0,
	_shader_radiosity_ignore_normals_bit,
	_shader_radiosity_FILTHY_transparent_lit_bit,
	NUMBER_OF_SHADER_RADIOSITY_FLAGS
};

enum
{
	_shader_framebuffer_blend_function_alpha_blend = 0,
	_shader_framebuffer_blend_function_multiply,
	_shader_framebuffer_blend_function_double_multiply,
	_shader_framebuffer_blend_function_add,
	_shader_framebuffer_blend_function_reverse_subtract,
	_shader_framebuffer_blend_function_min,
	_shader_framebuffer_blend_function_max,
	_shader_framebuffer_blend_function_alpha_multiply_add,
	NUMBER_OF_SHADER_FRAMEBUFFER_BLEND_FUNCTIONS
};

enum
{
	_shader_framebuffer_fade_mode_none = 0,
	_shader_framebuffer_fade_mode_fade_when_perpendicular,
	_shader_framebuffer_fade_mode_fade_when_parallel,
	NUMBER_OF_SHADER_FRAMEBUFFER_FADE_MODES
};

enum
{
	_shader_effect_secondary_map_anchor_with_primary = 0,
	_shader_effect_secondary_map_anchor_screen_space,
	_shader_effect_secondary_map_zsprite,
	NUMBER_OF_SHADER_EFFECT_SECONDARY_MAP_ANCHORS
};

enum
{
	_shader_environment_reflection_mirror_bit = 0,
	NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_FLAGS
};

enum
{
	_shader_transparent_generic_alpha_tested_bit = 0,
	_shader_transparent_generic_decal_bit,
	_shader_transparent_generic_two_sided_bit,
	_shader_transparent_generic_first_map_is_in_screenspace_bit,
	_shader_transparent_generic_draw_before_water_bit,
	_shader_transparent_generic_ignore_effect_bit,
	_shader_transparent_generic_scale_first_map_with_distance_bit,
	_shader_transparent_generic_numeric_bit,
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_FLAGS
};

enum
{
	_shader_transparent_generic_type_2d_map = 0,
	_shader_transparent_generic_type_first_map_is_reflection_cube_map,
	_shader_transparent_generic_type_first_map_is_object_centered_cube_map,
	_shader_transparent_generic_type_first_map_is_viewer_centered_cube_map,
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_TYPES
};

enum
{
	_shader_transparent_chicago_alpha_tested_bit = 0,
	_shader_transparent_chicago_decal_bit,
	_shader_transparent_chicago_two_sided_bit,
	_shader_transparent_chicago_first_map_is_in_screenspace_bit,
	_shader_transparent_chicago_draw_before_water_bit,
	_shader_transparent_chicago_ignore_effect_bit,
	_shader_transparent_chicago_scale_first_map_with_distance_bit,
	_shader_transparent_chicago_numeric_bit,
	NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_FLAGS
};

enum
{
	_shader_transparent_chicago_type_2d_map = 0,
	_shader_transparent_chicago_type_first_map_is_reflection_cube_map,
	_shader_transparent_chicago_type_first_map_is_object_centered_cube_map,
	_shader_transparent_chicago_type_first_map_is_viewer_centered_cube_map,
	NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_TYPES
};

enum
{
	_shader_transparent_glass_alpha_tested_bit = 0,
	_shader_transparent_glass_decal_bit,
	_shader_transparent_glass_two_sided_bit,
	_shader_transparent_glass_bump_map_is_specular_mask_bit,
	NUMBER_OF_SHADER_TRANSPARENT_GLASS_FLAGS
};

enum
{
	_shader_transparent_glass_reflection_type_bumped = 0,
	_shader_transparent_glass_reflection_type_flat,
	_shader_transparent_glass_reflection_type_mirror,
	NUMBER_OF_SHADER_TRANSPARENT_GLASS_REFLECTION_TYPES
};

enum
{
	_shader_transparent_meter_decal_bit = 0,
	_shader_transparent_meter_two_sided_bit,
	_shader_transparent_meter_flash_color_is_negative_bit,
	_shader_transparent_meter_tint_mode_2_bit,
	_shader_transparent_meter_point_sampled_bit,
	NUMBER_OF_SHADER_TRANSPARENT_METER_FLAGS
};

/* ---------- macros */

#define shader_definition_get(index) ((struct shader *)tag_get(SHADER_DEFINITION_TAG, (index))) /* fake name */

/* ---------- structures */

struct shader_radiosity_properties
{
	word flags;
	short detail_level;
	real power;
	real_rgb_color color;
	real_rgb_color tint_color;
};

struct shader_physics_properties
{
	word flags;
	short material_type;
};

struct _shader
{
	struct shader_radiosity_properties radiosity;
	struct shader_physics_properties physics;
	short type;
	word pad;
};

struct shader
{
	struct _shader base;
};

struct shader_texture_animation
{
	short u_source;
	short u_function;
	real u_period;
	real u_phase;
	real u_scale;
	short v_source;
	short v_function;
	real v_period;
	real v_phase;
	real v_scale;
	short r_source;
	short r_function;
	real r_period;
	real r_phase;
	real r_scale;
	real_point2d r_center;
};

struct _shader_effect
{
	word flags;
	short framebuffer_blend_function;
	short framebuffer_fade_mode;
	word primary_map_flags;
	long unused1[7];
	struct tag_reference secondary_map;
	short secondary_map_anchor;
	word secondary_map_flags;
	struct shader_texture_animation secondary_map_animation;
	real secondary_map_radius;
	real zsprite_radius_scale;
	long unused2[5];
};

struct shader_effect
{
	struct _shader shader;
	struct _shader_effect effect;
};

struct _shader_decal
{
	word flags;
	short type;
	short framebuffer_blend_function;
	word pad1;
	long unused1[5];
	struct tag_reference map;
	long unused2[5];
};

struct shader_decal
{
	struct _shader shader;
	struct _shader_decal decal;
};

struct shader_environment_diffuse_properties
{
	word flags;
	short type;
	long unused1[6];
	struct tag_reference base_map;
	long unused2[6];
	short detail_map_function;
	short detail_pad;
	real primary_detail_map_scale;
	struct tag_reference primary_detail_map;
	real secondary_detail_map_scale;
	struct tag_reference secondary_detail_map;
	long unused3[6];
	short micro_detail_map_function;
	short micro_detail_pad;
	real micro_detail_map_scale;
	struct tag_reference micro_detail_map;
	real_rgb_color material_color;
	long unused4[3];
	real bump_map_scale;
	struct tag_reference bump_map;
	real_vector2d runtime_bump_map_scale;
	long unused5[4];
	short u_animation_function;
	short u_animation_pad;
	real u_animation_period;
	real u_animation_scale;
	short v_animation_function;
	short v_animation_pad;
	real v_animation_period;
	real v_animation_scale;
	long unused6[6];
};

struct shader_environment_self_illumination_properties
{
	word flags;
	short type;
	long unused1[6];
	real_rgb_color primary_on_color;
	real_rgb_color primary_off_color;
	short primary_animation_function;
	short primary_animation_pad;
	real primary_animation_period;
	real primary_animation_phase;
	long unused2[6];
	real_rgb_color secondary_on_color;
	real_rgb_color secondary_off_color;
	short secondary_animation_function;
	short secondary_animation_pad;
	real secondary_animation_period;
	real secondary_animation_phase;
	long unused3[6];
	real_rgb_color plasma_on_color;
	real_rgb_color plasma_off_color;
	short plasma_animation_function;
	short plasma_animation_pad;
	real plasma_animation_period;
	real plasma_animation_phase;
	long unused4[6];
	real map_scale;
	struct tag_reference map;
	long unused5[6];
};

struct shader_environment_specular_properties
{
	word flags;
	short type;
	long unused1[4];
	real brightness;
	long unused2[5];
	real_rgb_color view_perpendicular_color;
	real_rgb_color view_parallel_color;
	long unused3[4];
};

struct shader_environment_reflection_properties
{
	word flags;
	short type;
	real lightmap_brightness_scale;
	long unused1[7];
	real view_perpendicular_brightness;
	real view_parallel_brightness;
	long unused2[4];
	real mirror_index_of_refraction;
	real mirror_depth;
	long unused3[4];
	struct tag_reference map;
	long unused4[4];
};

struct _shader_environment
{
	word flags;
	short type;
	real lens_flare_spacing;
	struct tag_reference lens_flare;
	long unused[11];
	struct shader_environment_diffuse_properties diffuse;
	struct shader_environment_self_illumination_properties self_illumination;
	struct shader_environment_specular_properties specular;
	struct shader_environment_reflection_properties reflection;
};

struct shader_environment
{
	struct _shader shader;
	struct _shader_environment environment;
};

struct _shader_model
{
	word flags;
	short type;
	long unused1[3];
	real translucency;
	long unused2[4];
	short diffuse_change_color_source;
	short pad;
	long unused3[7];
	word self_illumination_flags;
	word self_illumination_pad;
	short self_illumination_color_source;
	short self_illumination_animation_function;
	real self_illumination_animation_period;
	real_rgb_color self_illumination_animation_color_lower_bound;
	real_rgb_color self_illumination_animation_color_upper_bound;
	long unused4[3];
	real_vector2d map_scale;
	struct tag_reference base_map;
	long unused5[2];
	struct tag_reference multipurpose_map;
	long unused6[2];
	short detail_function;
	short detail_mask;
	real detail_map_scale;
	struct tag_reference detail_map;
	real detail_map_v_scale;
	long unused7[3];
	struct shader_texture_animation animation;
	long unused8[2];
	real reflection_falloff_distance;
	real reflection_cutoff_distance;
	real_argb_color reflection_view_perpendicular_color;
	real_argb_color reflection_view_parallel_color;
	struct tag_reference reflection_map;
	long unused9[4];
	real reflection_bump_map_scale;
	struct tag_reference reflection_bump_map;
	long unused10[8];
};

struct shader_model
{
	struct _shader shader;
	struct _shader_model model;
};

struct _shader_transparent_generic
{
	byte numeric_counter_limit;
	byte flags;
	short type;
	short framebuffer_blend_function;
	short framebuffer_fade_mode;
	short framebuffer_fade_source;
	short framebuffer_fade_unused;
	real lens_flare_spacing;
	struct tag_reference lens_flare;
	struct tag_block extra_layers;		// shader_transparent_layer
	struct tag_block maps;				// shader_transparent_generic_map
	struct tag_block stages;			// shader_transparent_generic_stage
};

struct shader_transparent_generic
{
	struct _shader shader;
	struct _shader_transparent_generic generic;
};

struct _shader_transparent_chicago
{
	byte numeric_counter_limit;
	byte flags;
	short type;
	short framebuffer_blend_function;
	short framebuffer_fade_mode;
	short framebuffer_fade_source;
	short framebuffer_fade_unused;
	real lens_flare_spacing;
	struct tag_reference lens_flare;
	struct tag_block extra_layers;		// shader_transparent_layer
	struct tag_block maps;				// shader_transparent_chicago_map
	long extra_flags;
	long unused[2];
};

struct shader_transparent_chicago
{
	struct _shader shader;
	struct _shader_transparent_chicago chicago;
};

struct _shader_transparent_glass
{
	word flags;
	short type;
	long unused1[10];
	real_rgb_color tint_color;
	real tint_map_scale;
	struct tag_reference tint_map;
	long unused2[5];
	word reflection_flags;
	short reflection_type;
	real_argb_color reflection_view_perpendicular_color;
	real_argb_color reflection_view_parallel_color;
	struct tag_reference reflection_map;
	real reflection_bump_map_scale;
	struct tag_reference reflection_bump_map;
	long unused3[32];
	word diffuse_flags;
	word diffuse_pad;
	real diffuse_map_scale;
	struct tag_reference diffuse_map;
	real diffuse_detail_map_scale;
	struct tag_reference diffuse_detail_map;
	long diffuse_unused[7];
	word specular_flags;
	word specular_pad;
	real specular_map_scale;
	struct tag_reference specular_map;
	real specular_detail_map_scale;
	struct tag_reference specular_detail_map;
	long specular_unused[7];
};

struct shader_transparent_glass
{
	struct _shader shader;
	struct _shader_transparent_glass glass;
};

struct _shader_transparent_meter
{
	word flags;
	short type;
	long unused1[8];
	struct tag_reference map;
	long unused2[8];
	real_rgb_color gradient_min_color;
	real_rgb_color gradient_max_color;
	real_rgb_color background_color;
	real_rgb_color flash_color;
	real_rgb_color tint_color;
	real meter_transparency;
	real background_transparency;
	long unused3[6];
	short meter_brightness_source;
	short flash_brightness_source;
	short value_source;
	short gradient_source;
	short flash_extension_source;
	word pad;
	long unused4[8];
};

struct shader_transparent_meter
{
	struct _shader shader;
	struct _shader_transparent_meter meter;
};

/* ---------- prototypes/SHADER_DEFINITIONS.C */

void *shader_get_and_verify_type(struct shader const *shader, short shader_type);

/* ---------- globals */

extern struct shader_effect global_shader_effect_additive;
extern struct shader_effect global_shader_effect_alpha_blended;

/* ---------- public code */

#endif // __SHADER_DEFINITIONS_H
