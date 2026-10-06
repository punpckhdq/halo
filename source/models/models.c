/*
MODELS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "models.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "console.h"
#include "rasterizer_console_vars.h"
#include "shaders.h"
#include "render_debug.h"
#include "triangle_strips.h"

/* ---------- macros */

#define CORTANA_NODE_LIST_CHECKSUM 0x769c097 /* fake name */

/* ---------- structures */

enum
{
	_render_model_pass_solid = 0,
	_render_model_pass_decal,
	_render_model_pass_transparent,
	NUMBER_OF_RENDER_MODEL_PASSES,
};

/* ---------- prototypes */

static void render_model_parts(struct model const *model, char const *region_permutation_indices, struct render_skinning const *skinning, long object_index, short geometry_detail_level_index, short forced_shader_permutation_index, unsigned long flags);
static void model_geometry_part_build_tangent_matrices(struct model_geometry_part *part);

/* ---------- globals */

boolean render_model_no_geometry;
boolean render_model_markers;
boolean render_model_index_counts;
boolean render_model_vertex_counts;
boolean render_model_nodes;

static struct profile_section render_model_section = {"render_model", NONE, TRUE};

/* ---------- public code */

void render_model(
	long model_index,
	real level_of_detail_pixels,
	real_matrix4x3 const *node_matrices,
	char const *region_permutation_indices,
	real_rgb_color const *change_colors,
	real const *function_values,
	struct render_lighting const *lighting,
	real_point3d const *centroid,
	real radius,
	struct render_model_effect const *model_effect,
	long object_index,
	short forced_shader_permutation_index,
	unsigned long flags)
{
	static char default_region_permutation_indices[MAXIMUM_REGIONS_PER_MODEL];
	static struct render_model_effect default_model_effect;
	static real_rgb_color default_change_colors[MAXIMUM_CHANGE_COLORS_PER_MODEL];
	static real default_function_values[MAXIMUM_FUNCTION_VALUES_PER_MODEL];
	struct model *model = model_definition_get(model_index);

	profile_enter(render_model_section);

	match_assert("c:\\halo\\SOURCE\\models\\models.c", 82, lighting);

	rasterizer_model_cortana_hack = model->node_list_checksum==CORTANA_NODE_LIST_CHECKSUM && TEST_FLAG(global_scenario_get()->flags, _scenario_cortana_hack_bit);

	if (level_of_detail_pixels>=model->detail_cutoff_pixels[0] || TEST_FLAG(flags, _render_model_shadow_bit))
	{
		real_matrix4x3 relative_node_matrices[MAXIMUM_NODES_PER_MODEL];
		struct rasterizer_model_begin_parameters model_parameters;
		short geometry_detail_level_index;

		if (!region_permutation_indices)
		{
			region_permutation_indices = default_region_permutation_indices;
		}

		if (!model_effect)
		{
			model_effect = &default_model_effect;
		}

		if (!change_colors)
		{
			change_colors = default_change_colors;
		}

		if (!function_values)
		{
			function_values = default_function_values;
		}

		if (!centroid)
		{
			centroid = &node_matrices->position;
		}

		if (node_matrices)
		{
			short node_index;

			for (node_index = 0; node_index<model->nodes.count; node_index++)
			{
				struct model_node *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node);

				matrix4x3_multiply(&node_matrices[node_index], &node->runtime_default_inverse_matrix, &relative_node_matrices[node_index]);
			}
		}
		else
		{
			short node_index;

			for (node_index = 0; node_index<model->nodes.count; node_index++)
			{
				relative_node_matrices[node_index] = render.frustum.world_to_view;
			}
		}

		geometry_detail_level_index = NUMBER_OF_DETAIL_LEVELS_PER_MODEL-1;
		while (geometry_detail_level_index>0 && level_of_detail_pixels<model->detail_cutoff_pixels[geometry_detail_level_index])
		{
			geometry_detail_level_index--;
		}

		if (rasterizer_debug_options.debug_model_lod!=NONE)
		{
			geometry_detail_level_index = PIN(rasterizer_debug_options.debug_model_lod, 0, NUMBER_OF_DETAIL_LEVELS_PER_MODEL-1);
		}
		match_assert("c:\\halo\\SOURCE\\models\\models.c", 169, geometry_detail_level_index>=0 && geometry_detail_level_index<NUMBER_OF_DETAIL_LEVELS_PER_MODEL);

		if (!TEST_FLAG(flags, _render_model_shadow_bit))
		{
			if (render_model_nodes)
			{
				short node_index;

				for (node_index = 0; node_index<model->nodes.count; node_index++)
				{
					struct model_node *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node);

					if (node->parent_node_index!=NONE)
					{
						render_debug_line(TRUE, &node_matrices[node_index].position, &node_matrices[node->parent_node_index].position, global_real_argb_white);
					}
					render_debug_matrix(TRUE, &node_matrices[node_index], 0.05f);
				}
			}

			if (render_model_markers)
			{
				short marker_index;

				for (marker_index = 0; marker_index<model->markers.count; marker_index++)
				{
					short instance_index;
					struct model_marker *marker = TAG_BLOCK_GET_ELEMENT(&model->markers, marker_index, struct model_marker);

					for (instance_index = 0; instance_index<marker->instances.count; instance_index++)
					{
						struct model_marker_instance *instance = TAG_BLOCK_GET_ELEMENT(&marker->instances, instance_index, struct model_marker_instance);

						if (region_permutation_indices[instance->region_index]==instance->permutation_index)
						{
							real_matrix4x3 marker_matrix;

							matrix4x3_from_point_and_quaternion(&marker_matrix, &instance->translation, &instance->rotation);
							matrix4x3_multiply(&node_matrices[instance->node_index], &marker_matrix, &marker_matrix);
							render_debug_matrix(FALSE, &marker_matrix, 0.05f);
							render_debug_string_at_point(FALSE, &marker_matrix.position, marker->name, global_real_argb_white);
						}
					}
				}
			}

			if (render_model_vertex_counts || render_model_index_counts)
			{
				short region_index;
				boolean triangle_lists = FALSE;
				short maximum_actual_detail_level_index = geometry_detail_level_index;
				short vertex_count = 0;
				short index_count = 0;

				for (region_index = 0; region_index<model->regions.count; region_index++)
				{
					struct model_region *region = TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region);

					if (region_permutation_indices[region_index]!=NONE)
					{
						short actual_detail_level_index;
						short geometry_index;
						struct model_region_permutation *permutation = TAG_BLOCK_GET_ELEMENT(&region->permutations, region_permutation_indices[region_index], struct model_region_permutation);

						for (actual_detail_level_index = geometry_detail_level_index+1;
							actual_detail_level_index<NUMBER_OF_DETAIL_LEVELS_PER_MODEL && permutation->geometry_indices[actual_detail_level_index]==permutation->geometry_indices[geometry_detail_level_index];
							actual_detail_level_index++);
						match_assert("c:\\halo\\SOURCE\\models\\models.c", 247, actual_detail_level_index > 0);
						actual_detail_level_index--;
						match_assert("c:\\halo\\SOURCE\\models\\models.c", 249, (actual_detail_level_index >= 0) && (actual_detail_level_index < NUMBER_OF_DETAIL_LEVELS_PER_MODEL));
						maximum_actual_detail_level_index = MAX(maximum_actual_detail_level_index, actual_detail_level_index);

						geometry_index = permutation->geometry_indices[geometry_detail_level_index];
						if (geometry_index!=NONE)
						{
							short part_index;
							struct model_geometry *geometry = TAG_BLOCK_GET_ELEMENT(&model->geometries, geometry_index, struct model_geometry);

							for (part_index = 0; part_index<geometry->parts.count; part_index++)
							{
								struct model_geometry_part *part = TAG_BLOCK_GET_ELEMENT(&geometry->parts, part_index, struct model_geometry_part);

								vertex_count += part->vertex_buffer.count;
								switch (part->triangle_buffer.type)
								{
								case _triangle_buffer_type_triangles:
									triangle_lists = TRUE;
									index_count += 3*part->triangle_buffer.count;
									break;
								case _triangle_buffer_type_precompiled_strip:
									index_count += part->triangle_buffer.count+2;
									break;
								default:
									match_unreachable("c:\\halo\\SOURCE\\models\\models.c", 276);
								}
							}
						}
					}
				}

				{
					real height = (real)(fabs(centroid->x*render.frustum.world_to_view.forward.k + centroid->y*render.frustum.world_to_view.left.k + centroid->z*render.frustum.world_to_view.up.k + render.frustum.world_to_view.position.z)/render.frustum.projection_world_to_screen.j)*level_of_detail_pixels*0.5f;

					if (height>0.0001f)
					{
						real_argb_color const *detail_level_colors[NUMBER_OF_DETAIL_LEVELS_PER_MODEL] =
						{
							global_real_argb_blue,
							global_real_argb_green,
							global_real_argb_yellow,
							global_real_argb_orange,
							global_real_argb_red
						};
						real_argb_color const *color = detail_level_colors[maximum_actual_detail_level_index];
						char string[256];
						real_point3d point;

						if (triangle_lists && (game_time_get()+model_index)%TICKS_PER_SECOND<TICKS_PER_SECOND/2)
						{
							color = global_real_argb_white;
						}

						strcpy(string, "");
						if (render_model_vertex_counts)
						{
							_snprintf(string+strlen(string), 256-strlen(string), "%d", vertex_count);
						}

						if (render_model_vertex_counts && render_model_index_counts)
						{
							_snprintf(string+strlen(string), 256-strlen(string), "/");
						}

						if (render_model_index_counts)
						{
							_snprintf(string+strlen(string), 256-strlen(string), "%d", index_count);
						}

						set_real_point3d(&point, centroid->x, centroid->y, centroid->z + height);
						render_debug_string_at_point(FALSE, &point, string, color);
					}
				}
			}
		}

		model_parameters.unique_id = object_index;
		model_parameters.lighting = *lighting;
		model_parameters.centroid = *centroid;
		model_parameters.radius = radius;
		model_parameters.effect = *model_effect;
		model_parameters.animation.colors = change_colors;
		model_parameters.animation.values = function_values;
		model_parameters.skinning.node_matrices = relative_node_matrices;
		model_parameters.skinning.node_matrix_count = (short)model->nodes.count;
		model_parameters.geometry_flags = 0;
		model_parameters.base_map_scale = model->base_map_scale;
		if (TEST_FLAG(flags, _render_model_immediate_bit))
		{
			model_parameters.geometry_flags |= FLAG(_rasterizer_geometry_no_sort_bit) | FLAG(_rasterizer_geometry_no_queue_bit) | FLAG(_rasterizer_geometry_no_fog_bit) | FLAG(_rasterizer_geometry_no_zbuffer_bit) | FLAG(_rasterizer_geometry_sky_bit);
		}
		SET_FLAG(model_parameters.geometry_flags, _rasterizer_geometry_atmospheric_fog_but_no_planar_fog_bit, TEST_FLAG(flags, _render_model_no_planar_fog_bit));
		SET_FLAG(model_parameters.geometry_flags, _rasterizer_geometry_first_person_bit, TEST_FLAG(flags, _render_model_first_person_bit));

		if (TEST_FLAG(flags, _render_model_shadow_bit))
		{
			rasterizer_environment_shadow_model_begin(&model_parameters);
		}
		else
		{
			rasterizer_model_begin(&model_parameters, FALSE);
		}

		render_model_parts(model, region_permutation_indices, &model_parameters.skinning, object_index, geometry_detail_level_index, forced_shader_permutation_index, flags);

		if (TEST_FLAG(flags, _render_model_shadow_bit))
		{
			rasterizer_environment_shadow_model_end();
		}
		else
		{
			rasterizer_model_end();
		}
	}

	rasterizer_model_cortana_hack = FALSE;

	profile_exit(render_model_section);

	return;
}

static void render_model_parts(
	struct model const *model,
	char const *region_permutation_indices,
	struct render_skinning const *skinning,
	long object_index,
	short geometry_detail_level_index,
	short forced_shader_permutation_index,
	unsigned long flags)
{
	short pass;
	boolean immediate = TEST_FLAG(flags, _render_model_immediate_bit);
	short last_pass = TEST_FLAG(flags, _render_model_shadow_bit) ? _render_model_pass_solid : _render_model_pass_transparent;

	for (pass = _render_model_pass_solid; pass<=last_pass; pass++)
	{
		struct render_sort_filth sort_filth[32];
		short region_index;
		short sort_filth_count = 0;

		for (region_index = 0; region_index<model->regions.count; region_index++)
		{
			struct model_region *region = TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region);

			if (region_permutation_indices[region_index]!=NONE)
			{
				struct model_region_permutation *permutation = TAG_BLOCK_GET_ELEMENT(&region->permutations, region_permutation_indices[region_index], struct model_region_permutation);
				short geometry_index = permutation->geometry_indices[geometry_detail_level_index];

				if (!render_model_no_geometry && geometry_index!=NONE)
				{
					short part_index;
					struct model_geometry *geometry = TAG_BLOCK_GET_ELEMENT(&model->geometries, geometry_index, struct model_geometry);

					for (part_index = 0; part_index<geometry->parts.count; part_index++)
					{
						struct model_geometry_part *part = TAG_BLOCK_GET_ELEMENT(&geometry->parts, part_index, struct model_geometry_part);
						struct model_shader_reference *shader_reference = TAG_BLOCK_GET_ELEMENT(&model->shaders, part->shader_index, struct model_shader_reference);
						struct shader *shader = tag_get('shdr', shader_reference->shader.index);

						if (shader_type_is_valid_for_model(shader->base.type) && !TEST_FLAG(part->flags, _model_part_stripped_bit))
						{
							if (shader_type_is_transparent(shader->base.type))
							{
								if (pass==_render_model_pass_transparent)
								{
									real_point3d centroid;

									match_assert("c:\\halo\\SOURCE\\models\\models.c", 442, !TEST_FLAG(flags, _render_model_shadow_bit));


									match_assert("c:\\halo\\SOURCE\\models\\models.c", 445, part->centroid_primary_node_index>=0 && part->centroid_primary_node_index<model->nodes.count);
									match_assert("c:\\halo\\SOURCE\\models\\models.c", 446, part->centroid_secondary_node_index>=0 && part->centroid_secondary_node_index<model->nodes.count);
									matrix4x3_transform_point(&skinning->node_matrices[part->centroid_primary_node_index], &part->centroid, &centroid);
									rasterizer_model_transparent_geometry_submit(shader,
										forced_shader_permutation_index ? forced_shader_permutation_index : shader_reference->permutation_index,
										&part->triangle_buffer, NONE, part->triangle_buffer.count, &part->vertex_buffer, NONE, &centroid, &sort_filth[sort_filth_count]);

									if (sort_filth_count<32 && sort_filth[sort_filth_count].group_index!=NONE && !immediate &&
										(part->next_part_index>0 || part->prev_part_index>0))
									{
										sort_filth[sort_filth_count].part_index = part_index;
										sort_filth[sort_filth_count].next_part_index = part->next_part_index;
										sort_filth_count++;
									}
								}
							}
							else if (shader->base.type==_shader_type_model &&
								TEST_FLAG(((struct shader_model *)shader_get_and_verify_type(shader, _shader_type_model))->model.flags, _shader_model_alpha_blended_decal_bit))
							{
								if (pass==_render_model_pass_decal)
								{
									match_assert("c:\\halo\\SOURCE\\models\\models.c", 491, !TEST_FLAG(flags, _render_model_shadow_bit));
									rasterizer_model_draw(shader,
										forced_shader_permutation_index ? forced_shader_permutation_index : shader_reference->permutation_index,
										&part->triangle_buffer, NONE, part->triangle_buffer.count, &part->vertex_buffer, NONE);
								}
							}
							else if (pass==_render_model_pass_solid)
							{
								if (TEST_FLAG(flags, _render_model_shadow_bit))
								{
									rasterizer_environment_shadow_model_draw(shader,
										forced_shader_permutation_index ? forced_shader_permutation_index : shader_reference->permutation_index,
										&part->triangle_buffer, &part->vertex_buffer);
								}
								else
								{
									rasterizer_model_draw(shader,
										forced_shader_permutation_index ? forced_shader_permutation_index : shader_reference->permutation_index,
										&part->triangle_buffer, NONE, part->triangle_buffer.count, &part->vertex_buffer, NONE);
									rasterizer_debug_model_vertices(object_index, skinning, part);
								}
							}
						}
					}
				}
			}
		}

		{
			short sort_filth_index;

			for (sort_filth_index = 0; sort_filth_index<sort_filth_count; sort_filth_index++)
			{
				short other_sort_filth_index;

				for (other_sort_filth_index = 0; other_sort_filth_index<sort_filth_count; other_sort_filth_index++)
				{
					if (sort_filth[sort_filth_index].next_part_index==sort_filth[other_sort_filth_index].part_index && sort_filth[sort_filth_index].next_part_index>0)
					{
						*sort_filth[sort_filth_index].next_group_presorted_index_reference = sort_filth[other_sort_filth_index].group_index;
						*sort_filth[other_sort_filth_index].prev_group_presorted_index_reference = sort_filth[sort_filth_index].group_index;
						break;
					}
				}
			}
		}
	}

	return;
}

void model_interpolate_node_orientations(
	struct model *model,
	real_orientation *original_node_orientations,
	real_orientation *target_node_orientations,
	short frame_index,
	short frame_count)
{
	short node_index;
	real fraction = (real)(frame_index+1)/(real)frame_count;
	real inverse_fraction = 1.f - fraction;

	match_assert("c:\\halo\\SOURCE\\models\\models.c", 579, frame_count>0);
	match_assert("c:\\halo\\SOURCE\\models\\models.c", 580, frame_index<frame_count);

	for (node_index = 0; node_index<model->nodes.count; node_index++)
	{
		real_orientation *target = &target_node_orientations[node_index];
		real_orientation *original = &original_node_orientations[node_index];

		target->scale = inverse_fraction*original->scale + fraction*target->scale;
		quaternions_interpolate_and_normalize(&original->rotation, &target->rotation, fraction, &target->rotation);
		target->translation.x = inverse_fraction*original->translation.x + fraction*target->translation.x;
		target->translation.y = inverse_fraction*original->translation.y + fraction*target->translation.y;
		target->translation.z = inverse_fraction*original->translation.z + fraction*target->translation.z;
	}

	return;
}

void model_get_node_orientations(
	struct model const *model,
	real_orientation *node_orientations)
{
	short node_index;

	for (node_index = 0; node_index<model->nodes.count; node_index++)
	{
		struct model_node *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node);

		node_orientations[node_index].rotation = node->default_rotation;
		node_orientations[node_index].translation = node->default_translation;
		node_orientations[node_index].scale = 1.f;
	}

	return;
}

void model_get_node_matrices(
	struct model const *model,
	real_matrix4x3 *node_matrices,
	real_point3d const *origin,
	real_vector3d const *forward,
	real_vector3d const *up)
{
	short node_index_stack[MAXIMUM_NODES_PER_MODEL];
	real_matrix4x3 node_matrix;
	short stack_count;
	short stack_index = 0;

	node_index_stack[0] = 0;
	stack_count = 1;

	while (stack_index!=stack_count)
	{
		short node_index = node_index_stack[stack_index++];
		struct model_node *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node);

		matrix4x3_from_point_and_quaternion(&node_matrix, &node->default_translation, &node->default_rotation);
		if (node_index==0)
		{
			matrix4x3_from_point_and_vectors(&node_matrices[node_index],
				origin ? origin : global_origin3d,
				forward ? forward : global_forward3d,
				up ? up : global_up3d);
			matrix4x3_multiply(&node_matrices[node_index], &node_matrix, &node_matrices[node_index]);
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\models\\models.c", 650, node->parent_node_index!=NONE);
			matrix4x3_multiply(&node_matrices[node->parent_node_index], &node_matrix, &node_matrices[node_index]);
		}

		if (node->next_sibling_node_index!=NONE)
		{
			node_index_stack[stack_count++] = node->next_sibling_node_index;
		}

		if (node->first_child_node_index!=NONE)
		{
			node_index_stack[stack_count++] = node->first_child_node_index;
		}
	}

	return;
}

void model_node_matrices_from_orientations(
	struct model const *model,
	real_matrix4x3 *node_matrices,
	real_orientation const *node_orientations,
	real_point3d const *origin,
	real_vector3d const *forward,
	real_vector3d const *up)
{
	real_matrix4x3 root_matrix;
	real_matrix4x3 orientation_matrix;
	short node_stack[MAXIMUM_NODES_PER_MODEL];

	matrix4x3_from_point_and_vectors(&root_matrix, origin, forward, up);
	if (model->nodes.count>0)
	{
		short stack_index = 0;
		short stack_count = 1;

		node_stack[0] = 0;

		while (stack_index!=stack_count)
		{
			short node_index = node_stack[stack_index++];
			struct model_node *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node);
			real_matrix4x3 *parent_matrix = node_index==0 ? &root_matrix : &node_matrices[node->parent_node_index];

			matrix4x3_from_orientation(&orientation_matrix, &node_orientations[node_index]);
			matrix4x3_multiply(parent_matrix, &orientation_matrix, &node_matrices[node_index]);

			if (node->next_sibling_node_index!=NONE)
			{
				node_stack[stack_count++] = node->next_sibling_node_index;
			}

			if (node->first_child_node_index!=NONE)
			{
				node_stack[stack_count++] = node->first_child_node_index;
			}
		}
	}

	return;
}

short model_find_marker(
	long model_index,
	char const *name)
{
	if (model_index!=NONE && name && name[0])
	{
		struct model *model = model_definition_get(model_index);
		short low = 0;
		short high = model->markers.count-1;

		while (low<=high)
		{
			short middle = (low+high)/2;
			struct model_marker *marker = TAG_BLOCK_GET_ELEMENT(&model->markers, middle, struct model_marker);
			long comparison = _stricmp(name, marker->name);

			if (comparison)
			{
				if (comparison<0)
				{
					high = middle-1;
				}
				else
				{
					low = middle+1;
				}
			}
			else
			{
				return middle;
			}
		}
	}

	return NONE;
}

short model_get_marker_by_name(
	long model_index,
	char const *name,
	byte const *region_permutations,
	short const *node_remapping_table,
	short node_count,
	real_matrix4x3 const *node_matrices,
	boolean mirrored_flag,
	struct object_marker *markers,
	short maximum_marker_count)
{
	short result = 0;
	short marker_index = model_find_marker(model_index, name);

	match_assert("c:\\halo\\SOURCE\\models\\models.c", 760, node_matrices);
	match_assert("c:\\halo\\SOURCE\\models\\models.c", 761, markers);

	if (marker_index!=NONE)
	{
		short i;

		struct model *model = model_definition_get(model_index);
		struct model_marker *marker = TAG_BLOCK_GET_ELEMENT(&model->markers, marker_index, struct model_marker);

		for (i = 0; i<marker->instances.count; i++)
		{
			struct model_marker_instance *instance = TAG_BLOCK_GET_ELEMENT(&marker->instances, i, struct model_marker_instance);
			if (!region_permutations ||
				region_permutations[instance->region_index]==instance->permutation_index)
			{
				struct object_marker *object_marker;

				if (result>=maximum_marker_count)
				{
					break;
				}

				object_marker = &markers[result++];
				object_marker->node_index = node_remapping_table ? node_remapping_table[instance->node_index] : instance->node_index;
				matrix4x3_from_point_and_quaternion(&object_marker->node_matrix, &instance->translation, &instance->rotation);
				match_assert(
					"c:\\halo\\SOURCE\\models\\models.c",
					785,
					object_marker->node_index>=0 && object_marker->node_index<(node_remapping_table ? node_count : model->nodes.count));
				
				matrix4x3_multiply(&node_matrices[object_marker->node_index], &object_marker->node_matrix, &object_marker->matrix);
				if (mirrored_flag)
				{
					negate_vector3d(&object_marker->matrix.left, &object_marker->matrix.left);
				}
			}
		}
	}

	return result;
}

real_matrix4x3 const *model_get_default_inverse_matrix(
	struct model const *model,
	short node_index)
{
	return &TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node)->runtime_default_inverse_matrix;
}

short model_find_node(
	long model_index,
	char const *name)
{
	if (model_index!=NONE)
	{
		short node_index;
		struct model *model = model_definition_get(model_index);

		for (node_index = 0; node_index<model->nodes.count; node_index++)
		{
			struct model_node *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node);

			if (!strcmp(node->name, name))
			{
				return node_index;
			}
		}
	}

	return NONE;
}

void model_build_tangent_matrices(
	struct model *model)
{
	short geometry_index;

	for (geometry_index = 0; geometry_index<model->geometries.count; geometry_index++)
	{
		short part_index;
		struct model_geometry *geometry = TAG_BLOCK_GET_ELEMENT(&model->geometries, geometry_index, struct model_geometry);

		for (part_index = 0; part_index<geometry->parts.count; part_index++)
		{
			model_geometry_part_build_tangent_matrices(TAG_BLOCK_GET_ELEMENT(&geometry->parts, part_index, struct model_geometry_part));
		}
	}

	return;
}

/* ---------- private code */

static void model_geometry_part_build_tangent_matrices(
	struct model_geometry_part *part)
{
	return;
}
