/*
RENDER_SKY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "objects.h"
#include "rasterizer.h"
#include "light_definitions.h"
#include "collisions.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "structures/radiosity/radiosity.h"
#include "vector_tree.h"
#include "sky_definitions.h"
#include "structures/radiosity/radiosity_io.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static struct
{
	real animation_states[MAXIMUM_ANIMATIONS_PER_SKY];
} render_sky_globals;

/* ---------- public code */

void render_sky(
	void)
{
	real_matrix4x3 node_matrices[MAXIMUM_NODES_PER_MODEL];
	struct sky *sky;
	struct model *model;
	struct render_lighting lighting;
	real_orientation node_orientations[MAXIMUM_NODES_PER_MODEL];
	real shader_values[MAXIMUM_SHADER_FUNCTIONS_PER_SKY];
	short animation_index;
	short shader_function_index;
	short light_index;

	match_assert(
		"c:\\halo\\SOURCE\\render\\render_sky.c",
		38,
		!render.visible_sky_model || scenario_get_sky(render.visible_sky_index));

	if (render.visible_sky_model)
	{
		sky = scenario_get_sky(render.visible_sky_index);
		model = model_definition_get(sky->model.index);
		model_get_node_orientations(model, node_orientations);

		if (sky->animation_graph.index != NONE)
		{
			struct animation_graph *animation_graph = animation_graph_definition_get(sky->animation_graph.index);

			for (animation_index = 0; animation_index < sky->animations.count; animation_index++)
			{
				struct sky_animation *sky_animation = TAG_BLOCK_GET_ELEMENT(
					&sky->animations,
					animation_index,
					struct sky_animation);

				if (sky_animation->animation_index >= 0 &&
					sky_animation->animation_index < animation_graph->animations.count &&
					sky_animation->period != 0.f)
				{
					struct animation *animation = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						animation_index,
						struct animation);

					if (animation->node_count == model->nodes.count)
					{
						render_sky_globals.animation_states[animation_index] += render.time_delta_since_tick_sec / sky_animation->period;
						render_sky_globals.animation_states[animation_index] = (real)fmod(
							render_sky_globals.animation_states[animation_index],
							1.f);
						overlay_animation_apply_continuous(
							animation,
							animation->frame_count * render_sky_globals.animation_states[animation_index],
							node_orientations);
					}
				}
			}
		}

		model_node_matrices_from_orientations(
			model,
			node_matrices,
			node_orientations,
			global_origin3d,
			global_forward3d,
			global_up3d);

		for (shader_function_index = 0; shader_function_index < sky->shader_functions.count; shader_function_index++)
		{
			struct sky_shader_function *shader_function = TAG_BLOCK_GET_ELEMENT(
				&sky->shader_functions,
				shader_function_index,
				struct sky_shader_function);

			shader_values[shader_function_index] = 1.f;
		}

		for (light_index = 0; light_index < sky->lights.count; light_index++)
		{
			struct sky_light *light = TAG_BLOCK_GET_ELEMENT(&sky->lights, light_index, struct sky_light);

			if (light->lens_flare.index != NONE)
			{
				real_vector3d offset_vector;
				struct object_marker marker;
				boolean valid = TRUE;

				if (strlen(light->lens_flare_marker_name))
				{
					if (model_get_marker_by_name(
						sky->model.index,
						light->lens_flare_marker_name,
						NULL,
						NULL,
						NONE,
						node_matrices,
						FALSE,
						&marker,
						1))
					{
						vector_from_points3d(&render.camera.position, &marker.matrix.position, &offset_vector);
						normalize3d(&offset_vector);
					}
					else
					{
						valid = FALSE;
					}
				}
				else
				{
					vector3d_from_euler_angles2d(&offset_vector, &light->radiosity.direction);
				}

				if (valid)
				{
					point_from_line3d(&render.camera.position, &offset_vector, 1023.875f, &marker.matrix.position);
					negate_vector3d(&offset_vector, &marker.matrix.forward);
					perpendicular3d(&marker.matrix.forward, &marker.matrix.up);
					normalize3d(&marker.matrix.up);
					lights_queue_lens_flare(
						light->lens_flare.index,
						&marker.matrix.position,
						&marker.matrix.forward,
						&marker.matrix.up,
						global_real_rgb_white,
						1.f);
				}
			}
		}

		{
			real_matrix4x3 sky_scale_matrix;
			short node_index;
			real const scale = 1.f / 1024.f;

			sky_scale_matrix = *global_identity4x3;
			sky_scale_matrix.scale = scale;
			sky_scale_matrix.position.x = (1.f - scale) * render.camera.position.x;
			sky_scale_matrix.position.y = (1.f - scale) * render.camera.position.y;
			sky_scale_matrix.position.z = (1.f - scale) * render.camera.position.z;

			for (node_index = 0; node_index < model->nodes.count; node_index++)
			{
				matrix4x3_multiply(&sky_scale_matrix, &node_matrices[node_index], &node_matrices[node_index]);
			}
		}

		rasterizer_models_begin(TRUE);
		memset(&lighting, 0, sizeof(lighting));
		lighting.ambient_color = *global_real_rgb_white;
		render_model(
			sky->model.index,
			0.f,
			node_matrices,
			NULL,
			NULL,
			shader_values,
			&lighting,
			&render.camera.position,
			0.f,
			NULL,
			0,
			0,
			FLAG(_rasterizer_geometry_no_sort_bit));
		rasterizer_models_end();
	}

	return;
}
