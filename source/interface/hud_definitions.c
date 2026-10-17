/*
HUD_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hud_definitions.h"
#include "hud.h"
#include "weapon_hud_interface_definition.h"
#include "hud_messaging_definitions.h"
#include "objects.h"
#include "network_game_globals.h"
#include "players.h"
#include "input.h"
#include "text_group.h"
#include "sound_definitions.h"
#include "font_group.h"

/* ---------- globals */

char const *global_hud_anchor_names[NUMBER_OF_HUD_ANCHORS] =
{
	"top_left",
	"top_right",
	"bottom_left",
	"bottom_right",
	"center"
};
