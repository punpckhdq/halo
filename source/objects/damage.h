/*
DAMAGE.H

header included in hcex build.
*/

#ifndef __DAMAGE_H
#define __DAMAGE_H
#pragma once

/* ---------- headers */

#include "objects.h"

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
void damage_data_new(struct damage_data *damage_data, long definition_index);

void object_cause_damage(struct damage_data *damage_data, long object_index, short node_index, short region_index, short material_index, real_vector3d const *object_normal);

/* ---------- globals */

/* ---------- public code */

#endif // __DAMAGE_H
