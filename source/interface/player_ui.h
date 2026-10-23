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
void player_ui_reset_single_player_local_player_controllers(void);
void player_ui_set_single_player_local_player_controller(short local_player_index, short controller_index);
short player_ui_get_single_player_local_player_controller(short local_player_index);
short player_ui_get_single_player_local_player_from_controller(short controller_index);
void player_ui_local_player_joined_multiplayer_game(short local_player_index);
boolean player_ui_local_player_wants_to_play_multiplayer(short local_player_index);
void player_ui_clear_multiplayer_autojoin_for_local_player(short local_player_index);
void player_ui_autojoin_players_to_next_multiplayer_game(void);
void player_ui_clear_multiplayer_joins(void);
void player_ui_clear_multiplayer_variant(void);
void player_ui_set_active_player_profile(short local_player_index, long profile_index, struct player_profile const *profile);
void player_ui_get_active_player_profile(short local_player_index, struct player_profile *profile);
long player_ui_get_active_player_profile_index(short local_player_index);
short player_ui_get_last_single_player_level_played(short local_player_index);
void player_ui_set_game_variant(struct game_variant const *variant);
boolean player_ui_game_variant_specified(struct game_variant *variant);
boolean player_ui_rumble_disabled(short local_player_index);
boolean player_ui_autolevel_enabled(short controller_index);
boolean player_ui_get_path_to_local_player_profile_directory(short local_player_index, char *full_path);
void player_ui_remember_player1_profile(boolean write_to_disk);
long player_ui_get_player1_last_used_profile_index(void);
void player_ui_fast_setup_network_server(void);
boolean player_ui_edit_profile_is_default_profile(void);
boolean player_ui_edit_profile_name_is_dirty(void);
boolean player_ui_prompt_user_to_rename_edit_profile(void);
struct player_profile *player_ui_get_edit_player_profile(void);
struct game_variant *player_ui_get_edit_playlist_profile(void);
void player_ui_begin_editing_profile(long profile_index);
boolean player_ui_save_profile(void);
void player_ui_end_editing_profile(void);
boolean player_ui_edit_profile_is_dirty(void);
void player_ui_activate_all_solo_levels(void);
void player0_look_invert_pitch(boolean invert);
boolean player0_look_pitch_is_inverted(void);
boolean player0_joystick_set_is_normal(void);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_UI_H
