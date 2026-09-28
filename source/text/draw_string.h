/*
DRAW_STRING.H

header included in hcex build.
*/

#ifndef __DRAW_STRING_H
#define __DRAW_STRING_H
#pragma once

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

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/DRAW_STRING.C */

void draw_string_set_tab_stops(short const *tab_stops, short count);
void draw_string_set_indents(short initial_indent, short paragraph_indent);
void draw_string_set_color(real_argb_color const *color);
void draw_string_get_color(real_argb_color *color);

void draw_string_set_draw_mode(long font_index, short style, short justification, unsigned long flags, union real_argb_color const *color);

/* ---------- globals */

/* ---------- public code */

#endif // __DRAW_STRING_H
