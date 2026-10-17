/*
MATERIAL_EFFECT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __MATERIAL_EFFECT_DEFINITIONS_H
#define __MATERIAL_EFFECT_DEFINITIONS_H
#pragma once

/* ---------- headers */

/* ---------- constants */

enum
{
	MATERIAL_EFFECTS_DEFINITION_TAG = 'foot',
	MATERIAL_EFFECTS_DEFINITION_VERSION = 1,
};

enum
{
	_material_effect_biped_walk = 0,
	_material_effect_biped_run,
	_material_effect_biped_sliding,
	_material_effect_biped_shuffle,
	_material_effect_biped_jump,
	_material_effect_biped_jump_land,
	_material_effect_biped_unused1,
	_material_effect_biped_unused2,
	_material_effect_impact,
	_material_effect_vehicle_slip_nonmetallic,
	_material_effect_vehicle_slip_metallic,
	_material_effect_vehicle_unused1,
	_material_effect_vehicle_unused2,
	NUMBER_OF_MATERIAL_EFFECT_TYPES,
};

/* ---------- macros */

#define material_effects_definition_get(index) ((struct material_effects_definition *)tag_get(MATERIAL_EFFECTS_DEFINITION_TAG, index))

/* ---------- structures */

struct material_effect_material
{
	struct tag_reference effect;
	struct tag_reference sound;
	long unused[4];
};

struct material_effect
{
	struct tag_block materials;
	long unused[4];
};

struct material_effects_definition
{
	struct tag_block effects;
	long unused[32];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __MATERIAL_EFFECT_DEFINITIONS_H
