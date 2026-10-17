/*
DRAW_STRING.H

header included in hcex build.
*/

#ifndef __DRAW_STRING_H
#define __DRAW_STRING_H
#pragma once

/* ---------- headers */

#include "draw_string_types.h"

/* ---------- constants */

enum
{
	_text_style_plain = NONE,
	_text_style_bold = 0,
	_text_style_italic,
	_text_style_condense,
	_text_style_underline,
	NUMBER_OF_TEXT_STYLES
};

enum
{
	MAXIMUM_NUMBER_OF_TAB_STOPS = 16,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/DRAW_STRING.C */

void draw_string_initialize(void);
void draw_string_initialize_for_new_map(void);
void draw_string_dispose_from_old_map(void);
void draw_string_dispose(void);

void draw_string_compute_bounds(rectangle2d const *bounds, char *string, rectangle2d *text_bounds, rectangle2d *cursor_bounds);
void draw_unicode_string_compute_bounds(rectangle2d const *bounds, wchar_t const *string, rectangle2d *text_bounds, rectangle2d *cursor_bounds);
short draw_string_pick(rectangle2d const *bounds, char *string, point2d const *point);

char *draw_string_get_string(short index);

void draw_string_set_tab_stops(short const *tab_stops, short count);
void draw_string_set_indents(short initial_indent, short paragraph_indent);
void draw_string_set_color(real_argb_color const *color);
void draw_string_get_color(real_argb_color *color);
void draw_string_set_font(long font_index);
void draw_string_set_format(short style, short justification, unsigned long flags);
void draw_string_set_draw_mode(long font_index, short style, short justification, unsigned long flags, real_argb_color const *color);
void draw_string_set_highlight(short start_index, short stop_index);

void bitmap_draw_string(struct bitmap_data *bitmap, rectangle2d const *bounds, rectangle2d const *clip, char *string);
void draw_string(draw_character_proc draw_character, rectangle2d const *bounds, point2d *cursor_reference, rectangle2d const *clip, short height_adjust, char *string);
void draw_unicode_string(draw_character_proc draw_character, rectangle2d const *bounds, point2d *cursor_reference, rectangle2d const *clip, short height_adjust, wchar_t const *string);

short parse_string(struct parse_string_state *state);

/* ---------- globals */

/* ---------- public code */

#endif // __DRAW_STRING_H
