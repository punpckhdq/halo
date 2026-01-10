/*
FONT_GROUP.H

header included in hcex build.
*/

#ifndef __FONT_GROUP_H
#define __FONT_GROUP_H
#pragma once

/* ---------- constants */

enum
{
	FONT_GROUP_TAG= 'font',
};

/* ---------- macros */

/* ---------- structures */

struct font_header
{
	unsigned long flags;
	short ascending_height;
	short descending_height;
	short leading_height;
	short leading_width;
	long pad[9];
	struct tag_block character_tables;
	struct tag_reference style_fonts[NUMBER_OF_TEXT_STYLES];
	struct tag_block characters;
	struct tag_data pixels;
};

/* ---------- prototypes/FONT_GROUP.C */

/* ---------- globals */

/* ---------- public code */

#endif // __FONT_GROUP_H
