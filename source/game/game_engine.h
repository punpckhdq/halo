/*
GAME_ENGINE.H

header included in hcex build.
*/

#ifndef __GAME_ENGINE_H
#define __GAME_ENGINE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/GAME_ENGINE.C */

boolean game_engine_running(void);

long game_engine_remap_object_definition(long definition_index);

long game_engine_remap_vehicle(long vehicle_definition_index);
long game_engine_remap_equipment(long equipment_definition_index);
long game_engine_remap_weapon(long weapon_definition_index);

/* ---------- globals */

extern struct game_engine *game_engine;

/* ---------- public code */

#endif // __GAME_ENGINE_H
