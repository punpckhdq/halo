/*
RASTERIZER.H

header included in hcex build.
*/

#ifndef __RASTERIZER_H
#define __RASTERIZER_H
#pragma once

/* ---------- headers */

#include "render.h"

/* ---------- constants */

enum
{
	MAXIMUM_WINDOWS = 4,
	MAXIMUM_LENS_FLARES_PER_FRAME = 1024,
	MAXIMUM_LIGHTS_PER_WINDOW = 128,
	NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS = 3, /* fake name */
};

enum
{
	_rasterizer_profile_clear = 0,
	_rasterizer_profile_model_sky,
	_rasterizer_profile_models,
	_rasterizer_profile_environment_lightmaps,
	_rasterizer_profile_environment_shadows,
	_rasterizer_profile_environment_diffuse_lights,
	_rasterizer_profile_environment_decals_light,
	_rasterizer_profile_environment_decals_alpha_tested,
	_rasterizer_profile_environment_textures,
	_rasterizer_profile_environment_decals_primary,
	_rasterizer_profile_environment_decals_secondary,
	_rasterizer_profile_environment_specular_lights,
	_rasterizer_profile_environment_specular_lightmaps,
	_rasterizer_profile_environment_reflection_lightmap_masks,
	_rasterizer_profile_environment_reflection_mirrors,
	_rasterizer_profile_environment_reflections,
	_rasterizer_profile_environment_transparents,
	_rasterizer_profile_environment_fog,
	_rasterizer_profile_environment_fog_screen,
	_rasterizer_profile_water,
	_rasterizer_profile_environment_decals_water,
	_rasterizer_profile_detail_objects,
	_rasterizer_profile_queued_transparents,
	_rasterizer_profile_lens_flare_occlusion_submit,
	_rasterizer_profile_lens_flare_occlusion_query,
	_rasterizer_profile_lens_flares,
	_rasterizer_profile_screen_effect,
	_rasterizer_profile_hud,
	_rasterizer_profile_screen_flash,
	NUMBER_OF_RASTERIZER_PROFILES,
};

enum
{
	_rasterizer_lock_none = 0,
	_rasterizer_lock_texture_changed,
	_rasterizer_lock_vertexbuffer_new,
	_rasterizer_lock_detail_objects,
	_rasterizer_lock_decal_update,
	_rasterizer_lock_decal_vertices,
	_rasterizer_lock_bink,
	_rasterizer_lock_ui,
	_rasterizer_lock_cinematics,
	_rasterizer_lock_koth,
	_rasterizer_lock_hud,
	_rasterizer_lock_flag,
	_rasterizer_lock_lightning,
	_rasterizer_lock_debug,
	_rasterizer_lock_text,
	_rasterizer_lock_contrail,
	_rasterizer_lock_sprite,
	_rasterizer_lock_bsp_switch
};

enum
{
	_rasterizer_target_render_primary = 0,
	_rasterizer_target_render_secondary,
	_rasterizer_target_shadow_primary,
	_rasterizer_target_shadow_secondary,
	_rasterizer_target_sun_glow_primary,
	_rasterizer_target_sun_glow_secondary,
	_rasterizer_target_water,
	_rasterizer_target_z,
	NUMBER_OF_RASTERIZER_TARGETS,
};

enum
{
	_render_planar_fog_mode_off = 0,
	_render_planar_fog_mode_normal,
	_render_planar_fog_mode_fully_fogged,
	NUMBER_OF_RENDER_PLANAR_FOG_MODES,
};


/* ---------- macros */

#define RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH 640
#define RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT 480

/* ---------- structures */

struct rasterizer_frame_begin_parameters
{
	real game_time_sec;
	real dt;
};

struct dynamic_screen_vertex
{
	real_point2d position;
	real_point2d texcoord;
	pixel32 color;
};

struct rasterizer_dynamic_screen_geometry_parameters
{
	struct rasterizer_meter_parameters *meter_parameters;
	real_vector2d *offset;
	boolean map_anchor_screen[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	struct bitmap_data *map[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	boolean map_wrapped[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	real_point2d *map_offset[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	real_vector2d map_scale[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	real_vector2d map_texture_scale[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	real_rgb_color *map_tint[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	real_argb_color plasma_fade;
	boolean doing_plasma_effect;
	real *map_fade[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
	short map0_to_1_blend_function;
	short map1_to_2_blend_function;
	short framebuffer_blend_function;
	boolean point_sampled;
};

struct rasterizer_window_begin_parameters
{
	short rasterizer_target;
	short window_index;
	boolean has_mirror;
	boolean suppress_clear;
	struct render_camera camera;
	struct render_frustum frustum;
	struct render_fog fog;
	struct render_screen_flash screen_flash;
	struct render_screen_effect screen_effect;
};

struct rasterizer_debug_options_struct
{
	boolean fps_accumulation;
	short statistics_mode;
	short drawing_mode;
	boolean wireframe_enabled;
	boolean debug_model_vertices_enabled;
	short debug_model_lod;
	boolean debug_transparent_geometry_enabled;
	boolean debug_meter_shader_enabled;
	boolean draw_models;
	boolean draw_model_transparent_geometry;
	boolean draw_first_person_weapon_first;
	boolean stencil_mask_enabled;
	boolean draw_environment;
	boolean draw_environment_lightmaps;
	boolean draw_environment_shadows;
	boolean draw_environment_diffuse_lights;
	boolean draw_environment_textures;
	boolean draw_environment_decals;
	boolean draw_environment_specular_lights;
	boolean draw_environment_specular_lightmaps;
	boolean draw_environment_reflection_lightmap_masks;
	boolean draw_environment_reflection_mirrors;
	boolean draw_environment_reflections;
	boolean draw_environment_transparent_geometry;
	boolean draw_environment_fog;
	boolean draw_environment_fog_screen;
	boolean draw_water;
	boolean draw_lens_flares;
	boolean draw_dynamic_unlit_geometry;
	boolean draw_dynamic_lit_geometry;
	boolean draw_dynamic_screen_geometry;
	boolean draw_hud_motion_sensor;
	boolean draw_detail_objects;
	boolean draw_debug_geometry;
	boolean debug_geometry_multipass;
	boolean fog_atmospheric_enabled;
	boolean fog_planar_enabled;
	boolean bump_mapping_enabled;
	real lightmap_ambient;
	short _lightmap_mode;
	short pad3;
	boolean lightmap_incident_radiosity_enabled;
	boolean lightmap_filtering_enabled;
	real model_lighting_ambient;
	boolean environment_alpha_testing_enabled;
	boolean environment_specular_mask_enabled;
	boolean shadow_convolution_enabled;
	boolean shadow_debug_enabled;
	boolean water_mipmapping_enabled;
	boolean active_camouflage_enabled;
	boolean active_camouflage_multipass_enabled;
	boolean plasma_energy_enabled;
	boolean lens_flare_occlusion_enabled;
	boolean lens_flare_occlusion_debug;
	boolean lens_flare_sun_glow_enabled;
	boolean screen_flash_enabled;
	boolean screen_effects_enabled;
	boolean DXTC_noise_enabled;
	boolean soft_filter_enabled;
	boolean secondary_render_target_debug_enabled;
	boolean profile_log_enabled;
	real detail_object_screen_facing_offset_multiplier;
	long zbias;
	real zoffset;
	boolean force_all_player_views_to_default_player;
	boolean safe_frame_bounds_adjust_enabled;
	short freeze_flying_camera;
	boolean zsprite_enabled;
	boolean filthy_decal_fog_hack_enabled;
};

struct rasterizer_frame_statistics_s
{
	real fps;
	short fps_sample_count;
	short pad;
	real fps_average;
	real fps_min;
	real fps_max;
	long fogged_count;
	long normal_count;
	long fast_count;
	long scenery_count;
	long environment_lightmap_vertex_count;
	long environment_lightmap_triangle_count;
	long environment_lightmap_primitive_count;
	long environment_shadow_count;
	long environment_shadow_vertex_count;
	long environment_shadow_triangle_count;
	long environment_shadow_primitive_count;
	long environment_light_vertex_count;
	long environment_light_triangle_count;
	long environment_light_primitive_count;
	long decal_vertex_count;
	long decal_triangle_count;
	long decal_primitive_count;
	long decal_shader_count;
	long decal_texture_count;
};

struct rasterizer_lens_flare_submit_parameters
{
	struct lens_flare_definition *definition;
	real_point3d position;
	unsigned long compressed_direction;
	unsigned long compressed_up;
	unsigned long compressed_light_color;
	short light_identifier;
	short light_index;
	short lens_flare_index;
	byte compressed_window_index;
	byte compressed_light_scale;
	long internal__occlusion_pixels;
};

/* ---------- prototypes/RASTERIZER.C */

void rasterizer_reset_state(void);
void rasterizer_set_vblank_callback(void (*callback)(unsigned long));
void rasterizer_debug_draw(void);
void rasterizer_transparent_geometry_draw(boolean water);

boolean rasterizer_initialize(void);

void rasterizer_frame_begin(const struct rasterizer_frame_begin_parameters *parameters);
boolean rasterizer_windows_begin(void);
void rasterizer_window_begin(const struct rasterizer_window_begin_parameters *parameters);

void rasterizer_window_end(void);
void rasterizer_windows_end(void);
void rasterizer_frame_end(void);

void rasterizer_present(struct bitmap_data *screenshot_bitmap, const point2d *screenshot_index);
void rasterizer_dispose(void);

void rasterizer_decals_update_function_pointers(void);

/* ---------- prototypes/RASTERIZER_TEXT.C */

void rasterizer_draw_string(union rectangle2d const *bounds, union rectangle2d const *clip, union point2d *cursor_reference, short height_adjust, char const *string);

/* ---------- prototypes/RASTERIZER_MEMORY_POOL.C */

boolean rasterizer_memory_pool_initialize(void);
void rasterizer_memory_pool_begin(void);
void *rasterizer_memory_alloc(const void *src, unsigned long size);
const void *rasterizer_memory_alloc_const(const void *src, unsigned long size);
void rasterizer_memory_pool_end(void);
void rasterizer_memory_pool_dispose(void);

/* ---------- prototypes/RASTERIZER_LIGHTS.C */

void rasterizer_lights_reset_for_new_map(void);

/* ---------- prototypes/RASTERIZER_TEXT.C */

void rasterizer_text_cache_flush(void);

/* ---------- globals */

struct rasterizer_globals_struct
{
	boolean active;
	short current_lock_operation;
	rectangle2d screen_bounds;
	rectangle2d frame_bounds;
	byte __unknown14[4];
	__int64 frame_index;
	unsigned long flip_index;
	volatile __int64 vblank_index;
	volatile __int64 flip_vblank_index;
	byte __unknown38[5];
	boolean use_rasterizer_frame_rate_throttle;
	boolean use_rasterizer_frame_rate_stabilization;
	short refresh_rate;
	real z_near;
	real z_far;
};

extern struct rasterizer_globals_struct rasterizer_globals;

extern real_argb_color *global_rasterizer_model_ambient_reflection_tint;

/* comm. not sure where this should be */
struct rasterizer_frame_begin_parameters global_frame_parameters;

extern struct rasterizer_frame_statistics_s rasterizer_frame_statistics;
extern struct rasterizer_debug_options_struct rasterizer_debug_options;
extern struct rasterizer_window_begin_parameters global_window_parameters;

/* ---------- public code */

#endif // __RASTERIZER_H
