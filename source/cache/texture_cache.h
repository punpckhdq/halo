/*
TEXTURE_CACHE.H

header included in hcex build.
*/

#ifndef __TEXTURE_CACHE_H
#define __TEXTURE_CACHE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct texture_cache_debug_options_definition
{
	boolean display_cache_graph;
	boolean display_cache_list;
};

/* ---------- prototypes/XBOX_TEXTURE_CACHE.C */

void texture_cache_new(void);
void texture_cache_delete(void);
void texture_cache_open(void);
void texture_cache_close(void);
void texture_cache_idle(void);
void texture_cache_bitmap_new(long tag_index, struct bitmap_data *bitmap);
void texture_cache_bitmap_delete(struct bitmap_data *bitmap);
void *_texture_cache_bitmap_get_hardware_format(struct bitmap_data const *bitmap, boolean block, boolean load);
void *texture_cache_steal_memory(long size);
void texture_cache_return_memory(void);
void texture_cache_flush(void);
D3DFORMAT bitmap_format_to_d3d_format(short format, word flags);
D3DFORMAT bitmap_format_to_d3d_linear_format(short format, word flags);
void texture_cache_debug_render(void);

/* ---------- globals */

extern struct texture_cache_debug_options_definition texture_cache_debug_options;
extern boolean debug_texture_cache;

/* ---------- public code */

#endif // __TEXTURE_CACHE_H
