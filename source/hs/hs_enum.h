/*
HS_ENUM.H

header included in hcex build.
*/

#ifndef __HS_ENUM_H
#define __HS_ENUM_H
#pragma once

/* ---------- globals */

struct hs_enum_definition const hs_enum_table[NUMBER_OF_HS_ENUM_TYPES] =
{
	{NUMBER_OF_GAME_DIFFICULTY_LEVELS, global_game_difficulty_level_names},	// _hs_type_enum_game_difficulty
	{NUMBER_OF_SOLO_CAMPAIGN_TEAMS, global_game_team_names},					// _hs_type_enum_team
	{NUMBER_OF_ACTOR_DEFAULT_STATES, global_ai_default_state_names},			// _hs_type_enum_ai_default_state
	{NUMBER_OF_ACTOR_TYPES, global_actor_type_names},							// _hs_type_enum_actor_type
	{NUMBER_OF_HUD_ANCHORS, global_hud_anchor_names}							// _hs_type_enum_hud_corner
};

#endif // __HS_ENUM_H
