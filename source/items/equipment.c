/*
EQUIPMENT.C

symbols in this file:
000E5F40 0060:
	_equipment_place (0000)
000E5FA0 0040:
	_equipment_handle_pickup (0000)
000E5FE0 0030:
	_equipment_definition_handle_pickup (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "equipment.h"
#include "equipment_definitions.h"
#include "scenario_definitions.h"
#include "unit_definitions.h"
#include "network_game_globals.h"
#include "index_resolution.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "object_types.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void equipment_place(
	long equipment_index,
	struct scenario_equipment_datum *scenario_equipment)
{
	struct equipment_datum *equipment = equipment_get(equipment_index);

	SET_FLAG(equipment->object.flags, _object_at_rest_bit, TEST_FLAG(scenario_equipment->object.misc_flags, _equipment_created_at_rest_bit));
	SET_FLAG(equipment->object.flags, _object_cannot_be_garbage_bit, TRUE);
	SET_FLAG(equipment->object.flags, _object_shadowless_bit, TRUE);
	SET_FLAG(equipment->item.flags, _item_does_not_accelerate_bit, !TEST_FLAG(scenario_equipment->object.misc_flags, _equipment_does_accelerate_bit));

	if (!TEST_FLAG(scenario_equipment->object.misc_flags, _equipment_created_at_rest_bit))
	{
		equipment->object.position.z += 0.05f;
	}
}

void equipment_handle_pickup(
	long equipment_index)
{
	struct equipment_datum *equipment = equipment_get(equipment_index);
	struct equipment_definition *definition = equipment_definition_get(equipment->definition_index);

	if (definition->equipment.pickup_sound.index != NONE)
	{
		unspatialized_impulse_sound_new(definition->equipment.pickup_sound.index, 1.f);
	}
}

void equipment_definition_handle_pickup(
	long equipment_definition_index)
{
	struct equipment_definition *definition = equipment_definition_get(equipment_definition_index);

	if (definition->equipment.pickup_sound.index != NONE)
	{
		unspatialized_impulse_sound_new(definition->equipment.pickup_sound.index, 1.f);
	}
}

/* ---------- private code */
