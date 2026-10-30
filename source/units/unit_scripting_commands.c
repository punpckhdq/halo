/*
UNIT_SCRIPTING_COMMANDS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "units.h"
#include "network_game_globals.h"
#include "players.h"
#include "actor_definitions.h"
#include "console.h"
#include "network_messages.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "object_lists.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void unit_scripting_set_maximum_vitality(
	long unit_index,
	real body_vitality,
	real shield_vitality)
{
	if (unit_index != NONE && !TEST_FLAG(object_get(unit_index)->object.damage_flags, _object_dead_bit))
	{
		object_initialize_vitality(unit_index, &body_vitality, &shield_vitality);
	}

	return;
}

void units_scripting_set_maximum_vitality(
	long object_list_index,
	real body_vitality,
	real shield_vitality)
{
	long object_list_reference_index;
	long unit_index;

	for (unit_index = object_list_get_first(object_list_index, &object_list_reference_index);
		unit_index != NONE;
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index))
	{
		unit_scripting_set_maximum_vitality(unit_index, body_vitality, shield_vitality);
	}

	return;
}

void unit_scripting_set_current_vitality(
	long unit_index,
	real body_vitality,
	real shield_vitality)
{
	if (unit_index != NONE)
	{
		struct object_datum *object = object_get(unit_index);

		if (!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
		{
			real shield_fraction;
			real body_fraction;

			if (object->object.maximum_shield_vitality <= 0.f)
			{
				shield_fraction = 0.f;
			}
			else if (shield_vitality >= object->object.maximum_shield_vitality)
			{
				shield_fraction = 1.f;
			}
			else
			{
				shield_fraction = shield_vitality / object->object.maximum_shield_vitality;
			}

			if (object->object.maximum_body_vitality <= 0.f)
			{
				body_fraction = 0.f;
			}
			else if (body_vitality >= object->object.maximum_body_vitality)
			{
				body_fraction = 1.f;
			}
			else
			{
				body_fraction = body_vitality / object->object.maximum_body_vitality;
			}

			if (object->object.shield_vitality > 0.f && shield_fraction <= 0.f)
			{
				object_deplete_shield(unit_index);
			}
			object->object.shield_vitality = shield_fraction;

			if (object->object.body_vitality > 0.f && body_fraction <= 0.f)
			{
				object_deplete_body(unit_index);
			}
			object->object.body_vitality = body_fraction;
		}
	}

	return;
}

void units_scripting_set_current_vitality(
	long object_list_index,
	real body_vitality,
	real shield_vitality)
{
	long object_list_reference_index;
	long unit_index;

	for (unit_index = object_list_get_first(object_list_index, &object_list_reference_index);
		unit_index != NONE;
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index))
	{
		unit_scripting_set_current_vitality(unit_index, body_vitality, shield_vitality);
	}

	return;
}

real unit_scripting_get_health(
	long unit_index)
{
	struct object_datum *object = object_try_and_get(unit_index);
	real health = -1.f;

	if (object)
	{
		if (TEST_FLAG(object->object.damage_flags, _object_dead_bit))
		{
			health = 0.f;
		}
		else
		{
			health = object->object.body_vitality;
		}
	}

	return health;
}

real unit_scripting_get_shield(
	long unit_index)
{
	struct object_datum *object = object_try_and_get(unit_index);
	real shield = -1.f;

	if (object)
	{
		if (TEST_FLAG(object->object.damage_flags, _object_dead_bit))
		{
			shield = 0.f;
		}
		else
		{
			shield = object->object.shield_vitality;
		}
	}

	return shield;
}

short unit_scripting_get_grenade_count(
	long unit_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);
	short grenade_count = 0;

	if (unit)
	{
		short grenade_index;

		for (grenade_index = 0; grenade_index < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_index++)
		{
			grenade_count += unit->unit.grenade_counts[grenade_index];
		}
	}

	return grenade_count;
}

void unit_scripting_impervious(
	long object_list_index,
	boolean impervious)
{
	long object_list_reference_index;
	long unit_index;

	for (unit_index = object_list_get_first(object_list_index, &object_list_reference_index);
		unit_index != NONE;
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index))
	{
		struct unit_datum *unit = unit_try_and_get(unit_index);

		if (unit)
		{
			SET_FLAG(unit->unit.flags, _unit_impervious_bit, impervious);
		}
	}

	return;
}

boolean unit_scripting_start_user_animation_list(
	long object_list_index,
	long animation_graph_index,
	char const *animation_name,
	boolean interpolate)
{
	long object_list_reference_index;
	long unit_index;
	boolean success = TRUE;

	for (unit_index = object_list_get_first(object_list_index, &object_list_reference_index);
		unit_index != NONE;
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index))
	{
		if (unit_try_and_get(unit_index))
		{
			success = success && unit_start_user_animation(unit_index, animation_graph_index, animation_name, interpolate);
		}
	}

	return success;
}

boolean unit_scripting_has_weapon(
	long unit_index,
	long weapon_definition_index)
{
	boolean result = FALSE;

	if (unit_index != NONE && weapon_definition_index != NONE)
	{
		result = unit_has_weapon_definition_index(unit_index, weapon_definition_index);
	}

	return result;
}

boolean unit_scripting_has_weapon_readied(
	long unit_index,
	long weapon_definition_index)
{
	boolean result = FALSE;

	if (unit_index != NONE && weapon_definition_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		long weapon_index = unit_inventory_get_weapon(unit_index, unit->unit.current_weapon_index);

		if (weapon_index != NONE)
		{
			result = weapon_get(weapon_index)->definition_index == weapon_definition_index;
		}
	}

	return result;
}

/* ---------- private code */
