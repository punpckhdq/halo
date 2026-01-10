/*
TERMINAL.C

symbols in this file:
000D2B20 0040:
	_terminal_initialize (0000)
000D2B60 0030:
	_terminal_dispose (0000)
000D2B90 0090:
	_code_000d2b90 (0000)
000D2C20 0030:
	_terminal_clear (0000)
000D2C50 0010:
	_terminal_gets_active (0000)
000D2C60 0080:
	_terminal_gets_begin (0000)
000D2CE0 0020:
	_terminal_gets_end (0000)
000D2D00 00c0:
	_code_000d2d00 (0000)
000D2DC0 0050:
	_code_000d2dc0 (0000)
000D2E10 02b0:
	_terminal_draw (0000)
000D30C0 00a0:
	_code_000d30c0 (0000)
000D3160 0030:
	_terminal_update (0000)
000D3190 0130:
	_terminal_printf (0000)
002712D0 0006:
	_rdata_002712d0 (0000)
002712D8 0010:
	??_C@_0BA@LJDMLJDP@terminal?5output?$AA@ (0000)
002712E8 0006:
	??_C@_05HDBKIDE@state?$AA@ (0000)
002712F0 0024:
	??_C@_0CE@JGPOFDIE@c?3?2halo?2SOURCE?2interface?2termina@ (0000)
00271314 0015:
	??_C@_0BF@NFLIKMOD@new_line_index?$CB?$DNNONE?$AA@ (0000)
00271330 0050:
	??_C@_0FA@BEBNLNOA@terminal_printf?5call?5generated?5?$CF@ (0000)
00301D99 0001:
	_terminal_render_enable (0000)
00454224 001c:
	_bss_00454224 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "terminal.h"

#include "cache_files.h"
#include "game.h"
#include "data.h"
#include "interface.h"
#include "console.h"
#include "telnet_console.h"
#include "rasterizer.h"
#include "render.h"
#include "draw_string.h"
#include "byte_swapping.h"
#include "tag_groups.h"
#include "text_group.h"
#include "font_group.h"

/* ---------- constants */

enum
{
	OUTPUT_LINGER_TIME= 120,
	OUTPUT_FADE_TIME= 30,
	OUTPUT_TOTAL_TIME= 150,
	OUTPUT_MAXIMUM_LINES= 32,
	INSERTION_POINT_FLASH_PERIOD= 500,
};

/* ---------- macros */

#define output_line_get(index) ((struct output_line_datum *)datum_get(terminal_globals.output_lines, (index)))

/* ---------- structures */

struct terminal_globals
{
	boolean initialized;
	struct data_array *output_lines;
	long newest_output_line_index;
	long oldest_output_line_index;
	struct terminal_gets_state *input_state;
	boolean insertion_point_visible;
	long last_insertion_point_toggle;
};

struct output_line_datum
{
	short identifier;
	long newer_line_index;
	long older_line_index;
	boolean tabstop;
	char buffer[256];
	real_argb_color color;
	long timer;
};

/* ---------- prototypes */

static void terminal_remove_line(long line_index);
static boolean terminal_update_input(void);
static void terminal_update_output(void);
static long terminal_new_line(void);

/* ---------- globals */

boolean terminal_render_enable= TRUE;

static const short terminal_tab_stops[]= {160, 320, 470};

static struct terminal_globals terminal_globals;

/* ---------- public code */

void terminal_initialize(
	void)
{
	terminal_globals.output_lines= data_new("terminal output", OUTPUT_MAXIMUM_LINES, sizeof(struct output_line_datum));
	terminal_globals.initialized= TRUE;
	data_make_valid(terminal_globals.output_lines);
	terminal_globals.input_state= NULL;
	terminal_globals.newest_output_line_index= NONE;
	terminal_globals.oldest_output_line_index= NONE;

	return;
}

void terminal_dispose(
	void)
{
	if (terminal_globals.output_lines)
	{
		if (terminal_globals.output_lines->valid)
		{
			data_make_invalid(terminal_globals.output_lines);
		}
		data_dispose(terminal_globals.output_lines);
	}
	terminal_globals.initialized= FALSE;

	return;
}

void terminal_clear(
	void)
{
	if (terminal_globals.initialized)
	{
		terminal_globals.newest_output_line_index= NONE;
		terminal_globals.oldest_output_line_index= NONE;
		data_delete_all(terminal_globals.output_lines);
	}

	return;
}

boolean terminal_gets_active(
	void)
{
	return terminal_globals.input_state != NULL;
}

boolean terminal_gets_begin(
	struct terminal_gets_state *state)
{
	boolean result= FALSE;
	match_assert("c:\\halo\\SOURCE\\interface\\terminal.c", 195, state);

	if (!terminal_globals.input_state)
	{
		terminal_globals.input_state= state;
		terminal_globals.input_state->edit.buffer= terminal_globals.input_state->result;

		terminal_globals.input_state->edit.maximum_length= TERMINAL_GETS_MAXIMUM_LINE_LENGTH;
		edit_text_new(&terminal_globals.input_state->edit);
		state->key_count= 0;
		result= TRUE;
	}

	return result;
}

void terminal_gets_end(
	struct terminal_gets_state *state)
{
	if (state==terminal_globals.input_state)
	{
		terminal_globals.input_state= NULL;
	}

	return;
}

void terminal_draw(
	void)
{
	char buffer[288];
	rectangle2d terminal_gets_bounds;
	rectangle2d terminal_render_bounds;
	rectangle2d line_bounds;
	real_argb_color color;

	long font_tag_index= interface_get_tag_index(_interface_font_terminal);

	if (terminal_globals.initialized)
	{
		struct font_header *font= (struct font_header *)tag_get(FONT_GROUP_TAG, font_tag_index);
		short line_height= font->ascending_height + font->descending_height + font->leading_height;
		if (terminal_globals.input_state)
		{
			short buff_len;

			buffer[0]= '\0';
			terminal_globals.input_state->prompt[TERMINAL_GETS_MAXIMUM_PROMPT_LENGTH]= '\0';
			strcat(buffer, terminal_globals.input_state->prompt);
			terminal_globals.input_state->result[TERMINAL_PRINTF_MAXIMUM_LINE_LENGTH]= '\0';
			buff_len= csstrlen(buffer);

			strcpy(&buffer[buff_len], terminal_globals.input_state->result);

			terminal_gets_bounds.x0= render.camera.window_bounds.x0;
			terminal_gets_bounds.x1= render.camera.window_bounds.x1;
			terminal_gets_bounds.y0= render.camera.window_bounds.y1 - line_height;
			terminal_gets_bounds.y1= render.camera.window_bounds.y1;

			offset_rectangle2d(&terminal_gets_bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
			draw_string_set_draw_mode(font_tag_index, NONE, 0, 0, &terminal_globals.input_state->color);

			if (terminal_globals.insertion_point_visible)
			{
				short cursor_index= (buff_len + terminal_globals.input_state->edit.insertion_point_index);
				if (buffer[cursor_index]=='\0')
				{
					buffer[cursor_index+1]= '\0';
				}
				buffer[cursor_index]= '_';
			}
			rasterizer_draw_string(&terminal_gets_bounds, NULL, NULL, 0, buffer);
		}

		if (terminal_render_enable)
		{
			struct output_line_datum *line;
			long line_index;
			short height= render.camera.window_bounds.y1 - line_height;

			for (line_index= terminal_globals.newest_output_line_index;
				line_index!=NONE && height - line_height > 0;
				line_index= line->older_line_index)
			{
				real fade_progress;

				line= output_line_get(line_index);
				fade_progress= 4.f - line->timer * 0.033333335f;

				color= line->color;
				color.alpha*= PIN(fade_progress, 0.f, 1.f);

				terminal_gets_bounds.x0= render.camera.window_bounds.x0;
				terminal_gets_bounds.x1= render.camera.window_bounds.x1;
				terminal_gets_bounds.y1= height;
				height -= line_height;
				terminal_gets_bounds.y0= height;

				offset_rectangle2d(
					&terminal_gets_bounds,
					-render.camera.viewport_bounds.x0,
					-render.camera.viewport_bounds.y0
				);

				if (line->tabstop)
				{
					draw_string_set_tab_stops(
						terminal_tab_stops,
						NUMBEROF(terminal_tab_stops)
					);
				}

				draw_string_set_draw_mode(font_tag_index, NONE, 0, 0, &color);
				rasterizer_draw_string(&terminal_gets_bounds, NULL, NULL, 0, line->buffer);
				draw_string_set_tab_stops(terminal_tab_stops, 0);
			}
		}
	}

	return;
}

boolean terminal_update(
	void)
{
	boolean result= FALSE;
	if (terminal_globals.initialized)
	{
		result= terminal_update_input();
		if (!console_is_active())
		{
			terminal_update_output();
		}
	}

	return result;
}

void terminal_printf(
	real_argb_color const *color,
	char const *format,
	...)
{
	real_argb_color default_terminal_printf_color;
	char *arglist;

	va_start(arglist, format);

	default_terminal_printf_color.alpha= 1.f;
	default_terminal_printf_color.red= 0.7f;
	default_terminal_printf_color.green= 0.7f;
	default_terminal_printf_color.blue= 0.7f;

	if (terminal_globals.initialized)
	{
		long line_index= terminal_new_line();
		match_assert("c:\\halo\\SOURCE\\interface\\terminal.c", 397, format);

		if (line_index!=NONE)
		{
			short count;
			struct output_line_datum *line= output_line_get(line_index);
			line->timer= 0;
			line->color= !color ? default_terminal_printf_color : *color;
			count= _vsnprintf(line->buffer, NUMBEROF(line->buffer)-2, format, arglist) - 1;
			match_vassert(
				"c:\\halo\\SOURCE\\interface\\terminal.c",
				413,
				count < (short)(NUMBEROF(line->buffer)-1),
				csprintf(
					temporary,
					"terminal_printf call generated %d characters; the buffer size is %d characters.",
					count,
					NUMBEROF(line->buffer)-1
				)
			);
			line->tabstop= strstr(line->buffer, "|t") != 0;
			telnet_console_print(line->buffer);
		}
	}

	va_end(arglist);

	return;
}

/* ---------- private code */

static void terminal_remove_line(
	long line_index)
{
	struct output_line_datum *output_line= output_line_get(line_index);

	if (output_line->older_line_index!=NONE)
	{
		output_line_get(output_line->older_line_index)->newer_line_index= output_line->newer_line_index;
	}
	else
	{
		terminal_globals.oldest_output_line_index= output_line->newer_line_index;
	}

	if (output_line->newer_line_index!=NONE)
	{
		output_line_get(output_line->newer_line_index)->older_line_index= output_line->older_line_index;
	}
	else
	{
		terminal_globals.newest_output_line_index= output_line->older_line_index;
	}

	datum_delete(terminal_globals.output_lines, line_index);

	return;
}

static boolean terminal_update_input(
	void)
{
	struct key_stroke key;

	boolean result= FALSE;

	if (terminal_globals.input_state)
	{
		long time= local_time_get();
		
		for (terminal_globals.input_state->key_count= 0;
			  input_get_key(&key);
			  terminal_globals.last_insertion_point_toggle= time)
		{
			if (terminal_globals.input_state->key_count < TERMINAL_GETS_MAXIMUM_KEYS)
			{
				terminal_globals.input_state->keys[terminal_globals.input_state->key_count]= key;
				terminal_globals.input_state->key_count++;
			}

			edit_text_handle_key(&terminal_globals.input_state->edit, &key);
			terminal_globals.insertion_point_visible= TRUE;
		}

		if (time > terminal_globals.last_insertion_point_toggle + OUTPUT_FADE_TIME)
		{
			terminal_globals.last_insertion_point_toggle= time;
			terminal_globals.insertion_point_visible= terminal_globals.insertion_point_visible==FALSE;
		}
		result= TRUE;
	}

	return result;
}


static void terminal_update_output(
	void)
{
	struct output_line_datum *line;
	long line_index= terminal_globals.newest_output_line_index;

	while (line_index!=NONE)
	{
		long older_line_index;
		
		line= output_line_get(line_index);
		older_line_index= line->older_line_index;
		line->timer++;

		if (line->timer>OUTPUT_TOTAL_TIME)
		{
			terminal_remove_line(line_index);
		}
		line_index= older_line_index;
	}

	return;
}

static long terminal_new_line(
	void)
{
	long new_line_index;
	struct output_line_datum *line;

	if (terminal_globals.output_lines->count==OUTPUT_MAXIMUM_LINES)
	{
		terminal_remove_line(terminal_globals.oldest_output_line_index);
	}

	new_line_index= datum_new(terminal_globals.output_lines);
	match_assert("c:\\halo\\SOURCE\\interface\\terminal.c", 122, new_line_index!=NONE);

	line= output_line_get(new_line_index);
	line->newer_line_index= NONE;
	line->older_line_index= terminal_globals.newest_output_line_index;
	terminal_globals.newest_output_line_index= new_line_index;

	if (line->older_line_index!=NONE)
	{
		output_line_get(line->older_line_index)->newer_line_index= new_line_index;
	}
	else
	{
		terminal_globals.oldest_output_line_index= new_line_index;
	}

	return new_line_index;
}
