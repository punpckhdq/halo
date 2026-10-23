/*
STRUCTURE_DETAIL_OBJECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "structures.h"
#include "detail_object_definitions.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "bitmap_macros.h"
#include "game_state.h"
#include "render_debug.h"

/* ---------- constants */

#define DEBUG_DETAIL_OBJECT_POINT_SIZE 0.1f		/* fake name */
#define DEBUG_DETAIL_OBJECT_ALPHA 0.3f			/* fake name */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static real calculate_world_from_cell_index_and_offset(real cell, real offset);
static struct detail_object_runtime_data *get_local_player_datum(short local_player_index);
static boolean key_compare_cells_lower_bound(struct detail_object_cell_coord const *key, struct detail_object_cell_definition const *cell);
static boolean key_compare_cells_upper_bound(struct detail_object_cell_coord const *key, struct detail_object_cell_definition const *cell);
static struct detail_object_cell_definition *get_lower_bound_cell(struct detail_object_cell_definition *begin, struct detail_object_cell_definition *end, struct detail_object_cell_coord const *key);
static struct detail_object_cell_definition *get_upper_bound_cell(struct detail_object_cell_definition *begin, struct detail_object_cell_definition *end, struct detail_object_cell_coord const *key);

/* ---------- globals */

boolean debug_detail_objects = FALSE;

static struct detail_object_global_runtime_data *detail_object_global_runtime_data = NULL;
static boolean fudge_vector = FALSE;
static real fudge_offset = 0.f;
static real final_offset = 0.f;

/* ---------- public code */

static real calculate_world_from_cell_index_and_offset(
	real cell,
	real offset)
{
	return (offset / DETAIL_OBJECT_CELL_GRANULARITY + cell) * DETAIL_OBJECT_CELL_SIZE;
}

static struct detail_object_runtime_data *get_local_player_datum(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structure_detail_objects.c", 86, local_player_index==0);

	return &detail_object_global_runtime_data->local_player_data[local_player_index];
}

void structure_detail_objects_initialize(
	void)
{
	detail_object_global_runtime_data = game_state_malloc("structure detail objects", NULL, sizeof(*detail_object_global_runtime_data));
	detail_object_global_runtime_data->default_z_reference_vector.i = 0.f;
	detail_object_global_runtime_data->default_z_reference_vector.j = 0.f;
	detail_object_global_runtime_data->default_z_reference_vector.k = 1.f;
	detail_object_global_runtime_data->default_z_reference_vector.l = 0.f;

	return;
}

void structure_detail_objects_initialize_for_new_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structure_detail_objects.c", 109, detail_object_global_runtime_data);
	memset(detail_object_global_runtime_data, 0, sizeof(*detail_object_global_runtime_data));
	structure_detail_objects_flush();

	return;
}

void structure_detail_objects_dispose_from_old_map(
	void)
{
	return;
}

void structure_detail_objects_dispose(
	void)
{
	return;
}

void structure_detail_objects_flush(
	void)
{
	get_local_player_datum(0)->last_cell_coord.initialized = FALSE;

	return;
}

void structure_render_detail_objects(
	void)
{
	if (local_player_count() == 1 && render.local_player_index != NONE)
	{
		struct structure_detail_object_data *data = global_structure_bsp_get()->detail_object_data.count ?
			TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->detail_object_data, 0, struct structure_detail_object_data) :
			NULL;
		struct detail_object_runtime_data *runtime_data = get_local_player_datum(0);
		struct detail_object_cell_coord coord =
		{
			(short)fast_ftol(render.camera.position.x / DETAIL_OBJECT_CELL_SIZE - 0.5f),
			(short)fast_ftol(render.camera.position.y / DETAIL_OBJECT_CELL_SIZE - 0.5f),
			(short)fast_ftol(render.camera.position.z / DETAIL_OBJECT_CELL_SIZE - 0.5f)
		};

		if (data->valid)
		{
			rasterizer_detail_objects_begin();

			if (coord.x != runtime_data->last_cell_coord.x ||
				coord.y != runtime_data->last_cell_coord.y ||
				coord.z != runtime_data->last_cell_coord.z ||
				!runtime_data->last_cell_coord.initialized ||
				TEST_FLAG(data->valid, _structure_detail_object_data_dirty_bit))
			{
				short i;
				short j;
				short layer_index;
				short layer_count;
				unsigned long valid_layers = 0;
				short next_cell_index[MAXIMUM_DETAIL_OBJECT_LAYERS_PER_STRUCTURE] = { 0 };

				data->valid = TRUE;
				runtime_data->last_cell_coord = coord;
				runtime_data->last_cell_coord.initialized = TRUE;

				for (i = -1; i <= 1; i++)
				{
					for (j = -1; j <= 1; j++)
					{
						struct detail_object_cell_definition *begin = TAG_BLOCK_GET_ELEMENT(&data->cells, 0, struct detail_object_cell_definition);
						struct detail_object_cell_definition *end = TAG_BLOCK_GET_ELEMENT(&data->cells, data->cells.count - 1, struct detail_object_cell_definition) + 1;
						struct detail_object_cell_definition *lower_bound_cell = NULL;
						struct detail_object_cell_definition *upper_bound_cell = NULL;
						struct detail_object_cell_coord key = { coord.x - i, coord.y - j, coord.z };

						key.z--;
						lower_bound_cell = get_lower_bound_cell(begin, end, &key);
						key.z += 3;
						upper_bound_cell = get_upper_bound_cell(begin, end, &key) - 1;
						key.z = coord.z;

						if (lower_bound_cell->cell_x == key.x &&
							lower_bound_cell->cell_y == key.y &&
							upper_bound_cell->cell_x == key.x &&
							upper_bound_cell->cell_y == key.y)
						{
							struct detail_object_cell_definition *cell;

							match_assert("c:\\halo\\SOURCE\\structures\\structure_detail_objects.c", 198, lower_bound_cell<=upper_bound_cell);
							upper_bound_cell++;

							for (cell = lower_bound_cell; cell < upper_bound_cell; cell++)
							{
								if (abs(key.z - cell->cell_z) <= 1)
								{
									short layer;
									long detail_object_index = 0;
									short count_index = 0;

									valid_layers |= cell->valid_layers;

									for (layer = 0; layer < MAXIMUM_DETAIL_OBJECT_LAYERS_PER_STRUCTURE; layer++)
									{
										if (TEST_FLAG(cell->valid_layers, layer))
										{
											struct detail_object_cell_data *cell_data = &runtime_data->cells[layer][next_cell_index[layer]++];

											cell_data->cell_x = cell->cell_x;
											cell_data->cell_y = cell->cell_y;
											cell_data->cell_z = cell->cell_z + cell->offset_z * (1.f / DETAIL_OBJECT_CELL_GRANULARITY);
											cell_data->first_detail_object_index = detail_object_index + cell->start_index;
											cell_data->detail_object_count = *TAG_BLOCK_GET_ELEMENT(&data->detail_objects_counts, cell->count_index + count_index, word);
											cell_data->z_reference_vector = data->detail_object_z_reference_vectors.count ?
												TAG_BLOCK_GET_ELEMENT(&data->detail_object_z_reference_vectors, cell->count_index + count_index, real_vector4d) :
												&detail_object_global_runtime_data->default_z_reference_vector;
											detail_object_index += cell_data->detail_object_count;
											count_index++;
										}
									}
								}
							}
						}
					}
				}

				layer_count = 0;
				runtime_data->view_data.layer_count = 0;
				runtime_data->view_data.layers = runtime_data->layers;

				for (layer_index = 0; layer_index < MAXIMUM_DETAIL_OBJECT_LAYERS_PER_STRUCTURE; layer_index++)
				{
					if (TEST_FLAG(valid_layers, layer_index) && next_cell_index[layer_index])
					{
						struct detail_object_layer_data *layer = &runtime_data->layers[layer_count++];

						layer->cells = runtime_data->cells[layer_index];
						layer->cell_count = next_cell_index[layer_index];
						layer->collection_definition_index = layer_index;
						runtime_data->view_data.layer_count++;
					}
				}

				rasterizer_detail_objects_rebuild_vertices(&runtime_data->view_data);
			}

			rasterizer_detail_objects_draw(&runtime_data->view_data);
			rasterizer_detail_objects_end();
		}
	}

	return;
}

void detail_object_offset(
	real offset)
{
	final_offset = offset;
	fudge_vector = TRUE;
	fudge_offset = offset - fudge_offset;

	return;
}

void render_debug_detail_objects(
	void)
{
	if (local_player_count() == 1 && render.local_player_index != NONE && debug_detail_objects)
	{
		struct structure_detail_object_data *data = global_structure_bsp_get()->detail_object_data.count ?
			TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->detail_object_data, 0, struct structure_detail_object_data) :
			NULL;
		struct detail_object_runtime_data *runtime_data = get_local_player_datum(0);

		if (debug_detail_objects)
		{
			short layer_index;

			for (layer_index = 0; layer_index < runtime_data->view_data.layer_count; layer_index++)
			{
				short cell_index;
				struct detail_object_layer_data *layer = &runtime_data->layers[layer_index];

				for (cell_index = 0; cell_index < layer->cell_count; cell_index++)
				{
					long detail_object_index;
					struct detail_object_cell_data *cell = &layer->cells[cell_index];
					boolean out_of_bounds = FALSE;

					if (fudge_vector)
					{
						((real_vector4d *)cell->z_reference_vector)->l += fudge_offset / DETAIL_OBJECT_CELL_SIZE;
					}

					for (detail_object_index = 0; detail_object_index < cell->detail_object_count; detail_object_index++)
					{
						struct detail_object *detail_object = TAG_BLOCK_GET_ELEMENT(
							&data->detail_objects,
							cell->first_detail_object_index + detail_object_index,
							struct detail_object);
						real_vector4d point_vector;
						real_point3d obj_pos;

						point_vector.i = detail_object->position[0];
						point_vector.j = detail_object->position[1];
						point_vector.k = detail_object->position[2];
						point_vector.l = DETAIL_OBJECT_CELL_GRANULARITY;
						obj_pos.x = calculate_world_from_cell_index_and_offset(cell->cell_x, detail_object->position[0]);
						obj_pos.y = calculate_world_from_cell_index_and_offset(cell->cell_y, detail_object->position[1]);
						obj_pos.z = calculate_world_from_cell_index_and_offset(cell->cell_z, dot_product4d(&point_vector, cell->z_reference_vector));

						if (obj_pos.z > (cell->cell_z + 1.f) * DETAIL_OBJECT_CELL_SIZE || cell->cell_z * DETAIL_OBJECT_CELL_SIZE > obj_pos.z)
						{
							out_of_bounds = TRUE;
						}

						render_debug_point(TRUE, &obj_pos, DEBUG_DETAIL_OBJECT_POINT_SIZE, global_real_argb_red);
					}

					{
						real_rectangle3d box;

						box.x0 = (real)(cell->cell_x * DETAIL_OBJECT_CELL_SIZE);
						box.x1 = (real)(cell->cell_x * DETAIL_OBJECT_CELL_SIZE + DETAIL_OBJECT_CELL_SIZE);
						box.y0 = (real)(cell->cell_y * DETAIL_OBJECT_CELL_SIZE);
						box.y1 = (real)(cell->cell_y * DETAIL_OBJECT_CELL_SIZE + DETAIL_OBJECT_CELL_SIZE);
						box.z0 = cell->cell_z * DETAIL_OBJECT_CELL_SIZE;
						box.z1 = (cell->cell_z + 1.f) * DETAIL_OBJECT_CELL_SIZE;
						render_debug_box_outline(TRUE, &box, global_real_argb_blue);

						if (out_of_bounds)
						{
							real_argb_color color = *global_real_argb_grey;

							color.alpha = DEBUG_DETAIL_OBJECT_ALPHA;
							render_debug_box_outline(TRUE, &box, &color);
						}
					}
				}
			}
		}

		fudge_vector = FALSE;
		fudge_offset = final_offset;
	}

	return;
}

/* ---------- private code */

static boolean key_compare_cells_lower_bound(
	struct detail_object_cell_coord const *key,
	struct detail_object_cell_definition const *cell)
{
	return cell->cell_x < key->x ||
		cell->cell_x == key->x && (cell->cell_y < key->y || cell->cell_y == key->y && cell->cell_z < key->z);
}

static boolean key_compare_cells_upper_bound(
	struct detail_object_cell_coord const *key,
	struct detail_object_cell_definition const *cell)
{
	return cell->cell_x > key->x ||
		cell->cell_x == key->x && (cell->cell_y > key->y || cell->cell_y == key->y && cell->cell_z > key->z);
}

static struct detail_object_cell_definition *get_lower_bound_cell(
	struct detail_object_cell_definition *begin,
	struct detail_object_cell_definition *end,
	struct detail_object_cell_coord const *key)
{
	long count = end - begin;

	while (count > 0)
	{
		long half = count / 2;
		struct detail_object_cell_definition *middle = begin + half;

		if (key_compare_cells_lower_bound(key, middle))
		{
			begin = middle + 1;
			count -= half + 1;
		}
		else
		{
			count = half;
		}
	}

	return begin;
}

static struct detail_object_cell_definition *get_upper_bound_cell(
	struct detail_object_cell_definition *begin,
	struct detail_object_cell_definition *end,
	struct detail_object_cell_coord const *key)
{
	long count = end - begin;

	while (count > 0)
	{
		long half = count / 2;
		struct detail_object_cell_definition *middle = begin + half;

		if (!key_compare_cells_upper_bound(key, middle))
		{
			begin = middle + 1;
			count -= half + 1;
		}
		else
		{
			count = half;
		}
	}

	return begin;
}
