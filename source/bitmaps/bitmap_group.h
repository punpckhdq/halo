/*
BITMAP_GROUP.H

header included in hcex build.
*/

#ifndef __BITMAP_GROUP_H
#define __BITMAP_GROUP_H
#pragma once

/* ---------- constants */

enum
{
	BITMAP_GROUP_TAG = 'bitm',
	BITMAP_GROUP_VERSION = 7
};

/* ---------- macros */

/* ---------- structures */

struct bitmap_group
{
	short type;
	short format;
	short usage;
	word flags;
	real detail_fade;
	real sharpen_amount;
	real bump_height;
	short sprite_budget_size;
	short sprite_budget_count;
	short import_width;
	short import_height;
	struct tag_data import_bitmap;
	struct tag_data pixel_data;
	real smoothing_filter_size;
	real alpha_bias;
	short mipmap_count;
	short sprite_usage;
	short sprite_spacing;
	word pad;
	struct tag_block sequences;		// bitmap_group_sequence
	struct tag_block bitmaps;		// bitmap_data
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __BITMAP_GROUP_H
