/*
OBJECT_TYPES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "object_types.h"
#include "object_lights.h"
#include "scenery.h"
#include "placeholder_definitions.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "network_game_manager.h"
#include "actor_definitions.h"
#include "network_messages.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "bipeds.h"
#include "structures.h"
#include "editor_stubs.h"
#include "game_engine_list.h"
#include "cinematics.h"
#include "devices.h"
#include "projectiles.h"
#include "equipment_definitions.h"
#include "projectile_definitions.h"
#include "device_definitions.h"
#include "equipment.h"
#include "garbage.h"
#include "sound_scenery.h"
#include "garbage_definitions.h"
#include "scenario.h"
#include "scenario_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct object_type_definition object_data_definition =
{
	"object",											// name
	OBJECT_DEFINITION_TAG,								// group_tag
	sizeof(struct object_datum),						// game_datum_size
	NONE,												// placement_tag_block_offset
	NONE,												// palette_tag_block_offset
	NONE,												// placement_tag_block_element_size
	NULL,												// initialize
	NULL,												// dispose
	NULL,												// initialize_for_new_map
	NULL,												// dispose_from_old_map
	NULL,												// datum_adjust_placement
	NULL,												// datum_new
	NULL,												// datum_place
	NULL,												// datum_delete
	NULL,												// datum_update
	object_export_function_values,						// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	object_render_debug,								// render_debug
	{&object_data_definition},							// part_definitions
	NULL												// next
};

struct object_type_definition unit_data_definition =
{
	"unit",												// name
	UNIT_DEFINITION_TAG,								// group_tag
	sizeof(struct unit_datum),							// game_datum_size
	NONE,												// placement_tag_block_offset
	NONE,												// palette_tag_block_offset
	NONE,												// placement_tag_block_element_size
	units_initialize,									// initialize
	units_dispose,										// dispose
	units_initialize_for_new_map,						// initialize_for_new_map
	units_dispose_from_old_map,							// dispose_from_old_map
	NULL,												// datum_adjust_placement
	unit_new,											// datum_new
	NULL,												// datum_place
	unit_delete,										// datum_delete
	unit_update,										// datum_update
	unit_export_function_values,						// datum_export_function_values
	unit_handle_deleted_object,							// handle_deleted_object
	unit_handle_region_destroyed,						// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	unit_preprocess_node_orientations,					// datum_preprocess_node_orientations
	unit_postprocess_node_matrices,						// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	unit_notify_impulse_sound,							// notify_impulse_sound
	unit_render_debug,									// render_debug
	{&object_data_definition, &unit_data_definition},	// part_definitions
	NULL												// next
};

struct object_type_definition biped_data_definition =
{
	"biped",											// name
	BIPED_DEFINITION_TAG,								// group_tag
	sizeof(struct biped_datum),							// game_datum_size
	offsetof(struct scenario, bipeds),					// placement_tag_block_offset
	offsetof(struct scenario, biped_palette),			// palette_tag_block_offset
	sizeof(struct scenario_biped_datum),				// placement_tag_block_element_size
	bipeds_initialize,									// initialize
	bipeds_dispose,										// dispose
	bipeds_initialize_for_new_map,						// initialize_for_new_map
	bipeds_dispose_from_old_map,						// dispose_from_old_map
	biped_adjust_placement,								// datum_adjust_placement
	biped_new,											// datum_new
	biped_place,										// datum_place
	biped_delete,										// datum_delete
	biped_update,										// datum_update
	biped_export_function_values,						// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	biped_preprocess_node_orientations,					// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	biped_reset,										// reset
	biped_disconnect_from_structure_bsp,				// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	biped_render_debug,									// render_debug
	{&object_data_definition, &unit_data_definition, &biped_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition vehicle_data_definition =
{
	"vehicle",											// name
	VEHICLE_DEFINITION_TAG,								// group_tag
	sizeof(struct vehicle_datum),						// game_datum_size
	offsetof(struct scenario, vehicles),				// placement_tag_block_offset
	offsetof(struct scenario, vehicle_palette),			// palette_tag_block_offset
	sizeof(struct scenario_vehicle_datum),				// placement_tag_block_element_size
	vehicles_initialize,								// initialize
	vehicles_dispose,									// dispose
	vehicles_initialize_for_new_map,					// initialize_for_new_map
	vehicles_dispose_from_old_map,						// dispose_from_old_map
	NULL,												// datum_adjust_placement
	vehicle_new,										// datum_new
	vehicle_place,										// datum_place
	vehicle_delete,										// datum_delete
	vehicle_update,										// datum_update
	vehicle_export_function_values,						// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	vehicle_preprocess_node_orientations,				// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	vehicle_reset,										// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	vehicle_render_debug,								// render_debug
	{&object_data_definition, &unit_data_definition, &vehicle_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition item_data_definition =
{
	"item",												// name
	ITEM_DEFINITION_TAG,								// group_tag
	sizeof(struct item_datum),							// game_datum_size
	NONE,												// placement_tag_block_offset
	NONE,												// palette_tag_block_offset
	NONE,												// placement_tag_block_element_size
	items_initialize,									// initialize
	items_dispose,										// dispose
	items_initialize_for_new_map,						// initialize_for_new_map
	items_dispose_from_old_map,							// dispose_from_old_map
	NULL,												// datum_adjust_placement
	item_new,											// datum_new
	NULL,												// datum_place
	item_delete,										// datum_delete
	item_update,										// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &item_data_definition},	// part_definitions
	NULL												// next
};

struct object_type_definition weapon_data_definition =
{
	"weapon",											// name
	WEAPON_DEFINITION_TAG,								// group_tag
	sizeof(struct weapon_datum),						// game_datum_size
	offsetof(struct scenario, weapons),					// placement_tag_block_offset
	offsetof(struct scenario, weapon_palette),			// palette_tag_block_offset
	sizeof(struct scenario_weapon_datum),				// placement_tag_block_element_size
	weapons_initialize,									// initialize
	weapons_dispose,									// dispose
	weapons_initialize_for_new_map,						// initialize_for_new_map
	weapons_dispose_from_old_map,						// dispose_from_old_map
	NULL,												// datum_adjust_placement
	weapon_new,											// datum_new
	weapon_place,										// datum_place
	weapon_delete,										// datum_delete
	weapon_update,										// datum_update
	weapon_export_function_values,						// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	weapon_preprocess_node_orientations,				// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &item_data_definition, &weapon_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition equipment_data_definition =
{
	"equipment",										// name
	EQUIPMENT_DEFINITION_TAG,							// group_tag
	sizeof(struct equipment_datum),						// game_datum_size
	offsetof(struct scenario, equipment),				// placement_tag_block_offset
	offsetof(struct scenario, equipment_palette),		// palette_tag_block_offset
	sizeof(struct scenario_equipment_datum),			// placement_tag_block_element_size
	NULL,												// initialize
	NULL,												// dispose
	NULL,												// initialize_for_new_map
	NULL,												// dispose_from_old_map
	NULL,												// datum_adjust_placement
	NULL,												// datum_new
	equipment_place,									// datum_place
	NULL,												// datum_delete
	NULL,												// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &item_data_definition, &equipment_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition garbage_data_definition =
{
	"garbage",											// name
	GARBAGE_DEFINITION_TAG,								// group_tag
	sizeof(struct garbage_datum),						// game_datum_size
	NONE,												// placement_tag_block_offset
	NONE,												// palette_tag_block_offset
	NONE,												// placement_tag_block_element_size
	NULL,												// initialize
	NULL,												// dispose
	NULL,												// initialize_for_new_map
	NULL,												// dispose_from_old_map
	NULL,												// datum_adjust_placement
	garbage_new,										// datum_new
	NULL,												// datum_place
	NULL,												// datum_delete
	garbage_update,										// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &item_data_definition, &garbage_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition projectile_data_definition =
{
	"projectile",										// name
	PROJECTILE_DEFINITION_TAG,							// group_tag
	sizeof(struct projectile_datum),					// game_datum_size
	NONE,												// placement_tag_block_offset
	NONE,												// palette_tag_block_offset
	NONE,												// placement_tag_block_element_size
	projectiles_initialize,								// initialize
	projectiles_dispose,								// dispose
	projectiles_initialize_for_new_map,					// initialize_for_new_map
	projectiles_dispose_from_old_map,					// dispose_from_old_map
	NULL,												// datum_adjust_placement
	projectile_new,										// datum_new
	NULL,												// datum_place
	projectile_delete,									// datum_delete
	projectile_update,									// datum_update
	projectile_export_function_values,					// datum_export_function_values
	projectile_handle_deleted_object,					// handle_deleted_object
	NULL,												// handle_region_destroyed
	projectile_handle_parent_destroyed,					// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &projectile_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition scenery_data_definition =
{
	"scenery",											// name
	SCENERY_DEFINITION_TAG,								// group_tag
	sizeof(struct scenery_datum),						// game_datum_size
	offsetof(struct scenario, scenery),					// placement_tag_block_offset
	offsetof(struct scenario, scenery_palette),			// palette_tag_block_offset
	sizeof(struct scenario_scenery_datum),				// placement_tag_block_element_size
	scenery_initialize,									// initialize
	scenery_dispose,									// dispose
	scenery_initialize_for_new_map,						// initialize_for_new_map
	scenery_dispose_from_old_map,						// dispose_from_old_map
	NULL,												// datum_adjust_placement
	scenery_new,										// datum_new
	scenery_place,										// datum_place
	scenery_delete,										// datum_delete
	scenery_update,										// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &scenery_data_definition},	// part_definitions
	NULL												// next
};

struct object_type_definition sound_scenery_data_definition =
{
	"sound_scenery",									// name
	SOUND_SCENERY_DEFINITION_TAG,						// group_tag
	sizeof(struct sound_scenery_datum),					// game_datum_size
	offsetof(struct scenario, sound_scenery),			// placement_tag_block_offset
	offsetof(struct scenario, sound_scenery_palette),	// palette_tag_block_offset
	sizeof(struct scenario_sound_scenery_datum),		// placement_tag_block_element_size
	NULL,												// initialize
	NULL,												// dispose
	NULL,												// initialize_for_new_map
	NULL,												// dispose_from_old_map
	NULL,												// datum_adjust_placement
	sound_scenery_new,									// datum_new
	NULL,												// datum_place
	sound_scenery_delete,								// datum_delete
	NULL,												// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &sound_scenery_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition device_data_definition =
{
	"device",											// name
	DEVICE_DEFINITION_TAG,								// group_tag
	sizeof(struct device_datum),						// game_datum_size
	NONE,												// placement_tag_block_offset
	NONE,												// palette_tag_block_offset
	NONE,												// placement_tag_block_element_size
	devices_initialize,									// initialize
	devices_dispose,									// dispose
	devices_initialize_for_new_map,						// initialize_for_new_map
	devices_dispose_from_old_map,						// dispose_from_old_map
	NULL,												// datum_adjust_placement
	device_new,											// datum_new
	NULL,												// datum_place
	device_delete,										// datum_delete
	device_update,										// datum_update
	device_export_function_values,						// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	device_preprocess_node_orientations,				// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	device_render_debug,								// render_debug
	{&object_data_definition, &device_data_definition},	// part_definitions
	NULL												// next
};

struct object_type_definition machine_data_definition =
{
	"machine",											// name
	MACHINE_DEFINITION_TAG,								// group_tag
	sizeof(struct machine_datum),						// game_datum_size
	offsetof(struct scenario, machines),				// placement_tag_block_offset
	offsetof(struct scenario, machine_palette),			// palette_tag_block_offset
	sizeof(struct scenario_machine_datum),				// placement_tag_block_element_size
	machines_initialize,								// initialize
	machines_dispose,									// dispose
	machines_initialize_for_new_map,					// initialize_for_new_map
	machines_dispose_from_old_map,						// dispose_from_old_map
	NULL,												// datum_adjust_placement
	machine_new,										// datum_new
	machine_place,										// datum_place
	machine_delete,										// datum_delete
	machine_update,										// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &device_data_definition, &machine_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition control_data_definition =
{
	"control",											// name
	CONTROL_DEFINITION_TAG,								// group_tag
	sizeof(struct control_datum),						// game_datum_size
	offsetof(struct scenario, controls),				// placement_tag_block_offset
	offsetof(struct scenario, control_palette),			// palette_tag_block_offset
	sizeof(struct scenario_control_datum),				// placement_tag_block_element_size
	controls_initialize,								// initialize
	controls_dispose,									// dispose
	controls_initialize_for_new_map,					// initialize_for_new_map
	controls_dispose_from_old_map,						// dispose_from_old_map
	NULL,												// datum_adjust_placement
	control_new,										// datum_new
	control_place,										// datum_place
	control_delete,										// datum_delete
	control_update,										// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &device_data_definition, &control_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition light_fixture_data_definition =
{
	"light_fixture",									// name
	LIGHT_FIXTURE_DEFINITION_TAG,						// group_tag
	sizeof(struct light_fixture_datum),					// game_datum_size
	offsetof(struct scenario, light_fixtures),			// placement_tag_block_offset
	offsetof(struct scenario, light_fixtures_palette),	// palette_tag_block_offset
	sizeof(struct scenario_light_fixture_datum),		// placement_tag_block_element_size
	light_fixtures_initialize,							// initialize
	light_fixtures_dispose,								// dispose
	light_fixtures_initialize_for_new_map,				// initialize_for_new_map
	light_fixtures_dispose_from_old_map,				// dispose_from_old_map
	NULL,												// datum_adjust_placement
	light_fixture_new,									// datum_new
	light_fixture_place,								// datum_place
	light_fixture_delete,								// datum_delete
	light_fixture_update,								// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &device_data_definition, &light_fixture_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition placeholder_data_definition =
{
	"placeholder",										// name
	PLACEHOLDER_DEFINITION_TAG,							// group_tag
	sizeof(struct placeholder_datum),					// game_datum_size
	NONE,												// placement_tag_block_offset
	NONE,												// palette_tag_block_offset
	NONE,												// placement_tag_block_element_size
	placeholder_initialize,								// initialize
	placeholder_dispose,								// dispose
	placeholder_initialize_for_new_map,					// initialize_for_new_map
	placeholder_dispose_from_old_map,					// dispose_from_old_map
	NULL,												// datum_adjust_placement
	placeholder_new,									// datum_new
	placeholder_place,									// datum_place
	placeholder_delete,									// datum_delete
	NULL,												// datum_update
	NULL,												// datum_export_function_values
	NULL,												// handle_deleted_object
	NULL,												// handle_region_destroyed
	NULL,												// handle_parent_destroyed
	NULL,												// datum_preprocess_node_orientations
	NULL,												// datum_postprocess_node_matrices
	NULL,												// reset
	NULL,												// disconnect_from_structure_bsp
	NULL,												// notify_impulse_sound
	NULL,												// render_debug
	{&object_data_definition, &placeholder_data_definition}, // part_definitions
	NULL												// next
};

struct object_type_definition *object_type_definitions[NUMBER_OF_OBJECT_TYPES] =
{
	&biped_data_definition,
	&vehicle_data_definition,
	&weapon_data_definition,
	&equipment_data_definition,
	&garbage_data_definition,
	&projectile_data_definition,
	&scenery_data_definition,
	&machine_data_definition,
	&control_data_definition,
	&light_fixture_data_definition,
	&placeholder_data_definition,
	&sound_scenery_data_definition
};

static word processed_bsp_flags;

/* ---------- public code */

struct object_type_definition *object_type_definition_get(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		631,
		object_type >= 0 && object_type < NUMBER_OF_OBJECT_TYPES,
		csprintf(
			temporary,
			"#%d isn't a valid object type in [#0,#%d)",
			object_type,
			NUMBER_OF_OBJECT_TYPES));
	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 632, object_type_definitions[object_type]);
	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 633, object_type_definitions[object_type]->group_tag);

	return object_type_definitions[object_type];
}

short object_type_get_datum_size(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		642,
		object_type >= 0 && object_type < NUMBER_OF_OBJECT_TYPES,
		csprintf(
			temporary,
			"#%d isn't a valid object type in [#0,#%d)",
			object_type,
			NUMBER_OF_OBJECT_TYPES));
	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 643, object_type_definitions[object_type]);

	return object_type_definitions[object_type]->game_datum_size;
}

char const *object_type_get_name(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		652,
		object_type >= 0 && object_type < NUMBER_OF_OBJECT_TYPES,
		csprintf(
			temporary,
			"#%d isn't a valid object type in [#0,#%d)",
			object_type,
			NUMBER_OF_OBJECT_TYPES));
	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 653, object_type_definitions[object_type]);

	return object_type_definitions[object_type]->name;
}

void object_types_place_all(
	struct scenario *scenario)
{
	if (!game_in_editor())
	{
		short object_type;

		for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
		{
			if (!TEST_FLAG(_object_mask_remove_on_bsp_switch, object_type))
			{
				struct object_type_definition *definition = object_type_definition_get(object_type);

				if (definition->placement_tag_block_offset != NONE && definition->palette_tag_block_offset != NONE)
				{
					long element_size;
					short datum_index;
					struct tag_block *datums = scenario_get_object_type_scenario_datums(
						scenario,
						object_type,
						&element_size);
					struct tag_block *palette = scenario_get_object_type_scenario_palette(scenario, object_type);

					for (datum_index = 0; datum_index < datums->count; datum_index++)
					{
						struct scenario_object_datum *scenario_object = tag_block_get_element_with_size(
							datums,
							datum_index,
							element_size);

						object_new_from_scenario(scenario_object, palette);
						objects_garbage_collection();
					}
				}
			}
		}

		object_types_place_objects(TRUE);
	}

	return;
}

void object_names_postprocess(
	struct scenario *scenario,
	boolean editing)
{
	if (!editing)
	{
		short object_type;

		for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
		{
			struct object_type_definition *definition = object_type_definition_get(object_type);

			if (definition->placement_tag_block_offset != NONE && definition->palette_tag_block_offset != NONE)
			{
				long element_size;
				short datum_index;
				struct tag_block *datums = scenario_get_object_type_scenario_datums(
					scenario,
					object_type,
					&element_size);

				for (datum_index = 0; datum_index < datums->count; datum_index++)
				{
					struct scenario_object_datum *scenario_object = tag_block_get_element_with_size(
						datums,
						datum_index,
						element_size);

					if (scenario_object->name_index != NONE)
					{
						struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(
							&scenario->object_names,
							scenario_object->name_index,
							struct scenario_object_name);

						object_name->runtime_object_type = object_type;
						object_name->runtime_scenario_datum_index = datum_index;
					}
				}
			}
		}
	}

	return;
}

void object_types_initialize(
	void)
{
	struct object_type_definition *definition;
	short object_type;
	struct object_type_definition **next_definition = &first_object_type_definition;

	for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
	{
		short part_index;

		definition = object_type_definition_get(object_type);
		match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 746, !definition->next);
		*next_definition = definition;
		next_definition = &definition->next;

		for (part_index = 0; part_index < MAXIMUM_CHILDREN_PER_OBJECT_TYPE_DEFINITION && definition->part_definitions[part_index]; part_index++)
		{
			struct object_type_definition *part_definition = definition->part_definitions[part_index];

			if (!part_definition->next)
			{
				*next_definition = part_definition;
				next_definition = &part_definition->next;
			}
		}
	}

	*next_definition = NULL;

	for (definition = first_object_type_definition; definition; definition = definition->next)
	{
		if (definition->initialize)
		{
			definition->initialize();
		}
	}

	return;
}

void object_types_dispose(
	void)
{
	struct object_type_definition *definition;

	for (definition = first_object_type_definition; definition; definition = definition->next)
	{
		if (definition->dispose)
		{
			definition->dispose();
		}
	}

	return;
}

void object_types_initialize_for_new_map(
	void)
{
	struct object_type_definition *definition;

	processed_bsp_flags = 0;

	for (definition = first_object_type_definition; definition; definition = definition->next)
	{
		if (definition->initialize_for_new_map)
		{
			definition->initialize_for_new_map();
		}
	}

	return;
}

void object_types_dispose_from_old_map(
	void)
{
	struct object_type_definition *definition;

	for (definition = first_object_type_definition; definition; definition = definition->next)
	{
		if (definition->dispose_from_old_map)
		{
			definition->dispose_from_old_map();
		}
	}

	return;
}

void object_type_adjust_placement(
	long object_index,
	struct object_placement_data *data)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_adjust_placement)
		{
			part_definition->datum_adjust_placement(object_index, data);
		}
	}

	return;
}

boolean object_type_new(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);
	boolean result = TRUE;

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_new && !part_definition->datum_new(object_index))
		{
			result = FALSE;
			break;
		}
	}

	return result;
}

void object_type_place(
	long object_index,
	struct scenario_object_datum *scenario_object)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_place)
		{
			part_definition->datum_place(object_index, scenario_object);
		}
	}

	return;
}

long object_type_synchronize(
	long object_index,
	struct scenario_object_datum *scenario_object,
	struct tag_block *palette,
	short object_type,
	short scenario_datum_index)
{
	struct object_placement_data placement_data;
	real_matrix4x3 matrix;
	struct object_datum *object;

	if (scenario_object->palette_entry_index == NONE)
	{
		if (object_index != NONE)
		{
			object_delete(object_index);
			object_index = NONE;
		}
	}
	else if (object_index == NONE)
	{
		struct scenario_object_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
			palette,
			scenario_object->palette_entry_index,
			struct scenario_object_palette_entry);

		if (palette_entry->reference.index != NONE)
		{
			object_placement_data_new(&placement_data, palette_entry->reference.index, NONE);
			placement_data.position = scenario_object->position;
			vectors3d_from_euler_angles3d(
				&placement_data.forward,
				&placement_data.up,
				&scenario_object->rotation);
			placement_data.variant_number = scenario_object->variant_number;
			object_index = object_new(&placement_data);

			if (object_index != NONE)
			{
				object_type_place(object_index, scenario_object);
			}
		}
	}
	else
	{
		struct scenario_object_palette_entry *palette_entry;

		object = object_try_and_get(object_index);
		palette_entry = TAG_BLOCK_GET_ELEMENT(
			palette,
			scenario_object->palette_entry_index,
			struct scenario_object_palette_entry);

		if (!object || object->definition_index != palette_entry->reference.index)
		{
			if (object)
			{
				object_delete(object_index);
			}

			object_index = NONE;

			if (palette_entry->reference.index != NONE)
			{
				object_placement_data_new(&placement_data, palette_entry->reference.index, NONE);
				placement_data.position = scenario_object->position;
				vectors3d_from_euler_angles3d(
					&placement_data.forward,
					&placement_data.up,
					&scenario_object->rotation);
				placement_data.variant_number = scenario_object->variant_number;
				object_index = object_new(&placement_data);

				if (object_index != NONE)
				{
					object_type_place(object_index, scenario_object);
				}
			}
		}
	}

	if (object_index != NONE)
	{
		struct object_definition *definition;

		object = object_get(object_index);
		object_activate(object_index);
		matrix4x3_rotation_from_angles(
			&matrix,
			scenario_object->rotation.yaw,
			scenario_object->rotation.pitch,
			scenario_object->rotation.roll);
		match_assert_valid_real_matrix4x3("c:\\halo\\SOURCE\\objects\\object_types.c", 975, &matrix);

		definition = object_definition_get(object->definition_index);

		if (definition->object.physics.index != NONE)
		{
			real_point3d position = scenario_object->position;

			position.z += object->object.bounding_sphere_radius * 0.5f;
			object_set_position(
				object_index,
				&position,
				&matrix.forward,
				&matrix.up);
		}
		else
		{
			object_set_position(
				object_index,
				&scenario_object->position,
				&matrix.forward,
				&matrix.up);
		}

		object->object.name_index = scenario_object->name_index;
	}

	if (scenario_object->name_index != NONE)
	{
		struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->object_names,
			scenario_object->name_index,
			struct scenario_object_name);

		object_name->runtime_object_type = object_type;
		object_name->runtime_scenario_datum_index = scenario_datum_index;
		object_set_object_index_for_name_index(scenario_object->name_index, object_index);
	}

	return object_index;
}

void object_type_delete(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_delete)
		{
			part_definition->datum_delete(object_index);
		}
	}

	return;
}

boolean object_type_update(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);
	boolean result = FALSE;

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_update && part_definition->datum_update(object_index))
		{
			result = TRUE;
		}
	}

	return result;
}

void object_type_export_function_values(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_export_function_values)
		{
			part_definition->datum_export_function_values(object_index);
		}
	}

	return;
}

void object_type_handle_deleted_object(
	long object_index,
	long deleted_object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->handle_deleted_object)
		{
			part_definition->handle_deleted_object(object_index, deleted_object_index);
		}
	}

	return;
}

void object_type_handle_region_destroyed(
	long object_index,
	short region_index,
	unsigned long damage_region_flags)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->handle_region_destroyed)
		{
			part_definition->handle_region_destroyed(object_index, region_index, damage_region_flags);
		}
	}

	return;
}

boolean object_type_handle_parent_destroyed(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);
	boolean result = FALSE;

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->handle_parent_destroyed && part_definition->handle_parent_destroyed(object_index))
		{
			result = TRUE;
		}
	}

	return result;
}

void object_type_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_preprocess_node_orientations)
		{
			part_definition->datum_preprocess_node_orientations(object_index, node_orientations);
		}
	}

	return;
}

void object_type_postprocess_node_matrices(
	long object_index,
	struct real_matrix4x3 *node_matrices)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->datum_postprocess_node_matrices)
		{
			part_definition->datum_postprocess_node_matrices(object_index, node_matrices);
		}
	}

	return;
}

void object_type_reset(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->reset)
		{
			part_definition->reset(object_index);
		}
	}

	return;
}

void object_type_disconnect_from_structure_bsp(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->disconnect_from_structure_bsp)
		{
			part_definition->disconnect_from_structure_bsp(object_index);
		}
	}

	return;
}

void object_type_render_debug(
	long object_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->render_debug)
		{
			part_definition->render_debug(object_index);
		}
	}

	return;
}

void object_type_notify_impulse_sound(
	long object_index,
	long sound_definition_index,
	long impulse_sound_index)
{
	short part_index;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (part_index = 0; definition->part_definitions[part_index]; part_index++)
	{
		struct object_type_definition *part_definition = definition->part_definitions[part_index];

		if (part_definition->notify_impulse_sound)
		{
			part_definition->notify_impulse_sound(object_index, sound_definition_index, impulse_sound_index);
		}
	}

	return;
}

short object_definition_index_to_object_type(
	long definition_index)
{
	short object_type;
	unsigned long group_tag = tag_get_group_tag(definition_index);
	short result = NONE;

	for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
	{
		if (object_type_definition_get(object_type)->group_tag == group_tag)
		{
			result = object_type;
			break;
		}
	}

	return result;
}

struct tag_block *scenario_get_object_type_scenario_datums(
	struct scenario *scenario,
	short object_type,
	long *size)
{
	struct object_type_definition *definition = object_type_definition_get(object_type);

	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 1279, definition->placement_tag_block_offset!=NONE);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		1280,
		definition->placement_tag_block_offset>=0 && definition->placement_tag_block_offset<=sizeof(struct scenario)+sizeof(struct tag_block));

	if (size)
	{
		*size = definition->placement_tag_block_element_size;
	}

	return (struct tag_block *)((byte *)scenario + definition->placement_tag_block_offset);
}

struct tag_block *scenario_get_object_type_scenario_palette(
	struct scenario *scenario,
	short object_type)
{
	struct object_type_definition *definition = object_type_definition_get(object_type);

	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 1293, definition->palette_tag_block_offset!=NONE);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		1294,
		definition->palette_tag_block_offset>=0 && definition->palette_tag_block_offset<=sizeof(struct scenario)+sizeof(struct tag_block));

	return (struct tag_block *)((byte *)scenario + definition->palette_tag_block_offset);
}

void object_types_disconnect_from_structure_bsp(
	void)
{
	struct object_iterator iterator;
	struct object_datum *object;

	object_iterator_new(&iterator, _object_mask_remove_on_bsp_switch, 0);

	while ((object = object_iterator_next(&iterator)) != NULL)
	{
		if (object->object.name_index == NONE)
		{
			object_delete(iterator.index);
		}
	}

	return;
}

void object_types_reconnect_to_structure_bsp(
	void)
{
	if (!cinematic_in_progress() || !cinematic_globals->cinematic_suppress_bsp_object_creation)
	{
		object_types_place_objects(TRUE);
	}

	return;
}

void object_types_place_objects(
	boolean place)
{
	if (!game_in_editor() && global_structure_bsp_index != NONE)
	{
		short object_type;
		struct scenario *scenario = global_scenario_get();

		for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
		{
			if (TEST_FLAG(_object_mask_remove_on_bsp_switch, object_type))
			{
				struct object_type_definition *definition = object_type_definition_get(object_type);

				if (definition->placement_tag_block_offset != NONE && definition->palette_tag_block_offset != NONE)
				{
					long element_size;
					short datum_index;
					struct tag_block *datums = scenario_get_object_type_scenario_datums(
						scenario,
						object_type,
						&element_size);
					struct tag_block *palette = scenario_get_object_type_scenario_palette(scenario, object_type);

					if (!TEST_FLAG(processed_bsp_flags, global_structure_bsp_index))
					{
						for (datum_index = 0; datum_index < datums->count; datum_index++)
						{
							struct scenario_object_datum *scenario_object = tag_block_get_element_with_size(
								datums,
								datum_index,
								element_size);

							if (scenario_object->palette_entry_index != NONE)
							{
								real_matrix4x3 matrix;
								real_point3d bounding_sphere_center;
								struct scenario_object_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
									palette,
									scenario_object->palette_entry_index,
									struct scenario_object_palette_entry);
								struct object_definition *object_definition = object_definition_get(palette_entry->reference.index);

								matrix4x3_rotation_from_angles(
									&matrix,
									scenario_object->rotation.yaw,
									scenario_object->rotation.pitch,
									scenario_object->rotation.roll);
								matrix.position = scenario_object->position;
								matrix4x3_transform_point(
									&matrix,
									&object_definition->object.bounding_offset,
									&bounding_sphere_center);

								if (scenario_leaf_index_from_point(&scenario_object->position) == NONE &&
									scenario_leaf_index_from_point(&bounding_sphere_center) == NONE)
								{
									scenario_object->on_bsp_flags &= ~FLAG(global_structure_bsp_index);
								}
								else
								{
									scenario_object->on_bsp_flags |= FLAG(global_structure_bsp_index);
								}
							}
						}
					}

					if (place)
					{
						objects_memory_compact();

						for (datum_index = 0; datum_index < datums->count; datum_index++)
						{
							struct scenario_object_datum *scenario_object = tag_block_get_element_with_size(
								datums,
								datum_index,
								element_size);

							if ((scenario_object->name_index == NONE || object_index_from_name_index(scenario_object->name_index) == NONE) &&
								!TEST_FLAG(scenario_object->placement_flags, _scenario_object_placement_not_automatic_bit) &&
								TEST_FLAG(scenario_object->on_bsp_flags, global_structure_bsp_index))
							{
								object_new_from_scenario(scenario_object, palette);
								objects_garbage_collection();
							}
						}
					}
				}
			}
		}

		processed_bsp_flags |= FLAG(global_structure_bsp_index);
	}

	return;
}

/* ---------- private code */
