/*
INTERFACE.H

header included in hcex build.
*/

#ifndef __INTERFACE_H
#define __INTERFACE_H
#pragma once

/* ---------- constants */

enum
{
	_interface_font_system = 0,
	_interface_font_terminal,
	_interface_color_table_screen,
	_interface_color_table_hud,
	_interface_color_table_editor,
	_interface_color_table_dialog,
	_interface_hud_globals,
	_interface_bitmap_motion_sweep,
	_interface_bitmap_motion_sweep_mask,
	_interface_bitmap_multiplayer_hud,
	_interface_string_list_localization,
	_interface_hud_digits,
	_interface_bitmap_motion_blip,
	_interface_bitmap_iface_map1,
	_interface_bitmap_iface_map2,
	_interface_bitmap_iface_map3,
	NUMBER_OF_INTERFACE_TAGS
};

/* ---------- macros */

/* ---------- structures */

struct game_globals_interface_tag_references
{
	struct tag_reference interface_tag_references[NUMBER_OF_INTERFACE_TAGS];
	long unused0[12];
};

/* ---------- prototypes/INTERFACE.C */

void interface_initialize(void);
void interface_initialize_for_new_map(void);
void interface_dispose_from_old_map(void);
void interface_dispose(void);
long interface_get_tag_index(short interface_tag_index);
argb_color *interface_get_rgb_color(short interface_color_table_index, short color_index, argb_color *color);
real_argb_color *interface_get_real_argb_color(short interface_color_table_index, short color_index, real_argb_color *color);
void interface_set_bitmap_text_draw_mode(short interface_font_index, short style, short justification, unsigned long flags, short color_table_index, short color_index);
void interface_draw_screen(void);
void profile_graph_toggle(char const *graph_name);
void interface_draw_fullscreen_overlays(void);
void interface_draw_bitmap(struct bitmap_data const *bitmap, point2d const *point, real_rectangle2d const *clip, real scale, real theta, real fade);
void interface_draw_bitmap_modulated(struct bitmap_data const *bitmap, point2d const *point, real_rectangle2d const *clip, real scale, real theta, real_argb_color const *modulated_color, short shader_type);
void interface_draw_bitmap_modulated_p32(struct bitmap_data const *bitmap, point2d const *point, real_rectangle2d const *clip, real scale, real theta, pixel32 modulated_color, short shader_type);

/* ---------- globals */

/* ---------- public code */

#endif // __INTERFACE_H
