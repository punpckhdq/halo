/*
OBJECT_TYPES.C

symbols in this file:
0012B950 00b0:
	_object_type_definition_get (0000)
0012BA00 00a0:
	_object_type_get_datum_size (0000)
0012BAA0 0090:
	_object_type_get_name (0000)
0012BB30 00c0:
	_object_types_initialize (0000)
0012BBF0 0030:
	_object_types_dispose (0000)
0012BC20 0030:
	_object_types_initialize_for_new_map (0000)
0012BC50 0030:
	_object_types_dispose_from_old_map (0000)
0012BC80 0060:
	_object_type_adjust_placement (0000)
0012BCE0 0070:
	_object_type_new (0000)
0012BD50 0060:
	_object_type_place (0000)
0012BDB0 0060:
	_object_type_delete (0000)
0012BE10 0060:
	_object_type_update (0000)
0012BE70 0060:
	_object_type_export_function_values (0000)
0012BED0 0060:
	_object_type_handle_deleted_object (0000)
0012BF30 0060:
	_object_type_handle_region_destroyed (0000)
0012BF90 0060:
	_object_type_handle_parent_destroyed (0000)
0012BFF0 0060:
	_object_type_preprocess_node_orientations (0000)
0012C050 0060:
	_object_type_postprocess_node_matrices (0000)
0012C0B0 0060:
	_object_type_reset (0000)
0012C110 0060:
	_object_type_disconnect_from_structure_bsp (0000)
0012C170 0060:
	_object_type_render_debug (0000)
0012C1D0 0060:
	_object_type_notify_impulse_sound (0000)
0012C230 0050:
	_object_definition_index_to_object_type (0000)
0012C280 0080:
	_scenario_get_object_type_scenario_datums (0000)
0012C300 0080:
	_scenario_get_object_type_scenario_palette (0000)
0012C380 0050:
	_object_types_disconnect_from_structure_bsp (0000)
0012C3D0 0250:
	_object_types_place_objects (0000)
0012C620 00c0:
	_object_types_place_all (0000)
0012C6E0 00a0:
	_object_names_postprocess (0000)
0012C780 0020:
	_object_types_reconnect_to_structure_bsp (0000)
0012C7A0 0620:
	_object_type_synchronize (0000)
00289B14 000c:
	??_C@_0M@MDAGJHMB@placeholder?$AA@ (0000)
00289B20 000e:
	??_C@_0O@BFAJALOO@light_fixture?$AA@ (0000)
00289B30 000e:
	??_C@_0O@BJKPJAME@sound_scenery?$AA@ (0000)
00289B40 000b:
	??_C@_0L@BOMCPGNF@projectile?$AA@ (0000)
00289B4C 0008:
	??_C@_07BOKCJEKD@garbage?$AA@ (0000)
00289B54 0005:
	??_C@_04NHONDGDE@item?$AA@ (0000)
00289B5C 0006:
	??_C@_05LLJBOCMF@biped?$AA@ (0000)
00289B64 0030:
	??_C@_0DA@OHBPFGNG@object_type_definitions?$FLobject_t@ (0000)
00289B94 0025:
	??_C@_0CF@PNDIAODE@object_type_definitions?$FLobject_t@ (0000)
00289BBC 002a:
	??_C@_0CK@MHJDNGCD@?$CD?$CFd?5isn?8t?5a?5valid?5object?5type?5in@ (0000)
00289BE8 0026:
	??_C@_0CG@OKCLKJ@c?3?2halo?2SOURCE?2objects?2object_ty@ (0000)
00289C10 0012:
	??_C@_0BC@OEEHPIIN@?$CBdefinition?9?$DOnext?$AA@ (0000)
00289C28 0086:
	??_C@_0IG@INCFIBJJ@definition?9?$DOplacement_tag_block_@ (0000)
00289CB0 002d:
	??_C@_0CN@EBIFNKGJ@definition?9?$DOplacement_tag_block_@ (0000)
00289CE0 0082:
	??_C@_0IC@DHNDHHAP@definition?9?$DOpalette_tag_block_of@ (0000)
00289D64 002b:
	??_C@_0CL@LMPCIGEO@definition?9?$DOpalette_tag_block_of@ (0000)
00289D90 0008:
	??_C@_07OBIJJHNE@?$CGmatrix?$AA@ (0000)
0030B990 0a30:
	_object_data_definition (0000)
	_unit_data_definition (00a0)
	_biped_data_definition (0140)
	_vehicle_data_definition (01e0)
	_item_data_definition (0280)
	_weapon_data_definition (0320)
	_equipment_data_definition (03c0)
	_garbage_data_definition (0460)
	_projectile_data_definition (0500)
	_scenery_data_definition (05a0)
	_sound_scenery_data_definition (0640)
	_device_data_definition (06e0)
	_machine_data_definition (0780)
	_control_data_definition (0820)
	_light_fixture_data_definition (08c0)
	_placeholder_data_definition (0960)
	_object_type_definitions (0a00)
00456E98 0002:
	_bss_00456e98 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "object_types.h"

#include "objects.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean object_type_new(
	long object_index)
{
	short i;

	struct object_type_definition* definition= object_type_definition_get(object_get(object_index)->object.type);
	boolean result= TRUE;

	for (i= 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition= definition->part_definitions[i];

		if (current_definition->datum_new && !current_definition->datum_new(object_index))
		{
			result= FALSE;
			break;
		}
	}

	return result;
}

void object_type_delete(
	long object_index)
{
	short i;

	struct object_type_definition *definition= object_type_definition_get(object_get(object_index)->object.type);
	boolean result= TRUE;

	for (i= 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition= definition->part_definitions[i];
		if (current_definition->datum_delete)
		{
			current_definition->datum_delete(object_index);
		}
	}

	return;
}


/* ---------- private code */
