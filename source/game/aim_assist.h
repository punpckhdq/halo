/*
AIM_ASSIST.H

header included in hcex build.
*/

#ifndef __AIM_ASSIST_H
#define __AIM_ASSIST_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct aim_assist_parameters
{
	real autoaim_angle;
	real autoaim_distance;
	real magnetism_angle;
	real magnetism_distance;
	real deviation_angle;
	real unused;
};

struct aim_assist_target
{
	long object_index;
	real_point3d position;
	real_vector3d vector;
	real_vector3d direction;
	real distance;
	real angle;
	real autoaim_level;
	real magnetism_level;
};

/* ---------- prototypes/AIM_ASSIST.C */

boolean aim_assist_clear_line_of_sight(real_point3d const *p0, real_point3d const *p1, long ignore_object_index, long target_object_index);
boolean aim_assist_compute_target(struct aim_assist_parameters const *parameters, long object_index, real_point3d const *position, real_vector3d const *direction, struct aim_assist_target *target);
boolean autoaim_compute_target(long object_index, real_point3d const *position, real_vector3d const *direction, long ignore_object_index, real_point3d *target_position, real_vector3d *target_direction, real *target_distance, real *target_angle);
boolean aim_assist(struct aim_assist_parameters const *parameters, real_point3d const *position, real_vector3d const *direction, long ignore_object_index, short ignore_team_index, struct aim_assist_target *target);
long player_aim_projectile(long player_index, real_point3d const *position, real_vector3d *direction);
long local_player_aim_assist(short local_player_index, real *autoaim_level, real *magnetism_level, real_euler_angles2d *target_angular_position, real_euler_angles2d *target_angular_velocity);

/* ---------- globals */

/* ---------- public code */

#endif // __AIM_ASSIST_H
