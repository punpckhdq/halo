/*
UNIT_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "unit_definitions.h"
#include "units.h"
#include "vehicle_definitions.h"
#include "biped_definitions.h"
#include "dialogue_definitions.h"
#include "actor_definitions.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "effect_definitions.h"
#include "physics_constants.h"
#include "damage_effect_definitions.h"
#include "equipment_definitions.h"
#include "projectile_definitions.h"
#include "unit_hud_interface_definition.h"
#include "camera_track_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

long unit_definition_get_active_hud_index(
	struct unit_definition const *definition,
	boolean multiplayer)
{
	short hud_index = multiplayer ? _unit_hud_multiplayer : _unit_hud_default;

	hud_index = MIN(hud_index, definition->unit.huds.count-1);
	if (hud_index < 0)
	{
		return NONE;
	}

	return verify_tag_reference(&TAG_BLOCK_GET_ELEMENT(&definition->unit.huds, hud_index, struct unit_hud_reference)->unit_hud_interface);
}

long unit_definition_get_seat_active_hud_index(
	struct unit_definition const *definition,
	short seat_index,
	boolean multiplayer)
{
	struct unit_seat const *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, seat_index, struct unit_seat);
	short hud_index = multiplayer ? _unit_hud_multiplayer : _unit_hud_default;

	hud_index = MIN(hud_index, seat->seat_huds.count-1);
	if (hud_index < 0)
	{
		return NONE;
	}

	return verify_tag_reference(&TAG_BLOCK_GET_ELEMENT(&seat->seat_huds, hud_index, struct unit_hud_reference)->unit_hud_interface);
}

/* ---------- private code */
