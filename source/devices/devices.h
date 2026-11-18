/*
DEVICES.H

header included in hcex build.
*/

#ifndef __DEVICES_H
#define __DEVICES_H
#pragma once

/* ---------- headers */

#include "objects.h"

/* ---------- constants */

enum
{
	_scenario_device_group_can_change_only_once_bit = 0,
	NUMBER_OF_SCENARIO_DEVICE_GROUP_FLAGS,
};

enum
{
	_scenario_device_initially_open_bit = 0,
	_scenario_device_initially_off_bit,
	_scenario_device_changes_only_once_bit,
	_scenario_device_position_reversed_bit,
	_scenario_device_not_usable_bit,
	SCENARIO_DEVICE_DATUM_FLAGS,
};

enum
{
	_scenario_machine_does_not_operate_automatically_bit = 0,
	_scenario_machine_one_sided_bit,
	_scenario_machine_never_appears_locked_bit,
	_scenario_machine_opened_by_melee_attack_bit,
	NUMBER_OF_SCENARIO_MACHINE_FLAGS,
};

enum
{
	_scenario_control_usable_from_both_sides_bit = 0,
	NUMBER_OF_SCENARIO_CONTROL_DATUM_FLAGS,
};

enum
{
	_device_group_can_change_only_once_bit = 0,
	_device_group_changed_once_bit,
	_device_group_runtime_bit,
	NUMBER_OF_DEVICE_GROUP_FLAGS,
};

enum
{
	_device_position_reversed_bit = 0,
	_device_not_usable_bit,
	_device_animation_changed_bit,
	NUMBER_OF_DEVICE_FLAGS,
};

enum
{
	_machine_does_not_operate_automatically_bit = 0,
	_machine_one_sided_bit,
	_machine_never_appears_locked_bit,
	_machine_opened_by_melee_attack_bit,
	NUMBER_OF_MACHINE_DATUM_FLAGS,
};

enum
{
	_control_usable_from_both_sides_bit = 0,
	NUMBER_OF_CONTROL_DATUM_FLAGS,
};

/* ---------- macros */

#define device_group_get(index)				DATUM_GET(device_groups_data, index, struct device_group_datum) // [fake name?]
#define device_group_try_and_get(index)		((struct device_group_datum *)datum_try_and_get(device_groups_data, (index))) // [fake name?]

#define device_get(index)					((struct device_datum *)object_get_and_verify_type((index), _object_mask_device)) // [fake name?]
#define device_try_and_get(index)			((struct device_datum *)object_try_and_get_and_verify_type((index), _object_mask_device)) // [fake name?]

#define machine_get(index)					((struct machine_datum *)object_get_and_verify_type((index), _object_mask_machine)) // [fake name?]
#define machine_try_and_get(index)			((struct machine_datum *)object_try_and_get_and_verify_type((index), _object_mask_machine)) // [fake name?]

#define control_get(index)					((struct control_datum *)object_get_and_verify_type((index), _object_mask_control)) // [fake name?]
#define control_try_and_get(index)			((struct control_datum *)object_try_and_get_and_verify_type((index), _object_mask_control)) // [fake name?]

#define light_fixture_get(index)			((struct light_fixture_datum *)object_get_and_verify_type((index), _object_mask_light_fixture)) // [fake name?]
#define light_fixture_try_and_get(index)	((struct light_fixture_datum *)object_try_and_get_and_verify_type((index), _object_mask_light_fixture)) // [fake name?]

/* ---------- structures */

struct device_group_datum
{
	short identifier;
	word flags;
	real desired_value;
};

struct _device_datum
{
	unsigned long flags;
	short power_group_index;
	real power;
	real power_velocity;
	short position_group_index;
	real position;
	real position_velocity;
	short delay_ticks;
};

struct device_datum
{
	long definition_index;
	struct _object_datum object;
	struct _device_datum device;
};

struct _machine_datum
{
	unsigned long flags;
	long door_open_ticks;
	real_point3d elevator_position;
};

struct machine_datum
{
	long definition_index;
	struct _object_datum object;
	struct _device_datum device;
	struct _machine_datum machine;
};

struct _control_datum
{
	unsigned long flags;
	short hud_override_index;
	short pad;
};

struct control_datum
{
	long definition_index;
	struct _object_datum object;
	struct _device_datum device;
	struct _control_datum control;
};

struct _light_fixture_datum
{
	real_rgb_color color;
	real intensity;
	real falloff_angle;
	real cutoff_angle;
};

struct light_fixture_datum
{
	long definition_index;
	struct _object_datum object;
	struct _device_datum device;
	struct _light_fixture_datum light_fixture;
};

/* ---------- prototypes/DEVICES.C */

void devices_initialize(void);
void devices_dispose(void);
void devices_initialize_for_new_map(void);
void devices_dispose_from_old_map(void);

boolean device_new(long device_index);
void device_delete(long device_index);
boolean device_update(long device_index);
void device_export_function_values(long device_index);
void device_preprocess_node_orientations(long device_index, struct real_orientation *node_orientations);
void device_render_debug(long device_index);

boolean device_set_desired_position(long device_index, real desired_value);
real device_get_position(long device_index);
void device_set_power(long device_index, real power);
real device_get_power(long device_index);
boolean device_group_set_desired_value(short group_index, real desired_value);
void device_set_actual_position(long device_index, real desired_value);
void device_set_never_appears_locked(long device_index, boolean never_appears_locked);
void device_group_set_actual_value(short group_index, real value);
void device_one_sided_set(long device_index, boolean one_sided);
void device_operates_automatically_set(long device_index, boolean operates_automatically);
void device_group_change_only_once_more_set(long device_group_index, boolean change_only_once);
real device_group_get_value(short group_index);

void device_add_scenario_information(long device_index, struct scenario_device_datum *scenario_device);
void device_touched(long device_index, long unit_index);
boolean device_can_change_position(long device_index);
boolean device_frontfacing(long device_index, real_point3d const *point, real_vector3d const *vector);
void device_effect_new(long device_index, long effect_index);

/* ---------- prototypes/DEVICE_MACHINES.C */

void machines_initialize(void);
void machines_dispose(void);
void machines_initialize_for_new_map(void);
void machines_dispose_from_old_map(void);

void machine_place(long machine_index, struct scenario_machine_datum *scenario_machine);
boolean machine_new(long machine_index);
void machine_delete(long machine_index);
boolean machine_update(long machine_index);
void machine_bumped(long machine_index, long unit_index);
void machine_try_to_open_with_damage(long machine_index);

/* ---------- prototypes/DEVICE_LIGHT_FIXTURES.C */

void light_fixtures_initialize(void);
void light_fixtures_dispose(void);
void light_fixtures_initialize_for_new_map(void);
void light_fixtures_dispose_from_old_map(void);

void light_fixture_place(long light_fixture_index, struct scenario_light_fixture_datum *scenario_light_fixture);
boolean light_fixture_new(long light_fixture_index);
void light_fixture_delete(long light_fixture_index);
boolean light_fixture_update(long light_fixture_index);

/* ---------- prototypes/DEVICE_CONTROLS.C */

void controls_initialize(void);
void controls_dispose(void);
void controls_initialize_for_new_map(void);
void controls_dispose_from_old_map(void);

void control_place(long control_index, struct scenario_control_datum *scenario_control);
boolean control_new(long control_index);
void control_delete(long control_index);
boolean control_update(long control_index);
void control_touched(long control_index, long unit_index);
void control_destroyed(long control_index);

/* ---------- globals */

extern struct data_array *device_groups_data;

extern boolean debug_objects_devices;

/* ---------- public code */

#endif // __DEVICES_H
