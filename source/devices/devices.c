/*
DEVICES.C

*/

/* ---------- headers */

#include "cseries.h"
#include "devices.h"
#include "device_definitions.h"
#include "network_game_globals.h"
#include "game_state.h"
#include "collisions.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "physics_constants.h"
#include "editor_stubs.h"
#include "effects.h"
#include "object_lights.h"
#include "render_debug.h"

/* ---------- constants */

enum
{
	MAXIMUM_DEVICE_GROUPS_PER_MAP = 1024, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static short device_group_new(real desired_value, unsigned long flags);
static void device_group_delete(short device_group_index);
static void create_initial_device_groups(void);

/* ---------- globals */

struct data_array *device_groups_data;

boolean debug_objects_devices;

/* ---------- public code */

void devices_initialize(
	void)
{
	device_groups_data = game_state_data_new("device groups", MAXIMUM_DEVICE_GROUPS_PER_MAP, sizeof(struct device_group_datum));
	match_assert("c:\\halo\\SOURCE\\devices\\devices.c", 72, device_groups_data);

	return;
}

void devices_dispose(
	void)
{
	return;
}

void devices_initialize_for_new_map(
	void)
{
	data_make_valid(device_groups_data);
	create_initial_device_groups();

	return;
}

void devices_dispose_from_old_map(
	void)
{
	data_make_invalid(device_groups_data);

	return;
}

boolean device_new(
	long device_index)
{
	struct device_datum *device = device_get(device_index);
	struct device_definition const *definition = device_definition_get(device->definition_index);

	device->device.position_group_index = NONE;
	device->device.power_group_index = NONE;
	SET_FLAG(device->object.flags, _object_shadowless_bit, TRUE);

	return TRUE;
}

void device_delete(
	long device_index)
{
	struct device_datum const *device = device_get(device_index);

	device_group_delete(device->device.power_group_index);
	device_group_delete(device->device.position_group_index);

	return;
}

boolean device_update(
	long device_index)
{
	struct device_datum *device = device_get(device_index);
	struct device_definition const *definition = device_definition_get(device->definition_index);
	boolean result = FALSE;

	if (device->device.power_group_index != NONE)
	{
		struct device_group_datum const *power_group = device_group_get(device->device.power_group_index);

		if (power_group->desired_value != device->device.power || device->device.power_velocity != 0.f)
		{
			real old_power = device->device.power;

			if (!accelerate_to_position(
				&device->device.power,
				&device->device.power_velocity,
				power_group->desired_value,
				definition->device.runtime_maximum_power_acceleration,
				definition->device.runtime_maximum_power_velocity,
				0.f,
				1.f,
				FALSE))
			{
				result = TRUE;
			}

			if (old_power != device->device.power)
			{
				SET_FLAG(device->device.flags, _device_animation_changed_bit, TRUE);
			}
		}
	}

	if (device->device.position_group_index != NONE)
	{
		struct device_group_datum const *position_group = device_group_get(device->device.position_group_index);

		if (position_group->desired_value == device->device.position && device->device.position_velocity == 0.f)
		{
			device->device.delay_ticks = 0;
		}
		else
		{
			real maximum_acceleration = device->device.power * definition->device.runtime_maximum_powered_position_acceleration +
				(1.f - device->device.power) * definition->device.runtime_maximum_depowered_position_acceleration;
			real maximum_velocity = device->device.power * definition->device.runtime_maximum_powered_position_velocity +
				(1.f - device->device.power) * definition->device.runtime_maximum_depowered_position_velocity;
			boolean positive = device->device.position_velocity > 0.f;

			if (device->device.delay_ticks >= definition->device.runtime_delay_ticks ||
				device->device.position != 0.f ||
				position_group->desired_value < device->device.position)
			{
				real old_velocity = device->device.position_velocity;
				real old_position = device->device.position;

				if (fabs(device->device.position_velocity) > maximum_velocity)
				{
					device->device.position_velocity = positive ? maximum_velocity : -maximum_velocity;
				}

				if (accelerate_to_position(
					&device->device.position,
					&device->device.position_velocity,
					position_group->desired_value,
					maximum_acceleration,
					maximum_velocity,
					0.f,
					1.f,
					TEST_FLAG(definition->device.flags, _device_position_loops_bit)))
				{
					device_effect_new(device_index, positive ? definition->device.positive_stop_effect.index : definition->device.negative_stop_effect.index);
				}
				else
				{
					if (device->device.position_velocity != 0.f && old_velocity * device->device.position_velocity <= 0.f)
					{
						device_effect_new(device_index, device->device.position_velocity > old_velocity ? definition->device.positive_start_effect.index : definition->device.negative_start_effect.index);
					}
					result = TRUE;
				}

				if (old_position != device->device.position)
				{
					SET_FLAG(device->device.flags, _device_animation_changed_bit, TRUE);
				}
			}
			else
			{
				if (++device->device.delay_ticks == 1)
				{
					device_effect_new(device_index, definition->device.delay_effect.index);
				}
			}
		}
	}

	return result;
}

void device_export_function_values(
	long device_index)
{
	struct device_datum *device = device_get(device_index);
	struct device_definition const *definition = device_definition_get(device->definition_index);
	short function_index;

	for (function_index = 0; function_index < NUMBER_OF_INCOMING_OBJECT_FUNCTIONS; function_index++)
	{
		short function_mode = definition->device.function_modes[function_index];

		if (function_mode != _device_function_none)
		{
			real value = 0.f;

			switch (function_mode)
			{
			case _device_function_power:
				value = device->device.power;
				break;
			case _device_function_change_in_power:
				if (device->device.power_velocity != 0.f)
				{
					value = fabs(device->device.power_velocity) / definition->device.runtime_maximum_power_velocity;
				}
				break;
			case _device_function_position:
				value = device->device.position;
				break;
			case _device_function_change_in_position:
				if (device->device.position_velocity != 0.f)
				{
					value = fabs(device->device.position_velocity) / definition->device.runtime_maximum_powered_position_velocity;
				}
				break;
			case _device_function_locked:
				if (device->device.power == 0.f)
				{
					value = 1.f;
				}

				if (device->object.type == _object_type_machine && device->device.position_group_index != NONE)
				{
					struct machine_datum const *machine = machine_get(device_index);
					struct device_group_datum const *device_group = device_group_get(machine->device.position_group_index);

					if (TEST_FLAG(machine->machine.flags, _machine_does_not_operate_automatically_bit) ||
						TEST_FLAG(machine->machine.flags, _machine_one_sided_bit))
					{
						value = 1.f;
					}

					if (TEST_FLAG(device_group->flags, _device_group_can_change_only_once_bit) &&
						TEST_FLAG(device_group->flags, _device_group_changed_once_bit))
					{
						value = 1.f;
					}

					if (machine->device.position == 1.f ||
						TEST_FLAG(machine->machine.flags, _machine_never_appears_locked_bit))
					{
						value = 0.f;
					}
				}
				break;
			case _device_function_delay:
				value = (definition->device.runtime_delay_ticks <= 0.f || device->device.delay_ticks == definition->device.runtime_delay_ticks) ?
					0.f : device->device.delay_ticks / definition->device.runtime_delay_ticks;
				break;
			}

			device->object.incoming_function_values[function_index] = value;
		}
	}

	return;
}

void device_preprocess_node_orientations(
	long device_index,
	struct real_orientation *node_orientations)
{
	struct device_datum const *device = device_get(device_index);
	struct device_definition const *definition = device_definition_get(device->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(definition->object.animation_graph.index);
	struct animation_graph_device_animations *device_animations = animation_graph->device_animations.count ?
		TAG_BLOCK_GET_ELEMENT(&animation_graph->device_animations, 0, struct animation_graph_device_animations) : NULL;

	if (device_animations)
	{
		short animation_index = device_animations->animations.count > _device_animation_position ?
			animation_graph_animation_index_get(&device_animations->animations)[_device_animation_position].animation_index : NONE;

		if (animation_index != NONE)
		{
			struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
			real position = TEST_FLAG(device->device.flags, _device_position_reversed_bit) ? 1.f - device->device.position : device->device.position;
			real frame_count;
			real frame_index;

			if (TEST_FLAG(definition->device.flags, _device_position_loops_bit))
			{
				frame_count = animation->frame_count;
			}
			else
			{
				frame_count = animation->frame_count - 1;
			}
			frame_index = frame_count * position;

			if (TEST_FLAG(definition->device.flags, _device_position_animation_not_interpolated_bit))
			{
				overlay_animation_apply(animation, (short)frame_index, node_orientations);
			}
			else
			{
				overlay_animation_apply_continuous(animation, frame_index, node_orientations);
			}
		}

		animation_index = device_animations->animations.count > _device_animation_power ?
			animation_graph_animation_index_get(&device_animations->animations)[_device_animation_power].animation_index : NONE;

		if (animation_index != NONE)
		{
			struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

			overlay_animation_apply_continuous(animation, animation->frame_count * device->device.power, node_orientations);
		}
	}

	return;
}

void device_render_debug(
	long device_index)
{
	struct device_datum const *device = device_get(device_index);

	if (debug_objects_devices)
	{
		char string[512];
		real_point3d position;

		strcpy(string, "");
		sprintf(string + strlen(string), "power %.2f/vel %.2f", device->device.power, device->device.power_velocity);

		if (device->device.power_group_index != NONE)
		{
			sprintf(string + strlen(string), " (group %d desired %.2f)", DATUM_INDEX_TO_ABSOLUTE_INDEX(device->device.power_group_index), device_group_get(device->device.power_group_index)->desired_value);
		}

		sprintf(string + strlen(string), "|nposition %.2f/vel %.2f", device->device.position, device->device.position_velocity);

		if (device->device.position_group_index != NONE)
		{
			sprintf(string + strlen(string), " (group %d desired %.2f)", DATUM_INDEX_TO_ABSOLUTE_INDEX(device->device.position_group_index), device_group_get(device->device.position_group_index)->desired_value);
		}

		object_get_origin(device_index, &position);
		point_from_line3d(&position, global_up3d, 0.4f, &position);
		render_debug_string_at_point(FALSE, &position, string, global_real_argb_white);
	}

	return;
}

boolean device_set_desired_position(
	long device_index,
	real desired_value)
{
	boolean success;

	if (device_index != NONE)
	{
		struct device_datum const *device = device_get(device_index);

		if (device->device.position_group_index != NONE)
		{
			success = device_group_set_desired_value(device->device.position_group_index, desired_value);
		}
		else
		{
			success = FALSE;
		}
	}
	else
	{
		success = FALSE;
	}

	return success;
}

real device_get_position(
	long device_index)
{
	real position;

	if (device_index != NONE)
	{
		struct device_datum const *device = device_get(device_index);

		position = device->device.position;
	}
	else
	{
		position = 0.f;
	}

	return position;
}

void device_set_power(
	long device_index,
	real power)
{
	if (device_index != NONE)
	{
		struct device_datum *device = device_get(device_index);

		SET_FLAG(device->device.flags, _device_animation_changed_bit, TRUE);
		device->device.power = power;
		device_group_set_desired_value(device->device.power_group_index, power);
	}

	return;
}

real device_get_power(
	long device_index)
{
	real power;

	if (device_index != NONE)
	{
		struct device_datum const *device = device_get(device_index);

		power = device->device.power;
	}
	else
	{
		power = 0.f;
	}

	return power;
}

boolean device_group_set_desired_value(
	short group_index,
	real desired_value)
{
	boolean success = FALSE;

	desired_value = PIN(desired_value, 0.f, 1.f);

	if (group_index != NONE)
	{
		struct device_group_datum *device_group = device_group_get(group_index);

		if (device_group->desired_value != desired_value &&
			(!TEST_FLAG(device_group->flags, _device_group_can_change_only_once_bit) || !TEST_FLAG(device_group->flags, _device_group_changed_once_bit)))
		{
			SET_FLAG(device_group->flags, _device_group_changed_once_bit, TRUE);
			device_group->desired_value = desired_value;
			success = TRUE;
		}
	}

	if (success)
	{
		struct object_iterator iterator;
		struct device_datum const *device;

		object_iterator_new(&iterator, _object_mask_device, 0);
		while (device = (struct device_datum const *)object_iterator_next(&iterator))
		{
			struct device_definition const *definition = device_definition_get(device->definition_index);

			if (device->device.power_group_index == group_index)
			{
				device_effect_new(iterator.index, desired_value > 0.f ? definition->device.repowered_effect.index : definition->device.depowered_effect.index);
			}
		}
	}

	return success;
}

void device_set_actual_position(
	long device_index,
	real desired_value)
{
	if (device_index != NONE)
	{
		struct device_datum const *device = device_get(device_index);

		if (device->device.position_group_index != NONE)
		{
			device_group_set_actual_value(device->device.position_group_index, desired_value);
		}
	}

	return;
}

void device_set_never_appears_locked(
	long device_index,
	boolean never_appears_locked)
{
	if (device_index != NONE)
	{
		struct machine_datum *machine = machine_try_and_get(device_index);

		if (machine)
		{
			SET_FLAG(machine->machine.flags, _machine_never_appears_locked_bit, never_appears_locked);
		}
	}

	return;
}

void device_group_set_actual_value(
	short group_index,
	real value)
{
	struct object_iterator iterator;
	struct device_datum *device;

	{
		struct device_group_datum *device_group;

		value = PIN(value, 0.f, 1.f);
		device_group = device_group_get(group_index);
		device_group->desired_value = value;
	}

	object_iterator_new(&iterator, _object_mask_device, 0);
	while (device = (struct device_datum *)object_iterator_next(&iterator))
	{
		if (device->device.power_group_index == group_index)
		{
			SET_FLAG(device->device.flags, _device_animation_changed_bit, TRUE);
			device->device.power = value;
			device->device.power_velocity = 0.f;
		}

		if (device->device.position_group_index == group_index)
		{
			SET_FLAG(device->device.flags, _device_animation_changed_bit, TRUE);
			device->device.position = value;
			device->device.position_velocity = 0.f;
		}
	}

	return;
}

void device_one_sided_set(
	long device_index,
	boolean one_sided)
{
	struct machine_datum *machine = machine_try_and_get(device_index);

	if (machine)
	{
		SET_FLAG(machine->machine.flags, _machine_one_sided_bit, one_sided);
	}

	return;
}

void device_operates_automatically_set(
	long device_index,
	boolean operates_automatically)
{
	struct machine_datum *machine = machine_try_and_get(device_index);

	if (machine)
	{
		SET_FLAG(machine->machine.flags, _machine_does_not_operate_automatically_bit, !operates_automatically);
	}

	return;
}

void device_group_change_only_once_more_set(
	long device_group_index,
	boolean change_only_once)
{
	if (device_group_index != NONE)
	{
		struct device_group_datum *device_group = device_group_get(device_group_index);

		SET_FLAG(device_group->flags, _device_group_can_change_only_once_bit, change_only_once);
		SET_FLAG(device_group->flags, _device_group_changed_once_bit, FALSE);
	}

	return;
}

real device_group_get_value(
	short group_index)
{
	struct device_group_datum const *device_group = device_group_get(group_index);

	return device_group->desired_value;
}

void device_add_scenario_information(
	long device_index,
	struct scenario_device_datum *scenario_device)
{
	struct device_datum *device = device_get(device_index);
	struct device_definition const *definition = device_definition_get(device->definition_index);
	struct device_group_datum const *power_group;
	struct device_group_datum const *position_group;

	device->device.power_group_index = scenario_device->power_group_index == NONE ?
		device_group_new(TEST_FLAG(scenario_device->flags, _scenario_device_initially_off_bit) ? 0.f : 1.f, FLAG(_device_group_runtime_bit)) :
		scenario_device->power_group_index;
	device->device.position_group_index = scenario_device->position_group_index == NONE ?
		device_group_new(TEST_FLAG(scenario_device->flags, _scenario_device_initially_open_bit) ? 1.f : 0.f,
			TEST_FLAG(scenario_device->flags, _scenario_device_changes_only_once_bit) ? FLAG(_device_group_runtime_bit)|FLAG(_device_group_can_change_only_once_bit) : FLAG(_device_group_runtime_bit)) :
		scenario_device->position_group_index;

	device->device.power = device_group_get(device->device.power_group_index)->desired_value;
	device->device.position = device_group_get(device->device.position_group_index)->desired_value;

	power_group = device_group_get(device->device.power_group_index);
	position_group = device_group_get(device->device.position_group_index);

	if (TEST_FLAG(scenario_device->flags, _scenario_device_position_reversed_bit))
	{
		SET_FLAG(device->device.flags, _device_position_reversed_bit, TRUE);
	}

	if (TEST_FLAG(scenario_device->flags, _scenario_device_not_usable_bit))
	{
		SET_FLAG(device->device.flags, _device_not_usable_bit, TRUE);
	}

	return;
}

void device_touched(
	long device_index,
	long unit_index)
{
	struct device_datum const *device = device_get(device_index);

	switch (device->object.type)
	{
	case _object_type_machine:
		machine_bumped(device_index, unit_index);
		break;
	case _object_type_control:
		control_touched(device_index, unit_index);
		break;
	}

	return;
}

boolean device_can_change_position(
	long device_index)
{
	struct device_datum const *device = device_get(device_index);
	boolean can_change_position = FALSE;

	if (device->device.position_group_index != NONE)
	{
		struct device_group_datum const *position_group = device_group_get(device->device.position_group_index);
		struct device_group_datum const *power_group = device_group_get(device->device.power_group_index);

		can_change_position = TRUE;

		if (TEST_FLAG(position_group->flags, _device_group_can_change_only_once_bit) &&
			TEST_FLAG(position_group->flags, _device_group_changed_once_bit))
		{
			can_change_position = FALSE;
		}

		if (TEST_FLAG(device->device.flags, _device_not_usable_bit))
		{
			can_change_position = FALSE;
		}

		if (power_group->desired_value != 1.f)
		{
			can_change_position = FALSE;
		}
	}

	return can_change_position;
}

boolean device_frontfacing(
	long device_index,
	real_point3d const *point,
	real_vector3d const *vector)
{
	struct control_datum const *control = control_try_and_get(device_index);
	boolean frontfacing = TRUE;

	if (control)
	{
		struct object_marker marker;

		if (!TEST_FLAG(control->control.flags, _control_usable_from_both_sides_bit) &&
			object_get_marker_by_name(device_index, "front", &marker, 1) == 1 &&
			dot_product3d(&marker.matrix.forward, vector) > 0.f)
		{
			frontfacing = FALSE;
		}
	}

	return frontfacing;
}

void device_effect_new(
	long device_index,
	long effect_index)
{
	if (effect_index != NONE)
	{
		struct device_datum const *device = device_get(device_index);
		long group_tag = tag_get_group_tag(effect_index);

		if (group_tag != EFFECT_DEFINITION_TAG)
		{
			match_vassert("c:\\halo\\SOURCE\\devices\\devices.c", 761, group_tag==SOUND_DEFINITION_TAG, NULL);

			if (group_tag == SOUND_DEFINITION_TAG)
			{
				object_impulse_sound_new(device_index, effect_index, NONE, global_origin3d, global_forward3d, 1.f);
			}
		}
		else
		{
			effect_new_from_object(effect_index, device_index, device_index, NONE, device->device.position, device->device.power, NULL, NULL);
		}
	}

	return;
}

/* ---------- private code */

static short device_group_new(
	real desired_value,
	unsigned long flags)
{
	short group_index = (short)datum_new(device_groups_data);

	if (group_index != NONE)
	{
		struct device_group_datum *device_group = device_group_get(group_index);

		device_group->desired_value = desired_value;
		device_group->flags = flags;
	}
	else
	{
		match_vassert("c:\\halo\\SOURCE\\devices\\devices.c", 785, FALSE, "no more free device groups");
	}

	return group_index;
}

static void device_group_delete(
	short device_group_index)
{
	if (device_group_index != NONE)
	{
		struct device_group_datum const *device_group = device_group_get(device_group_index);

		if (TEST_FLAG(device_group->flags, _device_group_runtime_bit))
		{
			datum_delete(device_groups_data, device_group_index);
		}
	}

	return;
}

static void create_initial_device_groups(
	void)
{
	struct scenario *scenario = global_scenario_get();
	short group_index;

	for (group_index = 0; group_index < scenario->device_groups.count; group_index++)
	{
		struct scenario_device_group const *device_group = TAG_BLOCK_GET_ELEMENT(&scenario->device_groups, group_index, struct scenario_device_group);
		unsigned long flags = 0;
		short new_group_index;

		if (TEST_FLAG(device_group->flags, _scenario_device_group_can_change_only_once_bit))
		{
			SET_FLAG(flags, _device_group_can_change_only_once_bit, TRUE);
		}

		new_group_index = device_group_new(device_group->initial_value, flags);
		match_assert("c:\\halo\\SOURCE\\devices\\devices.c", 825, new_group_index==group_index);
	}

	return;
}
