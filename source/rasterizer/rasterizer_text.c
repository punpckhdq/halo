/*
RASTERIZER_TEXT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "rasterizer_hardware_format_utilities.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "unicode.h"
#include "bitmaps_inlines.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "font_group.h"
#include "draw_string.h"
#include "draw_string_types.h"
#include "rasterizer/xbox/rasterizer_xbox.h"
#include "bitmaps.h"

/* ---------- constants */

enum
{
	MAXIMUM_HARDWARE_CHARACTERS = 256,
	MAXIMUM_HARDWARE_CHARACTER_MASK = MAXIMUM_HARDWARE_CHARACTERS-1,
	HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH = 128,
	HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT = 128
};

/* ---------- structures */

struct hardware_character
{
	struct font_character *character;
	short x0;
	short y0;
};


/* ---------- prototypes */

__inline static void lock_rasterizer_text_data(void);
__inline static void unlock_rasterizer_text_data(void);
static void rasterizer_draw_character(struct parse_string_state *parse_state, struct font_header *font_header, struct font_character *font_character, pixel32 color, short x0, short y0, short x, short y, short dx, short dy);
static void rasterizer_draw_character_with_dropshadow(struct parse_string_state *parse_state, struct font_header *font_header, struct font_character *font_character, pixel32 color, short x0, short y0, short x, short y, short dx, short dy);
static struct bitmap_data *hardware_character_cache_get_bitmap(void);
static void hardware_character_cache_get_origin(short hardware_character_index, short *x0, short *y0);
static void flush_hardware_character(struct hardware_character *hardware_character);
static void cache_hardware_format_character(struct font_header *font_header, struct font_character *font_character);

/* ---------- globals */

static struct
{
	boolean initialized;
	short read_index;
	short write_index;
	short x0;
	short y0;
	short maximum_character_height;
	struct bitmap_data *bitmap;
	struct hardware_character characters[MAXIMUM_HARDWARE_CHARACTERS];
} hardware_character_cache;
static pixel32 global_shadow_color = 0;
static word magic_number = 12;

/* ---------- public code */

__inline static void lock_rasterizer_text_data(
	void)
{
	return;
}

__inline static void unlock_rasterizer_text_data(
	void)
{
	return;
}

boolean rasterizer_text_cache_initialize(
	void)
{
	boolean success = TRUE;
	struct bitmap_data *bitmap;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 118, !hardware_character_cache.initialized);

	bitmap = bitmap_2d_new(HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH, HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT, 0, _bitmap_format_a4r4g4b4);
	if (bitmap)
	{
		memset(&hardware_character_cache, 0, sizeof(hardware_character_cache));
		if (rasterizer_bitmap_new(bitmap))
		{
			hardware_character_cache.bitmap = bitmap;
			hardware_character_cache.initialized = success;
		}
		else
		{
			error(_error_silent, "### ERROR failed to initialize hardware text cache");
			success = FALSE;
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to initialize hardware text cache");
		success = FALSE;
	}

	return success;
}

void rasterizer_text_set_shadow_color(
	pixel32 color)
{
	global_shadow_color = color;

	return;
}

void rasterizer_draw_string(
	rectangle2d const *bounds,
	rectangle2d const *clip,
	point2d *cursor_reference,
	short height_adjust,
	char const *string)
{
	static boolean warned = FALSE;
	boolean success = TRUE;
	boolean use_dropshadow = TRUE;
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	struct bitmap_data *bitmap;
	long string_length;
	long maximum_vertex_count; /* fake name */
	rectangle2d adjusted_bounds;
	rectangle2d adjusted_clip;
	draw_character_proc draw_character;

	lock_rasterizer_text_data();
	if (rasterizer_debug_options.draw_dynamic_screen_geometry && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
	{
		magic_number++;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 180, string);

		bitmap = hardware_character_cache_get_bitmap();
		if (bitmap && string[0])
		{
			string_length = strlen(string);
			if (use_dropshadow)
			{
				maximum_vertex_count = 4*string_length*2;
				draw_character = rasterizer_draw_character_with_dropshadow;
			}
			else
			{
				maximum_vertex_count = 4*string_length;
				draw_character = rasterizer_draw_character;
			}

			if (!bounds)
			{
				adjusted_bounds = render.camera.window_bounds;
				offset_rectangle2d(&adjusted_bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
			}
			else
			{
				adjusted_bounds = *bounds;
			}

			if (!clip)
			{
				adjusted_clip = render.camera.viewport_bounds;
				offset_rectangle2d(&adjusted_clip, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
			}
			else
			{
				set_rectangle2d(&adjusted_clip,
					FLOOR(clip->x0, 0),
					FLOOR(clip->y0, 0),
					MIN(render.camera.viewport_bounds.x1-render.camera.viewport_bounds.x0, clip->x1),
					MIN(render.camera.viewport_bounds.y1-render.camera.viewport_bounds.y0, clip->y1));
			}

			memset(&parameters, 0, sizeof(parameters));
			parameters.map_texture_scale[0].i = 1.f/bitmap->width;
			parameters.map_texture_scale[0].j = 1.f/bitmap->height;
			parameters.map_scale[0].i = parameters.map_scale[0].j = 1.f;
			parameters.meter_parameters = NULL;
			parameters.point_sampled = FALSE;
			parameters.framebuffer_blend_function = _shader_framebuffer_blend_function_alpha_blend;
			parameters.map[0] = bitmap;

			if (TRUE)
			{
				rasterizer_text_begin(&parameters);
				draw_string(draw_character, &adjusted_bounds, cursor_reference, &adjusted_clip, height_adjust, (char *)string);
				rasterizer_text_end();
			}
			else if (!warned)
			{
				error(_error_silent, "### ERROR failed to allocate dynamic vertices for screen geometry");
				success = FALSE;
				warned = TRUE;
			}
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_string failed");
	}
	unlock_rasterizer_text_data();

	return;
}

void rasterizer_draw_unicode_string(
	rectangle2d const *bounds,
	rectangle2d const *clip,
	point2d *cursor_reference,
	short height_adjust,
	wchar_t const *string)
{
	static boolean warned = FALSE;
	boolean success = TRUE;
	boolean use_dropshadow = TRUE;
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	struct bitmap_data *bitmap;
	long string_length;
	long maximum_vertex_count; /* fake name */
	rectangle2d adjusted_bounds;
	rectangle2d adjusted_clip;
	draw_character_proc draw_character;

	lock_rasterizer_text_data();
	if (rasterizer_debug_options.draw_dynamic_screen_geometry && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
	{
		magic_number++;
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 310, string);

		bitmap = hardware_character_cache_get_bitmap();
		if (bitmap && string[0])
		{
			string_length = ustrlen(string);
			if (use_dropshadow)
			{
				maximum_vertex_count = 4*string_length*2;
				draw_character = rasterizer_draw_character_with_dropshadow;
			}
			else
			{
				maximum_vertex_count = 4*string_length;
				draw_character = rasterizer_draw_character;
			}

			if (!bounds)
			{
				adjusted_bounds = render.camera.window_bounds;
				offset_rectangle2d(&adjusted_bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
			}
			else
			{
				adjusted_bounds = *bounds;
			}

			if (!clip)
			{
				adjusted_clip = render.camera.viewport_bounds;
				offset_rectangle2d(&adjusted_clip, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
			}
			else
			{
				set_rectangle2d(&adjusted_clip,
					FLOOR(clip->x0, 0),
					FLOOR(clip->y0, 0),
					MIN(render.camera.viewport_bounds.x1-render.camera.viewport_bounds.x0, clip->x1),
					MIN(render.camera.viewport_bounds.y1-render.camera.viewport_bounds.y0, clip->y1));
			}

			memset(&parameters, 0, sizeof(parameters));
			parameters.map_texture_scale[0].i = 1.f/bitmap->width;
			parameters.map_texture_scale[0].j = 1.f/bitmap->height;
			parameters.map_scale[0].i = parameters.map_scale[0].j = 1.f;
			parameters.meter_parameters = NULL;
			parameters.point_sampled = FALSE;
			parameters.framebuffer_blend_function = _shader_framebuffer_blend_function_alpha_blend;
			parameters.map[0] = bitmap;

			if (TRUE)
			{
				rasterizer_text_begin(&parameters);
				draw_unicode_string(draw_character, &adjusted_bounds, cursor_reference, &adjusted_clip, height_adjust, (wchar_t *)string);
				rasterizer_text_end();
			}
			else if (!warned)
			{
				error(_error_silent, "### ERROR failed to allocate dynamic vertices for screen geometry");
				success = FALSE;
				warned = TRUE;
			}
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_unicode_string failed");
	}
	unlock_rasterizer_text_data();

	return;
}

void rasterizer_text_cache_flush(
	void)
{
	if (hardware_character_cache.initialized)
	{
		short hardware_character_index;

		for (hardware_character_index = 0; hardware_character_index<MAXIMUM_HARDWARE_CHARACTERS; hardware_character_index++)
		{
			struct hardware_character *hardware_character = &hardware_character_cache.characters[hardware_character_index];

			if (hardware_character->character)
			{
				hardware_character->character->hardware_character_index = NONE;
			}
			hardware_character->character = NULL;
		}
	}

	return;
}

void rasterizer_text_cache_dispose(
	void)
{
	if (hardware_character_cache.initialized)
	{
		rasterizer_text_cache_flush();
		bitmap_delete(hardware_character_cache.bitmap);
		hardware_character_cache.initialized = FALSE;
	}

	return;
}

static void rasterizer_draw_character(
	struct parse_string_state *parse_state,
	struct font_header *font_header,
	struct font_character *font_character,
	pixel32 color,
	short x0,
	short y0,
	short x,
	short y,
	short dx,
	short dy)
{
	cache_hardware_format_character(font_header, font_character);
	if (font_character->hardware_character_index!=NONE)
	{
		short cache_bitmap_x0;
		short cache_bitmap_y0;
		struct dynamic_screen_vertex vertices[4];

		hardware_character_cache_get_origin(font_character->hardware_character_index, &cache_bitmap_x0, &cache_bitmap_y0);
		cache_bitmap_x0 += x;
		cache_bitmap_y0 += y;
		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = color;
		vertices[0].position.x = vertices[3].position.x = x0;
		vertices[1].position.x = vertices[2].position.x = x0+dx;
		vertices[0].position.y = vertices[1].position.y = y0;
		vertices[2].position.y = vertices[3].position.y = y0+dy;
		vertices[0].texcoord.x = vertices[3].texcoord.x = cache_bitmap_x0;
		vertices[1].texcoord.x = vertices[2].texcoord.x = cache_bitmap_x0+dx;
		vertices[0].texcoord.y = vertices[1].texcoord.y = cache_bitmap_y0;
		vertices[2].texcoord.y = vertices[3].texcoord.y = cache_bitmap_y0+dy;
		rasterizer_text_draw_character(vertices);
	}

	return;
}

static void rasterizer_draw_character_with_dropshadow(
	struct parse_string_state *parse_state,
	struct font_header *font_header,
	struct font_character *font_character,
	pixel32 color,
	short x0,
	short y0,
	short x,
	short y,
	short dx,
	short dy)
{
	cache_hardware_format_character(font_header, font_character);
	if (font_character->hardware_character_index!=NONE)
	{
		real shadow_offset_x = 1.f;
		real shadow_offset_y = 1.f;
		pixel32 shadow_color = global_shadow_color ? global_shadow_color : (color&0xff000000);
		boolean shadow = TRUE;

		while (TRUE)
		{
			short cache_bitmap_x0;
			short cache_bitmap_y0;
			struct dynamic_screen_vertex vertices[4];

			hardware_character_cache_get_origin(font_character->hardware_character_index, &cache_bitmap_x0, &cache_bitmap_y0);
			cache_bitmap_x0 += x;
			cache_bitmap_y0 += y;
			vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = shadow ? shadow_color : color;
			vertices[0].position.x = vertices[3].position.x = x0+shadow_offset_x;
			vertices[1].position.x = vertices[2].position.x = (x0+dx)+shadow_offset_x;
			vertices[0].position.y = vertices[1].position.y = y0+shadow_offset_y;
			vertices[2].position.y = vertices[3].position.y = (y0+dy)+shadow_offset_y;
			vertices[0].texcoord.x = vertices[3].texcoord.x = cache_bitmap_x0;
			vertices[1].texcoord.x = vertices[2].texcoord.x = cache_bitmap_x0+dx;
			vertices[0].texcoord.y = vertices[1].texcoord.y = cache_bitmap_y0;
			vertices[2].texcoord.y = vertices[3].texcoord.y = cache_bitmap_y0+dy;
			rasterizer_text_draw_character(vertices);

			if (shadow)
			{
				shadow = FALSE;
				shadow_offset_x = shadow_offset_y = 0.f;
			}
			else
			{
				break;
			}
		}
	}

	return;
}

static struct bitmap_data *hardware_character_cache_get_bitmap(
	void)
{
	return hardware_character_cache.initialized ? hardware_character_cache.bitmap : NULL;
}

static void hardware_character_cache_get_origin(
	short hardware_character_index,
	short *x0,
	short *y0)
{
	struct hardware_character *hardware_character = &hardware_character_cache.characters[hardware_character_index];

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 597, hardware_character_cache.initialized);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 598, hardware_character_index>=0 && hardware_character_index<MAXIMUM_HARDWARE_CHARACTERS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 599, x0 && y0);

	*x0 = hardware_character->x0;
	*y0 = hardware_character->y0;

	return;
}

static void flush_hardware_character(
	struct hardware_character *hardware_character)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 610, hardware_character);

	if (hardware_character->character)
	{
		hardware_character->character->hardware_character_index = NONE;
		if (hardware_character->character->pad==magic_number)
		{
			error(_error_log, "font cache overwrote character in use");
		}
		hardware_character->character = NULL;
	}

	return;
}

static void cache_hardware_format_character(
	struct font_header *font_header,
	struct font_character *font_character)
{
	boolean cached = FALSE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 633, hardware_character_cache.initialized);

	if (font_character->hardware_character_index!=NONE)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 637, font_character->hardware_character_index>=0 && font_character->hardware_character_index<MAXIMUM_HARDWARE_CHARACTERS);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 638, font_character==hardware_character_cache.characters[font_character->hardware_character_index].character);
		cached = TRUE;
	}

	if (!cached)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 645, font_character->bitmap_width<=HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_text.c", 646, font_character->bitmap_height<=HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT);

		font_character->pad = magic_number;

		if (hardware_character_cache.x0+font_character->bitmap_width>HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH)
		{
			hardware_character_cache.x0 = 0;
			hardware_character_cache.y0 += hardware_character_cache.maximum_character_height;
			hardware_character_cache.maximum_character_height = 0;
		}

		if (hardware_character_cache.y0+font_character->bitmap_height>HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT)
		{
			hardware_character_cache.y0 = 0;
			hardware_character_cache.x0 = hardware_character_cache.y0;
			hardware_character_cache.maximum_character_height = 0;

			for (; hardware_character_cache.read_index!=hardware_character_cache.write_index; hardware_character_cache.read_index = (hardware_character_cache.read_index+1)&MAXIMUM_HARDWARE_CHARACTER_MASK)
			{
				struct hardware_character *read_character = &hardware_character_cache.characters[hardware_character_cache.read_index];

				if (read_character->y0>0)
				{
					flush_hardware_character(read_character);
				}
				else
				{
					break;
				}
			}
		}

		if (font_character->bitmap_height>hardware_character_cache.maximum_character_height)
		{
			short y0 = hardware_character_cache.y0+hardware_character_cache.maximum_character_height;
			short y1 = hardware_character_cache.y0+font_character->bitmap_height;

			for (; hardware_character_cache.read_index!=hardware_character_cache.write_index; hardware_character_cache.read_index = (hardware_character_cache.read_index+1)&MAXIMUM_HARDWARE_CHARACTER_MASK)
			{
				struct hardware_character *read_character = &hardware_character_cache.characters[hardware_character_cache.read_index];

				if (read_character->y0>=y0 && read_character->y0<y1)
				{
					flush_hardware_character(read_character);
				}
				else
				{
					break;
				}
			}
			hardware_character_cache.maximum_character_height = font_character->bitmap_height;
		}

		{
			short next_write_index = (hardware_character_cache.write_index+1)&MAXIMUM_HARDWARE_CHARACTER_MASK;

			if (next_write_index==hardware_character_cache.read_index)
			{
				struct hardware_character *read_character = &hardware_character_cache.characters[hardware_character_cache.read_index];

				flush_hardware_character(read_character);
				hardware_character_cache.read_index = (hardware_character_cache.read_index+1)&MAXIMUM_HARDWARE_CHARACTER_MASK;
			}
		}

		{
			struct hardware_character *hardware_character;
			byte *pixels;
			short y;

			hardware_character = &hardware_character_cache.characters[hardware_character_cache.write_index];
			font_character->hardware_character_index = hardware_character_cache.write_index;
			hardware_character->character = font_character;
			hardware_character->x0 = hardware_character_cache.x0;
			hardware_character->y0 = hardware_character_cache.y0;

			pixels = (byte *)font_header->pixels.address+font_character->pixels_offset;
			for (y = 0; y<font_character->bitmap_height; y++)
			{
				word *destination = bitmap_2d_address(hardware_character_cache.bitmap, hardware_character->x0, (short)(hardware_character->y0+y), 0);
				short x;

				for (x = 0; x<font_character->bitmap_width; x++)
				{
					*destination++ = (*pixels++<<8) | 0x0fff;
				}
			}

			rasterizer_bitmap_changed(hardware_character_cache.bitmap);
			hardware_character_cache.x0 += font_character->bitmap_width;
			hardware_character_cache.write_index = (hardware_character_cache.write_index+1)&MAXIMUM_HARDWARE_CHARACTER_MASK;
		}
	}

	return;
}
