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
	_lens_flare_parameters_light_index_structure_bit = 15,
	_lens_flare_parameters_light_index_mask = MASK(_lens_flare_parameters_light_index_structure_bit),
};

enum
{
	_lens_flare_window_index_first_person_bit = 7,
	_lens_flare_window_index_mask = ~FLAG(_lens_flare_window_index_first_person_bit),
};

enum
{
	_rasterizer_statistics_mode_none = 0,
	_rasterizer_statistics_mode_fps_only,
	_rasterizer_statistics_mode_full,
	_rasterizer_statistics_mode_profile,
	_rasterizer_statistics_mode_memory,
	NUMBER_OF_RASTERIZER_STATISTICS_MODES,
};

enum
{
	_rasterizer_drawing_mode_normal = 0,
	_rasterizer_drawing_mode_overdraw,
	_rasterizer_drawing_mode_bump_color,
	_rasterizer_drawing_mode_specular_mask,
	_rasterizer_drawing_mode_specular_mask_times_bump_color,
	_rasterizer_drawing_mode_diffuse_texture_times_bump_color,
	_rasterizer_drawing_mode_bump_edge,
	_rasterizer_drawing_mode_specular_mask_times_bump_edge,
	_rasterizer_drawing_mode_diffuse_texture_times_bump_edge,
	_rasterizer_drawing_mode_vectors,
	NUMBER_OF_RASTERIZER_DRAWING_MODES,
};

enum
{
	_rasterizer_geometry_no_sort_bit = 0,
	_rasterizer_geometry_no_queue_bit,
	_rasterizer_geometry_no_fog_bit,
	_rasterizer_geometry_no_zbuffer_bit,
	_rasterizer_geometry_sky_bit,
	_rasterizer_geometry_viewspace_bit,
	_rasterizer_geometry_atmospheric_fog_but_no_planar_fog_bit,
	_rasterizer_geometry_first_person_bit,
	_rasterizer_geometry_parts_define_local_nodes_bit,
};

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

struct dynamic_unlit_vertex
{
	real_point3d position;
	pixel32 color;
	real_point2d texcoord;
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
	boolean smart_states_enabled;
	boolean splitscreen_VB_optimization_enabled;
	boolean profile_print_locks;
	real profile_objectlock_time;
	real pad3_scale;
	real f[6];
	boolean __unknown88;
	boolean transparent_pixel_counter_enabled; /* fake name */
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
	long __unknown60[56];
	long debug_primitive_count; /* fake name */
	long __unknown144;
	long dynamic_light_count; /* fake name */
	long lens_flare_count; /* fake name */
	long __unknown150[8];
};

struct rasterizer_model_begin_parameters
{
	unsigned long geometry_flags;
	unsigned long unique_id;
	struct render_skinning skinning;
	struct render_lighting lighting;
	struct render_animation animation;
	struct render_model_effect effect;
	real_point3d centroid;
	real radius;
	real_vector2d base_map_scale;
};

struct transparent_geometry_group
{
	unsigned long geometry_flags;
	long object_index;
	long source_object_index;
	struct shader const *shader;
	short shader_permutation_index;
	struct render_model_effect effect;
	real_vector2d model_base_map_scale;
	long dynamic_triangle_buffer_index;
	struct triangle_buffer const *triangle_buffer;
	long first_triangle_index;
	long triangle_count;
	long dynamic_vertex_buffer_index;
	struct vertex_buffer const *vertex_buffers;
	struct bitmap_data const *lightmap;
	real_matrix4x3 const *node_matrices;
	short node_matrix_count;
	struct render_lighting const *lighting;
	struct render_animation const *animation;
	real z_sort;
	real_point3d centroid;
	real_plane3d plane;
	long sorted_index;
	short prev_group_presorted_index;
	short next_group_presorted_index;
	long active_camouflage_transparent_source_object_index;
	boolean sort_last;
	boolean cortana_hack;
};

struct rasterizer_global_defaults
{
	real z_near;
	real z_far;
	real z_near_first_person;
	real z_far_first_person;
};

struct rasterizer_light_submit_parameters
{
	struct point_light_definition *definition;
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real_rgb_color color;
	real radius;
};

struct rasterizer_lights
{
	long light_count;
	struct rasterizer_light_submit_parameters lights[MAXIMUM_LIGHTS_PER_WINDOW];
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

boolean rasterizer_initialize(void);
void rasterizer_reset_state(void);
void rasterizer_frame_begin(struct rasterizer_frame_begin_parameters const *parameters);
boolean rasterizer_windows_begin(void);
void rasterizer_window_begin(struct rasterizer_window_begin_parameters const *parameters);
void rasterizer_window_get_fog(struct render_fog *fog);
void rasterizer_window_set_fog(struct render_fog const *fog);
void rasterizer_window_end(void);
void rasterizer_windows_end(void);
void rasterizer_frame_end(void);
void rasterizer_present(struct bitmap_data *screenshot_bitmap, point2d const *screenshot_index);
void rasterizer_dispose(void);
void rasterizer_set_vblank_callback(void (*callback)(unsigned long));
void rasterizer_profile_enable(boolean enable);
long rasterizer_dynamic_triangles_new(long count);
struct rasterizer_triangle *rasterizer_dynamic_triangles_lock(long dynamic_triangle_buffer_index);
void rasterizer_dynamic_triangles_unlock(long dynamic_triangle_buffer_index);
void rasterizer_dynamic_triangles_delete(long dynamic_triangle_buffer_index);
long rasterizer_dynamic_vertices_new(short type, long count);
short rasterizer_dynamic_vertices_get_type(long dynamic_vertex_buffer_index);
void *rasterizer_dynamic_vertices_lock(long dynamic_vertex_buffer_index);
void rasterizer_dynamic_vertices_unlock(long dynamic_vertex_buffer_index);
void rasterizer_dynamic_vertices_delete(long dynamic_vertex_buffer_index);
void rasterizer_debug_immediate_begin(void);
void rasterizer_debug_immediate_point(real_point3d const *p, real size, real_rgb_color const *color);
void rasterizer_debug_immediate_vector(real_point3d const *p, real_vector3d const *v, real size, real_rgb_color const *color);
void rasterizer_debug_immediate_line(real_point3d const *p0, real_point3d const *p1, real_rgb_color const *color0, real_rgb_color const *color1);
void rasterizer_debug_immediate_triangle(real_point3d const *p0, real_point3d const *p1, real_point3d const *p2, real_rgb_color const *color0, real_rgb_color const *color1, real_rgb_color const *color2);
void rasterizer_debug_immediate_end(void);
void rasterizer_debug_immediate_begin_screenspace(void);
void rasterizer_debug_immediate_line_screenspace(point2d const *p0, point2d const *p1, real_rgb_color const *color0, real_rgb_color const *color1);
void rasterizer_debug_immediate_linestrip_screenspace(point2d const *points, short point_count, real_rgb_color const *color);
void rasterizer_debug_immediate_end_screenspace(void);
void rasterizer_decals_initialize(void);
void rasterizer_decals_update_function_pointers(void);
void rasterizer_decals_initialize_for_new_map(void);
void rasterizer_decals_dispose_from_old_map(void);
void rasterizer_decals_flush(void);
void rasterizer_decals_dispose(void);
long rasterizer_decal_vertices_new(long cache_size);
void *rasterizer_decal_vertices_lock(long cache_index, long cache_size);
void rasterizer_decal_vertices_unlock(void);
void rasterizer_decal_vertices_delete(long cache_index);
void rasterizer_decals_begin(short layer);
void rasterizer_decals_draw(short cluster_index);
void rasterizer_decals_end(void);
void rasterizer_detail_objects_begin(void);
void rasterizer_detail_objects_rebuild_vertices(struct detail_object_view_data const *detail_object_view_data);
void rasterizer_detail_objects_draw(struct detail_object_view_data const *detail_object_view_data);
void rasterizer_detail_objects_end(void);
void rasterizer_screen_effect(struct rasterizer_screen_effect_parameters const *parameters);
void rasterizer_screen_flash(void);
void rasterizer_models_begin(boolean sky);
void rasterizer_model_begin(struct rasterizer_model_begin_parameters const *parameters, boolean do_not_change_z_stencil_states);
void rasterizer_model_draw(struct shader const *shader, short shader_permutation_index, struct triangle_buffer const *triangle_buffer, long dynamic_triangle_buffer_index, long triangle_count, struct vertex_buffer const *vertex_buffer, long dynamic_vertex_buffer_index);
void rasterizer_model_transparent_geometry_submit(struct shader const *shader, short shader_permutation_index, struct triangle_buffer const *triangle_buffer, long dynamic_triangle_buffer_index, long triangle_count, struct vertex_buffer const *vertex_buffer, long dynamic_vertex_buffer_index, real_point3d const *centroid, struct render_sort_filth *sort_filth);
void rasterizer_model_end(void);
void rasterizer_models_end(void);
void rasterizer_debug_model_vertices(long target_object_index, struct render_skinning const *skinning, struct model_geometry_part const *part);
void rasterizer_environment_lightmaps_begin(void);
void rasterizer_environment_lightmap_begin(struct bitmap_data const *lightmap);
void rasterizer_environment_lightmap_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffers);
void rasterizer_environment_lightmap_end(void);
void rasterizer_environment_lightmaps_end(void);
void rasterizer_environment_diffuse_lights_begin(void);
void rasterizer_environment_diffuse_light_begin(long light_index);
void rasterizer_environment_diffuse_light_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_diffuse_light_end(void);
void rasterizer_environment_diffuse_lights_end(void);
void rasterizer_environment_shadows_begin(void);
boolean rasterizer_environment_shadow_begin(long object_index, real_matrix4x3 const *shadow_matrix, real_rgb_color const *light_color, real object_bounding_radius, real *shadow_volume_bounding_radius);
void rasterizer_environment_shadow_model_begin(struct rasterizer_model_begin_parameters const *parameters);
void rasterizer_environment_shadow_model_draw(struct shader const *shader, short shader_permutation_index, struct triangle_buffer const *triangle_buffer, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_shadow_model_end(void);
void rasterizer_environment_shadow_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_shadow_end(void);
void rasterizer_environment_shadows_end(void);
void rasterizer_environment_diffuse_textures_begin(void);
void rasterizer_environment_diffuse_texture_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_diffuse_textures_end(void);
void rasterizer_environment_specular_lights_begin(void);
void rasterizer_environment_specular_light_begin(long light_index);
void rasterizer_environment_specular_light_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_specular_light_end(void);
void rasterizer_environment_specular_lights_end(void);
void rasterizer_environment_specular_lightmaps_begin(void);
void rasterizer_environment_specular_lightmap_begin(struct bitmap_data const *lightmap);
void rasterizer_environment_specular_lightmap_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_specular_lightmap_end(void);
void rasterizer_environment_specular_lightmaps_end(void);
void rasterizer_environment_reflection_lightmap_masks_begin(void);
void rasterizer_environment_reflection_lightmap_mask_begin(struct bitmap_data const *lightmap);
void rasterizer_environment_reflection_lightmap_mask_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_reflection_lightmap_mask_end(void);
void rasterizer_environment_reflection_lightmap_masks_end(void);
void rasterizer_environment_reflection_mirrors_begin(void);
void rasterizer_environment_reflection_mirror_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_reflection_mirrors_end(void);
void rasterizer_environment_reflections_begin(void);
void rasterizer_environment_reflection_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_reflections_end(void);
void rasterizer_environment_transparent_geometry_begin(void);
void rasterizer_environment_transparent_geometry_submit(struct shader const *shader, short shader_permutation_index, struct bitmap_data const *lightmap, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffers, real_point3d const *centroid, real_plane3d const *plane, real_vector3d const *vector, struct render_lighting const *lighting, unsigned long geometry_flags);
void rasterizer_environment_transparent_geometry_end(void);
void rasterizer_environment_fog_begin(void);
void rasterizer_environment_fog_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_fog_end(void);
void rasterizer_environment_fog_screen_wind_get_vector(short window_index, real dt, real_vector3d *wind_vector);
void rasterizer_environment_fog_screen_begin(short pass);
void rasterizer_environment_fog_screen_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_fog_screen_end(void);
void rasterizer_hud_begin(void);
void rasterizer_hud_end(void);
void rasterizer_dynamic_unlit_geometry_draw(struct shader const *shader, struct bitmap_data const *primary_map, struct render_animation const *animation, long dynamic_triangle_buffer_index, long dynamic_vertex_buffer_index, long triangle_count, real_point3d const *centroid, unsigned long geometry_flags);
void rasterizer_dynamic_lit_geometry_draw(struct shader const *shader, struct bitmap_data const *primary_map, struct render_animation const *animation, struct render_lighting const *lighting, long dynamic_triangle_buffer_index, long dynamic_vertex_buffer_index, long triangle_count, real_point3d const *centroid, unsigned long geometry_flags);
void rasterizer_dynamic_screen_geometry_draw(struct rasterizer_dynamic_screen_geometry_parameters const *parameters, long dynamic_triangle_buffer_index, long dynamic_vertex_buffer_index, long triangle_count);
void rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(struct rasterizer_dynamic_screen_geometry_parameters *base, struct rasterizer_dynamic_screen_geometry_parameters const *multitext_params);
void rasterizer_psuedo_dynamic_screen_quad_draw(struct rasterizer_dynamic_screen_geometry_parameters const *parameters, struct dynamic_screen_vertex *verts);
void rasterizer_widget_submit(long object_index, long widget_index, real_point3d const *centroid, void (*render_proc)(long, long));
void rasterizer_widget_begin(short type, word flags);
boolean rasterizer_widget_set_texture(short stage_index, long bitmap_group_index, short sequence_index);
void rasterizer_widget_set_tint_factor(real tint_factor);
void rasterizer_widget_set_zbuffer_enable(boolean zbuffer_enable);
void rasterizer_widget_draw_sprite2d(real_point2d const *point, real radius, real_vector2d const *scale, real_vector2d const *texture_size, real rotation, pixel32 color);
void rasterizer_widget_draw_sprite3d(real_point3d const *point, real radius, real_vector2d const *scale, real rotation, pixel32 color);
void rasterizer_widget_end(void);
long rasterizer_widget_submit_occlusion_test(real_point3d const *point, real radius, long index);
long rasterizer_widget_get_occlusion_test_result(long index);
void rasterizer_hud_motion_sensor_blip_begin(void);
void rasterizer_hud_motion_sensor_blip_draw(real_point2d const *blip_position, real fade, real radius, real_rgb_color const *blip_color, boolean custom);
void rasterizer_hud_motion_sensor_blip_end(real_point2d const *center_point, real theta);

/* ---------- prototypes/RASTERIZER_DEBUG.C */

long rasterizer_debug_new_primitive(long *count);
boolean rasterizer_debug_initialize(void);
void rasterizer_debug_begin(void);
void rasterizer_debug_end(void);
void rasterizer_debug_dispose(void);
void rasterizer_debug_line(real_point3d const *p0, real_point3d const *p1, real_argb_color const *color);
void rasterizer_debug_line_shaded(real_point3d const *p0, real_point3d const *p1, real_argb_color const *color0, real_argb_color const *color1);
void rasterizer_debug_triangle(real_point3d const *p0, real_point3d const *p1, real_point3d const *p2, real_argb_color const *color);
void rasterizer_debug_triangle_shaded(real_point3d const *p0, real_point3d const *p1, real_point3d const *p2, real_argb_color const *color0, real_argb_color const *color1, real_argb_color const *color2);
void rasterizer_debug_test(void);
void rasterizer_debug_draw(void);

/* ---------- prototypes/RASTERIZER_TRANSPARENT_GEOMETRY.C */

boolean rasterizer_transparent_geometry_initialize(void);
void rasterizer_transparent_geometry_begin(void);
struct transparent_geometry_group *rasterizer_transparent_geometry_new_group(void);
struct transparent_geometry_group *rasterizer_transparent_geometry_new_group2(void);
struct transparent_geometry_group const *rasterizer_transparent_geometry_get_groups2(short *count);
struct transparent_geometry_group const *rasterizer_transparent_geometry_next_group(struct transparent_geometry_group const *group);
struct transparent_geometry_group *rasterizer_transparent_geometry_get_groups(void);
struct transparent_geometry_group const *rasterizer_transparent_geometry_get_group_from_presorted_index(short group_presorted_index);
short rasterizer_transparent_geometry_get_group_presorted_index(struct transparent_geometry_group const *group);
boolean rasterizer_transparent_geometry_get_group_pending_status(struct transparent_geometry_group const *group);
void rasterizer_transparent_geometry_set_group_pending_status(struct transparent_geometry_group const *group, boolean status);
short rasterizer_transparent_geometry_get_primary_vertex_type(struct transparent_geometry_group const *group);
void rasterizer_transparent_geometry_end(void);
void rasterizer_transparent_geometry_dispose(void);
void rasterizer_transparent_geometry_draw(boolean water);
void rasterizer_transparent_geometry_stop(void);

/* ---------- prototypes/RASTERIZER_XBOX_HARDWARE_BITMAPS.C */

boolean rasterizer_bitmap_new(struct bitmap_data *bitmap);
void rasterizer_bitmap_delete(struct bitmap_data *bitmap);
void rasterizer_bitmap_changed(struct bitmap_data *bitmap);

/* ---------- prototypes/RASTERIZER_TEXT.C */

boolean rasterizer_text_cache_initialize(void);
void rasterizer_text_set_shadow_color(pixel32 color);
void rasterizer_draw_string(rectangle2d const *bounds, rectangle2d const *clip, point2d *cursor_reference, short height_adjust, char const *string);
void rasterizer_draw_unicode_string(rectangle2d const *bounds, rectangle2d const *clip, point2d *cursor_reference, short height_adjust, wchar_t const *string);
void rasterizer_text_cache_flush(void);
void rasterizer_text_cache_dispose(void);

/* ---------- prototypes/RASTERIZER_MEMORY_POOL.C */

boolean rasterizer_memory_pool_initialize(void);
void rasterizer_memory_pool_begin(void);
void *rasterizer_memory_alloc(const void *src, unsigned long size);
const void *rasterizer_memory_alloc_const(const void *src, unsigned long size);
void rasterizer_memory_pool_end(void);
void rasterizer_memory_pool_dispose(void);

/* ---------- prototypes/RASTERIZER_LIGHTS.C */

void rasterizer_lights_reset_for_new_map(void);
void rasterizer_lights_begin_for_new_frame(void);
void rasterizer_lights_begin(void);
long rasterizer_light_submit(struct rasterizer_light_submit_parameters const *parameters);
void rasterizer_lens_flare_submit(struct rasterizer_lens_flare_submit_parameters const *parameters);
void rasterizer_lens_flare_submit_for_cluster(short cluster_index);
void rasterizer_lights_end(void);
void rasterizer_lens_flares_submit_occlusion_tests(void);
void rasterizer_lens_flares_draw(void);

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
	short pushbuffer_size;
	short pushbuffer_kickoff_size;
	boolean use_floating_point_zbuffer;
	boolean use_rasterizer_frame_rate_throttle;
	boolean use_rasterizer_frame_rate_stabilization;
	short refresh_rate;
	real z_near;
	real z_far;
	real z_near_first_person;
	real z_far_first_person;
	void *default_white_hardware_format;
	void *default_2d_hardware_format;
	void *default_3d_hardware_format;
	void *default_cm_hardware_format;
	short lightmap_mode;
	short maximum_nodes_per_model;
};

extern struct rasterizer_globals_struct rasterizer_globals;
extern struct rasterizer_lights rasterizer_lights;
extern struct rasterizer_global_defaults const rasterizer_global_defaults;

extern real_argb_color *global_rasterizer_model_ambient_reflection_tint;

/* comm. not sure where this should be */
struct rasterizer_frame_begin_parameters global_frame_parameters;

extern struct rasterizer_frame_statistics_s rasterizer_frame_statistics;
extern struct rasterizer_debug_options_struct rasterizer_debug_options;
extern boolean rasterizer_model_cortana_hack;
extern struct rasterizer_window_begin_parameters global_window_parameters;

/* ---------- public code */

#endif // __RASTERIZER_H
