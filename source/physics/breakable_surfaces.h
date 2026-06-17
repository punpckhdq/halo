/*
BREAKABLE_SURFACES.H

header included in hcex build.
*/

#ifndef __BREAKABLE_SURFACES_H
#define __BREAKABLE_SURFACES_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_BREAKABLE_SURFACE_QUEUE_SIZE = 1024,
};

enum
{
	MAXIMUM_BREAKABLE_SURFACES_PER_MAP = 256,
	MAXIMUM_PARTICLE_EFFECTS_PER_BREAKABLE_SURFACE_DEFINITION = 8,
};

/* ---------- macros */

/* ---------- structures */

struct breakable_surface_datum
{
	real vitality;
};

struct structure_breakable_surface
{
	real_point3d centroid;
	real bounding_radius;
	long collision_surface_index;
	long unused[7];
};

struct breakable_surface_particle_effect
{
	struct tag_reference particle;
	unsigned long flags;
	real density;
	real velocity_scale_lower_bound;
	real velocity_scale_upper_bound;
	real velocity_cone_angle;
	real angular_velocity_lower_bound;
	real angular_velocity_upper_bound;
	unsigned long unused1[2];
	real radius_lower_bound;
	real radius_upper_bound;
	unsigned long unused2[2];
	real_argb_color tint_lower_bound;
	real_argb_color tint_upper_bound;
	unsigned long unused3[7];
};

/* ---------- prototypes/BREAKABLE_SURFACES.C */

struct breakable_surface_datum *breakable_surface_get(short breakable_surface_index);
void breakable_surfaces_initialize(void);
void breakable_surfaces_dispose(void);
void breakable_surfaces_initialize_for_new_map(void);
void breakable_surfaces_dispose_from_old_map(void);
void breakable_surfaces_enable(boolean state);
void breakable_surfaces_reset(void);
byte *breakable_surface_flags_get(void);
boolean breakable_surface_extant(short breakable_surface_index);
void breakable_surface_damage(short breakable_surface_index, struct damage_data *damage_data, long seed_surface_index);
void breakable_surface_damage_area_of_effect(const struct damage_data *damage_data);

/* ---------- globals */

/* ---------- public code */

#endif // __BREAKABLE_SURFACES_H
