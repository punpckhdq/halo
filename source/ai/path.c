/*
PATH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "path.h"
#include "ai_constants.h"
#include "ai_debug.h"
#include "ai_profile.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "console.h"

/* ---------- constants */

enum
{
	PATH_HASH_TABLE_SIZE_PER_KEY = 8,
	PATH_HASH_KEY_MASK = 511,
	PATH_HASH_TABLE_MASK = 4095,
	MAXIMUM_PATH_EDGES_PER_COLLISION_SURFACE = 64
};

/* ---------- macros */

#define PATH_COST_ESTIMATE_GRANULARITY 0.1f

/* ---------- structures */

struct path_edge
{
	long adjacent_surface_index;
	byte adjacent_pathfinding_surface;
	real_point3d base_point;
	real_vector3d edge_vector;
};

/* ---------- prototypes */

static void path_state_reset(struct path_state *state);
static boolean path_state_begin(struct path_state *state);
static boolean path_state_traverse(struct path_state *state);
static boolean path_heap_verify(struct path_state *state, short node_index, short heap_location);
static void path_heap_bubble_up(struct path_state *state, short heap_location);
static void path_heap_bubble_down(struct path_state *state, short heap_location);
static short path_heap_pop_cheapest_node(struct path_state *state);
static void path_heap_insert(struct path_state *state, short node_index, short quantized_cost_estimate);
static real closest_available_point_on_surface(struct structure_bsp const *structure, long surface_index, real_point3d const *target_point, real_point3d *closest_point);
static boolean surface_is_broken(struct structure_bsp const *structure, long surface_index);
static short build_path_edges_for_surface(struct structure_bsp const *structure, long surface_index, struct path_edge *edges);

/* ---------- public code */

void paths_initialize(
	void)
{
	return;
}

void paths_dispose(
	void)
{
	return;
}

void paths_initialize_for_new_map(
	void)
{
	return;
}

void paths_dispose_from_old_map(
	void)
{
	return;
}

void path_input_new(
	struct path_input *input,
	real pathfinding_radius,
	boolean ignore_broken_surfaces,
	long source_object_index)
{
	memset(input, 0, sizeof(*input));
	input->pathfinding_radius = pathfinding_radius;
	input->ignore_broken_surfaces = ignore_broken_surfaces;
	input->ignore_source_object_index = source_object_index;
	input->ignore_target_object_index = NONE;

	return;
}

void path_input_set_target_object(
	struct path_input *input,
	long target_object_index)
{
	input->ignore_target_object_index = target_object_index;

	return;
}

void path_input_set_start(
	struct path_input *input,
	real_point3d const *start_point,
	long start_surface_index)
{
	input->start_valid = TRUE;
	input->start_point = *start_point;
	input->start_surface_index = start_surface_index;

	return;
}

void path_input_set_attractor(
	struct path_input *input,
	real_point3d const *attractor_point,
	real radius,
	long object_index,
	real weight)
{
	input->attractor_valid = TRUE;
	input->attractor_point = *attractor_point;
	input->attractor_radius = radius;
	input->attractor_object_index = object_index;
	input->attractor_weight = weight;

	return;
}

void path_input_set_search_bounds(
	struct path_input *input,
	real maximum_distance)
{
	input->search_bounded = TRUE;
	input->search_maximum_distance = maximum_distance;

	return;
}

boolean path_3d_available(
	struct structure_bsp *structure_bsp,
	real_point3d const *start_point,
	real avoidance_distance,
	real_point3d const *end_point,
	boolean *finishing_path_reference,
	real_point3d *path_endpoint)
{
	struct collision_bsp_test_vector_result result;
	real_vector3d path_vector;
	boolean available = FALSE;
	boolean finishing_path = FALSE;
	real_point3d found_endpoint = *end_point;

	vector_from_points3d(start_point, end_point, &path_vector);

	if (!collision_bsp_test_vector(
		FLAG(_collision_bsp_test_front_facing_surfaces_bit),
		TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp),
		0,
		NULL,
		start_point,
		&path_vector,
		REAL_MAX,
		&result) ||
		result.t >= 1.f ||
		magnitude_squared3d(&path_vector) * ((1.f - result.t) * (1.f - result.t)) < 0.1f)
	{
		available = TRUE;
		finishing_path = TRUE;
	}

	if (finishing_path_reference)
	{
		*finishing_path_reference = finishing_path;
	}

	if (path_endpoint)
	{
		*path_endpoint = found_endpoint;
	}

	return available;
}

boolean path_3d_build_path(
	struct structure_bsp *structure_bsp,
	real_point3d const *start_point,
	real avoidance_distance,
	real_point3d const *end_point,
	struct path_result *path)
{
	boolean finishing_path;
	real_point3d path_endpoint;

	memset(path, 0, sizeof(*path));

	if (path_3d_available(structure_bsp, start_point, avoidance_distance, end_point, &finishing_path, &path_endpoint))
	{
		path->step_count = 1;
		path->steps[0].point = path_endpoint;
		path->steps[0].surface_index = NONE;
		path->step_index = 0;
		path->steps_finish_path = finishing_path;
		path->endpoint.point = *end_point;
		path->endpoint.surface_index = NONE;
		path->endpoint.target_radius = 0.f;
		path->valid = TRUE;
	}

	return path->valid;
}

boolean path_state_approach_point(
	struct path_state *state,
	real_point3d const *end_point,
	long end_surface_index,
	boolean *straight_line_reference,
	real_point3d *approach_point_reference)
{
	short node_index = path_node_from_hash_table(state, end_surface_index);
	boolean success = FALSE;

	if (node_index != NONE)
	{
		struct path_node *node;

		for (node = path_get_node(state, node_index); node->parent_node_index != NONE; node = path_get_node(state, node_index))
		{
			struct path_collision_result collision_result;
			struct path_node *parent_node = path_get_node(state, node->parent_node_index);

			if (structure_test_line2d(
				state->structure,
				state->input.ignore_broken_surfaces,
				(real_point2d const *)end_point,
				end_surface_index,
				(real_point2d const *)&parent_node->entry_point,
				parent_node->surface_index,
				&collision_result))
			{
				break;
			}

			node_index = node->parent_node_index;
		}

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 301, approach_point_reference);
		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 302, straight_line_reference);

		if (node->parent_node_index == NONE)
		{
			*straight_line_reference = TRUE;
			*approach_point_reference = state->input.start_point;
		}
		else
		{
			*straight_line_reference = FALSE;
			*approach_point_reference = node->entry_point;
		}

		success = TRUE;
	}

	return success;
}

boolean path_state_estimated_distance(
	struct path_state *state,
	real_point3d const *end_point,
	long end_surface_index,
	real *distance_reference,
	real *closest_approach_to_attractor_reference,
	real_vector3d *estimated_direction_reference)
{
	boolean success;
	short node_index = path_node_from_hash_table(state, end_surface_index);

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 334, distance_reference);

	if (node_index != NONE)
	{
		real closest_approach_to_attractor;
		struct path_node *node = path_get_node(state, node_index);
		real distance = distance3d(&node->entry_point, end_point) + node->path_distance_from_origin;

		if (state->input.attractor_valid)
		{
			real_point3d closest_point;

			closest_point_to_attractor(&node->entry_point, end_point, &state->input.attractor_point, &closest_point);
			closest_approach_to_attractor = distance3d(&state->input.attractor_point, &closest_point);
			closest_approach_to_attractor = MIN(closest_approach_to_attractor, node->closest_approach_to_attractor);
		}
		else
		{
			closest_approach_to_attractor = 0.f;
		}

		if (closest_approach_to_attractor_reference)
		{
			*closest_approach_to_attractor_reference = closest_approach_to_attractor;
		}

		*distance_reference = distance;
		success = TRUE;

		if (estimated_direction_reference)
		{
			real_point3d const *target_point;
			short index = node_index;
			short child_node_index = NONE;
			real accumulated_distance = 0.f;

			while (index != NONE)
			{
				node = path_get_node(state, index);
				node->child_node_index = child_node_index;
				child_node_index = index;
				index = node->parent_node_index;
			}

			for (index = child_node_index; index != NONE && accumulated_distance < 0.8f; index = node->child_node_index)
			{
				node = path_get_node(state, index);
				accumulated_distance += node->linear_distance_to_entry_point;
			}

			if (index == NONE)
			{
				target_point = end_point;
			}
			else
			{
				target_point = &node->entry_point;
			}

			vector_from_points3d(&state->input.start_point, target_point, estimated_direction_reference);
			normalize3d(estimated_direction_reference);
		}
	}
	else
	{
		if (closest_approach_to_attractor_reference)
		{
			*closest_approach_to_attractor_reference = REAL_MAX;
		}

		if (estimated_direction_reference)
		{
			*estimated_direction_reference = *global_zero_vector3d;
		}

		*distance_reference = REAL_MAX;
		success = FALSE;
	}

	return success;
}

boolean path_state_build_path(
	struct path_state *state,
	struct path_result *path)
{
	if (state->debug)
	{
		state->debug->path_build_result = _path_build_result_none;
	}

	path->valid = FALSE;

	if (state->destination_valid)
	{
		struct path_node const *node;
		short node_index = path_node_from_hash_table(state, state->destination.surface_index);

		if (node_index != NONE)
		{
			node = path_get_node(state, node_index);
			path->endpoint = state->destination;
			path->endpoint.target_radius = 0.f;
		}
		else if (state->closest_distance < state->destination.target_radius)
		{
			node_index = state->closest_node_index;
			node = path_get_node(state, node_index);
			path->endpoint.point = state->closest_point;
			path->endpoint.surface_index = node->surface_index;
			path->endpoint.target_radius = state->closest_distance;
		}

		if (node_index != NONE)
		{
			struct path_step raw_steps[MAXIMUM_RAW_PATH_STEPS];
			struct path_step smoothed_steps[MAXIMUM_SMOOTHED_PATH_STEPS];
			struct path_step avoided_steps[MAXIMUM_SMOOTHED_PATH_STEPS];
			short smoothed_step_count = 0;
			short avoided_step_count = 0;
			boolean steps_finish_path = TRUE;
			boolean success = TRUE;
			short child_node_index = NONE;
			struct path_node const *child_node = NULL;
			short raw_step_count = MIN(node->depth + 1, MAXIMUM_RAW_PATH_STEPS);

			while (node_index != NONE)
			{
				node = path_get_node(state, node_index);

				if (node->depth >= MAXIMUM_RAW_PATH_STEPS)
				{
					steps_finish_path = FALSE;
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\ai\\path.c", 488, (node->depth >= 0) && (node->depth < raw_step_count));

					raw_steps[node->depth].surface_index = node->surface_index;

					if (child_node_index == NONE)
					{
						raw_steps[node->depth].point = path->endpoint.point;
					}
					else
					{
						match_assert("c:\\halo\\SOURCE\\ai\\path.c", 496, node->depth == child_node->depth - 1);

						raw_steps[node->depth].point = child_node->entry_point;
					}
				}

				child_node_index = node_index;
				child_node = node;
				node_index = node->parent_node_index;
			}

			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 507, child_node_index != NONE);
			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 508, child_node->depth == 0);

			if (game_connection() == _game_connection_local && ai_debug.path_disable_smoothing)
			{
				smoothed_step_count = MIN(raw_step_count, MAXIMUM_SMOOTHED_PATH_STEPS);
				memcpy(smoothed_steps, raw_steps, sizeof(*smoothed_steps) * smoothed_step_count);
			}
			else
			{
				path_smooth(state, raw_step_count, raw_steps, &smoothed_step_count, smoothed_steps, &steps_finish_path);
			}

			if (game_connection() == _game_connection_local && ai_debug.path_disable_obstacle_avoidance)
			{
				avoided_step_count = MIN(smoothed_step_count, MAXIMUM_SMOOTHED_PATH_STEPS);
				memcpy(avoided_steps, smoothed_steps, sizeof(*avoided_steps) * avoided_step_count);
			}
			else
			{
				success = path_avoid_obstacles(state, smoothed_step_count, smoothed_steps, &avoided_step_count, avoided_steps, &steps_finish_path);

				if (state->debug && !success)
				{
					state->debug->path_build_result = _path_build_result_obstacle_avoidance_failed;
				}
			}

			if (success)
			{
				path->valid = TRUE;
				path->steps_finish_path = steps_finish_path;
				path->step_index = 0;
				path->step_count = (char)avoided_step_count;
				memcpy(path->steps, avoided_steps, sizeof(*path->steps) * avoided_step_count);

				if (path->steps_finish_path)
				{
					path->endpoint.point = path->steps[path->step_count - 1].point;
					path->endpoint.surface_index = path->steps[path->step_count - 1].surface_index;
					path->endpoint.target_radius = distance3d(&path->endpoint.point, &state->destination.point);
				}

				if (state->debug)
				{
					state->debug->path_build_result = _path_build_result_success;
				}
			}

			if (state->debug)
			{
				state->debug->raw_step_count = raw_step_count;
				memcpy(state->debug->raw_steps, raw_steps, sizeof(*raw_steps) * raw_step_count);
				state->debug->smoothed_step_count = smoothed_step_count;
				memcpy(state->debug->smoothed_steps, smoothed_steps, sizeof(*smoothed_steps) * smoothed_step_count);
				state->debug->avoided_step_count = avoided_step_count;
				memcpy(state->debug->avoided_steps, avoided_steps, sizeof(*avoided_steps) * avoided_step_count);
			}
		}
		else if (state->debug)
		{
			state->debug->path_build_result = state->closest_node_index == NONE ? _path_build_result_cached_node_missing : _path_build_result_not_close_enough_to_destination;
		}
	}
	else if (state->debug)
	{
		state->debug->path_build_result = _path_build_result_no_destination;
	}

	if (state->debug)
	{
		state->debug->result = *path;

		if (state->debug->path_build_result != _path_build_result_success)
		{
			state->debug->failure = TRUE;
		}

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 613, state->debug->path_build_result != _path_build_result_none);
	}

	return path->valid;
}

void path_state_new(
	struct path_input const *input,
	struct path_state *state,
	struct path_debug_storage *debug)
{
	memset(state, 0, sizeof(*state));
	state->structure = global_structure_bsp_get();
	state->input = *input;
	state->debug = debug;

	return;
}

void path_state_destination(
	struct path_state *state,
	real_point3d const *destination_point,
	long destination_surface_index,
	real destination_accept_radius)
{
	state->destination_valid = TRUE;
	state->destination.point = *destination_point;
	state->destination.surface_index = destination_surface_index;
	state->destination.target_radius = destination_accept_radius;

	return;
}

static void path_state_reset(
	struct path_state *state)
{
	state->node_count = 0;
	state->heap_count = 1;
	memset(state->hash_table, NONE, sizeof(state->hash_table));
	state->closest_node_index = NONE;
	state->closest_distance = REAL_MAX;
	state->closest_cost_estimate = REAL_MAX;

	return;
}

static boolean path_state_begin(
	struct path_state *state)
{
	boolean success = FALSE;

	if (state->input.start_surface_index != NONE && state->input.start_point.z > -1000.f)
	{
		real distance;
		long quantized_cost_estimate;
		struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&state->structure->collision_bsp, 0, struct collision_bsp);

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 689, (state->input.start_surface_index >= 0) && (state->input.start_surface_index < bsp->surfaces.count));

		if (state->destination_valid)
		{
			distance = distance3d(&state->input.start_point, &state->destination.point);
			quantized_cost_estimate = (long)(distance / PATH_COST_ESTIMATE_GRANULARITY);

			if (quantized_cost_estimate >= SHORT_MAX)
			{
				error(
					_error_silent,
					"pathfinding: attempted to build path from (%.1f %.1f %.1f) to (%.1f %.1f %.1f) ... distance %.1f > maximum allowed %.1f",
					state->input.start_point.x,
					state->input.start_point.y,
					state->input.start_point.z,
					state->destination.point.x,
					state->destination.point.y,
					state->destination.point.z,
					distance,
					SHORT_MAX * PATH_COST_ESTIMATE_GRANULARITY);
			}
		}
		else
		{
			distance = 0.f;
			quantized_cost_estimate = 0;
		}

		if (quantized_cost_estimate < SHORT_MAX)
		{
			struct path_node *initial_node;
			short node_index;

			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 716, state->node_count == 0);

			node_index = state->node_count++;
			initial_node = &state->node_list[node_index];
			initial_node->parent_node_index = NONE;
			initial_node->parent_node_surface_index = NONE;
			initial_node->surface_index = state->input.start_surface_index;
			initial_node->entry_point = state->input.start_point;
			initial_node->linear_distance_to_entry_point = 0.f;
			initial_node->closest_approach_to_attractor = REAL_MAX;
			initial_node->path_distance_from_origin = 0.f;
			initial_node->cumulative_cost = 0.f;
			initial_node->total_cost_estimate = distance;
			initial_node->quantized_cost_estimate = (short)quantized_cost_estimate;
			initial_node->depth = 0;
			initial_node->debug_render_traverse_index = NONE;

			bsp = TAG_BLOCK_GET_ELEMENT(&state->structure->collision_bsp, 0, struct collision_bsp);
			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 739, (initial_node->surface_index >= 0) && (initial_node->surface_index < bsp->surfaces.count));

			if (state->destination_valid)
			{
				state->closest_node_index = node_index;
				state->closest_distance = distance;
				state->closest_point = state->input.start_point;
				state->closest_cost_estimate = distance;
			}

			state->hash_table[(initial_node->surface_index & PATH_HASH_KEY_MASK) * PATH_HASH_TABLE_SIZE_PER_KEY] = node_index;
			path_heap_insert(state, node_index, (short)quantized_cost_estimate);
			success = TRUE;
		}
	}

	return success;
}

boolean path_state_find(
	struct path_state *state)
{
	boolean success = FALSE;

	if (state->destination_valid)
	{
		ai_profile.meters[_ai_meter_path_find].current_count++;
	}
	else
	{
		ai_profile.meters[_ai_meter_path_flood].current_count++;
	}

	path_state_reset(state);

	if (state->debug)
	{
		state->debug->path_traverse_result = _path_traverse_result_none;
	}

	if (path_state_begin(state))
	{
		success = path_state_traverse(state);
	}
	else if (state->debug)
	{
		state->debug->path_traverse_result = _path_traverse_result_initial_not_pathfindable;
	}

	if (state->debug)
	{
		state->debug->path_state = *state;
		state->debug->structure_bsp_index = global_structure_bsp_index_get();

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 813, state->debug->path_traverse_result != _path_traverse_result_none);

		if (state->debug->path_traverse_result != _path_traverse_result_success)
		{
			state->debug->failure = TRUE;
		}
	}

	return success;
}

static boolean path_state_traverse(
	struct path_state *state)
{
	struct path_edge edges[MAXIMUM_PATH_EDGES_PER_COLLISION_SURFACE];
	real radius = MAX(0.2f, state->input.pathfinding_radius);
	boolean success = TRUE;
	boolean warned = FALSE;

	while (TRUE)
	{
		struct path_node *cheapest_node;
		struct collision_bsp const *bsp;
		short edge_count;
		short edge_index;
		short cheapest_node_index = path_heap_pop_cheapest_node(state);

		if (cheapest_node_index == NONE)
		{
			if (state->debug && state->debug->path_traverse_result == _path_traverse_result_none)
			{
				state->debug->path_traverse_result = _path_traverse_result_exhausted_search;
			}

			break;
		}

		cheapest_node = path_get_node(state, cheapest_node_index);
		bsp = TAG_BLOCK_GET_ELEMENT(&state->structure->collision_bsp, 0, struct collision_bsp);

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 861, (cheapest_node->surface_index >= 0) && (cheapest_node->surface_index < bsp->surfaces.count));

		if (state->destination_valid)
		{
			if (cheapest_node->surface_index == state->destination.surface_index)
			{
				state->closest_node_index = cheapest_node_index;
				state->closest_distance = 0.f;
				state->closest_point = state->destination.point;

				break;
			}

			if (cheapest_node->total_cost_estimate > 10.f * MAX(5.f, state->closest_distance) + state->closest_cost_estimate)
			{
				if (state->debug && state->debug->path_traverse_result == _path_traverse_result_none)
				{
					state->debug->path_traverse_result = _path_traverse_result_never_close_enough;
				}

				break;
			}
		}

		edge_count = build_path_edges_for_surface(state->structure, cheapest_node->surface_index, edges);

		for (edge_index = 0; edge_index < edge_count; edge_index++)
		{
			struct path_edge const *edge = &edges[edge_index];
			boolean valid = TRUE;

			if (edge->adjacent_surface_index == cheapest_node->parent_node_surface_index)
			{
				valid = FALSE;
			}

			if (!TEST_FLAG(edge->adjacent_pathfinding_surface, PATHFINDING_SURFACE_WALKABLE_BIT))
			{
				valid = FALSE;
			}

			if (!state->input.ignore_broken_surfaces &&
				TEST_FLAG(edge->adjacent_pathfinding_surface, PATHFINDING_SURFACE_BREAKABLE_BIT) &&
				surface_is_broken(state->structure, edge->adjacent_surface_index))
			{
				valid = FALSE;
			}

			if (valid)
			{
				real_point3d edge_target_point;
				real distance;
				real path_distance_from_origin;
				real closest_approach_to_attractor;
				real cost;
				real cumulative_cost;
				real total_cost_estimate;
				real destination_distance;
				long quantized_cost_estimate;
				short new_node_index = NONE;

				point_from_line3d(&edge->base_point, &edge->edge_vector, 0.5f, &edge_target_point);

				if (state->destination_valid)
				{
					real edge_length_squared = magnitude_squared3d(&edge->edge_vector);

					if (edge_length_squared > 16.f && edge_length_squared > (2.f * radius) * (2.f * radius))
					{
						real_vector3d edge_to_destination;
						real edge_length = square_root(edge_length_squared);
						real t = dot_product3d(vector_from_points3d(&edge->base_point, &state->destination.point, &edge_to_destination), &edge->edge_vector) / magnitude_squared3d(&edge->edge_vector);

						t = PIN(t, radius / edge_length, 1.f - radius / edge_length);
						point_from_line3d(&edge->base_point, &edge->edge_vector, t, &edge_target_point);
					}
				}

				distance = distance3d(&cheapest_node->entry_point, &edge_target_point);
				path_distance_from_origin = cheapest_node->path_distance_from_origin + distance;

				if (state->input.attractor_valid)
				{
					real attractor_weight = path_attractor_weight(state, &cheapest_node->entry_point, &edge_target_point, &closest_approach_to_attractor);

					cost = (attractor_weight + 1.f) * distance;
					closest_approach_to_attractor = MIN(cheapest_node->closest_approach_to_attractor, closest_approach_to_attractor);
				}
				else
				{
					closest_approach_to_attractor = 0.f;
					cost = distance;
				}

				cumulative_cost = cheapest_node->cumulative_cost + cost;
				total_cost_estimate = cumulative_cost;

				if (state->destination_valid)
				{
					destination_distance = distance3d(&edge_target_point, &state->destination.point);
					total_cost_estimate += destination_distance;
				}

				match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1005, total_cost_estimate - cheapest_node->total_cost_estimate >= -PATH_COST_ESTIMATE_GRANULARITY);

				quantized_cost_estimate = (long)(total_cost_estimate / PATH_COST_ESTIMATE_GRANULARITY);

				if (quantized_cost_estimate >= SHORT_MAX)
				{
					if (!warned)
					{
						error(
							_error_log,
							"path_state_traverse: cost %.1f exceeded maximum allowed %.1f, discarding node",
							total_cost_estimate,
							SHORT_MAX * PATH_COST_ESTIMATE_GRANULARITY);
						warned = TRUE;
					}
				}
				else
				{
					if (!state->input.search_bounded || !(path_distance_from_origin > state->input.search_maximum_distance))
					{
						short hash_index;

						for (hash_index = (short)((edge->adjacent_surface_index & PATH_HASH_KEY_MASK) * PATH_HASH_TABLE_SIZE_PER_KEY); ; hash_index = (hash_index + 1) & PATH_HASH_TABLE_MASK)
						{
							struct path_node *node;
							short node_index = state->hash_table[hash_index];

							if (node_index != NONE)
							{
								node = path_get_node(state, node_index);

								if (node->surface_index == edge->adjacent_surface_index)
								{
									if (quantized_cost_estimate < node->quantized_cost_estimate)
									{
										short heap_location = node->heap_location;

										if (heap_location == NONE)
										{
											error(_error_silent, "path_state_traverse: found a 'better' path to a closed node");
										}
										else
										{
											new_node_index = node_index;

											match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1059, (heap_location >= 1) && (heap_location < state->heap_count));
											match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1060, state->heap[heap_location].node_index == node_index);
											match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1061, state->heap[heap_location].quantized_cost_estimate == node->quantized_cost_estimate);
										}
									}

									break;
								}
							}
							else
							{
								if (state->node_count < PATH_NODE_LIST_SIZE)
								{
									new_node_index = state->node_count++;
									state->hash_table[hash_index] = new_node_index;
									state->node_list[new_node_index].heap_location = NONE;
								}
								else if (state->debug && state->debug->path_traverse_result == _path_traverse_result_none)
								{
									state->debug->path_traverse_result = _path_traverse_result_overflowed_nodes;
								}

								break;
							}
						}
					}
				}

				if (new_node_index != NONE)
				{
					struct path_node *new_node = path_get_node(state, new_node_index);
					struct path_node previous_node_values = *new_node;

					new_node->parent_node_index = cheapest_node_index;
					new_node->parent_node_surface_index = cheapest_node->surface_index;
					new_node->surface_index = edge->adjacent_surface_index;
					new_node->entry_point = edge_target_point;
					new_node->linear_distance_to_entry_point = distance;
					new_node->closest_approach_to_attractor = closest_approach_to_attractor;
					new_node->path_distance_from_origin = path_distance_from_origin;
					new_node->cumulative_cost = cumulative_cost;
					new_node->total_cost_estimate = total_cost_estimate;
					new_node->quantized_cost_estimate = (short)quantized_cost_estimate;
					new_node->depth = cheapest_node->depth + 1;
					new_node->debug_render_traverse_index = NONE;
					new_node->closest_distance = REAL_MAX;

					bsp = TAG_BLOCK_GET_ELEMENT(&state->structure->collision_bsp, 0, struct collision_bsp);
					match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1126, (new_node->surface_index >= 0) && (new_node->surface_index < bsp->surfaces.count));

					if (new_node->heap_location == NONE)
					{
						path_heap_insert(state, new_node_index, (short)quantized_cost_estimate);
					}
					else
					{
						match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1137, (new_node->heap_location >= 1) && (new_node->heap_location < state->heap_count));
						match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1138, quantized_cost_estimate <= state->heap[new_node->heap_location].quantized_cost_estimate);

						state->heap[new_node->heap_location].quantized_cost_estimate = (short)quantized_cost_estimate;
						path_heap_bubble_up(state, new_node->heap_location);
					}

					if (state->destination_valid)
					{
						real closest_distance = destination_distance;
						real_point3d closest_point = new_node->entry_point;

						if (destination_distance < 4.f)
						{
							closest_distance = closest_available_point_on_surface(state->structure, edge->adjacent_surface_index, &state->destination.point, &closest_point);
						}

						new_node->closest_distance = closest_distance;
						new_node->closest_point = closest_point;

						if (closest_distance < state->closest_distance)
						{
							state->closest_distance = closest_distance;
							state->closest_node_index = new_node_index;
							state->closest_point = closest_point;
							state->closest_cost_estimate = total_cost_estimate;
						}
					}
				}
			}
		}
	}

	if (state->destination_valid)
	{
		success = state->closest_distance <= state->destination.target_radius;
	}

	if (state->debug && success)
	{
		state->debug->path_traverse_result = _path_traverse_result_success;
	}

	return success;
}

static boolean path_heap_verify(
	struct path_state *state,
	short node_index,
	short heap_location)
{
	return TRUE;
}

static void path_heap_bubble_up(
	struct path_state *state,
	short heap_location)
{
	short node_index;
	short node_cost;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1258, (heap_location >= 1) && (heap_location <= PATH_NODE_LIST_SIZE));

	node_index = state->heap[heap_location].node_index;
	node_cost = state->heap[heap_location].quantized_cost_estimate;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1263, (node_index >= 0) && (node_index < PATH_NODE_LIST_SIZE));
	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1264, state->node_list[node_index].quantized_cost_estimate == node_cost);

	while (heap_location > 1)
	{
		short parent_node_index;
		short parent_cost_estimate;
		short parent_location = heap_location >> 1;

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1271, path_heap_verify(state, node_index, heap_location));

		parent_node_index = state->heap[parent_location].node_index;
		parent_cost_estimate = state->heap[parent_location].quantized_cost_estimate;

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1279, (parent_node_index >= 0) && (parent_node_index < PATH_NODE_LIST_SIZE));
		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1280, state->node_list[parent_node_index].heap_location == parent_location);
		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1281, state->node_list[parent_node_index].quantized_cost_estimate == parent_cost_estimate);

		if (node_cost >= parent_cost_estimate)
		{
			break;
		}

		state->heap[heap_location].node_index = parent_node_index;
		state->heap[heap_location].quantized_cost_estimate = parent_cost_estimate;
		state->node_list[parent_node_index].heap_location = heap_location;
		heap_location = parent_location;
	}

	state->heap[heap_location].node_index = node_index;
	state->heap[heap_location].quantized_cost_estimate = node_cost;
	state->node_list[node_index].heap_location = heap_location;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1301, path_heap_verify(state, NONE, NONE));

	return;
}

static void path_heap_bubble_down(
	struct path_state *state,
	short heap_location)
{
	short node_index;
	short node_cost;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1316, (heap_location >= 1) && (heap_location <= PATH_NODE_LIST_SIZE));

	node_index = state->heap[heap_location].node_index;
	node_cost = state->heap[heap_location].quantized_cost_estimate;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1321, (node_index >= 0) && (node_index < PATH_NODE_LIST_SIZE));
	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1322, state->node_list[node_index].quantized_cost_estimate == node_cost);

	while (TRUE)
	{
		short child_index;
		short best_heap_location = heap_location;
		short best_node_index = node_index;
		short best_cost = node_cost;

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1331, path_heap_verify(state, node_index, heap_location));

		for (child_index = 0; child_index < 2; child_index++)
		{
			short child_node_index;
			short child_cost;
			short child_heap_location = 2 * heap_location + child_index;

			if (child_heap_location >= state->heap_count)
			{
				break;
			}

			child_node_index = state->heap[child_heap_location].node_index;
			child_cost = state->heap[child_heap_location].quantized_cost_estimate;

			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1342, (child_node_index >= 0) && (child_node_index < PATH_NODE_LIST_SIZE));
			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1343, state->node_list[child_node_index].heap_location == child_heap_location);
			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1344, state->node_list[child_node_index].quantized_cost_estimate == child_cost);

			if (child_cost < best_cost)
			{
				best_heap_location = child_heap_location;
				best_node_index = child_node_index;
				best_cost = child_cost;
			}
		}

		if (best_heap_location == heap_location)
		{
			break;
		}

		state->heap[heap_location].node_index = best_node_index;
		state->heap[heap_location].quantized_cost_estimate = best_cost;
		state->node_list[best_node_index].heap_location = heap_location;
		heap_location = best_heap_location;
	}

	state->heap[heap_location].node_index = node_index;
	state->heap[heap_location].quantized_cost_estimate = node_cost;
	state->node_list[node_index].heap_location = heap_location;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1379, path_heap_verify(state, NONE, NONE));

	return;
}

static short path_heap_pop_cheapest_node(
	struct path_state *state)
{
	short node_index = NONE;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1392, path_heap_verify(state, NONE, NONE));
	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1394, state->heap_count >= 1);

	if (state->heap_count > 1)
	{
		node_index = state->heap[1].node_index;

		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1399, (node_index >= 0) && (node_index < PATH_NODE_LIST_SIZE));
		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1401, state->node_list[node_index].heap_location == 1);
		match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1402, state->node_list[node_index].quantized_cost_estimate == state->heap[1].quantized_cost_estimate);

		state->node_list[node_index].heap_location = NONE;
		state->heap_count--;

		if (state->heap_count > 1)
		{
			state->heap[1] = state->heap[state->heap_count];
			path_heap_bubble_down(state, 1);
		}
	}

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1415, path_heap_verify(state, NONE, NONE));

	return node_index;
}

static void path_heap_insert(
	struct path_state *state,
	short node_index,
	short quantized_cost_estimate)
{
	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1426, path_heap_verify(state, node_index, NONE));
	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1428, state->heap_count >= 1);

	if (state->heap_count < PATH_NODE_LIST_SIZE)
	{
		short heap_location = state->heap_count++;

		state->heap[heap_location].node_index = node_index;
		state->heap[heap_location].quantized_cost_estimate = quantized_cost_estimate;
		path_heap_bubble_up(state, heap_location);
	}
	else
	{
		error(_error_silent, "path_heap_insert: overflowed static size heap");
	}

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1444, path_heap_verify(state, NONE, NONE));

	return;
}

static real closest_available_point_on_surface(
	struct structure_bsp const *structure,
	long surface_index,
	real_point3d const *target_point,
	real_point3d *closest_point)
{
	real_point2d result_point;
	struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp);

	collision_surface_find_closest_point2d(bsp, surface_index, _z, TRUE, (real_point2d const *)target_point, &result_point);
	collision_surface_project_point2d(bsp, surface_index, _z, TRUE, &result_point, closest_point);

	return distance3d(target_point, closest_point);
}

static boolean surface_is_broken(
	struct structure_bsp const *structure,
	long surface_index)
{
	struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp);
	struct collision_surface const *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	boolean broken = FALSE;

	if (TEST_FLAG(surface->flags, _collision_surface_breakable_bit))
	{
		byte *breakable_surface_flags = breakable_surface_flags_get();

		broken = !BIT_VECTOR_TEST_FLAG((long *)breakable_surface_flags, surface->breakable_surface_index);
	}

	return broken;
}

static short build_path_edges_for_surface(
	struct structure_bsp const *structure,
	long surface_index,
	struct path_edge *edges)
{
	struct collision_surface const *surface;
	long collision_edge_index;
	byte const *pathfinding_surfaces = structure->pathfinding_surfaces.address;
	struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp);
	short edge_count = 0;

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1496, (surface_index >= 0) && (surface_index < bsp->surfaces.count));

	surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	collision_edge_index = surface->first_edge_index;

	do
	{
		struct collision_vertex const *vertex0;
		struct collision_vertex const *vertex1;
		struct collision_edge const *collision_edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, collision_edge_index, struct collision_edge);
		boolean forward = surface_index == collision_edge->surface_indices[1];
		struct path_edge *edge = &edges[edge_count++];

		edge->adjacent_surface_index = collision_edge->surface_indices[!forward];

		if (edge->adjacent_surface_index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1518, (edge->adjacent_surface_index >= 0) && (edge->adjacent_surface_index < bsp->surfaces.count));
		}

		edge->adjacent_pathfinding_surface = pathfinding_surfaces[edge->adjacent_surface_index];
		vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, collision_edge->vertex_indices[0], struct collision_vertex);
		vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, collision_edge->vertex_indices[1], struct collision_vertex);
		edge->base_point = vertex0->point;
		vector_from_points3d(&vertex0->point, &vertex1->point, &edge->edge_vector);

		if (edge_count == MAXIMUM_PATH_EDGES_PER_COLLISION_SURFACE)
		{
			break;
		}

		collision_edge_index = collision_edge->edge_indices[forward];
	}
	while (collision_edge_index != surface->first_edge_index);

	return edge_count;
}

struct path_node *path_get_node(
	struct path_state *state,
	short node_index)
{
	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1553, node_index != NONE);
	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1554, (node_index >= 0) && (node_index < state->node_count));

	return &state->node_list[node_index];
}

short path_node_from_hash_table(
	struct path_state *state,
	long surface_index)
{
	short node_index;
	short hash_index = (short)((surface_index & PATH_HASH_KEY_MASK) * PATH_HASH_TABLE_SIZE_PER_KEY);

	do
	{
		node_index = state->hash_table[hash_index];
		hash_index = (hash_index + 1) & PATH_HASH_TABLE_MASK;
	}
	while (node_index != NONE && state->node_list[node_index].surface_index != surface_index);

	return node_index;
}

void closest_point_to_attractor(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *q,
	real_point3d *result)
{
	real_vector3d segment;
	real_vector3d offset;
	real t;

	vector_from_points3d(p0, p1, &segment);
	vector_from_points3d(q, p0, &offset);
	t = dot_product3d(&segment, &offset) / magnitude_squared3d(&segment);

	if (t < 0.f || t > 1.f)
	{
		*result = *p1;
	}
	else
	{
		point_from_line3d(p0, &segment, t, result);
	}

	return;
}

real path_attractor_weight(
	struct path_state const *state,
	real_point3d const *p0,
	real_point3d const *p1,
	real *distance_reference)
{
	real_point3d closest_point;
	real distance_squared;
	real weight = 0.f;
	real distance = REAL_MAX;

	closest_point_to_attractor(p0, p1, &state->input.attractor_point, &closest_point);
	distance_squared = distance_squared3d(&state->input.attractor_point, &closest_point);

	if (distance_squared < state->input.attractor_radius * state->input.attractor_radius)
	{
		distance = square_root(distance_squared);
		weight = (1.f - distance / state->input.attractor_radius) * state->input.attractor_weight;
	}

	match_assert("c:\\halo\\SOURCE\\ai\\path.c", 1631, distance_reference);

	*distance_reference = distance;

	return weight;
}
