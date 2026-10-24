/*
TEXTURE_PAGE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "texture_page.h"

/* ---------- constants */

enum
{
	MAXIMUM_TEXTURE_CHANNELS = 512,
};

/* ---------- structures */

struct texture_channel /* fake name */
{
	short x;
	short y;
	short height;
};

/* ---------- prototypes */

static void texture_page_verify(struct texture_page *texture_page);
static boolean texture_page_resort(struct texture_page *texture_page);
static boolean qsort_texture_indexes(short index0, short index1);

/* ---------- globals */

static struct texture_page *qsort_texture_page; /* fake name */

/* ---------- public code */

struct texture_page *texture_page_new(
	void *user_data,
	short page_width,
	short page_height,
	short spacing)
{
	struct texture_page *texture_page = (struct texture_page *)match_malloc("c:\\halo\\SOURCE\\memory\\texture_page.c", 29, sizeof(*texture_page));

	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 31, page_width>0 && page_height>0);

	if (texture_page)
	{
		memset(texture_page, 0, sizeof(*texture_page));
		texture_page->width = page_width;
		texture_page->height = page_height;
		texture_page->spacing = spacing;
		texture_page->user_data = user_data;
		texture_page->texture_pixel_count = 0;
		texture_page->used_pixel_count = 0;
		texture_page->contains_unsorted_textures = FALSE;
		texture_page->textures = data_new("texture page textures", SHORT_MAX, sizeof(struct texture_page_texture));

		if (texture_page->textures)
		{
			data_make_valid(texture_page->textures);
			texture_page_verify(texture_page);
		}
		else
		{
			texture_page = NULL;
			match_free("c:\\halo\\SOURCE\\memory\\texture_page.c", 56, texture_page);
		}
	}

	return texture_page;
}

void texture_page_delete(
	struct texture_page *texture_page)
{
	texture_page_verify(texture_page);
	data_dispose(texture_page->textures);
	match_free("c:\\halo\\SOURCE\\memory\\texture_page.c", 69, texture_page);

	return;
}

void texture_page_textures_begin(
	struct texture_page *texture_page)
{
	texture_page_verify(texture_page);
	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 78, !texture_page->contains_unsorted_textures);
	texture_page->contains_unsorted_textures = TRUE;

	return;
}

long texture_page_texture_new(
	struct texture_page *texture_page,
	short width,
	short height,
	boolean immediate)
{
	long texture_index = NONE;

	texture_page_verify(texture_page);
	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 96, immediate || texture_page->contains_unsorted_textures);

	if (width<=texture_page->width && height<=texture_page->height && width*height + texture_page->texture_pixel_count<texture_page->width*texture_page->height)
	{
		texture_index = datum_new(texture_page->textures);

		if (texture_index!=NONE)
		{
			struct texture_page_texture *texture = texture_page_texture_get(texture_page, texture_index);

			texture->width = width;
			texture->height = height;
			texture->sorted = !texture_page->contains_unsorted_textures;
			texture_page->texture_pixel_count += width*height;

			if (immediate && !texture_page_resort(texture_page))
			{
				texture_page->texture_pixel_count -= width*height;
				datum_delete(texture_page->textures, texture_index);
				texture_index = NONE;
			}
		}
	}

	return texture_index;
}

void texture_page_texture_delete(
	struct texture_page *texture_page,
	long texture_index)
{
	texture_page_verify(texture_page);
	datum_delete(texture_page->textures, texture_index);
	texture_page_resort(texture_page);

	return;
}

void texture_page_textures_cancel(
	struct texture_page *texture_page)
{
	struct texture_page_texture *texture;
	short texture_index;
	boolean resort_succeeded;

	texture_page_verify(texture_page);
	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 138, texture_page->contains_unsorted_textures);

	for (texture_index = 0, texture = (struct texture_page_texture *)texture_page->textures->data; texture_index<texture_page->textures->count; texture_index++, texture++)
	{
		if (texture->identifier && !texture->sorted)
		{
			texture_page->texture_pixel_count -= texture->width*texture->height;
			datum_delete(texture_page->textures, texture_index);
		}
	}

	texture_page->contains_unsorted_textures = FALSE;
	resort_succeeded = texture_page_resort(texture_page);
	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 157, resort_succeeded);

	return;
}

boolean texture_page_textures_end(
	struct texture_page *texture_page)
{
	boolean success;
	struct texture_page_texture *texture;
	short texture_index;

	texture_page_verify(texture_page);
	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 170, texture_page->contains_unsorted_textures);

	success = texture_page_resort(texture_page);

	if (success)
	{
		for (texture_index = 0, texture = (struct texture_page_texture *)texture_page->textures->data; texture_index<texture_page->textures->count; texture_index++, texture++)
		{
			if (texture->identifier)
			{
				texture->sorted = TRUE;
			}
		}

		texture_page->contains_unsorted_textures = FALSE;
	}

	return success;
}

struct texture_page_texture *texture_page_texture_get(
	struct texture_page *texture_page,
	long texture_index)
{
	texture_page_verify(texture_page);

	return (struct texture_page_texture *)datum_get(texture_page->textures, texture_index);
}

boolean texture_page_resize(
	struct texture_page *texture_page,
	short width,
	short height)
{
	short old_width;
	short old_height;
	boolean success;

	texture_page_verify(texture_page);

	old_width = texture_page->width;
	old_height = texture_page->height;
	texture_page->width = width;
	texture_page->height = height;

	if (texture_page_resort(texture_page))
	{
		success = TRUE;
	}
	else
	{
		texture_page->width = old_width;
		texture_page->height = old_height;
		texture_page_resort(texture_page);
		success = FALSE;
	}

	return success;
}

real texture_page_fraction_used(
	struct texture_page const *texture_page,
	boolean sorted_only)
{
	return (real)(sorted_only ? texture_page->used_pixel_count : texture_page->texture_pixel_count) / (texture_page->width*texture_page->height);
}

/* ---------- private code */

static void texture_page_verify(
	struct texture_page *texture_page)
{
	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 260, texture_page);
	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 261, texture_page->width>0 && texture_page->height>0);
	data_verify(texture_page->textures);

	return;
}

static boolean texture_page_resort(
	struct texture_page *texture_page)
{
	short sorted_texture_indexes[SHORT_MAX];
	short sorted_texture_count;
	boolean success = TRUE;
	short texture_index;
	struct texture_page_texture *texture;
	struct texture_channel channels[MAXIMUM_TEXTURE_CHANNELS];
	short channel_count;
	short spacing;
	short spacing_mask;

	match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 292, texture_page->spacing==0 || ceiling_power2(texture_page->spacing)==texture_page->spacing);
	texture_page_verify(texture_page);

	sorted_texture_count = 0;

	for (texture_index = 0, texture = (struct texture_page_texture *)texture_page->textures->data; texture_index<texture_page->textures->count; texture_index++, texture++)
	{
		if (texture->identifier)
		{
			sorted_texture_indexes[sorted_texture_count++] = texture_index;
		}
	}

	qsort_texture_page = texture_page;
	qsort_2byte(sorted_texture_indexes, sorted_texture_count, qsort_texture_indexes);

	spacing = sorted_texture_count>1 ? texture_page->spacing : 0;
	spacing_mask = spacing ? ceiling_power2(texture_page->spacing) - 1 : 0;

	texture_page->used_pixel_count = 0;
	channel_count = 1;
	channels[0].x = 0;
	channels[0].y = 0;
	channels[0].height = texture_page->height;

	for (texture_index = 0; texture_index<sorted_texture_count; texture_index++)
	{
		struct texture_page_texture *texture = texture_page_texture_get(texture_page, sorted_texture_indexes[texture_index]);
		short width = texture->width + 2*spacing;
		short height = texture->height + 2*spacing;
		short channel_index;

		if (width & spacing_mask)
		{
			width = (width & ~spacing_mask) + spacing;
		}

		if (height & spacing_mask)
		{
			height = (height & ~spacing_mask) + spacing;
		}

		for (channel_index = 0; channel_index<channel_count; channel_index++)
		{
			struct texture_channel *channel = &channels[channel_index];

			if (width<=texture_page->width - channel->x && height<=channel->height)
			{
				break;
			}
		}

		if (channel_index<channel_count)
		{
			struct texture_channel *channel = &channels[channel_index];
			short remaining_height = channel->height - height;

			texture->x = channel->x + spacing;
			texture->y = channel->y + spacing;

			if (remaining_height)
			{
				struct texture_channel *new_channel = &channels[channel_count++];

				match_assert("c:\\halo\\SOURCE\\memory\\texture_page.c", 360, channel_count<MAXIMUM_TEXTURE_CHANNELS);
				new_channel->x = channel->x;
				new_channel->y = channel->y + height;
				new_channel->height = remaining_height;
			}

			channel->height = height;
			channel->x += width;
			texture_page->used_pixel_count += width*height;
		}
		else
		{
			success = FALSE;
			break;
		}
	}

	return success;
}

static boolean qsort_texture_indexes(
	short index0,
	short index1)
{
	return texture_page_texture_get(qsort_texture_page, index1)->height - texture_page_texture_get(qsort_texture_page, index0)->height>0;
}
