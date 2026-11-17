/*
ACTORS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "ai_globals.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "game_state.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "game_sound.h"
#include "bipeds.h"
#include "projectiles.h"
#include "equipment_definitions.h"

/* ---------- constants */

enum
{
	ACTOR_DECISION_HISTORY_COUNT = 5, /* fake name */
	MAXIMUM_ACTOR_DECISION_ITERATIONS = 10, /* fake name */
};

/* ---------- prototypes */

static void actor_swarm_component_refresh(long unit_index, long swarm_component_index);
static void actor_update(long actor_index);
static void actor_debug_preupdate(long actor_index);
static void actor_debug_postupdate(void);
static boolean actor_general_update(long actor_index);
static boolean actor_get_timeslice(long actor_index);
static void actor_clear_output(long actor_index);
static void actor_input_update(long actor_index);
static void actor_clear_orders(long actor_index);
static void actor_decision_loop(long actor_index);
static void actor_unit_control(long actor_index);

/* ---------- globals */

struct data_array *swarm_component_data;
struct data_array *swarm_data;
struct data_array *actor_data;

short const global_movement_animation_states[NUMBER_OF_ACTOR_MOVEMENT_TYPES] =
{
	_unit_animation_state_alert,
	_unit_animation_state_asleep,
	_unit_animation_state_in_combat,
	_unit_animation_state_flee,
	_unit_animation_state_flaming
};

long global_updating_actor_index = NONE;

/* ---------- public code */

void actors_initialize(
	void)
{
	actor_types_initialize();

	actor_data = game_state_data_new("actor", MAXIMUM_NUMBER_OF_ACTORS, sizeof(struct actor_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 121, actor_data);

	swarm_data = game_state_data_new("swarm", MAXIMUM_NUMBER_OF_ACTIVE_SWARMS, sizeof(struct swarm_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 124, swarm_data);

	swarm_component_data = game_state_data_new("swarm component", MAXIMUM_NUMBER_OF_SWARM_COMPONENTS, sizeof(struct swarm_component_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 127, swarm_component_data);

	return;
}

void actors_dispose(
	void)
{
	return;
}

void actors_initialize_for_new_map(
	void)
{
	data_make_valid(actor_data);
	data_make_valid(swarm_data);
	data_make_valid(swarm_component_data);

	return;
}

void actors_dispose_from_old_map(
	void)
{
	data_make_invalid(actor_data);
	data_make_invalid(swarm_data);
	data_make_invalid(swarm_component_data);

	return;
}

long actor_new(
	long variant_definition_index)
{
	long actor_index = NONE;

	if (variant_definition_index!=NONE)
	{
		struct actor_variant_definition *actor_variant = actor_variant_definition_get(variant_definition_index);
		long definition_index = actor_variant->actor_reference.index;

		if (definition_index!=NONE)
		{
			struct actor_definition *actor_definition = actor_definition_get(definition_index);

			actor_index = datum_new(actor_data);

			if (actor_index!=NONE)
			{
				struct actor_debug_info *debug_info;
				struct actor_datum *actor = actor_get(actor_index);

				actor->meta.swarm = TEST_FLAG(actor_definition->flags, _actor_definition_swarm_actor_bit);
				actor->meta.definition_index = definition_index;
				actor->meta.variant_definition_index = variant_definition_index;
				actor->meta.type = actor_definition->type;
				actor->meta.unit_index = NONE;
				actor->meta.unique_leader = FALSE;
				actor->meta.encounter_index = NONE;
				actor->meta.squad_index = NONE;
				actor->meta.platoon_index = NONE;
				actor->meta.encounterless = FALSE;
				actor->meta.disconnected_encounter_index = NONE;
				actor->meta.disconnected_squad_index = NONE;
				actor->meta.swarm_unit_count = 0;
				actor->meta.swarm_original_unit_count = 0;
				actor->meta.swarm_unit_index = NONE;
				actor->meta.swarm_cache_index = NONE;
				actor->meta.frozen = TRUE;
				actor->meta.active = FALSE;
				actor->meta.last_active_time = NONE;
				actor->meta.dormant = TRUE;
				actor->meta.dormant_desire = TRUE;
				actor->meta.service_timer = 0;
				actor->meta.first_prop_index = NONE;
				actor->meta.interesting_orphan_index = NONE;
				actor->firing_positions.current_position_index = NONE;

				actor->state.initial_state = NONE;
				actor->state.default_state = NONE;
				actor->state.last_default_state_time = NONE;
				actor->state.command_list_immediate = FALSE;
				actor->state.command_list_index = NONE;
				actor->state.last_command_list_time = NONE;
				actor->state.action = _actor_action_none;
				actor->state.mode = _actor_mode_alert;
				actor->state.combat_status = _actor_combat_status_none;
				actor->state.artificial_combat_status = _actor_combat_status_none;
				actor->state.suspicion_combat_status = _actor_combat_status_none;
				actor->state.uncertain_combat_timer = NONE;
				actor->state.searching = FALSE;
				actor->state.flying = TEST_FLAG(actor_definition->flags, _actor_definition_flying_bit);

				actor->input.pathfinding_surface_index = NONE;
				actor->input.vehicle_index = NONE;

				actor->external_orders.defending = FALSE;
				actor->external_orders.pursuit_is_coordinator = FALSE;
				actor->external_orders.pursuit_group_prop_index = NONE;
				actor->external_orders.desired_target_type = _desired_target_none;
				actor->external_orders.conversation_index = NONE;

				memset(&actor->emotions, 0, sizeof(actor->emotions));
				actor->emotions.last_active_cover_seeking_time = NONE;
				actor->emotions.last_melee_check_time = NONE;
				actor->emotions.last_melee_attack_time = NONE;
				actor->emotions.last_defensive_cover_seeking_time = NONE;
				actor->emotions.last_vehicle_check_time = NONE;
				actor->emotions.last_vehicle_charge_time = NONE;
				actor->emotions.last_flee_failed_time = NONE;
				actor->emotions.corpse_ignore_time = NONE;
				actor->emotions.unopposable_friend_ignore_time = NONE;
				actor->emotions.unopposable_retreat_prop_index = NONE;
				actor->emotions.unopposable_retreat_start_time = NONE;
				actor->emotions.original_body_vitality = 1.f;
				actor->emotions.vehicle_ignore_index = NONE;
				actor->emotions.vehicle_ignore_time = NONE;
				actor->emotions.flee_with_friends_disable_time = NONE;

				if (actor_definition->moving.glass_ignorance_chance>0.f)
				{
					actor->emotions.ignorant_of_broken_surfaces = real_random()<actor_definition->moving.glass_ignorance_chance;
				}

				actor->orders.look.primary_priority = _primary_priority_none;
				actor->orders.move.destination.destination_type = 0;

				actor->control.path.destination_orders.destination_type = 0;
				actor->control.path.destination_orders.ignore_target_object_index = NONE;
				actor->control.path.destination.surface_index = NONE;
				memset(&actor->control.path.path, 0, sizeof(actor->control.path.path));
				actor->control.moving = FALSE;
				actor->control.moving_forced_by_aiming = FALSE;
				actor->control.fire_state = _actor_fire_state_holding;
				actor->control.fire_state_timer = 0;
				actor->control.burst_disable_timer = 0;
				actor->control.trigger_delay_timer = 0;
				actor->control.blocked_communication_timer = 0;
				actor->control.current_fire_target_timer = 0;
				actor->control.current_fire_target_prop_index = NONE;
				actor->control.grenade_last_check_time = NONE;
				actor->control.grenade_current_prop_index = NONE;
				memset(actor->control.vector_avoidance_clear_times, NONE, sizeof(actor->control.vector_avoidance_clear_times));
				actor->control.vector_avoidance_current_direction = NONE;
				actor->control.vector_avoidance_sharp_turn_timer = NONE;
				actor->control.secondary_look_type = _secondary_look_none;
				actor->control.secondary_look_timer = 0;
				actor->control.desired_aiming_vector = *global_forward3d;
				actor->control.desired_facing_vector = *global_forward3d;
				actor->control.desired_looking_vector = *global_forward3d;
				actor->control.idle_vocalization_combat = FALSE;
				actor->control.idle_vocalization_timer = TICKS_PER_SECOND;

				actor->target.target_type = _actor_target_none;
				actor->target.target_prop_index = NONE;
				actor->target.target_last_visible_time = NONE;
				actor->target.since_any_target_visible_timer = NONE;

				actor_clear_discarded_firing_positions(actor_index, FALSE);
				actor->firing_positions.pursuit_prop_index = NONE;

				debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
				memset(debug_info, 0, sizeof(*debug_info));
				debug_info->last_path_refresh = NONE;
				debug_info->last_projectile_aiming_time = NONE;
				debug_info->last_vehicle_avoidance_time = NONE;
				debug_info->last_melee_time = NONE;
				debug_info->grenade_eval_time = NONE;
				debug_info->danger_avoidance_time = NONE;
				debug_info->charge_last_time = NONE;
				debug_info->field_19C = NONE;
				debug_info->vision_last_time = NONE;
				debug_info->num_debug_evaluations = NONE;

				actor_type_initialize(actor_index);
			}
		}
	}

	return actor_index;
}

long actor_create_for_unit(
	boolean swarm,
	long unit_index,
	long actor_variant_definition_index,
	long encounter_index,
	short squad_index,
	boolean allow_addition_to_other_squads,
	long disallow_actor_index,
	boolean initially_braindead,
	short initial_state,
	short default_state,
	short initial_command_list_index,
	char noncombat_sequence_id)
{
	long actor_index = NONE;

	if (unit_index!=NONE && actor_variant_definition_index!=NONE)
	{
		boolean valid = TRUE;

		if (swarm)
		{
			struct encounter_actor_iterator iterator;
			struct actor_datum *actor;

			encounter_actor_iterator_new(&iterator, encounter_index);

			while ((actor = encounter_actor_iterator_next(&iterator))!=NULL)
			{
				if (actor->meta.swarm &&
					iterator.index!=disallow_actor_index &&
					actor->meta.swarm_unit_count<MAXIMUM_NUMBER_OF_UNITS_PER_SWARM &&
					actor->meta.variant_definition_index==actor_variant_definition_index &&
					(allow_addition_to_other_squads || actor->meta.squad_index==squad_index))
				{
					actor_index = iterator.index;
					break;
				}
			}
		}
		else
		{
			struct biped_datum *biped = biped_try_and_get(unit_index);

			if (!biped || TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				valid = FALSE;
			}
		}

		if (valid)
		{
			if (actor_index==NONE)
			{
				actor_index = actor_new(actor_variant_definition_index);

				if (actor_index!=NONE)
				{
					struct actor_datum *actor = actor_get(actor_index);

					if (encounter_index==NONE)
					{
						encounterless_attach_actor(actor_index);
					}
					else
					{
						struct encounter_datum *encounter = encounter_get(encounter_index);

						if (!DATUM_INDEX_TO_IDENTIFIER(encounter_index))
						{
							encounter_index = DATUM_INDEX_NEW(DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index), encounter->identifier);
						}

						encounter_attach_actor(actor_index, encounter_index, squad_index, FALSE);
					}

					if (initially_braindead)
					{
						actor->state.mode = _actor_mode_braindead;

						if (actor->meta.active)
						{
							actor_set_dormant(actor_index, FALSE);
						}
					}
					else
					{
						actor->state.mode = _actor_mode_alert;
					}

					actor->state.initial_state = initial_state;
					actor->state.default_state = default_state;

					if (actor->state.default_state==NONE || actor->state.default_state==_actor_default_state_none)
					{
						actor->state.default_state = actor_action_get_default_state(initial_state);
					}

					actor->state.command_list_immediate = FALSE;
					actor->state.command_list_delay_timer = 2;
					actor->state.command_list_index = initial_command_list_index;
					actor->state.noncombat_sequence_id = noncombat_sequence_id;

					if (actor->meta.swarm!=actor_type_get_swarm(actor->meta.type))
					{
						error(
							_error_silent,
							"%s actor variant %s cannot have type %s (swarm flag does not match)",
							actor->meta.swarm ? "swarm" : "individual",
							tag_name_strip_path(tag_get_name(actor_variant_definition_index)),
							actor_type_get_name(actor->meta.type));
						actor_delete(actor_index, FALSE);
						actor_index = NONE;
					}
				}
			}
		}

		if (actor_index!=NONE)
		{
			if (swarm)
			{
				if (!actor_swarm_attach_unit(actor_index, unit_index))
				{
					if (actor_get(actor_index)->meta.swarm_unit_count==0)
					{
						actor_delete(actor_index, FALSE);
					}

					actor_index = NONE;
				}
			}
			else
			{
				actor_attach_unit(actor_index, unit_index);
			}

			actor_verify_activation(actor_index);
		}
	}

	return actor_index;
}

void actor_customize_unit(
	long actor_variant_definition_index,
	long unit_index)
{
	short i;
	struct actor_variant_definition *actor_variant = actor_variant_definition_get(actor_variant_definition_index);
	struct actor_definition *actor_definition = actor_definition_get(actor_variant->actor_reference.index);
	struct unit_datum *unit = unit_get(unit_index);

	if (actor_variant->unit.maximum_body_vitality>0.f || actor_variant->unit.maximum_shield_vitality>0.f)
	{
		object_initialize_vitality(unit_index, &actor_variant->unit.maximum_body_vitality, &actor_variant->unit.maximum_shield_vitality);
	}

	if (actor_variant->unit.forced_shader_permutation_index!=0)
	{
		unit->object.forced_shader_permutation_index = actor_variant->unit.forced_shader_permutation_index;
	}

	for (i = 0; i<actor_variant->change_colors.count; ++i)
	{
		struct actor_variant_change_colors *change_color = TAG_BLOCK_GET_ELEMENT(&actor_variant->change_colors, i, struct actor_variant_change_colors);

		if (i<NUMBER_OF_OBJECT_CHANGE_COLORS)
		{
			rgb_colors_interpolate(
				&unit->object.base_change_colors[i],
				_actor_variant_definition_color_interpolate_hsv_bit,
				&change_color->color_lower_bound,
				&change_color->color_upper_bound,
				real_random());
			unit->object.outgoing_change_colors[i] = unit->object.base_change_colors[i];
		}
	}

	if (actor_variant->ranged_combat.reference.index!=NONE)
	{
		struct object_placement_data placement_data;
		long weapon_index;

		object_placement_data_new(&placement_data, actor_variant->ranged_combat.reference.index, unit_index);
		weapon_index = object_new(&placement_data);

		if (weapon_index!=NONE && !unit_add_weapon_to_inventory(unit_index, weapon_index, _unit_add_weapon_as_only_weapon))
		{
			object_delete(weapon_index);
		}
	}

	if (actor_variant->grenade_combat.grenade_type!=NONE)
	{
		short grenade_count = random_range(actor_variant->items.grenades_lower_bound, actor_variant->items.grenades_upper_bound + 1);

		unit_add_grenade_type_to_inventory(unit_index, actor_variant->grenade_combat.grenade_type, grenade_count);
	}

	if (actor_variant->items.equipment_reference.index!=NONE)
	{
		struct equipment_definition *equipment_definition = equipment_definition_get(actor_variant->items.equipment_reference.index);

		if (equipment_definition->equipment.powerup_type==_equipment_powerup_none || equipment_definition->equipment.powerup_type==_equipment_powerup_grenade)
		{
			error(_error_silent, "cannot add grenades or non-powerups to an actor's inventory as equipment... try using the 'grenade' fields maybe?");
		}
		else
		{
			struct object_placement_data placement_data;
			long equipment_index;

			object_placement_data_new(&placement_data, actor_variant->items.equipment_reference.index, unit_index);
			equipment_index = object_new(&placement_data);

			if (equipment_index!=NONE && !unit_add_equipment_to_inventory(unit_index, equipment_index, 1))
			{
				object_delete(equipment_index);
			}
		}
	}

	if (TEST_FLAG(actor_variant->flags, _actor_variant_definition_active_camouflage_bit) || TEST_FLAG(actor_variant->flags, _actor_variant_definition_super_active_camouflage_bit))
	{
		if (TEST_FLAG(actor_variant->flags, _actor_variant_definition_super_active_camouflage_bit))
		{
			SET_FLAG(unit->unit.flags, _unit_super_camouflaged_bit, TRUE);
		}

		SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, TRUE);
		unit->unit.active_camouflage = 1.f;
		unit->unit.active_camouflage_super_amount = TEST_FLAG(actor_definition->flags, _actor_variant_definition_super_active_camouflage_bit) ? 1.f : 0.f;
	}

	return;
}

long actor_place(
	long actor_variant_definition_index,
	long encounter_index,
	long squad_index,
	struct actor_starting_location_definition *starting_location,
	boolean upgrade_major,
	short variant_number)
{
	struct actor_variant_definition *actor_variant;
	struct actor_definition *actor_definition;
	struct object_placement_data placement_data;
	long unit_index;
	long actor_index = NONE;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 603, starting_location);

	objects_garbage_collection();

	actor_variant = actor_variant_definition_get(actor_variant_definition_index);

	if (upgrade_major)
	{
		actor_variant_definition_index = actor_variant->major_upgrade_reference.index;
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 613, actor_variant_definition_index != NONE);
		actor_variant = actor_variant_definition_get(actor_variant_definition_index);
	}

	actor_definition = actor_definition_get(actor_variant->actor_reference.index);

	object_placement_data_new(&placement_data, actor_variant->unit_reference.index, NONE);
	placement_data.position = starting_location->position;
	vector3d_from_angle(&placement_data.forward, starting_location->facing);
	placement_data.variant_number = variant_number;

	unit_index = object_new(&placement_data);

	if (unit_index==NONE)
	{
		char aibuf[256];

		if (encounter_index==NONE)
		{
			strcpy(aibuf, "<encounterless>");
		}
		else
		{
			ai_index_to_string(AI_INDEX_NEW(_ai_index_squad, encounter_index, squad_index), global_scenario_get(), aibuf, sizeof(aibuf));
		}

		error(_error_silent, "WARNING: cannot create unit for actor %s %s", tag_name_strip_path(tag_get_name(actor_variant_definition_index)), aibuf);
	}

	if (unit_index!=NONE)
	{
		struct encounter_definition *encounter_definition = encounter_index==NONE ? NULL : TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index), struct encounter_definition);
		struct squad_definition *squad_definition = encounter_definition ? TAG_BLOCK_GET_ELEMENT(&encounter_definition->squads, squad_index, struct squad_definition) : NULL;
		boolean swarm = TEST_FLAG(actor_definition->flags, _actor_definition_swarm_actor_bit);
		boolean initially_braindead = FALSE;
		short initial_state = _actor_default_state_none;
		short default_state = _actor_default_state_none;
		short command_list_index = NONE;
		short noncombat_sequence_id = 0;

		actor_customize_unit(actor_variant_definition_index, unit_index);

		if (encounter_index!=NONE)
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index), struct encounter_definition);
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(&encounter_definition->squads, squad_index, struct squad_definition);

			initially_braindead = TEST_FLAG(encounter_definition->flags, _encounter_braindead_bit);
			initial_state = squad_definition->initial_state;
			default_state = squad_definition->default_state;
		}

		if (starting_location->initial_state>0)
		{
			initial_state = starting_location->initial_state;
		}

		if (starting_location->default_state>0)
		{
			default_state = starting_location->default_state;
		}

		command_list_index = starting_location->command_list_index;
		noncombat_sequence_id = starting_location->sequence_id;

		actor_index = actor_create_for_unit(
			swarm,
			unit_index,
			actor_variant_definition_index,
			encounter_index,
			(short)squad_index,
			FALSE,
			NONE,
			initially_braindead,
			initial_state,
			default_state,
			command_list_index,
			noncombat_sequence_id);

		if (actor_index==NONE)
		{
			char aibuf[256];

			if (encounter_index==NONE)
			{
				strcpy(aibuf, "<encounterless>");
			}
			else
			{
				ai_index_to_string(AI_INDEX_NEW(_ai_index_squad, encounter_index, squad_index), global_scenario_get(), aibuf, sizeof(aibuf));
			}

			error(_error_silent, "WARNING: cannot create actor %s %s", tag_name_strip_path(tag_get_name(actor_variant_definition_index)), aibuf);
		}
	}

	if (actor_index==NONE || unit_index==NONE)
	{
		if (unit_index!=NONE)
		{
			object_delete(unit_index);
			unit_index = NONE;
		}

		if (actor_index!=NONE)
		{
			actor_delete(actor_index, FALSE);
			actor_index = NONE;
		}
	}

	if (actor_index!=NONE)
	{
		actor_verify_activation(actor_index);
	}

	return actor_index;
}

short actors_spawn_from_unit(
	long unit_index,
	long actor_variant_definition_index,
	short count,
	real velocity)
{
	short spawned_count = 0;

	if (actor_variant_definition_index!=NONE && count>0)
	{
		short encounter_index;
		short squad_index;
		struct unit_datum *unit = unit_get(unit_index);
		long actor_index = unit->unit.swarm_actor_index==NONE ? unit->unit.actor_index : unit->unit.swarm_actor_index;

		if (actor_index!=NONE)
		{
			struct actor_datum *actor = actor_get(unit->unit.actor_index);

			encounter_index = (short)actor->meta.encounter_index;
			squad_index = actor->meta.squad_index;
		}
		else
		{
			encounter_index = unit->unit.fake_encounter_index;
			squad_index = unit->unit.fake_squad_index;
		}

		if (encounter_index!=NONE && squad_index!=NONE)
		{
			short i;
			struct actor_variant_definition *actor_variant = actor_variant_definition_get(actor_variant_definition_index);
			struct actor_definition *actor_definition = actor_definition_get(actor_variant->actor_reference.index);

			for (i = 0; i<count; ++i)
			{
				struct object_placement_data placement_data;
				long spawned_unit_index;
				real angle = real_random_range(0.f, 2.f * _pi);

				object_placement_data_new(&placement_data, actor_variant->unit_reference.index, NONE);
				vector3d_from_angle(&placement_data.forward, angle);
				object_get_origin(unit_index, &placement_data.position);
				placement_data.position.z += 0.3f;
				point_from_line3d(&placement_data.position, &placement_data.forward, 0.3f, &placement_data.position);

				spawned_unit_index = object_new(&placement_data);

				if (spawned_unit_index!=NONE)
				{
					long spawned_actor_index;
					struct unit_datum *spawned_unit = unit_get(spawned_unit_index);

					if (spawned_unit->object.type==_object_type_biped)
					{
						biped_fix_position(spawned_unit_index, NONE, &placement_data.position, NULL, 1.f, TRUE, FALSE, FALSE);
					}

					actor_customize_unit(actor_variant_definition_index, spawned_unit_index);

					spawned_actor_index = actor_create_for_unit(
						TEST_FLAG(actor_definition->flags, _actor_definition_swarm_actor_bit),
						spawned_unit_index,
						actor_variant_definition_index,
						encounter_index,
						squad_index,
						FALSE,
						NONE,
						FALSE,
						_actor_default_state_alert,
						_actor_default_state_none,
						NONE,
						0);

					if (spawned_actor_index==NONE)
					{
						error(_error_silent, "WARNING: cannot create actor to be spawned from unit");
						object_delete(spawned_unit_index);
					}
					else
					{
						actor_verify_activation(spawned_actor_index);

						if (velocity>0.f)
						{
							real_vector3d acceleration;
							real horizontal_scale = real_random_range(0.5f, 1.f);
							real vertical_scale = real_random_range(0.8f, 1.5f);

							acceleration.i = horizontal_scale * placement_data.forward.i;
							acceleration.j = horizontal_scale * placement_data.forward.j;
							acceleration.k = vertical_scale;
							scale_vector3d(&acceleration, velocity, &acceleration);

							if (spawned_unit->object.type==_object_type_biped)
							{
								biped_accelerate(spawned_unit_index, &acceleration);
							}
						}

						++spawned_count;
					}
				}
			}
		}
	}

	return spawned_count;
}

void actor_attach_unit(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.actor_index==actor_index)
	{
	}
	else
	{
		if (unit->unit.swarm_actor_index!=NONE)
		{
			actor_swarm_detach_from_unit(unit->unit.swarm_actor_index, unit_index);
		}

		if (unit->unit.actor_index!=NONE)
		{
			actor_delete(unit->unit.actor_index, FALSE);
		}

		if (actor->meta.unit_index!=NONE)
		{
			actor_detach_from_unit(actor_index);
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 868, !actor->meta.swarm);
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 869, actor->meta.unit_index == NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 870, unit->unit.actor_index == NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 871, unit->unit.swarm_actor_index == NONE);

		actor->meta.unit_index = unit_index;
		unit->unit.actor_index = actor_index;

		if (actor->meta.encounter_index!=NONE)
		{
			struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

			encounter_attach_unit(actor->meta.encounter_index, unit_index);
			unit->object.owner_team_index = encounter->team_index;
		}

		actor->meta.team_index = unit->object.owner_team_index;

		if (unit->object.variant_number>=100)
		{
			actor->meta.unique_leader = TRUE;

			if (actor->meta.encounter_index!=NONE)
			{
				struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

				++encounter->unique_leader_count;
			}
		}

		actor_input_update(actor_index);

		object_set_automatic_deactivation(unit_index, FALSE);

		if (actor->meta.dormant)
		{
			object_deactivate(unit_index);
		}
		else
		{
			object_activate(unit_index);
		}

		unit_set_actively_controlled(unit_index, TRUE);
	}

	actor_verify_activation(actor_index);

	return;
}

void actor_set_team(
	long actor_index,
	short team_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm)
	{
		if (actor->meta.swarm_cache_index!=NONE)
		{
			short i;
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

			for (i = 0; i<swarm->unit_count; ++i)
			{
				struct unit_datum *unit = unit_get(swarm->unit_indices[i]);

				unit->object.owner_team_index = team_index;
			}
		}
		else
		{
			long unit_index;

			for (unit_index = actor->meta.swarm_unit_index; unit_index!=NONE; )
			{
				struct unit_datum *unit = unit_get(unit_index);

				unit->object.owner_team_index = team_index;
				unit_index = unit->unit.swarm_next_unit_index;
			}
		}
	}
	else
	{
		if (actor->meta.unit_index!=NONE)
		{
			struct unit_datum *unit = unit_get(actor->meta.unit_index);

			unit->object.owner_team_index = team_index;
		}
	}

	return;
}

boolean actor_set_active(
	long actor_index,
	boolean active)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean success = TRUE;

	actor_verify_activation(actor_index);

	if (actor->meta.active!=active)
	{
		if (active)
		{
			if (actor->meta.swarm)
			{
				actor_swarm_cache_new(actor_index);

				if (actor->meta.swarm_cache_index==NONE)
				{
					error(_error_silent, "swarm tried to become active but couldn't, erasing %d units", actor->meta.swarm_unit_count);
					actor->meta.erase_next_tick = TRUE;
					success = FALSE;
				}
			}
		}
		else
		{
			actor_delete_props(actor_index);
			actor_swarm_cache_delete(actor_index);
		}

		if (success)
		{
			if (active)
			{
				actor->meta.active = TRUE;

				if (actor->state.mode==_actor_mode_braindead)
				{
					actor_set_dormant(actor_index, FALSE);
				}
			}
			else
			{
				actor_set_dormant(actor_index, TRUE);
				actor->meta.active = FALSE;
				actor->meta.last_active_time = game_time_get();
			}
		}
	}

	actor_verify_activation(actor_index);

	return success;
}

void actor_set_dormant(
	long actor_index,
	boolean dormant)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.active && actor->meta.dormant!=dormant)
	{
		if (actor->meta.swarm)
		{
			if (actor->meta.swarm_cache_index!=NONE)
			{
				short i;
				struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

				for (i = 0; i<swarm->unit_count; ++i)
				{
					if (dormant)
					{
						object_deactivate(swarm->unit_indices[i]);
					}
					else
					{
						object_activate(swarm->unit_indices[i]);
					}
				}
			}
			else
			{
				long unit_index;

				for (unit_index = actor->meta.swarm_unit_index; unit_index!=NONE; )
				{
					struct unit_datum *unit = unit_get(unit_index);

					if (dormant)
					{
						object_deactivate(unit_index);
					}
					else
					{
						object_activate(unit_index);
					}

					unit_index = unit->unit.swarm_next_unit_index;
				}
			}
		}
		else
		{
			if (actor->meta.unit_index!=NONE)
			{
				if (dormant)
				{
					object_deactivate(actor->meta.unit_index);
				}
				else
				{
					object_activate(actor->meta.unit_index);
				}
			}
		}

		actor->meta.dormant = dormant;

		if (!dormant)
		{
			actor->meta.become_dormant_timer = 0;
		}
	}

	actor_verify_activation(actor_index);

	return;
}

real_argb_color const *actor_activation_debug_color(
	long actor_index)
{
	real_argb_color const *color;
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.active)
	{
		if (actor->meta.dormant)
		{
			color = global_real_argb_blue;
		}
		else if (actor->meta.dormant_desire)
		{
			if (actor->meta.become_dormant_timer>0)
			{
				color = global_real_argb_green;
			}
			else
			{
				color = global_real_argb_yellow;
			}
		}
		else
		{
			color = global_real_argb_white;
		}
	}
	else
	{
		short structure_bsp_index = NONE;

		if (actor->meta.encounter_index!=NONE)
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index), struct encounter_definition);

			structure_bsp_index = encounter_definition->runtime_structure_bsp_reference_index;
		}

		if (structure_bsp_index==NONE || structure_bsp_index==global_structure_bsp_index)
		{
			color = global_real_argb_purple;
		}
		else
		{
			color = global_real_argb_red;
		}
	}

	return color;
}

static void actor_verify_unit_activation(
	long actor_index,
	long unit_index,
	char const *description)
{
	struct object_header_datum *header = object_header_get(unit_index);
	struct unit_datum *unit = unit_get(unit_index);
	struct actor_datum *actor = actor_get(actor_index);

	if (unit->object.parent_object_index!=NONE)
	{
	}
	else if (unit->unit.last_vehicle_index + TICKS_PER_SECOND>=game_time_get())
	{
	}
	else if (actor->meta.dormant!=!TEST_FLAG(header->flags, _object_header_active_bit))
	{
		error(_error_silent, "%s unit activation logic error", description);
	}

	return;
}

void actor_verify_activation(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (!actor->meta.active && !actor->meta.dormant)
	{
		error(_error_silent, "actor dormancy logic error");
	}

	if (actor->meta.swarm)
	{
		if (actor->meta.swarm_cache_index!=NONE)
		{
			short i;
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

			for (i = 0; i<swarm->unit_count; ++i)
			{
				actor_verify_unit_activation(actor_index, swarm->unit_indices[i], "active swarm");
			}
		}
		else
		{
			long unit_index;

			for (unit_index = actor->meta.swarm_unit_index; unit_index!=NONE; )
			{
				struct unit_datum *unit = unit_get(unit_index);

				actor_verify_unit_activation(actor_index, unit_index, "inactive swarm");
				unit_index = unit->unit.swarm_next_unit_index;
			}
		}
	}
	else
	{
		if (actor->meta.unit_index!=NONE)
		{
			actor_verify_unit_activation(actor_index, actor->meta.unit_index, "individual");
		}
	}

	return;
}

static void actor_swarm_component_setup(
	long swarm_index,
	long unit_index,
	long swarm_component_index)
{
	struct swarm_datum *swarm = swarm_get(swarm_index);
	struct swarm_component_datum *swarm_component = swarm_component_get(swarm_component_index);

	swarm_component->combat_target_prop_index = NONE;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1244, swarm->unit_count < MAXIMUM_NUMBER_OF_UNITS_PER_SWARM);

	swarm->unit_indices[swarm->unit_count] = unit_index;
	swarm->component_indices[swarm->unit_count++] = swarm_component_index;

	actor_swarm_component_refresh(unit_index, swarm_component_index);

	return;
}

boolean actor_swarm_attach_unit(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(unit_index);
	boolean success = TRUE;

	if (unit->unit.swarm_actor_index==actor_index)
	{
	}
	else
	{
		long swarm_component_index = NONE;

		if (actor->meta.swarm_cache_index!=NONE)
		{
			swarm_component_index = datum_new(swarm_component_data);
			success = swarm_component_index!=NONE;
		}

		if (!success)
		{
			error(_error_silent, "unable to create any more swarm components (max %d)", MAXIMUM_NUMBER_OF_SWARM_COMPONENTS);
		}
		else
		{
			if (unit->unit.swarm_actor_index!=NONE)
			{
				actor_swarm_detach_from_unit(unit->unit.swarm_actor_index, unit_index);
			}

			if (unit->unit.actor_index!=NONE)
			{
				actor_delete(unit->unit.actor_index, FALSE);
			}

			if (actor->meta.unit_index!=NONE)
			{
				actor_detach_from_unit(actor_index);
			}

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1299, actor->meta.swarm);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1300, actor->meta.unit_index == NONE);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1301, unit->unit.actor_index == NONE);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1302, unit->unit.swarm_actor_index == NONE);

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1305, actor->meta.swarm_unit_count < MAXIMUM_NUMBER_OF_UNITS_PER_SWARM);

			unit->unit.swarm_actor_index = actor_index;
			unit->unit.swarm_next_unit_index = actor->meta.swarm_unit_index;
			unit->unit.swarm_prev_unit_index = NONE;

			if (actor->meta.swarm_unit_index!=NONE)
			{
				struct unit_datum *swarm_first_unit = unit_get(actor->meta.swarm_unit_index);

				match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1316, swarm_first_unit->unit.swarm_prev_unit_index == NONE);
				swarm_first_unit->unit.swarm_prev_unit_index = unit_index;
			}

			actor->meta.swarm_unit_index = unit_index;

			if (actor->meta.swarm_cache_index!=NONE)
			{
				actor_swarm_component_setup(actor->meta.swarm_cache_index, unit_index, swarm_component_index);
			}

			++actor->meta.swarm_unit_count;
			++actor->meta.swarm_original_unit_count;

			if (actor->meta.encounter_index!=NONE)
			{
				struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

				encounter_attach_unit(actor->meta.encounter_index, unit_index);
				unit->object.owner_team_index = encounter->team_index;
			}

			actor->meta.team_index = unit->object.owner_team_index;

			object_set_automatic_deactivation(unit_index, FALSE);

			if (actor->meta.dormant)
			{
				object_deactivate(unit_index);
			}
			else
			{
				object_activate(unit_index);
			}

			unit_set_actively_controlled(unit_index, TRUE);
		}
	}

	return success;
}

void actor_detach_from_unit(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		object_set_automatic_deactivation(actor->meta.unit_index, TRUE);
		unit_set_actively_controlled(actor->meta.unit_index, FALSE);

		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1374, unit->unit.actor_index == actor_index);
		unit->unit.actor_index = NONE;

		if (actor->meta.unique_leader && actor->meta.encounter_index!=NONE)
		{
			struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1381, encounter->unique_leader_count > 0);
			--encounter->unique_leader_count;
		}

		actor->meta.unit_index = NONE;
		actor->meta.unique_leader = FALSE;
	}

	return;
}

void actor_swarm_detach_from_unit(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.swarm_actor_index==actor_index)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1401, actor->meta.swarm_unit_count > 0);

		object_set_automatic_deactivation(unit_index, TRUE);
		unit_set_actively_controlled(unit_index, FALSE);

		if (actor->meta.swarm_cache_index!=NONE)
		{
			short i;
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
			boolean found = FALSE;

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1413, swarm->actor_index == actor_index);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1414, swarm->unit_count == actor->meta.swarm_unit_count);

			for (i = 0; i<swarm->unit_count; ++i)
			{
				if (swarm->unit_indices[i]==unit_index)
				{
					long swarm_component_index = swarm->component_indices[i];

					--swarm->unit_count;

					if (i<swarm->unit_count)
					{
						swarm->unit_indices[i] = swarm->unit_indices[swarm->unit_count];
						swarm->component_indices[i] = swarm->component_indices[swarm->unit_count];
					}

					datum_delete(swarm_component_data, swarm_component_index);
					found = TRUE;
					break;
				}
			}

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1436, found);
		}

		if (unit->unit.swarm_prev_unit_index!=NONE)
		{
			unit_get(unit->unit.swarm_prev_unit_index)->unit.swarm_next_unit_index = unit->unit.swarm_next_unit_index;
		}
		else
		{
			actor->meta.swarm_unit_index = unit->unit.swarm_next_unit_index;
		}

		if (unit->unit.swarm_next_unit_index!=NONE)
		{
			unit_get(unit->unit.swarm_next_unit_index)->unit.swarm_prev_unit_index = unit->unit.swarm_prev_unit_index;
		}

		unit->unit.swarm_actor_index = NONE;
		--actor->meta.swarm_unit_count;
	}

	return;
}

void actor_delete_props(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	while (actor->meta.first_prop_index!=NONE)
	{
		actor_switch_props(actor_index, actor->meta.first_prop_index, NONE);
		prop_delete(actor_index, actor->meta.first_prop_index);
	}

	return;
}

void actor_delete(
	long actor_index,
	boolean died)
{
	struct data_iterator iterator;
	struct prop_datum *prop;
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1488, actor_index != global_updating_actor_index);

#ifdef DEBUG
	if (ai_debug.selected_actor_index==actor_index)
	{
		ai_debug.selected_actor_index = NONE;
	}

	if (ai_debug.aiming_validity_actor_index==actor_index)
	{
		ai_debug.aiming_validity_actor_index = NONE;
	}
#endif

	if (actor->meta.encounterless)
	{
		encounterless_detach_actor(actor_index);
	}
	else
	{
		encounter_detach_actor(actor_index, died);
	}

	if (actor->meta.swarm)
	{
		actor_swarm_cache_delete(actor_index);

		while (actor->meta.swarm_unit_index!=NONE)
		{
			actor_swarm_detach_from_unit(actor_index, actor->meta.swarm_unit_index);
		}
	}
	else
	{
		actor_detach_from_unit(actor_index);
	}

	actor_delete_props(actor_index);

	data_iterator_new(&iterator, prop_data);

	while ((prop = (struct prop_datum *)data_iterator_next(&iterator))!=NULL)
	{
		if (prop->actor_index==actor_index)
		{
			prop->actor_index = NONE;
		}
	}

	ai_debug_actor_deleted(actor_index);
	ai_conversation_actor_deleted(actor_index);
	datum_delete(actor_data, actor_index);

	return;
}

long actor_swarm_cache_new(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm_cache_index==NONE)
	{
		actor->meta.swarm_cache_index = datum_new(swarm_data);

		if (actor->meta.swarm_cache_index==NONE)
		{
			error(_error_silent, "exceeded MAXIMUM_NUMBER_OF_ACTIVE_SWARMS (%d)", MAXIMUM_NUMBER_OF_ACTIVE_SWARMS);
		}
		else
		{
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
			long unit_index = actor->meta.swarm_unit_index;

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1570, actor->meta.swarm_unit_count <= MAXIMUM_NUMBER_OF_UNITS_PER_SWARM);

			swarm->actor_index = actor_index;
			swarm->unit_count = 0;

			while (unit_index!=NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);
				long swarm_component_index = datum_new(swarm_component_data);

				if (swarm_component_index==NONE)
				{
					error(_error_silent, "unable to create any more swarm components (max %d)", MAXIMUM_NUMBER_OF_SWARM_COMPONENTS);
					break;
				}

				actor_swarm_component_setup(actor->meta.swarm_cache_index, unit_index, swarm_component_index);
				unit_index = unit->unit.swarm_next_unit_index;
			}
		}
	}

	return actor->meta.swarm_cache_index;
}

void actor_swarm_cache_delete(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm_cache_index!=NONE)
	{
		short i;
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

		for (i = 0; i<swarm->unit_count; ++i)
		{
			datum_delete(swarm_component_data, swarm->component_indices[i]);
		}

		datum_delete(swarm_data, actor->meta.swarm_cache_index);
		actor->meta.swarm_cache_index = NONE;
	}

	return;
}

static void actor_swarm_component_refresh(
	long unit_index,
	long swarm_component_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct swarm_component_datum *swarm_component = swarm_component_get(swarm_component_index);
	long surface_index = NONE;

	if (unit->object.type==_object_type_biped)
	{
		surface_index = ((struct biped_datum *)unit)->biped.support_surface_index;
	}

	object_get_origin(unit_index, &swarm_component->position);
	swarm_component->surface_index = surface_index;

	return;
}

boolean actor_is_noncombat(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	return (boolean)(actor->state.mode<_actor_mode_combat);
}

boolean actor_in_combat(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	return (boolean)(actor->state.mode==_actor_mode_combat && actor->state.combat_status>actor->state.artificial_combat_status);
}

boolean actor_is_fighting(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean fighting = actor->state.combat_status>=_actor_combat_status_visible;

	if (fighting && actor->state.action==_actor_action_flee && actor->state.action_data.flee.panic_type>_actor_panic_none)
	{
		fighting = FALSE;
	}

	return fighting;
}

boolean actor_attacking_target(
	long actor_index,
	real_vector3d *attack_vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean attacking = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1688, attack_vector);

	if (actor->target.target_type>_actor_target_clear_line_of_sight_enemy && actor_action_class(actor_index)==_action_class_active)
	{
		if (actor->control.grenade_trying_to_throw)
		{
			attacking = TRUE;
			*attack_vector = actor->input.aiming_vector;
		}
		else if (actor->control.current_fire_target_type>_actor_fire_target_none)
		{
			attacking = TRUE;
			*attack_vector = actor->control.current_fire_target_aim_vector;
		}
	}

	return attacking;
}

boolean actor_is_leaping(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean leaping = FALSE;

	if (actor->state.action==_actor_action_charge)
	{
		leaping = action_charge_is_leaping(actor_index);
	}

	return leaping;
}

long actor_get_weapon(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	long weapon_index = NONE;

	if (actor->input.vehicle_gunner && actor->input.vehicle_index!=NONE)
	{
		weapon_index = unit_inventory_get_weapon(actor->input.vehicle_index, unit_get(actor->input.vehicle_index)->unit.current_weapon_index);
	}

	if (weapon_index==NONE &&
		actor->meta.unit_index!=NONE &&
		!TEST_FLAG(actor_variant_definition_get(actor->meta.variant_definition_index)->flags, _actor_variant_definition_cannot_use_ranged_weapons_bit))
	{
		weapon_index = unit_inventory_get_weapon(actor->meta.unit_index, unit_get(actor->meta.unit_index)->unit.current_weapon_index);
	}

	return weapon_index;
}

boolean actor_has_ranged_weapon(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	long weapon_index = actor_get_weapon(actor_index);
	boolean has_ranged_weapon = weapon_index!=NONE;

	if (has_ranged_weapon && actor->meta.unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		if (TEST_FLAG(unit->object.damage_flags, _object_ranged_attack_inhibited_bit))
		{
			has_ranged_weapon = FALSE;
		}
	}

	return has_ranged_weapon;
}

long actor_target_unit_index(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	long unit_index = NONE;

	if (actor->target.target_prop_index!=NONE)
	{
		unit_index = prop_get(actor->target.target_prop_index)->unit_index;
	}

	return unit_index;
}

void actor_derive_target_information(
	long actor_index,
	long friend_actor_index)
{
	struct actor_datum *friend_actor = actor_get(friend_actor_index);
	long friend_prop_index = friend_actor->target.target_prop_index;

	if (friend_prop_index!=NONE)
	{
		struct prop_datum *friend_prop = prop_get(friend_prop_index);

		actor_perception_create_orphan_from_friend(actor_index, friend_prop->unit_index, friend_actor_index, friend_prop_index);
	}

	return;
}

void actor_switch_props(
	long actor_index,
	long prop_index,
	long replacement_prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->target.target_prop_index==prop_index)
	{
		actor->target.target_prop_index = replacement_prop_index;

		if (replacement_prop_index==NONE)
		{
			actor->target.target_type = _actor_target_none;
		}
	}

	if (actor->control.current_fire_target_type==_actor_fire_target_prop && actor->control.current_fire_target_prop_index==prop_index)
	{
		actor->control.current_fire_target_prop_index = replacement_prop_index;

		if (replacement_prop_index==NONE)
		{
			actor->control.current_fire_target_type = _actor_fire_target_none;
		}
	}

	if (actor->control.grenade_current_prop_index==prop_index)
	{
		actor->control.grenade_current_prop_index = replacement_prop_index;
	}

	if (actor->stimuli.surprise_prop_index==prop_index)
	{
		actor->stimuli.surprise_prop_index = replacement_prop_index;
	}

	if (actor->stimuli.panic_prop_index==prop_index)
	{
		actor->stimuli.panic_prop_index = replacement_prop_index;
	}

	if (actor->stimuli.combat_transition_prop_index==prop_index)
	{
		actor->stimuli.combat_transition_prop_index = replacement_prop_index;
	}

	if (actor->emotions.unopposable_retreat_prop_index==prop_index)
	{
		if (replacement_prop_index==NONE)
		{
			actor->emotions.unopposable_retreat_timer = 0;
		}

		actor->emotions.unopposable_retreat_prop_index = replacement_prop_index;
	}

	if (actor->external_orders.pursuit_group_prop_index==prop_index)
	{
		actor->external_orders.pursuit_group_prop_index = replacement_prop_index;
	}

	if (actor->external_orders.postcombat_prop_index==prop_index)
	{
		actor->external_orders.postcombat_prop_index = replacement_prop_index;

		if (replacement_prop_index==NONE)
		{
			actor->external_orders.postcombat_type = _actor_postcombat_none;
		}
	}

	if (actor->control.path.destination_orders.destination_type==_destination_prop && actor->control.path.destination_orders.prop.prop_index==prop_index)
	{
		if (replacement_prop_index==NONE)
		{
			actor->control.path.destination_orders.destination_type = _destination_none;
			actor->control.path.destination_orders.ignore_target_object_index = NONE;
		}
		else
		{
			actor->control.path.destination_orders.prop.prop_index = replacement_prop_index;
		}
	}

	if (actor->control.secondary_look_direction.type==_direction_specification_prop && actor->control.secondary_look_direction.prop_index==prop_index)
	{
		actor->control.secondary_look_direction.prop_index = replacement_prop_index;
	}

	if (actor->control.idle_major_direction.type==_direction_specification_prop && actor->control.idle_major_direction.prop_index==prop_index)
	{
		actor->control.idle_major_direction.prop_index = replacement_prop_index;
	}

	if (actor->control.idle_minor_direction.type==_direction_specification_prop && actor->control.idle_minor_direction.prop_index==prop_index)
	{
		actor->control.idle_minor_direction.prop_index = replacement_prop_index;
	}

	if (actor->meta.swarm && actor->meta.swarm_cache_index!=NONE)
	{
		short i;
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

		for (i = 0; i<swarm->unit_count; ++i)
		{
			struct swarm_component_datum *swarm_component = swarm_component_get(swarm->component_indices[i]);

			if (swarm_component->combat_target_prop_index==prop_index)
			{
				swarm_component->combat_target_prop_index = replacement_prop_index;
			}
		}
	}

	actor_action_replace_prop(actor_index, prop_index, replacement_prop_index);

	return;
}

void actor_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->firing_positions.current_position_index = NONE;

	if (actor->control.path.destination_orders.destination_type==_destination_firing_position ||
		actor->control.path.destination_orders.destination_type==_destination_move_position)
	{
		actor->control.path.destination_orders.destination_type = _destination_none;
		actor->control.path.destination_orders.ignore_target_object_index = NONE;
	}

	actor_action_flush_position_indices(actor_index);

	return;
}

void actor_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->input.position.body_location.cluster_index = NONE;
	actor->input.position.body_location.leaf_index = NONE;
	actor->input.pathfinding_surface_index = NONE;
	actor->stimuli.combat_transition_guard_point_surface_index = NONE;

	if (actor->orders.move.destination.destination_type==_destination_raw_location)
	{
		actor->orders.move.destination.raw.surface_index = NONE;
	}

	if (actor->control.path.destination_orders.destination_type==_destination_raw_location)
	{
		actor->control.path.destination_orders.raw.surface_index = NONE;
	}

	actor->control.path.destination.surface_index = NONE;

	if (actor->meta.swarm && actor->meta.swarm_cache_index!=NONE)
	{
		short i;
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

		for (i = 0; i<swarm->unit_count; ++i)
		{
			struct swarm_component_datum *swarm_component = swarm_component_get(swarm->component_indices[i]);

			swarm_component->surface_index = NONE;
		}
	}

	actor_action_flush_structure_indices(actor_index);

	return;
}

boolean actor_get_running_blind_vector(
	long actor_index,
	real_vector3d *run_vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1966, run_vector);

	if (actor->meta.swarm)
	{
	}
	else
	{
		if (actor->control.moving)
		{
			*run_vector = actor->control.moving_towards_vector;
			success = TRUE;
		}
		else if (actor->control.path.path.valid)
		{
			vector_from_points3d(&actor->input.position.body_position, &actor->control.path.destination.point, run_vector);
			success = TRUE;
		}
	}

	if (success)
	{
		if (normalize3d(run_vector)==0.f)
		{
			success = FALSE;
		}
	}

	return success;
}

boolean actor_has_unlimited_grenades(
	long actor_index)
{
	return TRUE;
}

void actor_handle_damage(
	long actor_index,
	long aggressor_unit_index,
	real damage_fraction,
	real_vector3d *damage_velocity)
{
	if (aggressor_unit_index!=NONE)
	{
		long prop_index = prop_get_base_by_unit_index(actor_index, aggressor_unit_index, TRUE, TRUE);

		if (prop_index!=NONE)
		{
			struct prop_datum *prop = prop_get(prop_index);

			prop->damage_inflicted_on_me += damage_fraction;
			prop->ticks_since_damage = 0;
			prop->currently_damaging_me = TRUE;

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 2038, !prop_orphaned(prop));

			if (prop->orphan_prop_index!=NONE)
			{
				struct prop_datum *orphan_prop = prop_get(prop->orphan_prop_index);

				orphan_prop->damage_inflicted_on_me += damage_fraction;
				orphan_prop->ticks_since_damage = 0;
				orphan_prop->currently_damaging_me = TRUE;
			}

			if (prop->state<_prop_state_becoming_unacknowledged || prop->state>_prop_state_acknowledged)
			{
				prop_index = NONE;
			}
		}

		actor_stimulus_damage(actor_index, prop_index, damage_fraction, damage_velocity);
	}

	return;
}

void actor_erase(
	long actor_index,
	boolean immediate)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm)
	{
		while (actor->meta.swarm_unit_index!=NONE)
		{
			long unit_index = actor->meta.swarm_unit_index;

			actor_swarm_detach_from_unit(actor_index, unit_index);

			if (immediate)
			{
				object_delete_immediately(unit_index);
			}
			else
			{
				object_delete(unit_index);
			}
		}

		actor_delete(actor_index, TRUE);
	}
	else
	{
		long unit_index = actor->meta.unit_index;

		actor_died(actor_index);

		if (immediate)
		{
			object_delete_immediately(unit_index);
		}
		else
		{
			object_delete(unit_index);
		}
	}

	return;
}

void actor_kill(
	long actor_index,
	boolean silent,
	boolean delayed)
{
	struct actor_datum *actor = actor_get(actor_index);
	long encounter_index = actor->meta.encounter_index;

	if (actor->meta.swarm)
	{
		long unit_index;

		for (unit_index = actor->meta.swarm_unit_index; unit_index!=NONE; )
		{
			struct unit_datum *unit = unit_get(unit_index);

			if (silent)
			{
				SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_silent_bit, TRUE);
			}
			else
			{
				SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_bit, TRUE);
			}

			if (!delayed)
			{
				actor_swarm_detach_from_unit(actor_index, unit_index);
			}

			unit_index = unit->unit.swarm_next_unit_index;
		}
	}
	else
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		if (silent)
		{
			SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_silent_bit, TRUE);
		}
		else
		{
			SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_bit, TRUE);
		}

		if (!delayed)
		{
			actor_detach_from_unit(actor_index);
		}
	}

	if (!delayed)
	{
		actor_delete(actor_index, TRUE);

		if (encounter_index!=NONE)
		{
			encounter_update_status(encounter_index);
		}
	}

	return;
}

void actor_died(
	long actor_index)
{
	struct unit_datum *unit;
	real random;
	long weapon_index;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant = actor_variant_definition_get(actor->meta.variant_definition_index);
	long encounter_index = actor->meta.encounter_index;

	if (actor->state.mode==_actor_mode_combat && actor->state.combat_status>=_actor_combat_status_investigate)
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		if (unit_controllable(actor->meta.unit_index) &&
			unit_inventory_get_weapon(actor->meta.unit_index, unit_get(actor->meta.unit_index)->unit.current_weapon_index)!=NONE &&
			unit->unit.weapon_drop_delay_ticks>0)
		{
			real wildfire_chance = PIN(actor_variant->ranged_combat.death_wildfire_chance, 0.1f, 0.6f);

			if (actor->emotions.berserk || (actor->control.current_fire_target_type>_actor_fire_target_none && actor->control.current_fire_target_distance<3.f))
			{
				real close_wildfire_chance = MIN(4.f * wildfire_chance, 0.6f);

				wildfire_chance = MAX(wildfire_chance, close_wildfire_chance);
			}

			if (wildfire_chance>real_random())
			{
				real wildfire_time;
				short wildfire_ticks;

				if (actor_variant->ranged_combat.death_wildfire_time==0.f)
				{
					wildfire_time = real_random_range(0.8f, 1.3f);
				}
				else
				{
					wildfire_time = PIN(actor_variant->ranged_combat.death_wildfire_time, 0.8f, 1.3f);
				}

				wildfire_ticks = (short)(wildfire_time * TICKS_PER_SECOND);
				unit_persistent_control(actor->meta.unit_index, wildfire_ticks, FLAG(_unit_control_weapon_primary_trigger_bit));
				unit->unit.weapon_drop_delay_ticks = (char)wildfire_ticks;
			}
		}
	}

	unit = unit_get(actor->meta.unit_index);
	random = real_random();
	weapon_index = unit_inventory_get_weapon(actor->meta.unit_index, unit_get(actor->meta.unit_index)->unit.current_weapon_index);

	if (!ai_globals->grenades_enabled || random<actor_variant->items.dont_drop_grenades_chance)
	{
		memset(unit->unit.grenade_counts, 0, sizeof(unit->unit.grenade_counts));
	}

	if (weapon_index!=NONE)
	{
		if (actor_variant->items.weapon_loaded_lower_bound>0.f || actor_variant->items.weapon_loaded_upper_bound>0.f)
		{
			real weapon_loaded = real_random_range(actor_variant->items.weapon_loaded_lower_bound, actor_variant->items.weapon_loaded_upper_bound);

			weapon_set_current_amount(weapon_index, weapon_loaded);
		}

		if (actor_variant->items.weapon_ammo_lower_bound>0 || actor_variant->items.weapon_ammo_upper_bound>0)
		{
			short ammo_counts[2];

			memset(ammo_counts, 0, sizeof(ammo_counts));
			ammo_counts[0] = random_range(actor_variant->items.weapon_ammo_lower_bound, actor_variant->items.weapon_ammo_upper_bound + 1);
			weapon_set_total_rounds(weapon_index, ammo_counts);
		}
	}

	actor_delete(actor_index, TRUE);

	if (encounter_index!=NONE)
	{
		encounter_update_status(encounter_index);
	}

	return;
}

void actor_swarm_unit_died(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 2272, actor->meta.swarm);

	actor_swarm_detach_from_unit(actor_index, unit_index);

	if (actor->meta.swarm_unit_count==0)
	{
		long encounter_index = actor->meta.encounter_index;

		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 2279, actor->meta.swarm_unit_index == NONE);

		actor_delete(actor_index, TRUE);

		if (encounter_index!=NONE)
		{
			encounter_update_status(encounter_index);
		}
	}

	return;
}

void actors_update(
	void)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;

	ai_globals->last_highest_service_timer = ai_globals->current_highest_service_timer;
	ai_globals->current_highest_service_timer = 0;
	ai_globals->time_given_this_frame = FALSE;

	actor_iterator_new(&iterator, TRUE);

	while ((actor = actor_iterator_next(&iterator))!=NULL)
	{
		if (actor->meta.erase_next_tick)
		{
			actor_erase(iterator.index, FALSE);
		}
		else if (actor->state.mode>_actor_mode_braindead)
		{
			actor_update(iterator.index);
		}
	}

	return;
}

static void actor_freeze_unit(
	long actor_index,
	long unit_index)
{
	struct unit_control_data control_data;

	memset(&control_data, 0, sizeof(control_data));
	control_data.animation_state = _unit_animation_state_alert;
	control_data.control_flags = 0;
	control_data.throttle = *global_zero_vector3d;
	control_data.aiming_speed = _unit_aiming_speed_casual;
	control_data.weapon_index = NONE;
	control_data.grenade_index = NONE;
	control_data.zoom_level = NONE;
	unit_get_facing_vector(unit_index, &control_data.facing_vector);
	unit_get_aiming_vector(unit_index, &control_data.aiming_vector);
	unit_get_looking_vector(unit_index, &control_data.looking_vector);

	unit_control(unit_index, &control_data);
	unit_set_actively_controlled(unit_index, FALSE);

	return;
}

static void actor_freeze(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm)
	{
		if (actor->meta.swarm_cache_index!=NONE)
		{
			short i;
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

			for (i = 0; i<swarm->unit_count; ++i)
			{
				actor_freeze_unit(actor_index, swarm->unit_indices[i]);
			}
		}
	}
	else
	{
		actor_freeze_unit(actor_index, actor->meta.unit_index);
	}

	actor->meta.frozen = TRUE;

	return;
}

void actors_freeze(
	void)
{
	struct actor_iterator iterator;

	actor_iterator_new(&iterator, TRUE);

	while (actor_iterator_next(&iterator))
	{
		actor_freeze(iterator.index);
	}

	return;
}

static void actor_randomly_control_unit(
	long actor_index,
	long unit_index)
{
	struct unit_control_data control_data;

	memset(&control_data, 0, sizeof(control_data));
	control_data.animation_state = _unit_animation_state_in_combat;
	control_data.control_flags = 0;
	control_data.throttle = *global_forward3d;
	control_data.aiming_speed = _unit_aiming_speed_casual;
	control_data.weapon_index = NONE;
	control_data.grenade_index = NONE;
	control_data.zoom_level = NONE;
	unit_get_facing_vector(unit_index, &control_data.facing_vector);
	rotate_vector_about_axis(&control_data.facing_vector, global_up3d, _sine30, _cosine30);
	control_data.aiming_vector = control_data.facing_vector;
	control_data.looking_vector = control_data.facing_vector;

	if ((unit_index + game_time_get()) % 5==0)
	{
		SET_FLAG(control_data.control_flags, _unit_control_weapon_primary_trigger_bit, TRUE);
		control_data.primary_trigger = 1.f;
	}

	unit_control(unit_index, &control_data);
	unit_set_actively_controlled(unit_index, FALSE);

	return;
}

void actors_move_randomly(
	void)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;

	actor_iterator_new(&iterator, TRUE);

	while ((actor = actor_iterator_next(&iterator))!=NULL)
	{
		if (actor->meta.swarm)
		{
			if (actor->meta.swarm_cache_index!=NONE)
			{
				short i;
				struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

				for (i = 0; i<swarm->unit_count; ++i)
				{
					actor_randomly_control_unit(iterator.index, swarm->unit_indices[i]);
				}
			}
		}
		else
		{
			actor_randomly_control_unit(iterator.index, actor->meta.unit_index);
		}
	}

	return;
}

void actor_braindead(
	long actor_index,
	boolean braindead)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (braindead)
	{
		actor->state.mode = _actor_mode_braindead;
		actor->state.action = _actor_action_none;
		actor_delete_props(actor_index);
		actor_freeze(actor_index);
		actor_set_dormant(actor_index, FALSE);
	}
	else
	{
		if (actor->state.mode==_actor_mode_braindead)
		{
			actor->state.mode = _actor_mode_alert;
		}
	}

	return;
}

void actor_change_encounter(
	long actor_index,
	long encounter_index,
	short squad_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor_flush_position_indices(actor_index);

	if (actor->meta.encounterless)
	{
		encounterless_detach_actor(actor_index);
	}
	else if (actor->meta.encounter_index!=NONE)
	{
		encounter_detach_actor(actor_index, FALSE);
	}

	if (encounter_index==NONE)
	{
		encounterless_attach_actor(actor_index);
	}
	else
	{
		encounter_attach_actor(actor_index, encounter_index, squad_index, TRUE);
	}

	return;
}

static void actor_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor_debug_preupdate(actor_index);

	if (actor_general_update(actor_index))
	{
		actor_get_timeslice(actor_index);
		actor_input_update(actor_index);
		actor_perception_update(actor_index);
		actor_situation_update(actor_index);
		actor_emotion_update(actor_index);
		actor_clear_orders(actor_index);
		actor_decision_loop(actor_index);
		actor_action_update(actor_index);

		if (!actor->meta.dormant)
		{
			if (actor->meta.swarm)
			{
				actor_type_swarm_control(actor_index);
			}
			else
			{
				actor_clear_output(actor_index);
				actor_action_control(actor_index);
				actor_communication_update(actor_index);
				actor_conversation_control(actor_index);
				actor_destination_update(actor_index);
				actor_look_affect_movement(actor_index);
				actor_move_update(actor_index);
				actor_look_update(actor_index);
				actor_combat_update(actor_index);
				actor_unit_control(actor_index);
			}
		}
	}

	actor_debug_postupdate();

	return;
}

static void actor_debug_preupdate(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	global_updating_actor_index = actor_index;

#ifdef DEBUG
	if (ai_debug.enter_debugger && ai_debug.selected_actor_index==actor_index)
	{
		ai_debug.enter_debugger = FALSE;
	}
#endif

	return;
}

static void actor_debug_postupdate(
	void)
{
	global_updating_actor_index = NONE;

	return;
}

static boolean actor_general_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	++ai_profile.meters[_ai_meter_actor_active].current_count;

	if (!actor->meta.dormant)
	{
		++ai_profile.meters[_ai_meter_actor_non_dormant].current_count;
	}

	if (actor->meta.swarm)
	{
		ai_profile.meters[_ai_meter_unit_active].current_count += actor->meta.swarm_unit_count;

		if (!actor->meta.dormant)
		{
			ai_profile.meters[_ai_meter_unit_non_dormant].current_count += actor->meta.swarm_unit_count;
		}
	}
	else
	{
		++ai_profile.meters[_ai_meter_unit_active].current_count;

		if (!actor->meta.dormant)
		{
			++ai_profile.meters[_ai_meter_unit_non_dormant].current_count;
		}
	}

	if (actor->meta.swarm && actor->meta.swarm_cache_index==NONE)
	{
		match_vwarn("c:\\halo\\SOURCE\\ai\\actors.c", 2733, FALSE, csprintf(temporary, "tried to update a swarm actor without a swarm cache, erasing %d units", actor->meta.swarm_unit_count));
		actor_erase(actor_index, FALSE);
		result = FALSE;
	}
	else
	{
		struct encounter_datum *encounter;
		boolean force_active;

		actor->control.path.refreshed_this_tick = FALSE;

		if (actor->state.suspicion_timer>0 && --actor->state.suspicion_timer==0)
		{
			actor->state.suspicion_combat_status = _actor_combat_status_none;
		}

		if (actor->state.command_list_delay_timer>0)
		{
			--actor->state.command_list_delay_timer;
		}

		encounter = actor->meta.encounter_index==NONE ? NULL : encounter_get(actor->meta.encounter_index);
		force_active = actor->meta.force_active;

		if (encounter)
		{
			force_active |= encounter->force_active;
		}

		if (!actor->meta.dormant_desire || force_active)
		{
			actor_set_dormant(actor_index, FALSE);
		}
		else if (!actor->meta.dormant)
		{
			short action_class = actor_action_class(actor_index);
			boolean allow_dormant = TRUE;

			if (action_class==_action_class_transitory)
			{
				allow_dormant = FALSE;
			}
			else if (actor->target.target_prop_index!=NONE)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				if (prop->player &&
					prop->enemy &&
					!prop->dead &&
					((prop->state>=_prop_state_becoming_unacknowledged && prop->state<=_prop_state_acknowledged) ||
					(prop->state>=_prop_state_uninspected_orphan && prop->state<=_prop_state_inspected_orphan && action_class==_action_class_pursuit)))
				{
					allow_dormant = FALSE;
				}
			}

			if (allow_dormant && actor_path_has_path(actor_index))
			{
				switch (actor->control.path.destination_orders.destination_type)
				{
				case _destination_firing_position:
					if (actor->state.action==_actor_action_guard && encounter->follow_target_type==_follow_target_players)
					{
						allow_dormant = FALSE;
					}
					break;

				case _destination_prop:
					if (prop_get(actor->control.path.destination_orders.prop.prop_index)->player)
					{
						allow_dormant = FALSE;
					}
					break;
				}
			}

			if (allow_dormant)
			{
				if (++actor->meta.become_dormant_timer>=2 * TICKS_PER_SECOND)
				{
					actor_set_dormant(actor_index, TRUE);
				}
			}
		}
	}

	return result;
}

static boolean actor_get_timeslice(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean timeslice = FALSE;
	boolean high_priority = FALSE;

	if (actor->state.action==_actor_action_charge &&
		(actor->state.action_data.charge.goal==_charge_goal_melee ||
		actor->state.action_data.charge.goal==_charge_goal_melee_leaping ||
		actor->state.action_data.charge.goal==_charge_goal_vehicle_strafing ||
		actor->state.action_data.charge.goal==_charge_goal_vehicle_ramming))
	{
		high_priority = TRUE;
	}

	actor->meta.service_timer += high_priority ? 3 : 1;

	if (ai_globals->time_given_this_frame ||
		actor->meta.service_timer<=ai_globals->last_highest_service_timer ||
		actor->meta.service_timer<=15)
	{
		if (actor->meta.service_timer>ai_globals->current_highest_service_timer)
		{
			ai_globals->current_highest_service_timer = actor->meta.service_timer;
		}
	}
	else
	{
		actor->meta.service_timer = 0;
		ai_globals->time_given_this_frame = TRUE;
		timeslice = TRUE;
	}

	actor->meta.timeslice = timeslice;

	return timeslice;
}

static void actor_clear_output(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->output.facing_vector = actor->input.facing_vector;
	actor->output.aiming_vector = actor->input.aiming_vector;
	actor->output.looking_vector = actor->input.looking_vector;
	actor->output.control_flags = 0;
	actor->output.analog_primary_trigger = 0.f;
	actor->output.throttle = *global_zero_vector3d;
	actor->output.animation.impulse = NONE;

	return;
}

void actor_find_pathfinding_location(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (actor->input.pathfinding_surface_index==NONE)
	{
		actor->input.pathfinding_point = actor->input.position.body_position;

		if (actor->state.flying)
		{
		}
		else
		{
			if (actor->input.vehicle_index!=NONE)
			{
				switch (actor->input.vehicle_driver_type)
				{
				case _actor_vehicle_driver_hovering_ground:
				case _actor_vehicle_driver_nondirectional_ground:
					actor->input.pathfinding_surface_index = vehicle_find_pathfinding_surface_index(actor->input.vehicle_index, &actor->input.pathfinding_point);
					break;
				}
			}
			else
			{
				if (biped_try_and_get(actor->meta.unit_index))
				{
					actor->input.pathfinding_surface_index = biped_find_pathfinding_surface_index(actor->meta.unit_index, &actor->input.pathfinding_point);
				}
			}
		}
	}

	return;
}

real actor_destination_tolerance(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	real tolerance = 0.5f;

	if (actor->input.vehicle_index!=NONE)
	{
		struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);

		tolerance = vehicle_definition_get(vehicle->definition_index)->vehicle.ai_destination_radius;
	}

	if (actor->state.action==_actor_action_obey && actor->state.action_data.obey.complex_control.destination_radius_valid)
	{
		tolerance = actor->state.action_data.obey.complex_control.destination_radius;
	}

	if (actor->state.action==_actor_action_vehicle)
	{
		tolerance = 0.7f;
	}

	return MAX(tolerance, 0.2f);
}

void actor_input_sample_position(
	long actor_index,
	long unit_index,
	struct actor_position_data *position)
{
	long parent_index;
	struct object_datum *parent;
	struct unit_datum *unit = unit_get(unit_index);

	object_get_origin(unit_index, &position->body_position);
	position->body_facing = unit->object.forward;
	unit_get_head_position(unit_index, &position->head_position);
	object_get_velocities(unit_index, &position->velocity, NULL);

	parent_index = object_get_ultimate_parent(unit_index);
	parent = object_get(parent_index);
	position->body_location = parent->object.location;

	return;
}

static void actor_input_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (actor->meta.swarm)
	{
		short i;
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

		swarm->swarm_center = *global_origin3d;

		for (i = 0; i<swarm->unit_count; ++i)
		{
			struct swarm_component_datum *swarm_component = swarm_component_get(swarm->component_indices[i]);

			actor_swarm_component_refresh(swarm->unit_indices[i], swarm->component_indices[i]);
			add_vectors3d((real_vector3d *)&swarm_component->position, (real_vector3d *)&swarm->swarm_center, (real_vector3d *)&swarm->swarm_center);
		}

		if (swarm->unit_count>0)
		{
			scale_vector3d((real_vector3d *)&swarm->swarm_center, 1.f / swarm->unit_count, (real_vector3d *)&swarm->swarm_center);
		}

		memset(&actor->input, 0, sizeof(actor->input));
		actor->input.vehicle_index = NONE;
		actor->input.pathfinding_surface_index = NONE;

		if (actor->meta.swarm_unit_index!=NONE)
		{
			actor_input_sample_position(actor_index, actor->meta.swarm_unit_index, &actor->input.position);
		}
	}
	else
	{
		real_point3d center_of_mass;
		long child_index;
		struct unit_datum *unit = unit_get(actor->meta.unit_index);
		long parent_index = unit->object.parent_object_index;
		struct object_datum *parent = parent_index==NONE ? NULL : object_get(parent_index);

		actor_input_sample_position(actor_index, actor->meta.unit_index, &actor->input.position);
		unit_get_center_of_mass(actor->meta.unit_index, &center_of_mass);
		actor->input.underwater = scenario_location_underwater(&actor->input.position.body_location, &center_of_mass, NULL);
		actor->state.flying = TEST_FLAG(actor_definition->flags, _actor_definition_flying_bit);

		if (parent && parent->object.type==_object_type_vehicle)
		{
			struct vehicle_datum *vehicle = (struct vehicle_datum *)parent;
			struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

			actor->input.vehicle_index = parent_index;
			actor->input.vehicle_gunner = FALSE;
			actor->input.vehicle_gunner_bombardment = FALSE;
			actor->input.vehicle_driver_type = _actor_vehicle_driver_none;

			if (vehicle->unit.driver_object_index==actor->meta.unit_index)
			{
				actor->input.vehicle_driver_type = _actor_vehicle_driver_unknown;

				if (TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_driver_enable_bit))
				{
					if (TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_driver_flying_bit))
					{
						actor->input.vehicle_driver_type = _actor_vehicle_driver_directional_flying;
						actor->state.flying = TRUE;
					}
					else if (TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_driver_nondirectional_bit))
					{
						actor->input.vehicle_driver_type = TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_driver_hovering_bit) ? _actor_vehicle_driver_hovering_ground : _actor_vehicle_driver_nondirectional_ground;
					}
				}
			}

			if (vehicle->unit.gunner_object_index==actor->meta.unit_index)
			{
				struct actor_variant_definition *actor_variant;

				actor->input.vehicle_gunner = TRUE;
				actor_variant = actor_combat_get_firing_variant_definition(actor_index);
				actor->input.vehicle_gunner_bombardment = actor_variant->ranged_combat.weapon_bombardment_range>0.f;
			}

			actor->input.vehicle_passenger = actor->input.vehicle_driver_type<=_actor_vehicle_driver_unknown;

			if (vehicle->unit.fake_encounter_index!=NONE)
			{
				boolean change_encounter = FALSE;

				if (DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index)==vehicle->unit.fake_encounter_index)
				{
					if (vehicle->unit.fake_squad_index!=NONE && actor->meta.squad_index!=vehicle->unit.fake_squad_index)
					{
						struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

						change_encounter = TRUE;

						if (encounter->follow_target_type>_follow_target_none)
						{
							struct squad_datum *squad = encounter_get_squad(encounter, actor->meta.squad_index);
							struct squad_datum *vehicle_squad = encounter_get_squad(encounter, vehicle->unit.fake_squad_index);

							if (squad->automatic_migration_target && vehicle_squad->automatic_migration_target)
							{
								change_encounter = FALSE;
							}
						}
					}
				}
				else
				{
					change_encounter = TRUE;
				}

				if (change_encounter)
				{
					if (!actor->meta.stored_prevehicle_encounter)
					{
						actor->meta.prevehicle_encounter_index = actor->meta.encounter_index;
						actor->meta.prevehicle_squad_index = actor->meta.squad_index;
						actor->meta.stored_prevehicle_encounter = TRUE;

						if (actor->meta.encounter_index!=NONE)
						{
							struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

							encounter->is_prevehicle_encounter = TRUE;
						}
					}

					actor_change_encounter(actor_index, vehicle->unit.fake_encounter_index, vehicle->unit.fake_squad_index);
				}
			}
		}
		else
		{
			actor->input.vehicle_index = NONE;
			actor->input.vehicle_driver_type = _actor_vehicle_driver_none;
			actor->input.vehicle_passenger = FALSE;
			actor->input.vehicle_gunner = FALSE;

			if (actor->meta.stored_prevehicle_encounter)
			{
				actor_change_encounter(actor_index, actor->meta.prevehicle_encounter_index, actor->meta.prevehicle_squad_index);
				actor->meta.stored_prevehicle_encounter = FALSE;
			}
		}

		if (actor->meta.encounter_index!=NONE)
		{
			struct actor_debug_info *debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index), struct encounter_definition);
			boolean wrong_encounter_type = FALSE;

			if (actor->state.flying && !TEST_FLAG(encounter_definition->flags, _encounter_3d_firing_positions_bit))
			{
				wrong_encounter_type = TRUE;
			}
			else if (!actor->state.flying && TEST_FLAG(encounter_definition->flags, _encounter_3d_firing_positions_bit))
			{
				wrong_encounter_type = TRUE;
			}

			if (wrong_encounter_type)
			{
				if (debug_info->evaluation_mode<5 * TICKS_PER_SECOND && ++debug_info->evaluation_mode==5 * TICKS_PER_SECOND)
				{
					if (actor->state.flying)
					{
						error(_error_silent, "actor %s is flying but was placed in a non-3d-firing-position encounter (%s)", tag_get_name(actor->meta.definition_index), encounter_definition->name);
					}
					else
					{
						error(_error_silent, "actor %s is not flying but was placed in a 3d-firing-position encounter (%s)", tag_get_name(actor->meta.definition_index), encounter_definition->name);
					}
				}
			}
			else
			{
				debug_info->evaluation_mode = 0;
			}
		}

		actor->input.burning_to_death = unit->unit.flaming_death_delay>0;
		actor->input.melee_attacker_attached = FALSE;
		actor->input.delayed_attached_projectile_index = NONE;

		for (child_index = unit->object.first_child_object_index; child_index!=NONE; )
		{
			struct object_datum *child = object_get(child_index);

			if (child->object.type==_object_type_biped && game_team_is_enemy(actor->meta.team_index, child->object.owner_team_index))
			{
				actor->input.melee_attacker_attached = TRUE;
			}
			else if (child->object.type==_object_type_projectile)
			{
				struct projectile_datum *projectile = (struct projectile_datum *)child;

				if (TEST_FLAG(projectile->projectile.flags, _projectile_will_super_explode_bit))
				{
					actor->input.delayed_attached_projectile_index = child_index;
				}
				else if (actor->danger_zone.danger_type==_actor_danger_zone_projectile && child_index==actor->danger_zone.object_index)
				{
					actor->input.delayed_attached_projectile_index = child_index;
				}
			}

			child_index = child->object.next_object_index;
		}

		actor->input.in_midair = FALSE;
		actor->input.pathfinding_surface_index = NONE;

		if (unit->object.type==_object_type_biped && actor->input.vehicle_index==NONE)
		{
			struct biped_datum *biped = biped_get(actor->meta.unit_index);

			if (biped->biped.airborne_ticks>=6)
			{
				actor->input.in_midair = TRUE;
			}

			actor->input.pathfinding_surface_index = biped->biped.pathfinding_surface_index;
			actor->input.pathfinding_point = biped->biped.pathfinding_point;
		}

		if (actor->input.vehicle_driver_type>_actor_vehicle_driver_none)
		{
			unit_get_facing_vector(actor->input.vehicle_index, &actor->input.facing_vector);
		}
		else
		{
			unit_get_facing_vector(actor->meta.unit_index, &actor->input.facing_vector);
		}

		if (!actor->state.flying)
		{
			if (normalize2d((real_vector2d *)&actor->input.facing_vector)>0.f)
			{
				actor->input.facing_vector.k = 0.f;
			}
			else
			{
				actor->input.facing_vector = *global_forward3d;
			}
		}

		if (actor->input.vehicle_gunner)
		{
			struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
			struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

			if (TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_weapon_cannot_rotate_bit))
			{
				unit_get_facing_vector(actor->meta.unit_index, &actor->input.aiming_vector);
			}
			else
			{
				actor->input.aiming_vector = vehicle->unit.aiming_vector;
			}
		}
		else
		{
			actor->input.aiming_vector = unit->unit.aiming_vector;
		}

		actor->input.looking_vector = unit->unit.looking_vector;
		cross_product3d(global_up3d, &actor->input.looking_vector, &actor->input.looking_left_vector);
		normalize3d(&actor->input.looking_left_vector);
		cross_product3d(&actor->input.looking_vector, &actor->input.looking_left_vector, &actor->input.looking_up_vector);

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3313, &actor->input.facing_vector);
		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3314, &actor->input.aiming_vector);
		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3315, &actor->input.looking_vector);

		if (!actor->state.flying)
		{
			match_assert_valid_real_normal2d("c:\\halo\\SOURCE\\ai\\actors.c", 3318, (real_vector2d *) &actor->input.facing_vector);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 3319, realcmp(actor->input.facing_vector.k, 0.0f));
		}

		actor->input.body_vitality = unit->object.body_vitality;
		actor->input.shield_vitality = unit->object.shield_vitality;
		actor->input.recent_body_damage = unit->object.recent_body_damage;
		actor->input.recent_shield_damage = unit->object.recent_shield_damage;
	}

	return;
}

static void actor_clear_orders(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	memset(&actor->orders, 0, sizeof(actor->orders));
	actor->orders.move.animation.impulse = NONE;
	actor->orders.move.override_movement_type = NONE;
	actor->orders.move.override_movement_facing = NONE;

	return;
}

static void actor_decision_loop(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean performed = FALSE;
	long iteration_count = 0;
	long history_index = 0;
	short update_history[ACTOR_DECISION_HISTORY_COUNT];
	boolean error_condition;

	memset(update_history, NONE, sizeof(update_history));

	while (TRUE)
	{
		error_condition = FALSE;
		++iteration_count;
		update_history[history_index] = actor->state.action;
		history_index = (history_index + 1) % ACTOR_DECISION_HISTORY_COUNT;
		actor->state.action_changed = FALSE;

		actor_type_decide_action(actor_index);
		actor_stimulus_clear(actor_index);

		if (performed && !actor->state.action_changed)
		{
			error_condition = TRUE;
		}
		else if (iteration_count>=MAXIMUM_ACTOR_DECISION_ITERATIONS)
		{
			error_condition = TRUE;
		}

		if (error_condition)
		{
			break;
		}

		performed = actor_action_perform(actor_index);

		if (!performed && !actor->state.action_changed)
		{
			break;
		}
	}

	if (error_condition)
	{
		char errbuf[1024];
		char encounterbuf[256];

		if (actor->meta.encounter_index==NONE)
		{
			strcpy(encounterbuf, "<no encounter>");
		}
		else
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index), struct encounter_definition);
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(&encounter_definition->squads, actor->meta.squad_index, struct squad_definition);

			sprintf(encounterbuf, "%s/%s", encounter_definition->name, squad_definition->name);
		}

		if (iteration_count>=MAXIMUM_ACTOR_DECISION_ITERATIONS)
		{
			long i;

			sprintf(errbuf, "actor-type %s ", actor_type_get_name(actor->meta.type));
			i = history_index;

			do
			{
				if (update_history[i]!=NONE)
				{
					strcat(errbuf, actor_action_name(update_history[i]));
					strcat(errbuf, "/");
				}

				i = (i + 1) % ACTOR_DECISION_HISTORY_COUNT;
			} while (i!=history_index);

			sprintf(temporary, " infinite decision loop (%s)", encounterbuf);
			strcat(errbuf, temporary);
		}
		else
		{
			sprintf(errbuf, "actor-type %s %s internal logic error (%s)", actor_type_get_name(actor->meta.type), actor_action_name(actor->state.action), encounterbuf);
		}

		match_vwarn("c:\\halo\\SOURCE\\ai\\actors.c", 3436, FALSE, errbuf);
		error(_error_silent, "AI error condition detected, attempting to recover (please tell butcher)...");
		actor_action_change(actor_index, _actor_action_none, NULL);
	}

	return;
}

void actors_handle_spatial_effect(
	long object_index,
	short effect_type,
	real_point3d const *position,
	short volume,
	short count)
{
	struct actor_iterator iterator;
	struct location location;
	struct actor_datum *actor;

	scenario_location_from_point(&location, position);
	actor_iterator_new(&iterator, TRUE);

	while ((actor = actor_iterator_next(&iterator))!=NULL)
	{
		if (actor->state.combat_status<_actor_combat_status_visible)
		{
			struct actor_position_data sense_position;

			actor_perception_find_sense_position(iterator.index, position, NONE, &sense_position);

			if (actor_audibility_at_point(iterator.index, &sense_position, position, &location, volume, 1.f, 0)>=_actor_perception_full)
			{
				switch (effect_type)
				{
				case _ai_spatial_effect_environmental_noise:
					actor_stimulus_environmental_noise(iterator.index, object_index, position, count);
					break;

				case _ai_spatial_effect_weapon_impact:
					actor_stimulus_weapon_impact(iterator.index, object_index, position, count);
					break;

				case _ai_spatial_effect_weapon_detonation:
					actor_stimulus_weapon_detonation(iterator.index, object_index, position, count);
					break;

				default:
					match_halt("c:\\halo\\SOURCE\\ai\\actors.c", 3498);
					break;
				}
			}
		}
	}

	return;
}

void actors_handle_unit_effect(
	long owner_unit_index,
	short effect_type,
	short volume)
{
	struct structure_bsp *structure_bsp;
	unsigned long potentially_audible_clusters[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
	struct actor_iterator iterator;
	real_point3d sound_position;
	struct actor_datum *actor;
	struct unit_datum *unit = unit_get(owner_unit_index);
	struct location *location = &unit->object.location;
	long owner_actor_index = unit->unit.swarm_actor_index==NONE ? unit->unit.actor_index : unit->unit.swarm_actor_index;

	if (unit->object.parent_object_index!=NONE)
	{
		long parent_index = object_get_ultimate_parent(owner_unit_index);
		struct object_datum *parent = object_get(parent_index);

		location = &parent->object.location;
	}

	structure_bsp = global_structure_bsp_get();
	memset(potentially_audible_clusters, 0, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));

	if (location->cluster_index!=NONE)
	{
		short cluster_index;

		for (cluster_index = 0; cluster_index<structure_bsp->clusters.count; ++cluster_index)
		{
			byte encoded_sound_distance = structure_bsp_get_cluster_encoded_sound_distance(structure_bsp, location->cluster_index, cluster_index);

			if (!TEST_FLAG(encoded_sound_distance, 7) && (real)(encoded_sound_distance & ~FLAG(7)) * 256.f / 127.f<40.f)
			{
				BIT_VECTOR_SET_FLAG(potentially_audible_clusters, cluster_index, TRUE);
			}
		}
	}

	object_get_origin(owner_unit_index, &sound_position);
	actor_iterator_new(&iterator, TRUE);

	while ((actor = actor_iterator_next(&iterator))!=NULL)
	{
		if (iterator.index!=owner_actor_index &&
			actor->input.position.body_location.cluster_index!=NONE &&
			BIT_VECTOR_TEST_FLAG(potentially_audible_clusters, actor->input.position.body_location.cluster_index))
		{
			struct actor_position_data sense_position;

			actor_perception_find_sense_position(iterator.index, &sound_position, NONE, &sense_position);

			if (actor_audibility_at_point(iterator.index, &sense_position, &sound_position, location, volume, 1.f, 0)>=_actor_perception_full)
			{
				long prop_index = prop_get_base_by_unit_index(iterator.index, owner_unit_index, TRUE, TRUE);

				if (prop_index!=NONE)
				{
					struct prop_datum *prop = prop_get(prop_index);

					if (actor_audibility_at_point(iterator.index, &sense_position, &prop->body_position, &prop->body_location, volume, 1.f, prop->line_of_sight)>=_actor_perception_full)
					{
						actor_handle_unit_effect(iterator.index, prop_index, effect_type);
					}
				}
			}
		}
	}

	return;
}

void actor_handle_unit_effect(
	long actor_index,
	long prop_index,
	short effect_type)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);
	boolean deaf = actor->meta.encounter_index!=NONE ? encounter_get(actor->meta.encounter_index)->deaf : FALSE;

	if (AI_BEHAVIOR(deaf))
	{
		deaf = TRUE;
	}

	if (prop->unit_effect==NONE || prop->unit_effect<=effect_type)
	{
		prop->unit_effect = effect_type;
		prop->unit_effect_decay_ticks = effect_type==_ai_unit_effect_magic_sight ? 5 * TICKS_PER_SECOND : TICKS_PER_SECOND;
	}

	switch (effect_type)
	{
	case _ai_unit_effect_shooting:
		if (!deaf && !prop->ignore)
		{
			prop->shooting = TRUE;
			prop->audibility = _actor_perception_unmistakable;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;

			if (prop->player)
			{
				actor_set_dormant(actor_index, FALSE);
			}

			actor_stimulus_heard_shooting(actor_index, prop_index);
		}
		break;

	case _ai_unit_effect_death_scream:
		if (!deaf && !prop->ignore)
		{
			prop->dead = TRUE;
			prop->audibility = _actor_perception_unmistakable;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;
			actor_set_dormant(actor_index, FALSE);
			actor_stimulus_prop_just_killed(actor_index, prop_index);
		}
		break;

	case _ai_unit_effect_bump:
		if (!prop->ignore)
		{
			prop->ineffability = _actor_perception_unmistakable;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;
			actor_stimulus_bumped(actor_index, prop_index);
		}
		break;

	case _ai_unit_effect_magic_sight:
		if (!prop->ignore)
		{
			prop->ineffability = _actor_perception_unmistakable;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;

			if (prop->player)
			{
				actor_set_dormant(actor_index, FALSE);
			}

			actor_perception_become_acknowledged(actor_index, prop_index, NULL);
		}
		break;
	}

	return;
}

void actor_handle_communication(
	long actor_index,
	long prop_index,
	struct ai_information_packet *ai_information)
{
	if (ai_information)
	{
		switch (ai_information->information_type)
		{
		case _ai_information_combat_stimulus:
			actor_stimulus_enter_combat_friend_in_combat(actor_index, prop_index);
			break;

		case _ai_information_target_knowledge:
		{
			struct prop_datum *prop = prop_get(prop_index);

			if (prop->actor_index!=NONE)
			{
				struct prop_datum *friend_prop = prop_try_and_get(ai_information->information_data.target_knowledge.prop_index);

				if (friend_prop)
				{
					actor_perception_create_orphan_from_friend(actor_index, friend_prop->unit_index, prop->actor_index, ai_information->information_data.target_knowledge.prop_index);
				}
			}
			break;
		}

		case _ai_information_flee:
			actor_stimulus_prop_fleeing(actor_index, prop_index);
			break;
		}
	}

	return;
}

void actor_unit_control_crouch(
	long actor_index,
	boolean crouch)
{
	struct actor_datum *actor = actor_get(actor_index);

	SET_FLAG(actor->output.control_flags, _unit_control_crouch_modifier_bit, crouch);

	return;
}

void actor_unit_control_jump(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	SET_FLAG(actor->output.control_flags, _unit_control_jump_bit, TRUE);

	return;
}

void actor_unit_control_primary_trigger(
	long actor_index,
	boolean trigger,
	real analog_value)
{
	struct actor_datum *actor = actor_get(actor_index);

	SET_FLAG(actor->output.control_flags, _unit_control_weapon_primary_trigger_bit, trigger);
	actor->output.analog_primary_trigger = analog_value;

	return;
}

void actor_unit_control_secondary_trigger(
	long actor_index,
	boolean trigger)
{
	struct actor_datum *actor = actor_get(actor_index);

	SET_FLAG(actor->output.control_flags, _unit_control_weapon_secondary_trigger_bit, trigger);

	return;
}

void actor_unit_control_throw_grenade(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	SET_FLAG(actor->output.control_flags, _unit_control_throw_grenade_bit, TRUE);

	return;
}

void actor_unit_control_exact_facing(
	long actor_index,
	boolean exact_facing)
{
	struct actor_datum *actor = actor_get(actor_index);

	SET_FLAG(actor->output.control_flags, _unit_control_exact_facing_bit, exact_facing);

	return;
}

void actor_unit_control_stop_animation_impulse(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->output.animation.impulse = NONE;

	return;
}

static void actor_unit_control(
	long actor_index)
{
	struct unit_control_data control_data;
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(actor->meta.unit_index);

	control_data.animation_state = (char)global_movement_animation_states[actor->output.movement_type];
	control_data.control_flags = (word)actor->output.control_flags;
	control_data.primary_trigger = actor->output.analog_primary_trigger;
	control_data.throttle = actor->output.throttle;
	control_data.aiming_speed = (char)actor->output.aiming_speed;
	control_data.facing_vector = actor->output.facing_vector;
	control_data.aiming_vector = actor->output.aiming_vector;
	control_data.looking_vector = actor->output.looking_vector;
	control_data.weapon_index = NONE;
	control_data.grenade_index = NONE;
	control_data.zoom_level = NONE;

	if (!actor->state.flying)
	{
		match_assert_valid_real_normal2d("c:\\halo\\SOURCE\\ai\\actors.c", 3846, (real_vector2d *) &control_data.facing_vector);
	}
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3848, &control_data.facing_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3849, &control_data.aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3850, &control_data.looking_vector);
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 3851, (fabs(control_data.throttle.i) <= 1.0f) && (fabs(control_data.throttle.j) <= 1.0f) && (fabs(control_data.throttle.k) <= 1.0f));

	if (unit->unit.player_index==NONE || !player_input_enabled())
	{
		if (actor->meta.frozen)
		{
			unit_set_actively_controlled(actor->meta.unit_index, TRUE);
			actor->meta.frozen = FALSE;
		}

		unit_control(actor->meta.unit_index, &control_data);

		if (actor->output.animation.impulse!=NONE)
		{
			unit_start_animation_impulse(actor->meta.unit_index, actor->output.animation.impulse, &actor->output.animation.alignment_vector);
		}

		if (actor->output.persistent_control_ticks>0)
		{
			unit_persistent_control(actor->meta.unit_index, actor->output.persistent_control_ticks, actor->output.persistent_control_flags);
		}
	}

	return;
}
