/*
DRAW_STRING.C

*/

/* ---------- headers */

#include "cseries.h"
#include "draw_string.h"
#include "text_group.h"
#include "font_group.h"
#include "international_strings.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmap_macros.h"

/* ---------- constants */

#define NUMBER_OF_TEXT_STRINGS 1

enum
{
	_string_index_language = 0,
	_string_index_country_code,
	_string_index_truncation_string,
	_string_index_illegal_truncation_characters,
	_string_index_can_end_words,
	_string_index_cannot_end_words,
	_string_index_cannot_begin_words,
	_string_index_first_bitmap_text_string,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void text_bounds_draw_character(struct parse_string_state *parse_state, struct font_header *font_header, struct font_character *character, pixel32 color, short x0, short y0, short x, short y, short dx, short dy);
static void text_pick_draw_character(struct parse_string_state *parse_state, struct font_header *font_header, struct font_character *character, pixel32 color, short x0, short y0, short x, short y, short dx, short dy);
static void bitmap_draw_character(struct parse_string_state *parse_state, struct font_header *font_header, struct font_character *character, pixel32 color, short x0, short y0, short x, short y, short dx, short dy);

static struct font_header *styled_font_get(long base_font_index, short style);
static void parse_string_new(struct parse_string_state *state, char *string, long base_font_index, short style, short justification, real_argb_color const *color);
static short parse_unicode_string(struct parse_string_state *state);
static void draw_string_partial(draw_character_proc draw_character, point2d *cursor, rectangle2d const *bounds, rectangle2d const *clip, pixel32 color, char *string, short string_index, short string_length);
static void draw_unicode_string_partial(draw_character_proc draw_character, point2d *cursor, rectangle2d const *bounds, rectangle2d const *clip, pixel32 color, wchar_t const *string, short string_index, short string_length);

/* ---------- globals */

static struct
{
	long string_list_index;
	long current_font_index;
	unsigned long current_flags;
	short current_style;
	short current_justification;
	real_argb_color current_color;
	short tab_stop_count;
	short tab_stops[MAXIMUM_NUMBER_OF_TAB_STOPS];
	short highlight_start_index;
	short highlight_stop_index;
	short initial_indent;
	short paragraph_indent;
	struct rasterizer_dynamic_screen_geometry_parameters multitexture_params;
} font_drawing_globals;

static struct
{
	rectangle2d bounds;
	struct font_header *last_font_header;
} text_bounds_globals;

static struct
{
	point2d pick_point;
	short best_pick_string_index;
	short best_pick_distance;
	short last_string_index;
} text_pick_globals;

static struct
{
	struct bitmap_data *bitmap;
	short encoding_shift;
} draw_character_software_globals;

/* ---------- public code */

void draw_string_initialize(
	void)
{
	return;
}

void draw_string_initialize_for_new_map(
	void)
{
	font_drawing_globals.string_list_index = interface_get_tag_index(_interface_string_list_localization);
	if (font_drawing_globals.string_list_index != NONE)
	{
		set_language_code((short)atoi(string_list_get_string(font_drawing_globals.string_list_index, _string_index_language)));
		font_drawing_globals.current_font_index = NONE;
		font_drawing_globals.tab_stop_count = 0;
		font_drawing_globals.current_flags = 0;
		font_drawing_globals.current_justification = _text_justification_left;
		font_drawing_globals.initial_indent = 0;
		font_drawing_globals.paragraph_indent = 0;
	}
	else
	{
		error(_error_immediate, "internal string localization tag is missing.");
	}

	return;
}

void draw_string_dispose_from_old_map(
	void)
{
	font_drawing_globals.string_list_index = NONE;

	return;
}

void draw_string_dispose(
	void)
{
	return;
}

void draw_string_compute_bounds(
	rectangle2d const *bounds,
	char *string,
	rectangle2d *text_bounds,
	rectangle2d *cursor_bounds)
{
	point2d cursor;

	text_bounds_globals.bounds.y0 = SHORT_MAX;
	text_bounds_globals.bounds.x0 = SHORT_MAX;
	text_bounds_globals.bounds.y1 = SHORT_MIN;
	text_bounds_globals.bounds.x1 = SHORT_MIN;
	text_bounds_globals.last_font_header = styled_font_get(font_drawing_globals.current_font_index, font_drawing_globals.current_style);
	draw_string(text_bounds_draw_character, bounds, &cursor, NULL, 0, string);

	cursor_bounds->x0 = cursor.x;
	cursor_bounds->x1 = cursor.x + 1;
	cursor_bounds->y0 = cursor.y - text_bounds_globals.last_font_header->ascending_height;
	cursor_bounds->y1 = cursor.y + text_bounds_globals.last_font_header->descending_height;

	text_bounds->x0 = text_bounds_globals.bounds.x0;
	text_bounds->y0 = bounds->y0;
	text_bounds->x1 = text_bounds_globals.bounds.x1;
	text_bounds->y1 = cursor_bounds->y1;

	return;
}

void draw_unicode_string_compute_bounds(
	rectangle2d const *bounds,
	wchar_t const *string,
	rectangle2d *text_bounds,
	rectangle2d *cursor_bounds)
{
	point2d cursor;

	text_bounds_globals.bounds.y0 = SHORT_MAX;
	text_bounds_globals.bounds.x0 = SHORT_MAX;
	text_bounds_globals.bounds.y1 = SHORT_MIN;
	text_bounds_globals.bounds.x1 = SHORT_MIN;
	text_bounds_globals.last_font_header = styled_font_get(font_drawing_globals.current_font_index, font_drawing_globals.current_style);
	draw_unicode_string(text_bounds_draw_character, bounds, &cursor, NULL, 0, string);

	cursor_bounds->x0 = cursor.x;
	cursor_bounds->x1 = cursor.x + 1;
	cursor_bounds->y0 = cursor.y - text_bounds_globals.last_font_header->ascending_height;
	cursor_bounds->y1 = cursor.y + text_bounds_globals.last_font_header->descending_height;

	text_bounds->x0 = text_bounds_globals.bounds.x0;
	text_bounds->y0 = bounds->y0;
	text_bounds->x1 = text_bounds_globals.bounds.x1;
	text_bounds->y1 = cursor_bounds->y1;

	return;
}

short draw_string_pick(
	rectangle2d const *bounds,
	char *string,
	point2d const *point)
{
	text_pick_globals.pick_point = *point;
	text_pick_globals.best_pick_distance = SHORT_MAX;
	text_pick_globals.best_pick_string_index = 0;
	text_pick_globals.last_string_index = 0;
	draw_string(text_pick_draw_character, bounds, NULL, NULL, 0, string);

	return text_pick_globals.best_pick_string_index;
}

char *draw_string_get_string(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 341, index>=0 && index<NUMBER_OF_TEXT_STRINGS);

	return string_list_get_string(font_drawing_globals.string_list_index, _string_index_first_bitmap_text_string + index);
}

void draw_string_set_tab_stops(
	short const *tab_stops,
	short count)
{
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 350, count>=0 && count<MAXIMUM_NUMBER_OF_TAB_STOPS);

	font_drawing_globals.tab_stop_count = MIN(count, MAXIMUM_NUMBER_OF_TAB_STOPS);
	if (font_drawing_globals.tab_stop_count > 0)
	{
		memcpy(font_drawing_globals.tab_stops, tab_stops, font_drawing_globals.tab_stop_count * sizeof(short));
	}

	return;
}

void draw_string_set_indents(
	short initial_indent,
	short paragraph_indent)
{
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 366, initial_indent>=0);
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 367, paragraph_indent>=0);

	font_drawing_globals.initial_indent = initial_indent;
	font_drawing_globals.paragraph_indent = paragraph_indent;

	return;
}

void draw_string_set_color(
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 378, color);
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 379, (color->alpha >= 0.f) && (color->alpha <= 1.f));
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 380, (color->red >= 0.f) && (color->red <= 1.f));
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 381, (color->green >= 0.f) && (color->green <= 1.f));
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 382, (color->blue >= 0.f) && (color->blue <= 1.f));

	font_drawing_globals.current_color = *color;

	return;
}

void draw_string_get_color(
	real_argb_color *color)
{
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 392, color);

	*color = font_drawing_globals.current_color;

	return;
}

void draw_string_set_font(
	long font_index)
{
	font_definition_get(font_index);
	font_drawing_globals.current_font_index = font_index;

	return;
}

void draw_string_set_format(
	short style,
	short justification,
	unsigned long flags)
{
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 410, VALID_FLAGS(flags, NUMBER_OF_TEXT_FLAGS));
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 411, style==_text_style_plain || (style>=0 && style<NUMBER_OF_TEXT_STYLES));
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 412, justification>=0 && justification<NUMBER_OF_TEXT_JUSTIFICATIONS);

	font_drawing_globals.current_style = style;
	font_drawing_globals.current_justification = justification;
	font_drawing_globals.current_flags = flags;

	return;
}

void draw_string_set_draw_mode(
	long font_index,
	short style,
	short justification,
	unsigned long flags,
	real_argb_color const *color)
{
	draw_string_set_font(font_index);
	draw_string_set_color(color);
	draw_string_set_format(style, justification, flags);

	return;
}

void draw_string_set_highlight(
	short start_index,
	short stop_index)
{
	font_drawing_globals.highlight_start_index = start_index;
	font_drawing_globals.highlight_stop_index = stop_index;

	return;
}

void bitmap_draw_string(
	struct bitmap_data *bitmap,
	rectangle2d const *bounds,
	rectangle2d const *clip,
	char *string)
{
	draw_character_software_globals.bitmap = bitmap;

	switch (bitmap->format)
	{
	case _bitmap_format_a8:
	case _bitmap_format_y8:
	case _bitmap_format_ay8:
	case _bitmap_format_r5g6b5:
	case _bitmap_format_a8r8g8b8:
		{
			rectangle2d bitmap_bounds;
			rectangle2d bitmap_clip;

			if (!bounds)
			{
				set_rectangle2d(&bitmap_bounds, MAX(0, bounds->x0), MAX(0, bounds->y0), MIN(bitmap->width, bounds->x1), MIN(bitmap->height, bounds->y1));
				bounds = &bitmap_bounds;
			}
			if (clip)
			{
				set_rectangle2d(&bitmap_clip, MAX(0, clip->x0), MAX(0, clip->y0), MIN(bitmap->width, clip->x1), MIN(bitmap->height, clip->y1));
				clip = &bitmap_clip;
			}

			draw_string(bitmap_draw_character, bounds, NULL, clip, 0, string);
		}
		break;
	}

	return;
}

void draw_string(
	draw_character_proc draw_character,
	rectangle2d const *bounds,
	point2d *cursor_reference,
	rectangle2d const *clip,
	short height_adjust,
	char *string)
{
	struct parse_string_state parse_state;
	point2d cursor;
	short tab_stop_index = 0;
	short line_index = 0;
	short column_line_index = 0;
	short maximum_column_line_index = 0;

	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 648, bounds);
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 649, string);

	parse_string_new(&parse_state, string, font_drawing_globals.current_font_index, font_drawing_globals.current_style, font_drawing_globals.current_justification, &font_drawing_globals.current_color);

	do
	{
		short end_index;
		short word_break_width;
		short width = 0;
		short word_break_index = 0;
		short previous_result = NONE;
		short justification = parse_state.justification;
		short start_index = parse_state.string_index;
		boolean done = FALSE;
		rectangle2d line_bounds = *bounds;

		if (font_drawing_globals.tab_stop_count > 0)
		{
			match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 681, tab_stop_index>=0 && tab_stop_index<=font_drawing_globals.tab_stop_count);

			if (tab_stop_index)
			{
				line_bounds.x0 = font_drawing_globals.tab_stops[tab_stop_index - 1];
			}
			else if (line_index)
			{
				line_bounds.x0 += font_drawing_globals.paragraph_indent;
			}
			else
			{
				line_bounds.x0 += font_drawing_globals.initial_indent;
			}

			if (tab_stop_index < font_drawing_globals.tab_stop_count)
			{
				line_bounds.x1 = font_drawing_globals.tab_stops[tab_stop_index];
			}
		}
		else
		{
			if (line_index)
			{
				line_bounds.x0 += font_drawing_globals.paragraph_indent;
			}
			else
			{
				line_bounds.x0 += font_drawing_globals.initial_indent;
			}
		}

		cursor.x = line_bounds.x0 + parse_state.font_header->leading_width;
		cursor.y = line_bounds.y0 + parse_state.font_header->ascending_height + (line_index + column_line_index) *
			(height_adjust + parse_state.font_header->ascending_height + parse_state.font_header->descending_height + parse_state.font_header->leading_height);

		while (!done)
		{
			boolean use_word_break = FALSE;

			parse_string(&parse_state);
			if (parse_state.result == _parsed_end_of_word || parse_state.result == _parsed_character)
			{
				struct font_character *character = font_get_character_by_ascii_code(parse_state.font_header, parse_state.character);

				if (character)
				{
					if (parse_state.result != _parsed_end_of_word && previous_result == _parsed_end_of_word)
					{
						word_break_index = end_index;
						word_break_width = width;
					}

					if (width + cursor.x + character->bitmap_width < line_bounds.x1)
					{
						width += character->character_width;
					}
					else if (TEST_FLAG(font_drawing_globals.current_flags, _draw_text_wrap_horizontally_bit))
					{
						if (word_break_index > 0)
						{
							end_index = word_break_index;
							width = word_break_width;
							use_word_break = TRUE;
						}
						done = TRUE;
					}
				}
			}
			else
			{
				done = TRUE;
			}

			if (!use_word_break)
			{
				end_index = parse_state.string_index;
			}
			previous_result = parse_state.result;
		}

		switch (justification)
		{
		case _text_justification_right:
			cursor.x = line_bounds.x0 + rectangle2d_width(&line_bounds) - width - parse_state.font_header->leading_width;
			break;
		case _text_justification_center:
			cursor.x = line_bounds.x0 + ((rectangle2d_width(&line_bounds) - width) >> 1);
			break;
		}

		if (TEST_FLAG(font_drawing_globals.current_flags, _draw_text_wrap_vertically_bit) || cursor.y < line_bounds.y1)
		{
			draw_string_partial(draw_character, &cursor, &line_bounds, clip, parse_state.color, string, start_index, end_index);
		}
		parse_state.string_index = end_index;

		switch (parse_state.result)
		{
		case _parsed_end_of_string:
		case _parsed_color_change:
			break;
		case _parsed_end_of_word:
		case _parsed_character:
			if (++column_line_index > maximum_column_line_index)
			{
				maximum_column_line_index = column_line_index;
			}
			break;
		case _parsed_end_of_column:
			if (tab_stop_index < font_drawing_globals.tab_stop_count)
			{
				tab_stop_index++;
				column_line_index = 0;
			}
			break;
		case _parsed_justification_change:
			column_line_index = 0;
			break;
		case _parsed_end_of_line:
			tab_stop_index = 0;
			column_line_index = 0;
			line_index += maximum_column_line_index + 1;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\text\\draw_string.c", 817, FALSE, NULL);
		}
	}
	while (parse_state.result != _parsed_end_of_string);

	font_drawing_globals.highlight_start_index = font_drawing_globals.highlight_stop_index = 0;

	if (cursor_reference)
	{
		*cursor_reference = cursor;
	}

	return;
}

void draw_unicode_string(
	draw_character_proc draw_character,
	rectangle2d const *bounds,
	point2d *cursor_reference,
	rectangle2d const *clip,
	short height_adjust,
	wchar_t const *string)
{
	struct parse_string_state parse_state;
	point2d cursor;
	short tab_stop_index = 0;
	short line_index = 0;
	short column_line_index = 0;
	short maximum_column_line_index = 0;

	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 848, bounds);
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 849, string);

	parse_string_new(&parse_state, (char *)string, font_drawing_globals.current_font_index, font_drawing_globals.current_style, font_drawing_globals.current_justification, &font_drawing_globals.current_color);

	do
	{
		short end_index;
		short word_break_width;
		short width = 0;
		short word_break_index = 0;
		short previous_result = NONE;
		short justification = parse_state.justification;
		short start_index = parse_state.string_index;
		boolean done = FALSE;
		rectangle2d line_bounds = *bounds;

		if (font_drawing_globals.tab_stop_count > 0)
		{
			match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 881, tab_stop_index>=0 && tab_stop_index<=font_drawing_globals.tab_stop_count);

			if (tab_stop_index)
			{
				line_bounds.x0 = font_drawing_globals.tab_stops[tab_stop_index - 1];
			}
			else if (line_index)
			{
				line_bounds.x0 += font_drawing_globals.paragraph_indent;
			}
			else
			{
				line_bounds.x0 += font_drawing_globals.initial_indent;
			}

			if (tab_stop_index < font_drawing_globals.tab_stop_count)
			{
				line_bounds.x1 = font_drawing_globals.tab_stops[tab_stop_index];
			}
		}
		else
		{
			if (line_index)
			{
				line_bounds.x0 += font_drawing_globals.paragraph_indent;
			}
			else
			{
				line_bounds.x0 += font_drawing_globals.initial_indent;
			}
		}

		cursor.x = line_bounds.x0 + parse_state.font_header->leading_width;
		cursor.y = line_bounds.y0 + parse_state.font_header->ascending_height + (line_index + column_line_index) *
			(height_adjust + parse_state.font_header->ascending_height + parse_state.font_header->descending_height + parse_state.font_header->leading_height);

		while (!done)
		{
			boolean use_word_break = FALSE;

			parse_unicode_string(&parse_state);
			if (parse_state.result == _parsed_end_of_word || parse_state.result == _parsed_character)
			{
				struct font_character *character = font_get_character_by_ascii_code(parse_state.font_header, parse_state.character);

				if (character)
				{
					if (parse_state.result != _parsed_end_of_word && previous_result == _parsed_end_of_word)
					{
						word_break_index = end_index;
						word_break_width = width;
					}

					if (width + cursor.x + character->bitmap_width < line_bounds.x1)
					{
						width += character->character_width;
					}
					else if (TEST_FLAG(font_drawing_globals.current_flags, _draw_text_wrap_horizontally_bit))
					{
						if (word_break_index > 0)
						{
							end_index = word_break_index;
							width = word_break_width;
							use_word_break = TRUE;
						}
						done = TRUE;
					}
				}
			}
			else
			{
				done = TRUE;
			}

			if (!use_word_break)
			{
				end_index = parse_state.string_index;
			}
			previous_result = parse_state.result;
		}

		switch (justification)
		{
		case _text_justification_right:
			cursor.x = line_bounds.x0 + rectangle2d_width(&line_bounds) - width - parse_state.font_header->leading_width;
			break;
		case _text_justification_center:
			cursor.x = line_bounds.x0 + ((rectangle2d_width(&line_bounds) - width) >> 1);
			break;
		}

		if (TEST_FLAG(font_drawing_globals.current_flags, _draw_text_wrap_vertically_bit) || cursor.y < line_bounds.y1)
		{
			draw_unicode_string_partial(draw_character, &cursor, &line_bounds, clip, parse_state.color, string, start_index, end_index);
		}
		parse_state.string_index = end_index;

		switch (parse_state.result)
		{
		case _parsed_end_of_string:
		case _parsed_color_change:
			break;
		case _parsed_end_of_word:
		case _parsed_character:
			if (++column_line_index > maximum_column_line_index)
			{
				maximum_column_line_index = column_line_index;
			}
			break;
		case _parsed_end_of_column:
			if (tab_stop_index < font_drawing_globals.tab_stop_count)
			{
				tab_stop_index++;
				column_line_index = 0;
			}
			break;
		case _parsed_justification_change:
			column_line_index = 0;
			break;
		case _parsed_end_of_line:
			tab_stop_index = 0;
			column_line_index = 0;
			line_index += maximum_column_line_index + 1;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\text\\draw_string.c", 1017, FALSE, NULL);
		}
	}
	while (parse_state.result != _parsed_end_of_string);

	font_drawing_globals.highlight_start_index = font_drawing_globals.highlight_stop_index = 0;

	if (cursor_reference)
	{
		*cursor_reference = cursor;
	}

	return;
}

/* ---------- private code */

static void text_bounds_draw_character(
	struct parse_string_state *parse_state,
	struct font_header *font_header,
	struct font_character *character,
	pixel32 color,
	short x0,
	short y0,
	short x,
	short y,
	short dx,
	short dy)
{
	short x1 = x0 + dx;
	short y1 = y0 + dy;

	if (x0 < text_bounds_globals.bounds.x0)
	{
		text_bounds_globals.bounds.x0 = x0;
	}
	if (y0 < text_bounds_globals.bounds.y0)
	{
		text_bounds_globals.bounds.y0 = y0;
	}
	if (x1 > text_bounds_globals.bounds.x1)
	{
		text_bounds_globals.bounds.x1 = x1;
	}
	if (y1 > text_bounds_globals.bounds.y1)
	{
		text_bounds_globals.bounds.y1 = y1;
	}
	text_bounds_globals.last_font_header = font_header;

	return;
}

static void text_pick_draw_character(
	struct parse_string_state *parse_state,
	struct font_header *font_header,
	struct font_character *character,
	pixel32 color,
	short x0,
	short y0,
	short x,
	short y,
	short dx,
	short dy)
{
	short distance;
	short x1 = x0 + dx;
	short x0_distance = x0 - text_pick_globals.pick_point.x;
	short x1_distance = x1 - text_pick_globals.pick_point.x;
	short y0_distance = y0 - text_pick_globals.pick_point.y;
	short y1_distance = y0 + dy - text_pick_globals.pick_point.y;

	x0_distance = ABS(x0_distance);
	x1_distance = ABS(x1_distance);
	y0_distance = ABS(y0_distance);
	y1_distance = ABS(y1_distance);
	distance = MAX(x0_distance, x1_distance);
	distance = MAX(distance, y0_distance);
	distance = MAX(distance, y1_distance);

	if (distance < text_pick_globals.best_pick_distance)
	{
		text_pick_globals.best_pick_distance = distance;
		text_pick_globals.best_pick_string_index = text_pick_globals.pick_point.x - x0 < (x1 - x0) >> 1 ?
			text_pick_globals.last_string_index : parse_state->string_index;
	}
	text_pick_globals.last_string_index = parse_state->string_index;

	return;
}

static void bitmap_draw_character(
	struct parse_string_state *parse_state,
	struct font_header *font_header,
	struct font_character *character,
	pixel32 color,
	short x0,
	short y0,
	short x,
	short y,
	short dx,
	short dy)
{
	word color16;
	short format = draw_character_software_globals.bitmap->format;
	short alpha = (short)PIXEL32_ALPHA(color);
	byte *character_pixels = (byte *)font_header->pixels.address + character->pixels_offset;

	if (format == _bitmap_format_r5g6b5)
	{
		color16 = PIXEL32_TO_PIXEL16_565(color);
	}

	while (dy-- > 0)
	{
		long bytes_per_row = draw_character_software_globals.bitmap->width * bitmap_format_get_bits_per_pixel(draw_character_software_globals.bitmap->format) / 8;
		byte *destination = (byte *)draw_character_software_globals.bitmap->base_address + y0 * bytes_per_row + (x0 << draw_character_software_globals.encoding_shift);
		byte *source = character_pixels + x + y * character->bitmap_width;
		short i;

		match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 495, y0>=0 && y0<=draw_character_software_globals.bitmap->height);
		match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 496, x0>=0 && x0+dx<=draw_character_software_globals.bitmap->width);

		switch (format)
		{
		case _bitmap_format_a8:
		case _bitmap_format_y8:
		case _bitmap_format_ay8:
			for (i = dx; i > 0; i--)
			{
				short intensity = *source;

				if (intensity)
				{
					byte destination_intensity = *destination;

					intensity = (intensity * alpha) >> 8;
					*destination = (byte)MIN(intensity, destination_intensity);
				}
				source++;
				destination++;
			}
			break;
		case _bitmap_format_r5g6b5:
			for (i = dx; i > 0; i--)
			{
				if (*source)
				{
					word pixel = *(word *)destination;
					short intensity = (*source * alpha) >> 8;
					short inverse_intensity = 255 - intensity;

					*(word *)destination = (((inverse_intensity * (pixel & 0x1f) + intensity * (color16 & 0x1f)) >> 8) & 0x1f) |
						(((inverse_intensity * (pixel & 0x7ff) + intensity * (color16 & 0x7ff)) >> 8) & 0x7e0) |
						(((inverse_intensity * pixel + intensity * color16) >> 8) & 0xf800);
				}
				source++;
				destination += sizeof(word);
			}
			break;
		case _bitmap_format_a8r8g8b8:
			for (i = dx; i > 0; i--)
			{
				short intensity = *source;

				if (intensity)
				{
					pixel32 pixel = *(pixel32 *)destination;
					short inverse_intensity;

					intensity = (intensity * alpha) >> 8;
					inverse_intensity = 255 - intensity;

					*(pixel32 *)destination = ((((pixel & 0xff) * inverse_intensity) >> 8) + (((color & 0xff) * intensity) >> 8)) |
						(((((pixel >> 8) & 0xff) * inverse_intensity) >> 8) + ((((color >> 8) & 0xff) * intensity) >> 8)) << 8 |
						(((((pixel >> 16) & 0xff) * inverse_intensity) >> 8) + ((((color >> 16) & 0xff) * intensity) >> 8)) << 16 |
						MAX((pixel32)intensity, pixel >> 24) << 24;
				}
				source++;
				destination += sizeof(pixel32);
			}
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\text\\draw_string.c", 565, FALSE, "### ERROR unsupported bitmap format");
		}

		y++;
		y0++;
	}

	return;
}

static struct font_header *styled_font_get(
	long base_font_index,
	short style)
{
	long font_index;

	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 1039, style==_text_style_plain || (style>=0 && style<NUMBER_OF_TEXT_STYLES));

	if (style == _text_style_plain)
	{
		font_index = base_font_index;
	}
	else
	{
		font_index = font_definition_get(base_font_index)->style_fonts[style].index;
	}

	if (font_index == NONE)
	{
		font_index = base_font_index;
	}

	return font_definition_get(font_index);
}

static void parse_string_new(
	struct parse_string_state *state,
	char *string,
	long base_font_index,
	short style,
	short justification,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 1054, style==_text_style_plain || (style>=0 && style<NUMBER_OF_TEXT_STYLES));
	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 1055, justification>=0 && justification<NUMBER_OF_TEXT_JUSTIFICATIONS);

	state->base_font_index = base_font_index;
	state->string = (byte *)string;
	state->string_index = 0;
	state->style = style;
	state->justification = justification;
	state->color = REAL_ARGB_COLOR_TO_PIXEL32(color);
	state->font_header = styled_font_get(state->base_font_index, state->style);

	return;
}

short parse_string(
	struct parse_string_state *state)
{
	word character;
	short result;

	do
	{
		character = get_next_character(state->string, &state->string_index);
		result = NONE;

		if ((character >> 8) == '|')
		{
			switch (tolower(character & UNSIGNED_CHAR_MAX))
			{
			case 'p':
				state->style = _text_style_plain;
				result = _parsed_style_change;
				break;
			case 'i':
				state->style = _text_style_italic;
				result = _parsed_style_change;
				break;
			case 'b':
				state->style = _text_style_bold;
				result = _parsed_style_change;
				break;
			case 'k':
				state->style = _text_style_condense;
				result = _parsed_style_change;
				break;
			case 'u':
				state->style = _text_style_underline;
				result = _parsed_style_change;
				break;
			case 'l':
				state->justification = _text_justification_left;
				result = _parsed_justification_change;
				break;
			case 'r':
				state->justification = _text_justification_right;
				result = _parsed_justification_change;
				break;
			case 'c':
				state->justification = _text_justification_center;
				result = _parsed_justification_change;
				break;
			case 'n':
				result = _parsed_end_of_line;
				break;
			case 't':
				result = _parsed_end_of_column;
				break;
			}
		}

		switch (result)
		{
		case NONE:
			switch (character)
			{
			case '\0':
				result = _parsed_end_of_string;
				break;
			case '\t':
				result = _parsed_end_of_column;
				break;
			case '\r':
				result = _parsed_end_of_line;
				break;
			default:
				{
					short next_index = state->string_index;
					long next_character = get_next_character(state->string, &next_index);
					char *can_end_words = string_list_get_string(font_drawing_globals.string_list_index, _string_index_can_end_words);
					char *cannot_end_words = string_list_get_string(font_drawing_globals.string_list_index, _string_index_cannot_end_words);
					char *cannot_begin_words = string_list_get_string(font_drawing_globals.string_list_index, _string_index_cannot_begin_words);

					if (!((character & 0xff00) == 0 && !character_in_pattern(character, can_end_words) ||
						(character & 0xff00) != 0 && character_in_pattern(character, cannot_end_words) ||
						character_in_pattern((word)next_character, cannot_begin_words)))
					{
						result = _parsed_end_of_word;
					}
					else
					{
						result = _parsed_character;
					}
				}
				break;
			}
			break;
		case _parsed_style_change:
			state->font_header = styled_font_get(state->base_font_index, state->style);
			break;
		}
	}
	while (result == _parsed_style_change || result == _parsed_color_change);

	match_assert("c:\\halo\\SOURCE\\text\\draw_string.c", 1203, result!=NONE);

	state->character = character;
	state->result = result;

	return result;
}

static short parse_unicode_string(
	struct parse_string_state *state)
{
	wchar_t character = ((wchar_t *)state->string)[state->string_index++];

	state->character = character;
	switch (character)
	{
	case L'|':
		if (((wchar_t *)state->string)[state->string_index++] == L'n')
		{
			state->character = L'\r';
			state->result = _parsed_end_of_line;
		}
		break;
	case L'\0':
		state->result = _parsed_end_of_string;
		break;
	case L'\t':
		state->result = _parsed_end_of_column;
		break;
	case L'\r':
		state->result = _parsed_end_of_line;
		break;
	default:
		state->result = _parsed_character;
		break;
	}

	return state->result;
}

static void draw_string_partial(
	draw_character_proc draw_character,
	point2d *cursor,
	rectangle2d const *bounds,
	rectangle2d const *clip,
	pixel32 color,
	char *string,
	short string_index,
	short string_length)
{
	short x0 = SHORT_MIN;
	short y0 = SHORT_MIN;
	short x1 = SHORT_MAX;
	short y1 = SHORT_MAX;

	if (bounds)
	{
		if (bounds->x0 > x0)
		{
			x0 = bounds->x0;
		}
		if (bounds->x1 < x1)
		{
			x1 = bounds->x1;
		}
		if (bounds->y0 > y0)
		{
			y0 = bounds->y0;
		}
		if (bounds->y1 < y1)
		{
			y1 = bounds->y1;
		}
	}

	if (clip)
	{
		if (clip->x0 > x0)
		{
			x0 = clip->x0;
		}
		if (clip->x1 < x1)
		{
			x1 = clip->x1;
		}
		if (clip->y0 > y0)
		{
			y0 = clip->y0;
		}
		if (clip->y1 < y1)
		{
			y1 = clip->y1;
		}
	}

	if (x0 < x1 && y0 < y1)
	{
		struct parse_string_state parse_state;

		parse_string_new(&parse_state, string, font_drawing_globals.current_font_index, font_drawing_globals.current_style, font_drawing_globals.current_justification, &font_drawing_globals.current_color);
		for (parse_state.string_index = string_index; parse_state.string_index < string_length; )
		{
			pixel32 character_color = parse_state.string_index < font_drawing_globals.highlight_start_index || parse_state.string_index >= font_drawing_globals.highlight_stop_index ?
				color : color ^ 0x00ffffff;
			struct font_character *character;

			parse_string(&parse_state);
			character = font_get_character_by_ascii_code(parse_state.font_header, parse_state.character);
			if (character)
			{
				short x = cursor->x - character->bitmap_origin_x;
				short y = cursor->y - character->bitmap_origin_y;
				short x_offset = 0;
				short dx = character->bitmap_width;
				short y_offset = 0;
				short dy = character->bitmap_height;

				cursor->x += character->character_width;

				if (x + dx > x1)
				{
					dx = x1 - x;
				}
				if (x < x0)
				{
					x_offset = x0 - x;
					x = x0;
					dx -= x_offset;
				}
				if (y + dy > y1)
				{
					dy = y1 - y;
				}
				if (y < y0)
				{
					y_offset = y0 - y;
					y = y0;
					dy -= y_offset;
				}

				if (dx > 0 && dy > 0)
				{
					draw_character(&parse_state, parse_state.font_header, character, character_color, x, y, x_offset, y_offset, dx, dy);
				}
			}
		}
	}

	return;
}

static void draw_unicode_string_partial(
	draw_character_proc draw_character,
	point2d *cursor,
	rectangle2d const *bounds,
	rectangle2d const *clip,
	pixel32 color,
	wchar_t const *string,
	short string_index,
	short string_length)
{
	short x0 = SHORT_MIN;
	short y0 = SHORT_MIN;
	short x1 = SHORT_MAX;
	short y1 = SHORT_MAX;

	if (bounds)
	{
		if (bounds->x0 > x0)
		{
			x0 = bounds->x0;
		}
		if (bounds->x1 < x1)
		{
			x1 = bounds->x1;
		}
		if (bounds->y0 > y0)
		{
			y0 = bounds->y0;
		}
		if (bounds->y1 < y1)
		{
			y1 = bounds->y1;
		}
	}

	if (clip)
	{
		if (clip->x0 > x0)
		{
			x0 = clip->x0;
		}
		if (clip->x1 < x1)
		{
			x1 = clip->x1;
		}
		if (clip->y0 > y0)
		{
			y0 = clip->y0;
		}
		if (clip->y1 < y1)
		{
			y1 = clip->y1;
		}
	}

	if (x0 < x1 && y0 < y1)
	{
		struct parse_string_state parse_state;

		parse_string_new(&parse_state, (char *)string, font_drawing_globals.current_font_index, font_drawing_globals.current_style, font_drawing_globals.current_justification, &font_drawing_globals.current_color);
		for (parse_state.string_index = string_index; parse_state.string_index < string_length; )
		{
			pixel32 character_color = parse_state.string_index < font_drawing_globals.highlight_start_index || parse_state.string_index >= font_drawing_globals.highlight_stop_index ?
				color : color ^ 0x00ffffff;
			struct font_character *character;

			parse_unicode_string(&parse_state);
			character = font_get_character_by_ascii_code(parse_state.font_header, parse_state.character);
			if (character)
			{
				short x = cursor->x - character->bitmap_origin_x;
				short y = cursor->y - character->bitmap_origin_y;
				short x_offset = 0;
				short dx = character->bitmap_width;
				short y_offset = 0;
				short dy = character->bitmap_height;

				cursor->x += character->character_width;

				if (x + dx > x1)
				{
					dx = x1 - x;
				}
				if (x < x0)
				{
					x_offset = x0 - x;
					x = x0;
					dx -= x_offset;
				}
				if (y + dy > y1)
				{
					dy = y1 - y;
				}
				if (y < y0)
				{
					y_offset = y0 - y;
					y = y0;
					dy -= y_offset;
				}

				if (dx > 0 && dy > 0)
				{
					draw_character(&parse_state, parse_state.font_header, character, character_color, x, y, x_offset, y_offset, dx, dy);
				}
			}
		}
	}

	return;
}
