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

/* ---------- prototypes/XBOX_TEXTURE_CACHE.C */

void *_texture_cache_bitmap_get_hardware_format(struct bitmap_data const *bitmap, boolean block, boolean load);
void texture_cache_bitmap_new(long tag_index, struct bitmap_data *bitmap);
void texture_cache_bitmap_delete(struct bitmap_data *bitmap);

/* ---------- globals */

/* ---------- public code */

#endif // __TEXTURE_CACHE_H
