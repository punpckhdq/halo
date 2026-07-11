/*
PATH.H

file has inline function assertions.
*/

#ifndef __PATH_H
#define __PATH_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

enum
{
	PATH_NODE_LIST_SIZE = 0x400,
	PATH_HASH_TABLE_SIZE = 0x1000,
};

enum
{
	PATH_HASH_TABLE_SIZE_PER_KEY = 8,
	PATH_HASH_KEY_MASK = 511,
	PATH_HASH_TABLE_MASK = 4095,
	MAXIMUM_PATH_EDGES_PER_COLLISION_SURFACE = 64
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


/* ---------- macros */

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
	struct path_step steps[4];
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
};

struct path_heap_element
{
	short node_index;
	short quantized_cost_estimate;
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
	char __unknown[51676];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __PATH_H
