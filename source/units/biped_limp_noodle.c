/*
BIPED_LIMP_NOODLE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bipeds.h"
#include "console.h"
#include "collisions.h"
#include "render_debug.h"
#include "physics_constants.h"
#include "biped_limp_noodle.h"
#include "biped_definitions.h"
#include "model_definitions.h"
#include "model_animation_definitions.h"
#include "collision_usage.h"

/* ---------- constants */

enum
{
	MAXIMUM_LIMP_NOODLE_RELAXATION_ITERATIONS = 20, /* fake name */
	NUMBER_OF_LIMP_NOODLE_CONSTRAINT_PASSES = 4, /* fake name */
};

/* ---------- macros */

#define LIMP_NOODLE_GRAVITY (-0.032086615f) /* fake name */
#define LIMP_NOODLE_COLLISION_FLAGS (_collision_test_for_bipeds_dead_flags | FLAG(_collision_test_ignore_invisible_surfaces_bit)) /* fake name */

/* ---------- structures */

/* ---------- prototypes */

static boolean biped_limp_noodle_valid_joint_rotation(long biped_index, long node_index, real_matrix4x3 *node_matrices, real_point3d *current_world_pos, real_point3d *new_world_pos, long *moved_nodes_flags);
static void biped_limp_noodle_move_relax_and_constrain_positions(long biped_index, real_matrix4x3 *node_matrices);
static void biped_limp_noodle_adjust_orientations(long biped_index, real_matrix4x3 *node_matrices, long node_count, real_point3d *last_positions);

/* ---------- globals */

/* ---------- public code */

boolean biped_limp_noodle_relax_nodes_onto_environment(
	long biped_index)
{
	static real_point3d last_positions[MAXIMUM_NODES_PER_ANIMATION];
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(biped_definition->object.animation_graph.index);
	struct object_datum *object = object_get(biped_index);
	real_matrix4x3 *node_matrices = (real_matrix4x3 *)object_header_block_get(biped_index, &object->object.node_matrices);
	boolean finished = biped->biped.limp_body_current_relaxation_iterations >= biped->biped.limp_body_max_relaxation_iterations;

	if (!finished)
	{
		long node_index;

		for (node_index = 0; node_index < animation_graph->nodes.count; node_index++)
		{
			last_positions[node_index] = node_matrices[node_index].position;
		}

		biped_limp_noodle_move_relax_and_constrain_positions(biped_index, node_matrices);
		biped_limp_noodle_adjust_orientations(biped_index, node_matrices, animation_graph->nodes.count, last_positions);

		if (biped->biped.limp_body_current_relaxation_iterations < UNSIGNED_CHAR_MAX/2)
		{
			biped->biped.limp_body_current_relaxation_iterations++;
		}
	}

	return finished;
}

long biped_limp_noodle_get_max_relaxation_iterations(
	long biped_index)
{
	return MAXIMUM_LIMP_NOODLE_RELAXATION_ITERATIONS;
}

static boolean biped_limp_noodle_valid_joint_rotation(
	long biped_index,
	long node_index,
	real_matrix4x3 *node_matrices,
	real_point3d *current_world_pos,
	real_point3d *new_world_pos,
	long *moved_nodes_flags)
{
	boolean valid = FALSE;
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(biped_definition->object.animation_graph.index);
	struct animation_graph_node *node = TAG_BLOCK_GET_ELEMENT(&animation_graph->nodes, node_index, struct animation_graph_node);
	struct animation_graph_node *parent_node = TAG_BLOCK_GET_ELEMENT(&animation_graph->nodes, node->parent_node_index, struct animation_graph_node);
	real node_radius = animation_graph->limp_body_node_collision_radius;

	if (realcmp(node_radius, 0.f) || node_radius < 0.f || node_radius > 0.07f)
	{
		node_radius = 0.03f;
	}

	if (node->parent_node_index && !TEST_FLAG(parent_node->flags, _joint_type_no_movement))
	{
		real_vector3d old_vector;
		real_vector3d new_vector;
		real_vector3d rotation_axis;
		real cosine;

		vector_from_points3d(&node_matrices[node->parent_node_index].position, &node_matrices[node_index].position, &old_vector);
		vector_from_points3d(&node_matrices[node->parent_node_index].position, new_world_pos, &new_vector);
		normalize3d(&old_vector);
		normalize3d(&new_vector);
		cross_product3d(&old_vector, &new_vector, &rotation_axis);
		normalize3d(&rotation_axis);
		cosine = dot_product3d(&old_vector, &new_vector);

		if (!realcmp(cosine, 1.f))
		{
			real angle = arccosine(cosine);
			real_matrix4x3 parent_inverse;
			real_matrix4x3 parent_parent_inverse;
			real_vector3d local_space_rotation_axis;
			real_vector3d local_forward;

			matrix4x3_inverse(&node_matrices[node->parent_node_index], &parent_inverse);
			matrix4x3_inverse(&node_matrices[parent_node->parent_node_index], &parent_parent_inverse);
			matrix4x3_transform_vector(&parent_inverse, &rotation_axis, &local_space_rotation_axis);
			local_forward = node_matrices[node->parent_node_index].forward;
			matrix4x3_transform_vector(&parent_parent_inverse, &local_forward, &local_forward);

			if (TEST_FLAG(parent_node->flags, _joint_type_hinge))
			{
				real_vector3d world_space_hinge_rotation_axis = node_matrices[node->parent_node_index].up;
				real_plane3d plane;
				real_point3d rotate_to_position;
				real_vector3d new_world_hinge_rotation_vector;
				real_vector3d new_local_hinge_rotation_vector;

				plane3d_from_point_and_normal(&plane, &node_matrices[node->parent_node_index].position, &world_space_hinge_rotation_axis);
				point_from_line3d(new_world_pos, &world_space_hinge_rotation_axis, -1.f * plane3d_distance_to_point(&plane, new_world_pos), &rotate_to_position);
				match_assert("c:\\halo\\SOURCE\\units\\biped_limp_noodle.c", 231, realcmp(plane3d_distance_to_point(&plane, &rotate_to_position), 0.f));

				vector_from_points3d(&node_matrices[node->parent_node_index].position, &rotate_to_position, &new_world_hinge_rotation_vector);
				normalize3d(&new_world_hinge_rotation_vector);
				matrix4x3_transform_vector(&parent_inverse, &new_world_hinge_rotation_vector, &new_local_hinge_rotation_vector);

				if (!realcmp(dot_product3d(&new_local_hinge_rotation_vector, &parent_node->base_vector), 1.f))
				{
					BIT_VECTOR_SET_FLAG(moved_nodes_flags, node_index, TRUE);
					if (current_world_pos->z >= rotate_to_position.z && !collision_test_sphere(&rotate_to_position, node_radius, biped_index))
					{
						*current_world_pos = rotate_to_position;
					}
					valid = TRUE;
				}
			}
			else
			{
				rotate_vector_about_axis(&local_forward, &local_space_rotation_axis, sine(angle), cosine);
				cosine = dot_product3d(&local_forward, &parent_node->base_vector);

				if (!realcmp(cosine, 1.f) && fabs(arccosine(cosine)) < parent_node->range && current_world_pos->z > new_world_pos->z)
				{
					BIT_VECTOR_SET_FLAG(moved_nodes_flags, node_index, TRUE);
					*current_world_pos = *new_world_pos;
					valid = TRUE;
				}
			}
		}
	}

	if ((TEST_FLAG(parent_node->flags, _joint_type_no_movement) || !valid) &&
		BIT_VECTOR_TEST_FLAG(moved_nodes_flags, node->parent_node_index) &&
		current_world_pos->z > new_world_pos->z)
	{
		BIT_VECTOR_SET_FLAG(moved_nodes_flags, node_index, TRUE);
		*current_world_pos = *new_world_pos;
		valid = TRUE;
	}

	return valid;
}

static void biped_limp_noodle_move_relax_and_constrain_positions(
	long biped_index,
	real_matrix4x3 *node_matrices)
{
	static struct collision_feature_list features;
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(biped_definition->object.animation_graph.index);
	real node_radius = animation_graph->limp_body_node_collision_radius;
	boolean finished = FALSE;

	match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\biped_limp_noodle.c", 319, _collision_user_limp_body_physics);

	if (realcmp(node_radius, 0.f) || node_radius < 0.f || node_radius > 0.07f)
	{
		node_radius = 0.03f;
	}

	if (biped->biped.limp_body_max_relaxation_iterations > 0 && biped->biped.limp_body_max_relaxation_iterations < 30)
	{
		real relaxation_fraction;

		finished = biped->biped.limp_body_current_relaxation_iterations >= biped->biped.limp_body_max_relaxation_iterations;
		relaxation_fraction = (real)(biped->biped.limp_body_current_relaxation_iterations + 1) / (real)biped->biped.limp_body_max_relaxation_iterations;

		if (!realcmp(relaxation_fraction, 0.f))
		{
			if (!finished)
			{
				long moved_nodes_flags[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_NODES_PER_ANIMATION)];
				long pass;

				collision_get_features_in_sphere(LIMP_NOODLE_COLLISION_FLAGS, &biped->object.position, biped->object.bounding_sphere_radius + 0.0625f, 0.f, node_radius, biped_index, &features);
				memset(moved_nodes_flags, 0, sizeof(moved_nodes_flags));

				for (pass = 0; pass < NUMBER_OF_LIMP_NOODLE_CONSTRAINT_PASSES; pass++)
				{
					short node_stack[MAXIMUM_NODES_PER_ANIMATION];
					short stack_bottom = 0;
					short stack_top = 1;
					struct collision_plane collisions[10];
					real_point3d *node_position;
					real_point3d *parent_position;
					real_vector3d to_node;
					real_vector3d vel;

					node_stack[0] = 0;
					while (stack_bottom != stack_top)
					{
						short node_index = node_stack[stack_bottom++];
						struct animation_graph_node *node = TAG_BLOCK_GET_ELEMENT(&animation_graph->nodes, node_index, struct animation_graph_node);

						if (node_index)
						{
							struct collision_result collision;
							real_point3d parent_sphere_edge;
							struct model *model;
							real desired_distance;
							real distance;

							set_real_vector3d(&vel, 0.f, 0.f, LIMP_NOODLE_GRAVITY * relaxation_fraction);
							node_position = &node_matrices[node_index].position;
							parent_position = &node_matrices[node->parent_node_index].position;
							vector_from_points3d(parent_position, node_position, &to_node);

							if (!pass && !collision_test_sphere(node_position, node_radius, biped_index))
							{
								real_point3d new_pos;
								real_vector3d new_pos_vec;

								collision_move_point(node_position, &vel, &features, &new_pos, &new_pos_vec, 3, collisions);
								if (realcmp(new_pos_vec.i, 0.f) && realcmp(new_pos_vec.j, 0.f))
								{
									biped_limp_noodle_valid_joint_rotation(biped_index, node_index, node_matrices, node_position, &new_pos, moved_nodes_flags);
								}
							}

							vector_from_points3d(parent_position, node_position, &to_node);
							point_from_line3d(parent_position, &to_node, -0.015f, &parent_sphere_edge);
							scale_vector3d(&to_node, 1.03f, &to_node);

							if (collision_test_vector(LIMP_NOODLE_COLLISION_FLAGS, &parent_sphere_edge, &to_node, biped_index, &collision))
							{
								boolean outside_geometry[2];
								long outside_count;

								outside_geometry[0] = collision_test_sphere(node_position, 0.03f, biped_index);
								outside_geometry[1] = collision_test_sphere(parent_position, 0.03f, biped_index);
								outside_count = outside_geometry[0] + outside_geometry[1];

								if (outside_count)
								{
									real movement[2];
									long point_index;

									if (outside_count == 2)
									{
										for (point_index = 0; point_index < 2; point_index++)
										{
											movement[point_index] = vector_intersect_plane3d(!point_index ? node_position : parent_position, &collision.plane.n, &collision.plane);
											if (movement[point_index] != 0.f)
											{
												movement[point_index] = node_radius * 2.5f + movement[point_index];
											}
										}
									}
									else
									{
										for (point_index = 0; point_index < 2; point_index++)
										{
											if (outside_geometry[point_index])
											{
												movement[point_index] = vector_intersect_plane3d(!point_index ? node_position : parent_position, &collision.plane.n, &collision.plane);
												if (movement[point_index] != 0.f)
												{
													movement[point_index] = node_radius * 2.5f + movement[point_index];
												}
											}
											else
											{
												movement[point_index] = 0.f;
											}
										}
									}

									for (point_index = 0; point_index < 2; point_index++)
									{
										if (!realcmp(movement[point_index], 0.f))
										{
											point_from_line3d(!point_index ? node_position : parent_position, &collision.plane.n, movement[point_index] * relaxation_fraction, !point_index ? node_position : parent_position);
										}
									}
								}
							}

							vector_from_points3d(parent_position, node_position, &to_node);
							model = model_definition_get(biped_definition->object.model.index);
							desired_distance = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node)->node_distance_from_parent;
							distance = distance3d(node_position, parent_position);

							if (!(desired_distance <= 0.f || desired_distance > 10.f) && distance >= 0.f && distance < 20.f && !realcmp(distance, 0.f))
							{
								if (!realcmp(desired_distance, 0.f) && desired_distance != distance && !realcmp(distance, 0.f))
								{
									real scale = (desired_distance - distance) / distance;

									if (node->parent_node_index)
									{
										real half_scale = scale / 2.f;

										scale_vector3d(&to_node, -half_scale, &vel);
										collision_move_point(parent_position, &vel, &features, parent_position, &vel, 3, collisions);
										scale_vector3d(&to_node, half_scale, &vel);
										collision_move_point(node_position, &vel, &features, node_position, &vel, 3, collisions);
									}
									else
									{
										scale_vector3d(&to_node, scale, &vel);
										if (!collision_test_sphere(node_position, node_radius, biped_index))
										{
											collision_move_point(node_position, &vel, &features, node_position, &vel, 3, collisions);
										}
									}
								}
							}
							else
							{
								if (node->next_sibling_node_index != NONE)
								{
									node_stack[stack_top++] = node->next_sibling_node_index;
								}

								if (node->first_child_node_index != NONE)
								{
									node_stack[stack_top++] = node->first_child_node_index;
								}

								continue;
							}
						}

						if (node->next_sibling_node_index != NONE)
						{
							node_stack[stack_top++] = node->next_sibling_node_index;
						}

						if (node->first_child_node_index != NONE)
						{
							node_stack[stack_top++] = node->first_child_node_index;
						}
					}
				}
			}

			match_collision_log_end_user("c:\\halo\\SOURCE\\units\\biped_limp_noodle.c", 530);
		}
	}

	return;
}

void validate_real_vector3d_axes3(
	real_vector3d *forward,
	real_vector3d *left,
	real_vector3d *up)
{
	match_assert("c:\\halo\\SOURCE\\units\\biped_limp_noodle.c", 535, forward);
	match_assert("c:\\halo\\SOURCE\\units\\biped_limp_noodle.c", 535, left);
	match_assert("c:\\halo\\SOURCE\\units\\biped_limp_noodle.c", 535, up);

	if (normalize3d(forward) == 0.f)
	{
		*forward = *global_forward3d;
	}

	if (normalize3d(up) == 0.f)
	{
		*up = *global_up3d;
	}

	cross_product3d(up, forward, left);
	if (normalize3d(left) == 0.f)
	{
		*left = *global_left3d;
	}

	cross_product3d(forward, left, up);
	if (normalize3d(up) == 0.f)
	{
		*up = *global_up3d;
	}

	cross_product3d(up, forward, left);
	if (normalize3d(left) == 0.f)
	{
		*left = *global_left3d;
	}

	return;
}

static void biped_limp_noodle_adjust_orientations(
	long biped_index,
	real_matrix4x3 *node_matrices,
	long node_count,
	real_point3d *last_positions)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(biped_definition->object.animation_graph.index);
	long node_index;

	for (node_index = 0; node_index < animation_graph->nodes.count; node_index++)
	{
		if (node_index)
		{
			struct animation_graph_node *node = TAG_BLOCK_GET_ELEMENT(&animation_graph->nodes, node_index, struct animation_graph_node);
			struct animation_graph_node *parent_node = TAG_BLOCK_GET_ELEMENT(&animation_graph->nodes, node->parent_node_index, struct animation_graph_node);
			real_vector3d old_vector;
			real_vector3d new_vector;
			real_vector3d rotation_axis;
			real cosine;
			real sine_angle;

			if (!TEST_FLAG(parent_node->flags, _joint_type_no_movement))
			{
				vector_from_points3d(&last_positions[node->parent_node_index], &last_positions[node_index], &old_vector);
				vector_from_points3d(&node_matrices[node->parent_node_index].position, &node_matrices[node_index].position, &new_vector);
				normalize3d(&old_vector);
				normalize3d(&new_vector);
				cross_product3d(&old_vector, &new_vector, &rotation_axis);
				normalize3d(&rotation_axis);
				cosine = dot_product3d(&old_vector, &new_vector);

				if (!realcmp(cosine, 1.f))
				{
					real angle = arccosine(cosine);

					if (!realcmp(angle, 0.f) && fabs(angle) < _pi/4.f)
					{
						if (!valid_real_vector3d_axes3(&node_matrices[node->parent_node_index].forward, &node_matrices[node->parent_node_index].left, &node_matrices[node->parent_node_index].up))
						{
							validate_real_vector3d_axes3(&node_matrices[node->parent_node_index].forward, &node_matrices[node->parent_node_index].left, &node_matrices[node->parent_node_index].up);
						}

						sine_angle = sine(angle);
						rotate_vector_about_axis(&node_matrices[node->parent_node_index].forward, &rotation_axis, sine_angle, cosine);
						rotate_vector_about_axis(&node_matrices[node->parent_node_index].up, &rotation_axis, sine_angle, cosine);
						normalize3d(&node_matrices[node->parent_node_index].forward);
						normalize3d(&node_matrices[node->parent_node_index].up);
						cross_product3d(&node_matrices[node->parent_node_index].up, &node_matrices[node->parent_node_index].forward, &node_matrices[node->parent_node_index].left);
						normalize3d(&node_matrices[node->parent_node_index].left);

						if (!valid_real_vector3d_axes3(&node_matrices[node->parent_node_index].forward, &node_matrices[node->parent_node_index].left, &node_matrices[node->parent_node_index].up))
						{
							validate_real_vector3d_axes3(&node_matrices[node->parent_node_index].forward, &node_matrices[node->parent_node_index].left, &node_matrices[node->parent_node_index].up);
						}
					}
				}
			}
		}
	}

	return;
}

/* ---------- private code */
