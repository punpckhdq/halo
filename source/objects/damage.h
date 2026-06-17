/*
DAMAGE.H

header included in hcex build.
*/

#ifndef __DAMAGE_H
#define __DAMAGE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct damage_data
{
	long definition_index;
	unsigned long flags;
	long owner_player_index;
	long owner_object_index;
	short owner_team_index;
	struct location location;
	real_point3d origin;
	real_point3d epicenter;
	real_vector3d direction;
	real scale;
	real multiplier;
	real material_effect_scale;
	short material_type;
	struct projectile_material_response_definition const *material_response;
};

/* ---------- prototypes/DAMAGE.C */

void damage_initialize(void);
void damage_dispose(void);
void damage_initialize_for_new_map(void);
void damage_dispose_from_old_map(void);
void damage_render_debug(void);
void object_initialize_vitality(long object_index, real *custom_body_vitality, real *custom_shield_vitality);

void object_damage_update(long object_index);

/* ---------- globals */

/* ---------- public code */

#endif // __DAMAGE_H
