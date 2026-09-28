/*
PLACEHOLDER_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "object_definitions.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "placeholder_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "models.h"

/* ---------- public code */

void placeholder_initialize(
	void)
{
	return;
}

void placeholder_initialize_for_new_map(
	void)
{
	return;
}

void placeholder_dispose_from_old_map(
	void)
{
	return;
}

void placeholder_dispose(
	void)
{
	return;
}

void placeholder_place(
	long placeholder_index,
	struct scenario_placeholder_datum *scenario_placeholder)
{
	return;
}

boolean placeholder_new(
	long object_index)
{
	return TRUE;
}

void placeholder_delete(
	long placeholder_index)
{
	return;
}
