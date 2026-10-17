/*
RENDER_CONTRAILS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "objects.h"
#include "collision_bsp.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "texture_cache.h"
#include "contrails.h"
#include "contrail_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void render_contrails(unsigned long render_type_flags);
static void render_contrail(struct contrail_datum const *contrail, struct contrail_definition const *definition, short instance_index);

/* ---------- globals */

/* ---------- public code */

void render_contrails_ground_mapped(
	void)
{
	render_contrails(FLAG(_contrail_render_type_ground));

	return;
}

void render_contrails_media_mapped(
	void)
{
	render_contrails(FLAG(_contrail_render_type_media));

	return;
}

void render_contrails_normal(
	void)
{
	render_contrails(~(FLAG(_contrail_render_type_media) | FLAG(_contrail_render_type_ground)));

	return;
}

/* ---------- private code */

static void render_contrails(
	unsigned long render_type_flags)
{
	if (render_contrails_enabled)
	{
		long contrail_index;

		for (contrail_index = data_next_index(contrail_data, NONE);
			contrail_index != NONE;
			contrail_index = data_next_index(contrail_data, contrail_index))
		{
			short instance_index;
			struct contrail_datum *contrail = contrail_get(contrail_index);
			struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);

			for (instance_index = 0; instance_index < MAXIMUM_INSTANCES_PER_CONTRAIL; instance_index++)
			{
				if (TEST_FLAG(render_type_flags, definition->render_type) && contrail->contrail_point_counts[instance_index] >= 2)
				{
					render_contrail(contrail, definition, instance_index);
				}
			}
		}
	}

	return;
}

__inline real contrail_fade(
	struct contrail_definition const *definition,
	short fade_mode,
	real_point3d const *world_point,
	real_vector3d const *world_normal)
{
	real fade = 1.f;

	if (fade_mode)
	{
		real_vector3d camera_vector;

		vector_from_points3d(world_point, &render.camera.position, &camera_vector);
		fade = (real)fabs(dot_product3d(world_normal, &camera_vector) / magnitude3d(&camera_vector));

		if (TEST_FLAG(definition->flags, _contrail_fades_slowly_bit))
		{
			fade = transition_function_evaluate(_transition_function_very_early, fade);
		}

		if (fade_mode == _shader_framebuffer_fade_mode_fade_when_parallel)
		{
			fade = 1.f - fade;
		}
	}

	return fade;
}

static void render_contrail(
	struct contrail_datum const *contrail,
	struct contrail_definition const *definition,
	short instance_index)
{
	struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(
		definition->bitmap.index,
		contrail->sequence_index,
		contrail->frame_index);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_contrail;

	if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
	{
		short segment_count = contrail->contrail_point_counts[instance_index] - 1;
		short triangle_count = 2 * segment_count;
		short vertex_count = triangle_count + 2;
		long triangle_buffer_index = rasterizer_dynamic_triangles_new(triangle_count);
		long vertex_buffer_index = rasterizer_dynamic_vertices_new(_rasterizer_vertex_type_dynamic_unlit, vertex_count);

		if (triangle_buffer_index != NONE && vertex_buffer_index != NONE)
		{
			real texture_u;
			real texture_u_delta;
			real texture_v0;
			real texture_v1;
			struct contrail_point_datum *previous_point;
			long point_index;
			struct rasterizer_triangle *triangles = rasterizer_dynamic_triangles_lock(triangle_buffer_index);
			struct dynamic_unlit_vertex *vertices = rasterizer_dynamic_vertices_lock(vertex_buffer_index);
			real_point3d centroid = *global_origin3d;
			struct shader_effect const *shader = &definition->shader;
			boolean orient = shader->effect.framebuffer_fade_mode != _shader_framebuffer_fade_mode_none;

			match_assert("c:\\halo\\SOURCE\\render\\render_contrails.c", 136, triangles && vertices);

			texture_u = contrail->texture_offset_u;
			texture_u_delta = -contrail_scale_value(
				contrail->density,
				definition->texture_repeats_u,
				definition->scale_flags,
				_contrail_texture_repeats_u_bit);
			texture_v0 = contrail->texture_offset_v;
			texture_v1 = contrail_scale_value(
				contrail->density,
				definition->texture_repeats_v,
				definition->scale_flags,
				_contrail_texture_repeats_v_bit) + texture_v0;
			previous_point = NULL;

			for (point_index = contrail->first_contrail_point_indices[instance_index];
				point_index != NONE;
				point_index = previous_point->next_contrail_point_index)
			{
				real_argb_color rgbcolor;
				real_vector3d normal;
				struct contrail_point_datum *point = contrail_point_get(point_index);
				struct contrail_point_state *state = TAG_BLOCK_GET_ELEMENT(
					&definition->states,
					point->state_index,
					struct contrail_point_state);
				real color_scale = contrail_scale_value(
					point->density,
					1.f,
					state->scale_flags,
					_contrail_state_color_bit);
				real width = contrail_scale_value(
					point->density,
					state->width,
					state->scale_flags,
					_contrail_state_width_bit);

				rgbcolor.alpha = (state->color_upper_bound.alpha - state->color_lower_bound.alpha) * color_scale + state->color_lower_bound.alpha;
				rgbcolor.red = (state->color_upper_bound.red - state->color_lower_bound.red) * color_scale + state->color_lower_bound.red;
				rgbcolor.green = (state->color_upper_bound.green - state->color_lower_bound.green) * color_scale + state->color_lower_bound.green;
				rgbcolor.blue = (state->color_upper_bound.blue - state->color_lower_bound.blue) * color_scale + state->color_lower_bound.blue;

				if (TEST_FLAG(point->flags, _contrail_point_transitioning_bit))
				{
					real_argb_color next_color;
					struct contrail_point_state *next_state = TAG_BLOCK_GET_ELEMENT(
						&definition->states,
						point->state_index + 1,
						struct contrail_point_state);
					real transition = point->time;
					real next_color_scale = contrail_scale_value(
						point->density,
						1.f,
						next_state->scale_flags,
						_contrail_state_color_bit);
					real next_width = contrail_scale_value(
						point->density,
						next_state->width,
						next_state->scale_flags,
						_contrail_state_width_bit);

					next_color.alpha = (next_state->color_upper_bound.alpha - next_state->color_lower_bound.alpha) * next_color_scale + next_state->color_lower_bound.alpha;
					next_color.red = (next_state->color_upper_bound.red - next_state->color_lower_bound.red) * next_color_scale + next_state->color_lower_bound.red;
					next_color.green = (next_state->color_upper_bound.green - next_state->color_lower_bound.green) * next_color_scale + next_state->color_lower_bound.green;
					next_color.blue = (next_state->color_upper_bound.blue - next_state->color_lower_bound.blue) * next_color_scale + next_state->color_lower_bound.blue;
					width = (next_width - width) * transition + width;
					rgbcolor.alpha = (next_color.alpha - rgbcolor.alpha) * transition + rgbcolor.alpha;
					rgbcolor.red = (next_color.red - rgbcolor.red) * transition + rgbcolor.red;
					rgbcolor.green = (next_color.green - rgbcolor.green) * transition + rgbcolor.green;
					rgbcolor.blue = (next_color.blue - rgbcolor.blue) * transition + rgbcolor.blue;
				}

				if (contrail->object_index != NONE)
				{
					struct object_datum *object = object_get(contrail->object_index);
					struct object_definition *object_definition = object_definition_get(object->definition_index);
					struct object_attachment_definition *attachment = TAG_BLOCK_GET_ELEMENT(
						&object_definition->object.attachments,
						contrail->attachment_index,
						struct object_attachment_definition);
					short change_color_index = attachment->change_color_reference - 1;

					if (change_color_index != NONE)
					{
						real_rgb_color const *change_color = &object->object.outgoing_change_colors[change_color_index];

						rgbcolor.red *= change_color->red;
						rgbcolor.green *= change_color->green;
						rgbcolor.blue *= change_color->blue;
					}
				}

				width *= 0.5f;
				vertices[0].texcoord.x = texture_u;
				vertices[0].texcoord.y = texture_v1;
				vertices[1].texcoord.x = texture_u;
				vertices[1].texcoord.y = texture_v0;
				centroid.x += point->position.x;
				centroid.y += point->position.y;
				centroid.z += point->position.z;

				switch (definition->render_type)
				{
				case _contrail_render_type_vertical:
					vertices[0].position.x = point->position.x;
					vertices[0].position.y = point->position.y;
					vertices[0].position.z = point->position.z - width;
					vertices[1].position.x = point->position.x;
					vertices[1].position.y = point->position.y;
					vertices[1].position.z = point->position.z + width;

					if (orient)
					{
						if (previous_point)
						{
							normal.i = previous_point->position.y - point->position.y;
							normal.j = point->position.x - previous_point->position.x;
						}
						else
						{
							struct contrail_point_datum *next_point = contrail_point_get(point->next_contrail_point_index);

							normal.i = point->position.y - next_point->position.y;
							normal.j = next_point->position.x - point->position.x;
						}

						normal.k = 0.f;
						normalize3d(&normal);
					}
					break;
				case _contrail_render_type_horizontal:
				case _contrail_render_type_media:
				{
					real_vector2d vector;

					if (previous_point)
					{
						vector.i = previous_point->position.y - point->position.y;
						vector.j = point->position.x - previous_point->position.x;
					}
					else
					{
						struct contrail_point_datum *next_point = contrail_point_get(point->next_contrail_point_index);

						vector.i = point->position.y - next_point->position.y;
						vector.j = next_point->position.x - point->position.x;
					}

					normalize2d(&vector);
					vertices[0].position.x = point->position.x - vector.i * width;
					vertices[0].position.y = point->position.y - vector.j * width;
					vertices[0].position.z = point->position.z;
					vertices[1].position.x = vector.i * width + point->position.x;
					vertices[1].position.y = vector.j * width + point->position.y;
					vertices[1].position.z = point->position.z;

					if (orient)
					{
						normal = *global_up3d;
					}
					break;
				}
				case _contrail_render_type_viewer:
				{
					real_vector3d camera_vector;
					real_vector3d point_vector;
					real_vector3d vector;

					if (previous_point)
					{
						camera_vector.i = render.camera.position.x - previous_point->position.x;
						camera_vector.j = render.camera.position.y - previous_point->position.y;
						camera_vector.k = render.camera.position.z - previous_point->position.z;
						point_vector.i = point->position.x - previous_point->position.x;
						point_vector.j = point->position.y - previous_point->position.y;
						point_vector.k = point->position.z - previous_point->position.z;
					}
					else
					{
						struct contrail_point_datum *next_point = contrail_point_get(point->next_contrail_point_index);

						camera_vector.i = render.camera.position.x - point->position.x;
						camera_vector.j = render.camera.position.y - point->position.y;
						camera_vector.k = render.camera.position.z - point->position.z;
						point_vector.i = next_point->position.x - point->position.x;
						point_vector.j = next_point->position.y - point->position.y;
						point_vector.k = next_point->position.z - point->position.z;
					}

					vector.i = point_vector.k * camera_vector.j - point_vector.j * camera_vector.k;
					vector.j = point_vector.i * camera_vector.k - point_vector.k * camera_vector.i;
					vector.k = point_vector.j * camera_vector.i - point_vector.i * camera_vector.j;
					normalize3d(&vector);
					vertices[0].position.x = point->position.x - vector.i * width;
					vertices[0].position.y = point->position.y - vector.j * width;
					vertices[0].position.z = point->position.z - vector.k * width;
					vertices[1].position.x = vector.i * width + point->position.x;
					vertices[1].position.y = vector.j * width + point->position.y;
					vertices[1].position.z = vector.k * width + point->position.z;

					if (orient)
					{
						normal.i = vector.k * point_vector.j - vector.j * point_vector.k;
						normal.j = vector.i * point_vector.k - vector.k * point_vector.i;
						normal.k = vector.j * point_vector.i - vector.i * point_vector.j;
						normalize3d(&normal);
					}
					break;
				}
				default:
					error(
						_error_silent,
						"contrail %s uses an unsupported render type.",
						tag_get_name(contrail->definition_index));
					return;
				}

				rgbcolor.alpha *= contrail_fade(definition, shader->effect.framebuffer_fade_mode, &point->position, &normal);
				rgbcolor.alpha = PIN(rgbcolor.alpha, 0.f, 1.f);
				vertices[1].color = real_argb_color_to_pixel32(&rgbcolor);
				vertices[0].color = vertices[1].color;
				texture_u += texture_u_delta;
				vertices += 2;
				previous_point = point;
			}

			vertices -= vertex_count;

			if (!TEST_FLAG(definition->flags, _contrail_first_point_unfaded_bit))
			{
				vertices[0].color &= ~PIXEL32_ALPHA_MASK;
				vertices[1].color &= ~PIXEL32_ALPHA_MASK;
			}

			if (!TEST_FLAG(definition->flags, _contrail_last_point_unfaded_bit))
			{
				vertices[vertex_count - 1].color &= ~PIXEL32_ALPHA_MASK;
				vertices[vertex_count - 2].color &= ~PIXEL32_ALPHA_MASK;
			}

			{
				short segment_index;

				for (segment_index = 0; segment_index < segment_count; segment_index++)
				{
					triangles->vertex_indices[0] = 2 * segment_index;
					triangles->vertex_indices[1] = 2 * segment_index + 1;
					triangles->vertex_indices[2] = 2 * segment_index + 2;
					triangles++;
					triangles->vertex_indices[0] = 2 * segment_index + 2;
					triangles->vertex_indices[1] = 2 * segment_index + 1;
					triangles->vertex_indices[2] = 2 * segment_index + 3;
					triangles++;
				}
			}

			scale_vector3d(
				(real_vector3d *)&centroid,
				1.f / contrail->contrail_point_counts[instance_index],
				(real_vector3d *)&centroid);
			rasterizer_dynamic_triangles_unlock(triangle_buffer_index);
			rasterizer_dynamic_vertices_unlock(vertex_buffer_index);
			rasterizer_dynamic_unlit_geometry_draw(
				(struct shader const *)shader,
				bitmap,
				NULL,
				triangle_buffer_index,
				vertex_buffer_index,
				triangle_count,
				&centroid,
				0);
			rasterizer_dynamic_triangles_delete(triangle_buffer_index);
			rasterizer_dynamic_vertices_delete(vertex_buffer_index);
		}
	}

	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}
