/*
FLAGS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "flags.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "game_state.h"
#include "shaders.h"
#include "physics.h"
#include "point_physics.h"

/* ---------- constants */

enum
{
	MAXIMUM_FLAG_VERTEX_AREA = 225,
	MAXIMUM_FLAG_CELL_AREA = 196,
	MAXIMUM_FLAG_HEIGHT = 40,
	FLAG_MATERIAL_COUNT = 1,
};

enum
{
	_tesselate_both = 0,
	_tesselate_none,
	_tesselate_top_left,
	_tesselate_bottom_left,
	_tesselate_top_right,
	_tesselate_bottom_right,
};

enum
{
	MAXIMUM_NUMBER_OF_ESTIMATED_POSITIONS = 3,
	MAXIMUM_FLAGS_PER_MAP = 2,
};

#define FLAG_RENDER_CATCH_UP_UPDATE_THRESHOLD 5				/* fake name */
#define FLAG_RENDER_CATCH_UP_DT 5.f							/* fake name */
#define FLAG_VERTEX_RADIUS 0.02f							/* fake name */

/* ---------- macros */

#define flag_get(index) DATUM_GET(flag_data, index, struct flag_datum) /* fake name */
#define flag_try_and_get(index) ((struct flag_datum *)datum_try_and_get(flag_data, (index))) /* fake name */

/* ---------- structures */

struct flag_vertex_datum
{
	real_point3d p;
	real_vector3d v;
};

struct flag_cell_datum
{
	short tesselation;
};

struct flag_datum
{
	short identifier;
	boolean error;
	boolean initialized;
	boolean update_state;
	short updates_since_last_render;
	long object_index;
	long definition_index;
	real_point3d first_attachment;
	struct flag_vertex_datum vertices[MAXIMUM_FLAG_VERTEX_AREA];
	struct flag_cell_datum cells[MAXIMUM_FLAG_CELL_AREA];
};

/* ---------- prototypes */

struct flag_vertex_datum *flag_datum_get_vertex(struct flag_datum *flag, struct flag_definition const *definition, short x, short y);
struct flag_cell_datum *flag_datum_get_cell(struct flag_datum *flag, struct flag_definition const *definition, short x, short y);
void flag_set_attachment_shape(struct flag_definition const *flag_definition, struct flag_datum *flag);
void flag_set_trailing_shape(struct flag_definition const *flag_definition, struct flag_datum *flag);
void flag_tesselate_region(struct flag_definition const *flag_definition, struct flag_datum *flag, short x, short y, short size, short tesselation);
void flag_update(struct flag_datum *flag, struct flag_definition const *flag_definition, real time);
void flag_update_attachment(struct flag_datum *flag, struct flag_definition const *flag_definition, struct location *attachment_location, real_point3d *attachment_points, real_point3d *attachment_force_points, short *attachment_y, short *y_attachments);
void flag_render_proper(struct flag_datum *flag, struct flag_definition const *flag_definition, struct render_lighting const *lighting, struct render_animation const *animation);

/* ---------- globals */

struct data_array *flag_data;

/* ---------- public code */

struct flag_vertex_datum *flag_datum_get_vertex(
	struct flag_datum *flag,
	struct flag_definition const *definition,
	short x,
	short y)
{
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 96, flag && definition);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 97, x>=0 && x<definition->width);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 98, y>=0 && y<definition->height);

	return &flag->vertices[x * definition->height + y];
}

struct flag_cell_datum *flag_datum_get_cell(
	struct flag_datum *flag,
	struct flag_definition const *definition,
	short x,
	short y)
{
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 109, flag && definition);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 110, x>=0 && x<definition->width-1);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 111, y>=0 && y<definition->height-1);

	return &flag->cells[x * (definition->height - 1) + y];
}

void flags_initialize(
	void)
{
	flag_data = game_state_data_new("flag", MAXIMUM_FLAGS_PER_MAP, sizeof(struct flag_datum));

	if (!flag_data)
	{
		error(_error_immediate, "couldn't allocate flag globals");
	}

	return;
}

void flags_initialize_for_new_map(
	void)
{
	data_make_valid(flag_data);

	return;
}

void flags_dispose_from_old_map(
	void)
{
	data_make_invalid(flag_data);

	return;
}

void flags_dispose(
	void)
{
	if (flag_data)
	{
		flag_data = NULL;
	}

	return;
}

long flag_new(
	long flag_definition_index)
{
	struct scenario *scenario = global_scenario_get();
	long flag_index = NONE;

	if (flag_definition_index != NONE)
	{
		struct flag_definition *definition = flag_definition_get(flag_definition_index);

		flag_index = datum_new(flag_data);

		if (flag_index != NONE)
		{
			struct flag_datum *flag = flag_get(flag_index);

			if (definition->width * definition->height < MAXIMUM_FLAG_VERTEX_AREA &&
				definition->width < MAXIMUM_FLAG_HEIGHT &&
				definition->shader_blue.index != NONE)
			{
				short x;
				short y;

				flag->error = FALSE;
				flag->initialized = FALSE;
				flag->object_index = NONE;
				flag->definition_index = flag_definition_index;
				flag->first_attachment.z = 0.f;
				flag->first_attachment.y = 0.f;
				flag->first_attachment.x = 0.f;

				for (x = 0; x < definition->width; x++)
				{
					for (y = 0; y < definition->height; y++)
					{
						struct flag_vertex_datum *vertex = flag_datum_get_vertex(flag, definition, x, y);

						vertex->p = *global_origin3d;
						vertex->v = *global_zero_vector3d;

						if (x < definition->width - 1 && y < definition->height - 1)
						{
							struct flag_cell_datum *cell = flag_datum_get_cell(flag, definition, x, y);

							cell->tesselation = _tesselate_both;
						}
					}
				}

				flag_set_attachment_shape(definition, flag);
				flag_set_trailing_shape(definition, flag);
			}
			else
			{
				flag->error = TRUE;
			}
		}
	}

	return flag_index;
}

void flag_set_attachment_shape(
	struct flag_definition const *flag_definition,
	struct flag_datum *flag)
{
	if (flag_definition->attached_edge_shape)
	{
		short attachment_index;
		short y;

		for (attachment_index = 0, y = 0;
			attachment_index < flag_definition->attachment_points.count && y < flag_definition->height;
			attachment_index++)
		{
			struct flag_attachment_point *attachment_point = TAG_BLOCK_GET_ELEMENT(
				&flag_definition->attachment_points,
				attachment_index,
				struct flag_attachment_point);
			short even_height = PIN(attachment_point->height_to_next_attachment, 0, flag_definition->height - y) & ~MASK(1);
			short half_height = even_height >> 1;

			flag_tesselate_region(
				flag_definition,
				flag,
				0,
				y,
				half_height,
				_tesselate_top_right);
			flag_tesselate_region(
				flag_definition,
				flag,
				0,
				y + half_height,
				half_height,
				_tesselate_bottom_right);
			y += even_height;
		}
	}

	return;
}

void flag_set_trailing_shape(
	struct flag_definition const *flag_definition,
	struct flag_datum *flag)
{
	if (flag_definition->trailing_edge_shape)
	{
		short size = flag_definition->trailing_edge_shape == _trailing_edge_shape_trapezoid_short_top ||
			flag_definition->trailing_edge_shape == _trailing_edge_shape_trapezoid_short_bottom ?
			flag_definition->height - 1 :
			flag_definition->height >> 1;
		short x = FLOOR(flag_definition->width - size + flag_definition->trailing_edge_offset - 1, 0);

		if (flag_definition->trailing_edge_shape == _trailing_edge_shape_trapezoid_short_top)
		{
			flag_tesselate_region(flag_definition, flag, x, 0, size, _tesselate_bottom_left);
		}
		else if (flag_definition->trailing_edge_shape == _trailing_edge_shape_trapezoid_short_bottom)
		{
			flag_tesselate_region(flag_definition, flag, x, 0, size, _tesselate_top_left);
		}
		else if (flag_definition->trailing_edge_shape == _trailing_edge_shape_concave_triangular)
		{
			flag_tesselate_region(flag_definition, flag, x, 0, size, _tesselate_top_left);
			flag_tesselate_region(flag_definition, flag, x, size, size, _tesselate_bottom_left);
		}
		else if (flag_definition->trailing_edge_shape == _trailing_edge_shape_convex_triangular)
		{
			flag_tesselate_region(flag_definition, flag, x, 0, size, _tesselate_bottom_left);
			flag_tesselate_region(flag_definition, flag, x, size, size, _tesselate_top_left);
		}
	}

	return;
}

void flag_tesselate_region(
	struct flag_definition const *flag_definition,
	struct flag_datum *flag,
	short x,
	short y,
	short size,
	short tesselation)
{
	short i;

	for (i = x; i < x + size; i++)
	{
		short j;

		for (j = y; j < y + size; j++)
		{
			if (i >= 0 && j >= 0 && i < flag_definition->width - 1 && j < flag_definition->height - 1)
			{
				short u = (tesselation == _tesselate_top_right || tesselation == _tesselate_bottom_right) ? i - x : size - (i - x) - 1;
				short v = (tesselation == _tesselate_top_right || tesselation == _tesselate_top_left) ? j - y : size - (j - y) - 1;
				struct flag_cell_datum *cell = flag_datum_get_cell(flag, flag_definition, i, j);

				if (u == v)
				{
					cell->tesselation = tesselation;
				}
				else if (u > v)
				{
					cell->tesselation = _tesselate_both;
				}
				else
				{
					cell->tesselation = _tesselate_none;
				}
			}
		}
	}

	return;
}

void flag_delete(
	long flag_index)
{
	datum_delete(flag_data, flag_index);

	return;
}

void flag_render(
	long object_index,
	long flag_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	struct object_datum *object = object_get(object_index);
	struct flag_datum *flag = flag_get(flag_index);
	struct flag_definition *definition = flag_definition_get(flag->definition_index);

	flag->object_index = object_index;

	if (flag->updates_since_last_render > FLAG_RENDER_CATCH_UP_UPDATE_THRESHOLD || !flag->initialized)
	{
		flag_update(flag, definition, FLAG_RENDER_CATCH_UP_DT);
		flag->initialized = TRUE;
	}

	flag->updates_since_last_render = 0;

	if (!flag->error)
	{
		flag_render_proper(flag, definition, lighting, animation);
	}

	return;
}

void flags_update(
	real dt)
{
	long flag_index;

	for (flag_index = data_next_index(flag_data, NONE);
		flag_index != NONE;
		flag_index = data_next_index(flag_data, flag_index))
	{
		struct flag_datum *flag = flag_get(flag_index);
		struct flag_definition *definition = flag_definition_get(flag->definition_index);

		flag->updates_since_last_render++;

		if (flag->object_index != NONE &&
			flag->updates_since_last_render < FLAG_RENDER_CATCH_UP_UPDATE_THRESHOLD &&
			dt != 0.f)
		{
			flag_update(flag, definition, dt);
		}
	}

	return;
}

void flag_update(
	struct flag_datum *flag,
	struct flag_definition const *flag_definition,
	real time)
{
	short x;
	short forward = flag->update_state == 0;
	struct location attachment_location;
	real_point3d attachment_points[MAXIMUM_NUMBER_OF_ATTACHMENT_POINTS];
	real_point3d attachment_force_points[MAXIMUM_FLAG_HEIGHT];
	short attachment_y[MAXIMUM_NUMBER_OF_ATTACHMENT_POINTS];
	short y_attachments[MAXIMUM_FLAG_HEIGHT];
	struct object_datum *object = object_get(flag->object_index);
	boolean underwater;
	short weather_palette_index;

	flag_update_attachment(
		flag,
		flag_definition,
		&attachment_location,
		attachment_points,
		attachment_force_points,
		attachment_y,
		y_attachments);
	underwater = scenario_location_underwater(
		&attachment_location,
		&attachment_points[flag_definition->attachment_points.count - 1],
		&weather_palette_index);

	if (!flag->error)
	{
		point2d const parent_deltas[MAXIMUM_NUMBER_OF_ESTIMATED_POSITIONS] = { { -1, 0 }, { 0, 1 }, { 0, -1 } };
		real parent_distances[MAXIMUM_NUMBER_OF_ESTIMATED_POSITIONS];
		short i;

		for (i = 0; i < MAXIMUM_NUMBER_OF_ESTIMATED_POSITIONS; i++)
		{
			real_vector2d parent_vector;

			parent_vector.i = parent_deltas[i].x * flag_definition->cell_width_scale;
			parent_vector.j = parent_deltas[i].y * flag_definition->cell_height_scale;
			parent_distances[i] = magnitude2d(&parent_vector);
		}

		for (x = 0; x < flag_definition->width; x++)
		{
			short y;
			short step = forward ? 1 : -1;

			for (y = forward ? 0 : flag_definition->height - 1;
				forward ? y < flag_definition->height : y > 0;
				y += step)
			{
				short attachment_index = y_attachments[y];
				struct flag_vertex_datum *vertex = flag_datum_get_vertex(flag, flag_definition, x, y);
				real_vector3d turbulence;
				real wind_scale;
				real_point3d new_position;
				struct location new_location;
				short estimated_position_count = 0;
				real_point3d estimated_positions[MAXIMUM_NUMBER_OF_ESTIMATED_POSITIONS];
				unsigned long physics_flags = FLAG(_point_physics_ignore_position_bit);

				if (!underwater)
				{
					wind_scale = flag_definition->wind_noise *
						point_physics_definition_get(flag_definition->physics.index)->air_friction *
						0.02f *
						0.02f;
				}
				else
				{
					wind_scale = 0.4f *
						flag_definition->wind_noise *
						point_physics_definition_get(flag_definition->physics.index)->water_friction *
						0.02f *
						0.02f;
					SET_FLAG(physics_flags, _point_physics_ignore_position_under_water_bit, TRUE);
				}

				local_random_direction3d(&turbulence);
				turbulence.i *= wind_scale;
				turbulence.j *= wind_scale;
				turbulence.k *= wind_scale;
				new_position = vertex->p;
				new_location = attachment_location;
				point_physics_update(
					physics_flags,
					point_physics_definition_get(flag_definition->physics.index),
					&new_location,
					weather_palette_index,
					&new_position,
					&vertex->v,
					&turbulence,
					NULL,
					NULL,
					FLAG_VERTEX_RADIUS,
					time);

				if (!x && attachment_index != NONE)
				{
					new_position = attachment_points[attachment_index];
				}
				else
				{
					short i;

					for (i = 0; i < MAXIMUM_NUMBER_OF_ESTIMATED_POSITIONS; i++)
					{
						short parent_x = parent_deltas[i].x + x;
						short parent_y = parent_deltas[i].y + y;

						if (parent_x >= 0 &&
							parent_x < flag_definition->width &&
							parent_y >= 0 &&
							parent_y < flag_definition->height)
						{
							struct flag_vertex_datum *parent_vertex = flag_datum_get_vertex(
								flag,
								flag_definition,
								parent_x,
								parent_y);
							real_vector3d parent;

							parent.i = new_position.x - parent_vertex->p.x;
							parent.j = new_position.y - parent_vertex->p.y;
							parent.k = new_position.z - parent_vertex->p.z;
							normalize3d(&parent);
							estimated_positions[estimated_position_count].x = parent_distances[i] * parent.i + parent_vertex->p.x;
							estimated_positions[estimated_position_count].y = parent_distances[i] * parent.j + parent_vertex->p.y;
							estimated_positions[estimated_position_count].z = parent_distances[i] * parent.k + parent_vertex->p.z;
							estimated_position_count++;
						}
					}

					{
						short estimated_position_index;
						real total_weight = 0.f;

						new_position.z = 0.f;
						new_position.y = new_position.z;
						new_position.x = new_position.y;

						for (estimated_position_index = 0;
							estimated_position_index < estimated_position_count;
							estimated_position_index++)
						{
							real weight = (x && !estimated_position_index) ? 4.f : 1.f;

							new_position.x += weight * estimated_positions[estimated_position_index].x;
							new_position.y += weight * estimated_positions[estimated_position_index].y;
							new_position.z += weight * estimated_positions[estimated_position_index].z;
							total_weight += weight;
						}

						if (!x)
						{
							real weight = 4.f;

							new_position.x += weight * attachment_force_points[y].x;
							new_position.y += weight * attachment_force_points[y].y;
							new_position.z += weight * attachment_force_points[y].z;
							total_weight += weight;
						}

						total_weight = 1.f / total_weight;
						new_position.x *= total_weight;
						new_position.y *= total_weight;
						new_position.z *= total_weight;
					}
				}

				vertex->v.i = (new_position.x - vertex->p.x) / time;
				vertex->v.j = (new_position.y - vertex->p.y) / time;
				vertex->v.k = (new_position.z - vertex->p.z) / time;
				vertex->p = new_position;
			}
		}
	}

	return;
}

void flag_update_attachment(
	struct flag_datum *flag,
	struct flag_definition const *flag_definition,
	struct location *attachment_location,
	real_point3d *attachment_points,
	real_point3d *attachment_force_points,
	short *attachment_y,
	short *y_attachments)
{
	real const maximum_displacement = 1.f;
	short attachment_index;
	short y;

	for (attachment_index = 0; attachment_index < flag_definition->attachment_points.count; attachment_index++)
	{
		struct flag_attachment_point *attachment_point = TAG_BLOCK_GET_ELEMENT(
			&flag_definition->attachment_points,
			attachment_index,
			struct flag_attachment_point);
		struct object_marker marker;

		object_get_marker_by_name(flag->object_index, attachment_point->marker_name, &marker, 1);
		attachment_points[attachment_index] = marker.matrix.position;
	}

	scenario_location_from_point(attachment_location, attachment_points);

	if (!flag->error)
	{
		for (y = 0; y < flag_definition->height; y++)
		{
			y_attachments[y] = NONE;
		}

		for (attachment_index = 0, y = 0;
			attachment_index < flag_definition->attachment_points.count && y < flag_definition->height;
			attachment_index++)
		{
			struct flag_attachment_point *attachment_point = TAG_BLOCK_GET_ELEMENT(
				&flag_definition->attachment_points,
				attachment_index,
				struct flag_attachment_point);
			short height = PIN(attachment_point->height_to_next_attachment, 0, flag_definition->height - y) & ~MASK(1);
			short first_y = y;
			short last_y = first_y + height;

			attachment_y[attachment_index] = y;
			y_attachments[y] = attachment_index;

			for (; y <= last_y; y++)
			{
				real t = ((real)y - (real)first_y) / ((real)last_y - (real)first_y);
				real one_minus_t = 1.f - t;

				attachment_force_points[y].x = one_minus_t * attachment_points[attachment_index].x +
					t * attachment_points[attachment_index + 1].x;
				attachment_force_points[y].y = one_minus_t * attachment_points[attachment_index].y +
					t * attachment_points[attachment_index + 1].y;
				attachment_force_points[y].z = one_minus_t * attachment_points[attachment_index].z +
					t * attachment_points[attachment_index + 1].z;
			}

			y--;
		}

		{
			real_vector3d attachment_displacement;

			attachment_displacement.i = attachment_points[0].x - flag->first_attachment.x;
			attachment_displacement.j = attachment_points[0].y - flag->first_attachment.y;
			attachment_displacement.k = attachment_points[0].z - flag->first_attachment.z;

			if (abs(attachment_displacement.i) > maximum_displacement ||
				abs(attachment_displacement.j) > maximum_displacement ||
				abs(attachment_displacement.k) > maximum_displacement)
			{
				short x;
				short y;

				for (x = 0; x < flag_definition->width; x++)
				{
					for (y = 0; y < flag_definition->height; y++)
					{
						struct flag_vertex_datum *vertex = flag_datum_get_vertex(flag, flag_definition, x, y);

						vertex->p.x += attachment_displacement.i;
						vertex->p.y += attachment_displacement.j;
						vertex->p.z += attachment_displacement.k;
					}
				}
			}

			flag->first_attachment = attachment_points[0];
		}
	}

	return;
}

void flag_render_proper(
	struct flag_datum *flag,
	struct flag_definition const *flag_definition,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	struct object_datum *object = object_get(flag->object_index);
	long shader_index = object->object.owner_team_index ? flag_definition->shader_blue.index : flag_definition->shader_red.index;
	short maximum_triangles;
	long dynamic_triangle_buffer_index;
	long dynamic_vertex_buffer_index;

	if (shader_index == NONE)
	{
		shader_index = flag_definition->shader_blue.index;
	}

	rasterizer_globals.current_lock_operation = _rasterizer_lock_flag;
	maximum_triangles = (flag_definition->width - 1) * (flag_definition->height - 1) * 2;
	dynamic_triangle_buffer_index = rasterizer_dynamic_triangles_new(maximum_triangles);
	dynamic_vertex_buffer_index = rasterizer_dynamic_vertices_new(
		_rasterizer_vertex_type_model_compressed,
		flag_definition->height * flag_definition->width);

	if (dynamic_triangle_buffer_index != NONE && dynamic_vertex_buffer_index != NONE)
	{
		struct object_datum *parent_object = object_get(flag->object_index);
		struct object_definition *object_definition = object_definition_get(parent_object->definition_index);
		struct model_vertex_compressed *vertices = rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);
		struct rasterizer_triangle *triangles = rasterizer_dynamic_triangles_lock(dynamic_triangle_buffer_index);
		short triangle_count = 0;

		{
			struct model_vertex_uncompressed uncompressed_vertex;
			short x;
			short y;
			short vertex_index;
			real u_scale = 1.f / (flag_definition->width - 1);
			real v_scale = 1.f / (flag_definition->height - 1);

			memset(&uncompressed_vertex, 0, sizeof(uncompressed_vertex));

			for (x = 0, vertex_index = x; x < flag_definition->width; x++)
			{
				for (y = 0; y < flag_definition->height; y++, vertex_index++)
				{
					struct flag_vertex_datum *vertex = flag_datum_get_vertex(flag, flag_definition, x, y);
					short x0 = x < flag_definition->width - 1 ? x : x - 1;
					short y0 = y < flag_definition->height - 1 ? y : y - 1;
					struct flag_vertex_datum *vertex0 = flag_datum_get_vertex(flag, flag_definition, x0, y0);
					struct flag_vertex_datum *vertex1 = flag_datum_get_vertex(flag, flag_definition, x0 + 1, y0);
					struct flag_vertex_datum *vertex2 = flag_datum_get_vertex(flag, flag_definition, x0, y0 + 1);
					real_vector3d v1;
					real_vector3d v2;

					vector_from_points3d(&vertex0->p, &vertex1->p, &v1);
					vector_from_points3d(&vertex0->p, &vertex2->p, &v2);
					cross_product3d(&v1, &v2, &uncompressed_vertex.normal);
					normalize3d(&uncompressed_vertex.normal);
					uncompressed_vertex.position.x = vertex->p.x;
					uncompressed_vertex.position.y = vertex->p.y;
					uncompressed_vertex.position.z = vertex->p.z;
					uncompressed_vertex.texcoord.x = x * u_scale;
					uncompressed_vertex.texcoord.y = y * v_scale;
					rasterizer_geometry_compress_vertices(
						_rasterizer_vertex_type_model_uncompressed,
						1,
						&vertices[vertex_index],
						sizeof(*vertices),
						&uncompressed_vertex,
						sizeof(uncompressed_vertex));
				}
			}
		}

		{
			short x;
			short y;
			short i;

			for (x = 0, i = x; x < flag_definition->width - 1; x++)
			{
				for (y = 0; y < flag_definition->height - 1; y++)
				{
					short tesselation = flag_datum_get_cell(flag, flag_definition, x, y)->tesselation;

					switch (tesselation)
					{
					case _tesselate_both:
						match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 742, i<maximum_triangles);
						triangles[i].vertex_indices[0] = y + flag_definition->height * x;
						triangles[i].vertex_indices[1] = y + flag_definition->height * (x + 1);
						triangles[i].vertex_indices[2] = flag_definition->height * x + y + 1;
						i++;
						match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 747, i<maximum_triangles);
						triangles[i].vertex_indices[0] = flag_definition->height * x + y + 1;
						triangles[i].vertex_indices[1] = y + flag_definition->height * (x + 1);
						triangles[i].vertex_indices[2] = flag_definition->height * (x + 1) + y + 1;
						i++;
						break;
					case _tesselate_none:
						break;
					case _tesselate_top_left:
						match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 754, i<maximum_triangles);
						triangles[i].vertex_indices[0] = y + flag_definition->height * x;
						triangles[i].vertex_indices[1] = flag_definition->height * x + y + 1;
						triangles[i].vertex_indices[2] = y + flag_definition->height * (x + 1);
						i++;
						break;
					case _tesselate_bottom_left:
						match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 761, i<maximum_triangles);
						triangles[i].vertex_indices[0] = y + flag_definition->height * x;
						triangles[i].vertex_indices[1] = flag_definition->height * x + y + 1;
						triangles[i].vertex_indices[2] = flag_definition->height * (x + 1) + y + 1;
						i++;
						break;
					case _tesselate_top_right:
						match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 768, i<maximum_triangles);
						triangles[i].vertex_indices[0] = y + flag_definition->height * x;
						triangles[i].vertex_indices[1] = flag_definition->height * (x + 1) + y + 1;
						triangles[i].vertex_indices[2] = y + flag_definition->height * (x + 1);
						i++;
						break;
					case _tesselate_bottom_right:
						match_assert("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 775, i<maximum_triangles);
						triangles[i].vertex_indices[0] = flag_definition->height * x + y + 1;
						triangles[i].vertex_indices[1] = flag_definition->height * (x + 1) + y + 1;
						triangles[i].vertex_indices[2] = y + flag_definition->height * (x + 1);
						i++;
						break;
					default:
						match_halt("c:\\halo\\SOURCE\\objects\\widgets\\flags.c", 784);
					}
				}
			}

			triangle_count = i;
		}

		rasterizer_dynamic_triangles_unlock(dynamic_triangle_buffer_index);
		rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);

		{
			struct shader *shader = shader_definition_get(shader_index);
			struct rasterizer_model_begin_parameters parameters;
			struct flag_vertex_datum *vertex00 = flag_datum_get_vertex(flag, flag_definition, 0, 0);
			struct flag_vertex_datum *vertex10 = flag_datum_get_vertex(
				flag,
				flag_definition,
				flag_definition->width - 1,
				0);
			struct flag_vertex_datum *vertex01 = flag_datum_get_vertex(
				flag,
				flag_definition,
				0,
				flag_definition->height - 1);
			struct flag_vertex_datum *vertex11 = flag_datum_get_vertex(
				flag,
				flag_definition,
				flag_definition->width - 1,
				flag_definition->height - 1);
			real_point3d centroid;

			centroid.x = (vertex00->p.x + vertex10->p.x + vertex01->p.x + vertex11->p.x) / 4.f;
			centroid.y = (vertex00->p.y + vertex10->p.y + vertex01->p.y + vertex11->p.y) / 4.f;
			centroid.z = (vertex00->p.z + vertex10->p.z + vertex01->p.z + vertex11->p.z) / 4.f;
			memset(&parameters, 0, sizeof(parameters));
			parameters.unique_id = 1;
			parameters.skinning.node_matrix_count = 1;
			parameters.skinning.node_matrices = (real_matrix4x3 *)global_identity4x3;
			parameters.lighting = *lighting;
			parameters.animation = *animation;
			parameters.centroid = centroid;
			parameters.base_map_scale.j = 1.f;
			parameters.base_map_scale.i = parameters.base_map_scale.j;
			rasterizer_profile_enable(FALSE);
			rasterizer_models_begin(FALSE);
			rasterizer_model_begin(&parameters, FALSE);

			if (shader_type_is_transparent(shader->base.type))
			{
				rasterizer_model_transparent_geometry_submit(
					shader,
					0,
					NULL,
					dynamic_triangle_buffer_index,
					triangle_count,
					NULL,
					dynamic_vertex_buffer_index,
					&centroid,
					NULL);
			}
			else
			{
				rasterizer_model_draw(
					shader,
					0,
					NULL,
					dynamic_triangle_buffer_index,
					triangle_count,
					NULL,
					dynamic_vertex_buffer_index);
			}

			rasterizer_model_end();
			rasterizer_models_end();
			rasterizer_profile_enable(TRUE);
		}
	}

	rasterizer_dynamic_triangles_delete(dynamic_triangle_buffer_index);
	rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);
	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}

/* ---------- private code */
