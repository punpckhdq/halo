/*
WEAPONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "weapons.h"
#include "weapon_definitions.h"
#include "projectiles.h"
#include "projectile_definitions.h"
#include "equipment.h"
#include "items.h"
#include "actors.h"
#include "ai.h"
#include "cheats.h"
#include "damage.h"
#include "effect_definitions.h"
#include "effects.h"
#include "first_person_weapons.h"
#include "game.h"
#include "game_engine.h"
#include "game_sound.h"
#include "models.h"
#include "model_animation_definitions.h"
#include "object_types.h"
#include "players.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "sound_definitions.h"
#include "unit_definitions.h"
#include "units.h"

/* ---------- macros */

#define SECONDS_PER_TICK (1.f/TICKS_PER_SECOND) /* fake name */

/* ---------- prototypes */

static struct weapon_trigger *weapon_trigger_get(struct weapon_datum *weapon, short trigger_index);
static struct weapon_magazine *weapon_magazine_get(struct weapon_datum *weapon, short magazine_index);
static boolean weapon_busy(long weapon_index);
static boolean weapon_magazine_state_change_ok(long weapon_index);
static long weapon_get_effect_object_index(long weapon_index);
static long weapon_get_owner_object_index(long weapon_index);
static long weapon_get_projectile_owner_object_index(long weapon_index);
static real weapon_trigger_get_charged_fraction(long weapon_index, short trigger_index);
static boolean weapon_trigger_can_fire_again(long weapon_index, short trigger_index);
static void weapon_trigger_begin_firing(long weapon_index, short trigger_index, boolean force_firing);
static void weapon_magazine_start_reload(long weapon_index, short magazine_index, boolean first_round);
static void weapon_magazine_finish_reload(long weapon_index, short magazine_index);
static void weapon_magazine_start_chamber(long weapon_index, short magazine_index);
static void weapon_magazine_finish_chamber(long weapon_index, short magazine_index);
static void weapon_magazine_idle(long weapon_index, short magazine_index);
static void weapon_trigger_fully_charged(long weapon_index, short trigger_index);
static void weapon_trigger_overload(long weapon_index, long trigger_index);
static void weapon_trigger_release_charge(long weapon_index, short trigger_index);
static void weapon_trigger_overcharged(long weapon_index, short trigger_index);
static void weapon_trigger_idle(long weapon_index, short trigger_index);
static void weapon_trigger_locked(long weapon_index, short trigger_index);
static void weapon_trigger_finish_tracking(long weapon_index, short trigger_index);
static void weapon_trigger_fire(long weapon_index, short trigger_index);
static long weapon_effect_new(long weapon_index, long effect_index, real effect_scale, real effect_error);
static long weapon_effect_looping_new(long weapon_index, long effect_index);
static void weapon_trigger_recover(long weapon_index, short trigger_index);
static void weapon_detonate(long weapon_index);
static void weapon_trigger_change_state(long weapon_index, short trigger_index, short new_state, short new_state_timer);
static void weapon_reset(long weapon_index);
static void trigger_create_projectiles(long weapon_index, short trigger_index);
static void projectile_distribute(real_vector3d *forward, real_vector3d *up, short distribution_function, real distribution_angle, short index, short count);
static void weapon_trigger_start_ejection_port(long weapon_index, short trigger_index, boolean chamber);
static void weapon_state_key_frame(long weapon_index);
static void weapon_state_next(long weapon_index);
static boolean weapon_magazine_state_interruptable(short old_state, short new_state);
static boolean weapon_state_interruptable(short old_state, short new_state);
static boolean weapon_set_state(long weapon_index, short new_state, boolean immediate);

/* ---------- globals */

static char *blurred_permutation_names[MAXIMUM_NUMBER_OF_TRIGGERS_PER_WEAPON] =
{
	"~primary-blur",
	"~secondary-blur"
};

static struct profile_section weapon_update_section =
{
	"weapon_update",
	NONE,
	TRUE
};

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

long weapon_place(
	long weapon_index,
	struct scenario_weapon_datum const *scenario_weapon)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	if (weapon_definition->weapon.magazines.count > 0)
	{
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			0,
			struct weapon_magazine_definition);

		weapon->weapon.magazines[_weapon_magazine_primary].rounds_total = MIN(scenario_weapon->rounds_total, magazine_definition->rounds_total_maximum);
		weapon->weapon.magazines[_weapon_magazine_primary].rounds_loaded = MIN(scenario_weapon->rounds_loaded, magazine_definition->rounds_loaded_maximum);
	}

	SET_FLAG(weapon->object.flags, _object_at_rest_bit, TEST_FLAG(scenario_weapon->flags, _weapon_created_at_rest_bit));
	SET_FLAG(weapon->object.flags, _object_cannot_be_garbage_bit, TRUE);
	SET_FLAG(weapon->item.flags, _item_does_not_accelerate_bit, !TEST_FLAG(scenario_weapon->flags, _weapon_does_accelerate_bit));

	if (!TEST_FLAG(scenario_weapon->flags, _weapon_created_at_rest_bit))
	{
		weapon->object.position.z += 0.05f;
	}

	return weapon_index;
}

boolean weapon_new(
	long weapon_index)
{
	short magazine_index;
	short trigger_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	weapon->weapon.state = _weapon_state_idle;
	weapon->weapon.overheated_effect_index = NONE;

	for (magazine_index = 0; magazine_index < weapon_definition->weapon.magazines.count; magazine_index++)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			magazine_index,
			struct weapon_magazine_definition);

		magazine->rounds_loaded = MIN(magazine_definition->rounds_total_initial, magazine_definition->rounds_loaded_maximum);
		magazine->rounds_total = magazine_definition->rounds_total_initial - magazine->rounds_loaded;
	}

	for (trigger_index = 0; trigger_index < weapon_definition->weapon.triggers.count; trigger_index++)
	{
		struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
		struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.triggers,
			trigger_index,
			struct weapon_trigger_definition);

		trigger->charging_effect_index = NONE;
		trigger->idle_ticks = CHAR_MAX;
	}

	return TRUE;
}

void weapon_delete(
	long weapon_index)
{
#ifdef DEBUG
	if (game_engine_running())
	{
		match_assert("c:\\halo\\SOURCE\\items\\weapons.c", 234, !weapon_is_flag(weapon_index));
	}
#endif

	return;
}

boolean weapon_update(
	long weapon_index)
{
	boolean trigger_down[MAXIMUM_NUMBER_OF_TRIGGERS_PER_WEAPON];
	short magazine_index;
	short trigger_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	profile_enter(weapon_update_section);

	if (weapon->weapon.tracked_object_index != NONE && !object_try_and_get(weapon->weapon.tracked_object_index))
	{
		weapon->weapon.tracked_object_index = NONE;
	}

	if (weapon_definition->object.animation_graph.index != NONE && weapon->object.animation.state.index != NONE)
	{
		switch (animation_update(weapon_definition->object.animation_graph.index, &weapon->object.animation.state, NULL))
		{
		case _animation_key_frame:
			weapon_state_key_frame(weapon_index);
			break;

		case _animation_will_restart_on_next_frame:
			weapon_state_next(weapon_index);
			break;
		}
	}

	if (TEST_FLAG(weapon_definition->weapon.flags, _weapon_detonates_when_dropped_bit) && weapon->object.parent_object_index == NONE)
	{
		item_detonate(weapon_index);
	}

	if (weapon->weapon.integrated_light_power > 0.f)
	{
		boolean light_decays = TRUE;

		if (weapon->object.parent_object_index != NONE)
		{
			struct unit_datum *unit = unit_try_and_get(weapon->object.parent_object_index);

			if (unit && unit->definition_index != NONE)
			{
				struct unit_definition const *unit_definition = unit_definition_get(unit->definition_index);

				if (TEST_FLAG(unit_definition->unit.flags, _unit_integrated_light_controls_weapon_directly_bit))
				{
					light_decays = FALSE;
				}
			}
		}

		if (light_decays)
		{
			if ((weapon->weapon.integrated_light_power -= 1.f / 24.f) < 0.f)
			{
				weapon->weapon.integrated_light_power = 0.f;
			}
		}
	}

	if (weapon->weapon.heat > 0.f)
	{
		if (weapon->weapon.heat >= weapon_definition->weapon.heat_overheated_threshold && !TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit))
		{
			SET_FLAG(weapon->weapon.flags, _weapon_overheated_bit, TRUE);

			if (weapon_definition->weapon.weapon_type == _weapon_type_plasma_pistol && TEST_FLAG(weapon->weapon.flags, _weapon_overheat_recoil_bit))
			{
				SET_FLAG(weapon->weapon.flags, _weapon_overheat_recoil_bit, FALSE);
				first_person_weapon_message_from_weapon(
					weapon_index,
					_first_person_weapon_message_overheating_super_recoil);
			}
			else
			{
				first_person_weapon_message_from_weapon(weapon_index, _first_person_weapon_message_overheating);
			}

			weapon->weapon.overheated_effect_index = weapon_effect_looping_new(
				weapon_index,
				weapon_definition->weapon.overheated_effect.index);
		}

		if (weapon->weapon.overcharged == 0.f)
		{
			real heat_loss = weapon_definition->weapon.heat_loss_per_second * SECONDS_PER_TICK;

			if (weapon_definition->weapon.age_heat_recovery_penalty > 0.f)
			{
				heat_loss *= 1.f - weapon->weapon.age * weapon_definition->weapon.age_heat_recovery_penalty;
			}

			weapon->weapon.heat -= heat_loss;

			if (weapon->weapon.heat < 0.f)
			{
				weapon->weapon.heat = 0.f;
			}

			if (TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit) && !TEST_FLAG(weapon->weapon.flags, _weapon_overheated_exit_bit))
			{
				short exit_ticks = 1;
				real heat_loss_per_tick = heat_loss;

				if ((weapon->weapon.heat - weapon_definition->weapon.heat_recovery_threshold) / heat_loss_per_tick <= exit_ticks)
				{
					SET_FLAG(weapon->weapon.flags, _weapon_overheated_exit_bit, TRUE);
				}
			}
		}

		if (TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit) && weapon->weapon.heat < weapon_definition->weapon.heat_recovery_threshold)
		{
			SET_FLAG(weapon->weapon.flags, _weapon_overheated_bit, FALSE);
			SET_FLAG(weapon->weapon.flags, _weapon_overheated_exit_bit, FALSE);

			if (weapon->weapon.overheated_effect_index != NONE)
			{
				effect_stop(weapon->weapon.overheated_effect_index, TRUE);
			}
		}
	}

	weapon->weapon.overcharged = 0.f;

	if (weapon->weapon.state_timer > 0)
	{
		weapon->weapon.state_timer--;
	}

	if (TEST_FLAG(weapon->weapon.control_flags, _weapon_control_user_busy_bit) || weapon->weapon.state_timer > 0)
	{
		trigger_down[_weapon_trigger_primary] = FALSE;
		trigger_down[_weapon_trigger_secondary] = FALSE;
	}
	else
	{
		trigger_down[_weapon_trigger_primary] = TEST_FLAG(weapon->weapon.control_flags, _weapon_control_primary_trigger_bit);
		trigger_down[_weapon_trigger_secondary] = TEST_FLAG(weapon_definition->weapon.flags, _weapon_secondary_trigger_overrides_grenades_bit) && TEST_FLAG(weapon->weapon.control_flags, _weapon_control_secondary_trigger_bit);
	}

	switch (weapon_definition->weapon.secondary_trigger_mode)
	{
	case _weapon_secondary_trigger_slaved_to_primary:
		if (trigger_down[_weapon_trigger_secondary] && weapon_definition->weapon.triggers.count > 0 && weapon->weapon.triggers[_weapon_trigger_primary].rate_of_fire != 1.f)
		{
			trigger_down[_weapon_trigger_secondary] = FALSE;
		}
		break;

	case _weapon_secondary_trigger_inhibits_primary:
		if (trigger_down[_weapon_trigger_secondary])
		{
			trigger_down[_weapon_trigger_primary] = FALSE;
		}
		break;
	}

	if (TEST_FLAG(weapon->weapon.control_flags, _weapon_control_reload_bit) && weapon_definition->weapon.magazines.count > 0)
	{
		SET_FLAG(weapon->weapon.flags, _weapon_needs_to_reload_bit, TRUE);
	}

	if (TEST_FLAG(weapon->weapon.flags, _weapon_needs_to_reload_bit))
	{
		weapon_magazine_start_reload(weapon_index, _weapon_magazine_primary, TRUE);
	}

	for (magazine_index = 0; magazine_index < weapon_definition->weapon.magazines.count; magazine_index++)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			magazine_index,
			struct weapon_magazine_definition);

		if (magazine_definition->rounds_recharged_per_second > 0 && magazine->rounds_loaded < magazine_definition->rounds_loaded_maximum)
		{
			short rounds_recharged = magazine_definition->rounds_recharged_per_second / TICKS_PER_SECOND;
			short fractional_rounds_recharged = magazine_definition->rounds_recharged_per_second % TICKS_PER_SECOND;

			magazine->rounds_loaded += rounds_recharged;

			if ((magazine->rounds_fractional_recharged += fractional_rounds_recharged) >= TICKS_PER_SECOND)
			{
				magazine->rounds_loaded++;
				magazine->rounds_fractional_recharged -= TICKS_PER_SECOND;
			}

			if (magazine->rounds_loaded > magazine_definition->rounds_loaded_maximum)
			{
				magazine->rounds_loaded = magazine_definition->rounds_loaded_maximum;
			}
		}

		if (magazine->state_timer)
		{
			magazine->state_timer--;
		}

		switch (magazine->state)
		{
		case _magazine_reloading:
			if (magazine->state_timer - 1 <= 0)
			{
				weapon_magazine_finish_reload(weapon_index, magazine_index);
			}
			break;

		case _magazine_unchambered:
			weapon_magazine_start_chamber(weapon_index, magazine_index);
			break;

		case _magazine_chambering:
			if (!magazine->state_timer)
			{
				weapon_magazine_finish_chamber(weapon_index, magazine_index);
			}
			break;
		}
	}

	for (trigger_index = 0; trigger_index < weapon_definition->weapon.triggers.count; trigger_index++)
	{
		struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
		struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.triggers,
			trigger_index,
			struct weapon_trigger_definition);

		if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_analog_rate_of_fire_bit) && TEST_FLAG(weapon->item.flags, _item_belongs_to_player_bit))
		{
			trigger_down[trigger_index] = weapon->weapon.primary_trigger > 0.05f;
		}

		if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_sticks_when_dropped_bit) && weapon->object.parent_object_index == NONE)
		{
			trigger_down[trigger_index] = TRUE;
		}

		if (trigger->state_timer)
		{
			trigger->state_timer--;
		}

		if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_toggles_bit))
		{
			if (!TEST_FLAG(trigger->flags, _weapon_trigger_was_down_bit) && trigger_down[trigger_index])
			{
				trigger->flags ^= FLAG(_weapon_trigger_toggled_bit);
			}

			SET_FLAG(trigger->flags, _weapon_trigger_was_down_bit, trigger_down[trigger_index]);
			trigger_down[trigger_index] = TEST_FLAG(trigger->flags, _weapon_trigger_toggled_bit);
		}

		if (!trigger_down[trigger_index])
		{
			trigger->flags |= FLAG(_weapon_trigger_released_since_last_shot_bit);
		}

		if (trigger->ejection_port_position > 0.f)
		{
			if ((trigger->ejection_port_position -= trigger_definition->runtime_ejection_port_recovery_time) <= 0.f)
			{
				trigger->ejection_port_position = 0.f;
			}
		}

		if (trigger->illumination > 0.f)
		{
			if ((trigger->illumination -= trigger_definition->runtime_illumination_recovery_time) <= 0.f)
			{
				trigger->illumination = 0.f;
			}
		}

		switch (trigger->state)
		{
		case _trigger_idle:
			if (!TEST_FLAG(weapon->weapon.control_flags, _weapon_control_user_busy_bit) &&
				weapon->object.parent_object_index != NONE &&
				trigger_definition->magazine_index != NONE)
			{
				struct weapon_magazine *magazine = weapon_magazine_get(weapon, trigger_definition->magazine_index);

				if (magazine->rounds_loaded < trigger_definition->rounds_per_shot && !TEST_FLAG(trigger_definition->flags, _weapon_trigger_can_fire_with_partial_ammunition_bit) ||
					magazine->rounds_loaded < trigger_definition->minimum_rounds_loaded_per_shot ||
					!magazine->rounds_loaded)
				{
					weapon_magazine_start_reload(weapon_index, trigger_definition->magazine_index, TRUE);
				}
			}

			if (trigger_down[trigger_index] && weapon_trigger_can_fire_again(weapon_index, trigger_index))
			{
				weapon_trigger_begin_firing(weapon_index, trigger_index, FALSE);
			}
			else if (trigger->idle_ticks < CHAR_MAX)
			{
				trigger->idle_ticks++;
			}
			break;

		case _trigger_spewing:
			if (trigger->state_timer)
			{
				weapon_trigger_begin_firing(weapon_index, trigger_index, TRUE);
			}
			else
			{
				weapon_trigger_recover(weapon_index, trigger_index);
			}
			break;

		case _trigger_overloading:
			if (!trigger_down[trigger_index])
			{
				weapon_trigger_begin_firing(weapon_index, trigger_index, TRUE);
			}
			else if (!trigger->state_timer && weapon->weapon.alternate_shots_loaded < weapon_definition->weapon.maximum_alternate_shots_loaded)
			{
				weapon_trigger_overload(weapon_index, trigger_index);
			}
			break;

		case _trigger_charging:
			if (trigger->state_timer)
			{
				if (!trigger_down[trigger_index])
				{
					if (trigger_index == _weapon_trigger_primary &&
						weapon_definition->weapon.triggers.count > 1 &&
						!TEST_FLAG(trigger->flags, _weapon_trigger_fired_before_charging_bit))
					{
						weapon_trigger_begin_firing(weapon_index, trigger_index, TRUE);
					}
					else
					{
						weapon_trigger_idle(weapon_index, trigger_index);
					}

					if (trigger->charging_effect_index != NONE)
					{
						effect_stop(trigger->charging_effect_index, TRUE);
						trigger->charging_effect_index = NONE;
					}
				}
			}
			else
			{
				weapon_trigger_fully_charged(weapon_index, trigger_index);
			}
			break;

		case _trigger_charged:
			if (trigger_down[trigger_index])
			{
				weapon->weapon.overcharged = 1.f - (trigger->state_timer * SECONDS_PER_TICK) / trigger_definition->charged_time;

				if (trigger->state_timer)
				{
					struct weapon_magazine *magazine = weapon_magazine_get(weapon, trigger_definition->magazine_index);

					if (magazine->rounds_loaded < trigger_definition->rounds_per_shot && !TEST_FLAG(trigger_definition->flags, _weapon_trigger_can_fire_with_partial_ammunition_bit))
					{
						weapon_trigger_release_charge(weapon_index, trigger_index);
					}
				}
				else
				{
					weapon_trigger_overcharged(weapon_index, trigger_index);
				}
			}
			else
			{
				weapon_trigger_release_charge(weapon_index, trigger_index);
			}
			break;

		case _trigger_recovering:
			if (!trigger->state_timer)
			{
				if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_latched_bit) &&
					TEST_FLAG(weapon->item.flags, _item_belongs_to_player_bit) &&
					!TEST_FLAG(trigger->flags, _weapon_trigger_released_since_last_shot_bit))
				{
					weapon_trigger_locked(weapon_index, trigger_index);
				}
				else
				{
					weapon_trigger_idle(weapon_index, trigger_index);
				}
			}
			break;

		case _trigger_tracking:
			if (!trigger_down[trigger_index] || weapon->weapon.tracked_object_index == NONE)
			{
				weapon_trigger_finish_tracking(weapon_index, trigger_index);
			}
			break;

		case _trigger_locked:
			if (!trigger_down[trigger_index])
			{
				weapon_trigger_idle(weapon_index, trigger_index);
			}
			break;

		case _trigger_uninitialized:
			if (!trigger->state_timer)
			{
				weapon_trigger_idle(weapon_index, trigger_index);
			}
			break;

		default:
			match_halt("c:\\halo\\SOURCE\\items\\weapons.c", 778);
			break;
		}

		if (trigger_down[trigger_index])
		{
			if ((trigger->rate_of_fire += trigger_definition->runtime_rate_of_fire_acceleration_time) > 1.f)
			{
				trigger->rate_of_fire = 1.f;
			}

			if (trigger_definition->blurred_rate_of_fire != 0.f &&
				!TEST_FLAG(trigger->flags, _weapon_trigger_blurred_bit) &&
				trigger->rate_of_fire > trigger_definition->blurred_rate_of_fire)
			{
				object_permute_region(
					weapon_get_effect_object_index(weapon_index),
					blurred_permutation_names[trigger_index],
					NONE,
					TRUE);
				trigger->flags |= FLAG(_weapon_trigger_blurred_bit);
			}
		}
		else
		{
			if ((trigger->rate_of_fire -= trigger_definition->runtime_rate_of_fire_deceleration_time) < 0.f)
			{
				trigger->rate_of_fire = 0.f;
			}

			if (TEST_FLAG(trigger->flags, _weapon_trigger_blurred_bit) && trigger->rate_of_fire < trigger_definition->blurred_rate_of_fire)
			{
				object_permute_region(
					weapon_get_effect_object_index(weapon_index),
					blurred_permutation_names[trigger_index],
					NONE,
					FALSE);
				trigger->flags &= ~FLAG(_weapon_trigger_blurred_bit);
			}
		}

		if (trigger->state == _trigger_spewing || trigger->state == _trigger_recovering || trigger_down[trigger_index])
		{
			if ((trigger->error += trigger_definition->runtime_error_acceleration_time) > 1.f)
			{
				trigger->error = 1.f;
			}
		}
		else
		{
			if ((trigger->error -= trigger_definition->runtime_error_deceleration_time) < 0.f)
			{
				trigger->error = 0.f;
			}
		}
	}

	profile_exit(weapon_update_section);

	return TRUE;
}

void weapon_export_function_values(
	long weapon_index)
{
	short function_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct object_datum *object = (struct object_datum *)weapon;

	while (TEST_FLAG(object->object.flags, _object_invisible_bit) && object->object.parent_object_index != NONE)
	{
		object = object_get(object->object.parent_object_index);
	}

	for (function_index = 0; function_index < NUMBER_OF_INCOMING_OBJECT_FUNCTIONS; function_index++)
	{
		short function_mode = weapon_definition->weapon.function_modes[function_index];

		if (function_mode != _weapon_function_none)
		{
			real function_value = 0.f;

			switch (function_mode)
			{
			case _weapon_function_ready:
				function_value = 1.f;
				break;

			case _weapon_function_heat:
				function_value = weapon->weapon.heat;
				break;

			case _weapon_function_overheated:
				if (TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit) && weapon_definition->weapon.heat_recovery_threshold != 1.f)
				{
					function_value = (weapon->weapon.heat - weapon_definition->weapon.heat_recovery_threshold) / (1.f - weapon_definition->weapon.heat_recovery_threshold);
				}
				break;

			case _weapon_function_illumination:
			{
				short trigger_index;
				real heat_illumination;

				for (trigger_index = 0; trigger_index < weapon_definition->weapon.triggers.count; trigger_index++)
				{
					struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
						&weapon_definition->weapon.triggers,
						trigger_index,
						struct weapon_trigger_definition);
					struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);

					if (trigger_definition->charging_time > 0.f)
					{
						real charging_illumination = weapon_trigger_get_charged_fraction(weapon_index, trigger_index) * trigger_definition->charged_illumination;

						function_value = MAX(function_value, charging_illumination);
					}

					if (trigger->state == _trigger_charged)
					{
						real charged_illumination = (1.f - trigger_definition->charged_illumination) * weapon->weapon.overcharged + trigger_definition->charged_illumination;

						function_value = MAX(function_value, charged_illumination);
					}

					function_value = MAX(function_value, trigger->illumination);
					trigger->illumination = function_value;
				}

				heat_illumination = weapon->weapon.heat * weapon_definition->weapon.heat_illumination;
				function_value = MAX(function_value, heat_illumination);
				break;
			}
			case _weapon_function_primary_ammunition:
			case _weapon_function_secondary_ammunition:
			{
				short magazine_index = function_mode - _weapon_function_primary_ammunition;

				if (magazine_index < weapon_definition->weapon.magazines.count)
				{
					struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
						&weapon_definition->weapon.magazines,
						magazine_index,
						struct weapon_magazine_definition);

					if (magazine_definition->rounds_loaded_maximum)
					{
						function_value = (real)weapon->weapon.magazines[magazine_index].rounds_loaded / magazine_definition->rounds_loaded_maximum;
					}
				}
				break;
			}
			case _weapon_function_primary_ejection_port:
			case _weapon_function_secondary_ejection_port:
			{
				short trigger_index = function_mode - _weapon_function_primary_ejection_port;

				if (trigger_index < weapon_definition->weapon.triggers.count)
				{
					function_value = weapon->weapon.triggers[trigger_index].ejection_port_position;
				}
				break;
			}
			case _weapon_function_primary_rate_of_fire:
			case _weapon_function_secondary_rate_of_fire:
			{
				short trigger_index = function_mode - _weapon_function_primary_rate_of_fire;

				if (trigger_index < weapon_definition->weapon.triggers.count)
				{
					function_value = weapon->weapon.triggers[trigger_index].rate_of_fire;
				}
				break;
			}
			case _weapon_function_primary_firing:
			case _weapon_function_secondary_firing:
			{
				short trigger_index = function_mode - _weapon_function_primary_firing;

				if (trigger_index < weapon_definition->weapon.triggers.count)
				{
					function_value = weapon->weapon.triggers[trigger_index].rate_of_fire;

					if (game_time_get() - weapon->weapon.game_time_last_fired > 1)
					{
						function_value = 0.f;
					}
				}
				break;
			}
			case _weapon_function_primary_charged:
			case _weapon_function_secondary_charged:
			{
				short trigger_index = function_mode - _weapon_function_primary_charged;

				if (trigger_index < weapon_definition->weapon.triggers.count)
				{
					struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
						&weapon_definition->weapon.triggers,
						trigger_index,
						struct weapon_trigger_definition);
					struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);

					function_value = weapon_trigger_get_charged_fraction(weapon_index, trigger_index);
				}
				break;
			}
			case _weapon_function_integrated_light:
				function_value = weapon->weapon.integrated_light_power;
				break;

			case _weapon_function_age:
				function_value = weapon->weapon.age;
				break;
			}

			object->object.incoming_function_values[function_index] = function_value;
		}
	}

	return;
}

void weapon_preprocess_node_orientations(
	long weapon_index,
	real_orientation *node_orientations)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct animation_graph const *animation_graph = animation_graph_definition_get(weapon_definition->object.animation_graph.index);
	struct animation_graph_weapon_animations const *weapon_animations = animation_graph->weapon_animations.count ? TAG_BLOCK_GET_ELEMENT(&animation_graph->weapon_animations, 0, struct animation_graph_weapon_animations) : NULL;

	return;
}

char const *weapon_get_label(
	long weapon_index)
{
	char const *label = "";

	if (weapon_index != NONE)
	{
		struct weapon_datum *weapon = weapon_get(weapon_index);
		struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

		label = weapon_definition->weapon.label;
	}

	return label;
}

boolean weapon_handle_potential_inventory_item(
	long weapon_index,
	long item_index,
	short local_player_parent_index,
	short *rounds_picked_up)
{
	short magazine_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	long item_definition_index = item_get(item_index)->definition_index;
	boolean handled = FALSE;

	for (magazine_index = 0; magazine_index < weapon_definition->weapon.magazines.count; magazine_index++)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			magazine_index,
			struct weapon_magazine_definition);

		if (magazine->rounds_total < magazine_definition->rounds_total_maximum)
		{
			short rounds_needed = magazine_definition->rounds_total_maximum - magazine->rounds_total;
			short rounds = 0;

			if (weapon->definition_index == item_definition_index)
			{
				struct weapon_datum *item_weapon = weapon_get(item_index);
				struct weapon_magazine *item_magazine = weapon_magazine_get(item_weapon, magazine_index);

				rounds = MIN(item_magazine->rounds_total, rounds_needed);

				if (rounds > 0)
				{
					item_magazine->rounds_total -= rounds;

					if (weapon_definition->weapon.pickup_sound.index != NONE && local_player_parent_index != NONE)
					{
						unspatialized_impulse_sound_new(weapon_definition->weapon.pickup_sound.index, 1.f);
					}

					if (!item_magazine->rounds_total)
					{
						object_delete(item_index);
					}
				}

				handled = TRUE;
			}
			else
			{
				short ammunition_index;

				for (ammunition_index = 0; ammunition_index < magazine_definition->ammunition_objects.count; ammunition_index++)
				{
					struct weapon_ammunition_object const *ammunition_object = TAG_BLOCK_GET_ELEMENT(
						&magazine_definition->ammunition_objects,
						ammunition_index,
						struct weapon_ammunition_object);

					if (ammunition_object->object.index == item_definition_index)
					{
						rounds = MIN(ammunition_object->rounds, rounds_needed);

						if (rounds > 0)
						{
							if (local_player_parent_index != NONE)
							{
								equipment_definition_handle_pickup(ammunition_object->object.index);
							}

							object_delete(item_index);
							handled = TRUE;
							break;
						}
					}
				}
			}

			magazine->rounds_total += rounds;
			*rounds_picked_up = rounds;
		}
	}

	return handled;
}

void weapon_ready(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	weapon_reset(weapon_index);
	weapon_set_state(weapon_index, _weapon_state_ready, TRUE);
	first_person_weapon_message_from_weapon(weapon_index, _first_person_weapon_message_ready);
	weapon_effect_new(weapon_index, weapon_definition->weapon.ready_effect.index, 0.f, 0.f);
	weapon->weapon.state_timer = weapon_get_first_person_animation_time(
		weapon_index,
		_first_person_animation_time_end,
		_first_person_weapon_animation_ready,
		NONE);

	return;
}

boolean weapon_put_away(
	long weapon_index,
	boolean immediate)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
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

		first_person_weapon_message_from_weapon(weapon_index, _first_person_weapon_message_put_away);
		put_away = TRUE;
	}

	return put_away;
}

void weapon_owner_update(
	long weapon_index,
	unsigned long control_flags,
	real primary_trigger)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	weapon->weapon.control_flags = control_flags;
	weapon->weapon.primary_trigger = transition_function_evaluate(_transition_function_very_late, primary_trigger);
	match_assert_valid_real("c:\\halo\\SOURCE\\items\\weapons.c", 1199, weapon->weapon.primary_trigger);

	return;
}

void weapon_build_weapon_interface_state(
	long weapon_index,
	struct weapon_interface_state *state)
{
	short magazine_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	state->heat = weapon->weapon.heat;
	state->age = weapon->weapon.age;
	state->overheated = TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit);
	state->magazine_count = (short)weapon_definition->weapon.magazines.count;

	for (magazine_index = 0; magazine_index < weapon_definition->weapon.magazines.count; magazine_index++)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			magazine_index,
			struct weapon_magazine_definition);

		state->magazines[magazine_index].reloading = magazine->state == _magazine_reloading || magazine->state == _magazine_chambering;
		state->magazines[magazine_index].can_fire = magazine->state == _magazine_idle;
		state->magazines[magazine_index].rounds_loaded = magazine->rounds_loaded;
		state->magazines[magazine_index].rounds_loaded_maximum = magazine_definition->rounds_loaded_maximum;
		state->magazines[magazine_index].rounds_remaining = magazine->rounds_total;
		state->magazines[magazine_index].rounds_remaining_maximum = magazine_definition->rounds_total_maximum;
	}

	return;
}

boolean weapon_reloading(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	boolean reloading = FALSE;

	if (weapon_definition->weapon.magazines.count > 0)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, _weapon_magazine_primary);

		if (magazine->state == _magazine_reloading)
		{
			reloading = TRUE;
		}
	}

	return reloading;
}

void weapon_set_integrated_light_power(
	long weapon_index,
	real light_power)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	weapon->weapon.integrated_light_power = light_power;

	return;
}

boolean weapon_aim(
	long weapon_index,
	short trigger_index,
	real_point3d const *origin,
	real_point3d const *target_point,
	boolean lob,
	real_vector3d *result_aim_vector,
	real *result_ticks,
	real *result_distance,
	boolean *result_linear)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	boolean success = FALSE;

	if (trigger_index >= 0 && trigger_index < weapon_definition->weapon.triggers.count)
	{
		struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
		struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.triggers,
			trigger_index,
			struct weapon_trigger_definition);

		projectile_aim(
			projectile_definition_get(trigger_definition->projectile.index),
			origin,
			target_point,
			NULL,
			NULL,
			NULL,
			NULL,
			lob,
			result_aim_vector,
			NULL,
			result_ticks,
			result_distance,
			result_linear);
		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\items\\weapons.c", 1301, result_aim_vector);
		success = TRUE;
	}

	return success;
}

real weapon_estimate_time_to_target(
	long weapon_index,
	short trigger_index,
	real target_distance)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	real time = 0.f;

	if (trigger_index >= 0 && trigger_index < weapon_definition->weapon.triggers.count)
	{
		struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.triggers,
			trigger_index,
			struct weapon_trigger_definition);

		time = projectile_estimate_time_to_target(
			projectile_definition_get(trigger_definition->projectile.index),
			target_distance);
	}

	return time;
}

/* Used to determine if a weapon can ever be fired again. Used to determine if a weapon should be deleted in multiplayer */
boolean weapon_can_be_fired(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	// Weapons that use battery/age can't be refilled so they cannot be fired again
	if (weapon->weapon.age >= 1.f)
	{
		return FALSE;
	}
	// If not in multiplayer a player might pick up new ammo for a weapon. So it can technically be fired
	// Weapons that can't have ammo can still be "fired"
	else if (game_engine_running() &&
		weapon_definition->weapon.magazines.count > 0 &&
		TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, 0, struct weapon_magazine_definition)->rounds_loaded_maximum > 0 &&
		!weapon->weapon.magazines[_weapon_magazine_primary].rounds_loaded &&
		!weapon->weapon.magazines[_weapon_magazine_primary].rounds_total)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

boolean weapon_useful(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	if (weapon->weapon.age >= 1.f)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

real weapon_compute_movement_penalty(
	long weapon_index,
	boolean forward,
	boolean zoomed)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	real penalty = forward ? weapon_definition->weapon.forward_movement_penalty : weapon_definition->weapon.sideways_movement_penalty;

	switch (weapon_definition->weapon.movement_penalty_mode)
	{
	case _weapon_movement_penalty_when_zoomed:
		if (!zoomed)
		{
			penalty = 0.f;
		}
		break;

	case _weapon_movement_penalty_when_zoomed_or_reloading:
		// original bug: clears the penalty while reloading unzoomed instead of while idle unzoomed (hcex too)
		if ((weapon->weapon.magazines[_weapon_magazine_primary].state == _magazine_reloading || weapon->weapon.magazines[_weapon_magazine_secondary].state == _magazine_reloading) && !zoomed)
		{
			penalty = 0.f;
		}
		break;
	}

	return penalty;
}

short weapon_rotate_zoom_level(
	long weapon_index,
	short zoom_level)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	if (!weapon_reloading(weapon_index))
	{
		if (zoom_level >= 0 && zoom_level < weapon_definition->weapon.zoom_level_count - 1)
		{
			zoom_level++;
		}
		else if (zoom_level == weapon_definition->weapon.zoom_level_count - 1)
		{
			zoom_level = NONE;
		}
		else
		{
			zoom_level = 0;
		}
	}

	return zoom_level;
}

real weapon_get_zoom_magnification(
	long weapon_index,
	short zoom_level)
{
	real magnification = 1.f;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	if (zoom_level >= 0 && zoom_level < weapon_definition->weapon.zoom_level_count)
	{
		real zoom_fraction = weapon_definition->weapon.zoom_level_count > 1 ? (real)zoom_level / (weapon_definition->weapon.zoom_level_count - 1) : 0.f;
		real minimum_magnification = weapon_definition->weapon.zoom_magnification_minimum > 0.f ? weapon_definition->weapon.zoom_magnification_minimum : 1.f;
		real maximum_magnification = weapon_definition->weapon.zoom_magnification_maximum > 0.f ? weapon_definition->weapon.zoom_magnification_maximum : 1.f;

		magnification = power(maximum_magnification / minimum_magnification, zoom_fraction) * minimum_magnification;
		match_assert_valid_real("c:\\halo\\SOURCE\\items\\weapons.c", 1442, magnification);
		match_assert("c:\\halo\\SOURCE\\items\\weapons.c", 1443, magnification>0.0f);
	}

	return magnification;
}

real weapon_get_field_of_view(
	long weapon_index,
	real default_field_of_view,
	short zoom_level)
{
	real field_of_view = default_field_of_view;
	real magnification = weapon_get_zoom_magnification(weapon_index, zoom_level);

	if (magnification != 1.f)
	{
		real zoomed_field_of_view = default_field_of_view / magnification;

		if (zoomed_field_of_view > _pi / 100.f && zoomed_field_of_view < _pi - _pi / 100.f)
		{
			field_of_view = zoomed_field_of_view;
		}
	}

	return field_of_view;
}

void weapon_melee_attack(
	long weapon_index)
{
	return;
}

boolean weapon_must_be_readied(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	return TEST_FLAG(weapon_definition->weapon.flags, _weapon_must_be_readied_bit);
}

boolean weapon_is_flag(
	long weapon_index)
{
	return weapon_must_be_readied(weapon_index);
}

boolean weapon_prevents_melee_attack(
	long weapon_index)
{
	boolean prevents_melee_attack = TRUE;

	if (weapon_index != NONE)
	{
		struct weapon_datum *weapon = weapon_get(weapon_index);
		struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

		prevents_melee_attack = TEST_FLAG(weapon_definition->weapon.flags, _weapon_prevents_melee_attack_bit);

		if (weapon_overcharged(weapon_index))
		{
			prevents_melee_attack = TRUE;
		}
	}

	return prevents_melee_attack;
}

boolean weapon_prevents_grenade_throwing(
	long weapon_index)
{
	boolean prevents_grenade_throwing = TRUE;

	if (weapon_index != NONE)
	{
		struct weapon_datum *weapon = weapon_get(weapon_index);
		struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

		prevents_grenade_throwing = TEST_FLAG(weapon_definition->weapon.flags, _weapon_prevents_grenade_throwing_bit);

		if (weapon->weapon.state >= _weapon_state_primary_reload && weapon->weapon.state <= _weapon_state_put_away)
		{
			prevents_grenade_throwing = TRUE;
		}
	}

	return prevents_grenade_throwing;
}

void weapon_stop_reload(
	long weapon_index)
{
	weapon_reset(weapon_index);

	return;
}

short weapon_get_first_person_animation_time(
	long weapon_index,
	short mode,
	short animation_type,
	short shotgun_reload_type)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	short time = 0;

	if (weapon_definition->weapon.interface_definition.first_person_animations.index != NONE)
	{
		struct animation_graph const *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);
		struct animation_graph_first_person_weapon_animations const *weapon_animations = animation_graph->first_person_weapon_animations.count ? TAG_BLOCK_GET_ELEMENT(&animation_graph->first_person_weapon_animations, 0, struct animation_graph_first_person_weapon_animations) : NULL;

		if (weapon_animations)
		{
			short animation_index = animation_type >= 0 && animation_type < weapon_animations->animations.count ? animation_graph_animation_index_get(&weapon_animations->animations)[animation_type].animation_index : NONE;

			if (animation_index != NONE)
			{
				struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

				switch (mode)
				{
				case _first_person_animation_time_end:
					time = animation_convert_frame_to_pal(animation, animation->frame_count);
					break;

				case _first_person_animation_time_key:
					time = animation_convert_frame_to_pal(animation, animation_key_frame_index(animation));
					break;

				default:
					match_halt("c:\\halo\\SOURCE\\items\\weapons.c", 1588);
					break;
				}

				if (mode == _first_person_animation_time_end && weapon_definition->weapon.weapon_type == _weapon_type_shotgun)
				{
					struct animation const *enter_animation = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						_first_person_weapon_animation_shotgun_enter >= 0 && _first_person_weapon_animation_shotgun_enter < weapon_animations->animations.count ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_shotgun_enter].animation_index : NONE,
						struct animation);
					struct animation const *exit_empty_animation = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						_first_person_weapon_animation_shotgun_exit_empty >= 0 && _first_person_weapon_animation_shotgun_exit_empty < weapon_animations->animations.count ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_shotgun_exit_empty].animation_index : NONE,
						struct animation);
					struct animation const *exit_full_animation = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						_first_person_weapon_animation_shotgun_exit_full >= 0 && _first_person_weapon_animation_shotgun_exit_full < weapon_animations->animations.count ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_shotgun_exit_full].animation_index : NONE,
						struct animation);

					// the exit animations are looked up but never used (sapien and hcex do the same)
					switch (shotgun_reload_type)
					{
					case _shotgun_reload_type_first_round:
						time = animation_convert_frame_to_pal(enter_animation, enter_animation->frame_count);
						break;

					case _shotgun_reload_type_first_and_last_round:
						time = animation_convert_frame_to_pal(enter_animation, enter_animation->frame_count);
						break;
					}
				}
			}
		}
	}

	return time;
}

boolean weapon_overcharged(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	return weapon->weapon.triggers[_weapon_trigger_primary].state == _trigger_charging || weapon->weapon.triggers[_weapon_trigger_primary].state == _trigger_charged;
}

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
		weapon->weapon.triggers[_weapon_trigger_primary].state != _trigger_idle ||
		weapon->weapon.triggers[_weapon_trigger_secondary].state != _trigger_idle ||
		weapon->weapon.magazines[_weapon_magazine_primary].state != _magazine_idle ||
		weapon->weapon.magazines[_weapon_magazine_secondary].state != _magazine_idle ||
		weapon->weapon.state != _weapon_state_idle;
}

static boolean weapon_magazine_state_change_ok(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	return
		weapon->weapon.triggers[_weapon_trigger_primary].state == _trigger_idle &&
		weapon->weapon.triggers[_weapon_trigger_secondary].state == _trigger_idle &&
		weapon->weapon.state == _weapon_state_idle;
}

static long weapon_get_effect_object_index(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	long effect_object_index = weapon_index;

	if (TEST_FLAG(weapon->object.flags, _object_invisible_bit) && weapon->object.parent_object_index != NONE)
	{
		effect_object_index = weapon->object.parent_object_index;
	}

	return effect_object_index;
}

static long weapon_get_owner_object_index(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	long owner_object_index = NONE;

	if (weapon->object.parent_object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(weapon->object.parent_object_index);

		if (unit)
		{
			owner_object_index = weapon->object.parent_object_index;
		}
	}

	return owner_object_index;
}

static long weapon_get_projectile_owner_object_index(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	long owner_object_index = NONE;

	if (weapon->object.parent_object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(weapon->object.parent_object_index);

		if (unit)
		{
			owner_object_index = weapon->object.parent_object_index;

			if (unit->unit.gunner_object_index != NONE)
			{
				owner_object_index = unit->unit.gunner_object_index;
			}
		}
	}

	return owner_object_index;
}

static real weapon_trigger_get_charged_fraction(
	long weapon_index,
	short trigger_index)
{
	real charged_fraction;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	switch (trigger->state)
	{
	case _trigger_charging:
		charged_fraction = 1.f - (trigger->state_timer * SECONDS_PER_TICK) / trigger_definition->charging_time;
		break;

	case _trigger_charged:
		charged_fraction = 1.f;
		break;

	default:
		charged_fraction = 0.f;
		break;
	}

	return charged_fraction;
}

static boolean weapon_trigger_can_fire_again(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);
	boolean can_fire_again = FALSE;
	real rate_of_fire_fraction = TEST_FLAG(trigger_definition->flags, _weapon_trigger_analog_rate_of_fire_bit) ? weapon->weapon.primary_trigger : trigger->rate_of_fire;
	real rate_of_fire = (trigger_definition->final_rate_of_fire - trigger_definition->initial_rate_of_fire) * rate_of_fire_fraction + trigger_definition->initial_rate_of_fire;
	real ticks_between_shots = rate_of_fire > _real_epsilon ? TICKS_PER_SECOND / rate_of_fire : 0.f;

	if (weapon_definition->weapon.age_rate_of_fire_penalty > 0.f)
	{
		ticks_between_shots += weapon->weapon.age * ticks_between_shots * weapon_definition->weapon.age_rate_of_fire_penalty;
	}

	if (trigger->idle_ticks + 1.f >= ticks_between_shots)
	{
		can_fire_again = TRUE;
	}

	if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_latched_bit) &&
		TEST_FLAG(weapon->item.flags, _item_belongs_to_player_bit) &&
		!TEST_FLAG(trigger->flags, _weapon_trigger_released_since_last_shot_bit))
	{
		can_fire_again = FALSE;
	}

	return can_fire_again;
}

static void weapon_trigger_begin_firing(
	long weapon_index,
	short trigger_index,
	boolean force_firing)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);
	boolean can_fire = TRUE;

	if (trigger_definition->magazine_index != NONE)
	{
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			trigger_definition->magazine_index,
			struct weapon_magazine_definition);
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, trigger_definition->magazine_index);

		if (magazine->state != _magazine_idle)
		{
			can_fire = FALSE;
		}
	}

	if (TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit))
	{
		can_fire = FALSE;
	}

	if (scenario_location_underwater(&weapon->object.location, &weapon->object.position, NULL))
	{
		can_fire = FALSE;
	}

	if (can_fire)
	{
		if (!force_firing && trigger_definition->charging_time > 0.f)
		{
			if (TEST_FLAG(weapon_definition->weapon.flags, _weapon_cannot_fire_at_maximum_age_bit) && weapon->weapon.age >= 1.f)
			{
				weapon_trigger_fire(weapon_index, trigger_index);
			}
			else
			{
				if (weapon_definition->weapon.triggers.count > 1)
				{
					trigger->charging_effect_index = weapon_effect_new(
						weapon_index,
						trigger_definition->charging_effect.index,
						0.f,
						0.f);
				}
				else if (trigger->rate_of_fire > 0.f)
				{
					SET_FLAG(trigger->flags, _weapon_trigger_fired_before_charging_bit, TRUE);
					weapon_trigger_fire(weapon_index, trigger_index);
				}
				else
				{
					SET_FLAG(trigger->flags, _weapon_trigger_fired_before_charging_bit, FALSE);
				}

				weapon_trigger_change_state(
					weapon_index,
					trigger_index,
					_trigger_charging,
					(short)(trigger_definition->charging_time * TICKS_PER_SECOND));
			}
		}
		else if (!force_firing && trigger_definition->overloading_time > 0.f)
		{
			weapon_trigger_change_state(
				weapon_index,
				trigger_index,
				_trigger_overloading,
				(short)(trigger_definition->overloading_time * TICKS_PER_SECOND));
		}
		else
		{
			weapon_trigger_fire(weapon_index, trigger_index);
		}
	}

	return;
}

static void weapon_magazine_start_reload(
	long weapon_index,
	short magazine_index,
	boolean first_round)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.magazines,
		magazine_index,
		struct weapon_magazine_definition);

	if (weapon_magazine_state_interruptable(magazine->state, _magazine_reloading) && weapon_magazine_state_change_ok(weapon_index))
	{
		if (magazine->rounds_total > 0 && magazine->rounds_loaded < magazine_definition->rounds_loaded_maximum)
		{
			short shotgun_reload_type = NONE;

			weapon_set_state(weapon_index, _weapon_state_primary_reload + magazine_index, FALSE);
			weapon_effect_new(weapon_index, magazine_definition->reloading_effect.index, 0.f, 0.f);
			first_person_weapon_message_from_weapon(
				weapon_index,
				magazine->rounds_loaded ? _first_person_weapon_message_reload_while_full : _first_person_weapon_message_reload_while_empty);

			if (weapon_definition->weapon.weapon_type == _weapon_type_shotgun)
			{
				if (first_round)
				{
					shotgun_reload_type = magazine_definition->rounds_loaded_maximum - magazine->rounds_loaded == 1 ? _shotgun_reload_type_first_and_last_round : _shotgun_reload_type_first_round;
				}
				else
				{
					shotgun_reload_type = magazine_definition->rounds_loaded_maximum - magazine->rounds_loaded == 1 ? _shotgun_reload_type_last_round : NONE;
				}
			}

			magazine->state = _magazine_reloading;
			magazine->state_timer = weapon_get_first_person_animation_time(
				weapon_index,
				_first_person_animation_time_end,
				_first_person_weapon_animation_reload_while_empty,
				shotgun_reload_type);
			magazine->original_time = magazine->state_timer;
		}

		SET_FLAG(weapon->weapon.flags, _weapon_needs_to_reload_bit, FALSE);
	}

	return;
}

static void weapon_magazine_finish_reload(
	long weapon_index,
	short magazine_index)
{
	short rounds_loaded;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.magazines,
		magazine_index,
		struct weapon_magazine_definition);

	if (TEST_FLAG(magazine_definition->flags, _weapon_magazine_wastes_rounds_when_reloaded_bit))
	{
		magazine->rounds_loaded = 0;
	}

	rounds_loaded = MIN(magazine_definition->rounds_reloaded, magazine->rounds_total) + magazine->rounds_loaded;
	rounds_loaded = MIN(rounds_loaded, magazine_definition->rounds_loaded_maximum);

	if (!cheat.infinite_ammo && TEST_FLAG(weapon->item.flags, _item_belongs_to_player_bit))
	{
		magazine->rounds_total -= rounds_loaded - magazine->rounds_loaded;
	}

	magazine->rounds_loaded = rounds_loaded;
	magazine->state = _magazine_unchambered;
	magazine->state_timer = 0;

	if (magazine->rounds_total > 0 &&
		magazine->rounds_loaded < magazine_definition->rounds_loaded_maximum &&
		!TEST_FLAG(magazine_definition->flags, _weapon_magazine_wastes_rounds_when_reloaded_bit) &&
		!(weapon->weapon.control_flags & (FLAG(_weapon_control_primary_trigger_bit) | FLAG(_weapon_control_secondary_trigger_bit) | FLAG(_weapon_control_user_switching_weapons_bit))))
	{
		weapon_magazine_start_reload(weapon_index, magazine_index, FALSE);
	}

	return;
}

static void weapon_magazine_start_chamber(
	long weapon_index,
	short magazine_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);

	if (weapon_magazine_state_interruptable(magazine->state, _magazine_chambering) && weapon_magazine_state_change_ok(weapon_index))
	{
		struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			magazine_index,
			struct weapon_magazine_definition);

		weapon_set_state(weapon_index, _weapon_state_primary_chamber + magazine_index, FALSE);
		weapon_effect_new(weapon_index, magazine_definition->chambering_effect.index, 0.f, 0.f);
		magazine->state = _magazine_chambering;
		magazine->state_timer = (short)(magazine_definition->chamber_time * TICKS_PER_SECOND);
	}

	return;
}

static void weapon_magazine_finish_chamber(
	long weapon_index,
	short magazine_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.magazines,
		magazine_index,
		struct weapon_magazine_definition);

	weapon_magazine_idle(weapon_index, magazine_index);

	return;
}

static void weapon_magazine_idle(
	long weapon_index,
	short magazine_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.magazines,
		magazine_index,
		struct weapon_magazine_definition);

	magazine->state = _magazine_idle;
	magazine->state_timer = 0;

	return;
}

static void weapon_trigger_fully_charged(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	weapon_trigger_change_state(
		weapon_index,
		trigger_index,
		_trigger_charged,
		(short)(trigger_definition->charged_time * TICKS_PER_SECOND));
	weapon_set_state(weapon_index, _weapon_state_primary_charged + trigger_index, TRUE);
	first_person_weapon_message_from_weapon(weapon_index, _first_person_weapon_message_charged);

	return;
}

static void weapon_trigger_overload(
	long weapon_index,
	long trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, (short)trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	if (trigger_index + 1 < weapon_definition->weapon.triggers.count)
	{
		weapon_trigger_fire(weapon_index, (short)(trigger_index + 1));
	}

	weapon_trigger_change_state(
		weapon_index,
		(short)trigger_index,
		_trigger_overloading,
		(short)(trigger_definition->overloading_time * TICKS_PER_SECOND));

	return;
}

static void weapon_trigger_release_charge(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	if (trigger_definition->spew_time > 0.f)
	{
		weapon_trigger_change_state(
			weapon_index,
			trigger_index,
			_trigger_spewing,
			(short)(trigger_definition->spew_time * TICKS_PER_SECOND));
	}
	else
	{
		if (weapon_definition->weapon.triggers.count > 1)
		{
			weapon_trigger_fire(weapon_index, _weapon_trigger_secondary);
		}

		weapon_trigger_recover(weapon_index, trigger_index);
	}

	trigger->rate_of_fire = 0.f;

	return;
}

static void weapon_trigger_overcharged(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	switch (trigger_definition->overcharged_action)
	{
	case _trigger_overcharged_explodes:
		weapon_detonate(weapon_index);
		break;

	case _trigger_overcharged_fire:
		weapon_trigger_release_charge(weapon_index, trigger_index);
		break;
	}

	return;
}

static void weapon_trigger_idle(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	weapon_trigger_change_state(weapon_index, trigger_index, _trigger_idle, 0);

	return;
}

static void weapon_trigger_locked(
	long weapon_index,
	short trigger_index)
{
	weapon_trigger_change_state(weapon_index, trigger_index, _trigger_locked, NONE);

	return;
}

static void weapon_trigger_finish_tracking(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	weapon->weapon.tracked_object_index = NONE;
	weapon_trigger_recover(weapon_index, trigger_index);

	return;
}

static void weapon_trigger_fire(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);
	long owner_object_index = weapon_get_owner_object_index(weapon_index);
	long damage_effect_index = NONE;
	long effect_index = NONE;
	real effect_error = 0.f;
	real effect_scale = 0.f;
	boolean fired = FALSE;
	boolean misfire = FALSE;
	boolean loads_alternate_ammunition = FALSE;

	if (trigger_index == _weapon_trigger_secondary &&
		(weapon_definition->weapon.secondary_trigger_mode == _weapon_secondary_trigger_loads_alternate_ammunition ||
		weapon_definition->weapon.secondary_trigger_mode == _weapon_secondary_trigger_loads_multiple_primary_ammunition))
	{
		loads_alternate_ammunition = TRUE;
	}

	if (trigger_definition->magazine_index != NONE)
	{
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			trigger_definition->magazine_index,
			struct weapon_magazine_definition);
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, trigger_definition->magazine_index);

		if ((!loads_alternate_ammunition || weapon->weapon.alternate_shots_loaded < weapon_definition->weapon.maximum_alternate_shots_loaded) &&
			(magazine->rounds_loaded >= trigger_definition->rounds_per_shot || TEST_FLAG(trigger_definition->flags, _weapon_trigger_can_fire_with_partial_ammunition_bit)) &&
			!(TEST_FLAG(weapon_definition->weapon.flags, _weapon_cannot_fire_at_maximum_age_bit) && weapon->weapon.age >= 1.f) &&
			(magazine->rounds_loaded >= trigger_definition->minimum_rounds_loaded_per_shot || !TEST_FLAG(trigger->flags, _weapon_trigger_released_since_last_shot_bit)))
		{
			if (!cheat.bottomless_clip && (magazine->rounds_loaded -= trigger_definition->rounds_per_shot) <= 0)
			{
				magazine->rounds_loaded = 0;
			}
			else if (TEST_FLAG(magazine_definition->flags, _weapon_magazine_must_be_chambered_every_shot_bit))
			{
				magazine->state = _magazine_unchambered;
				magazine->state_timer = 0;
			}

			fired = TRUE;
		}
	}
	else
	{
		fired = TRUE;
	}

	if (cheat.bottomless_clip)
	{
		fired = TRUE;
	}

	if (trigger_definition->firing_effects.count > 0)
	{
		struct trigger_firing_effect const *firing_effect;
		short firing_effect_type;

		if (trigger->firing_effect_shots_remaining <= 0)
		{
			struct trigger_firing_effect const *firing_effect;
			short starting_firing_effect_index = trigger->firing_effect_index;
			short firing_effect_index = TEST_FLAG(trigger_definition->flags, _weapon_trigger_random_firing_effects_bit) ? random() % trigger_definition->firing_effects.count : trigger->firing_effect_index;

			do
			{
				if (trigger->firing_effects_used_flags == MASK(trigger_definition->firing_effects.count))
				{
					trigger->firing_effects_used_flags = 0;
				}

				do
				{
					firing_effect_index++;

					if (firing_effect_index >= trigger_definition->firing_effects.count)
					{
						firing_effect_index = 0;
					}
				}
				while (TEST_FLAG(trigger->firing_effects_used_flags, firing_effect_index));

				firing_effect = TAG_BLOCK_GET_ELEMENT(
					&trigger_definition->firing_effects,
					firing_effect_index,
					struct trigger_firing_effect);
				trigger->firing_effect_index = firing_effect_index;
				trigger->firing_effects_used_flags |= FLAG(firing_effect_index);
				trigger->firing_effect_shots_remaining = random_range(
					firing_effect->shots_lower_bound,
					firing_effect->shots_upper_bound);
			}
			while (trigger->firing_effect_shots_remaining <= 0 && firing_effect_index != starting_firing_effect_index);
		}

		trigger->firing_effect_shots_remaining--;
		firing_effect = TAG_BLOCK_GET_ELEMENT(
			&trigger_definition->firing_effects,
			trigger->firing_effect_index,
			struct trigger_firing_effect);

		if (weapon_definition->weapon.age_misfire_start > 0.f &&
			weapon_definition->weapon.age_misfire_start < 1.f &&
			weapon->weapon.age > weapon_definition->weapon.age_misfire_start)
		{
			real misfire_chance = (weapon->weapon.age - weapon_definition->weapon.age_misfire_start) * weapon_definition->weapon.age_misfire_chance / (1.f - weapon_definition->weapon.age_misfire_start);

			if (trigger->state == _trigger_spewing)
			{
				misfire_chance *= 2.f;
			}

			if (real_random() < misfire_chance)
			{
				misfire = TRUE;
			}
		}

		if (!fired)
		{
			firing_effect_type = _trigger_empty_effect;
			effect_scale = 1.f;
			effect_error = 0.f;
		}
		else if (misfire)
		{
			firing_effect_type = _trigger_overheated_effect;
			effect_scale = trigger->rate_of_fire;
			effect_error = 0.f;
		}
		else
		{
			firing_effect_type = _trigger_firing_effect;
			effect_scale = trigger->rate_of_fire;
			effect_error = weapon_definition->weapon.heat_overheated_threshold == 0.f ? 0.f : weapon->weapon.heat / weapon_definition->weapon.heat_overheated_threshold;
		}

		effect_index = firing_effect->effects[firing_effect_type].index;
		damage_effect_index = firing_effect->damage_effects[firing_effect_type].index;
	}

	if (fired)
	{
		if (TEST_FLAG(weapon->item.flags, _item_belongs_to_player_bit) && game_engine_running())
		{
			long player_index = player_index_from_unit_index(owner_object_index);

			if (player_index != NONE)
			{
				game_engine_weapon_fired(player_index);
			}
		}

		weapon->weapon.game_time_last_fired = game_time_get();
		first_person_weapon_message_from_weapon(
			weapon_index,
			misfire ?
			(trigger_index ? _first_person_weapon_message_secondary_misfire : _first_person_weapon_message_primary_misfire) :
			(trigger_index ? _first_person_weapon_message_secondary_fire : _first_person_weapon_message_primary_fire));
		weapon_trigger_start_ejection_port(weapon_index, trigger_index, FALSE);

		if (trigger_definition->illumination_recovery_time > 0.f)
		{
			trigger->illumination = 1.f;
		}

		if (!cheat.bottomless_clip)
		{
			weapon->weapon.heat += trigger_definition->heat_generated_per_round;
		}

		if (!TEST_FLAG(weapon->item.flags, _item_belongs_to_player_bit) && weapon->weapon.heat > weapon_definition->weapon.heat_overheated_threshold)
		{
			weapon->weapon.heat = weapon_definition->weapon.heat_overheated_threshold;
		}
		else if (weapon->weapon.heat > 1.f)
		{
			weapon->weapon.heat = 1.f;
		}

		if (TEST_FLAG(weapon->item.flags, _item_belongs_to_player_bit) && !cheat.infinite_ammo)
		{
			weapon->weapon.age += trigger_definition->age_generated_per_round;

			if (weapon->weapon.age > 1.f)
			{
				weapon->weapon.age = 1.f;
			}
		}

		weapon_set_state(
			weapon_index,
			trigger_index ? _weapon_state_secondary_recoil : _weapon_state_primary_recoil,
			FALSE);

		if (!misfire)
		{
			if (loads_alternate_ammunition)
			{
				weapon->weapon.alternate_shots_loaded++;
			}
			else
			{
				trigger_create_projectiles(weapon_index, trigger_index);
				ai_handle_unit_effect(owner_object_index, _ai_unit_effect_shooting, trigger_definition->firing_noise);
			}
		}

		if (owner_object_index != NONE && damage_effect_index != NONE)
		{
			struct damage_data damage_data;
			struct unit_datum *unit = unit_get(owner_object_index);

			damage_data_new(&damage_data, damage_effect_index);
			SET_FLAG(damage_data.flags, _damage_from_weapon_bit, TRUE);
			negate_vector3d(&unit->unit.aiming_vector, &damage_data.direction);
			damage_data.epicenter = unit->object.bounding_sphere_center;
			damage_data.origin = damage_data.epicenter;
			object_cause_damage(&damage_data, owner_object_index, NONE, NONE, NONE, NULL);
		}

		if (weapon_definition->weapon.weapon_type == _weapon_type_plasma_pistol && trigger_index == _weapon_trigger_secondary)
		{
			SET_FLAG(weapon->weapon.flags, _weapon_overheat_recoil_bit, TRUE);
		}
	}

	if (weapon->weapon.heat > weapon_definition->weapon.heat_detonation_threshold && real_random() < weapon_definition->weapon.overheated_explosion_fraction)
	{
		weapon_detonate(weapon_index);
	}

	if (!fired)
	{
		weapon_trigger_change_state(weapon_index, trigger_index, _trigger_locked, NONE);
	}
	else if (trigger->state != _trigger_spewing || misfire)
	{
		if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_tracks_projectile_bit))
		{
			weapon_trigger_change_state(weapon_index, trigger_index, _trigger_tracking, NONE);
		}
		else
		{
			weapon_trigger_recover(weapon_index, trigger_index);
		}
	}

	SET_FLAG(trigger->flags, _weapon_trigger_released_since_last_shot_bit, FALSE);
	weapon_effect_new(weapon_index, effect_index, effect_scale, effect_error);

	return;
}

static long weapon_effect_new(
	long weapon_index,
	long effect_index,
	real effect_scale,
	real effect_error)
{
	long new_effect_index = NONE;

	if (effect_index != NONE)
	{
		long effect_object_index = weapon_get_effect_object_index(weapon_index);
		long owner_object_index = weapon_get_owner_object_index(weapon_index);

		switch (tag_get_group_tag(effect_index))
		{
		case EFFECT_DEFINITION_TAG:
			new_effect_index = effect_new_from_object(
				effect_index,
				owner_object_index,
				effect_object_index,
				NONE,
				effect_scale,
				effect_error,
				NULL,
				NULL);
			break;

		case SOUND_DEFINITION_TAG:
			object_impulse_sound_new(
				owner_object_index,
				effect_index,
				NONE,
				global_origin3d,
				global_forward3d,
				effect_scale);
			break;

		default:
			match_halt("c:\\halo\\SOURCE\\items\\weapons.c", 2514);
			break;
		}
	}

	return new_effect_index;
}

static long weapon_effect_looping_new(
	long weapon_index,
	long effect_index)
{
	long new_effect_index = NONE;

	if (effect_index != NONE)
	{
		long effect_object_index = weapon_get_effect_object_index(weapon_index);
		long owner_object_index = weapon_get_owner_object_index(weapon_index);

		if (effect_object_index != NONE)
		{
			new_effect_index = effect_new_looping(effect_index, effect_object_index, NONE, NONE, NONE);
		}
	}

	return new_effect_index;
}

static void weapon_trigger_recover(
	long weapon_index,
	short trigger_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	trigger->idle_ticks = 0;
	weapon_trigger_idle(weapon_index, trigger_index);

	return;
}

static void weapon_detonate(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	weapon_effect_new(weapon_index, weapon_definition->weapon.detonation_effect.index, 0.f, 0.f);
	object_delete(weapon_index);

	return;
}

static void weapon_trigger_change_state(
	long weapon_index,
	short trigger_index,
	short new_state,
	short new_state_timer)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	match_assert("c:\\halo\\SOURCE\\items\\weapons.c", 2577, trigger_index>=0 && trigger_index<MAXIMUM_NUMBER_OF_TRIGGERS_PER_WEAPON);
	match_assert("c:\\halo\\SOURCE\\items\\weapons.c", 2578, new_state>=0 && new_state<NUMBER_OF_TRIGGER_STATES);

	weapon->weapon.triggers[trigger_index].state = (char)new_state;
	weapon->weapon.triggers[trigger_index].state_timer = new_state_timer;

	return;
}

static void weapon_reset(
	long weapon_index)
{
	short trigger_index;
	short magazine_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	for (trigger_index = 0; trigger_index < weapon_definition->weapon.triggers.count; trigger_index++)
	{
		struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
		struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.triggers,
			trigger_index,
			struct weapon_trigger_definition);

		trigger->state = _trigger_uninitialized;
		trigger->state_timer = 0;
	}

	for (magazine_index = 0; magazine_index < weapon_definition->weapon.magazines.count; magazine_index++)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			magazine_index,
			struct weapon_magazine_definition);

		if (magazine->state == _magazine_reloading &&
			2 * magazine->state_timer < weapon_get_first_person_animation_time(
				weapon_index,
				_first_person_animation_time_end,
				_first_person_weapon_animation_reload_while_empty,
				NONE))
		{
			weapon_magazine_finish_reload(weapon_index, magazine_index);
		}

		magazine->state = _magazine_idle;
		magazine->state_timer = 0;
	}

	return;
}

static void trigger_create_projectiles(
	long weapon_index,
	short trigger_index)
{
	char const *trigger_marker_names[MAXIMUM_NUMBER_OF_TRIGGERS_PER_WEAPON];
	struct object_marker markers[MAXIMUM_MARKERS_PER_OBJECT];
	short marker_count;
	short marker_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);
	long owner_object_index = weapon_get_owner_object_index(weapon_index);

	trigger_marker_names[_weapon_trigger_primary] = "primary trigger";
	trigger_marker_names[_weapon_trigger_secondary] = "secondary trigger";
	marker_count = object_get_marker_by_name(
		weapon_get_effect_object_index(weapon_index),
		trigger_marker_names[trigger_index],
		markers,
		MAXIMUM_MARKERS_PER_OBJECT);

	if (!marker_count)
	{
		marker_count = 1;
	}

	if (!TEST_FLAG(trigger_definition->flags, _weapon_trigger_uses_weapon_origin_bit))
	{
		marker_count = 1;
	}

	for (marker_index = 0; marker_index < marker_count; marker_index++)
	{
		long projectile_definition_index;
		short projectile_count;
		real_point3d origin = markers[marker_index].matrix.position;
		real_vector3d forward = markers[marker_index].matrix.forward;
		real inherited_velocity_magnitude = 0.f;
		real desired_error_angle = 0.f;
		struct unit_datum *unit = unit_try_and_get(owner_object_index);
		long target_object_index = NONE;

		if (!TEST_FLAG(trigger_definition->flags, _weapon_trigger_projectiles_cannot_be_aimed_bit) &&
			unit &&
			!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			boolean offset_origin;
			boolean offset_aim;
			struct unit_definition const *unit_definition = unit_definition_get(unit->definition_index);
			long player_index = unit->unit.player_index;
			long actor_index = unit->unit.actor_index;

			if (unit->unit.gunner_object_index != NONE)
			{
				struct unit_datum *gunner = unit_get(unit->unit.gunner_object_index);

				player_index = gunner->unit.player_index;
				actor_index = gunner->unit.actor_index;
			}

			offset_origin = TEST_FLAG(unit_definition->unit.flags, _unit_fires_from_camera_bit);
			offset_aim = TRUE;

			if (actor_index != NONE && actor_firing_blindly(actor_index))
			{
				offset_aim = FALSE;
			}

			if (unit->unit.gunner_object_index != NONE)
			{
				offset_aim = FALSE;
			}

			unit_adjust_projectile_ray(
				owner_object_index,
				&origin,
				&forward,
				&inherited_velocity_magnitude,
				offset_origin,
				offset_aim);

			if (player_index != NONE)
			{
				real_vector3d left;
				real_vector3d up;

				cross_product3d(global_up3d, &forward, &left);

				if (normalize3d(&left) == 0.f)
				{
					left = *global_left3d;
				}

				cross_product3d(&forward, &left, &up);
				normalize3d(&up);
				point_from_line3d(&origin, &forward, trigger_definition->first_person_weapon_offset.x, &origin);
				point_from_line3d(&origin, &left, trigger_definition->first_person_weapon_offset.y, &origin);
				point_from_line3d(&origin, &up, trigger_definition->first_person_weapon_offset.z, &origin);
				target_object_index = player_aim_projectile(player_index, &origin, &forward);
			}
			else if (actor_index != NONE)
			{
				target_object_index = actor_aim_projectile(actor_index, &origin, &forward, &desired_error_angle);
			}
		}

		if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_uses_weapon_origin_bit))
		{
			origin = markers[marker_index].matrix.position;
		}

		if (trigger_index == _weapon_trigger_primary && weapon->weapon.alternate_shots_loaded > 0)
		{
			struct weapon_trigger_definition const *secondary_trigger_definition = TAG_BLOCK_GET_ELEMENT(
				&weapon_definition->weapon.triggers,
				_weapon_trigger_secondary,
				struct weapon_trigger_definition);

			projectile_definition_index = secondary_trigger_definition->projectile.index;
			projectile_count = weapon->weapon.alternate_shots_loaded;

			if (weapon_definition->weapon.secondary_trigger_mode == _weapon_secondary_trigger_loads_multiple_primary_ammunition)
			{
				projectile_count++;
			}

			projectile_count *= trigger_definition->projectiles_per_shot;
			weapon->weapon.alternate_shots_loaded = 0;
		}
		else
		{
			projectile_definition_index = trigger_definition->projectile.index;
			projectile_count = trigger_definition->projectiles_per_shot;
		}

		if (projectile_definition_index != NONE)
		{
			real_vector3d first_forward;
			short projectile_index;
			long projectile_owner_object_index = weapon_get_projectile_owner_object_index(weapon_index);

			for (projectile_index = 0; projectile_index < projectile_count; projectile_index++)
			{
				struct object_placement_data data;
				boolean force_inside_bsp;
				long projectile_object_index;
				boolean tracer = FALSE;

				object_placement_data_new(&data, trigger_definition->projectile.index, projectile_owner_object_index);
				data.position = origin;
				data.forward = forward;

				if (trigger->rate_of_fire == 0.f || trigger->sequential_non_tracer_rounds++ >= trigger_definition->rounds_between_tracers)
				{
					tracer = TRUE;
					trigger->sequential_non_tracer_rounds = 0;
				}

				if (desired_error_angle == 0.f)
				{
					real error = TEST_FLAG(trigger_definition->flags, _weapon_trigger_analog_rate_of_fire_bit) ? weapon->weapon.primary_trigger : trigger->error;

					desired_error_angle = (1.f - error) * trigger_definition->projectile_error_angle_lower_bound + error * trigger_definition->projectile_error_angle_upper_bound;
				}

				if (!TEST_FLAG(trigger_definition->flags, _weapon_trigger_use_error_when_unzoomed_bit) || !TEST_FLAG(weapon->weapon.control_flags, _weapon_control_zoomed_bit))
				{
					random_vector_in_cone3d(
						&data.forward,
						trigger_definition->projectile_error_inner_cone_angle,
						desired_error_angle,
						&data.forward);
				}

				if (!projectile_index)
				{
					first_forward = data.forward;
				}

				if (TEST_FLAG(trigger_definition->flags, _weapon_trigger_projectiles_have_identical_error_bit))
				{
					data.forward = first_forward;
				}

				normalize3d(perpendicular3d(&data.forward, &data.up));
				projectile_distribute(
					&data.forward,
					&data.up,
					trigger_definition->projectile_distribution_function,
					trigger_definition->projectile_distribution_angle,
					projectile_index,
					projectile_count);
				scale_vector3d(&data.forward, inherited_velocity_magnitude, &data.translational_velocity);

				force_inside_bsp = unit && unit->unit.player_index != NONE;

				if (force_inside_bsp)
				{
					SET_FLAG(data.flags, _new_object_never_automatically_delete_bit, TRUE);
				}

				projectile_object_index = object_new(&data);

				if (projectile_object_index != NONE)
				{
					if (force_inside_bsp)
					{
						real_point3d camera_position;

						unit_get_camera_position(owner_object_index, &camera_position);
						object_force_inside_bsp(projectile_object_index, &camera_position);
					}

					if (target_object_index != NONE)
					{
						projectile_set_target_object_index(projectile_object_index, target_object_index);
					}

					if (!tracer)
					{
						projectile_kill_tracer(projectile_object_index);
					}
				}
			}
		}
	}

	return;
}

static void projectile_distribute(
	real_vector3d *forward,
	real_vector3d *up,
	short distribution_function,
	real distribution_angle,
	short index,
	short count)
{
	real offset;
	real angle_cosine;
	real angle_sine;

	if (count & 1)
	{
		if (index == 0)
		{
			offset = 0.f;
		}
		else
		{
			short side_index = index - 1;

			side_index = side_index & 1 ? side_index >> 1 : -(side_index >> 1);
			offset = side_index;
		}
	}
	else
	{
		offset = (index >> 1) - 0.5f;

		if (index & 1)
		{
			offset = -offset;
		}
	}

	offset *= distribution_angle;
	angle_cosine = cosine(offset);
	angle_sine = sine(offset);

	switch (distribution_function)
	{
	case _trigger_distribution_horizontal_fan:
		rotate_vector_about_axis(forward, up, angle_sine, angle_cosine);
		break;
	}

	return;
}

static void weapon_trigger_start_ejection_port(
	long weapon_index,
	short trigger_index,
	boolean chamber)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct weapon_trigger *trigger = weapon_trigger_get(weapon, trigger_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		trigger_index,
		struct weapon_trigger_definition);

	if (trigger_definition->ejection_port_recovery_time > 0.f &&
		(TEST_FLAG(trigger_definition->flags, _weapon_trigger_ejection_port_during_chamber_animation_bit) && chamber ||
		!TEST_FLAG(trigger_definition->flags, _weapon_trigger_ejection_port_during_chamber_animation_bit) && !chamber))
	{
		trigger->ejection_port_position = 1.f;
	}

	return;
}

static void weapon_state_key_frame(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	switch (weapon->weapon.state)
	{
	case _weapon_state_primary_chamber:
		weapon_trigger_start_ejection_port(weapon_index, _weapon_trigger_primary, TRUE);
		break;

	case _weapon_state_secondary_chamber:
		weapon_trigger_start_ejection_port(weapon_index, _weapon_trigger_secondary, TRUE);
		break;
	}

	return;
}

static void weapon_state_next(
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);

	if (weapon->weapon.state < _weapon_state_primary_charged ||
		weapon->weapon.state > _weapon_state_secondary_charged && weapon->weapon.state != _weapon_state_put_away)
	{
		weapon_set_state(weapon_index, _weapon_state_idle, TRUE);
	}

	return;
}

static boolean weapon_magazine_state_interruptable(
	short old_state,
	short new_state)
{
	boolean interruptable = FALSE;

	switch (old_state)
	{
	case _magazine_idle:
	case _magazine_unchambered:
		interruptable = TRUE;
		break;
	}

	return interruptable;
}

static boolean weapon_state_interruptable(
	short old_state,
	short new_state)
{
	boolean interruptable = FALSE;

	switch (old_state)
	{
	case _weapon_state_idle:
		interruptable = TRUE;
		break;

	case _weapon_state_primary_recoil:
	case _weapon_state_secondary_recoil:
		interruptable = new_state >= old_state;
		break;
	}

	return interruptable;
}

static boolean weapon_set_state(
	long weapon_index,
	short new_state,
	boolean immediate)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	boolean state_set = FALSE;

	if (immediate || weapon_state_interruptable(weapon->weapon.state, new_state))
	{
		long owner_object_index;

		if (weapon_definition->object.animation_graph.index != NONE)
		{
			struct animation_graph const *animation_graph = animation_graph_definition_get(weapon_definition->object.animation_graph.index);
			struct animation_graph_weapon_animations const *weapon_animations = animation_graph->weapon_animations.count ? TAG_BLOCK_GET_ELEMENT(&animation_graph->weapon_animations, 0, struct animation_graph_weapon_animations) : NULL;

			if (weapon_animations)
			{
				short animation_type = NONE;

				switch (new_state)
				{
				case _weapon_state_idle:
					animation_type = _weapon_animation_idle;
					break;

				case _weapon_state_primary_recoil:
					animation_type = _weapon_animation_primary_recoil;
					break;

				case _weapon_state_secondary_recoil:
					animation_type = _weapon_animation_secondary_recoil;
					break;

				case _weapon_state_primary_chamber:
					animation_type = _weapon_animation_primary_chamber;
					break;

				case _weapon_state_secondary_chamber:
					animation_type = _weapon_animation_secondary_chamber;
					break;

				case _weapon_state_primary_reload:
				case _weapon_state_secondary_reload:
					animation_type = _weapon_animation_primary_reload;
					break;

				case _weapon_state_primary_charged:
				case _weapon_state_secondary_charged:
					animation_type = _weapon_animation_secondary_charged;
					break;

				case _weapon_state_ready:
					animation_type = _weapon_animation_ready;
					break;

				case _weapon_state_put_away:
					animation_type = _weapon_animation_put_away;
					break;
				}

				if (animation_type != NONE)
				{
					short animation_index = animation_type >= 0 && animation_type < weapon_animations->animations.count ? animation_graph_animation_index_get(&weapon_animations->animations)[animation_type].animation_index : NONE;

					if (animation_index != NONE || new_state == _weapon_state_idle)
					{
						weapon->object.animation.state.index = animation_choose_random_permutation(
							weapon_definition->object.animation_graph.index,
							animation_index);
						weapon->object.animation.state.frame_index = 0;
						weapon->weapon.state = (char)new_state;
					}
				}
			}
		}

		owner_object_index = weapon_get_owner_object_index(weapon_index);

		if (unit_try_and_get(owner_object_index))
		{
			unit_handle_weapon_state_change(owner_object_index, new_state);
		}

		state_set = TRUE;
	}

	return state_set;
}

void weapon_set_total_rounds(
	long weapon_index,
	short const *rounds_array)
{
	short magazine_index;
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);

	match_assert("c:\\halo\\SOURCE\\items\\weapons.c", 3082, rounds_array);

	for (magazine_index = 0; magazine_index < weapon_definition->weapon.magazines.count; magazine_index++)
	{
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, magazine_index);
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			magazine_index,
			struct weapon_magazine_definition);

		magazine->rounds_total = MIN(magazine_definition->rounds_total_maximum, rounds_array[magazine_index]);
		magazine->rounds_loaded = MIN(magazine->rounds_loaded, magazine->rounds_total);
	}

	return;
}

void weapon_set_current_amount(
	long weapon_index,
	real current_amount)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon->definition_index);
	boolean uses_age = FALSE;

	if (!weapon_definition->weapon.magazines.count)
	{
		uses_age = TRUE;
	}
	else
	{
		short trigger_index;

		for (trigger_index = 0; trigger_index < weapon_definition->weapon.triggers.count; trigger_index++)
		{
			struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
				&weapon_definition->weapon.triggers,
				trigger_index,
				struct weapon_trigger_definition);

			if (trigger_definition->age_generated_per_round > 0.f)
			{
				uses_age = TRUE;
				break;
			}
		}
	}

	current_amount = PIN(current_amount, 0.f, 1.f);

	if (uses_age)
	{
		weapon->weapon.age = 1.f - current_amount;
	}
	else if (weapon_definition->weapon.magazines.count > 0)
	{
		struct weapon_magazine_definition const *magazine_definition = TAG_BLOCK_GET_ELEMENT(
			&weapon_definition->weapon.magazines,
			0,
			struct weapon_magazine_definition);
		struct weapon_magazine *magazine = weapon_magazine_get(weapon, _weapon_magazine_primary);
		short rounds_loaded = (short)fast_ftol(magazine_definition->rounds_loaded_maximum * current_amount);

		magazine->rounds_total += rounds_loaded - magazine->rounds_loaded;
		magazine->rounds_loaded = rounds_loaded;
	}

	return;
}

/* ---------- private code */
