/*
OBJECT_TYPES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "items/equipment.h"
#include "object_types.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

short bss_00456e98 = 0;

extern struct object_type_definition* first_object_type_definition;

struct object_type_definition object_data_definition = {
	"object",
	'obje',
	420,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition},
	0
};

struct object_type_definition unit_data_definition = {
	"unit",
	'unit',
	1060,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &unit_data_definition},
	0
};

struct object_type_definition biped_data_definition = {
	"biped",
	'bipd',
	1152,
	552,
	564,
	120,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &unit_data_definition, &biped_data_definition},
	0
};

struct object_type_definition vehicle_data_definition = {
	"vehicle",
	'vehi',
	1148,
	576,
	588,
	120,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &unit_data_definition, &vehicle_data_definition},
	0
};

struct object_type_definition item_data_definition = {
	"item",
	'item',
	476,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &item_data_definition},
	0
};

struct object_type_definition weapon_data_definition = {
	"weapon",
	'weap',
	636,
	624,
	636,
	92,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &item_data_definition, &weapon_data_definition},
	0
};

struct object_type_definition equipment_data_definition = {
	"equipment",
	'eqip',
	500,
	600,
	612,
	40,
	0,
	0,
	0,
	0,
	0,
	0,
	equipment_place,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &item_data_definition, &equipment_data_definition},
	0
};

struct object_type_definition garbage_data_definition = {
	"garbage",
	'garb',
	500,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &item_data_definition, &garbage_data_definition},
	0
};

struct object_type_definition projectile_data_definition = {
	"projectile",
	'proj',
	552,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &projectile_data_definition},
	0
};

struct object_type_definition scenery_data_definition = {
	"scenery",
	'scen',
	424,
	528,
	540,
	72,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &scenery_data_definition},
	0
};

struct object_type_definition sound_scenery_data_definition = {
	"sound_scenery",
	'ssce',
	424,
	732,
	744,
	40,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &sound_scenery_data_definition},
	0
};

struct object_type_definition device_data_definition = {
	"device",
	'devi',
	452,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &device_data_definition},
	0
};

struct object_type_definition machine_data_definition = {
	"machine",
	'mach',
	472,
	660,
	672,
	64,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &device_data_definition, &machine_data_definition},
	0
};

struct object_type_definition control_data_definition = {
	"control",
	'ctrl',
	460,
	684,
	696,
	64,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &device_data_definition, &control_data_definition},
	0
};

struct object_type_definition light_fixture_data_definition = {
	"light_fixture",
	'lifi',
	476,
	708,
	720,
	88,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &device_data_definition, &light_fixture_data_definition},
	0
};

struct object_type_definition placeholder_data_definition = {
	"placeholder",
	'plac',
	508,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{&object_data_definition, &placeholder_data_definition},
	0
};


struct object_type_definition* object_type_definitions[NUMBER_OF_OBJECT_TYPES] = {
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
	&sound_scenery_data_definition,
};

/* ---------- public code */

struct object_type_definition* object_type_definition_get(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c", 
		631, 
		object_type>=0 && object_type<NUMBER_OF_OBJECT_TYPES, 
		csprintf(temporary, "#%d isn't a valid object type in [#0,#%d)", object_type, NUMBER_OF_OBJECT_TYPES));
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
		object_type>=0 && object_type<NUMBER_OF_OBJECT_TYPES,
		csprintf(temporary, "#%d isn't a valid object type in [#0,#%d)", object_type, NUMBER_OF_OBJECT_TYPES));
	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 643, object_type_definitions[object_type]);
	return object_type_definitions[object_type]->game_datum_size;
}

const char* object_type_get_name(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		652,
		object_type>=0 && object_type<NUMBER_OF_OBJECT_TYPES,
		csprintf(temporary, "#%d isn't a valid object type in [#0,#%d)", object_type, NUMBER_OF_OBJECT_TYPES));
	match_assert("c:\\halo\\SOURCE\\objects\\object_types.c", 653, object_type_definitions[object_type]);
	return object_type_definitions[object_type]->name;
}

void object_types_initialize() 
{

}

void object_types_dispose() 
{
	struct object_type_definition* definition = first_object_type_definition;
	while (definition != 0) {
		if (definition->dispose)
			definition->dispose();
		definition = definition->next;
	}
}

void object_types_initialize_for_new_map() 
{
	struct object_type_definition* definition = first_object_type_definition;
	bss_00456e98 = 0;
	while (definition != 0) {
		if (definition->initialize_for_new_map)
			definition->initialize_for_new_map();
		definition = definition->next;
	}
}

void object_types_dispose_from_old_map() 
{
	struct object_type_definition* definition = first_object_type_definition;
	while (definition != 0) {
		if (definition->dispose_from_old_map)
			definition->dispose_from_old_map();
		definition = definition->next;
	}
}

/* // uncomment this block when pr 12 gets merged

void object_type_adjust_placement(long object_index, void* unk) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->adjust_placement) {
			(*typelist)->adjust_placement(object_index, unk);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

byte object_type_new(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->new) {
			int ret = (*typelist)->new(object_index);
			if (ret == 0) return 0;
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
	return 1;
}

void object_type_place(long object_index, void* unk) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->place) {
			(*typelist)->place(object_index, unk);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_delete(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->delete) {
			(*typelist)->delete(object_index);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

byte object_type_update(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	byte ret = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->update) {
			if ((*typelist)->update(object_index) != 0) ret = 1;
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
	return ret;
}

void object_type_export_function_values(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->export_function_values) {
			(*typelist)->export_function_values(object_index);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_handle_deleted_object(long object_index, void* unk) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->handle_deleted_object) {
			(*typelist)->handle_deleted_object(object_index, unk);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_handle_region_destroyed(long object_index, void* unk, void* unk2) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->handle_region_destroyed) {
			(*typelist)->handle_region_destroyed(object_index, unk, unk2);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

byte object_type_handle_parent_destroyed(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	byte ret = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->handle_parent_destroyed) {
			if ((*typelist)->handle_parent_destroyed(object_index) != 0) ret = 1;
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
	return ret;
}


void object_type_preprocess_node_orientations(long object_index, void* unk) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->preprocess_node_orientations) {
			(*typelist)->preprocess_node_orientations(object_index, unk);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_postprocess_node_matrices(long object_index, void* unk) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->postprocess_node_matrices) {
			(*typelist)->postprocess_node_matrices(object_index, unk);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_reset(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->reset) {
			(*typelist)->reset(object_index);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_disconnect_from_structure_bsp(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->disconnect_from_structure_bsp) {
			(*typelist)->disconnect_from_structure_bsp(object_index);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_render_debug(long object_index) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->render_debug) {
			(*typelist)->render_debug(object_index);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

void object_type_notify_impulse_sound(long object_index, void* unk, void* unk2) {
	struct object* o = object_get_and_verify_type(object_index, _object_mask_all);
	struct object_type_definition* basetype = object_type_definition_get(o->type_id);
	struct object_type_definition** typelist = basetype->inheritance;
	short type_index = 0;
	while ((*typelist)->inheritance[type_index] != 0) {
		if ((*typelist)->notify_impulse_sound) {
			(*typelist)->notify_impulse_sound(object_index, unk, unk2);
		}
		type_index++;
		typelist = &basetype->inheritance[type_index];
	}
}

*/

/* ---------- private code */
