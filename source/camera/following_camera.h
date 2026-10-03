/*
FOLLOWING_CAMERA.H

header included in hcex build.
*/

#ifndef __FOLLOWING_CAMERA_H
#define __FOLLOWING_CAMERA_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct following_camera
{
	boolean initialized;
	boolean confined;
	boolean crouched;
	boolean zoomed;
	short zoom_level;
	long unit_index;
	short seat_index;
	real_euler_angles2d facing_offset;
	real distance_scale;
};

/* ---------- prototypes/FOLLOWING_CAMERA.C */

void following_camera_new(struct following_camera *camera);
void following_camera_deterministic(long unit_index, real_point3d *position, real_vector3d *forward);
void following_camera_update(struct following_camera *camera, struct camera_control const *controls, struct observer_command *result);

/* ---------- globals */

/* ---------- public code */

#endif // __FOLLOWING_CAMERA_H
