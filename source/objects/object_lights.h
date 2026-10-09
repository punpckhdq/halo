/*
OBJECT_LIGHTS.H

header included in hcex build.
*/

#ifndef __OBJECT_LIGHTS_H
#define __OBJECT_LIGHTS_H
#pragma once

/* ---------- constants */

enum
{
	_distant_lighting_raycast_sideways_bit = 0,
	_distant_lighting_block_on_textures_bit,
	_distant_lighting_brighten_bit,
	NUMBER_OF_STATIC_LIGHTING_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/OBJECT_LIGHTS.C */

void sample_lightmap(struct structure_material const *material, struct bitmap_data const *bitmap, struct structure_surface const *surface, real s, real t, union real_rgb_color *lightmap_sample);
void sample_diffuse_texture(struct structure_material const *material, struct bitmap_data const *bitmap, struct structure_surface const *surface, real s, real t, union real_rgb_color *diffuse_sample);
void lights_initialize(void);
void lights_dispose(void);
void lights_initialize_for_new_map(void);
void lights_dispose_from_old_map(void);
boolean lights_enable(boolean enable);
long light_new(long definition_index, long object_index, short object_attachment_index, short object_function_index, short object_change_color_index);
void light_delete(long light_index);
long light_new_unattached(long definition_index, long object_index, short node_index, union real_point3d const *position, union real_vector3d const *forward, real scale);
void lights_preprocess_scene(void);
void lights_queue_lens_flare(long definition_index, union real_point3d const *position, union real_vector3d const *direction, union real_vector3d const *up, union real_rgb_color const *color, real scale);
void lights_render_diffuse(void);
void lights_render_specular(void);
real object_get_self_illumination(long object_index);
void lights_illumination_at_point(union real_point3d const *point, struct location const *location, union real_rgb_color *color);
void light_particle(union real_point3d const *point, union real_rgb_color *light_color, union real_rgb_color *diffuse_color, boolean block);
void lights_prepare_for_object_static(long object_index, struct render_lighting *lighting);
void lights_prepare_for_object_dynamic(long object_index, struct render_lighting *lighting);
boolean lights_distant_lighting_at_point(long flags, union real_point3d const *position, struct render_lighting *lighting);
void light_disconnect_from_map(long light_index);
void light_reconnect_to_map(long light_index);
void lights_disconnect_from_structure_bsp(void);
void lights_reconnect_to_structure_bsp(void);

/* ---------- globals */

extern struct render_lighting const default_object_lighting;

extern real object_light_ambient_base;
extern real object_light_ambient_scale;
extern real object_light_secondary_scale;
extern boolean object_light_interpolate;

extern boolean debug_lights;
extern boolean debug_object_lights;
extern short debug_rasterizer_light_count;

/* ---------- public code */

#endif // __OBJECT_LIGHTS_H
