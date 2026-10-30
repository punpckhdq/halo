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

void player_ui_initialize(void);
void player_ui_dispose(void);
void player_ui_remember_player1_profile(boolean write_to_disk);
short player_ui_get_single_player_local_player_controller(short local_player_index);
short player_ui_get_single_player_local_player_from_controller(short controller_index);
boolean player_ui_autolevel_enabled(short controller_index);
boolean player_ui_local_player_wants_to_play_multiplayer(short local_player_index);
boolean player_ui_rumble_disabled(short local_player_index);
void player_ui_get_active_player_profile(short local_player_index, struct player_profile *profile);
long player_ui_get_active_player_profile_index(short local_player_index);
boolean player_ui_get_path_to_local_player_profile_directory(short local_player_index, char *full_path);
void player_ui_set_active_player_profile(short local_player_index, long profile_index, struct player_profile *profile);
boolean player_ui_game_variant_specified(struct game_variant *variant);

void player_ui_get_active_player_profile(short local_player_index, struct player_profile *profile);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_UI_H
