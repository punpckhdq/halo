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
