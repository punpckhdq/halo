/*
DECALS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "decals.h"
#include "objects.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "collisions.h"
#include "collision_usage.h"
#include "collision_bsp_definitions.h"
#include "bsp3d.h"
#include "scenario.h"
#include "render_debug.h"
#include "texture_cache.h"

/* ---------- constants */

#define MAXIMUM_POINTS_PER_DECAL_SURFACE 12						/* fake name */
#define MAXIMUM_DECAL_LIST_ITERATIONS MAXIMUM_DECALS_PER_MAP	/* fake name */
#define MAXIMUM_DECAL_UNLOCK_PASSES 100							/* fake name */
#define DECAL_VERTICES_PER_QUAD 4								/* fake name */
#define DECAL_TEXCOORD_SCALE 32768								/* fake name */
#define DECAL_TEXCOORD_MAXIMUM (DECAL_TEXCOORD_SCALE - 1)		/* fake name */
#define DECAL_LOCAL_RANDOM_SEED_SALT 0xDEADC0DE					/* fake name */
#define DECAL_DEBUG_POINT_SIZE 0.0625f							/* fake name */
#define DECAL_WRAP_BOUNDS_TOLERANCE 0.5f						/* fake name */

/* ---------- macros */

/* ---------- structures */

struct decal_editor_geometry
{
	real_point3d positions[DECAL_VERTICES_PER_QUAD];
	real_point2d texcoords[DECAL_VERTICES_PER_QUAD];
	struct bitmap_group_sequence const *sequence;
	struct bitmap_group_sprite const *sprite;
	short bitmap_index;
};

struct decal_globals
{
	long first_decal_indices[NUMBER_OF_DECAL_LAYERS][MAXIMUM_CLUSTERS_PER_STRUCTURE];
	long first_disconnected_decal_index;
	long locked_count;
	long permanent_count;
};

struct decal_vertex_internal
{
	real_point3d position;
	real_point2d texcoord;
	boolean clipped;
};

struct decal_geometry
{
	struct decal_vertex_internal decal_vertices[MAXIMUM_TOTAL_VERTICES_PER_DECAL];
	short decal_vertex_count;
	short decal_surface_vertex_counts[MAXIMUM_DECAL_SURFACE_QUEUE_SIZE];
	short decal_surface_count;
	long decal_surface_indices[MAXIMUM_DECAL_SURFACE_QUEUE_SIZE];
};

struct decal_projection
{
	real_matrix4x3 basis;
	real_rectangle2d extent;
	real_plane3d plane;
	short axis;
	boolean sign;
	real_point2d decal_points2d[DECAL_VERTICES_PER_QUAD];
	real_vector2d texture_u_axis;
	real_vector2d texture_v_axis;
	real texture_scale;
};

/* ---------- prototypes */

static void decal_check(long decal_index, boolean layer_check);
static void decal_set_first_decal_index(short cluster_index, short layer, long decal_index);
static real_rectangle2d *decal_sprite_get_bounds(struct decal_definition const *definition, short sequence_index, short sprite_index, real scale, real_rectangle2d *sprite_bounds, real_rectangle2d *extent);
static void decal_update(long decal_index);
static void decal_reinsert(long decal_index, short cluster_index, short layer);
static long decal_insert(long decal_index, short cluster_index, short layer, long next_decal_index, boolean permanent);
static void decal_projection_create(real_matrix4x3 const *basis, real_rectangle2d const *extent, struct decal_projection *projection);
static void decal_clip_to_surface(struct decal_projection const *projection, struct decal_geometry *geometry, long surface_index, boolean wrap, real radius, short type, long *surface_queue, short *surface_queue_write_index, long *deviant_surface_list, short *deviant_surface_count);

/* ---------- globals */

struct data_array *global_decal_data;
boolean debug_decals;
boolean decals_enabled = TRUE;

struct decal_wrap_parameters const decal_wrap_parameters[NUMBER_OF_DECAL_TYPES] =
{
	{ 40.f, 110.f, 1.5f, TRUE },
	{ 40.f, 110.f, 1.5f, TRUE },
	{ 40.f, 110.f, 1.5f, TRUE },
	{ 10.f, 10.f, 1.5f, FALSE }
};

static struct decal_geometry decal_geometry;
static struct decal_globals *decal_globals;
static real_point2d decal_points2d_temp[2][MAXIMUM_POINTS_PER_DECAL_SURFACE];

/* ---------- private code */

static void decal_check(
	long decal_index,
	boolean layer_check)
{
	struct decal_datum const *decal = decal_get(decal_index);

	if (decal->prev_decal_index != NONE)
	{
		short cluster_index = decal_get(decal->prev_decal_index)->cluster_index;
		short layer = decal_get(decal->prev_decal_index)->layer;

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 194, cluster_index==decal->cluster_index);
		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 195, !layer_check || layer==decal->layer);
	}

	if (decal->next_decal_index != NONE)
	{
		short cluster_index = decal_get(decal->next_decal_index)->cluster_index;
		short layer = decal_get(decal->next_decal_index)->layer;

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 203, cluster_index==decal->cluster_index);
		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 204, !layer_check || layer==decal->layer);
	}

	return;
}

static void decal_set_first_decal_index(
	short cluster_index,
	short layer,
	long decal_index)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 216, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 217, layer>=0 && layer<NUMBER_OF_DECAL_LAYERS);

	decal_globals->first_decal_indices[layer][cluster_index] = decal_index;

	return;
}

static real_rectangle2d *decal_sprite_get_bounds(
	struct decal_definition const *definition,
	short sequence_index,
	short sprite_index,
	real scale,
	real_rectangle2d *sprite_bounds,
	real_rectangle2d *extent)
{
	real extent_scale;
	real width_scale;
	real height_scale;
	struct bitmap_group const *bitmap_group;
	struct bitmap_group_sequence const *sequence;
	struct bitmap_group_sprite const *sprite;
	struct bitmap_data const *bitmap;
	real aspect_ratio = 1.f;

	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 263, definition);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 264, sprite_bounds);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 265, extent);

	bitmap_group = bitmap_group_get(definition->shader.decal.map.index);
	sequence = TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, sequence_index, struct bitmap_group_sequence);
	sprite = TAG_BLOCK_GET_ELEMENT(&sequence->sprites, sprite_index, struct bitmap_group_sprite);
	bitmap = TAG_BLOCK_GET_ELEMENT(&bitmap_group->bitmaps, sprite->bitmap_index, struct bitmap_data);
	*sprite_bounds = sprite->bounds;

	if (TEST_FLAG(definition->flags, _decal_definition_preserve_aspect_bit))
	{
		aspect_ratio = ((real)bitmap->height / (real)bitmap->width) * ((sprite->bounds.x1 - sprite->bounds.x0) / (sprite->bounds.y1 - sprite->bounds.y0));
	}

	extent_scale = scale / definition->runtime_maximum_sprite_extent;
	width_scale = (real)bitmap->width * extent_scale;
	height_scale = (real)bitmap->height * extent_scale * aspect_ratio;
	extent->x0 = (0.f - sprite->registration_point.x) * width_scale;
	extent->x1 = (sprite->bounds.x1 - sprite->registration_point.x - sprite->bounds.x0) * width_scale;
	extent->y0 = (0.f - sprite->registration_point.y) * height_scale;
	extent->y1 = (sprite->bounds.y1 - sprite->registration_point.y - sprite->bounds.y0) * height_scale;

	return extent;
}

static void decal_update(
	long decal_index)
{
	struct decal_definition const *definition;
	static boolean locked_count_warning_issued = FALSE;
	struct decal_datum *decal = decal_get(decal_index);
	real age = (real)(game_time_get() - decal->creation_time) * SECONDS_PER_TICK;

	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 307, decal->definition_index!=NONE);
	definition = decal_definition_get(decal->definition_index);
	decal->intensity = UNSIGNED_CHAR_MAX;

	if (!TEST_FLAG(decal->flags, _decal_permanent_bit))
	{
		if (decal->lifetime == 0.f || age < decal->lifetime)
		{
			if (decal->lifetime > 0.f && decal->decay_time > 0.f && decal->lifetime - age < decal->decay_time)
			{
				real f = (decal->lifetime - age) / decal->decay_time;

				match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 322, f>=0.0f && f<=1.0f);
				decal->intensity = (byte)fast_ftol(f * UNSIGNED_CHAR_MAX);
			}
		}
		else
		{
			if (TEST_FLAG(decal->flags, _decal_locked_bit))
			{
				SET_FLAG(decal->flags, _decal_locked_bit, FALSE);

				if (--decal_globals->locked_count < 0 && !locked_count_warning_issued)
				{
					error(
						_error_silent,
						"### ERROR decals: locked count is invalid (#%d) -- tell Bernie!!",
						decal_globals->locked_count);
					locked_count_warning_issued = TRUE;
				}
			}

			rasterizer_decal_vertices_delete(decal_index);
		}
	}

	return;
}

static void decal_reinsert(
	long decal_index,
	short cluster_index,
	short layer)
{
	long first_decal_index = decal_get_first_decal_index(cluster_index, layer);
	struct decal_datum *decal = decal_get(decal_index);

	decal->prev_decal_index = NONE;
	decal->next_decal_index = first_decal_index;
	decal->cluster_index = cluster_index;
	decal->layer = layer;

	if (first_decal_index != NONE)
	{
		decal_get(first_decal_index)->prev_decal_index = decal_index;
	}

	decal_set_first_decal_index(cluster_index, layer, decal_index);

	return;
}

static long decal_insert(
	long decal_index,
	short cluster_index,
	short layer,
	long next_decal_index,
	boolean permanent)
{
	static boolean locked_count_warning_issued = FALSE;
	long new_decal_index = datum_new_at_index(global_decal_data, decal_index);

	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 479, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 480, layer>=0 && layer<NUMBER_OF_DECAL_LAYERS);

	if (new_decal_index != NONE)
	{
		struct decal_datum *decal = decal_get(new_decal_index);

		if (permanent)
		{
			decal->flags = FLAG(_decal_permanent_bit);
			decal_globals->permanent_count++;
		}
		else if (local_random() * 100 < DECAL_LOCK_PROBABILITY * UNSIGNED_SHORT_MAX)
		{
			decal->flags = FLAG(_decal_locked_bit);

			if (++decal_globals->locked_count > DECAL_LOCK_UPPER_LIMIT)
			{
				struct data_iterator iterator;
				short unlock_passes = 0;

				data_iterator_new(&iterator, global_decal_data);

				while (decal_globals->locked_count > DECAL_LOCK_LOWER_LIMIT)
				{
					struct decal_datum *unlock_decal = data_iterator_next(&iterator);

					if (unlock_decal)
					{
						if (TEST_FLAG(unlock_decal->flags, _decal_locked_bit) &&
							(local_random() * 100 < DECAL_UNLOCK_PROBABILITY * UNSIGNED_SHORT_MAX || unlock_decal->cluster_index == NONE))
						{
							SET_FLAG(unlock_decal->flags, _decal_locked_bit, FALSE);
							decal_globals->locked_count--;
						}
					}
					else
					{
						data_iterator_new(&iterator, global_decal_data);

						if (++unlock_passes >= MAXIMUM_DECAL_UNLOCK_PASSES)
						{
							error(
								_error_silent,
								"### ERROR decals: failed to unlock decals during insert -- tell Bernie!!");
							return NONE;
						}
					}
				}

				if (decal_globals->locked_count < 0 && !locked_count_warning_issued)
				{
					error(
						_error_silent,
						"### ERROR decals: locked count is invalid (#%d) -- tell Bernie!!",
						decal_globals->locked_count);
					locked_count_warning_issued = TRUE;
				}
			}
		}
		else
		{
			decal->flags = 0;
		}

		if (next_decal_index != NONE)
		{
			struct decal_datum *next = decal_get(next_decal_index);

			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 556, next->cluster_index==cluster_index);

			if (next->prev_decal_index != NONE)
			{
				decal_get(next->prev_decal_index)->next_decal_index = new_decal_index;
			}
			else
			{
				decal_set_first_decal_index(cluster_index, layer, new_decal_index);
			}

			next->prev_decal_index = new_decal_index;
			decal->prev_decal_index = next->prev_decal_index;
			decal->next_decal_index = next_decal_index;
			decal->cluster_index = cluster_index;
			decal->layer = layer;
		}
		else
		{
			decal_reinsert(new_decal_index, cluster_index, layer);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to insert decal");
	}

	return new_decal_index;
}

/* ---------- public code */

void decals_initialize(
	void)
{
	global_decal_data = game_state_data_new("decals", MAXIMUM_DECALS_PER_MAP, sizeof(struct decal_datum));
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 593, global_decal_data);
	global_decal_data->identifier_zero_invalid = TRUE;
	decal_globals = game_state_malloc("decal globals", NULL, sizeof(struct decal_globals));
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 596, decal_globals);
	rasterizer_decals_initialize();
	decal_geometry.decal_vertex_count = 0;
	decal_geometry.decal_surface_count = 0;

	return;
}

void decals_initialize_for_new_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 610, global_decal_data);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 611, decal_globals);

	memset(decal_globals, NONE, sizeof(decal_globals->first_decal_indices));
	decal_globals->first_disconnected_decal_index = NONE;
	decal_globals->locked_count = 0;
	decal_globals->permanent_count = 0;
	data_make_valid(global_decal_data);
	rasterizer_decals_initialize_for_new_map();
	decal_geometry.decal_vertex_count = 0;
	decal_geometry.decal_surface_count = 0;

	return;
}

void decals_reconnect_to_structure_bsp(
	void)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 633, global_decal_data);

	if (global_decal_data->valid)
	{
		long decal_index;
		long iterations = 0;

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 641, decal_globals);

		for (decal_index = decal_globals->first_disconnected_decal_index; decal_index != NONE; )
		{
			struct location location;
			struct decal_datum *decal = decal_get(decal_index);
			long next_decal_index = decal->next_decal_index;

			if (iterations++ > MAXIMUM_DECAL_LIST_ITERATIONS)
			{
				error(_error_silent, "### ERROR decals: infinite loop -- tell Bernie!!");
				break;
			}

			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 660, decal->cluster_index==NONE);
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 661, decal->layer>=0 && decal->layer<NUMBER_OF_DECAL_LAYERS);

			decal_check(decal_index, FALSE);
			scenario_location_from_point(&location, &decal->position);

			if (location.cluster_index != NONE)
			{
				if (decal->next_decal_index != NONE)
				{
					decal_get(decal->next_decal_index)->prev_decal_index = decal->prev_decal_index;
				}

				if (decal->prev_decal_index != NONE)
				{
					decal_get(decal->prev_decal_index)->next_decal_index = decal->next_decal_index;
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 682, decal_globals->first_disconnected_decal_index==decal_index);
					decal_globals->first_disconnected_decal_index = decal->next_decal_index;
				}

				decal_reinsert(decal_index, location.cluster_index, decal->layer);
			}

			decal_check(decal_index, FALSE);
			decal_index = next_decal_index;
		}
	}

	return;
}

void decals_disconnect_from_structure_bsp(
	void)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 713, global_decal_data);

	if (global_decal_data->valid)
	{
		short cluster_index;

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 719, decal_globals);

		for (cluster_index = 0; cluster_index < MAXIMUM_CLUSTERS_PER_STRUCTURE; cluster_index++)
		{
			short layer;

			for (layer = 0; layer < NUMBER_OF_DECAL_LAYERS; layer++)
			{
				long first_decal_index = decal_get_first_decal_index(cluster_index, layer);
				long decal_index = first_decal_index;
				long iterations = 0;

				while (decal_index != NONE)
				{
					struct decal_datum *decal = decal_get(decal_index);
					long next_decal_index = decal->next_decal_index;

					if (iterations++ > MAXIMUM_DECAL_LIST_ITERATIONS)
					{
						error(_error_silent, "### ERROR decals: infinite loop -- tell Bernie!!");
						break;
					}

					match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 746, decal->cluster_index==cluster_index);
					decal->cluster_index = NONE;

					if (decal->next_decal_index == NONE)
					{
						decal->next_decal_index = decal_globals->first_disconnected_decal_index;

						if (decal_globals->first_disconnected_decal_index != NONE)
						{
							decal_get(decal_globals->first_disconnected_decal_index)->prev_decal_index = decal_index;
						}

						decal_globals->first_disconnected_decal_index = first_decal_index;
						decal_set_first_decal_index(cluster_index, layer, NONE);
					}

					decal_index = next_decal_index;
				}
			}
		}
	}

	return;
}

void decals_dispose_from_old_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 777, global_decal_data);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 778, decal_globals);

	rasterizer_decals_dispose_from_old_map();
	data_make_invalid(global_decal_data);

	return;
}

void decals_dispose(
	void)
{
	global_decal_data = NULL;
	rasterizer_decals_dispose();

	return;
}

void decals_unlock(
	boolean permanent)
{
	static boolean locked_count_warning_issued = FALSE;
	static boolean permanent_count_warning_issued = FALSE;

	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 801, global_decal_data);

	if (global_decal_data->valid)
	{
		struct data_iterator iterator;
		struct decal_datum *decal;

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 808, decal_globals);

		data_iterator_new(&iterator, global_decal_data);

		while (decal = data_iterator_next(&iterator))
		{
			if (TEST_FLAG(decal->flags, _decal_locked_bit))
			{
				SET_FLAG(decal->flags, _decal_locked_bit, FALSE);
				decal_globals->locked_count--;
			}

			if (permanent && TEST_FLAG(decal->flags, _decal_permanent_bit))
			{
				SET_FLAG(decal->flags, _decal_permanent_bit, FALSE);
				decal_globals->permanent_count--;
			}
		}

		if (decal_globals->locked_count)
		{
			if (!locked_count_warning_issued)
			{
				error(
					_error_silent,
					"### ERROR decals: locked count is invalid (#%d) -- tell Bernie!!",
					decal_globals->locked_count);
				locked_count_warning_issued = TRUE;
			}

			decal_globals->locked_count = 0;
		}

		if (permanent && decal_globals->permanent_count)
		{
			if (!permanent_count_warning_issued)
			{
				error(
					_error_silent,
					"### ERROR decals: permanent count is invalid (#%d) -- tell Bernie!!",
					decal_globals->permanent_count);
				permanent_count_warning_issued = TRUE;
			}

			decal_globals->permanent_count = 0;
		}
	}

	decal_geometry.decal_vertex_count = 0;
	decal_geometry.decal_surface_count = 0;

	return;
}

void decals_update(
	void)
{
	if (global_decal_data->valid)
	{
		struct data_iterator iterator;

		data_iterator_new(&iterator, global_decal_data);

		while (data_iterator_next(&iterator))
		{
			decal_update(iterator.index);
		}
	}

	return;
}

void decals_delete_permanent_from_cluster(
	short cluster_index)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 887, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);

	if (global_decal_data->valid)
	{
		short layer;

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 893, decal_globals);

		for (layer = 0; layer < NUMBER_OF_DECAL_LAYERS; layer++)
		{
			long decal_index;

			if (cluster_index == NONE)
			{
				if (layer)
				{
					continue;
				}

				decal_index = decal_globals->first_disconnected_decal_index;
			}
			else
			{
				decal_index = decal_get_first_decal_index(cluster_index, layer);
			}

			while (decal_index != NONE)
			{
				struct decal_datum *decal = decal_get(decal_index);
				long next_decal_index = decal->next_decal_index;

				match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 920, decal->cluster_index==cluster_index);

				if (TEST_FLAG(decal->flags, _decal_permanent_bit))
				{
					SET_FLAG(decal->flags, _decal_permanent_bit, FALSE);
					decal_globals->permanent_count--;
					match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 927, !TEST_FLAG(decal->flags, _decal_locked_bit));
					rasterizer_decal_vertices_delete(decal_index);
				}

				decal_index = next_decal_index;
			}
		}

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 936, decal_globals->permanent_count>=0);
	}

	return;
}

void decal_delete(
	long decal_index)
{
	static boolean locked_warning_issued = FALSE;
	static boolean permanent_warning_issued = FALSE;
	struct decal_datum *decal = decal_get(decal_index);

	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 947, decal);

	if (TEST_FLAG(decal->flags, _decal_locked_bit) && !locked_warning_issued)
	{
		error(_error_silent, "### ERROR decals: deleting locked decal (#%d) -- tell Bernie!!", decal_index);
		locked_warning_issued = TRUE;
	}

	if (TEST_FLAG(decal->flags, _decal_permanent_bit) && !permanent_warning_issued)
	{
		error(_error_silent, "### ERROR decals: deleting permanent decal (#%d) -- tell Bernie!!", decal_index);
		permanent_warning_issued = TRUE;
	}

	if (decal->next_decal_index != NONE)
	{
		decal_get(decal->next_decal_index)->prev_decal_index = decal->prev_decal_index;
	}

	if (decal->prev_decal_index != NONE)
	{
		decal_get(decal->prev_decal_index)->next_decal_index = decal->next_decal_index;
	}
	else if (decal->cluster_index == NONE)
	{
		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 987, decal_globals->first_disconnected_decal_index==decal_index);
		decal_globals->first_disconnected_decal_index = decal->next_decal_index;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 992, decal_get_first_decal_index(decal->cluster_index, decal->layer)==decal_index);
		decal_set_first_decal_index(decal->cluster_index, decal->layer, decal->next_decal_index);
	}

	datum_delete(global_decal_data, decal_index);

	return;
}

long decal_get_first_decal_index(
	short cluster_index,
	short layer)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1006, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1007, layer>=0 && layer<NUMBER_OF_DECAL_LAYERS);

	return decal_globals->first_decal_indices[layer][cluster_index];
}

/* ---------- private code */

static void decal_projection_create(
	real_matrix4x3 const *basis,
	real_rectangle2d const *extent,
	struct decal_projection *projection)
{
	real_point3d decal_point;

	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1040, basis);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1041, projection);

	projection->basis = *basis;
	projection->extent = *extent;
	plane3d_from_point_and_normal(&projection->plane, &basis->position, &basis->up);
	projection->axis = projection_from_vector3d(&projection->plane.n);
	projection->sign = projection_sign_from_vector3d(&projection->plane.n, projection->axis);

	decal_point.x = extent->x0 * basis->forward.i + basis->position.x + extent->y0 * basis->left.i;
	decal_point.y = extent->x0 * basis->forward.j + basis->position.y + extent->y0 * basis->left.j;
	decal_point.z = extent->x0 * basis->forward.k + basis->position.z + extent->y0 * basis->left.k;
	project_point3d(&decal_point, projection->axis, projection->sign, &projection->decal_points2d[0]);

	decal_point.x = extent->x1 * basis->forward.i + basis->position.x + extent->y0 * basis->left.i;
	decal_point.y = extent->x1 * basis->forward.j + basis->position.y + extent->y0 * basis->left.j;
	decal_point.z = extent->x1 * basis->forward.k + basis->position.z + extent->y0 * basis->left.k;
	project_point3d(&decal_point, projection->axis, projection->sign, &projection->decal_points2d[1]);

	decal_point.x = extent->x1 * basis->forward.i + basis->position.x + extent->y1 * basis->left.i;
	decal_point.y = extent->x1 * basis->forward.j + basis->position.y + extent->y1 * basis->left.j;
	decal_point.z = extent->x1 * basis->forward.k + basis->position.z + extent->y1 * basis->left.k;
	project_point3d(&decal_point, projection->axis, projection->sign, &projection->decal_points2d[2]);

	decal_point.x = extent->x0 * basis->forward.i + basis->position.x + extent->y1 * basis->left.i;
	decal_point.y = extent->x0 * basis->forward.j + basis->position.y + extent->y1 * basis->left.j;
	decal_point.z = extent->x0 * basis->forward.k + basis->position.z + extent->y1 * basis->left.k;
	project_point3d(&decal_point, projection->axis, projection->sign, &projection->decal_points2d[3]);

	vector_from_points2d(&projection->decal_points2d[0], &projection->decal_points2d[1], &projection->texture_u_axis);
	vector_from_points2d(&projection->decal_points2d[0], &projection->decal_points2d[3], &projection->texture_v_axis);
	projection->texture_scale = 1.f / cross_product2d(&projection->texture_u_axis, &projection->texture_v_axis);

	return;
}

static void decal_clip_to_surface(
	struct decal_projection const *projection,
	struct decal_geometry *geometry,
	long surface_index,
	boolean wrap,
	real radius,
	short type,
	long *surface_queue,
	short *surface_queue_write_index,
	long *deviant_surface_list,
	short *deviant_surface_count)
{
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1147, type>=0 && type<NUMBER_OF_DECAL_TYPES);

	if (surface_index != NONE)
	{
		real_plane3d surface_plane;
		real surface_angle;
		short working_surface_queue_write_index;
		short working_deviant_surface_count;
		struct collision_bsp const *bsp = global_collision_bsp_get();
		struct collision_surface const *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);

		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1160, projection);
		match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1161, geometry);

		if (wrap)
		{
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1165, surface_queue);
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1166, surface_queue_write_index && *surface_queue_write_index>=0 && *surface_queue_write_index<=MAXIMUM_DECAL_SURFACE_QUEUE_SIZE);
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1167, deviant_surface_list);
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1168, deviant_surface_count && *deviant_surface_count>=0 && *deviant_surface_count<=MAXIMUM_DECAL_SURFACE_QUEUE_SIZE);

			working_surface_queue_write_index = *surface_queue_write_index;
			working_deviant_surface_count = *deviant_surface_count;
		}

		bsp3d_get_plane_from_designator(&bsp->bsp3d, surface->plane_designator, &surface_plane);
		surface_angle = angle_between_normals3d(&surface_plane.n, &projection->plane.n);

		if (!wrap || (_pi / 180.f) * decal_wrap_parameters[type].minimum_wrap_angle >= surface_angle)
		{
			real_point2d surface_point2d_previous;
			real_point2d surface_point2d;
			real_plane2d surface_edge_plane;
			real_point2d *clipped_points;
			long edge_index = surface->first_edge_index;
			short edge_count = 0;
			real_point2d const *points = projection->decal_points2d;
			short point_count = DECAL_VERTICES_PER_QUAD;
			unsigned long clip_flags = 0;

			do
			{
				boolean clipped;
				struct collision_edge const *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
				boolean forward = edge->surface_indices[1] == surface_index;
				struct collision_vertex const *vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!forward], struct collision_vertex);

				clipped_points = decal_points2d_temp[edge_count & 1];

				if (!edge_count)
				{
					struct collision_vertex const *previous_vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[forward], struct collision_vertex);

					project_point3d(
						&previous_vertex->point,
						projection->axis,
						projection->sign,
						&surface_point2d_previous);
				}

				project_point3d(&vertex->point, projection->axis, projection->sign, &surface_point2d);

				if (plane2d_from_points(&surface_edge_plane, &surface_point2d, &surface_point2d_previous))
				{
					point_count = convex_polygon2d_clip_to_plane(
						point_count,
						points,
						&surface_edge_plane,
						MAXIMUM_POINTS_PER_DECAL_SURFACE,
						clipped_points,
						&clip_flags,
						&clipped,
						0.f);

					if (wrap && clipped && working_surface_queue_write_index < MAXIMUM_DECAL_SURFACE_QUEUE_SIZE)
					{
						real_vector3d surface_edge_vector;

						vector_from_points3d(&vertex->point, &TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[forward], struct collision_vertex)->point, &surface_edge_vector);

						if (fast_vector_intersects_sphere(&vertex->point, &surface_edge_vector, &projection->basis.position, radius * decal_wrap_parameters[type].radius_exclusion_multiplier))
						{
							short queue_index;
							long adjacent_surface_index = edge->surface_indices[!forward];

							for (queue_index = 0; adjacent_surface_index != NONE && queue_index < working_surface_queue_write_index; queue_index++)
							{
								if (surface_queue[queue_index] == adjacent_surface_index)
								{
									adjacent_surface_index = NONE;
								}
							}

							if (adjacent_surface_index != NONE)
							{
								surface_queue[working_surface_queue_write_index++] = adjacent_surface_index;
							}
						}
					}
				}
				else
				{
					point_count = 0;
				}

				points = clipped_points;
				edge_index = edge->edge_indices[forward];
				edge_count++;
				surface_point2d_previous = surface_point2d;
			}
			while (edge_index != surface->first_edge_index && point_count > 0);

			if (point_count >= NUMBER_OF_VERTICES_PER_TRIANGLE &&
				point_count <= MAXIMUM_TOTAL_VERTICES_PER_DECAL - geometry->decal_vertex_count &&
				!TEST_FLAG(surface->flags, _collision_surface_two_sided_bit) &&
				!TEST_FLAG(surface->flags, _collision_surface_invisible_bit) &&
				!TEST_FLAG(surface->flags, _collision_surface_breakable_bit))
			{
				short point_index;

				match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1365, geometry->decal_surface_count<MAXIMUM_DECAL_SURFACE_QUEUE_SIZE);
				geometry->decal_surface_indices[geometry->decal_surface_count] = surface_index;
				geometry->decal_surface_vertex_counts[geometry->decal_surface_count++] = point_count;

				for (point_index = 0; point_index < point_count; point_index++)
				{
					real_vector2d vector;
					real u;
					real v;

					vector_from_points2d(&projection->decal_points2d[0], &points[point_index], &vector);
					u = cross_product2d(&vector, &projection->texture_v_axis) * projection->texture_scale;
					v = -projection->texture_scale * cross_product2d(&vector, &projection->texture_u_axis);
					geometry->decal_vertices[geometry->decal_vertex_count].texcoord.x = u;
					geometry->decal_vertices[geometry->decal_vertex_count].texcoord.y = v;
					geometry->decal_vertices[geometry->decal_vertex_count].clipped = TEST_FLAG(clip_flags, point_index);
					project_point2d(
						&points[point_index],
						&surface_plane,
						projection->axis,
						projection->sign,
						&geometry->decal_vertices[geometry->decal_vertex_count].position);

					if (!TEST_FLAG(clip_flags, point_index))
					{
						point_from_line3d(
							&geometry->decal_vertices[geometry->decal_vertex_count].position,
							&surface_plane.n,
							rasterizer_debug_options.zoffset,
							&geometry->decal_vertices[geometry->decal_vertex_count].position);
					}

					geometry->decal_vertex_count++;
				}
			}
		}
		else
		{
			long edge_index = surface->first_edge_index;

			do
			{
				struct collision_edge const *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
				boolean forward = edge->surface_indices[1] == surface_index;
				struct collision_vertex const *vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!forward], struct collision_vertex);

				if (working_surface_queue_write_index < MAXIMUM_DECAL_SURFACE_QUEUE_SIZE)
				{
					real_vector3d surface_edge_vector;
					struct collision_vertex const *next_vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[forward], struct collision_vertex);

					vector_from_points3d(&vertex->point, &next_vertex->point, &surface_edge_vector);

					if (fast_vector_intersects_sphere(&vertex->point, &surface_edge_vector, &projection->basis.position, radius * decal_wrap_parameters[type].radius_exclusion_multiplier))
					{
						short queue_index;
						long adjacent_surface_index = edge->surface_indices[!forward];

						for (queue_index = 0; adjacent_surface_index != NONE && queue_index < working_surface_queue_write_index; queue_index++)
						{
							if (surface_queue[queue_index] == adjacent_surface_index)
							{
								adjacent_surface_index = NONE;
							}
						}

						if (adjacent_surface_index != NONE)
						{
							surface_queue[working_surface_queue_write_index++] = adjacent_surface_index;
						}
					}
				}

				edge_index = edge->edge_indices[forward];
			}
			while (edge_index != surface->first_edge_index);

			if ((_pi / 180.f) * decal_wrap_parameters[type].minimum_skip_angle >= surface_angle && working_deviant_surface_count < MAXIMUM_DECAL_SURFACE_QUEUE_SIZE)
			{
				deviant_surface_list[working_deviant_surface_count++] = surface_index;
			}
		}

		if (wrap)
		{
			*surface_queue_write_index = working_surface_queue_write_index;
			*deviant_surface_count = working_deviant_surface_count;
		}
	}

	return;
}

/* ---------- public code */

void decal_new(
	long definition_index,
	real_point3d const *origin,
	real_vector3d const *velocity,
	real radius_modifier,
	boolean permanent,
	short forced_sequence_index,
	struct decal_editor_geometry *editor_geometry)
{
	if (decals_enabled)
	{
		struct collision_result collision;
		unsigned long *local_random_seed_address = get_global_local_random_seed_address();
		unsigned long local_random_seed = 0;

		if (permanent)
		{
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1533, local_random_seed_address);
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1534, origin);

			local_random_seed = *local_random_seed_address;
			*local_random_seed_address = *(unsigned long *)&origin->x ^ *(unsigned long *)&origin->y ^ *(unsigned long *)&origin->z ^ DECAL_LOCAL_RANDOM_SEED_SALT;
		}

		match_collision_log_begin_user("c:\\halo\\SOURCE\\effects\\decals.c", 1550, _collision_user_decals);

		if (collision_test_vector(_collision_test_environment_flags | FLAG(_collision_test_try_to_keep_location_valid_bit), origin, velocity, NONE, &collision))
		{
			if (collision.type == _collision_result_media)
			{
				decal_new_from_media_collision();
			}
			else if (collision.type == _collision_result_structure && !TEST_FLAG(decal_definition_get(definition_index)->flags, _decal_definition_water_effect_bit))
			{
				decal_new_from_collision(
					definition_index,
					&collision,
					velocity,
					radius_modifier,
					permanent,
					forced_sequence_index,
					editor_geometry);
			}
		}

		match_collision_log_end_user("c:\\halo\\SOURCE\\effects\\decals.c", 1586);

		if (permanent)
		{
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 1590, local_random_seed_address);
			*local_random_seed_address = local_random_seed;
		}
	}

	return;
}

void decal_new_from_media_collision(
	void)
{
	return;
}

void decal_new_from_collision(
	long definition_index,
	struct collision_result const *collision,
	real_vector3d const *velocity,
	real radius_modifier,
	boolean permanent,
	short forced_sequence_index,
	struct decal_editor_geometry *editor_geometry)
{
	struct collision_bsp const *bsp = global_collision_bsp_get();
	struct decal_definition const *definition;
	long surface_queue[MAXIMUM_DECAL_SURFACE_QUEUE_SIZE];
	short surface_queue_read_index;
	short surface_queue_write_index;
	long deviant_surface_list[MAXIMUM_DECAL_SURFACE_QUEUE_SIZE];
	short deviant_surface_count;
	struct decal_projection projection;
	real_rectangle3d up_bounds;
	real radius;
	real_matrix4x3 basis;
	real_rectangle2d extent;
	real_rectangle2d sprite_bounds;
	short sequence_index;
	short sprite_index;
	short bitmap_index;
	struct bitmap_group const *bitmap_group;
	boolean geometry_inherited = FALSE;

	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2033, collision);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2034, velocity);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2035, decals_enabled);
	match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2037, !editor_geometry);

	while (definition_index != NONE)
	{
		surface_queue_read_index = 0;
		surface_queue_write_index = 0;
		deviant_surface_count = 0;
		definition = decal_definition_get(definition_index);
		bitmap_group = bitmap_group_get(definition->shader.decal.map.index);

		if (!geometry_inherited)
		{
			real cosine_angle;
			real sine_angle;
			real_vector3d left;
			real_vector3d up;
			real_vector3d forward;

			if (TEST_FLAG(definition->flags, _decal_definition_no_random_rotation_bit) && dot_product3d(&collision->plane.n, velocity) < -_real_epsilon)
			{
				cosine_angle = -1.f;
				sine_angle = 0.f;

				if (TEST_FLAG(definition->flags, _decal_definition_SAPIEN_ONLY_snap_to_axis_bit))
				{
					short projection_axis = projection_from_vector3d(velocity);
					real sign = projection_sign_from_vector3d(velocity, projection_axis) ? 1.f : -1.f;

					switch (projection_axis)
					{
					case _x:
						set_real_vector3d(&forward, sign, 0.f, 0.f);
						break;
					case _y:
						set_real_vector3d(&forward, 0.f, sign, 0.f);
						break;
					case _z:
						set_real_vector3d(&forward, 0.f, 0.f, sign);
						break;
					default:
						match_vhalt("c:\\halo\\SOURCE\\effects\\decals.c", 2120, "### ERROR unsupported projection dimension");
						break;
					}

					if (dot_product3d(&collision->plane.n, velocity) > 0.f)
					{
						add_vectors3d(&forward, &collision->plane.n, &forward);
					}
					else
					{
						subtract_vectors3d(&forward, &collision->plane.n, &forward);
					}

					normalize3d(&forward);
					cross_product3d(&collision->plane.n, &forward, &left);
					cross_product3d(&collision->plane.n, &left, &up);

					if (magnitude_squared3d(&left) < _real_epsilon || magnitude_squared3d(&up) < _real_epsilon)
					{
						short projection_axis;
						real sign;

						point_from_line3d(
							(real_point3d const *)velocity,
							&collision->plane.n,
							-(dot_product3d(velocity, &collision->plane.n)),
							(real_point3d *)&forward);
						projection_axis = projection_from_vector3d(&forward);
						sign = projection_sign_from_vector3d(&forward, projection_axis) ? 1.f : -1.f;

						switch (projection_axis)
						{
						case _x:
							set_real_vector3d(&forward, sign, 0.f, 0.f);
							break;
						case _y:
							set_real_vector3d(&forward, 0.f, sign, 0.f);
							break;
						case _z:
							set_real_vector3d(&forward, 0.f, 0.f, sign);
							break;
						default:
							match_vhalt("c:\\halo\\SOURCE\\effects\\decals.c", 2152, "### ERROR unsupported projection dimension");
							break;
						}

						if (dot_product3d(&collision->plane.n, &forward) > 0.f)
						{
							add_vectors3d(&forward, &collision->plane.n, &forward);
						}
						else
						{
							subtract_vectors3d(&forward, &collision->plane.n, &forward);
						}

						normalize3d(&forward);
						cross_product3d(&collision->plane.n, &forward, &left);
						cross_product3d(&collision->plane.n, &left, &up);
					}
				}
				else
				{
					cross_product3d(&collision->plane.n, velocity, &left);
					cross_product3d(&collision->plane.n, &left, &up);
				}
			}
			else
			{
				real angle = real_local_random_range(0.f, 2.f * _pi);

				cosine_angle = cosine(angle);
				sine_angle = sine(angle);
				perpendicular3d(&collision->plane.n, &left);
				cross_product3d(&collision->plane.n, &left, &up);
			}

			normalize3d(&left);
			normalize3d(&up);
			basis.position = collision->point;
			basis.up = collision->plane.n;
			basis.forward.i = up.i * cosine_angle - left.i * sine_angle;
			basis.forward.j = up.j * cosine_angle - left.j * sine_angle;
			basis.forward.k = up.k * cosine_angle - left.k * sine_angle;
			basis.left.i = up.i * sine_angle + left.i * cosine_angle;
			basis.left.j = up.j * sine_angle + left.j * cosine_angle;
			basis.left.k = up.k * sine_angle + left.k * cosine_angle;

			if (forced_sequence_index != NONE)
			{
				sequence_index = forced_sequence_index;
			}
			else
			{
				sequence_index = local_random_range(0, (short)bitmap_group->sequences.count);

				if (sequence_index >= bitmap_group->sequences.count)
				{
					error(
						_error_silent,
						"### DECALS: Bernie doesn't understand how local_random_range() works, don't tell Jason!!");
					sequence_index = bitmap_group->sequences.count - 1;
				}
			}

			sprite_index = 0;

			if (radius_modifier == 0.f)
			{
				radius_modifier = 1.f;
			}

			radius = real_local_random_range(definition->radius_lower_bounds, definition->radius_upper_bounds) * radius_modifier;
		}

		if (bitmap_group->type == _bitmap_group_type_sprites)
		{
			struct bitmap_group_sequence const *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, sequence_index, struct bitmap_group_sequence);
			struct bitmap_group_sprite const *sprite = TAG_BLOCK_GET_ELEMENT(&sequence->sprites, sprite_index, struct bitmap_group_sprite);

			bitmap_index = sprite->bitmap_index;
			decal_sprite_get_bounds(definition, sequence_index, 0, radius, &sprite_bounds, &extent);
		}
		else
		{
			real aspect_ratio = 1.f;

			bitmap_index = 0;

			if (TEST_FLAG(definition->flags, _decal_definition_preserve_aspect_bit))
			{
				struct bitmap_data const *bitmap = TAG_BLOCK_GET_ELEMENT(&bitmap_group->bitmaps, bitmap_index, struct bitmap_data);

				aspect_ratio = (real)bitmap->height / (real)bitmap->width;
			}

			extent.x0 = -radius;
			extent.y0 = -radius * aspect_ratio;
			extent.x1 = radius;
			extent.y1 = radius * aspect_ratio;
			sprite_bounds.y0 = 0.f;
			sprite_bounds.x0 = 0.f;
			sprite_bounds.y1 = 1.f;
			sprite_bounds.x1 = 1.f;
		}

		if (!permanent && !_texture_cache_bitmap_get_hardware_format(TAG_BLOCK_GET_ELEMENT(&bitmap_group->bitmaps, bitmap_index, struct bitmap_data), FALSE, TRUE))
		{
			break;
		}

		decal_projection_create(&basis, &extent, &projection);
		up_bounds.x1 = basis.up.i;
		up_bounds.x0 = basis.up.i;
		up_bounds.y1 = basis.up.j;
		up_bounds.y0 = basis.up.j;
		up_bounds.z1 = basis.up.k;
		up_bounds.z0 = basis.up.k;
		decal_geometry.decal_surface_count = 0;
		decal_geometry.decal_vertex_count = 0;
		surface_queue[surface_queue_write_index++] = collision->surface_index;

		while (surface_queue_read_index < surface_queue_write_index)
		{
			long surface_index;

			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2353, surface_queue_read_index<MAXIMUM_DECAL_SURFACE_QUEUE_SIZE);
			match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2354, surface_queue_write_index<=MAXIMUM_DECAL_SURFACE_QUEUE_SIZE);

			surface_index = surface_queue[surface_queue_read_index++];
			decal_clip_to_surface(
				&projection,
				&decal_geometry,
				surface_index,
				TRUE,
				radius,
				definition->type,
				surface_queue,
				&surface_queue_write_index,
				deviant_surface_list,
				&deviant_surface_count);
		}

		if (decal_wrap_parameters[definition->type].wrap)
		{
			short remaining_deviant_surface_count = deviant_surface_count;
			long deviant_surface_bunch[MAXIMUM_DECAL_SURFACE_QUEUE_SIZE];

			while (remaining_deviant_surface_count > 0)
			{
				short deviant_surface_bunch_size = 0;
				short deviant_surface_index;

				for (deviant_surface_index = 0; !deviant_surface_bunch_size && deviant_surface_index < deviant_surface_count; deviant_surface_index++)
				{
					long surface_index = deviant_surface_list[deviant_surface_index];

					if (surface_index != NONE)
					{
						real_plane3d surface_plane;
						short other_deviant_surface_index;
						long other_surface_index;
						struct collision_surface const *other_surface;
						real_plane3d other_surface_plane;
						real angle;
						real_point3d closest_edge_point0;
						real_point3d closest_edge_point1;
						real closest_distance0;
						real closest_distance1;
						real_plane3d closest_surface_plane;
						long closest_surface_index;
						struct decal_projection wrapped_projection;
						short bunch_index;
						long bunch_surface_index;
						struct collision_surface const *bunch_surface;
						long edge_index;
						struct collision_edge const *edge;
						boolean forward_edge;
						struct collision_vertex const *vertex0;
						struct collision_vertex const *vertex1;
						real distance0;
						real distance1;
						real temporary;
						real_vector3d axis;
						real_matrix4x3 rotation;
						real_matrix4x3 wrapped_basis;
						real sign;
						real wrap_angle;
						struct collision_surface const *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);

						bsp3d_get_plane_from_designator(&bsp->bsp3d, surface->plane_designator, &surface_plane);
						match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2399, deviant_surface_bunch_size<MAXIMUM_DECAL_SURFACE_QUEUE_SIZE);
						deviant_surface_bunch[deviant_surface_bunch_size++] = surface_index;
						deviant_surface_list[deviant_surface_index] = NONE;

						for (other_deviant_surface_index = deviant_surface_index + 1; other_deviant_surface_index < deviant_surface_count; other_deviant_surface_index++)
						{
							other_surface_index = deviant_surface_list[other_deviant_surface_index];

							if (other_surface_index != NONE)
							{
								other_surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, other_surface_index, struct collision_surface);
								bsp3d_get_plane_from_designator(
									&bsp->bsp3d,
									other_surface->plane_designator,
									&other_surface_plane);
								angle = angle_between_normals3d(&surface_plane.n, &other_surface_plane.n);

								if (angle <= (_pi / 180.f) * decal_wrap_parameters[definition->type].minimum_wrap_angle)
								{
									match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2422, deviant_surface_bunch_size<MAXIMUM_DECAL_SURFACE_QUEUE_SIZE);
									deviant_surface_bunch[deviant_surface_bunch_size++] = other_surface_index;
									deviant_surface_list[other_deviant_surface_index] = NONE;
								}
							}
						}

						closest_surface_index = NONE;

						for (bunch_index = 0; bunch_index < deviant_surface_bunch_size; bunch_index++)
						{
							bunch_surface_index = deviant_surface_bunch[bunch_index];
							bunch_surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, bunch_surface_index, struct collision_surface);
							edge_index = bunch_surface->first_edge_index;

							do
							{
								edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
								forward_edge = edge->surface_indices[1] == bunch_surface_index;
								vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!forward_edge], struct collision_vertex);
								vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[forward_edge], struct collision_vertex);
								distance0 = fabs(plane3d_distance_to_point(&projection.plane, &vertex0->point));
								distance1 = fabs(plane3d_distance_to_point(&projection.plane, &vertex1->point));

								if (distance0 > distance1)
								{
									temporary = distance0;
									distance0 = distance1;
									distance1 = temporary;
								}

								if (closest_surface_index == NONE || distance0 <= closest_distance0 && distance1 <= closest_distance1)
								{
									bsp3d_get_plane_from_designator(
										&bsp->bsp3d,
										bunch_surface->plane_designator,
										&closest_surface_plane);
									closest_distance0 = distance0;
									closest_distance1 = distance1;
									closest_edge_point0 = vertex0->point;
									closest_edge_point1 = vertex1->point;
									closest_surface_index = bunch_surface_index;
								}

								edge_index = edge->edge_indices[forward_edge];
							}
							while (edge_index != bunch_surface->first_edge_index);
						}

						match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2493, closest_surface_index!=NONE);
						vector_from_points3d(&closest_edge_point0, &closest_edge_point1, &axis);

						if (normalize3d(&axis) > 0.f)
						{
							sign = triple_product3d(&closest_surface_plane.n, &projection.plane.n, &axis) < 0.f ? 1.f : -1.f;
							wrap_angle = angle_between_normals3d(&closest_surface_plane.n, &projection.plane.n) * sign;
							matrix4x3_rotation_from_axis_and_angle(
								&rotation,
								&axis,
								sine(wrap_angle),
								cosine(wrap_angle));
							wrapped_basis.position.x = basis.position.x - closest_edge_point0.x;
							wrapped_basis.position.y = basis.position.y - closest_edge_point0.y;
							wrapped_basis.position.z = basis.position.z - closest_edge_point0.z;
							matrix4x3_transform_point(&rotation, &wrapped_basis.position, &wrapped_basis.position);
							matrix4x3_transform_normal(&rotation, &basis.forward, &wrapped_basis.forward);
							matrix4x3_transform_normal(&rotation, &basis.left, &wrapped_basis.left);
							matrix4x3_transform_normal(&rotation, &basis.up, &wrapped_basis.up);
							wrapped_basis.position.x += closest_edge_point0.x;
							wrapped_basis.position.y += closest_edge_point0.y;
							wrapped_basis.position.z += closest_edge_point0.z;
							wrapped_basis.scale = 1.f;
							decal_projection_create(&wrapped_basis, &extent, &wrapped_projection);
							up_bounds.x0 = MIN(wrapped_basis.up.i, up_bounds.x0);
							up_bounds.x1 = MAX(wrapped_basis.up.i, up_bounds.x1);
							up_bounds.y0 = MIN(wrapped_basis.up.j, up_bounds.y0);
							up_bounds.y1 = MAX(wrapped_basis.up.j, up_bounds.y1);
							up_bounds.z0 = MIN(wrapped_basis.up.k, up_bounds.z0);
							up_bounds.z1 = MAX(wrapped_basis.up.k, up_bounds.z1);
						}
						else
						{
							error(
								_error_silent,
								"### ERROR: decals: failed to wrap decal around edge -- tell Bernie!!");
							wrapped_projection = projection;
						}

						for (bunch_index = 0; bunch_index < deviant_surface_bunch_size; bunch_index++)
						{
							long bunch_clip_surface_index = deviant_surface_bunch[bunch_index];

							decal_clip_to_surface(
								&wrapped_projection,
								&decal_geometry,
								bunch_clip_surface_index,
								FALSE,
								radius,
								definition->type,
								NULL,
								NULL,
								NULL,
								NULL);
						}

						remaining_deviant_surface_count -= deviant_surface_bunch_size;
					}
				}
			}
		}

		if (decal_geometry.decal_surface_count > 0 && decal_geometry.decal_vertex_count > 0)
		{
			real_vector3d offset = { 0.f, 0.f, 0.f };
			short decal_quad_count = 0;
			short decal_surface_index;
			long cache_size;
			long cache_index;

			if (up_bounds.x1 - up_bounds.x0 <= DECAL_WRAP_BOUNDS_TOLERANCE &&
				up_bounds.y1 - up_bounds.y0 <= DECAL_WRAP_BOUNDS_TOLERANCE &&
				up_bounds.z1 - up_bounds.z0 <= DECAL_WRAP_BOUNDS_TOLERANCE)
			{
				set_real_vector3d(
					&offset,
					up_bounds.x1 + up_bounds.x0,
					up_bounds.y1 + up_bounds.y0,
					up_bounds.z1 + up_bounds.z0);
				normalize3d(&offset);
				scale_vector3d(&offset, rasterizer_debug_options.zoffset, &offset);
			}

			for (decal_surface_index = 0; decal_surface_index < decal_geometry.decal_surface_count; decal_surface_index++)
			{
				match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2650, decal_geometry.decal_surface_vertex_counts[decal_surface_index]>=NUMBER_OF_VERTICES_PER_TRIANGLE);
				decal_quad_count += (decal_geometry.decal_surface_vertex_counts[decal_surface_index] - 1) / 2;
			}

			cache_size = decal_quad_count * DECAL_VERTICES_PER_QUAD * sizeof(struct decal_vertex);
			cache_index = rasterizer_decal_vertices_new(cache_size);

			if (cache_index != NONE)
			{
				long decal_index = decal_insert(cache_index, collision->location.cluster_index, definition->layer, NONE, permanent);

				if (decal_index != NONE)
				{
					struct decal_datum *decal = decal_get(decal_index);
					struct decal_vertex *decal_vertices = rasterizer_decal_vertices_lock(cache_index, cache_size);

					if (decal_vertices)
					{
						struct decal_vertex vertices[MAXIMUM_TOTAL_VERTICES_PER_DECAL];
						short vertex_index;
						short quad_index;
						struct decal_vertex_internal const *vertex;
						real_point2d texcoord;
						short u;
						short v;

						for (vertex_index = 0; vertex_index < decal_geometry.decal_vertex_count; vertex_index++)
						{
							vertex = &decal_geometry.decal_vertices[vertex_index];
							texcoord.x = PIN((sprite_bounds.x1 - sprite_bounds.x0) * vertex->texcoord.x + sprite_bounds.x0, 0.f, 1.f);
							texcoord.y = PIN((sprite_bounds.y1 - sprite_bounds.y0) * vertex->texcoord.y + sprite_bounds.y0, 0.f, 1.f);
							u = (short)fast_ftol((real)floor(PIN(texcoord.x * DECAL_TEXCOORD_SCALE, 0.f, DECAL_TEXCOORD_MAXIMUM) + 0.5f));
							v = (short)fast_ftol((real)floor(PIN(texcoord.y * DECAL_TEXCOORD_SCALE, 0.f, DECAL_TEXCOORD_MAXIMUM) + 0.5f));
							match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2696, (u&0x8000)==0 && (v&0x8000)==0);
							vertices[vertex_index].texcoord_u = u;
							vertices[vertex_index].texcoord_v = v;
							vertices[vertex_index].position.x = vertex->position.x + offset.i;
							vertices[vertex_index].position.y = vertex->position.y + offset.j;
							vertices[vertex_index].position.z = vertex->position.z + offset.k;
						}

						{
							real intensity;
							real_rgb_color color;
							short decal_surface_vertex_count;
							short decal_surface_vertex_index;

							decal->position = collision->point;
							decal->creation_time = game_time_get();
							decal->sequence_index = (char)sequence_index;
							decal->sprite_index = 0;
							decal->bitmap_index = (char)bitmap_index;
							decal->lifetime = real_local_random_range(
								definition->lifetime_lower_bounds,
								definition->lifetime_upper_bounds);
							decal->decay_time = real_local_random_range(
								definition->decay_time_lower_bounds,
								definition->decay_time_upper_bounds);
							decal->quad_count = decal_quad_count;
							decal->definition_index = definition_index;
							intensity = real_local_random_range(
								definition->intensity_lower_bounds,
								definition->intensity_upper_bounds);
							rgb_colors_interpolate(
								&color,
								(definition->flags >> 1) & 3,
								&definition->color_lower_bounds,
								&definition->color_upper_bounds,
								real_local_random_range(0.f, 1.f));
							decal->color = real_a_rgb_color_to_pixel32(intensity, &color);
							decal->intensity = UNSIGNED_CHAR_MAX;
							quad_index = 0;
							vertex_index = 0;

							for (decal_surface_index = 0; decal_surface_index < decal_geometry.decal_surface_count; decal_surface_index++)
							{
								decal_surface_vertex_count = decal_geometry.decal_surface_vertex_counts[decal_surface_index];
								match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2744, decal_surface_vertex_count>=NUMBER_OF_VERTICES_PER_TRIANGLE);

								for (decal_surface_vertex_index = 1; decal_surface_vertex_index + 1 < decal_surface_vertex_count; decal_surface_vertex_index += 2)
								{
									match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2748, decal_surface_vertex_index+1<decal_surface_vertex_count);
									decal_vertices[0] = vertices[vertex_index];
									decal_vertices[1] = vertices[vertex_index + decal_surface_vertex_index];
									decal_vertices[2] = vertices[vertex_index + decal_surface_vertex_index + 1];
									decal_vertices[3] = vertices[decal_surface_vertex_index + 2 >= decal_surface_vertex_count ? vertex_index : vertex_index + decal_surface_vertex_index + 2];
									decal_vertices += DECAL_VERTICES_PER_QUAD;
									quad_index++;
								}

								vertex_index += decal_surface_vertex_count;
							}

							match_assert("c:\\halo\\SOURCE\\effects\\decals.c", 2764, quad_index==decal_quad_count);
							rasterizer_decal_vertices_unlock();
							geometry_inherited = TEST_FLAG(definition->flags, _decal_definition_geometry_inherited_by_next_decal_in_chain_bit);
							definition_index = definition->next_decal_in_chain.index;
						}
					}
					else
					{
						rasterizer_decal_vertices_delete(cache_index);
						definition_index = NONE;

						if (debug_decals)
						{
							error(_error_silent, "### WTF decals: failed to lock vertices");
						}
					}
				}
				else
				{
					rasterizer_decal_vertices_delete(cache_index);
					definition_index = NONE;

					if (debug_decals)
					{
						error(
							_error_silent,
							"### WTF decals: failed to insert decal (locked=%d, permanent=%d)",
							decal_globals->locked_count,
							decal_globals->permanent_count);
					}
				}
			}
			else
			{
				definition_index = NONE;

				if (debug_decals)
				{
					error(
						_error_silent,
						"### WTF decals: failed to allocate vertices (locked=%d, permanent=%d)",
						decal_globals->locked_count,
						decal_globals->permanent_count);
				}
			}
		}
		else
		{
			definition_index = NONE;
		}
	}

	return;
}

void render_debug_decals(
	void)
{
	if (debug_decals)
	{
		short decal_surface_index;
		short rendered_cluster_index;
		short vertex_offset = 0;

		for (decal_surface_index = 0; decal_surface_index < decal_geometry.decal_surface_count; decal_surface_index++)
		{
			short vertex_index;
			short other_surface_index;
			short vertex_count = decal_geometry.decal_surface_vertex_counts[decal_surface_index];
			struct decal_vertex_internal const *previous_vertex = &decal_geometry.decal_vertices[vertex_offset + vertex_count - 1];

			for (vertex_index = 0; vertex_index < decal_geometry.decal_surface_vertex_counts[decal_surface_index]; vertex_index++)
			{
				struct decal_vertex_internal const *vertex = &decal_geometry.decal_vertices[vertex_offset + vertex_index];

				render_debug_line(TRUE, &previous_vertex->position, &vertex->position, global_real_argb_yellow);
				render_debug_point(
					TRUE,
					&vertex->position,
					DECAL_DEBUG_POINT_SIZE,
					vertex->clipped ? global_real_argb_red : global_real_argb_white);
				previous_vertex = vertex;
			}

			for (other_surface_index = 0; other_surface_index < decal_geometry.decal_surface_count; other_surface_index++)
			{
				if (other_surface_index != decal_surface_index && decal_geometry.decal_surface_indices[decal_surface_index] == decal_geometry.decal_surface_indices[other_surface_index])
				{
					error(_error_silent, "### ERROR decals: duplicate surface indices in queue -- tell Bernie!!");
				}
			}

			vertex_offset += vertex_count;
		}

		for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
		{
			short layer;
			short cluster_index = rendered_cluster_get(rendered_cluster_index)->cluster_index;

			for (layer = 0; layer < NUMBER_OF_DECAL_LAYERS; layer++)
			{
				struct decal_datum const *decal;
				long decal_index;

				for (decal_index = decal_get_first_decal_index(cluster_index, layer); decal_index != NONE; decal_index = decal->next_decal_index)
				{
					char decal_string[64];

					decal = decal_get(decal_index);
					sprintf(decal_string, "%d", DECAL_VERTICES_PER_QUAD * decal->quad_count);
					render_debug_string_at_point(FALSE, &decal->position, decal_string, global_real_argb_red);
				}
			}
		}
	}

	return;
}
