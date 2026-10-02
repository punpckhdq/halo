/*
DEVICE_MACHINES.C

*/

/* ---------- headers */

#include "cseries.h"
#include "devices.h"
#include "device_definitions.h"
#include "bipeds.h"
#include "physics_constants.h"
#include "object_lights.h"

/* ---------- constants */

enum
{
	MAXIMUM_OBJECTS_OPENING_MACHINE = 16, /* fake name */
	MACHINE_DOOR_OPEN_TICKS_DELAY = -3, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void machines_initialize(
	void)
{
	return;
}

void machines_dispose(
	void)
{
	return;
}

void machines_initialize_for_new_map(
	void)
{
	return;
}

void machines_dispose_from_old_map(
	void)
{
	return;
}

void machine_place(
	long machine_index,
	struct scenario_machine_datum *scenario_machine)
{
	struct machine_datum *machine = machine_get(machine_index);

	device_add_scenario_information(machine_index, &scenario_machine->device);

	if (TEST_FLAG(scenario_machine->flags, _scenario_machine_does_not_operate_automatically_bit))
	{
		SET_FLAG(machine->machine.flags, _machine_does_not_operate_automatically_bit, TRUE);
	}

	if (TEST_FLAG(scenario_machine->flags, _scenario_machine_one_sided_bit))
	{
		SET_FLAG(machine->machine.flags, _machine_one_sided_bit, TRUE);
	}

	if (TEST_FLAG(scenario_machine->flags, _scenario_machine_never_appears_locked_bit))
	{
		SET_FLAG(machine->machine.flags, _machine_never_appears_locked_bit, TRUE);
	}

	if (TEST_FLAG(scenario_machine->flags, _scenario_machine_opened_by_melee_attack_bit))
	{
		SET_FLAG(machine->machine.flags, _machine_opened_by_melee_attack_bit, TRUE);
	}

	return;
}

boolean machine_new(
	long machine_index)
{
	struct machine_datum *machine = machine_get(machine_index);
	struct machine_definition const *definition = machine_definition_get(machine->definition_index);

	SET_FLAG(machine->object.flags, _object_dynamic_lighting_recompute_bit, TRUE);
	SET_FLAG(machine->object.flags, _object_static_lighting_recompute_bit, TEST_FLAG(definition->machine.flags, _machine_is_elevator_bit));
	SET_FLAG(machine->object.flags, _object_static_lighting_raycast_sideways_bit, TEST_FLAG(definition->machine.flags, _machine_is_elevator_bit));

	return TRUE;
}

void machine_delete(
	long machine_index)
{
	return;
}

boolean machine_update(
	long machine_index)
{
	struct machine_datum *machine = machine_get(machine_index);
	struct machine_definition const *definition = machine_definition_get(machine->definition_index);

	if (definition->machine.type == _machine_gear)
	{
		real velocity = machine->device.power * definition->device.runtime_maximum_powered_position_velocity +
			(1.f - machine->device.power) * definition->device.runtime_maximum_depowered_position_velocity;

		machine->device.position += velocity;
		if (machine->device.position >= 1.f)
		{
			machine->device.position -= 1.f;
		}
		machine->device.position_velocity = 0.f;
		SET_FLAG(machine->device.flags, _device_animation_changed_bit, TRUE);

		if (machine->device.position_group_index != NONE)
		{
			struct device_group_datum *device_group = device_group_get(machine->device.position_group_index);

			device_group->desired_value = machine->device.position;
		}
	}

	if (!TEST_FLAG(machine->machine.flags, _machine_does_not_operate_automatically_bit) &&
		definition->machine.type == _machine_door &&
		!((machine_index + game_time_get()) & 3))
	{
		boolean open = FALSE;
		real radius = definition->device.automatic_activation_radius < _real_epsilon ? machine->object.bounding_sphere_radius : definition->device.automatic_activation_radius;
		long object_indices[MAXIMUM_OBJECTS_OPENING_MACHINE];
		short object_count = objects_in_sphere(FLAG(_object_class_collideable), _object_mask_biped, &machine->object.location, &machine->object.bounding_sphere_center, radius, object_indices, NUMBEROF(object_indices));
		short object_index;

		for (object_index = 0; object_index < object_count; object_index++)
		{
			struct unit_datum *unit = unit_get(object_indices[object_index]);
			struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
			boolean can_open = TRUE;

			if (TEST_FLAG(unit->object.damage_flags, _object_dead_bit) ||
				TEST_FLAG(unit_definition->unit.flags, _unit_cannot_open_doors_automatically_bit))
			{
				can_open = FALSE;
			}

			if (TEST_FLAG(machine->machine.flags, _machine_one_sided_bit) &&
				machine->device.position == 0.f &&
				!game_team_is_enemy(_game_team_player, unit->object.owner_team_index))
			{
				real_vector3d direction;

				vector_from_points3d(&machine->object.bounding_sphere_center, &unit->object.bounding_sphere_center, &direction);
				if (dot_product3d(&machine->object.forward, &direction) > 0.f)
				{
					can_open = FALSE;
				}
			}

			if (can_open)
			{
				open = TRUE;
			}
		}

		if (open)
		{
			if (machine->device.position_group_index != NONE)
			{
				device_group_set_desired_value(machine->device.position_group_index, 1.f);
			}
			machine->machine.door_open_ticks = MACHINE_DOOR_OPEN_TICKS_DELAY;
		}
	}

	if (definition->machine.type == _machine_door)
	{
		if (machine->device.position == 1.f)
		{
			if (++machine->machine.door_open_ticks > definition->machine.runtime_door_open_ticks &&
				machine->device.position_group_index != NONE)
			{
				device_group_set_desired_value(machine->device.position_group_index, 0.f);
			}
		}
		else
		{
			machine->machine.door_open_ticks = 0;
		}
	}

	if (TEST_FLAG(definition->machine.flags, _machine_is_elevator_bit) &&
		definition->machine.elevator_node_index != NONE)
	{
		real_matrix4x3 *node_matrix = object_get_node_matrix(machine_index, definition->machine.elevator_node_index);
		real_vector3d displacement;

		vector_from_points3d(&machine->machine.elevator_position, &node_matrix->position, &displacement);
		if (displacement.i != 0.f || displacement.j != 0.f || displacement.k != 0.f)
		{
			long object_indices[MAXIMUM_OBJECTS_PER_MAP];
			short object_count = objects_in_sphere(FLAG(_object_class_collideable), _object_mask_biped, &machine->object.location, &machine->object.bounding_sphere_center, machine->object.bounding_sphere_radius, object_indices, NUMBEROF(object_indices));
			short object_index;

			for (object_index = 0; object_index < object_count; object_index++)
			{
				long biped_index = object_indices[object_index];
				struct biped_datum *biped = biped_get(biped_index);

				if (biped->biped.elevator_object_index == machine_index)
				{
					real_point3d position;

					add_vectors3d((real_vector3d *)&biped->object.position, &displacement, (real_vector3d *)&position);
					object_translate(biped_index, &position, NULL);
				}
			}
		}

		machine->machine.elevator_position = node_matrix->position;
	}

	if (TEST_FLAG(machine->device.flags, _device_animation_changed_bit))
	{
		object_translate(machine_index, &machine->object.position, NULL);
		SET_FLAG(machine->device.flags, _device_animation_changed_bit, FALSE);
	}

	return TRUE;
}

void machine_bumped(
	long machine_index,
	long unit_index)
{
	struct machine_datum const *machine = machine_get(machine_index);
	struct machine_definition const *definition = machine_definition_get(machine->definition_index);

	return;
}

void machine_try_to_open_with_damage(
	long machine_index)
{
	struct machine_datum const *machine = machine_get(machine_index);
	struct machine_definition const *definition = machine_definition_get(machine->definition_index);

	if (TEST_FLAG(machine->machine.flags, _machine_opened_by_melee_attack_bit))
	{
		device_set_actual_position(machine_index, 1.f);
	}

	return;
}

/* ---------- private code */
