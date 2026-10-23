/*
COLOR_TABLE_GROUP.H

header included in hcex build.
*/

#ifndef __COLOR_TABLE_GROUP_H
#define __COLOR_TABLE_GROUP_H
#pragma once

/* ---------- constants */

enum
{
	COLOR_TABLE_TAG = 'colo' /* fake name */
};

/* ---------- macros */

#define color_table_definition_get(index) ((struct color_table *)tag_get(COLOR_TABLE_TAG, index)) /* fake name */

/* ---------- structures */

struct color_table_color
{
	char name[TAG_STRING_LENGTH+1];
	real_argb_color real_color;
};

struct color_table
{
	struct tag_block colors;	// color_table_color
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __COLOR_TABLE_GROUP_H
