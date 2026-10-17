/*
RENDER_SPRITE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "render_debug.h"
#include "objects.h"
#include "collision_bsp.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "texture_cache.h"

/* ---------- constants */

enum
{
	_build_sprites_valid_bit = NUMBER_OF_BUILD_SPRITES_FLAGS,
	NUMBER_OF_BUILD_SPRITES_INTERNAL_FLAGS
};

enum
{
	NUMBER_OF_VERTICES_PER_SPRITE = 4,	/* fake name */
	DYNAMIC_TRIANGLE_QUAD_LIST = -4,	/* fake name */
	MAXIMUM_BIG_SPRITES = 10,			/* fake name */
};

#define PERPENDICULAR_SPRITE_PARALLEL_DOT_SQUARED 0.99f	/* fake name */
#define BIG_SPRITE_SCREEN_COVERAGE 0.5f					/* fake name */
#define MINIMUM_ROTATIONAL_SPRITE_BLEND 0.05f			/* fake name */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void build_sprite_transform_origin_and_direction(struct build_sprite_data const *data, unsigned long flags, real_point3d const *untransformed_origin, real_vector3d const *untransformed_direction, real_point3d *transformed_origin, real_vector3d *transformed_direction);
static void build_sprite_compute_basis(struct build_sprite_data const *data, short mode, unsigned long flags, real_point3d const *origin, real_vector3d const *direction, real_matrix4x3 *basis);
static void build_sprite_compute_scale(struct build_sprite_data const *data, short mode, unsigned long flags, real_point3d const *origin, struct bitmap_data const *bitmap, real *scale);
__inline real build_sprite_compute_vertex_fade(short fade_mode, real_point3d const *viewer_point, real_vector3d const *viewer_normal);
static short build_sprite_get_group(struct build_sprite_data *data, struct bitmap_data *bitmap);

/* ---------- globals */

boolean debug_sprites;

struct
{
	boolean initialized;
	boolean debug_flag;
	real screen_coverage;
	short big_sprite_count;
	real screen_area_scale;
	real_vector3d viewer_space_world_up;
	real_vector3d viewer_space_world_forward;
} build_sprite_globals;

static char *global_sprite_render_orientations_strings[NUMBER_OF_BUILD_SPRITE_ORIENTATIONS] = /* fake name */
{
	"screen facing",
	"parallel to direction",
	"perpendicular to direction"
};

struct enum_definition global_sprite_render_orientations_enum =
{
	NUMBEROF(global_sprite_render_orientations_strings),
	global_sprite_render_orientations_strings
};

real const one_over_full_circle = 1.f / (2.f * _pi);

/* ---------- public code */

static void build_sprite_transform_origin_and_direction(
	struct build_sprite_data const *data,
	unsigned long flags,
	real_point3d const *untransformed_origin,
	real_vector3d const *untransformed_direction,
	real_point3d *transformed_origin,
	real_vector3d *transformed_direction)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 74, data);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 75, untransformed_origin);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 76, transformed_origin);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 77, transformed_direction);

	if (TEST_FLAG(data->flags, _build_sprites_screen_space_bit))
	{
		render_camera_screen_to_view(
			&render.camera,
			&render.frustum,
			(real_point2d const *)untransformed_origin,
			(real_vector3d *)&transformed_origin);
		match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 84, !untransformed_direction);
	}
	else if (TEST_FLAG(flags, _build_sprite_viewer_space_bit))
	{
		*transformed_origin = *untransformed_origin;

		if (untransformed_direction)
		{
			*transformed_direction = *untransformed_direction;
		}
	}
	else
	{
		matrix4x3_transform_point(&render.frustum.world_to_view, untransformed_origin, transformed_origin);

		if (untransformed_direction)
		{
			matrix4x3_transform_normal(&render.frustum.world_to_view, untransformed_direction, transformed_direction);
		}
	}

	return;
}

static void build_sprite_compute_basis(
	struct build_sprite_data const *data,
	short mode,
	unsigned long flags,
	real_point3d const *origin,
	real_vector3d const *direction,
	real_matrix4x3 *basis)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 116, data);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 117, origin);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 118, basis);

	if (TEST_FLAG(data->flags, _build_sprites_screen_space_bit))
	{
		match_vassert(
			"c:\\halo\\SOURCE\\render\\render_sprite.c",
			122,
			mode == _build_sprite_normal,
			"build_sprite only supports normal sprites in screen space.");
	}
	else if (mode == _build_sprite_normal)
	{
		set_real_vector3d(&basis->forward, 1.f, 0.f, 0.f);
		set_real_vector3d(&basis->left, 0.f, 1.f, 0.f);
	}
	else if (mode == _build_sprite_parallel)
	{
		basis->forward = *direction;
		normalize3d(&basis->forward);
		cross_product3d((real_vector3d const *)origin, &basis->forward, &basis->left);
		normalize3d(&basis->left);
	}
	else if (mode == _build_sprite_perpendicular)
	{
		real_vector3d const *up = &build_sprite_globals.viewer_space_world_up;
		real dot = dot_product3d(direction, up);

		if (dot * dot > magnitude_squared3d(direction) * PERPENDICULAR_SPRITE_PARALLEL_DOT_SQUARED)
		{
			up = &build_sprite_globals.viewer_space_world_forward;
		}

		cross_product3d(direction, up, &basis->forward);
		normalize3d(&basis->forward);
		basis->left = basis->forward;
		basis->up = *direction;
		normalize3d(&basis->up);
		rotate_vector_about_axis(&basis->left, &basis->up, -1.f, 0.f);
	}
	else
	{
		match_halt("c:\\halo\\SOURCE\\render\\render_sprite.c", 172);
	}

	return;
}

static void build_sprite_compute_scale(
	struct build_sprite_data const *data,
	short mode,
	unsigned long flags,
	real_point3d const *origin,
	struct bitmap_data const *bitmap,
	real *scale)
{
	if (TEST_FLAG(data->flags, _build_sprites_screen_space_bit))
	{
		if (*scale == 0.f)
		{
			*scale = 1.f;
		}
	}
	else if (mode == _build_sprite_normal && *scale == 0.f)
	{
		*scale = -origin->z / render.frustum.projection_world_to_screen.i;
	}

	*scale = bitmap->width * *scale;

	return;
}

__inline real build_sprite_compute_vertex_fade(
	short fade_mode,
	real_point3d const *viewer_point,
	real_vector3d const *viewer_normal)
{
	real fade = 1.f;

	if (fade_mode != _shader_framebuffer_fade_mode_none)
	{
		fade = (real)fabs(
			dot_product3d((real_vector3d const *)viewer_point, viewer_normal) /
			magnitude3d((real_vector3d const *)viewer_point));

		if (fade_mode == _shader_framebuffer_fade_mode_fade_when_parallel)
		{
			fade = 1.f - fade;
		}
	}

	return fade;
}

void build_sprite_prepare_for_window(
	void)
{
	if (debug_sprites)
	{
		char printbuffer[512];

		sprintf(
			printbuffer,
			"   coverage: %.1f big sprites: %d",
			build_sprite_globals.screen_coverage,
			build_sprite_globals.big_sprite_count);
		render_debug_string(FALSE, printbuffer);
	}

	build_sprite_globals.screen_coverage = 0.f;
	build_sprite_globals.big_sprite_count = 0;
	matrix4x3_transform_normal(&render.frustum.world_to_view, global_up3d, &build_sprite_globals.viewer_space_world_up);
	matrix4x3_transform_normal(&render.frustum.world_to_view, global_left3d, &build_sprite_globals.viewer_space_world_forward);

	return;
}

static short build_sprite_get_group(
	struct build_sprite_data *data,
	struct bitmap_data *bitmap)
{
	static boolean warned;
	short group_index;

	for (group_index = 0; group_index < data->group_count && data->groups[group_index].bitmap != bitmap; group_index++);

	match_vassert(
		"c:\\halo\\SOURCE\\render\\render_sprite.c",
		275,
		group_index < data->group_count || data->group_count < MAXIMUM_NUMBER_OF_BUILD_SPRITE_BITMAPS,
		csprintf(
			temporary,
			"a build_sprites_begin call can accomodate at most %d bitmaps",
			MAXIMUM_NUMBER_OF_BUILD_SPRITE_BITMAPS));

	if (group_index >= data->group_count && data->group_count >= MAXIMUM_NUMBER_OF_BUILD_SPRITE_BITMAPS)
	{
		group_index = NONE;
	}
	else if (group_index >= data->group_count)
	{
		struct build_sprite_group *group = &data->groups[group_index];

		data->group_count++;
		group->bitmap = bitmap;

		if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
		{
			rasterizer_globals.current_lock_operation = _rasterizer_lock_sprite;
			group->vertex_buffer_index = rasterizer_dynamic_vertices_new(
				TEST_FLAG(data->flags, _build_sprites_screen_space_bit) ?
					_rasterizer_vertex_type_dynamic_screen :
					_rasterizer_vertex_type_dynamic_unlit,
				NUMBER_OF_VERTICES_PER_SPRITE * data->maximum_sprite_count);

			if (group->vertex_buffer_index != NONE)
			{
				group->vertices = rasterizer_dynamic_vertices_lock(group->vertex_buffer_index);
				match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 294, group->vertices);
			}
			else
			{
				if (!warned)
				{
					error(_error_silent, "build_sprite failed to allocate dynamic vertices");
					warned = TRUE;
				}

				group->vertices = NULL;
			}

			rasterizer_globals.current_lock_operation = _rasterizer_lock_none;
		}
		else
		{
			group->vertices = NULL;
		}

		group->sprite_count = 0;
	}

	if (group_index != NONE && !data->groups[group_index].vertices)
	{
		group_index = NONE;
	}

	return group_index;
}

void build_sprites_begin(
	struct build_sprite_data *data,
	short maximum_sprite_count,
	long bitmap_group_index,
	struct shader_effect const *shader,
	unsigned long flags)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 335, shader);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 336, !TEST_FLAG(flags, _build_sprites_valid_bit));

	data->bitmap_group_index = bitmap_group_index;
	data->group_count = 0;
	data->sprite_count = 0;
	data->maximum_sprite_count = maximum_sprite_count;
	data->flags = flags;
	data->shader = shader;
	data->centroid = *global_origin3d;
	SET_FLAG(data->flags, _build_sprites_valid_bit, TRUE);

	return;
}

void build_sprites_end(
	struct build_sprite_data *data)
{
	short group_index;
	real one_over_sprite_count = 1.f / data->sprite_count;

	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 358, TEST_FLAG(data->flags, _build_sprites_valid_bit));

	data->centroid.x *= one_over_sprite_count;
	data->centroid.y *= one_over_sprite_count;
	data->centroid.z *= one_over_sprite_count;
	matrix4x3_transform_point(&render.frustum.view_to_world, &data->centroid, &data->centroid);

	for (group_index = 0; group_index < data->group_count; group_index++)
	{
		struct build_sprite_group *group = &data->groups[group_index];

		if (group->sprite_count)
		{
			rasterizer_dynamic_vertices_unlock(group->vertex_buffer_index);

			if (TEST_FLAG(data->flags, _build_sprites_screen_space_bit))
			{
				match_vassert(
					"c:\\halo\\SOURCE\\render\\render_sprite.c",
					377,
					FALSE,
					"### ERROR sprites rendered with screen geometry -- tell Bernie!!");
				rasterizer_dynamic_screen_geometry_draw(
					NULL,
					DYNAMIC_TRIANGLE_QUAD_LIST,
					group->vertex_buffer_index,
					2 * group->sprite_count);
			}
			else
			{
				unsigned long geometry_flags = FLAG(_rasterizer_geometry_viewspace_bit);

				SET_FLAG(geometry_flags, _rasterizer_geometry_first_person_bit, TEST_FLAG(data->flags, _build_sprites_first_person_bit));
				rasterizer_dynamic_unlit_geometry_draw(
					(struct shader const *)data->shader,
					group->bitmap,
					NULL,
					DYNAMIC_TRIANGLE_QUAD_LIST,
					group->vertex_buffer_index,
					2 * group->sprite_count,
					&data->centroid,
					geometry_flags);
			}

			rasterizer_dynamic_vertices_delete(group->vertex_buffer_index);
		}
	}

	SET_FLAG(data->flags, _build_sprites_valid_bit, FALSE);

	return;
}

void build_sprite(
	struct build_sprite_data *data,
	short mode,
	short sequence_index,
	short sprite_index,
	real_point3d const *untransformed_origin,
	real_vector3d const *untransformed_direction,
	real rotation,
	real scale,
	real_argb_color const *color,
	real fade,
	unsigned long flags)
{
	struct bitmap_group *bitmap_group = bitmap_group_get(data->bitmap_group_index);

	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 418, untransformed_origin);
	match_assert(
		"c:\\halo\\SOURCE\\render\\render_sprite.c",
		419,
		mode==_build_sprite_normal || (untransformed_direction && magnitude_squared3d(untransformed_direction)));

	if (!color)
	{
		color = global_real_argb_white;
	}

	if (data->sprite_count < data->maximum_sprite_count)
	{
		if (sequence_index >= 0 && sequence_index < bitmap_group->sequences.count)
		{
			struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(
				&bitmap_group->sequences,
				sequence_index,
				struct bitmap_group_sequence);

			if (sequence->first_bitmap_index != NONE && sprite_index >= 0 && sprite_index < sequence->sprites.count)
			{
				struct bitmap_data *bitmap;
				short group_index;
				struct bitmap_group_sprite *sprite = TAG_BLOCK_GET_ELEMENT(
					&sequence->sprites,
					sprite_index,
					struct bitmap_group_sprite);

				match_vassert(
					"c:\\halo\\SOURCE\\render\\render_sprite.c",
					435,
					sprite->bitmap_index != NONE,
					csprintf(
						temporary,
						"the bitmap group %s sequence %d sprite %d references bitmap -1 (tell matt).",
						tag_get_name(data->bitmap_group_index),
						sequence_index,
						sprite_index));

				bitmap = TAG_BLOCK_GET_ELEMENT(&bitmap_group->bitmaps, sprite->bitmap_index, struct bitmap_data);
				group_index = build_sprite_get_group(data, bitmap);

				if (group_index != NONE)
				{
					struct build_sprite_group *group = &data->groups[group_index];

					if (group->sprite_count < data->maximum_sprite_count)
					{
						real_point3d origin;
						real_vector3d direction;
						real_matrix4x3 basis;
						real_rectangle3d bounding_rectangle;
						pixel32 color_pixel;
						byte alpha;
						short vertex;
						short vertex_index = NUMBER_OF_VERTICES_PER_SPRITE * group->sprite_count;
						real sine_rotation = 0.f;
						real cosine_rotation = 1.f;

						bounding_rectangle = *global_null_rectangle3d;

						if (rotation != 0.f)
						{
							sine_rotation = sine(rotation);
							cosine_rotation = cosine(rotation);
						}

						build_sprite_transform_origin_and_direction(
							data,
							flags,
							untransformed_origin,
							untransformed_direction,
							&origin,
							&direction);
						build_sprite_compute_basis(data, mode, flags, &origin, &direction, &basis);
						build_sprite_compute_scale(data, mode, flags, &origin, bitmap, &scale);

						if (data->shader &&
							data->shader->effect.framebuffer_fade_mode != _shader_framebuffer_fade_mode_none &&
							mode != _build_sprite_normal)
						{
							cross_product3d(&basis.forward, &basis.left, &basis.up);
							fade *= build_sprite_compute_vertex_fade(
								data->shader->effect.framebuffer_fade_mode,
								&origin,
								&basis.up);
						}

						color_pixel = real_argb_color_to_pixel32(color);

						if (data->shader &&
							data->shader->effect.framebuffer_blend_function != _shader_framebuffer_blend_function_alpha_blend &&
							!TEST_FLAG(data->shader->effect.flags, _shader_effect_uses_nonlinear_tint_bit))
						{
							alpha = (byte)(fade * UNSIGNED_CHAR_MAX);
						}
						else
						{
							alpha = (byte)(PIXEL32_ALPHA(color_pixel) * fade);
						}

						color_pixel = PIXEL32_FROM_ARGB(
							alpha,
							PIXEL32_RED(color_pixel),
							PIXEL32_GREEN(color_pixel),
							PIXEL32_BLUE(color_pixel));

						for (vertex = 0; vertex < NUMBER_OF_VERTICES_PER_SPRITE; vertex++)
						{
							real u = ((vertex >> 1) ^ vertex) & 1 ? sprite->bounds.x1 : sprite->bounds.x0;
							real v = TEST_FLAG(vertex, 1) ? sprite->bounds.y0 : sprite->bounds.y1;
							real x = u - (sprite->bounds.x0 + sprite->registration_point.x);
							real y = (sprite->bounds.y0 + sprite->registration_point.y) - v;
							real x_sine = x * sine_rotation;

							x = x * cosine_rotation - y * sine_rotation;
							y = y * cosine_rotation + x_sine;

							if (TEST_FLAG(flags, _build_sprite_u_mirror_bit))
							{
								x = -x;
							}

							if (TEST_FLAG(flags, _build_sprite_v_mirror_bit))
							{
								y = -y;
							}

							if (TEST_FLAG(data->flags, _build_sprites_screen_space_bit))
							{
								struct dynamic_screen_vertex *screen_vertex = (struct dynamic_screen_vertex *)group->vertices + vertex_index;

								screen_vertex->position.x = scale * x + origin.x;
								screen_vertex->position.y = scale * y + origin.y;
								screen_vertex->texcoord.x = u;
								screen_vertex->texcoord.y = v;
								screen_vertex->color = color_pixel;
							}
							else
							{
								real_point3d point;
								struct dynamic_unlit_vertex *unlit_vertex = (struct dynamic_unlit_vertex *)group->vertices + vertex_index;

								point.x = (basis.forward.i * x + basis.left.i * y) * scale + origin.x;
								point.y = (basis.forward.j * x + basis.left.j * y) * scale + origin.y;
								point.z = (basis.forward.k * x + basis.left.k * y) * scale + origin.z;

								if (point.x < bounding_rectangle.x0)
								{
									bounding_rectangle.x0 = point.x;
								}

								if (point.x > bounding_rectangle.x1)
								{
									bounding_rectangle.x1 = point.x;
								}

								if (point.y < bounding_rectangle.y0)
								{
									bounding_rectangle.y0 = point.y;
								}

								if (point.y > bounding_rectangle.y1)
								{
									bounding_rectangle.y1 = point.y;
								}

								if (point.z < bounding_rectangle.z0)
								{
									bounding_rectangle.z0 = point.z;
								}

								if (point.z > bounding_rectangle.z1)
								{
									bounding_rectangle.z1 = point.z;
								}

								unlit_vertex->position = point;
								unlit_vertex->texcoord.x = u;
								unlit_vertex->texcoord.y = v;
								unlit_vertex->color = color_pixel;
							}

							vertex_index++;
						}

						data->centroid.x += origin.x;
						data->centroid.y += origin.y;
						data->centroid.z += origin.z;
						group->sprite_count++;
						data->sprite_count++;

						if (!TEST_FLAG(data->flags, _build_sprites_screen_space_bit))
						{
							real coverage = render_frustum_cube_view_fraction(&render.frustum, &bounding_rectangle);

							build_sprite_globals.screen_coverage += coverage;

							if (coverage > BIG_SPRITE_SCREEN_COVERAGE && build_sprite_globals.big_sprite_count++ > MAXIMUM_BIG_SPRITES)
							{
								group->sprite_count--;
								data->sprite_count--;
							}

							if (debug_sprites)
							{
								real_point3d p0;
								real_point3d p1;
								real_point3d p2;
								real_point3d p3;
								struct dynamic_unlit_vertex *vertex0 = (struct dynamic_unlit_vertex *)group->vertices + vertex_index - 4;
								struct dynamic_unlit_vertex *vertex1 = (struct dynamic_unlit_vertex *)group->vertices + vertex_index - 3;
								struct dynamic_unlit_vertex *vertex2 = (struct dynamic_unlit_vertex *)group->vertices + vertex_index - 2;
								struct dynamic_unlit_vertex *vertex3 = (struct dynamic_unlit_vertex *)group->vertices + vertex_index - 1;

								matrix4x3_transform_point(&render.frustum.view_to_world, &vertex0->position, &p0);
								matrix4x3_transform_point(&render.frustum.view_to_world, &vertex1->position, &p1);
								matrix4x3_transform_point(&render.frustum.view_to_world, &vertex2->position, &p2);
								matrix4x3_transform_point(&render.frustum.view_to_world, &vertex3->position, &p3);
								rasterizer_debug_line(&p0, &p1, global_real_argb_white);
								rasterizer_debug_line(&p0, &p2, global_real_argb_white);
								rasterizer_debug_line(&p2, &p3, global_real_argb_white);
								rasterizer_debug_line(&p3, &p1, global_real_argb_white);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		error(_error_silent, "build_sprite sprite count exceeded.");
	}

	return;
}

void build_sprite_rotational(
	struct build_sprite_data *data,
	unsigned long flags,
	short first_sequence_index,
	short sprite_index,
	real_point3d const *untransformed_origin,
	real_vector3d const *untransformed_axis_of_rotation,
	real rotation,
	real scale,
	real_argb_color const *color,
	real fade)
{
	real_point3d origin;
	real_vector3d axis_of_rotation;
	real_euler_angles3d incident_angles;
	real blend;

	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 646, data);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 647, untransformed_origin);
	match_assert("c:\\halo\\SOURCE\\render\\render_sprite.c", 648, untransformed_axis_of_rotation);

	if (!color)
	{
		color = global_real_argb_white;
	}

	build_sprite_transform_origin_and_direction(
		data,
		flags & FLAG(_build_sprite_rotational_viewer_space_bit),
		untransformed_origin,
		untransformed_axis_of_rotation,
		&origin,
		&axis_of_rotation);
	incident_angles.pitch = angle_between_vectors3d((real_vector3d const *)&origin, &axis_of_rotation) - _pi2;
	blend = incident_angles.pitch * incident_angles.pitch * (1.f / (_pi2 * _pi2));
	blend = PIN(blend, 0.f, 1.f);

	if (blend > MINIMUM_ROTATIONAL_SPRITE_BLEND)
	{
		short sprite;
		real sprite_rotation;
		unsigned long sprite_flags = FLAG(_build_sprite_viewer_space_bit);
		short sprite_count = TAG_BLOCK_GET_ELEMENT(
			&bitmap_group_get(data->bitmap_group_index)->sequences,
			first_sequence_index + 1,
			struct bitmap_group_sequence)->sprites.count;

		if (TEST_FLAG(flags, _build_sprite_rotational_sideways_rotation_animates_bit))
		{
			sprite = (short)(sprite_index + fmod(sprite_count * one_over_full_circle * rotation + 0.5f, sprite_count));
			sprite_rotation = 0.f;

			if (incident_angles.pitch < 0.f)
			{
				sprite = sprite_count - sprite_index;
			}
		}
		else
		{
			sprite = sprite_index;
			sprite_rotation = rotation;

			if (incident_angles.pitch < 0.f)
			{
				SET_FLAG(sprite_flags, _build_sprite_u_mirror_bit, TRUE);
			}
		}

		build_sprite(
			data,
			_build_sprite_normal,
			first_sequence_index + 1,
			sprite,
			&origin,
			NULL,
			sprite_rotation,
			scale,
			color,
			blend * fade,
			sprite_flags);
	}

	blend = 1.f - blend;

	if (blend > MINIMUM_ROTATIONAL_SPRITE_BLEND)
	{
		short sprite_count = TAG_BLOCK_GET_ELEMENT(
			&bitmap_group_get(data->bitmap_group_index)->sequences,
			first_sequence_index,
			struct bitmap_group_sequence)->sprites.count;
		short sprite = (short)fmod(sprite_count * one_over_full_circle * rotation + 0.5f, sprite_count);

		incident_angles.roll = arctangent(axis_of_rotation.j, axis_of_rotation.i);
		build_sprite(
			data,
			_build_sprite_normal,
			first_sequence_index,
			sprite,
			&origin,
			NULL,
			incident_angles.roll,
			scale,
			color,
			blend * fade,
			FLAG(_build_sprite_viewer_space_bit));
	}

	return;
}
