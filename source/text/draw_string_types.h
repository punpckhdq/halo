/*
DRAW_STRING_TYPES.H

header included in hcex build.
*/

#ifndef __DRAW_STRING_TYPES_H
#define __DRAW_STRING_TYPES_H
#pragma once

/* ---------- constants */

enum
{
	_text_justification_left = 0,
	_text_justification_right,
	_text_justification_center,
	NUMBER_OF_TEXT_JUSTIFICATIONS,
};

enum
{
	_draw_text_wrap_horizontally_bit = 0,
	_draw_text_wrap_vertically_bit,
	_draw_text_center_vertically_bit,
	_draw_text_bottom_justify_bit,
	NUMBER_OF_TEXT_FLAGS,
};

enum
{
	_parsed_end_of_string = 0,
	_parsed_end_of_line,
	_parsed_end_of_word,
	_parsed_end_of_column,
	_parsed_justification_change,
	_parsed_color_change,
	_parsed_character,
	_parsed_style_change,
	NUMBER_OF_PARSE_STRING_STATES,
};

/* ---------- macros */

/* ---------- structures */

struct parse_string_state
{
	long base_font_index;
	struct font_header *font_header;
	byte *string;
	short string_index;
	short style;
	short justification;
	word character;
	short result;
	pixel32 color;
};

typedef void (*draw_character_proc)(struct parse_string_state *parse_state, struct font_header *font_header, struct font_character *character, pixel32 color, short x0, short y0, short x, short y, short dx, short dy); /* fake name */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

#endif // __DRAW_STRING_TYPES_H
