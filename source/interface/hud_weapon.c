/*
HUD_WEAPON.C
*/

/* ---------- headers */

#include "cseries.h"
#include "meter_definitions.h"
#include "hud.h"
#include "hud_messaging.h"
#include "weapon_hud_interface_definition.h"
#include "motion_sensor.h"
#include "interface_panels.h"
#include "inventory_displays.h"
#include "unit_hud_interface_definition.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "network_messages.h"
#include "collisions.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
#include "director.h"
#include "weapons.h"
#include "texture_cache.h"
#include "physics_constants.h"
#include "font_group.h"

/* ---------- constants */

enum
{
	_hud_crosshair_show_bit = 0,
};

enum
{
	NO_TARGET_NUMBER_BITS = 0xFFC00000,
};

/* ---------- structures */

union weapon_hud_number /* fake name */
{
	real value; /* fake name */
	long bits; /* fake name */
};

struct weapon_hud_state
{
	long last_weapon_flash_time[NUMBER_OF_WEAPON_HUD_STATES];
	long last_weapon_index;
	long last_grenade_flash_time;
};

union crosshair_state
{
	long reference_data;
	real frame_index;
};

struct crosshair_hud_state
{
	union crosshair_state states[NUMBER_OF_CROSSHAIR_STATES];
	unsigned long render_flags;
};

struct weapon_hud_globals_definition
{
	struct weapon_hud_state hud_state[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct crosshair_hud_state crosshair_state[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	long script_flags;
};

/* ---------- prototypes */

static void hud_update_weapon_local_player(short local_player_index, long weapon_index, long hud_index, struct weapon_interface_state *weapon_state);
static struct weapon_hud_state *get_hud_state(short local_player_index);
static struct crosshair_hud_state *get_crosshair_state(short local_player_index);
static void render_weapon_hud(
	long hud_index,
	short local_player_index,
	struct weapon_definition const *weapon_definition,
	struct weapon_interface_state const *weapon_state,
	short const *new_state_flags,
	short const *new_overlay_flags,
	short const *new_numbers);
static void play_weapon_hud_sounds(short local_player_index, long hud_index, struct weapon_interface_state const *weapon_state);
static void render_grenade_hud(short local_player_index, long unit_index);
static void crosshairs_draw(struct player_datum *player, long weapon_index, long hud_index, struct weapon_interface_state *weapon_state);

/* ---------- globals */

static struct weapon_hud_globals_definition *weapon_hud_globals = NULL;

/* ---------- public code */

void hud_initialize_weapon_interface(
	void)
{
	weapon_hud_globals = (struct weapon_hud_globals_definition *)game_state_malloc("hud weapon interface", NULL, sizeof(*weapon_hud_globals));
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 107, weapon_hud_globals);

	return;
}

void hud_initialize_weapon_interface_for_new_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 115, weapon_hud_globals);
	memset(weapon_hud_globals, NONE, sizeof(*weapon_hud_globals));

	return;
}

void hud_dispose_weapon_interface_from_old_map(
	void)
{
	return;
}

void hud_dispose_weapon_interface(
	void)
{
	return;
}

void hud_fix_weapon_data(
	short old_local_player_index,
	short new_local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 137, old_local_player_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 138, new_local_player_index!=NONE);

	*get_hud_state(new_local_player_index) = *get_hud_state(old_local_player_index);
	*get_crosshair_state(new_local_player_index) = *get_crosshair_state(old_local_player_index);

	return;
}

void hud_update_weapon(
	void)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	short local_player_index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	for (local_player_index = local_player_get_next(NONE); local_player_index != NONE; local_player_index = local_player_get_next(local_player_index))
	{
		long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;

		if (unit_index != NONE)
		{
			long weapon_index = unit_inventory_get_weapon(unit_index, unit_get(unit_index)->unit.current_weapon_index);
			boolean seat_has_no_weapon = FALSE;

			if (weapon_index == NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);

				if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
				{
					struct unit_datum *parent = unit_get(unit->object.parent_object_index);
					struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition_get(parent->definition_index)->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

					if (TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit))
					{
						weapon_index = unit_inventory_get_weapon(unit->object.parent_object_index, unit_get(unit->object.parent_object_index)->unit.current_weapon_index);
					}
					else
					{
						seat_has_no_weapon = TRUE;
					}
				}
			}

			if (weapon_index != NONE)
			{
				struct weapon_definition *weapon_definition = weapon_definition_get(weapon_get(weapon_index)->definition_index);
				struct weapon_interface_state weapon_state;

				weapon_build_weapon_interface_state(weapon_index, &weapon_state);

				if (weapon_definition->weapon.interface_definition.hud_interface.index != NONE)
				{
					long hud_index = weapon_definition->weapon.interface_definition.hud_interface.index;

					hud_update_weapon_local_player(local_player_index, weapon_index, hud_index, &weapon_state);
				}
			}
			else if (!seat_has_no_weapon && !unit_get_weapon_count(unit_index))
			{
				struct weapon_interface_state weapon_state = { 0 };

				hud_update_weapon_local_player(
					local_player_index,
					NONE,
					hud_globals->defaults.default_weapon_hud.index,
					&weapon_state);
			}

			get_hud_state(local_player_index)->last_weapon_index = weapon_index;
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 212);
	}

	return;
}

static void hud_update_weapon_local_player(
	short local_player_index,
	long weapon_index,
	long hud_index,
	struct weapon_interface_state *weapon_state)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct player_datum *player;
	long unit_index;
	struct unit_datum *unit;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	player = player_get(local_player_get_player_index(local_player_index));
	unit_index = player->unit_index;
	unit = unit_try_and_get(unit_index);

	if (unit)
	{
		struct crosshair_hud_state *crosshair = get_crosshair_state(player->local_player_index);
		struct weapon_hud_interface_definition *definition = weapon_hud_interface_definition_get(hud_index);
		struct weapon_datum *weapon = weapon_index == NONE ? NULL : weapon_get(weapon_index);
		struct weapon_definition *weapon_definition = weapon_index == NONE ? NULL : weapon_definition_get(weapon->definition_index);
		struct weapon_hud_interface_definition *weapon_hud_hierarchy[MAXIMUM_WEAPON_HUD_DEPTH] = { definition };
		long weapon_hud_indices[MAXIMUM_WEAPON_HUD_DEPTH] = { hud_index };
		unsigned long valid_crosshair_types = weapon_hud_hierarchy[0]->valid_crosshair_types_flags;
		short crosshair_index;
		unsigned long render_flags = 0;
		short depth = 1;

		if (weapon_index != get_hud_state(local_player_index)->last_weapon_index && weapon_index == NONE)
		{
			memset(crosshair, 0, sizeof(*crosshair));
		}

		while (depth < MAXIMUM_WEAPON_HUD_DEPTH && weapon_hud_hierarchy[depth - 1]->parent_hud.index != NONE)
		{
			weapon_hud_indices[depth] = weapon_hud_hierarchy[depth - 1]->parent_hud.index;
			weapon_hud_hierarchy[depth] = weapon_hud_interface_definition_get(weapon_hud_indices[depth]);
			valid_crosshair_types |= weapon_hud_hierarchy[depth]->valid_crosshair_types_flags;
			depth++;
		}

		if (depth == MAXIMUM_WEAPON_HUD_DEPTH)
		{
			error(_error_silent, "too many levels in current weapon hud hierarchy");
		}

		for (crosshair_index = 0; crosshair_index < NUMBER_OF_CROSSHAIR_STATES; crosshair_index++)
		{
			if (TEST_FLAG(valid_crosshair_types, crosshair_index))
			{
				union crosshair_state *state = &crosshair->states[crosshair_index];
				short value;

				switch (crosshair_index)
				{
				case _crosshair_state_primary_trigger_ready:
					value = weapon_state->magazines[0].can_fire;
					break;
				case _crosshair_state_secondary_trigger_ready:
					value = weapon_state->magazines[1].can_fire;
					break;
				case _crosshair_state_aim:
					value = weapon_index == NONE ? FALSE : player_control_get_autoaim_level(player->local_player_index) == 1.f;
					break;
				case _crosshair_state_zoom:
					value = player_control_get_zoom_level(player->local_player_index) == NONE ? 1 : player_control_get_zoom_level(player->local_player_index) + 2;
					break;
				case _crosshair_state_reload:
					value = weapon_state->magazines[0].reloading;
					break;
				case _crosshair_state_flash_ammo:
					value = (weapon_state->magazines[0].rounds_remaining || weapon_state->magazines[0].rounds_loaded) && weapon_state->magazines[0].rounds_remaining && weapon_state->magazines[0].rounds_loaded <= definition->flash_cutoffs.loaded_ammo;
					break;
				case _crosshair_state_flash_heat:
					value = weapon_state->heat * 100.f >= definition->flash_cutoffs.heat;
					break;
				case _crosshair_state_flash_total_ammo:
					value = weapon_state->magazines[0].rounds_remaining <= definition->flash_cutoffs.total_ammo && !weapon_state->magazines[0].reloading;
					break;
				case _crosshair_state_flash_total_battery:
					value = weapon_state->age < 1.f && (1.f - weapon_state->age) * 100.f <= definition->flash_cutoffs.age;
					break;
				case _crosshair_state_charge:
					value = FALSE;
					break;
				case _crosshair_state_flash_fired_battery_depleted:
					value = (weapon_state->age == 1.f && TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit)) || state->reference_data != NONE;
					break;
				case _crosshair_state_fired_with_no_ammo:
					value = (!weapon_state->magazines[0].rounds_loaded && !weapon_state->magazines[0].rounds_remaining && TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit)) || state->reference_data != NONE;
					break;
				case _crosshair_state_threw_with_no_grenade:
				{
					boolean no_grenades = TRUE;
					short grenade_index;

					for (grenade_index = 0; grenade_index < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_index++)
					{
						no_grenades = no_grenades && !unit->unit.grenade_counts[grenade_index];
					}

					value = (no_grenades && !unit->unit.grenade_throw_state && TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit)) || state->reference_data != NONE;
					break;
				}
				case _crosshair_state_flash_ammo_none_for_reload:
					value = (weapon_state->magazines[0].rounds_remaining || weapon_state->magazines[0].rounds_loaded) && !weapon_state->magazines[0].rounds_remaining && weapon_state->magazines[0].rounds_loaded <= definition->flash_cutoffs.loaded_ammo;
					break;
				case _crosshair_state_flash_secondary_ammo:
					value = (weapon_state->magazines[1].rounds_remaining || weapon_state->magazines[1].rounds_loaded) && weapon_state->magazines[1].rounds_remaining && weapon_state->magazines[1].rounds_loaded <= definition->flash_cutoffs.loaded_ammo;
					break;
				case _crosshair_state_flash_secondary_total_ammo:
					value = weapon_state->magazines[1].rounds_remaining <= definition->flash_cutoffs.total_ammo && !weapon_state->magazines[1].reloading;
					break;
				case _crosshair_state_secondary_reload:
					value = weapon_state->magazines[1].reloading;
					break;
				case _crosshair_state_fired_secondary_with_no_ammo:
					value = (!weapon_state->magazines[1].rounds_loaded && !weapon_state->magazines[1].rounds_remaining && TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit)) || state->reference_data != NONE;
					break;
				case _crosshair_state_flash_secondary_ammo_none_for_reload:
					value = (weapon_state->magazines[1].rounds_remaining || weapon_state->magazines[1].rounds_loaded) && !weapon_state->magazines[1].rounds_remaining && weapon_state->magazines[1].rounds_loaded <= definition->flash_cutoffs.loaded_ammo;
					break;
				default:
					match_unreachable("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 366);
					break;
				}

				if (value <= 0 && crosshair_index != _crosshair_state_aim)
				{
					render_flags &= ~FLAG(crosshair_index);
				}
				else
				{
					render_flags |= FLAG(crosshair_index);
				}

				switch (crosshair_index)
				{
				case _crosshair_state_aim:
					state->reference_data = value;
					break;
				case _crosshair_state_zoom:
					state->reference_data = value - 1;
					break;
				case _crosshair_state_charge:
				case _crosshair_state_flash_ammo:
				case _crosshair_state_flash_heat:
				case _crosshair_state_flash_total_ammo:
				case _crosshair_state_flash_total_battery:
				case _crosshair_state_reload:
				case _crosshair_state_fired_with_no_ammo:
				case _crosshair_state_threw_with_no_grenade:
				case _crosshair_state_flash_ammo_none_for_reload:
				case _crosshair_state_flash_secondary_ammo:
				case _crosshair_state_flash_secondary_total_ammo:
				case _crosshair_state_secondary_reload:
				case _crosshair_state_fired_secondary_with_no_ammo:
				case _crosshair_state_flash_secondary_ammo_none_for_reload:
				case _crosshair_state_primary_trigger_ready:
				case _crosshair_state_secondary_trigger_ready:
				case _crosshair_state_flash_fired_battery_depleted:
					if (!value)
					{
						state->reference_data = NONE;
					}
					else if (state->reference_data == NONE)
					{
						state->reference_data = game_time_get();
					}
					break;
				default:
					match_unreachable("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 407);
					break;
				}
			}
		}

		crosshair->render_flags = render_flags;
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 414);
	}

	return;
}

void scripted_hud_show_crosshair(
	boolean show)
{
	SET_FLAG(weapon_hud_globals->script_flags, _hud_crosshair_show_bit, show);

	return;
}

static struct weapon_hud_state *get_hud_state(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 430, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 431, weapon_hud_globals);

	return &weapon_hud_globals->hud_state[local_player_index];
}

static struct crosshair_hud_state *get_crosshair_state(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 439, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 440, weapon_hud_globals);

	return &weapon_hud_globals->crosshair_state[local_player_index];
}

void hud_render_weapon_interface(
	struct player_datum *player)
{
	long weapon_index = unit_inventory_get_weapon(player->unit_index, unit_get(player->unit_index)->unit.current_weapon_index);
	boolean seat_has_no_weapon = FALSE;

	if (weapon_index == NONE)
	{
		struct unit_datum *unit = unit_get(player->unit_index);

		if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
		{
			struct unit_datum *parent = unit_get(unit->object.parent_object_index);
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition_get(parent->definition_index)->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

			if (TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit))
			{
				weapon_index = unit_inventory_get_weapon(unit->object.parent_object_index, unit_get(unit->object.parent_object_index)->unit.current_weapon_index);
			}
			else
			{
				seat_has_no_weapon = TRUE;
			}
		}
	}

	match_assert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 472, player->local_player_index==render.local_player_index);

	if (weapon_index != NONE)
	{
		struct weapon_definition *weapon_definition = weapon_definition_get(weapon_get(weapon_index)->definition_index);
		struct weapon_interface_state weapon_state;

		weapon_build_weapon_interface_state(weapon_index, &weapon_state);

		if (weapon_definition->weapon.interface_definition.hud_interface.index != NONE)
		{
			long hud_index = weapon_definition->weapon.interface_definition.hud_interface.index;

			crosshairs_draw(player, weapon_index, hud_index, &weapon_state);
			render_weapon_hud(
				hud_index,
				player->local_player_index,
				weapon_definition,
				&weapon_state,
				NULL,
				NULL,
				NULL);
			play_weapon_hud_sounds(player->local_player_index, hud_index, &weapon_state);
		}
	}
	else if (!seat_has_no_weapon && !unit_get_weapon_count(player->unit_index))
	{
		struct weapon_interface_state weapon_state = { 0 };

		crosshairs_draw(player, NONE, hud_globals->defaults.default_weapon_hud.index, &weapon_state);
	}

	render_grenade_hud(player->local_player_index, player->unit_index);

	if (player->local_player_index != NONE)
	{
		get_hud_state(player->local_player_index)->last_weapon_index = weapon_index;
	}

	return;
}

static void render_weapon_hud(
	long hud_index,
	short local_player_index,
	struct weapon_definition const *weapon_definition,
	struct weapon_interface_state const *weapon_state,
	short const *new_state_flags,
	short const *new_overlay_flags,
	short const *new_numbers)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	{
		struct scenario *scenario;
		short map_type_flags;
		short element_index;
		struct weapon_hud_interface_definition *definition = weapon_hud_interface_definition_get(hud_index);
		struct weapon_hud_state *hud_state = get_hud_state(local_player_index);
		short state_flags[NUMBER_OF_WEAPON_HUD_STATES] = { 0 };
		short overlay_flags[NUMBER_OF_WEAPON_HUD_STATES] = { 0 };
		short numbers[NUMBER_OF_WEAPON_HUD_STATES] = { 0 };
		union weapon_hud_number numbers_real[NUMBER_OF_WEAPON_HUD_STATES] = { 0.f };

		if (TEST_FLAG(definition->flash_cutoffs.flags, _hud_use_parent_flash_parameters_bit) && new_state_flags && new_overlay_flags && new_numbers)
		{
			memcpy(state_flags, new_state_flags, sizeof(state_flags));
			memcpy(overlay_flags, new_overlay_flags, sizeof(overlay_flags));
			memcpy(numbers, new_numbers, sizeof(numbers));
		}
		else
		{
			short state_index;
			long target_object_index;

			SET_FLAG(
				state_flags[_weapon_state_total_ammo],
				_hud_draw_flashing_bit,
				weapon_state->magazines[0].rounds_remaining <= definition->flash_cutoffs.total_ammo);
			SET_FLAG(
				state_flags[_weapon_state_total_ammo],
				_hud_draw_disabled_bit,
				!weapon_state->magazines[0].rounds_remaining);
			SET_FLAG(
				state_flags[_weapon_state_total_ammo],
				_hud_draw_in_multiplayer_bit,
				local_player_count() > 1);
			SET_FLAG(
				state_flags[_weapon_state_loaded_ammo],
				_hud_draw_flashing_bit,
				weapon_state->magazines[0].rounds_loaded <= definition->flash_cutoffs.loaded_ammo && !weapon_state->magazines[0].reloading);
			SET_FLAG(
				state_flags[_weapon_state_loaded_ammo],
				_hud_draw_disabled_bit,
				FALSE);
			SET_FLAG(
				state_flags[_weapon_state_loaded_ammo],
				_hud_draw_in_multiplayer_bit,
				local_player_count() > 1);
			SET_FLAG(
				state_flags[_weapon_state_heat],
				_hud_draw_flashing_bit,
				weapon_state->heat * 100.f >= definition->flash_cutoffs.heat);
			SET_FLAG(
				state_flags[_weapon_state_heat],
				_hud_draw_disabled_bit,
				FALSE);
			SET_FLAG(
				state_flags[_weapon_state_heat],
				_hud_draw_in_multiplayer_bit,
				local_player_count() > 1);
			SET_FLAG(
				state_flags[_weapon_state_age],
				_hud_draw_flashing_bit,
				(1.f - weapon_state->age) * 100.f <= definition->flash_cutoffs.age);
			SET_FLAG(
				state_flags[_weapon_state_age],
				_hud_draw_disabled_bit,
				!(100 - PIN(fast_ftol_C(weapon_state->age * 100.f), 0, 100)));
			SET_FLAG(
				state_flags[_weapon_state_age],
				_hud_draw_in_multiplayer_bit,
				local_player_count() > 1);
			SET_FLAG(
				state_flags[_weapon_state_secondary_total_ammo],
				_hud_draw_flashing_bit,
				weapon_state->magazines[1].rounds_remaining <= definition->flash_cutoffs.total_ammo);
			SET_FLAG(
				state_flags[_weapon_state_secondary_total_ammo],
				_hud_draw_disabled_bit,
				!weapon_state->magazines[1].rounds_remaining);
			SET_FLAG(
				state_flags[_weapon_state_secondary_total_ammo],
				_hud_draw_in_multiplayer_bit,
				local_player_count() > 1);
			SET_FLAG(
				state_flags[_weapon_state_secondary_loaded_ammo],
				_hud_draw_flashing_bit,
				weapon_state->magazines[1].rounds_loaded <= definition->flash_cutoffs.loaded_ammo && !weapon_state->magazines[1].reloading);
			SET_FLAG(
				state_flags[_weapon_state_secondary_loaded_ammo],
				_hud_draw_disabled_bit,
				FALSE);
			SET_FLAG(
				state_flags[_weapon_state_secondary_loaded_ammo],
				_hud_draw_in_multiplayer_bit,
				local_player_count() > 1);

			for (state_index = 0; state_index < NUMBER_OF_WEAPON_HUD_STATES; state_index++)
			{
				if (TEST_FLAG(state_flags[state_index], _hud_draw_flashing_bit))
				{
					if (hud_state->last_weapon_flash_time[state_index] == NONE)
					{
						hud_state->last_weapon_flash_time[state_index] = game_time_get();
					}
				}
				else
				{
					hud_state->last_weapon_flash_time[state_index] = NONE;
				}
			}

			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_flashing_bit,
				weapon_state->magazines[0].rounds_remaining <= definition->flash_cutoffs.total_ammo && !weapon_state->magazines[0].reloading);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_reload_bit,
				weapon_state->magazines[0].reloading);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_empty_bit,
				!weapon_state->magazines[0].rounds_remaining);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_default_bit,
				!overlay_flags[_weapon_state_total_ammo]);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_always_bit,
				TRUE);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_flashing_bit,
				weapon_state->magazines[0].rounds_loaded <= definition->flash_cutoffs.loaded_ammo);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_reload_bit,
				weapon_state->magazines[0].reloading);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_empty_bit,
				!weapon_state->magazines[0].rounds_loaded);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_default_bit,
				!overlay_flags[_weapon_state_loaded_ammo]);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_always_bit,
				TRUE);
			SET_FLAG(
				overlay_flags[_weapon_state_heat],
				_weapon_overlay_on_flashing_bit,
				weapon_state->heat * 100.f >= definition->flash_cutoffs.heat);
			SET_FLAG(
				overlay_flags[_weapon_state_heat],
				_weapon_overlay_on_reload_bit,
				weapon_state->overheated);
			SET_FLAG(
				overlay_flags[_weapon_state_heat],
				_weapon_overlay_on_empty_bit,
				!(100 - PIN(fast_ftol_C(weapon_state->age * 100.f), 0, 100)));
			SET_FLAG(
				overlay_flags[_weapon_state_heat],
				_weapon_overlay_on_default_bit,
				!overlay_flags[_weapon_state_heat]);
			SET_FLAG(
				overlay_flags[_weapon_state_heat],
				_weapon_overlay_on_always_bit,
				TRUE);
			SET_FLAG(
				overlay_flags[_weapon_state_age],
				_weapon_overlay_on_flashing_bit,
				(1.f - weapon_state->age) * 100.f <= definition->flash_cutoffs.age);
			SET_FLAG(
				overlay_flags[_weapon_state_age],
				_weapon_overlay_on_reload_bit,
				weapon_state->overheated);
			SET_FLAG(
				overlay_flags[_weapon_state_age],
				_weapon_overlay_on_empty_bit,
				!(100 - PIN(fast_ftol_C(weapon_state->age * 100.f), 0, 100)));
			SET_FLAG(
				overlay_flags[_weapon_state_age],
				_weapon_overlay_on_default_bit,
				!overlay_flags[_weapon_state_age]);
			SET_FLAG(
				overlay_flags[_weapon_state_age],
				_weapon_overlay_on_always_bit,
				TRUE);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_flashing_bit,
				weapon_state->magazines[1].rounds_remaining <= definition->flash_cutoffs.total_ammo && !weapon_state->magazines[1].reloading);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_reload_bit,
				weapon_state->magazines[1].reloading);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_empty_bit,
				!weapon_state->magazines[1].rounds_remaining);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_default_bit,
				!overlay_flags[_weapon_state_total_ammo]);
			SET_FLAG(
				overlay_flags[_weapon_state_total_ammo],
				_weapon_overlay_on_always_bit,
				TRUE);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_flashing_bit,
				weapon_state->magazines[1].rounds_loaded <= definition->flash_cutoffs.loaded_ammo);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_reload_bit,
				weapon_state->magazines[1].reloading);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_empty_bit,
				!weapon_state->magazines[1].rounds_loaded);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_default_bit,
				!overlay_flags[_weapon_state_loaded_ammo]);
			SET_FLAG(
				overlay_flags[_weapon_state_loaded_ammo],
				_weapon_overlay_on_always_bit,
				TRUE);

			numbers[_weapon_state_total_ammo] = weapon_state->magazines[0].rounds_remaining;
			numbers[_weapon_state_loaded_ammo] = weapon_state->magazines[0].rounds_loaded;
			numbers[_weapon_state_heat] = (short)(weapon_state->heat * 255.f);
			numbers[_weapon_state_age] = (short)((1.f - weapon_state->age) * 100.f);
			numbers[_weapon_state_secondary_total_ammo] = weapon_state->magazines[1].rounds_remaining;
			numbers[_weapon_state_secondary_loaded_ammo] = weapon_state->magazines[1].rounds_loaded;

			target_object_index = player_control_get_target_object_index(local_player_index);

			if (player_control_get_autoaim_level(local_player_index) == 1.f && target_object_index != NONE)
			{
				real_point3d position;
				real_point3d target_position;

				unit_get_camera_position(
					local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index,
					&position);
				object_get_origin(target_object_index, &target_position);
				numbers_real[_weapon_state_distance_to_target].value = distance3d(&target_position, &position) * METERS_PER_UNIT;
				numbers_real[_weapon_state_elevation_to_target].value = (target_position.z - position.z) * METERS_PER_UNIT;
			}
			else
			{
				numbers_real[_weapon_state_distance_to_target].bits = NO_TARGET_NUMBER_BITS;
				numbers_real[_weapon_state_elevation_to_target].bits = NO_TARGET_NUMBER_BITS;
			}
		}

		if (definition->parent_hud.index != NONE)
		{
			render_weapon_hud(
				definition->parent_hud.index,
				local_player_index,
				weapon_definition,
				weapon_state,
				state_flags,
				overlay_flags,
				numbers);
		}

		scenario = global_scenario_get();
		map_type_flags = scenario->type != _scenario_type_main_menu;
		SET_FLAG(map_type_flags, _hud_use_on_solo_map, local_player_count() == 1);
		SET_FLAG(map_type_flags, _hud_use_on_multiplayer_map, local_player_count() > 1);

		for (element_index = 0; element_index < definition->statics.count; element_index++)
		{
			struct weapon_hud_static_element *element = TAG_BLOCK_GET_ELEMENT(&definition->statics, element_index, struct weapon_hud_static_element);

			if (!TEST_FLAG(element->header.runtime_flags, _hud_element_runtime_invalid_bit) && TEST_FLAG(map_type_flags, element->header.use_on_map_type))
			{
				hud_draw_static_element(
					local_player_index,
					&definition->absolute_placement,
					&element->static_element,
					state_flags[element->header.state_type],
					hud_state->last_weapon_flash_time[element->header.state_type]);
			}
		}

		for (element_index = 0; element_index < definition->meters.count; element_index++)
		{
			struct weapon_hud_meter_element *element = TAG_BLOCK_GET_ELEMENT(&definition->meters, element_index, struct weapon_hud_meter_element);

			if (!TEST_FLAG(element->header.runtime_flags, _hud_element_runtime_invalid_bit) && TEST_FLAG(map_type_flags, element->header.use_on_map_type))
			{
				hud_draw_meter(
					local_player_index,
					&definition->absolute_placement,
					&element->meter_element,
					(byte)numbers[element->header.state_type],
					(byte)numbers[element->header.state_type],
					state_flags[element->header.state_type],
					(real)hud_state->last_weapon_flash_time[element->header.state_type],
					0.f);
			}
		}

		for (element_index = 0; element_index < definition->numbers.count; element_index++)
		{
			struct weapon_hud_number_element *element = TAG_BLOCK_GET_ELEMENT(&definition->numbers, element_index, struct weapon_hud_number_element);

			if (!TEST_FLAG(element->header.runtime_flags, _hud_element_runtime_invalid_bit) && TEST_FLAG(map_type_flags, element->header.use_on_map_type))
			{
				short divisor = 1;

				if (TEST_FLAG(element->weapon_flags, _weapon_number_divide_by_clip_size_bit))
				{
					struct weapon_magazine_definition *magazine = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, 0, struct weapon_magazine_definition);

					divisor = magazine->rounds_loaded_maximum;
				}

				if (element->number_element.fractional_digits)
				{
					if (numbers_real[element->header.state_type].bits != NO_TARGET_NUMBER_BITS)
					{
						real scale = power(10.f, 4.f);

						hud_draw_numbers(
							local_player_index,
							&definition->absolute_placement,
							&element->number_element,
							(short)fast_ftol_C(numbers_real[element->header.state_type].value / divisor),
							(short)fast_ftol(fmod(fabs(scale * numbers_real[element->header.state_type].value), scale)),
							state_flags[element->header.state_type],
							hud_state->last_weapon_flash_time[element->header.state_type],
							0.f);
					}
				}
				else
				{
					hud_draw_numbers(
						local_player_index,
						&definition->absolute_placement,
						&element->number_element,
						numbers[element->header.state_type] / divisor,
						NONE,
						state_flags[element->header.state_type],
						hud_state->last_weapon_flash_time[element->header.state_type],
						0.f);
				}
			}
		}

		for (element_index = 0; element_index < definition->overlays.count; element_index++)
		{
			struct weapon_hud_overlays_element *element = TAG_BLOCK_GET_ELEMENT(&definition->overlays, element_index, struct weapon_hud_overlays_element);

			if (!TEST_FLAG(element->runtime_flags, _hud_element_runtime_invalid_bit) && TEST_FLAG(map_type_flags, element->use_on_map_type))
			{
				hud_draw_weapon_overlays(
					local_player_index,
					&definition->absolute_placement,
					&element->overlays,
					overlay_flags[element->state_type],
					hud_state->last_weapon_flash_time[element->state_type],
					state_flags[element->state_type],
					local_player_count() > 1);
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 776);
	}

	return;
}

static void play_weapon_hud_sounds(
	short local_player_index,
	long hud_index,
	struct weapon_interface_state const *weapon_state)
{
	if (hud_index != NONE)
	{
		unit_try_and_get(
			local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index);
	}

	return;
}

static void render_grenade_hud(
	short local_player_index,
	long unit_index)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	long weapon_index;
	struct unit_datum *unit;
	struct unit_definition *unit_definition;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	weapon_index = unit_inventory_get_weapon(unit_index, unit_get(unit_index)->unit.current_weapon_index);
	unit = unit_get(unit_index);
	unit_definition = unit_definition_get(unit->definition_index);

	if (!weapon_prevents_grenade_throwing(weapon_index) && unit->unit.current_grenade_index != NONE)
	{
		long parent_object_index = unit->object.parent_object_index;
		struct unit_datum *parent = unit_try_and_get(parent_object_index);

		if (!parent || (parent->unit.driver_object_index != unit_index && parent->unit.gunner_object_index != unit_index))
		{
			short grenade_type = unit_get_current_grenade_type(unit_index);

			if (grenade_type != NONE)
			{
				long hud_index = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, grenade_type, struct game_globals_grenade)->hud_interface.index;
				struct weapon_hud_state *hud_state = get_hud_state(local_player_index);

				if (hud_index != NONE)
				{
					struct grenade_hud_interface_definition *definition = grenade_hud_interface_definition_get(hud_index);
					short draw_flags = unit->unit.grenade_counts[unit->unit.current_grenade_index] <= definition->grenade_count_panel.flash_cutoff;

					SET_FLAG(
						draw_flags,
						_hud_draw_disabled_bit,
						!unit->unit.grenade_counts[unit->unit.current_grenade_index]);
					SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);

					if (TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
					{
						if (hud_state->last_grenade_flash_time == NONE)
						{
							hud_state->last_grenade_flash_time = game_time_get();
						}
					}
					else
					{
						hud_state->last_grenade_flash_time = NONE;
					}

					if (definition->background.interface_bitmap.index != NONE)
					{
						hud_draw_static_element(
							local_player_index,
							&definition->absolute_placement,
							&definition->background,
							draw_flags,
							hud_state->last_grenade_flash_time);
					}

					if (definition->grenade_count_panel.background.interface_bitmap.index != NONE)
					{
						hud_draw_static_element(
							local_player_index,
							&definition->absolute_placement,
							&definition->grenade_count_panel.background,
							draw_flags,
							hud_state->last_grenade_flash_time);
					}

					if (definition->grenade_count_panel.numbers.digits)
					{
						hud_draw_numbers(
							local_player_index,
							&definition->absolute_placement,
							&definition->grenade_count_panel.numbers,
							unit_get_grenade_count(unit_index, unit_get_current_grenade_type(unit_index)),
							NONE,
							draw_flags,
							hud_state->last_grenade_flash_time,
							0.f);
					}

					if (definition->overlays.bitmap.index != NONE)
					{
						short overlay_flags = unit->unit.grenade_counts[unit->unit.current_grenade_index] <= definition->grenade_count_panel.flash_cutoff;

						SET_FLAG(
							overlay_flags,
							_grenade_overlay_on_empty_bit,
							!unit->unit.grenade_counts[unit->unit.current_grenade_index]);
						SET_FLAG(overlay_flags, _grenade_overlay_on_default_bit, !overlay_flags);
						SET_FLAG(overlay_flags, _grenade_overlay_on_always_bit, TRUE);
						hud_draw_weapon_overlays(
							local_player_index,
							&definition->absolute_placement,
							&definition->overlays,
							overlay_flags,
							hud_state->last_grenade_flash_time,
							draw_flags,
							local_player_count() > 1);
					}
				}
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 930);
	}

	return;
}

__inline char *strip_path_name(
	char *path)
{
	char *name = strrchr(path, '\\');

	return name ? name + 1 : path;
}

static void crosshairs_draw(
	struct player_datum *player,
	long weapon_index,
	long hud_index,
	struct weapon_interface_state *weapon_state)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	if (TEST_FLAG(weapon_hud_globals->script_flags, _hud_crosshair_show_bit) && hud_index != NONE)
	{
		long unit_index = player->unit_index;
		struct crosshair_hud_state *crosshair = get_crosshair_state(player->local_player_index);
		struct weapon_hud_interface_definition *definition = weapon_hud_interface_definition_get(hud_index);
		short map_type_flags = 0;

		SET_FLAG(map_type_flags, _hud_use_on_any_map_type, global_scenario_get()->type != _scenario_type_main_menu);
		SET_FLAG(map_type_flags, _hud_use_on_solo_map, local_player_count() == 1);
		SET_FLAG(map_type_flags, _hud_use_on_multiplayer_map, local_player_count() > 1);

		if (unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);
			struct weapon_datum *weapon = weapon_index == NONE ? NULL : weapon_get(weapon_index);
			struct weapon_definition *weapon_definition = weapon_index == NONE ? NULL : weapon_definition_get(weapon->definition_index);
			struct weapon_hud_interface_definition *weapon_hud_hierarchy[MAXIMUM_WEAPON_HUD_DEPTH] = { definition };
			long weapon_hud_indices[MAXIMUM_WEAPON_HUD_DEPTH] = { hud_index };
			unsigned long render_flags = crosshair->render_flags;
			short depth;
			short hierarchy_index;

			for (depth = 1; depth < MAXIMUM_WEAPON_HUD_DEPTH && weapon_hud_hierarchy[depth - 1]->parent_hud.index != NONE; depth++)
			{
				weapon_hud_indices[depth] = weapon_hud_hierarchy[depth - 1]->parent_hud.index;
				weapon_hud_hierarchy[depth] = weapon_hud_interface_definition_get(weapon_hud_indices[depth]);
			}

			if (depth == MAXIMUM_WEAPON_HUD_DEPTH)
			{
				error(_error_silent, "too many levels in current weapon hud hierarchy");
			}

			for (hierarchy_index = 0; hierarchy_index < depth; hierarchy_index++)
			{
				struct weapon_hud_interface_definition *hud_definition = weapon_hud_hierarchy[hierarchy_index];
				struct hud_absolute_placement_definition placement = { _hud_center };
				boolean in_multiplayer = local_player_count() > 1;
				short crosshair_index;

				for (crosshair_index = 0; crosshair_index < hud_definition->crosshairs.count; crosshair_index++)
				{
					struct weapon_hud_crosshairs_element *element = TAG_BLOCK_GET_ELEMENT(&hud_definition->crosshairs, crosshair_index, struct weapon_hud_crosshairs_element);
					short state_type = element->crosshair_type;

					if (TEST_FLAG(render_flags, state_type) && TEST_FLAG(map_type_flags, element->use_on_map_type))
					{
						union crosshair_state *state = &crosshair->states[element->crosshair_type];
						short item_index;

						for (item_index = 0; item_index < element->crosshairs.items.count; item_index++)
						{
							struct weapon_hud_crosshair_item *item = TAG_BLOCK_GET_ELEMENT(&element->crosshairs.items, item_index, struct weapon_hud_crosshair_item);

							if (!TEST_FLAG(item->flags, _hud_crosshair_runtime_invalid_bit) &&
								(!TEST_FLAG(item->flags, _hud_crosshair_not_on_default_zoom_bit) || crosshair->states[_crosshair_state_zoom].reference_data > 0) &&
								(!TEST_FLAG(item->flags, _hud_crosshair_only_on_default_zoom_bit) || !crosshair->states[_crosshair_state_zoom].reference_data))
							{
								unsigned long color;
								real scale = local_player_count() <= 1 || TEST_FLAG(item->placement.multiplayer_scaling_flags, _hud_dont_scale_size_bit) ? 1.f : .5f;
								struct bitmap_group_sequence *sequence = !TEST_FLAG(item->flags, _hud_crosshair_not_a_sprite_bit) ? TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(verify_tag_reference(&element->crosshairs.bitmap))->sequences, item->sequence_index, struct bitmap_group_sequence) : NULL;
								short frame_index = 0;
								boolean firing;

								switch (state_type)
								{
								case _crosshair_state_aim:
									if (TEST_FLAG(item->flags, _hud_crosshair_flashes_bit))
									{
										frame_index = 0;

										if (state->reference_data > 0)
										{
											color = get_flash_color(&item->colors, 0);
										}
										else
										{
											color = item->colors.color;
										}
									}
									else
									{
										frame_index = (short)state->reference_data;
										color = item->colors.color;
									}
									break;
								case _crosshair_state_zoom:
									if (TEST_FLAG(item->flags, _hud_crosshair_one_zoom_level_bit))
									{
										if (!state->reference_data)
										{
											continue;
										}

										frame_index = 0;
									}
									else
									{
										frame_index = (short)state->reference_data - TEST_FLAG(item->flags, _hud_crosshair_not_on_default_zoom_bit);
									}

									if (TEST_FLAG(item->flags, _hud_crosshair_flashes_bit) && crosshair->states[_crosshair_state_aim].reference_data > 0)
									{
										color = get_flash_color(&item->colors, 0);
									}
									else
									{
										color = item->colors.color;
									}
									break;
								case _crosshair_state_fired_with_no_ammo:
								case _crosshair_state_threw_with_no_grenade:
								case _crosshair_state_fired_secondary_with_no_ammo:
								case _crosshair_state_flash_fired_battery_depleted:
									if (state_type == _crosshair_state_flash_fired_battery_depleted)
									{
										firing = weapon_state->age == 0.f && TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit);
									}

									if (state_type == _crosshair_state_fired_with_no_ammo)
									{
										firing = !weapon_state->magazines[0].rounds_loaded && !weapon_state->magazines[0].rounds_remaining && TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit);
									}
									else if (state_type == _crosshair_state_threw_with_no_grenade)
									{
										boolean no_grenades = TRUE;
										short grenade_index;

										for (grenade_index = 0; grenade_index < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_index++)
										{
											no_grenades = no_grenades && !unit->unit.grenade_counts[grenade_index];
										}

										firing = no_grenades && !unit->unit.grenade_throw_state && TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit);
									}

									if (!(firing || game_time_get() - state->reference_data < get_flash_duration(&item->colors)) || state->reference_data == NONE)
									{
										state->reference_data = NONE;
										continue;
									}
								case _crosshair_state_charge:
								case _crosshair_state_flash_ammo:
								case _crosshair_state_flash_heat:
								case _crosshair_state_flash_total_ammo:
								case _crosshair_state_flash_total_battery:
								case _crosshair_state_reload:
								case _crosshair_state_flash_ammo_none_for_reload:
								case _crosshair_state_flash_secondary_ammo:
								case _crosshair_state_flash_secondary_total_ammo:
								case _crosshair_state_secondary_reload:
								case _crosshair_state_flash_secondary_ammo_none_for_reload:
								case _crosshair_state_primary_trigger_ready:
								case _crosshair_state_secondary_trigger_ready:
									if (item->frame_rate > 0)
									{
										frame_index = (short)((game_time_get() - state->reference_data) / item->frame_rate / TICKS_PER_SECOND % sequence->sprites.count);
									}
									else
									{
										frame_index = 0;
									}

									if (TEST_FLAG(item->flags, _hud_crosshair_flashes_bit) && state->reference_data != NONE)
									{
										color = get_flash_color(&item->colors, state->reference_data);
									}
									else
									{
										color = item->colors.color;
									}
									break;
								default:
									match_vassert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 1186, FALSE, NULL);
									break;
								}

								match_vassert("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 1189, frame_index != NONE, csprintf(temporary, "frame index NONE when drawing crosshair %d, element %d, for weapon interface '%s'", crosshair_index, item_index, strip_path_name(tag_get_name(weapon_hud_indices[hierarchy_index]))));

								{
									struct bitmap_group *bitmap_group = bitmap_group_get(verify_tag_reference(&element->crosshairs.bitmap));
									struct bitmap_data *bitmap = TAG_BLOCK_GET_ELEMENT(&bitmap_group->bitmaps, sequence ? TAG_BLOCK_GET_ELEMENT(&sequence->sprites, frame_index, struct bitmap_group_sprite)->bitmap_index : item->sequence_index, struct bitmap_data);

									if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
									{
										if (TEST_FLAG(item->flags, _hud_crosshair_hide_outside_area_bit))
										{
											real scale_u = 1.f;
											real scale_v = 1.f;
											boolean is_interface_bitmap = bitmap_group->type == _bitmap_group_type_interface_bitmaps;
											real_rectangle2d clip;
											real offset_u;
											real offset_v;

											if (sequence)
											{
												clip = TAG_BLOCK_GET_ELEMENT(&sequence->sprites, frame_index, struct bitmap_group_sprite)->bounds;
											}
											else
											{
												clip.x0 = 0.f;
												clip.x1 = (real)(is_interface_bitmap ? bitmap->width : 1);
												clip.y0 = 0.f;
												clip.y1 = (real)(is_interface_bitmap ? bitmap->height : 1);
												scale_u = (real)(is_interface_bitmap ? 1.0 : 1.f / bitmap->width * 1.25);
												scale_v = (real)(is_interface_bitmap ? 1.0 : 1.f / bitmap->height * 1.25);
											}

											offset_u = (bitmap->width - (render.camera.viewport_bounds.x1 - render.camera.viewport_bounds.x0) * (1.f / scale)) * scale_u * -.5f;
											offset_v = (bitmap->height - (render.camera.viewport_bounds.y1 - render.camera.viewport_bounds.y0) * (1.f / scale)) * scale_v * -.5f;
											clip.x0 -= offset_u;
											clip.x1 += offset_u;
											clip.y0 -= offset_v;
											clip.y1 += offset_v;
											hud_draw_bitmap(
												bitmap,
												&placement,
												&item->placement,
												&clip,
												scale,
												0.f,
												color,
												in_multiplayer,
												is_interface_bitmap,
												TRUE);
										}
										else
										{
											hud_draw_bitmap(
												bitmap,
												&placement,
												&item->placement,
												sequence ? &TAG_BLOCK_GET_ELEMENT(&sequence->sprites, frame_index, struct bitmap_group_sprite)->bounds : NULL,
												scale,
												0.f,
												color,
												in_multiplayer,
												bitmap_group->type == _bitmap_group_type_interface_bitmaps,
												TRUE);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_weapon.c", 1250);
	}

	return;
}
