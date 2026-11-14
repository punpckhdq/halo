/*
COLLISION_DEBUG.H

header included in hcex build.
*/

#ifndef __COLLISION_DEBUG_H
#define __COLLISION_DEBUG_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/COLLISION_DEBUG.C */

void collision_debug_render(void);

/* ---------- globals */

extern boolean collision_debug;
extern boolean collision_debug_spray;
extern boolean collision_debug_features;
extern boolean collision_debug_repeat;
extern boolean collision_debug_flag_back_facing_surfaces;
extern boolean collision_debug_flag_ignore_two_sided_surfaces;
extern boolean collision_debug_flag_ignore_breakable_surfaces;
extern boolean collision_debug_flag_objects_bipeds;
extern boolean collision_debug_flag_objects_vehicles;
extern boolean collision_debug_flag_objects_weapons;
extern boolean collision_debug_flag_objects_equipment;
extern boolean collision_debug_flag_objects_projectiles;
extern boolean collision_debug_flag_objects_scenery;
extern boolean collision_debug_flag_objects_machines;
extern boolean collision_debug_flag_objects_controls;
extern boolean collision_debug_flag_objects_light_fixtures;
extern boolean collision_debug_flag_objects_placeholders;
extern boolean collision_debug_flag_try_to_keep_location_valid;
extern boolean collision_debug_flag_skip_passthrough_bipeds;
extern boolean collision_debug_flag_use_vehicle_physics;
extern real_point3d collision_debug_point;
extern real_vector3d collision_debug_vector;
extern boolean collision_debug_flag_front_facing_surfaces;
extern boolean collision_debug_flag_ignore_invisible_surfaces;
extern boolean collision_debug_flag_structure;
extern boolean collision_debug_flag_media;
extern boolean collision_debug_flag_objects;
extern real collision_debug_length;
extern real collision_debug_width;
extern real collision_debug_height;
extern boolean collision_debug_phantom_bsp;

/* ---------- public code */

#endif // __COLLISION_DEBUG_H
