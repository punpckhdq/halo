/*
OBSERVER.H

header included in hcex build.
*/

#ifndef __OBSERVER_H
#define __OBSERVER_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	OBSERVER_SIGNATURE = 'rad!',
};

enum
{
	_observer_command_valid_bit = 0,
	_observer_command_force_under_media_bit,
	_observer_command_force_above_media_bit,
	_observer_command_force_time_bit,
	_observer_command_ignore_obstructions_bit,
	_observer_command_freeze_camera_bit,
	NUMBER_OF_OBSERVER_COMMAND_FLAGS,
};

enum
{
	_observer_time_valid_bit = 0,
	_observer_time_force_bit,
	NUMBER_OF_OBSERVER_TIME_FLAGS,
};

enum
{
	_observer_focus_position = 0,
	_observer_focus_offset = 1,
	NUMBER_OF_OBSERVER_REAL3D_PARAMETERS = 2,
	_observer_focus_distance = 2,
	_observer_field_of_view = 3,
	NUMBER_OF_OBSERVER_CARTESIAN_PARAMETERS = 4,
	_observer_orientation = 4,
	NUMBER_OF_OBSERVER_PARAMETERS = 5,
	NUMBER_OF_OBSERVER_REAL1D_PARAMETERS = 2,
	NUMBER_OF_OBSERVER_POLAR_PARAMETERS = 1,
	NUMBER_OF_OBSERVER_CARTESIAN_REALS = 8,
	NUMBER_OF_OBSERVER_POLAR_REALS = 6,
	NUMBER_OF_OBSERVER_REAL_PARAMETERS = 14,
	NUMBER_OF_OBSERVER_CARTESIAN_VELOCITIES = 8,
	NUMBER_OF_OBSERVER_POLAR_VELOCITIES = 3,
	NUMBER_OF_OBSERVER_REAL_VELOCITIES = 11,
};

/* ---------- macros */

#define MAXIMUM_WORLD_COORDINATE 5000.f
#define MAXIMUM_OBSERVER_TIMER 3600.f
#define DEFAULT_HORIZONTAL_FIELD_OF_VIEW 70.f
#define DEFAULT_HORIZONTAL_FIELD_OF_VIEW_CHANGE_TIME 0.18f

#define valid_world_real(n) (valid_real(n) && (n) >= -MAXIMUM_WORLD_COORDINATE && (n) <= MAXIMUM_WORLD_COORDINATE)
#define valid_world_real_point3d(point) (valid_world_real((point)->x) && valid_world_real((point)->y) && valid_world_real((point)->z))
#define valid_focus_distance(distance) (valid_real(distance) && (distance) >= 0.f && (distance) <= MAXIMUM_WORLD_COORDINATE)
#define valid_field_of_view(field_of_view) (valid_real(field_of_view) && (field_of_view) >= 0.001f && (field_of_view) <= _half_pi)
#define valid_timer(time) (valid_real(time) && (time) >= 0.f && (time) <= MAXIMUM_OBSERVER_TIMER)

#define observer_valid_camera_command(command)																							\
	((command) &&																														\
	(!TEST_FLAG((command)->flags, _observer_command_valid_bit) ||																		\
	(valid_real_vector3d_axes2(&(command)->forward, &(command)->up) &&																	\
	valid_world_real_point3d(&(command)->focus_position) &&																				\
	valid_world_real_point3d((real_point3d *)&(command)->focus_offset) &&																\
	valid_real_vector3d(&(command)->focus_velocity) &&																					\
	valid_focus_distance((command)->focus_distance) &&																					\
	valid_field_of_view((command)->field_of_view) &&																					\
	valid_timer((command)->timer))))

#define assert_valid_observer_command(command)																							\
vassert(																																\
	observer_valid_camera_command(command),																								\
	csprintf(																														\
		temporary,																														\
		"Invalid camera command.\nF: (%f, %f, %f) U: (%f, %f, %f)\nP: (%f, %f, %f) O: (%f, %f, %f)\nD: %f V: (%f, %f, %f), FOV: %f, T: %f, FL: %ld",	\
		(command)->forward.i, (command)->forward.j, (command)->forward.k,																\
		(command)->up.i, (command)->up.j, (command)->up.k,																				\
		(command)->focus_position.x, (command)->focus_position.y, (command)->focus_position.z,											\
		(command)->focus_offset.i, (command)->focus_offset.j, (command)->focus_offset.k,												\
		(command)->focus_distance,																										\
		(command)->focus_velocity.i, (command)->focus_velocity.j, (command)->focus_velocity.k,											\
		(command)->field_of_view,																										\
		(command)->timer,																												\
		(command)->flags))

#define match_assert_valid_observer_command(file, line, command)																		\
match_vassert(																															\
	file,																																\
	line,																																\
	observer_valid_camera_command(command),																								\
	csprintf(																														\
		temporary,																														\
		"Invalid camera command.\nF: (%f, %f, %f) U: (%f, %f, %f)\nP: (%f, %f, %f) O: (%f, %f, %f)\nD: %f V: (%f, %f, %f), FOV: %f, T: %f, FL: %ld",	\
		(command)->forward.i, (command)->forward.j, (command)->forward.k,																\
		(command)->up.i, (command)->up.j, (command)->up.k,																				\
		(command)->focus_position.x, (command)->focus_position.y, (command)->focus_position.z,											\
		(command)->focus_offset.i, (command)->focus_offset.j, (command)->focus_offset.k,												\
		(command)->focus_distance,																										\
		(command)->focus_velocity.i, (command)->focus_velocity.j, (command)->focus_velocity.k,											\
		(command)->field_of_view,																										\
		(command)->timer,																												\
		(command)->flags))

/* ---------- structures */

struct observer_command
{
	long flags;
	union
	{
		struct
		{
			real_point3d focus_position;
			real_vector3d focus_offset;
			real focus_distance;
			real field_of_view;
			real_vector3d forward;
			real_vector3d up;
		};
		real parameters[14];
	};
	real_vector3d focus_velocity;
	real timer;
	union
	{
		struct
		{
			byte position_flags;
			byte focus_offset_flags;
			byte distance_flags;
			byte field_of_view_flags;
			byte orientation_flags;
		};
		byte parameter_flags[5];
	};
	union
	{
		struct
		{
			real position_timer;
			real focus_offset_timer;
			real distance_timer;
			real field_of_view_timer;
			real orientation_timer;
		};
		real parameter_timers[5];
	};
};

struct observer_result
{
	real_point3d position;
	struct location location;
	real_vector3d velocity;
	real_vector3d forward;
	real_vector3d up;
	real field_of_view;
};

struct observer_derivative
{
	union
	{
		struct
		{
			real_vector3d focus_position;
			real_vector3d focus_offset;
			real focus_distance;
			real field_of_view;
			real_vector3d rotation;
			real_vector3d rotation_offset;
		};
		real n[11];
	};
};

struct observer
{
	long header_signature;
	struct observer_command *pending_command;
	struct observer_command last_command;
	boolean updated_for_frame;
	boolean first_command;
	struct observer_result result;
	union
	{
		struct
		{
			real_point3d focus_position;
			real_vector3d focus_offset;
			real focus_distance;
			real field_of_view;
			real_vector3d forward;
			real_vector3d up;
		};
		real positions[14];
	};
	struct observer_derivative velocities;
	struct observer_derivative accelerations;
	real a[11];
	real b[11];
	real c[11];
	real d[11];
	real e[11];
	real f[11];
	struct observer_derivative displacements;
	long trailer_signature;
};

/* ---------- prototypes/OBSERVER.C */

void observer_initialize(void);
void observer_initialize_for_new_map(void);
void observer_dispose_from_old_map(void);
void observer_set_camera(short local_player_index, struct observer_command *command);
void observer_update(real dt);

struct observer_result const *observer_get_camera(short local_player_index);
boolean observer_command_has_finished(short local_player_index);
void observer_up_from_forward(real_vector3d const *forward, real_vector3d *up);
void observer_reconnect_to_structure_bsp(void);
void observer_obsolete_position(short local_player_index);

void observer_reconnect_to_structure_bsp(void);

/* ---------- globals */

/* ---------- public code */

#endif // __OBSERVER_H
