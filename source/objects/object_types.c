/*
OBJECT_TYPES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "objects/objects.h"
#include "object_types.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */
struct object_type_definition object_data_definition = {
	"object",
	'obje',
	420,
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	&object_export_function_values,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	&object_render_debug,
	{&object_data_definition},
	{0},
	0
};

struct object_type_definition unit_data_definition = {
	"unit",
	'unit',
	1060,
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
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
	{0},
	0
};

struct object_type_definition* object_type_definitions[NUMBER_OF_OBJECT_TYPES] = {0};

/* ---------- public code */

void* object_type_definition_get(
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
	return object_type_definitions[object_type]->datum_size;
}

char* object_type_get_name(
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

/* ---------- private code */
