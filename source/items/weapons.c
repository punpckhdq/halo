/*
WEAPONS.C

symbols in this file:
000EA4D0 0010:
	_animation_convert_frame_to_pal (0000)
000EA4E0 0010:
	_animation_key_frame_index (0000)
000EA4F0 0020:
	_animation_update (0000)
000EA510 0020:
	_animation_choose_random_permutation (0000)
000EA530 0010:
	_weapons_initialize (0000)
000EA540 0010:
	_weapons_initialize_for_new_map (0000)
000EA550 0010:
	_weapons_dispose_from_old_map (0000)
000EA560 0010:
	_weapons_dispose (0000)
000EA570 00d0:
	_weapon_place (0000)
000EA640 0050:
	_weapon_preprocess_node_orientations (0000)
000EA690 0030:
	_weapon_get_label (0000)
000EA6C0 0020:
	_weapon_set_integrated_light_power (0000)
000EA6E0 0080:
	_weapon_estimate_time_to_target (0000)
000EA760 0090:
	_weapon_can_be_fired (0000)
000EA7F0 0030:
	_weapon_useful (0000)
000EA820 0070:
	_weapon_compute_movement_penalty (0000)
000EA890 0010:
	_weapon_melee_attack (0000)
000EA8A0 0030:
	_weapon_must_be_readied (0000)
000EA8D0 0030:
	_weapon_is_flag (0000)
000EA900 0050:
	_weapon_prevents_grenade_throwing (0000)
000EA950 01b0:
	_weapon_get_first_person_animation_time (0000)
000EAB00 0030:
	_weapon_overcharged (0000)
000EAB30 0050:
	_code_000eab30 (0000)
000EAB80 0050:
	_code_000eab80 (0000)
000EABD0 0050:
	_code_000eabd0 (0000)
000EAC20 0040:
	_code_000eac20 (0000)
000EAC60 0030:
	_code_000eac60 (0000)
000EAC90 0040:
	_code_000eac90 (0000)
000EACD0 0050:
	_weapon_get_projectile_owner_object_index (0000)
000EAD20 0090:
	_weapon_trigger_get_charged_fraction (0000)
000EADB0 00f0:
	_weapon_trigger_can_fire_again (0000)
000EAEA0 0050:
	_weapon_magazine_idle (0000)
000EAEF0 00f0:
	_code_000eaef0 (0000)
000EAFE0 0070:
	_weapon_effect_looping_new (0000)
000EB050 0040:
	_weapon_detonate (0000)
000EB090 0090:
	_weapon_trigger_change_state (0000)
000EB120 0080:
	_weapon_trigger_start_ejection_port (0000)
000EB1A0 0050:
	_weapon_state_key_frame (0000)
000EB1F0 0020:
	_weapon_magazine_state_interruptable (0000)
000EB210 0020:
	_code_000eb210 (0000)
000EB230 01b0:
	_code_000eb230 (0000)
000EB3E0 0120:
	_weapon_set_total_rounds (0000)
000EB500 0010:
	_power (0000)
000EB510 0010:
	_random (0000)
000EB520 0190:
	_weapon_new (0000)
000EB6B0 0060:
	_weapon_delete (0000)
000EB710 0390:
	_weapon_export_function_values (0000)
000EBAA0 0220:
	_weapon_handle_potential_inventory_item (0000)
000EBCC0 00a0:
	_weapon_owner_update (0000)
000EBD60 0140:
	_weapon_build_weapon_interface_state (0000)
000EBEA0 0080:
	_weapon_reloading (0000)
000EBF20 0070:
	_weapon_rotate_zoom_level (0000)
000EBF90 0160:
	_weapon_get_zoom_magnification (0000)
000EC0F0 0050:
	_weapon_get_field_of_view (0000)
000EC140 0060:
	_weapon_prevents_melee_attack (0000)
000EC1A0 0160:
	_weapon_magazine_start_reload (0000)
000EC300 00e0:
	_weapon_magazine_finish_reload (0000)
000EC3E0 00c0:
	_weapon_magazine_start_chamber (0000)
000EC4A0 0080:
	_weapon_magazine_finish_chamber (0000)
000EC520 00c0:
	_weapon_trigger_fully_charged (0000)
000EC5E0 0090:
	_weapon_trigger_idle (0000)
000EC670 0060:
	_weapon_trigger_locked (0000)
000EC6D0 0060:
	_weapon_trigger_recover (0000)
000EC730 0190:
	_code_000ec730 (0000)
000EC8C0 00a0:
	_projectile_distribute (0000)
000EC960 0030:
	_weapon_state_next (0000)
000EC990 0160:
	_weapon_set_current_amount (0000)
000ECAF0 0080:
	_weapon_ready (0000)
000ECB70 00a0:
	_weapon_put_away (0000)
000ECC10 0110:
	_weapon_aim (0000)
000ECD20 0010:
	_weapon_stop_reload (0000)
000ECD30 0050:
	_weapon_trigger_finish_tracking (0000)
000ECD80 0720:
	_trigger_create_projectiles (0000)
000ED4A0 07c0:
	_weapon_trigger_fire (0000)
000EDC60 0270:
	_weapon_trigger_begin_firing (0000)
000EDED0 00d0:
	_weapon_trigger_overload (0000)
000EDFA0 0100:
	_weapon_trigger_release_charge (0000)
000EE0A0 0080:
	_weapon_trigger_overcharged (0000)
000EE120 0af0:
	_weapon_update (0000)
00279248 000e:
	??_C@_0O@NGIMIMAN@weapon_update?$AA@ (0000)
00279258 0010:
	??_C@_0BA@HKDAKBBH@?$HOsecondary?9blur?$AA@ (0000)
00279268 000e:
	??_C@_0O@MICLHJCM@?$HOprimary?9blur?$AA@ (0000)
00279278 001f:
	??_C@_0BP@EKCOHDKN@c?3?2halo?2SOURCE?2items?2weapons?4c?$AA@ (0000)
00279298 004b:
	??_C@_0EL@OJBCMDA@trigger_index?$DO?$DN0?5?$CG?$CG?5trigger_inde@ (0000)
002792E8 004e:
	??_C@_0EO@GOIBLAPE@magazine_index?$DO?$DN0?5?$CG?$CG?5magazine_in@ (0000)
00279338 0033:
	??_C@_0DD@MMDDCEO@new_state?$DO?$DN0?5?$CG?$CG?5new_state?$DMNUMBER@ (0000)
00279370 0048:
	??_C@_0EI@IEKIMHHO@trigger_index?$DO?$DN0?5?$CG?$CG?5trigger_inde@ (0000)
002793B8 000d:
	??_C@_0N@PABHPCND@rounds_array?$AA@ (0000)
002793C8 001e:
	??_C@_0BO@MNJMFFGN@?$CBweapon_is_flag?$CIweapon_index?$CJ?$AA@ (0000)
002793E8 001f:
	??_C@_0BP@BIBKIDBE@weapon?9?$DOweapon?4primary_trigger?$AA@ (0000)
00279408 0013:
	??_C@_0BD@CJIOBLLL@magnification?$DO0?40f?$AA@ (0000)
0027941C 000e:
	??_C@_0O@NODOJABJ@magnification?$AA@ (0000)
0027942C 0004:
	__real@40470d23 (0000)
00279430 0004:
	__real@3d00adfd (0000)
00279434 0012:
	??_C@_0BC@IMIGIGHG@secondary?5trigger?$AA@ (0000)
00279448 0004:
	__real@3d2aaaab (0000)
00307140 0600:
	_data_00307140 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "weapons.h"
#include "weapon_interface_definitions.h"
#include "projectiles.h"
#include "projectile_definitions.h"
#include "equipment.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "network_messages.h"
#include "actors.h"
#include "sound_manager.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "physics_constants.h"
#include "effects.h"
#include "network_server_message_handler.h"
#include "object_types.h"
#include "first_person_weapons.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static struct weapon_trigger *weapon_trigger_get(struct weapon_datum *weapon, short trigger_index);
static struct weapon_magazine *weapon_magazine_get(struct weapon_datum *weapon, short magazine_index);

static boolean weapon_busy(long weapon_index);
static boolean weapon_magazine_state_change_ok(long weapon_index);
static long weapon_get_effect_object_index(long weapon_index);
static long weapon_get_owner_object_index(long weapon_index);
static long weapon_effect_new(long weapon_index, long effect_index, real effect_scale, real effect_error);
static void weapon_reset(long weapon_index);

static boolean weapon_state_interruptable(short old_state, short new_state);
static boolean weapon_set_state(long weapon_index, short new_state, boolean immediate);

/* ---------- globals */

static char *blurred_permutation_names[2] = {"~primary-blur", "~secondary-blur"};

static struct profile_section weapon_update_section = {"weapon_update", NONE, TRUE};

/* ---------- public code */

void weapons_initialize(
	void)
{
	return;
}

void weapons_initialize_for_new_map(
	void)
{
	return;
}

void weapons_dispose_from_old_map(
	void)
{
	return;
}

void weapons_dispose(
	void)
{
	return;
}


void weapon_place(
	long weapon_index,
	const struct scenario_weapon_datum *scenario_weapon)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

	if (weapon_definition->weapon.magazines.count > 0)
	{
		struct weapon_magazine_definition *magazine =
			TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, 0, struct weapon_magazine_definition);

		weapon->weapon.magazines[0].rounds_total =
			scenario_weapon->rounds_total > magazine->rounds_total_maximum
				? magazine->rounds_total_maximum
				: scenario_weapon->rounds_total;

		weapon->weapon.magazines[0].rounds_loaded =
			scenario_weapon->rounds_loaded > magazine->rounds_loaded_maximum
				? magazine->rounds_loaded_maximum
				: scenario_weapon->rounds_loaded;
	}

	SET_FLAG(weapon->object.flags, _object_at_rest_bit, TEST_FLAG(scenario_weapon->flags, _weapon_created_at_rest_bit));
	SET_FLAG(weapon->object.flags, _object_cannot_be_garbage_bit, TRUE);
	SET_FLAG(weapon->item.flags, _item_does_not_accelerate_bit, TEST_FLAG(scenario_weapon->flags, _weapon_does_accelerate_bit));
	
	if (!TEST_FLAG(scenario_weapon->flags, _weapon_created_at_rest_bit))
	{
		weapon->object.position.z += 0.05f; // This might be a named const?
	}

	return;
}

void weapon_preprocess_node_orientations(
	long weapon_index,
	struct real_orientation *node_orientations)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct animation_graph *animation = animation_graph_definition_get(weapon_definition->object.animation_graph.index);

	if (animation->weapon_animations.count)
	{
		TAG_BLOCK_GET_ELEMENT(&animation->weapon_animations, 0, struct animation_graph_weapon_animations);
	}

	return;
}

char const *weapon_get_label(
	long weapon_index)
{
	char const *label = "";
	if (weapon_index != NONE)
	{
		label = weapon_definition_get(weapon_get(weapon_index)->definition_index)->weapon.label;
	}
	return label;
}

void weapon_set_integrated_light_power(
	long weapon_index,
	real light_power)
{
	weapon_get(weapon_index)->weapon.integrated_light_power = light_power;
	return;
}

real weapon_estimate_time_to_target(
	long weapon_index,
	short trigger_index,
	real target_distance)
{
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon_get(weapon_index)->definition_index);
	real result = 0.0f;
	
	if (trigger_index >= 0 && trigger_index < weapon_definition->weapon.triggers.count)
	{
		struct weapon_trigger_definition* weapon_trigger_definition = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.triggers, trigger_index, struct weapon_trigger_definition);
		result = projectile_estimate_time_to_target(projectile_definition_get(weapon_trigger_definition->projectile.index), target_distance);
	}

	return result;
}

/* Used to determine if a weapon can ever be fired again. Used to determine if a weapon should be deleted in multiplayer */
boolean weapon_can_be_fired(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
  	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

	boolean result;

	// Weapons that use battery/age can't be refilled so they cannot be fired again
	if (weapon->weapon.age >= 1.0f)
	{
		result = FALSE;
	}

	// If not in multiplayer a player might pick up new ammo for a weapon. So it can technically be fired
	else if (!game_engine_running())
	{
		result = TRUE;
	}

	// Weapons that can't have ammo can still be "fired"
	else if (weapon_definition->weapon.magazines.count <= 0)
	{
		result = TRUE;
	}
	else if (TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, 0, struct weapon_magazine_definition)->rounds_loaded_maximum <= 0)
	{
		result = TRUE;
	}
	else if (weapon->weapon.magazines[0].rounds_loaded > 0)
	{
		result = TRUE;
	}
	else if (weapon->weapon.magazines[0].rounds_total > 0)
	{
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	return result;
}

boolean weapon_useful(
	long weapon_index)
{
	boolean useful;

	if (weapon_get(weapon_index)->weapon.age >= 1.0f) 
	{
		useful = FALSE;
	}
	else {
		useful = TRUE;
	}
	
	return useful;
}

real weapon_compute_movement_penalty(
	long weapon_index,
	boolean forward,
	boolean zoomed)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
	real penalty;
	long movement_penalty_mode;

	if (forward)
	{
		penalty = weapon_definition->weapon.forward_movement_penalty;
	}
	else {
		penalty = weapon_definition->weapon.sideways_movement_penalty;
	}
	movement_penalty_mode = weapon_definition->weapon.movement_penalty_mode;

	if (movement_penalty_mode == _weapon_movement_penalty_when_zoomed && !zoomed)
	{
		penalty = 0;
	}	
	else if (
		(movement_penalty_mode == _weapon_movement_penalty_when_zoomed_or_reloading &&
		(weapon->weapon.magazines[0].state == _magazine_reloading || weapon->weapon.magazines[1].state == _magazine_reloading)
	) && !zoomed)
	{
		penalty = 0;
	}
	return penalty;
}

void weapon_melee_attack(
	long weapon_index)
{
	return;
}

boolean weapon_must_be_readied(
	long weapon_index)
{
	struct weapon_datum const *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_defintion = weapon_definition_get(weapon->definition_index);
	return TEST_FLAG(weapon_defintion->weapon.flags, _weapon_must_be_readied_bit);
}

boolean weapon_is_flag(
	long weapon_index)
{
	struct weapon_datum const *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_defintion = weapon_definition_get(weapon->definition_index);
	return TEST_FLAG(weapon_defintion->weapon.flags, _weapon_must_be_readied_bit);
}

boolean weapon_prevents_grenade_throwing(
	long weapon_index)
{
	boolean does_it = TRUE;

	if (weapon_index != NONE)
	{
		struct weapon_datum const *weapon = weapon_get(weapon_index);
		struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

		does_it = TEST_FLAG(weapon_definition->weapon.flags, _weapon_multiplayer_flag);
		if (weapon->weapon.state >= _weapon_state_primary_reload || weapon->weapon.state <= _weapon_state_put_away)
		{
			does_it = TRUE;
		}
	}
	
	return does_it;
}

void weapon_ready(
	long weapon_index)
{
	struct weapon_datum* weapon = weapon_get(weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

	weapon_reset(weapon_index);
	weapon_set_state(weapon_index, _weapon_state_ready, TRUE);
	first_person_weapon_message_from_weapon(weapon_index, _first_person_weapon_message_ready);
	weapon_effect_new(weapon_index, weapon_definition->weapon.ready_effect.index, 0.f, 0.f);
	weapon->weapon.state_timer = weapon_get_first_person_animation_time(weapon_index, 0, _first_person_weapon_animation_ready, NONE);

	return;
}


boolean weapon_put_away(
	long weapon_index,
	boolean immediate)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
	boolean put_away = FALSE;

	if ((immediate || !weapon_busy(weapon_index)) && weapon_set_state(weapon_index, _weapon_state_put_away, immediate))
	{
		weapon->weapon.control_flags = 0;
		weapon_reset(weapon_index);

		if (weapon->weapon.overheated_effect_index != NONE)
		{
			effect_delete(weapon->weapon.overheated_effect_index);
			weapon->weapon.overheated_effect_index = NONE;
		}
		
		first_person_weapon_message_from_weapon(weapon_index, 11);
		put_away = TRUE;
	}

	return put_away;
}

/* ---------- private code */

static struct weapon_trigger *weapon_trigger_get(
	struct weapon_datum *weapon,
	short trigger_index)
{
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	match_assert("c:\\halo\\SOURCE\\items\\weapons.c", 1639, trigger_index>=0 && trigger_index<weapon_definition->weapon.triggers.count);

	return &weapon->weapon.triggers[trigger_index];
}

static struct weapon_magazine *weapon_magazine_get(
	struct weapon_datum *weapon,
	short magazine_index)
{
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	match_assert("c:\\halo\\SOURCE\\items\\weapons.c", 1650, magazine_index>=0 && magazine_index<weapon_definition->weapon.magazines.count);

	return &weapon->weapon.magazines[magazine_index];
}

static boolean weapon_busy(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	return
		weapon->weapon.triggers[0].state != _trigger_idle ||
		weapon->weapon.triggers[1].state != _trigger_idle ||
		weapon->weapon.magazines[0].state != _magazine_idle ||
		weapon->weapon.magazines[1].state != _magazine_idle ||
		weapon->weapon.state != _weapon_state_idle;
}

static boolean weapon_magazine_state_change_ok(long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	return 
		weapon->weapon.triggers[0].state==_trigger_idle &&
		weapon->weapon.triggers[1].state==_trigger_idle &&
		weapon->weapon.state == _weapon_state_idle;
}

static long weapon_get_effect_object_index(long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	long result = weapon_index;

	if (TEST_FLAG(weapon->object.flags, _object_invisible_bit) && weapon->object.parent_object_index!=NONE)
	{
		result = weapon->object.parent_object_index;
	}

	return result;
}

static long weapon_get_owner_object_index(long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	long result = NONE;

	if (weapon->object.parent_object_index!=NONE && unit_try_and_get(weapon->object.parent_object_index))
	{
		result = weapon->object.parent_object_index;
	}

	return result;
}

static long weapon_effect_new(
	long weapon_index,
	long effect_index,
	real effect_scale,
	real effect_error)
{
	long result = NONE;

	if (effect_index!=NONE)
	{
		long effect_object_index = weapon_get_effect_object_index(weapon_index);
		long object_index = weapon_get_owner_object_index(weapon_index);
		long group_tag = tag_get_group_tag(effect_index);

		if (group_tag!=EFFECT_DEFINITION_TAG)
		{
			match_vassert("c:\\halo\\SOURCE\\items\\weapons.c", 2514, group_tag==SOUND_DEFINITION_TAG, NULL);
			
			if (group_tag==SOUND_DEFINITION_TAG)
			{
				object_impulse_sound_new(object_index, effect_index, NONE, global_origin3d, global_forward3d, effect_scale);
				result = NONE;
			}
		}
		else
		{
			result = effect_new_from_object(effect_index, object_index, effect_object_index, NONE, effect_scale, effect_error, NULL, NULL);
		}
	}

	return result;
}

// TODO: finish
static void weapon_reset(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
	short trigger_index;
	short magazine_index;

	for (trigger_index = 0; trigger_index<weapon_definition->weapon.triggers.count; ++trigger_index)
	{
		struct weapon_trigger* trigger = weapon_trigger_get(weapon, trigger_index);
		struct weapon_trigger_definition *trigger_definition = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.triggers, trigger_index, struct weapon_trigger_definition);

		trigger->state = _trigger_uninitialized;
		trigger->state_timer = 0;
	}

	for (magazine_index = 0; magazine_index<weapon_definition->weapon.magazines.count; ++magazine_index)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
		struct weapon_magazine_definition *magazine_definition = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, magazine_index, struct weapon_magazine_definition);

		if (magazine->state == _magazine_reloading)
		{
			// Need an enum value for 'mode' here
			if (2*magazine->state_timer<weapon_get_first_person_animation_time(weapon_index, 0, _first_person_weapon_message_shotgun_enter_reload, NONE))
			{
				weapon_magazine_finish_reload(weapon_index, magazine_index);
			}
		}

		magazine->state = _magazine_idle;
		magazine->state_timer = 0;
	}

	return;
}

static boolean weapon_state_interruptable(short old_state, short new_state)
{
	boolean interruptable = FALSE;

	if (old_state==_weapon_state_idle)
	{
		interruptable = TRUE;
	}
	else if (old_state>_weapon_state_idle && old_state<=_weapon_state_secondary_recoil && new_state >= old_state)
	{
		interruptable = TRUE;
	}

	return interruptable;
}

// TODO: finish, there's still discrepancies
static boolean weapon_set_state(
	long weapon_index,
	short new_state,
	boolean immediate)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
	boolean busy = FALSE;

	if (immediate || weapon_state_interruptable(weapon->weapon.state, new_state))
	{
		long animation_graph_index = weapon_definition->object.animation_graph.index;
		long owner_object_index;
		long new_animation_state_index;
		if (animation_graph_index != NONE)
		{
			struct animation_graph* animation_graph = animation_graph_definition_get(animation_graph_index);
			struct animation_graph_weapon_animations const *animations = TAG_BLOCK_GET_ELEMENT(&animation_graph->weapon_animations, 0, struct animation_graph_weapon_animations);
			if (animations)
			{
				short considered_animation_index = NONE;
				switch (new_state)
				{
					case _weapon_state_idle:
						new_animation_state_index = _weapon_animation_idle;
						break;
					case _weapon_state_primary_recoil:
						new_animation_state_index = _weapon_animation_primary_recoil;
						break;
					case _weapon_state_secondary_recoil:
						new_animation_state_index = _weapon_animation_secondary_recoil;
						break;
					case _weapon_state_primary_chamber:
						new_animation_state_index = _weapon_animation_primary_chamber;
						break;
					case _weapon_state_secondary_chamber:
						new_animation_state_index = _weapon_animation_secondary_chamber;
						break;
					case _weapon_state_primary_reload:
					case _weapon_state_secondary_reload:
						new_animation_state_index = _weapon_animation_primary_reload;
						break;
					case _weapon_state_primary_charged:
					case _weapon_state_secondary_charged:
						new_animation_state_index = _weapon_animation_secondary_charged;
						break;
					case _weapon_state_ready:
						new_animation_state_index = _weapon_animation_ready;
						break;
					case _weapon_state_put_away:
						new_animation_state_index = _weapon_animation_put_away;
						break;
					default:
						break;
				}

				if (new_animation_state_index != NONE) {
					if ( new_animation_state_index < 0 || new_animation_state_index >= animations->animations.count )
					{
						considered_animation_index = NONE;
					}
					else
					{
						considered_animation_index = ((short *)animations->animations.address) + new_animation_state_index;
					}
					if (considered_animation_index != NONE || !new_state)
					{
						weapon->object.animation.state.index = animation_choose_random_permutation_internal(
							TRUE, weapon_definition->object.animation_graph.index, new_animation_state_index
						);
						weapon->weapon.state = new_state;
						weapon->object.animation.state.frame_index = 0;
					}
				}
			}
			
		}

		owner_object_index = weapon_get_owner_object_index(weapon_index);
		if (unit_try_and_get(owner_object_index))
		{
			unit_handle_weapon_state_change(owner_object_index, new_state);
		}

		busy = TRUE;
	}

	return busy;
}
