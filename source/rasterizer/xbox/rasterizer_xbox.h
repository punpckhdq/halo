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

/* ---------- macros */

// original name unknown
#define D3DCALL(success, call) { HRESULT result = (call); (success) = (success) && SUCCEEDED(result); if (!(success)) rasterizer_error(result, #call); }

/* ---------- structures */

/* ---------- prototypes/RASTERIZER_XBOX.C */

void rasterizer_preinitialize__fill_you_up_with_the_devils_cock(void);
void *rasterizer_get_bitmap_default_hardware_format(struct bitmap_data const *bitmap);
union point2d *rasterizer_set_texture(short stage, short type, short usage, long bitmap_group_index, short bitmap_index);
void rasterizer_set_stencil_mode(short stencil_mode);
void rasterizer_set_framebuffer_blend_function(short framebuffer_blend_function);
void rasterizer_set_pixel_shader(D3DPIXELSHADERDEF const *shader);


/* ---------- prototypes/RASTERIZER_XBOX_ERRORS.C */

void rasterizer_error(HRESULT hr, char const *format, ...);

/* ---------- prototypes/RASTERIZER_SWIZZLE.C */

void rasterizer_xbox_bitmap_swizzle2d_byte(void *destination, void const *source, short width, short height);
void rasterizer_xbox_bitmap_swizzle2d_word(void *destination, void const *source, short width, short height);
void rasterizer_xbox_bitmap_swizzle2d_long(void *destination, void const *source, short width, short height);
void rasterizer_xbox_bitmap_swizzle3d_byte(void *destination, void const *source, short width, short height, short depth);
void rasterizer_xbox_bitmap_swizzle3d_word(void *destination, void const *source, short width, short height, short depth);
void rasterizer_xbox_bitmap_swizzle3d_long(void *destination, void const *source, short width, short height, short depth);
short rasterizer_xbox_bitmap_get_max_mipmap_count(struct bitmap_data const *bitmap);
long rasterizer_xbox_bitmap_get_pixel_data_size(struct bitmap_data const *bitmap);

/* ---------- prototypes/RASTERIZER_XBOX_PROFILE.C */

void rasterizer_profile_begin(short profile);
void rasterizer_profile_end(short profile);

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

/* ---------- globals */

extern IDirect3DDevice8 *global_d3d_device;

extern D3DPIXELSHADERDEF pixel_shader;

/* ---------- public code */

#endif // __RASTERIZER_XBOX_H
