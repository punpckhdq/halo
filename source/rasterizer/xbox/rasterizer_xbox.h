/*
RASTERIZER_XBOX.H
*/

#ifndef __RASTERIZER_XBOX_H
#define __RASTERIZER_XBOX_H
#pragma once

/* ---------- constants */

enum
{
	_rasterizer_stencil_mode_none = 0,
	_rasterizer_stencil_mode_write,
	_rasterizer_stencil_mode_reject,
	_rasterizer_stencil_mode_reject_invert,
	_rasterizer_stencil_mode_write_alpha_tested_decal,
	_rasterizer_stencil_mode_reject_alpha_tested_decal
};

enum
{
	RASTERIZER_MAXIMUM_TEXTURE_STAGES = 4,
	NUMBER_OF_VERTEX_SHADERS = 67
};

enum
{
	VSH_CONSTANTS__VIEWPROJ_OFFSET = -96,
	VSH_CONSTANTS__VIEWPROJ_COUNT = 6,
	VSH_CONSTANTS__EYEXFORM_OFFSET = -90,
	VSH_CONSTANTS__EYEXFORM_COUNT = 2,
	VSH_CONSTANTS__FOG_OFFSET = -88,
	VSH_CONSTANTS__FOG_COUNT = 4,
	VSH_CONSTANTS__TEXSCALE_OFFSET = -84,
	VSH_CONSTANTS__TEXSCALE_COUNT = 3,
	VSH_CONSTANTS__DETAILOBJ_OFFSET = -81,
	VSH_CONSTANTS__DETAILOBJ_COUNT = 6,
	VSH_CONSTANTS__VERTEXLIGHT_OFFSET = -79,
	VSH_CONSTANTS__VERTEXLIGHT_COUNT = 11,
	VSH_CONSTANTS__DETAILOBJ_TYPEDATA_OFFSET = -75,
	VSH_CONSTANTS__SCREENPROJ_OFFSET = -68,
	VSH_CONSTANTS__SCREENPROJ_COUNT = 5,
	VSH_CONSTANTS__SCREENPROJ2_OFFSET = -63,
	VSH_CONSTANTS__SCREENPROJ2_COUNT = 6,
	VSH_CONSTANTS__NODEMATRIX_OFFSET = -36,
	VSH_CONSTANTS__DETAILOBJ_FRAMEDATA_OFFSET = -36
};

/* ---------- macros */

#define VSDE_VERTEX 0

#define RASTERIZER_DYNAMIC_BUFFER_USAGE (D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY) /* fake name */
#define RASTERIZER_DYNAMIC_BUFFER_POOL D3DPOOL_DEFAULT /* fake name */

// original name unknown
#define D3DCALL(success, call) { HRESULT result = (call); (success) = (success) && SUCCEEDED(result); if (!(success)) rasterizer_error(result, #call); }

/* ---------- structures */

struct rasterizer_vertex_shader /* fake name */
{
	unsigned long *declaration;
	unsigned long *code;
	unsigned long handle;
	long size; /* fake name */
};

/* ---------- prototypes/RASTERIZER_XBOX.C */

boolean rasterizer_preinitialize__fill_you_up_with_the_devils_cock(void);
void *rasterizer_get_bitmap_default_hardware_format(struct bitmap_data const *bitmap);
boolean _rasterizer_initialize(void);
void _rasterizer_reset_state(void);
void rasterizer_spin_begin(void);
void rasterizer_spin_end(void);
void _rasterizer_frame_begin(struct rasterizer_frame_begin_parameters const *parameters);
boolean _rasterizer_windows_begin(void);
void _rasterizer_window_begin(struct rasterizer_window_begin_parameters const *parameters);
void _rasterizer_window_get_fog(struct render_fog *fog);
void _rasterizer_window_set_fog(struct render_fog const *fog);
void _rasterizer_window_end(void);
void _rasterizer_windows_end(void);
void _rasterizer_frame_end(void);
void _rasterizer_present(struct bitmap_data *screenshot_bitmap, point2d const *screenshot_index);
void _rasterizer_dispose(void);
void _rasterizer_set_vblank_callback(void (*callback)(unsigned long));
void rasterizer_set_framebuffer_blend_function(short framebuffer_blend_function);
boolean rasterizer_set_texture_bitmap_data(short stage, struct bitmap_data const *bitmap);
boolean rasterizer_set_texture_direct(short stage, long bitmap_group_index, short bitmap_index);
boolean rasterizer_set_texture_direct_non_blocking(short stage, long bitmap_group_index, short bitmap_index);
point2d *rasterizer_set_texture(short stage, short type, short usage, long bitmap_group_index, short bitmap_index);
boolean rasterizer_set_texture_non_blocking(short stage, short type, short usage, long bitmap_group_index, short bitmap_index);
IDirect3DSurface8 *rasterizer_get_target(short rasterizer_target, short mipmap_index);
void rasterizer_set_target(short rasterizer_target, short mipmap_index, pixel32 clear_color, boolean clear, boolean zbuffer);
void rasterizer_set_target_as_texture(short stage, short rasterizer_target, short max_mipmap);
void rasterizer_secondary_render_target_debug(rectangle2d const *bounds);
void rasterizer_set_vertex_shader(short vertex_shader_index);
void rasterizer_set_pixel_shader(D3DPIXELSHADERDEF const *shader);
void rasterizer_set_model_skinning(struct render_skinning const *skinning);
void rasterizer_set_model_lighting_point_light(long light_index, short light_num, struct rasterizer_model_lighting_constants *lighting_constants);
void rasterizer_set_model_lighting_distant_light(struct render_distant_light const *light, short light_num, struct rasterizer_model_lighting_constants *lighting_constants);
void rasterizer_set_model_lighting(struct render_lighting const *lighting);
void rasterizer_set_frustum_z(real z_near, real z_far);
void rasterizer_set_stencil_mode(short stencil_mode);
short rasterizer_get_stencil_mode(void);
void SetupSmartStates(void);
void SetRenderStateSmart(D3DRENDERSTATETYPE state, unsigned long value);
void SetTextureStageStateSmart(unsigned long stage, D3DTEXTURESTAGESTATETYPE type, unsigned long value);

/* ---------- prototypes/RASTERIZER_XBOX_ERRORS.C */

void rasterizer_error(HRESULT hr, char const *format, ...);

/* ---------- prototypes/RASTERIZER_XBOX_PROFILE.C */

boolean rasterizer_profile_initialize(void);
void rasterizer_profile_dispose(void);
void rasterizer_profile_frame_begin(void);
void rasterizer_profile_frame_end(void);
void rasterizer_profile_window_begin(void);
void rasterizer_profile_window_end(void);
void _rasterizer_profile_enable(boolean enable);
void rasterizer_profile_begin(short profile);
void rasterizer_profile_end(short profile);

/* ---------- prototypes/RASTERIZER_XBOX_VERTEX_SHADERS_INITIALIZE.C */

boolean rasterizer_vertex_shaders_initialize(void);
void rasterizer_vertex_shaders_dispose(void);

/* ---------- prototypes/RASTERIZER_XBOX_VERTEX_SHADERS_RUNTIME.C */

boolean rasterizer_set_vertex_shader_permutation(short vertex_shader_index, short vertex_type, short permutation_index);

/* ---------- prototypes/RASTERIZER_XBOX_DECALS.C */

void _rasterizer_decals_initialize(void);
void _rasterizer_decals_update_function_pointers(void);
void _rasterizer_decals_initialize_for_new_map(void);
void _rasterizer_decals_dispose_from_old_map(void);
void _rasterizer_decals_flush(void);
void _rasterizer_decals_dispose(void);
void rasterizer_decal_vertices_begin_update(void);
void rasterizer_decal_vertices_end_update(void);
long _rasterizer_decal_vertices_new(long cache_size);
void *_rasterizer_decal_vertices_lock(long cache_index, long cache_size);
void _rasterizer_decal_vertices_unlock(void);
void _rasterizer_decal_vertices_delete(long cache_index);
void _rasterizer_decals_begin(short layer);
void _rasterizer_decals_draw(short cluster_index);
void _rasterizer_decals_end(void);

/* ---------- prototypes/RASTERIZER_XBOX_DRAW_PRIMITIVES.C */

boolean rasterizer_dynamic_geometry_initialize(void);
void rasterizer_dynamic_geometry_begin(void);
void rasterizer_dynamic_geometry_end(void);
void rasterizer_dynamic_geometry_dispose(void);
long _rasterizer_dynamic_triangles_new(long count);
struct rasterizer_triangle *_rasterizer_dynamic_triangles_lock(long dynamic_triangle_buffer_index);
void _rasterizer_dynamic_triangles_unlock(long dynamic_triangle_buffer_index);
void _rasterizer_dynamic_triangles_delete(long dynamic_triangle_buffer_index);
long _rasterizer_dynamic_vertices_new(short type, long count);
short _rasterizer_dynamic_vertices_get_type(long dynamic_vertex_buffer_index);
void *_rasterizer_dynamic_vertices_lock(long dynamic_vertex_buffer_index);
void _rasterizer_dynamic_vertices_unlock(long dynamic_vertex_buffer_index);
void _rasterizer_dynamic_vertices_delete(long dynamic_vertex_buffer_index);
void rasterizer_draw_dynamic_vertices(long first_primitive_index, long primitive_count, long dynamic_vertex_buffer_index, short vertices_per_primitive);
void rasterizer_draw_dynamic_triangles_dynamic_vertices(long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, long dynamic_vertex_buffer_index);
void rasterizer_draw_dynamic_triangles_static_vertices(long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_draw_dynamic_triangles_static_vertices2(long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer0, struct vertex_buffer const *vertex_buffer1);
void rasterizer_draw_static_triangles_dynamic_vertices(struct triangle_buffer const *triangle_buffer, long first_triangle_index, long triangle_count, long dynamic_vertex_buffer_index);
void rasterizer_draw_static_triangles_static_vertices(struct triangle_buffer const *triangle_buffer, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void rasterizer_draw(struct triangle_buffer const *triangle_buffer, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer, long dynamic_vertex_buffer_index);

/* ---------- prototypes/RASTERIZER_XBOX_DEBUG.C */

void rasterizer_debug_drawing_begin(boolean opaque, long z_bias);
void rasterizer_debug_drawing_end(void);
void _rasterizer_debug_immediate_begin(void);
void _rasterizer_debug_immediate_line(real_point3d const *p0, real_point3d const *p1, real_rgb_color const *color0, real_rgb_color const *color1);
void _rasterizer_debug_immediate_triangle(real_point3d const *p0, real_point3d const *p1, real_point3d const *p2, real_rgb_color const *color0, real_rgb_color const *color1, real_rgb_color const *color2);
void _rasterizer_debug_immediate_end(void);
void _rasterizer_debug_immediate_begin_screenspace(void);
void _rasterizer_debug_immediate_line_screenspace(point2d const *p0, point2d const *p1, real_rgb_color const *color0, real_rgb_color const *color1);
void _rasterizer_debug_immediate_linestrip_screenspace(point2d const *points, short point_count, real_rgb_color const *color);
void _rasterizer_debug_immediate_end_screenspace(void);

/* ---------- prototypes/RASTERIZER_XBOX_DETAIL_OBJECTS.C */

boolean rasterizer_detail_objects_initialize(void);
void rasterizer_detail_objects_dispose(void);
void _rasterizer_detail_objects_begin(void);
void _rasterizer_detail_objects_rebuild_vertices(struct detail_object_view_data const *detail_object_view_data);
void _rasterizer_detail_objects_draw(struct detail_object_view_data const *detail_object_view_data);
void _rasterizer_detail_objects_end(void);

/* ---------- prototypes/RASTERIZER_XBOX_ACTIVE_CAMOUFLAGE.C */

void rasterizer_active_camouflage_set_visibility(boolean visibility);
void rasterizer_active_camouflage_cache_primary_render_target(void);
void rasterizer_active_camouflage_draw(struct transparent_geometry_group const *group);

/* ---------- prototypes/RASTERIZER_XBOX_SCREEN_EFFECT.C */

void _rasterizer_screen_effect(struct rasterizer_screen_effect_parameters const *parameters);
void _rasterizer_screen_flash(void);

/* ---------- prototypes/RASTERIZER_XBOX_MODELS.C */

void _rasterizer_models_begin(boolean sky);
void _rasterizer_model_begin(struct rasterizer_model_begin_parameters const *parameters, boolean do_not_change_z_stencil_states);
void _rasterizer_model_draw(struct shader const *shader, short shader_permutation_index, struct triangle_buffer const *triangle_buffer, long dynamic_triangle_buffer_index, long triangle_count, struct vertex_buffer const *vertex_buffer, long dynamic_vertex_buffer_index);
void _rasterizer_model_transparent_geometry_submit(struct shader const *shader, short shader_permutation_index, struct triangle_buffer const *triangle_buffer, long dynamic_triangle_buffer_index, long triangle_count, struct vertex_buffer const *vertex_buffer, long dynamic_vertex_buffer_index, real_point3d const *centroid, struct render_sort_filth *sort_filth);
void _rasterizer_model_end(void);
void _rasterizer_models_end(void);

/* ---------- prototypes/RASTERIZER_XBOX_ENVIRONMENT.C */

void _rasterizer_environment_lightmaps_begin(void);
void _rasterizer_environment_lightmap_begin(struct bitmap_data const *lightmap);
void _rasterizer_environment_lightmap_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffers);
void _rasterizer_environment_lightmap_end(void);
void _rasterizer_environment_lightmaps_end(void);
void _rasterizer_environment_diffuse_lights_begin(void);
void _rasterizer_environment_diffuse_light_begin(long light_index);
void _rasterizer_environment_diffuse_light_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_diffuse_light_end(void);
void _rasterizer_environment_diffuse_lights_end(void);
void _rasterizer_environment_diffuse_textures_begin(void);
void _rasterizer_environment_diffuse_texture_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_diffuse_textures_end(void);
void _rasterizer_environment_specular_lights_begin(void);
void _rasterizer_environment_specular_light_begin(long light_index);
void _rasterizer_environment_specular_light_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_specular_light_end(void);
void _rasterizer_environment_specular_lights_end(void);
void _rasterizer_environment_specular_lightmaps_begin(void);
void _rasterizer_environment_specular_lightmap_begin(struct bitmap_data const *lightmap);
void _rasterizer_environment_specular_lightmap_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_specular_lightmap_end(void);
void _rasterizer_environment_specular_lightmaps_end(void);
void _rasterizer_environment_reflection_lightmap_masks_begin(void);
void _rasterizer_environment_reflection_lightmap_mask_begin(struct bitmap_data const *lightmap);
void _rasterizer_environment_reflection_lightmap_mask_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_reflection_lightmap_mask_end(void);
void _rasterizer_environment_reflection_lightmap_masks_end(void);
void _rasterizer_environment_reflection_mirrors_begin(void);
void _rasterizer_environment_reflection_mirror_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_reflection_mirrors_end(void);
void _rasterizer_environment_reflections_begin(void);
void _rasterizer_environment_reflection_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_reflections_end(void);
void _rasterizer_environment_transparent_geometry_begin(void);
void _rasterizer_environment_transparent_geometry_submit(struct shader const *shader, short shader_permutation_index, struct bitmap_data const *lightmap, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffers, real_point3d const *centroid, real_plane3d const *plane, real_vector3d const *vector, struct render_lighting const *lighting, unsigned long geometry_flags);
void _rasterizer_environment_transparent_geometry_end(void);

/* ---------- prototypes/RASTERIZER_XBOX_SHADOWS.C */

void _rasterizer_environment_shadows_begin(void);
boolean _rasterizer_environment_shadow_begin(long object_index, real_matrix4x3 const *shadow_matrix, real_rgb_color const *light_color, real object_bounding_radius, real *shadow_volume_bounding_radius);
void _rasterizer_environment_shadow_model_begin(struct rasterizer_model_begin_parameters const *parameters);
void _rasterizer_environment_shadow_model_draw(struct shader const *shader, short shader_permutation_index, struct triangle_buffer const *triangle_buffer, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_shadow_model_end(void);
void _rasterizer_environment_shadow_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_shadow_end(void);
void _rasterizer_environment_shadows_end(void);

/* ---------- prototypes/RASTERIZER_XBOX_ENVIRONMENT_FOG.C */

boolean rasterizer_environment_fog_screen_initialize(void);
void rasterizer_environment_fog_screen_window_begin(void);
void rasterizer_environment_fog_screen_window_end(void);
void rasterizer_environment_fog_screen_dispose(void);
void _rasterizer_environment_fog_begin(void);
void _rasterizer_environment_fog_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_fog_end(void);
void _rasterizer_environment_fog_screen_wind_get_vector(short window_index, real dt, real_vector3d *wind_vector);
void _rasterizer_environment_fog_screen_begin(short pass);
void _rasterizer_environment_fog_screen_draw(struct shader const *shader, short shader_permutation_index, long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_fog_screen_end(void);

/* ---------- prototypes/RASTERIZER_XBOX_DYNAVOBGEOM.C */

void _rasterizer_hud_begin(void);
void _rasterizer_hud_end(void);
void _rasterizer_dynamic_unlit_geometry_draw(struct shader const *shader, struct bitmap_data const *primary_map, struct render_animation const *animation, long dynamic_triangle_buffer_index, long dynamic_vertex_buffer_index, long triangle_count, real_point3d const *centroid, unsigned long geometry_flags);
void _rasterizer_dynamic_lit_geometry_draw(struct shader const *shader, struct bitmap_data const *primary_map, struct render_animation const *animation, struct render_lighting const *lighting, long dynamic_triangle_buffer_index, long dynamic_vertex_buffer_index, long triangle_count, real_point3d const *centroid, unsigned long geometry_flags);
void _rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(struct rasterizer_dynamic_screen_geometry_parameters *base, struct rasterizer_dynamic_screen_geometry_parameters const *multitex_params);
void _rasterizer_dynamic_screen_geometry_draw(struct rasterizer_dynamic_screen_geometry_parameters const *parameters, long dynamic_triangle_buffer_index, long dynamic_vertex_buffer_index, long triangle_count);
void _rasterizer_psuedo_dynamic_screen_quad_draw(struct rasterizer_dynamic_screen_geometry_parameters const *parameters, struct dynamic_screen_vertex *verts);

/* ---------- prototypes/RASTERIZER_XBOX_LIGHTS.C */

void rasterizer_sun_glow_draw(struct rasterizer_lens_flare_submit_parameters const *parameters);

/* ---------- prototypes/RASTERIZER_XBOX_TRANSPARENT_GEOMETRY.C */

void rasterizer_transparent_geometry_groups_begin(void);
void rasterizer_transparent_geometry_group_draw(struct transparent_geometry_group const *group, boolean dirty);
void rasterizer_transparent_geometry_group_draw__internal(struct transparent_geometry_group const *group, boolean has_lightmap);
void rasterizer_transparent_geometry_groups_end(void);
boolean rasterizer_transparent_geometry_initialize_aux_buffer(void);
void rasterizer_transparent_geometry_dispose_aux_buffer(void);

/* ---------- prototypes/RASTERIZER_XBOX_TEXT.C */

void rasterizer_text_begin(struct rasterizer_dynamic_screen_geometry_parameters const *parameters);
void rasterizer_text_draw_character(struct dynamic_screen_vertex const *vertices);
void rasterizer_text_end(void);

/* ---------- prototypes/RASTERIZER_XBOX_WATER.C */

void rasterizer_water_set_visibility_for_frame(boolean visibility);
void rasterizer_water_set_visibility_for_window(boolean visibility);

/* ---------- prototypes/RASTERIZER_XBOX_WIDGETS.C */

void _rasterizer_widget_submit(long object_index, long widget_index, real_point3d const *centroid, void (*render_proc)(long, long));
void _rasterizer_widget_begin(short type, word flags);
boolean _rasterizer_widget_set_texture(short stage_index, long bitmap_group_index, short sequence_index);
void _rasterizer_widget_set_tint_factor(real tint_factor);
void _rasterizer_widget_set_zbuffer_enable(boolean zbuffer_enable);
void _rasterizer_widget_draw_sprite2d(real_point2d const *point, real radius, real_vector2d const *scale, real_vector2d const *texture_size, real rotation, pixel32 color);
void _rasterizer_widget_draw_sprite3d(real_point3d const *point, real radius, real_vector2d const *scale, real rotation, pixel32 color);
void _rasterizer_widget_end(void);
long _rasterizer_widget_submit_occlusion_test(real_point3d const *point, real radius, long index);
long _rasterizer_widget_get_occlusion_test_result(long index);

/* ---------- prototypes/RASTERIZER_XBOX_MOTION_SENSOR.C */

void _rasterizer_hud_motion_sensor_blip_begin(void);
void _rasterizer_hud_motion_sensor_blip_draw(real_point2d const *blip_position, real fade, real radius, real_rgb_color const *blip_color, boolean custom);
void _rasterizer_hud_motion_sensor_blip_end(real_point2d const *center_point, real theta);

/* ---------- globals */

extern IDirect3DDevice8 *global_d3d_device;
extern D3DCAPS8 global_d3d_caps;

extern D3DPIXELSHADERDEF pixel_shader;

extern unsigned long renderstate_table[D3DRS_MAX];
extern unsigned long texturestagestate_table[D3DTSS_MAXSTAGES][D3DTSS_MAX];
extern IDirect3DBaseTexture8 *texture_table[D3DTSS_MAXSTAGES];
extern struct rasterizer_vertex_shader vertex_shader_table[NUMBER_OF_VERTEX_SHADERS];

/* ---------- public code */

#endif // __RASTERIZER_XBOX_H
