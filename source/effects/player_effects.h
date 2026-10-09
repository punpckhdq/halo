/*
PLAYER_EFFECTS.H

header included in hcex build.
*/

#ifndef __PLAYER_EFFECTS_H
#define __PLAYER_EFFECTS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PLAYER_EFFECTS.C */

void player_effect_continuous_refresh(long effect_index, real_point3d const *origin);

void player_effect_get_camera_effect_matrix(short local_player_index, real_matrix4x3 *matrix);

void player_effect_get_damage_indicators(short local_player_index, byte *damage_indicators);
void player_effect_clear_damage_indicators(short local_player_index);
void player_effect_start(long player_index, struct damage_data const *damage_data, real_vector3d const *direction, real scale, real total_damage);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_EFFECTS_H
