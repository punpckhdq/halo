/*
TEXTURE_PAGE.H

header included in hcex build.
*/

#ifndef __TEXTURE_PAGE_H
#define __TEXTURE_PAGE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct texture_page_texture
{
	short identifier;
	boolean sorted;
	byte pad;
	short x;
	short y;
	short width;
	short height;
};

struct texture_page
{
	boolean contains_unsorted_textures;
	void *user_data;
	short width;
	short height;
	short spacing;
	long texture_pixel_count;
	long used_pixel_count;
	struct data_array *textures;
};

/* ---------- prototypes/TEXTURE_PAGE.C */

real texture_page_fraction_used(struct texture_page const *texture_page, boolean sorted_only);
struct texture_page *texture_page_new(void *user_data, short width, short height, short spacing);
void texture_page_delete(struct texture_page *texture_page);
void texture_page_textures_begin(struct texture_page *texture_page);
struct texture_page_texture *texture_page_texture_get(struct texture_page *texture_page, long texture_index);
long texture_page_texture_new(struct texture_page *texture_page, short width, short height, boolean immediate);
void texture_page_textures_cancel(struct texture_page *texture_page);
boolean texture_page_textures_end(struct texture_page *texture_page);
boolean texture_page_resize(struct texture_page *texture_page, short width, short height);

/* ---------- globals */

/* ---------- public code */

#endif // __TEXTURE_PAGE_H
