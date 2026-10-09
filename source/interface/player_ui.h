/*
PLAYER_UI.H

header included in hcex build.
*/

#ifndef __PLAYER_UI_H
#define __PLAYER_UI_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PLAYER_UI.C */

short player_ui_get_single_player_local_player_controller(short local_player_index);
boolean player_ui_local_player_wants_to_play_multiplayer(short local_player_index);
boolean player_ui_rumble_disabled(short local_player_index);

void player_ui_get_active_player_profile(short local_player_index, struct player_profile *profile);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_UI_H
