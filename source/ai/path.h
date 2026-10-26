/*
PATH.H

file has inline function assertions.
*/

#ifndef __PATH_H
#define __PATH_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	PATH_NODE_LIST_SIZE = 0x400,
	PATH_HASH_TABLE_SIZE = 0x1000,
};

enum
{
	_path_traverse_result_none = 0,
	_path_traverse_result_initial_not_pathfindable,
	_path_traverse_result_never_close_enough,
	_path_traverse_result_exhausted_search,
	_path_traverse_result_overflowed_nodes,
	_path_traverse_result_success,
	NUMBER_OF_PATH_TRAVERSE_RESULTS,
};

enum
{
	_path_build_result_none = 0,
	_path_build_result_no_destination,
	_path_build_result_cached_node_missing,
	_path_build_result_not_close_enough_to_destination,
	_path_build_result_obstacle_avoidance_failed,
	_path_build_result_success,
	NUMBER_OF_PATH_BUILD_RESULTS,
};

enum
{
	PATHFINDING_SURFACE_WALKABLE_BIT = 6,	/* fake name */
	PATHFINDING_SURFACE_BREAKABLE_BIT
};

enum
{
	_structure_test_pill2d_skip_endpoint_test_bit = 0,	/* fake name */
	NUMBER_OF_STRUCTURE_TEST_PILL2D_FLAGS				/* fake name */
};

enum
{
	_disc_optional_bit = 0,
	NUMBER_OF_DISC_FLAGS
};

/* ---------- macros */

#define MAXIMUM_SMOOTHED_PATH_STEPS 4
#define MAXIMUM_DISC_COUNT 128
#define MAXIMUM_OBSTACLE_AVOIDANCE_STEPS 128

#define MAXIMUM_RAW_PATH_STEPS 64

/* ---------- structures */

struct path_destination
{
	real_point3d point;
	long surface_index;
	real target_radius;
};

struct path_step
{
	long surface_index;
	real_point3d point;
};

struct path_result
{
	boolean valid;
	struct path_destination endpoint;
	boolean steps_finish_path;
	char step_count;
	char step_index;
	struct path_step steps[MAXIMUM_SMOOTHED_PATH_STEPS];
};

struct path_input
{
	real pathfinding_radius;
	boolean ignore_broken_surfaces;
	long ignore_source_object_index;
	long ignore_target_object_index;
	boolean start_valid;
	real_point3d start_point;
	long start_surface_index;
	boolean attractor_valid;
	real_point3d attractor_point;
	long attractor_object_index;
	real attractor_radius;
	real attractor_weight;
	boolean search_bounded;
	real search_maximum_distance;
};

struct path_node
{
	short child_node_index;
	short parent_node_index;
	long parent_node_surface_index;
	long surface_index;
	real_point3d entry_point;
	real linear_distance_to_entry_point;
	real closest_approach_to_attractor;
	real path_distance_from_origin;
	real cumulative_cost;
	real total_cost_estimate;
	short quantized_cost_estimate;
	short depth;
	short heap_location;
	short debug_render_traverse_index;
	real closest_distance;
	real_point3d closest_point;
};

struct path_heap_element
{
	short node_index;
	short quantized_cost_estimate;
};

struct path_collision_result
{
	boolean collision;
	real_point3d point;
	long surface_index;
	long edge_index;
	real t;
};

struct structure_test_ray2d_result
{
	real distance;
	long surface_index;
	long edge_index;
};

struct path_state
{
	struct path_input input;
	struct path_debug_storage *debug;
	boolean destination_valid;
	struct path_destination destination;
	struct structure_bsp const *structure;
	short closest_node_index;
	real closest_distance;
	real closest_cost_estimate;
	real_point3d closest_point;
	short node_count;
	struct path_node node_list[PATH_NODE_LIST_SIZE];
	short heap_count;
	struct path_heap_element heap[1025];
	short hash_table[PATH_HASH_TABLE_SIZE];
};

struct disc
{
	short flags;
	short obstacle_index;
	long object_index;
	real_point2d center;
	real radius;
	real z;
};

struct obstacles
{
	short obstacle_count;
	short disc_count;
	short disc_optional_count;
	struct disc discs[MAXIMUM_DISC_COUNT];
};

struct obstacles_test_pill_result
{
	real distance;
	short disc_index;
	short obstacle_index;
};

struct step
{
	real_point2d point;
	long surface_index;
	real_vector2d direction;
	real distance;
	short obstacle_index;
	byte obstacle_direction_index;
	short obstructed_goal_step_indices[2];
	real total_distance;
	short previous_step_index;
};

struct obstacle_path
{
	real radius;
	boolean ignore_broken_surfaces;
	struct obstacles const *obstacles;
	struct structure_bsp const *structure;
	real_point2d goal;
	long goal_surface_index;
	short goal_obstacle_index;
	short goal_step_index;
	short best_goal_blocked_step_index;
	real best_goal_blocked_distance;
	boolean goal_found_exactly;
	boolean finishing;
	boolean ignore_optional;
	short step_count;
	struct step steps[MAXIMUM_OBSTACLE_AVOIDANCE_STEPS];
	short heap_count;
	short heap[MAXIMUM_OBSTACLE_AVOIDANCE_STEPS];
};

struct path_debug_storage
{
	long actor_index;
	long path_time;
	long last_render_id;
	boolean valid;
	boolean failure;
	short structure_bsp_index;
	short path_traverse_result;
	short path_build_result;
	struct path_state path_state;
	struct path_result result;
	short raw_step_count;
	struct path_step raw_steps[MAXIMUM_RAW_PATH_STEPS];
	short smoothed_step_count;
	struct path_step smoothed_steps[MAXIMUM_SMOOTHED_PATH_STEPS];
	short avoided_step_count;
	struct path_step avoided_steps[MAXIMUM_SMOOTHED_PATH_STEPS];
	boolean debug_use_stored_obstacles;
	short stored_obstacle_step_count;
	struct obstacles path_obstacles[4];
	struct obstacle_path path_obstacle_paths[4];
};

/* ---------- prototypes/PATH.C */

void paths_initialize(void);
void paths_dispose(void);
void paths_initialize_for_new_map(void);
void paths_dispose_from_old_map(void);
void path_input_new(struct path_input *input, real pathfinding_radius, boolean ignore_broken_surfaces, long source_object_index);
void path_input_set_target_object(struct path_input *input, long target_object_index);
void path_input_set_start(struct path_input *input, real_point3d const *start_point, long start_surface_index);
void path_input_set_attractor(struct path_input *input, real_point3d const *attractor_point, real radius, long object_index, real weight);
void path_input_set_search_bounds(struct path_input *input, real maximum_distance);
boolean path_3d_available(struct structure_bsp *structure_bsp, real_point3d const *start_point, real avoidance_distance, real_point3d const *end_point, boolean *finishing_path_reference, real_point3d *path_endpoint);
boolean path_3d_build_path(struct structure_bsp *structure_bsp, real_point3d const *start_point, real avoidance_distance, real_point3d const *end_point, struct path_result *path);
boolean path_state_approach_point(struct path_state *state, real_point3d const *end_point, long end_surface_index, boolean *straight_line_reference, real_point3d *approach_point_reference);
boolean path_state_estimated_distance(struct path_state *state, real_point3d const *end_point, long end_surface_index, real *distance_reference, real *closest_approach_to_attractor_reference, real_vector3d *estimated_direction_reference);
boolean path_state_build_path(struct path_state *state, struct path_result *path);
void path_state_new(struct path_input const *input, struct path_state *state, struct path_debug_storage *debug);
void path_state_destination(struct path_state *state, real_point3d const *destination_point, long destination_surface_index, real destination_accept_radius);
boolean path_state_find(struct path_state *state);
struct path_node *path_get_node(struct path_state *state, short node_index);
short path_node_from_hash_table(struct path_state *state, long surface_index);
void closest_point_to_attractor(real_point3d const *p0, real_point3d const *p1, real_point3d const *q, real_point3d *result);
real path_attractor_weight(struct path_state const *state, real_point3d const *p0, real_point3d const *p1, real *distance_reference);

/* ---------- prototypes/PATH_SMOOTHING.C */

void path_smooth(struct path_state *state, short raw_step_count, struct path_step const *raw_steps, short *smoothed_step_count, struct path_step *smoothed_steps, boolean *steps_finish_path);

/* ---------- prototypes/PATH_STRUCTURE_BSP.C */

boolean structure_test_ray2d(struct structure_bsp const *structure, boolean ignore_broken_surfaces, real_point2d const *point, long surface_index, real_vector2d const *direction, real distance, struct structure_test_ray2d_result *result);
boolean structure_surfaces_are_equivalent(struct structure_bsp const *structure, real_point2d const *destination_point, long destination_surface_index, long test_surface_index);
boolean structure_test_line2d(struct structure_bsp const *structure, boolean ignore_broken_surfaces, real_point2d const *p0, long p0_surface_index, real_point2d const *p1, long p1_surface_index, struct path_collision_result *result);
long structure_surface_index_from_point(struct structure_bsp const *structure, boolean ignore_broken_surfaces, real_point2d const *known_point, long known_surface_index, real_point2d *point);
boolean structure_test_pill2d(struct structure_bsp const *structure, boolean ignore_broken_surfaces, real_point2d const *start_point, long start_surface_index, real_point2d const *end_point, long end_surface_index, real radius, unsigned long flags, struct path_collision_result *result);
boolean clip_empty_interval_by_solid_interval(real *empty_t0, real *empty_t1, real solid_t0, real solid_t1);

/* ---------- prototypes/PATH_OBSTACLES.C */

void obstacles_new(struct obstacles *obstacles);
boolean obstacles_add_disc(struct obstacles *obstacles, long object_index, short flags, real_point3d const *center, real radius);
void obstacles_get_discs_in_sphere(struct obstacles *obstacles, real_point3d const *center, real radius, real_vector3d const *movement_direction, long ignore_source_object_index, long ignore_target_object_index);
short obstacles_test_circle(struct obstacles const *obstacles, short ignore_disc_index, real_point2d const *center, real radius);
boolean obstacles_test_pill(struct obstacles const *obstacles, short ignore_disc_index, real_point2d const *point, real_vector2d const *direction, real radius, real distance, boolean ignore_optional, struct obstacles_test_pill_result *result);
void obstacles_disc_tangents(struct obstacles const *obstacles, short disc_index, real_point2d const *point, real radius, real_vector2d *right_direction, real_vector2d *left_direction, real *tangent_distance);
void obstacles_disc_neighborhood(struct obstacles const *obstacles, real radius, short seed_disc_index, byte *disc_flags);
void obstacles_recompute(struct obstacles *obstacles, real radius);
void render_debug_obstacles(struct obstacles const *obstacles, real radius);

/* ---------- prototypes/PATH_OBSTACLE_AVOIDANCE.C */

void render_debug_path(struct obstacle_path const *path);
boolean path_avoid_obstacles(struct path_state *state, short input_step_count, struct path_step const *input_steps, short *avoided_step_count, struct path_step *avoided_steps, boolean *steps_finish_path);
void render_debug_obstacle_path(void);

/* ---------- globals */

/* ---------- public code */

__inline struct disc const *obstacles_get_disc(
	struct obstacles const *obstacles,
	short disc_index)
{
	match_assert("c:\\halo\\source\\ai\\path.h", 396, disc_index>=0 && disc_index<obstacles->disc_count && obstacles->disc_count<=MAXIMUM_DISC_COUNT);

	return &obstacles->discs[disc_index];
}

__inline short obstacle_from_disc(
	struct obstacles const *obstacles,
	short disc_index)
{
	return disc_index != NONE ? obstacles_get_disc(obstacles, disc_index)->obstacle_index : NONE;
}

#endif // __PATH_H
