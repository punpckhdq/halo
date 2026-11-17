/*
PROJECTILE_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "projectile_definitions.h"
#include "item_definitions.h"
#include "objects.h"
#include "ai.h"
#include "unit_definitions.h"
#include "sound_definitions.h"
#include "effect_definitions.h"
#include "physics_constants.h"
#include "damage_effect_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct projectile_material_response_definition default_projectile_material_response =
{
	0,														// flags
	_projectile_response_disappear,							// default_response
	{EFFECT_DEFINITION_TAG, "", 0, NONE},					// default_effect
	{0, 0, 0, 0},											// unused0
	_projectile_response_disappear,							// possible_response
	0,														// possible_response_flags
	0.f,													// possible_response_skip_fraction
	0.f,													// possible_response_minimum_angle
	0.f,													// possible_response_maximum_angle
	0.f,													// possible_response_minimum_velocity
	0.f,													// possible_response_maximum_velocity
	{EFFECT_DEFINITION_TAG, "", 0, NONE},					// possible_response_effect
	{0, 0, 0, 0},											// unused1
	_projectile_material_response_scale_effects_by_damage,	// scale_effects_by
	0,														// pad
	0.f,													// angle_noise
	0.f,													// velocity_noise
	{EFFECT_DEFINITION_TAG, "", 0, NONE},					// detonation_effect
	{0, 0, 0, 0, 0, 0},										// unused2
	0.f,													// penetration_initial_friction
	0.f,													// penetration_maximum_distance
	0.f,													// reflection_parallel_friction
	0.f														// reflection_perpendicular_friction
};

/* ---------- public code */

/* ---------- private code */
