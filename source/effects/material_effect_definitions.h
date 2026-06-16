/*
MATERIAL_EFFECT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __MATERIAL_EFFECT_DEFINITIONS_H
#define __MATERIAL_EFFECT_DEFINITIONS_H
#pragma once

#include "real_math.h"
#include "tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct breakable_surface
{
	real maximum_vitality; // 0x0
	long unused1[2]; // 0x4
	unsigned long flags; // 0xC
	struct tag_reference effect; // 0x10
	struct tag_reference sound; // 0x20
	long unused2[6]; // 0x30
	struct tag_block particle_effects; // 0x48
};

struct material_definition
{
	unsigned long flags; // 0x0
	long modifiers_unused[24]; // 0x4
	unsigned long biped_flags; // 0x64
	real biped_maximum_acceleration; // 0x68
	real biped_slip_angle; // 0x6C
	real biped_slow_angle; // 0x70
	long biped_unused[8]; // 0x74
	real physics_ground_friction_scale; // 0x94
	real physics_ground_friction_normal_k1_scale; // 0x98
	real physics_ground_friction_normal_k0_scale; // 0x9C
	real physics_ground_depth_scale; // 0xA0
	real physics_ground_damp_fraction_scale; // 0xA4
	real physics_unused[19]; // 0xA8
	long unused[120]; // 0xF4
	struct breakable_surface breakable_surface; // 0x2D4
	long unused2[15]; // 0x328
	struct tag_reference melee_hit_sound; // 0x364
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __MATERIAL_EFFECT_DEFINITIONS_H
