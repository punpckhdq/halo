/*
OBJECT_TYPES.H
*/

#ifndef __OBJECT_TYPES_H
#define __OBJECT_TYPES_H
#pragma once

/* ---------- headers */

#include "objects.h"

/* ---------- constants */

enum
{
	MAXIMUM_CHILDREN_PER_OBJECT_TYPE_DEFINITION = 16,
};

/* ---------- structures */

struct object_type_definition
{
	char const *name;
	unsigned long group_tag;
	short game_datum_size;
	short placement_tag_block_offset;
	short palette_tag_block_offset;
	short placement_tag_block_element_size;
	void (*initialize)();
	void (*dispose)();
	void (*initialize_for_new_map)();
	void (*dispose_from_old_map)();
	void (*datum_adjust_placement)(long, struct object_placement_data *);
	boolean (*datum_new)(long);
	void (*datum_place)(long, void *);
	void (*datum_delete)(long);
	boolean(*datum_update)(long);
	void (*datum_export_function_values)(long);
	void (*handle_deleted_object)(long, long);
	void (*handle_region_destroyed)(long, short, unsigned long);
	boolean(*handle_parent_destroyed)(long);
	void (*datum_preprocess_node_orientations)(long, struct real_orientation *);
	void (*datum_postprocess_node_matrices)(long, struct real_matrix4x3 *);
	void (*reset)(long);
	void (*disconnect_from_structure_bsp)(long);
	void (*notify_impulse_sound)(long, long, long);
	void (*render_debug)(long);
	struct object_type_definition *part_definitions[MAXIMUM_CHILDREN_PER_OBJECT_TYPE_DEFINITION];
	struct object_type_definition *next;
};

/* ---------- prototypes/OBJECT_TYPES.C */

struct object_type_definition *object_type_definition_get(short object_type);
short object_type_get_datum_size(short object_type);
char const *object_type_get_name(short object_type);
void object_types_initialize(void);
void object_types_dispose(void);
void object_types_initialize_for_new_map(void);
void object_types_dispose_from_old_map(void);
void object_type_adjust_placement(long object_index, struct object_placement_data *data);
boolean object_type_new(long object_index);
void object_type_place(long object_index, struct scenario_object_datum *scenario_object);
void object_type_delete(long object_index);
boolean object_type_update(long object_index);
void object_type_export_function_values(long object_index);
void object_type_handle_deleted_object(long object_index, long deleted_object_index);
void object_type_preprocess_node_orientations(long object_index, struct real_orientation *node_orientations);
void object_type_postprocess_node_matrices(long object_index, struct real_matrix4x3 *node_matrices);
void object_type_reset(long object_index);
void object_type_disconnect_from_structure_bsp(long object_index);
struct tag_block *scenario_get_object_type_scenario_datums(struct scenario *scenario, short object_type, long *size);
struct tag_block *scenario_get_object_type_scenario_palette(struct scenario *scenario, short object_type);
void object_types_disconnect_from_structure_bsp(void);
void object_types_place_all(struct scenario *scenario);
void object_types_reconnect_to_structure_bsp(void);

#endif // __OBJECT_TYPES_H
