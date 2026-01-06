/*
DRAW_STRING.H

header included in hcex build.
*/

#ifndef __DRAW_STRING_H
#define __DRAW_STRING_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/DRAW_STRING.C */

void draw_string_set_tab_stops(short const *tab_stops, short count);

void draw_string_set_draw_mode(long font_index, short style, short justification, unsigned long flags, union real_argb_color const *color);

/* ---------- globals */

/* ---------- public code */

#endif // __DRAW_STRING_H
