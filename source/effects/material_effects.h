/*
MATERIAL_EFFECTS.H

header included in hcex build.
*/

#ifndef __MATERIAL_EFFECTS_H
#define __MATERIAL_EFFECTS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/MATERIAL_EFFECTS.C */

boolean material_effect_visible(real_point3d const *origin);
void material_effect_new(long effects_definition_index, short effect_index, short material_index, real_point3d const *origin, real_vector3d const *normal, struct location const *location, real scale);
void material_effect_new_from_point(long effects_definition_index, short effect_index, real_point3d const *origin, real scale);

/* ---------- globals */

/* ---------- public code */

#endif // __MATERIAL_EFFECTS_H
