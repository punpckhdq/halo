/*
UNITS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "units.h"
#include "vehicles.h"
#include "bipeds.h"
#include "dialogue_definitions.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "game_state.h"
#include "network_messages.h"
#include "actors.h"
#include "collisions.h"
#include "sound_manager.h"
#include "ai_debug.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
#include "weapons.h"
#include "game_sound.h"
#include "structures.h"
#include "physics_constants.h"
#include "network_client_manager.h"
#include "effects.h"
#include "network_server_message_handler.h"
#include "object_lights.h"
#include "object_types.h"
#include "damage_effect_definitions.h"
#include "devices.h"
#include "physics_definitions.h"
#include "projectiles.h"
#include "first_person_weapons.h"
#include "equipment_definitions.h"
#include "predicted_resources.h"
#include "equipment.h"
#include "object_lists.h"
#include "collision_models.h"

/* ---------- constants */

enum
{
	_unit_function_none = 0,
	_unit_function_driver_seat_power,
	_unit_function_gunner_seat_power,
	_unit_function_aiming_change,
	_unit_function_mouth_aperture,
	_unit_function_integrated_light_power,
	_unit_function_can_blink,
	_unit_function_shield_sapping,
	NUMBER_OF_UNIT_FUNCTION_MODES
};

#define RUN_BLINDLY_MAXIMUM_ANGLE DEGREES_TO_RADIANS(45.f) /* fake name */
#define RUN_BLINDLY_MAXIMUM_ANGULAR_VELOCITY DEGREES_TO_RADIANS(12.f) /* fake name */
#define RUN_BLINDLY_ANGULAR_ACCELERATION (RUN_BLINDLY_MAXIMUM_ANGULAR_VELOCITY * 0.1f) /* fake name */

enum
{
	_vehicle_seat_desire_any_non_driver = 0,
	_vehicle_seat_desire_gunner,
	_vehicle_seat_desire_passenger,
	_vehicle_seat_desire_driver,
	_vehicle_seat_desire_any,
	NUMBER_OF_VEHICLE_SEAT_DESIRE_TYPES
};

/* ---------- macros */

#define unit_get_current_weapon_index(unit_index) unit_inventory_get_weapon((unit_index), unit_get((unit_index))->unit.current_weapon_index) /* fake name */
#define unit_get_desired_weapon_index(unit_index) unit_inventory_get_weapon((unit_index), unit_get((unit_index))->unit.desired_weapon_index) /* fake name */

/* ---------- structures */

struct unit_globals
{
	short next_timer;
	short highest_timer;
	boolean used_time;
	char pad[3];
};

struct euler_axis_plan
{
	boolean stopped;
	char pad[3];
	real initial_p;
	real initial_v;
	real accel_a;
	real accel_t;
	real coast_t;
	real decel_a;
	real decel_t;
};

/* ---------- prototypes */

static char const *base_seat_label_get(short base_seat_index);
static short seat_label_to_base_seat_index(char const *seat_label);
static char const *base_weapon_label_get(short base_weapon_index);

static void unit_refresh_illumination(long unit_index);

static void unit_update_driver_and_gunner(long parent_object_index);
static void unit_add_initial_weapons(long unit_index);
static short unit_first_free_weapon_index(long unit_index);
static void unit_adjust_for_seat_change(long unit_index);
static void unit_ready_desired_weapon(long unit_index, boolean immediate);
static short unit_weapon_next_index(long unit_index, short current_index, short delta);
static char const *unit_get_current_weapon_label(long unit_index);

static void unit_throw_grenade_move_to_hand(long unit_index);

static boolean unit_animation_busy(struct unit_animation *animation);
static boolean unit_animation_state_interruptable(struct unit_animation *animation, short new_state);
static boolean unit_animation_overlay_action_loops(struct unit_animation *animation);
static boolean unit_animation_state_loops(struct unit_animation *animation);
static boolean unit_animation_weapon_ik(struct unit_animation *animation);
static boolean unit_animation_vehicle_ik(struct unit_animation *animation);
static boolean unit_animation_aiming_screen(struct unit_animation *animation);
static short unit_animation_state_get_aiming_screen_index(short state);
static boolean unit_animation_state_can_be_entered_without_animation(short new_state);
static short unit_animation_compute_interpolation_frame_count(short old_state, short new_state);

static void unit_euler_axis_buildplan(
	real current_location,
	real current_velocity,
	real velocity_limit,
	real acceleration_limit,
	struct euler_axis_plan *plan);

static void unit_euler_axis_couple(
	struct euler_axis_plan *plan1,
	struct euler_axis_plan *plan2,
	real velocity_limit,
	real acceleration_limit);

static boolean unit_set_or_test_seat_and_weapon_label(
	long object_index,
	char const *seat_label,
	char const *weapon_label,
	boolean change_flag);

static boolean unit_animation_set_state(
	long unit_index,
	short new_state);

static boolean unit_vectors_are_valid(long unit_index);
static void unit_throw_grenade_release(long unit_index, boolean premature);

static void unit_seat_update(long object_index);

static char const *unit_get_seat_label(long object_index);

static void unit_cause_continuous_melee_damage(long unit_index);

static void unit_set_animation(long unit_index, long animation_graph_index, short animation_index);
static short unit_animation_update(long unit_index, long animation_graph_index, struct animation_state *state);
static void unit_drop_grenades(long unit_index);
static void unit_drop_inventory_weapons(long unit_index);
static void unit_melee_sound(long unit_index, long damage_definition_index, short material_type);

static long unit_get_weapon(struct unit_datum *unit, short index);
static void unit_drop_item(long unit_index, long item_index);

#ifdef DEBUG
static void unit_verify_vectors(long unit_index, char const *debugstring);
#endif
static void unit_align_facing(long unit_index, real_vector2d const *alignment_vector);
static void unit_flame_to_death(long unit_index);
static void unit_running_blind(long unit_index, real_vector3d *run_vector);

static boolean unit_integrated_night_vision_is_active(long unit_index);

static void unit_animation_start_overlay_action(long unit_index, short overlay_action);
static short unit_test_spawning(long unit_index);

static short unit_animation_impulse_get_index(short animation_impulse, short *interpolation_frame_count);
static boolean unit_can_play_animation_impulse(long unit_index, short animation_impulse);

/* ---------- globals */

short magic_base_animation_seat_index = NONE;

static struct unit_globals *unit_globals;

static struct profile_section unit_update_section = {"unit_update", NONE, TRUE};

#ifdef DEBUG
boolean debug_objects_unit_mouth_apeture;
boolean debug_objects_unit_seats;
boolean debug_objects_unit_vectors;
#endif
boolean stun_enable;
#ifdef DEBUG
boolean debug_damage_taken;
boolean debug_unit_illumination;
boolean debug_unit_animations;
boolean debug_unit_all_animations;
#endif

/* ---------- public code */

void units_initialize(
	void)
{
	unit_globals = game_state_malloc("unit globals", NULL, sizeof(*unit_globals));
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 264, unit_globals);

	return;
}

void units_initialize_for_new_map(
	void)
{
	memset(unit_globals, 0, sizeof(unit_globals));

	return;
}

void units_dispose_from_old_map(
	void)
{
	return;
}

void units_dispose(
	void)
{
	return;
}

void unit_place(
	long unit_index,
	struct scenario_unit_datum *scenario_unit)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (scenario_unit->body_vitality > 0.f)
	{
		unit->object.body_vitality = scenario_unit->body_vitality;
	}

	if (TEST_FLAG(scenario_unit->flags, _scenario_unit_dead_bit))
	{
		unit_ping_animation(unit_index, TRUE, FALSE, FALSE, FALSE, FALSE, 0.f, NONE, NULL);

		if (unit->unit.animation.state == _unit_state_dying)
		{
			struct animation_graph *animation_graph;
			struct animation *animation;

			unit_delete_all_weapons(unit_index);
			memset(unit->unit.grenade_counts, 0, sizeof(unit->unit.grenade_counts));

			if (unit->unit.equipment_object_index != NONE)
			{
				object_delete(unit->unit.equipment_object_index);
				unit->unit.equipment_object_index = NONE;
			}

			animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);
			unit->object.animation.state.frame_index = FLOOR(animation->frame_count - 4, 0);
			SET_FLAG(unit->unit.flags, _unit_placed_here_dead_bit, TRUE);
			SET_FLAG(unit->object.flags, _object_cannot_be_garbage_bit, TRUE);
			SET_FLAG(unit->object.damage_flags, _object_dead_bit, TRUE);
			unit->unit.time_of_death = game_time_get();
			unit->object.body_vitality = 0.f;
			unit->object.shield_vitality = 0.f;
			object_deplete_shield(unit_index);
			object_compute_node_matrices_recursive(unit_index);
		}
	}

	return;
}

boolean unit_new(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	boolean success = FALSE;

	if (unit_definition->object.animation_graph.index != NONE)
	{
		short seat_index;
		boolean has_built_in_actor;

		unit->unit.equipment_object_index = NONE;
		memset(unit->unit.weapon_object_indices, NONE, sizeof(unit->unit.weapon_object_indices));
		unit->unit.current_weapon_index = NONE;
		unit->unit.desired_weapon_index = NONE;
		unit->unit.current_grenade_index = NONE;
		unit->unit.desired_grenade_index = NONE;
		unit->unit.current_zoom_level = NONE;
		unit->unit.desired_zoom_level = NONE;
		unit->unit.player_index = NONE;
		unit->unit.actor_index = NONE;
		unit->unit.swarm_actor_index = NONE;
		unit->unit.swarm_next_unit_index = NONE;
		unit->unit.swarm_prev_unit_index = NONE;
		unit->unit.parent_seat_index = NONE;
		unit->unit.driver_object_index = NONE;
		unit->unit.gunner_object_index = NONE;
		unit->unit.animation.flags = 0;
		unit->unit.animation.seat_index = NONE;
		unit->unit.animation.weapon_index = NONE;
		unit->unit.animation.weapon_type_index = NONE;
		unit->unit.animation.state = NONE;
		unit->unit.animation.action = 0;
		unit->unit.animation.overlay_action = 0;
		unit->unit.animation.aiming_screen_index = NONE;
		unit->unit.animation.looking_screen_index = NONE;
		unit->unit.animation.action_animation.index = NONE;
		unit->unit.animation.overlay_action_animation.index = NONE;
		unit->unit.animation.soft_ping_animation.index = NONE;
		unit->unit.animation.base_seat_index = _unit_base_seat_stand;
		unit->unit.animation.last_ping_animation_index = NONE;
		unit->unit.animation.emotion_index = NONE;
		unit->unit.override_emotion_animation_index = NONE;
		unit->unit.magic_seat_index = NONE;
		unit->unit.animation.aiming_with_euler_screen = FALSE;
		memset(&unit->unit.animation.aiming_screen_bounds, 0, sizeof(unit->unit.animation.aiming_screen_bounds));
		unit->unit.animation.looking_with_euler_screen = FALSE;
		memset(&unit->unit.animation.looking_screen_bounds, 0, sizeof(unit->unit.animation.looking_screen_bounds));
		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 393, &unit->object.forward);
		unit->unit.looking_vector = unit->object.forward;
		unit->unit.desired_looking_vector = unit->unit.looking_vector;
		unit->unit.aiming_vector = unit->object.forward;
		unit->unit.desired_aiming_vector = unit->unit.aiming_vector;
		unit->unit.desired_facing_vector = unit->object.forward;
		unit->unit.persistent_control_timer = 0;
		unit->unit.dialogue_index = NONE;
		SET_FLAG(unit->unit.flags, _unit_must_set_up_dialogue_bit, TRUE);
		memset(&unit->unit.speech, 0, sizeof(unit->unit.speech));
		unit->unit.speech.last_speech_finished_time = NONE;
		unit_dialogue_determine_variant(unit_index);
		memset(unit->unit.attackers, NONE, sizeof(unit->unit.attackers));
		unit->unit.last_damage_category = 0;
		unit->unit.delayed_damage_timer = 0;
		unit->unit.delayed_damage_peak = 0.f;
		unit->unit.delayed_damage_attacker_object_index = NONE;
		unit->unit.time_of_death = NONE;
		unit->unit.fake_encounter_index = NONE;
		unit->unit.fake_squad_index = NONE;
		unit->unit.integrated_light_battery = 1.f;
		unit->unit.flaming_death_delay = 0;
		unit->unit.flaming_death_attacker_object_index = NONE;
		unit->unit.killing_spree_count = 0;
		unit->unit.killing_spree_last_time = NONE;

		if (unit_definition->unit.grenade_type >= 0 &&
			unit_definition->unit.grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES &&
			unit_definition->unit.grenade_count >= 0)
		{
			unit->unit.grenade_counts[unit_definition->unit.grenade_type] = (char)unit_definition->unit.grenade_count;
		}

		SET_FLAG(unit->object.flags, _object_dynamic_lighting_recompute_bit, TRUE);
		SET_FLAG(unit->object.flags, _object_static_lighting_recompute_bit, TRUE);

		if (unit_definition->unit.feign_death_threshold > 0.f &&
			unit_definition->unit.feign_death_time > 0.f &&
			unit_definition->unit.feign_death_chance > 0.f)
		{
			SET_FLAG(unit->unit.flags, _unit_feign_death_allowed_bit, real_random() < unit_definition->unit.feign_death_chance);
		}

		if (!game_engine_running() &&
			(unit->object.owner_team_index == 0 || unit->object.owner_team_index == NONE))
		{
			unit->object.owner_team_index = unit_definition->unit.default_team;
		}

		unit_set_or_test_seat_and_weapon_label(unit_index, base_seat_label_get(_unit_base_seat_stand), NULL, TRUE);
		unit_add_initial_weapons(unit_index);

		has_built_in_actor = FALSE;

		for (seat_index = 0; seat_index < unit_definition->unit.seats.count; seat_index++)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, seat_index, struct unit_seat);

			if (seat->built_in_actor_reference.index != NONE)
			{
				has_built_in_actor = TRUE;
				break;
			}
		}

		if (has_built_in_actor)
		{
			ai_create_mounted_weapons_for_unit(unit_index);
		}

		success = TRUE;
	}

	return success;
}

void unit_kill(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_bit, TRUE);

	return;
}

void unit_kill_silent(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_silent_bit, TRUE);

	return;
}

void unit_kill_no_statistics(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_no_statistics_bit, TRUE);

	return;
}

void unit_delete(
	long unit_index)
{
	return;
}

void units_update(
	void)
{
	unit_globals->next_timer = unit_globals->highest_timer;
	unit_globals->highest_timer = 0;
	unit_globals->used_time = 0;

	return;
}

static boolean unit_vectors_are_valid(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = TRUE;

	result = result && valid_real_normal3d(&unit->unit.desired_facing_vector);
	result = result && valid_real_normal3d(&unit->unit.desired_aiming_vector);
	result = result && valid_real_normal3d(&unit->unit.desired_looking_vector);
	result = result && valid_real_vector3d_axes2(&unit->object.forward, &unit->object.up);
	result = result && valid_real_normal3d(&unit->unit.aiming_vector);
	result = result && valid_real_normal3d(&unit->unit.looking_vector);

	return result;
}

#ifdef DEBUG
static void unit_verify_vectors(
	long unit_index,
	char const *debugstring)
{
	char actorbuf[512];

	if (!unit_vectors_are_valid(unit_index))
	{
		struct unit_datum *unit = unit_get(unit_index);

		ai_debug_describe_actor(
			unit->unit.actor_index == NONE ? unit->unit.swarm_actor_index : unit->unit.actor_index,
			unit_index,
			1,
			actorbuf,
			NUMBEROF(actorbuf)
		);
		error(_error_silent, "**** unit_verify_vectors: problems with %s at location %s", actorbuf, debugstring);
		error(
			_error_silent,
			"  object: pos %f %f %f, fwd %f %f %f, up %f %f %f",
			unit->object.position.x,
			unit->object.position.y,
			unit->object.position.z,
			unit->object.forward.i,
			unit->object.forward.j,
			unit->object.forward.k,
			unit->object.up.i,
			unit->object.up.j,
			unit->object.up.k
		);
		error(
			_error_silent,
			"  desired facing %f %f %f, aiming %f %f %f, looking %f %f %f",
			unit->unit.desired_facing_vector.i,
			unit->unit.desired_facing_vector.j,
			unit->unit.desired_facing_vector.k,
			unit->unit.desired_aiming_vector.i,
			unit->unit.desired_aiming_vector.j,
			unit->unit.desired_aiming_vector.k,
			unit->unit.desired_looking_vector.i,
			unit->unit.desired_looking_vector.j,
			unit->unit.desired_looking_vector.k
		);
		error(
			_error_silent,
			"  aiming vector %f %f %f velocity %f %f %f",
			unit->unit.aiming_vector.i,
			unit->unit.aiming_vector.j,
			unit->unit.aiming_vector.k,
			unit->unit.aiming_velocity.i,
			unit->unit.aiming_velocity.j,
			unit->unit.aiming_velocity.k
		);
		error(
			_error_silent,
			"  looking vector %f %f %f velocity %f %f %f",
			unit->unit.looking_vector.i,
			unit->unit.looking_vector.j,
			unit->unit.looking_vector.k,
			unit->unit.looking_velocity.i,
			unit->unit.looking_velocity.j,
			unit->unit.looking_velocity.k
		);
		
		error(_error_silent, "  warning, hex dump follows...");

		error(
			_error_silent,
			"  object: pos %08X %08X %08X, fwd %08X %08X %08X, up %08X %08X %08X",
			*(long *)&unit->object.position.x,
			*(long *)&unit->object.position.y,
			*(long *)&unit->object.position.z,
			*(long *)&unit->object.forward.i,
			*(long *)&unit->object.forward.j,
			*(long *)&unit->object.forward.k,
			*(long *)&unit->object.up.i,
			*(long *)&unit->object.up.j,
			*(long *)&unit->object.up.k
		);
		error(
			_error_silent,
			"  desired facing %08X %08X %08X, aiming %08X %08X %08X, looking %08X %08X %08X",
			*(long *)&unit->unit.desired_facing_vector.i,
			*(long *)&unit->unit.desired_facing_vector.j,
			*(long *)&unit->unit.desired_facing_vector.k,
			*(long *)&unit->unit.desired_aiming_vector.i,
			*(long *)&unit->unit.desired_aiming_vector.j,
			*(long *)&unit->unit.desired_aiming_vector.k,
			*(long *)&unit->unit.desired_looking_vector.i,
			*(long *)&unit->unit.desired_looking_vector.j,
			*(long *)&unit->unit.desired_looking_vector.k
		);
		error(
			_error_silent,
			"  aiming vector %08X %08X %08X velocity %08X %08X %08X",
			*(long *)&unit->unit.aiming_vector.i,
			*(long *)&unit->unit.aiming_vector.j,
			*(long *)&unit->unit.aiming_vector.k,
			*(long *)&unit->unit.aiming_velocity.i,
			*(long *)&unit->unit.aiming_velocity.j,
			*(long *)&unit->unit.aiming_velocity.k
		);
		error(
			_error_silent,
			"  looking vector %08X %08X %08X velocity %08X %08X %08X",
			*(long *)&unit->unit.looking_vector.i,
			*(long *)&unit->unit.looking_vector.j,
			*(long *)&unit->unit.looking_vector.k,
			*(long *)&unit->unit.looking_velocity.i,
			*(long *)&unit->unit.looking_velocity.j,
			*(long *)&unit->unit.looking_velocity.k
		);

		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			594,
			unit_vectors_are_valid(unit_index),
			"unit_verify_vectors FAILURE, see above for details"
		);
	}
	
	return;
}
#endif

boolean unit_update(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	boolean used_time = FALSE;
	boolean jetpack_cheat_active = FALSE;
	boolean light_change_state = FALSE;
	boolean allow_integrated_lights = game_engine_allow_integrated_lights(unit_index);

	profile_enter(unit_update_section);

#ifdef DEBUG
	unit_verify_vectors(unit_index, "unit-update-begin");
#endif

	++unit->unit.timer;

	if (unit_globals->used_time != 0 || unit->unit.timer <= unit_globals->next_timer)
	{
		long timer;

		if (unit_globals->highest_timer > unit->unit.timer)
		{
			timer = unit_globals->highest_timer;
		}
		else
		{
			timer = unit->unit.timer;
		}

		unit_globals->highest_timer = timer;
	}
	else
	{
		used_time = TRUE;
		unit_globals->used_time = TRUE;
		unit->unit.timer = 0;
	}

	if (TEST_FLAG(unit->unit.flags, _unit_running_blindly_bit))
	{
		unit_running_blind(unit_index, &unit->unit.desired_facing_vector);
		unit->unit.desired_aiming_vector = unit->unit.desired_facing_vector;
		unit->unit.desired_looking_vector = unit->unit.desired_facing_vector;
		unit->unit.throttle = *global_forward3d;
		unit->unit.control_flags = 0;
	}
	else if (!TEST_FLAG(unit->unit.flags, _unit_actively_controlled_bit))
	{
		unit->unit.desired_looking_vector = unit->object.forward;
		unit->unit.desired_aiming_vector = unit->object.forward;
		unit->unit.desired_facing_vector = unit->object.forward;
		unit->unit.throttle = *global_zero_vector3d;
		unit->unit.control_flags = 0;
	}

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit))
	{
		boolean has_driver = FALSE;

		if (unit->unit.persistent_control_timer > 0)
		{
			unit->unit.control_flags = unit->unit.persistent_control_flags | unit->unit.control_flags;

			if (TEST_FLAG(unit->unit.persistent_control_flags, _unit_control_weapon_primary_trigger_bit))
			{
				SET_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit, (unit->unit.persistent_control_timer % 7) == 0);
				unit->unit.primary_trigger = 1.f;
			}
			else
			{
				unit->unit.primary_trigger = 0.f;
			}

			if (--unit->unit.persistent_control_timer == 0)
			{
				unit->unit.persistent_control_flags = 0;
			}
		}

		if (!TEST_FLAG(unit->unit.flags, _unit_possessed_by_recording_bit))
		{
			if (unit->unit.driver_object_index != NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				struct unit_datum *driver_unit = unit_get(unit->unit.driver_object_index);

				unit->object.owner_team_index = driver_unit->object.owner_team_index;
				has_driver = TRUE;

				if (driver_unit->unit.player_index != NONE ||
					driver_unit->unit.animation.state != _unit_state_exiting_seat &&
					driver_unit->unit.animation.state != _unit_state_entering_seat)
				{
					match_assert_valid_real_normal3d(
						"c:\\halo\\SOURCE\\units\\units.c",
						696,
						&driver_unit->unit.desired_facing_vector
					);
					unit->unit.control_flags |= driver_unit->unit.control_flags & UNIT_CONTROL_DRIVER_MASK;
					unit->unit.desired_facing_vector = driver_unit->unit.desired_facing_vector;
					unit->unit.throttle = driver_unit->unit.throttle;
				}
			}

			if (unit->unit.gunner_object_index != NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				struct unit_datum *gunner_unit = unit_get(unit->unit.gunner_object_index);

				if (!has_driver)
				{
					unit->object.owner_team_index = gunner_unit->object.owner_team_index;
				}

				if (gunner_unit->unit.player_index != NONE ||
					gunner_unit->unit.animation.state != _unit_state_exiting_seat &&
					gunner_unit->unit.animation.state != _unit_state_entering_seat)
				{
					match_assert_valid_real_normal3d(
						"c:\\halo\\SOURCE\\units\\units.c",
						720,
						&gunner_unit->unit.desired_aiming_vector
					);

					unit->unit.desired_aiming_vector = gunner_unit->unit.desired_aiming_vector;
					unit->unit.desired_looking_vector = gunner_unit->unit.desired_aiming_vector;
					unit->unit.control_flags |= gunner_unit->unit.control_flags & UNIT_CONTROL_GUNNER_MASK;
					unit->unit.primary_trigger = gunner_unit->unit.primary_trigger;
				}
			}

			if ((unit->unit.control_flags & UNIT_CONTROL_GUNNER_MASK) != 0)
			{
				unit->unit.gunner_inactive_ticks = 0;
			}
			else if (unit->unit.gunner_inactive_ticks < CHAR_MAX)
			{
				++unit->unit.gunner_inactive_ticks;
			}
		}

		/*
		* Jetpack cheat combo:
		* Press the following keys while the jetpack cheat is active
		* Grenade throw button
		* Primary weapon fire button
		*/
		if (cheat.jetpack &&
			TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit) &&
			TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit))
		{
			jetpack_cheat_active = TRUE;
		}

		if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
		{
			real active_camo_change_this_tick;

			if (game_engine_running())
			{
				if (unit->unit.cause_for_camo_regrowth == cause_for_camo_regrowth_default)
				{
					active_camo_change_this_tick = 1.f/120.f;
				}
				else if (unit->unit.cause_for_camo_regrowth == cause_for_camo_regrowth_shot_fired)
				{
					long weapon_index = unit_get_current_weapon_index(unit_index);

					if (weapon_index != NONE)
					{
						struct weapon_datum *weapon = weapon_get(weapon_index);
						struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

						if (weapon_definition->weapon.active_camo_regrowth_rate == 0.f)
						{
							active_camo_change_this_tick = 1.f/120.f;
						}
						else
						{
							active_camo_change_this_tick = weapon_definition->weapon.active_camo_regrowth_rate;
						}
					}
					else
					{
						active_camo_change_this_tick = 1.f/120.f;
					}
				}
				else
				{
					active_camo_change_this_tick = 1.f/120.f;
				}
			}
			else
			{
				active_camo_change_this_tick = 1.f/120.f;
			}

			unit->unit.active_camouflage += active_camo_change_this_tick;

			if (unit->unit.active_camouflage > 1.f)
			{
				unit->unit.active_camouflage = 1.f;
				unit->unit.cause_for_camo_regrowth = cause_for_camo_regrowth_default;
			}
		}
		else
		{
			unit->unit.active_camouflage -= 1.f/120.f;

			if (unit->unit.active_camouflage < 0.f)
			{
				unit->unit.active_camouflage = 0.f;
			}
		}

		if (TEST_FLAG(unit->unit.flags, _unit_super_camouflaged_bit))
		{
			unit->unit.active_camouflage_super_amount += 1.f/90.f;

			if (unit->unit.active_camouflage_super_amount > 1.f)
			{
				unit->unit.active_camouflage_super_amount = 1.f;
			}
		}
		else
		{
			unit->unit.active_camouflage_super_amount -= 1.f/90.f;

			if (unit->unit.active_camouflage_super_amount < 0.f)
			{
				unit->unit.active_camouflage_super_amount = 0.f;
			}
		}

		// Update stun timer
		if (unit->unit.body_stun_ticks > 0)
		{
			unit->unit.body_stun_ticks--;

			// Stop stunning when tick timer is done
			if (unit->unit.body_stun_ticks == 0)
			{
				unit->unit.body_stun = 0.f;
			}
		}

		// Drop weapon when drop delay is done
		if (unit->unit.weapon_drop_delay_ticks > 0 && --unit->unit.weapon_drop_delay_ticks == 0)
		{
			unit_drop_current_weapon(unit_index, TRUE);
		}

		// Update feign death timer
		if (unit->unit.feign_death_timer > 0 &&
			TEST_FLAG(unit->object.flags, _object_at_rest_bit))
		{
			unit->unit.feign_death_timer--;

			// Die when the unit has met it's end
			if (!unit->unit.feign_death_timer)
			{
				if (unit->object.body_vitality > 0.f)
				{
					short new_state = TEST_FLAG(unit->unit.animation.flags, _unit_animation_fallen_on_front_bit) ? _unit_state_resurrect_front : _unit_state_resurrect_back;

					SET_FLAG(unit->object.damage_flags, _object_dead_bit, FALSE);

					unit_set_actively_controlled(unit_index, TRUE);
					unit_set_or_test_seat_and_weapon_label(
						unit_index,
						base_seat_label_get(_unit_base_seat_stand),
						NULL,
						TRUE
					);
					unit_animation_set_state(unit_index, new_state);

					SET_FLAG(unit->unit.animation.flags, _unit_animation_ignore_translation_bit, FALSE);

					if (unit->object.type == _object_type_biped)
					{
						biped_stop_limp_body_physics(unit_index);
					}

					unit_scream(unit_index, _unit_scream_resurrection);
				}
				else
				{
					unit_died(unit_index, FALSE);
				}
			}
		}
	}

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_has_no_aiming_bit))
	{
		real aim_scale;
		real aiming_velocity_limit;
		real aiming_angular_acceleration_limit;
		real_vector3d last_aiming_vector;
		real aiming_change;

		if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			if (TEST_FLAG(unit->object.damage_flags, _object_cannot_hold_weapon_bit))
			{
				unit_drop_current_weapon(unit_index, TRUE);
			}
			else if (unit->unit.desired_weapon_index != unit->unit.current_weapon_index &&
				!unit_animation_busy(&unit->unit.animation))
			{
				long weapon_index = unit_get_desired_weapon_index(unit_index);

				if (weapon_index != NONE && unit_can_use_weapon(unit_index, weapon_index))
				{
					unit_ready_desired_weapon(unit_index, TRUE);
				}
			}

			if (unit->unit.desired_grenade_index != unit->unit.current_grenade_index &&
				!unit_animation_busy(&unit->unit.animation))
			{
				short grenade_index = unit_inventory_next_grenade(unit_index, unit->unit.desired_grenade_index, 0);

				if (grenade_index != NONE)
				{
					unit->unit.current_grenade_index = grenade_index;
				}
			}

			if (cheat.infinite_ammo && unit->unit.player_index != NONE)
			{
				short grenade_index;

				for (grenade_index = 0; grenade_index < NUMBEROF(unit->unit.grenade_counts); ++grenade_index)
				{
					unit->unit.grenade_counts[grenade_index] = unit->unit.grenade_counts[grenade_index] > 1 ? unit->unit.grenade_counts[grenade_index] : 1;
				}

				if (unit->unit.desired_grenade_index == NONE)
				{
					unit->unit.desired_grenade_index = 0;
				}
			}

			if (unit->unit.desired_zoom_level != unit->unit.current_zoom_level)
			{
				long local_player_index;

				unit->unit.current_zoom_level = unit->unit.desired_zoom_level;

				if (unit->unit.desired_zoom_level == NONE)
				{
					unit->unit.integrated_night_vision_power = 0.f;
				}

				if (player_index_from_unit_index(unit_index) != NONE)
				{
					local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
				}
				else
				{
					local_player_index = NONE;
				}

				if (local_player_index != NONE)
				{
					long weapon_index = unit_get_current_weapon_index(unit_index);

					if (weapon_index != NONE)
					{
						struct weapon_datum *weapon = weapon_get(weapon_index);
						struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
						long sound_index = unit->unit.current_zoom_level == NONE ? weapon_definition->weapon.zoom_out_sound.index : weapon_definition->weapon.zoom_in_sound.index;
						real scale = 1.f;

						if (unit->unit.current_zoom_level != NONE && weapon_definition->weapon.zoom_level_count > 1)
						{
							scale = (real)unit->unit.current_zoom_level / (real)(weapon_definition->weapon.zoom_level_count - 1);
						}

						if (sound_index != NONE)
						{
							unspatialized_impulse_sound_new(sound_index, scale);
						}
					}
				}
			}
		}

#ifdef DEBUG
		unit_verify_vectors(unit_index, "unit-update-prevectors");
#endif

		aim_scale = unit->unit.aiming_speed == _unit_aiming_speed_casual ? unit_definition->unit.casual_aiming_modifier : 1.f;
		aiming_velocity_limit = (aim_scale * unit_definition->unit.aiming_velocity_maximum) / TICKS_PER_SECOND;
		aiming_angular_acceleration_limit = aim_scale * unit_definition->unit.aiming_acceleration_maximum / (TICKS_PER_SECOND * TICKS_PER_SECOND);
		last_aiming_vector = unit->unit.aiming_vector;

		if (aiming_velocity_limit == 0.f && aiming_angular_acceleration_limit == 0.f)
		{
			match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 993, &unit->unit.desired_aiming_vector);
			
			unit->unit.aiming_vector = unit->unit.desired_aiming_vector;
			unit_clip_to_aiming_bounds(unit_index, &unit->unit.aiming_vector, TRUE);
			unit->unit.aiming_velocity = *global_zero_vector3d;
#ifdef DEBUG
			unit_verify_vectors(unit_index, "unit-update-aim-set");
#endif
		}
		else if (unit->unit.animation.aiming_with_euler_screen)
		{
			real_matrix4x3 object_matrix;

			object_matrix.scale = 1.f;
			object_get_orientation(unit_index, &object_matrix.forward, &object_matrix.up);
			cross_product3d(&object_matrix.up, &object_matrix.forward, &object_matrix.left);
			object_matrix.position = *global_origin3d;

			unit_euler_aiming_update(
				&object_matrix,
				&unit->unit.aiming_vector,
				&unit->unit.desired_aiming_vector,
				&unit->unit.aiming_velocity,
				&unit->unit.animation.aiming_screen_bounds,
				aiming_velocity_limit,
				aiming_angular_acceleration_limit);

#ifdef DEBUG
			unit_verify_vectors(unit_index, "unit-update-aim-euler");
#endif
		}
		else
		{
			angular_accelerate_to_position(
				&unit->unit.aiming_vector,
				&unit->unit.desired_aiming_vector,
				&unit->unit.aiming_velocity,
				aiming_velocity_limit,
				aiming_angular_acceleration_limit);
#ifdef DEBUG
			unit_verify_vectors(unit_index, "unit-update-aim-3d");
#endif
		}

		aiming_change = angle_between_vectors3d(&unit->unit.aiming_vector, &last_aiming_vector) / (unit_definition->unit.aiming_velocity_maximum / TICKS_PER_SECOND);
		aiming_change = PIN(aiming_change, 0.f, 1.f);
		unit->unit.aiming_change = (byte)(aiming_change * 255.f);

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1032, &unit->unit.aiming_vector);

		{
			real looking_velocity_limit = (aim_scale * unit_definition->unit.looking_velocity_maximum) / TICKS_PER_SECOND;
			real looking_angular_acceleration_limit = (aim_scale * unit_definition->unit.looking_acceleration_maximum) / (TICKS_PER_SECOND * TICKS_PER_SECOND);

			if (looking_velocity_limit == 0.f && looking_angular_acceleration_limit == 0.f)
			{
				unit->unit.looking_vector = unit->unit.desired_looking_vector;
				unit_clip_to_aiming_bounds(unit_index, &unit->unit.looking_vector, FALSE);
				unit->unit.looking_velocity = *global_zero_vector3d;

#ifdef DEBUG
				unit_verify_vectors(unit_index, "unit-update-look-set");
#endif
			}
			else if (unit->unit.animation.looking_with_euler_screen)
			{
				real_matrix4x3 object_matrix;

				object_matrix.scale = 1.f;
				object_get_orientation(unit_index, &object_matrix.forward, &object_matrix.up);
				cross_product3d(&object_matrix.up, &object_matrix.forward, &object_matrix.left);
				object_matrix.position = *global_origin3d;

				unit_euler_aiming_update(
					&object_matrix,
					&unit->unit.looking_vector,
					&unit->unit.desired_looking_vector,
					&unit->unit.looking_velocity,
					&unit->unit.animation.looking_screen_bounds,
					looking_velocity_limit,
					looking_angular_acceleration_limit);

#ifdef DEBUG
				unit_verify_vectors(unit_index, "unit-update-look-euler");
#endif
			}
			else
			{
				angular_accelerate_to_position(
					&unit->unit.looking_vector,
					&unit->unit.desired_looking_vector,
					&unit->unit.looking_velocity,
					looking_velocity_limit,
					looking_angular_acceleration_limit);
#ifdef DEBUG
				unit_verify_vectors(unit_index, "unit-update-look-3d");
#endif
			}
		}

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1076,&unit->unit.looking_vector);

#ifdef DEBUG
		unit_verify_vectors(unit_index, "unit-update-postvector");
#endif

		if (!jetpack_cheat_active)
		{
			boolean throw_grenade = TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit);

			switch (unit->unit.grenade_throw_state)
			{
			case _unit_grenade_throw_idle:
				if (throw_grenade)
				{
					unit_throw_grenade_begin(unit_index, NULL);
				}
				break;
			case _unit_grenade_throw_wind_up:
				if (unit->object.animation.state.frame_index >= 2)
				{
					unit_throw_grenade_move_to_hand(unit_index);
				}
				break;
			case _unit_grenade_throw_ending:
				if (unit->unit.animation.state != _unit_state_throw_grenade && !throw_grenade)
				{
					unit->unit.grenade_throw_state = _unit_grenade_throw_idle;
				}
				break;
			case _unit_grenade_throw_in_hand:
				++unit->unit.grenade_throw_ticks;

				if (unit->unit.animation.state != _unit_state_throw_grenade)
				{
					unit_throw_grenade_release(unit_index, TRUE);
				}
				break;
			default:
				break;
			}
		}

		if (unit->unit.current_weapon_index != NONE)
		{
			long flags = 0;
			real primary_trigger = unit->unit.primary_trigger;

			if (unit->unit.current_weapon_index == unit->unit.desired_weapon_index)
			{
				boolean const time_remaining = unit->unit.persistent_control_timer > 0 && TEST_FLAG(unit->unit.persistent_control_flags, _unit_control_weapon_primary_trigger_bit);

				if (!jetpack_cheat_active)
				{
					if (allow_integrated_lights && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit))
					{
						SET_FLAG(flags, _weapon_control_integrated_light_bit, TRUE);
					}

					if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit))
					{
						SET_FLAG(flags, _weapon_control_primary_trigger_bit, TRUE);
					}

					if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_secondary_trigger_bit))
					{
						SET_FLAG(flags, _weapon_control_secondary_trigger_bit, TRUE);
					}
				}

				if (TEST_FLAG(unit_definition_get(unit->definition_index)->unit.flags, _unit_integrated_light_controls_weapon_directly_bit))
				{
					weapon_set_integrated_light_power(unit_get_current_weapon_index(unit_index), unit->unit.integrated_light_power);
				}

				if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_reload_bit))
				{
					SET_FLAG(flags, _weapon_control_reload_bit, TRUE);
				}

				if (unit_animation_busy(&unit->unit.animation) && !time_remaining)
				{
					SET_FLAG(flags, _weapon_control_user_busy_bit, TRUE);
				}

				if (unit->object.type == _object_type_biped && ((struct biped_datum *)unit)->biped.player_melee_ticks > 0)
				{
					SET_FLAG(flags, _weapon_control_user_busy_bit, TRUE);
				}

				if (unit->unit.current_zoom_level != NONE)
				{
					SET_FLAG(flags, _weapon_control_zoomed_bit, TRUE);
				}
			}
			else
			{
				SET_FLAG(flags, _weapon_control_user_switching_weapons_bit, TRUE);
			}

			weapon_owner_update(unit_get_current_weapon_index(unit_index), flags, primary_trigger);
		}
	}

	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\units\\units.c", 1155, &unit->object.forward, &unit->object.up);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1156, &unit->unit.aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1157, &unit->unit.looking_vector);

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit))
	{
		short seat_index;

		if (TEST_FLAG(unit->unit.animation.flags, _unit_animation_showing_acceleration_bit))
		{
			unit_seat_update(unit_index);
			unit->unit.seat_acceleration.i = unit->unit.seat_desired_acceleration.i*0.3f + unit->unit.seat_acceleration.i*0.7f;
			unit->unit.seat_acceleration.j = unit->unit.seat_desired_acceleration.j*0.3f + unit->unit.seat_acceleration.j*0.7f;
			unit->unit.seat_acceleration.k = unit->unit.seat_desired_acceleration.k*0.3f + unit->unit.seat_acceleration.k*0.7f;
		}

		for (seat_index = 0; seat_index < unit_definition->unit.powered_seats.count; ++seat_index)
		{
			boolean powered;
			struct powered_seat_definition *powered_seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.powered_seats, seat_index, struct powered_seat_definition);

			if (seat_index == 0)
			{
				powered = unit->unit.driver_object_index != NONE || TEST_FLAG(unit->unit.flags, _unit_actively_controlled_bit);
			}
			else
			{
				powered = unit->unit.gunner_object_index != NONE && unit->unit.gunner_object_index != unit->unit.driver_object_index;
			}

			if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit) && powered)
			{
				if (unit->unit.seat_power[seat_index] != 1.f)
				{
					unit->unit.seat_power[seat_index] += 1.f / (powered_seat->powerup_time * TICKS_PER_SECOND);

					if (unit->unit.seat_power[seat_index] > 1.f)
					{
						unit->unit.seat_power[seat_index] = 1.f;
					}
				}
			}
			else if (unit->unit.seat_power[seat_index] != 0.f)
			{
				unit->unit.seat_power[seat_index] -= 1.f / (powered_seat->powerdown_time * TICKS_PER_SECOND);

				if (unit->unit.seat_power[seat_index] < 0.f)
				{
					unit->unit.seat_power[seat_index] = 0.f;
				}
			}
		}
	}

	if (unit->unit.delayed_damage_timer > 0)
	{
		unit->unit.delayed_damage_timer--;

		if (unit->unit.delayed_damage_timer == 0)
		{
			ai_handle_damage(
				unit_index,
				unit->unit.delayed_damage_attacker_object_index,
				unit->unit.last_damage_category,
				unit->unit.delayed_damage_peak,
				NULL,
				TRUE);

			unit->unit.last_damage_category = 0;
			unit->unit.delayed_damage_attacker_object_index = NONE;
			unit->unit.delayed_damage_peak = 0.f;
		}
	}

	unit_cause_continuous_melee_damage(unit_index);
	unit_dialogue_update(unit_index);

	if (used_time || unit->unit.player_index != NONE)
	{
		unit_refresh_illumination(unit_index);

#ifdef DEBUG
		if (debug_unit_illumination)
		{
			if (unit->unit.player_index != NONE)
			{
				long current_time = game_time_get();
				static long last_time = NONE;

				if (current_time >= last_time + TICKS_PER_SECOND)
				{
					error(
						_error_silent,
						"player illumination: self %.2f ambient %.2f",
						unit->unit.self_illumination,
						unit->unit.ambient_illumination);
					last_time = current_time;
				}
			}
		}
#endif
	}

	if (unit->unit.flaming_death_delay > 0)
	{
		if (--unit->unit.flaming_death_delay == 0)
		{
			unit_flame_to_death(unit_index);
		}
	}

	interpolate_scalar(&unit->unit.mouth_aperture, 0.f, 0.1f);

	if (TEST_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit))
	{
		if (!TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
		{
			light_change_state = TRUE;
		}

		SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit, FALSE);
	}

	if (TEST_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit))
	{
		if (TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
		{
			light_change_state = TRUE;
		}

		SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit, FALSE);
	}

	if (allow_integrated_lights &&
		TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit) ||
		unit->unit.integrated_light_battery <= 0.f ||
		light_change_state)
	{
		boolean const active = unit_integrated_night_vision_is_active(unit_index);

		if (active && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit))
		{
			long effect_index;
			struct game_globals *game_globals;
			struct game_globals_first_person_interface *game_globals_first_person_interface;

			game_globals = scenario_get_game_globals();

			match_assert("c:\\halo\\SOURCE\\units\\units.c", 1302, game_globals);
			game_globals_first_person_interface = TAG_BLOCK_GET_ELEMENT(&game_globals->first_person_interface, 0, struct game_globals_first_person_interface);
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 1304, game_globals_first_person_interface);

			if (TEST_FLAG(unit->unit.flags, _unit_integrated_night_vision_on_bit))
			{
				effect_index = game_globals_first_person_interface->night_vision_on_off_effect.index;
			}
			else
			{
				effect_index = game_globals_first_person_interface->night_vision_off_on_effect.index;
			}

			if (effect_index != NONE)
			{
				effect_new_from_object(effect_index, unit_index, unit_index, NONE, 0.f, 0.f, NULL, NULL);
			}

			SWAP_FLAG(unit->unit.flags, _unit_integrated_night_vision_on_bit);
		}

		if (!(active && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit)) &&
			(TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit) || unit->unit.integrated_light_battery > 0.2f) &&
			unit->object.parent_object_index == NONE)
		{
			effect_new_from_object(
				unit_definition->unit.integrated_light_toggle_effect.index,
				unit_index,
				unit_index,
				NONE,
				0.f,
				0.f,
				NULL,
				NULL);
			SWAP_FLAG(unit->unit.flags, _unit_integrated_light_on_bit);
		}
	}

	if (TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
	{
		if (!TEST_FLAG(unit_definition->unit.flags, _unit_integrated_light_fucking_lasts_forever_bit))
		{
			unit->unit.integrated_light_battery = unit->unit.integrated_light_battery - 1.f / (2.f * TICKS_PER_MINUTE);
		}

		if (unit->object.parent_object_index != NONE || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			SET_FLAG(unit->unit.flags, _unit_integrated_light_on_bit, FALSE);
		}

		if (unit->unit.integrated_light_power != 1.f)
		{
			unit->unit.integrated_light_power += 1.f / 6.f;

			if (unit->unit.integrated_light_power > 1.f)
			{
				unit->unit.integrated_light_power = 1.f;
			}
		}
	}
	else
	{
		if (unit->unit.integrated_light_battery < 1.f)
		{
			unit->unit.integrated_light_battery += 1.f / 900.f;
		}

		if (unit->unit.integrated_light_power != 0.f)
		{
			unit->unit.integrated_light_power -= 1.f / 24.f;

			if (unit->unit.integrated_light_power < 0.f)
			{
				unit->unit.integrated_light_power = 0.f;
			}
		}
	}

	if (unit_integrated_night_vision_is_active(unit_index))
	{
		if (TEST_FLAG(unit->unit.flags, _unit_integrated_night_vision_on_bit))
		{
			if (unit->unit.integrated_night_vision_power != 1.f)
			{
				unit->unit.integrated_night_vision_power += 1.f / 12.f;

				if (unit->unit.integrated_night_vision_power > 1.f)
				{
					unit->unit.integrated_night_vision_power = 1.f;
				}
			}
		}
		else if (unit->unit.integrated_night_vision_power != 0.f)
		{
			unit->unit.integrated_night_vision_power -= 1.f / 24.f;

			if (unit->unit.integrated_night_vision_power < 0.f)
			{
				unit->unit.integrated_night_vision_power = 0.f;
			}
		}
	}

#ifdef DEBUG
	unit_verify_vectors(unit_index, "unit-update-end");
#endif
	
	profile_exit(unit_update_section);
	
	return TRUE;
}

char const *base_seat_labels[NUMBER_OF_UNIT_BASE_SEATS] = {"asleep", "alert", "stand", "crouch", "flee", "flaming"};

void unit_export_function_values(
	long unit_index)
{
	short function_index;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *definition = unit_definition_get(unit->definition_index);

	for (function_index = 0; function_index < NUMBEROF(definition->unit.function_modes); function_index++)
	{
		if (definition->unit.function_modes[function_index] != _unit_function_none)
		{
			real value = 0.f;

			switch (definition->unit.function_modes[function_index])
			{
			case _unit_function_driver_seat_power:
				value = unit->unit.seat_power[0];
				break;
			case _unit_function_gunner_seat_power:
				value = unit->unit.seat_power[1];
				break;
			case _unit_function_aiming_change:
				value = unit->unit.aiming_change / 255.f;
				break;
			case _unit_function_mouth_aperture:
				value = unit->unit.mouth_aperture;
				break;
			case _unit_function_integrated_light_power:
				value = unit->unit.integrated_light_power;
				break;
			case _unit_function_can_blink:
				value = (TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || TEST_FLAG(unit->unit.flags, _unit_cannot_blink_bit)) ? 0.f : 1.f;
				break;
			case _unit_function_shield_sapping:
				{
					struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
					struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);
					short loop_frame_index = animation_loop_frame_index(animation);

					if (unit->object.animation.state.index >= loop_frame_index)
					{
						value = 1.f - unit->unit.shield_sap_timeout / 90.f;
					}
					else
					{
						value = (real)unit->object.animation.state.index / loop_frame_index;
					}
				}
				break;
			}

			unit->object.incoming_function_values[function_index] = value;
		}
	}

	return;
}

void unit_control(
	long unit_index,
	struct unit_control_data *control_data)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1505, magnitude3d(&control_data->throttle)<=3.0f);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1506, control_data->animation_state>=0 && control_data->animation_state<NUMBER_OF_UNIT_ANIMATION_STATES);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1507, control_data->aiming_speed>=0 && control_data->aiming_speed<NUMBER_OF_UNIT_AIMING_SPEEDS);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1508, VALID_FLAGS(control_data->control_flags, NUMBER_OF_UNIT_CONTROL_FLAGS));
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1509, &control_data->facing_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1510, &control_data->aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1511, &control_data->looking_vector);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1512, control_data->weapon_index==NONE || (control_data->weapon_index>=0 && control_data->weapon_index<MAXIMUM_WEAPONS_PER_UNIT));
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1513, control_data->grenade_index==NONE || (control_data->grenade_index>=0 && control_data->grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES));
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1514, control_data->zoom_level==NONE || (control_data->zoom_level>=0));
	match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 1515, control_data->primary_trigger);

	unit->unit.throttle = control_data->throttle;
	unit->unit.primary_trigger = control_data->primary_trigger;
	unit->unit.aiming_speed = control_data->aiming_speed;

	if (control_data->weapon_index != NONE)
	{
		unit->unit.desired_weapon_index = control_data->weapon_index;
	}

	if (control_data->grenade_index != NONE)
	{
		unit->unit.desired_grenade_index = (char)control_data->grenade_index;
	}

	unit->unit.desired_zoom_level = (char)control_data->zoom_level;
	unit->unit.control_flags = control_data->control_flags;
	unit->unit.desired_looking_vector = control_data->looking_vector;
	unit->unit.desired_aiming_vector = control_data->aiming_vector;
	unit->unit.desired_facing_vector = control_data->facing_vector;
	unit->unit.animation.desired_state = control_data->animation_state;

#ifdef DEBUG
	unit_verify_vectors(unit_index, "unit-control");
#endif

	return;
}

void unit_persistent_control(
	long unit_index,
	long persistent_control_timer,
	unsigned long persistent_control_flags)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1541, VALID_FLAGS(persistent_control_flags, NUMBER_OF_UNIT_CONTROL_FLAGS));

	unit->unit.persistent_control_timer = persistent_control_timer;
	unit->unit.persistent_control_flags = persistent_control_flags;

	return;
}

void unit_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations)
{
	struct unit_datum *unit = unit_get(object_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);

	if (unit->unit.animation.action_animation.index != NONE)
	{
		replacement_animation_apply(
			TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->unit.animation.action_animation.index, struct animation),
			unit->unit.animation.action_animation.frame_index,
			node_orientations);
	}

	if (unit->unit.animation.overlay_action_animation.index != NONE)
	{
		overlay_animation_apply(
			TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->unit.animation.overlay_action_animation.index, struct animation),
			unit->unit.animation.overlay_action_animation.frame_index,
			node_orientations);
	}

	if (unit->unit.animation.soft_ping_animation.index != NONE)
	{
		overlay_animation_apply(
			TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->unit.animation.soft_ping_animation.index, struct animation),
			unit->unit.animation.soft_ping_animation.frame_index,
			node_orientations);
	}

#ifdef DEBUG
	unit_verify_vectors(object_index, "unit-preprocess-nodes");
#endif

	unit->unit.animation.aiming_with_euler_screen = FALSE;
	unit->unit.animation.looking_with_euler_screen = FALSE;

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit) && unit->unit.animation.seat_index != NONE)
	{
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);

		if (unit->unit.animation.emotion_index != NONE)
		{
			short animation_index = unit_seat->animations.count > _unit_seat_animation_emotions ?
				animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_emotions].animation_index :
				NONE;

			if (unit->unit.override_emotion_animation_index != NONE)
			{
				animation_index = unit->unit.override_emotion_animation_index;
			}

			if (animation_index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

				if (unit->unit.animation.emotion_index >= 0 && unit->unit.animation.emotion_index < animation->frame_count)
				{
					overlay_animation_apply(animation, unit->unit.animation.emotion_index, node_orientations);
				}
			}
		}

		if (unit->unit.mouth_aperture > 0.f)
		{
			short animation_index = unit_seat->animations.count <= _unit_seat_animation_mouth_aperture ?
				NONE :
				animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_mouth_aperture].animation_index;

			if (animation_index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

				overlay_animation_apply_scaled(animation, 0, unit->unit.mouth_aperture, node_orientations);
			}
		}

		if (TEST_FLAG(unit->unit.animation.flags, _unit_animation_showing_acceleration_bit))
		{
			short axis_index;

			for (axis_index = 0; axis_index < 3; axis_index++)
			{
				short animation_index;

				if (_unit_seat_animation_acceleration_front_back + axis_index < 0 || _unit_seat_animation_acceleration_front_back + axis_index >= unit_seat->animations.count)
				{
					animation_index = NONE;
				}
				else
				{
					animation_index = animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_acceleration_front_back + axis_index].animation_index;
				}

				if (animation_index != NONE)
				{
					struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

					overlay_animation_apply_continuous(animation, (animation->frame_count - 1) * unit->unit.seat_acceleration.n[axis_index], node_orientations);
				}
			}
		}

		if (!TEST_FLAG(unit_definition->unit.flags, _unit_has_no_aiming_bit) && unit_animation_aiming_screen(&unit->unit.animation))
		{
			struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
			real_euler_angles2d relative_aiming_angles = *global_zero_angles2d;

			if (unit->unit.animation.aiming_screen_index != NONE)
			{
				real_matrix4x3 matrix;
				real_vector3d relative_aiming_vector;
				struct animation_aiming_screen_bounds *aiming_screen = &weapon_class->aiming_screen_bounds;

				match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1648, &unit->unit.aiming_vector);

				matrix.scale = 1.f;
				object_get_orientation(object_index, &matrix.forward, &matrix.up);
				cross_product3d(&matrix.up, &matrix.forward, &matrix.left);
				matrix.position = *global_origin3d;
				matrix4x3_inverse_transform_normal(&matrix, &unit->unit.aiming_vector, &relative_aiming_vector);
				match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 1661, &relative_aiming_vector);
				euler_angles2d_from_vector3d(&relative_aiming_angles, &relative_aiming_vector);

				match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 1664, relative_aiming_angles.pitch);
				match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 1665, relative_aiming_angles.yaw);

				unit->unit.animation.aiming_with_euler_screen = TRUE;
				unit->unit.animation.aiming_screen_bounds.x0 = -aiming_screen->negative_yaw_delta * aiming_screen->negative_yaw_frame_count;
				unit->unit.animation.aiming_screen_bounds.x1 = aiming_screen->positive_yaw_frame_count * aiming_screen->positive_yaw_delta;
				unit->unit.animation.aiming_screen_bounds.y0 = -aiming_screen->negative_pitch_delta * aiming_screen->negative_pitch_frame_count;
				unit->unit.animation.aiming_screen_bounds.y1 = aiming_screen->positive_pitch_frame_count * aiming_screen->positive_pitch_delta;
				aiming_screen_apply(
					TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->unit.animation.aiming_screen_index, struct animation),
					aiming_screen,
					relative_aiming_angles.yaw,
					relative_aiming_angles.pitch,
					node_orientations);
			}

			if (unit->unit.current_weapon_index != NONE || unit->unit.player_index != NONE)
			{
				struct animation_aiming_screen_bounds *looking_screen = &unit_seat->looking_screen_bounds;

				if (unit->unit.animation.looking_screen_index != NONE)
				{
					real_matrix4x3 matrix;
					real_euler_angles2d relative_looking_angles;
					real_vector3d relative_looking_vector;

					matrix.scale = 1.f;
					object_get_orientation(object_index, &matrix.forward, &matrix.up);
					cross_product3d(&matrix.up, &matrix.forward, &matrix.left);
					matrix.position = *global_origin3d;
					matrix4x3_inverse_transform_normal(&matrix, &unit->unit.looking_vector, &relative_looking_vector);
					match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 1703, &relative_looking_vector);
					euler_angles2d_from_vector3d(&relative_looking_angles, &relative_looking_vector);
					relative_looking_angles.yaw -= relative_aiming_angles.yaw;
					relative_looking_angles.pitch -= relative_aiming_angles.pitch;
					match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 1710, relative_looking_angles.pitch);
					match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 1711, relative_looking_angles.yaw);

					unit->unit.animation.looking_with_euler_screen = TRUE;
					unit->unit.animation.looking_screen_bounds.x0 = -looking_screen->negative_yaw_delta * looking_screen->negative_yaw_frame_count;
					unit->unit.animation.looking_screen_bounds.x1 = looking_screen->positive_yaw_frame_count * looking_screen->positive_yaw_delta;
					unit->unit.animation.looking_screen_bounds.y0 = -looking_screen->negative_pitch_delta * looking_screen->negative_pitch_frame_count;
					unit->unit.animation.looking_screen_bounds.y1 = looking_screen->positive_pitch_frame_count * looking_screen->positive_pitch_delta;
					aiming_screen_apply(
						TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->unit.animation.looking_screen_index, struct animation),
						looking_screen,
						relative_looking_angles.yaw,
						relative_looking_angles.pitch,
						node_orientations);
				}
			}
		}
	}

	return;
}

boolean unit_get_seat_entrance_point(
	long unit_index,
	long parent_unit_index,
	short seat_index,
	real_point3d *entrance_point,
	real_point3d *seat_point,
	real_point3d *hint_point)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct model *model = model_definition_get(unit_definition->object.model.index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	struct unit_datum *parent_unit = unit_get(parent_unit_index);
	struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
	struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, seat_index, struct unit_seat);
	short unit_seat_index = NONE;
	boolean success = FALSE;

	for (index = 0; index < animation_graph->unit_seats.count; index++)
	{
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, index, struct animation_graph_unit_seat);

		if (!_stricmp(unit_seat->label, seat->label))
		{
			unit_seat_index = index;
			break;
		}
	}

	if (unit_seat_index != NONE)
	{
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit_seat_index, struct animation_graph_unit_seat);
		short animation_index = unit_seat->animations.count <= _unit_seat_animation_seat_enter ?
			NONE :
			animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_seat_enter].animation_index;

		if (animation_index != NONE)
		{
			real_matrix4x3 root_matrix;
			real_matrix4x3 seat_matrix;
			struct object_marker seat_marker;
			struct object_marker hint_marker;
			char hint_name[256];
			struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

			object_get_marker_by_name(parent_unit_index, seat->marker_name, &seat_marker, 1);
			animation_get_root_matrix(model, animation, 0, &root_matrix);
			matrix4x3_multiply(&seat_marker.matrix, &root_matrix, &seat_matrix);
			strcpy(hint_name, seat->marker_name);
			strcat(hint_name, " enter-hint");
			object_get_marker_by_name(parent_unit_index, hint_name, &hint_marker, 1);

			if (seat_point)
			{
				*seat_point = seat_marker.matrix.position;
			}

			if (entrance_point)
			{
				*entrance_point = seat_matrix.position;
			}

			if (hint_point)
			{
				*hint_point = hint_marker.matrix.position;
			}

			success = TRUE;
		}
	}

	return success;
}

boolean unit_get_melee_range_and_ticks(
	long unit_index,
	boolean airborne,
	short *start_tick_count,
	real *start_range,
	short *end_tick_count,
	real *end_range)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct model *model = model_definition_get(unit_definition->object.model.index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
	struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
	long weapon_class_animation_index = airborne ? _unit_weapon_class_animation_melee_airborne : _unit_weapon_class_animation_melee_attack;
	short animation_index = weapon_class_animation_index >= 0 && weapon_class_animation_index < weapon_class->animations.count ?
		animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index :
		NONE;
	boolean success = FALSE;

	if (animation_index != NONE)
	{
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

		animation_get_x_offsets(animation, start_range, end_range);

		if (start_tick_count)
		{
			*start_tick_count = animation_key_frame_index(animation);
		}

		if (end_tick_count)
		{
			*end_tick_count = animation->frame_count;
		}

		success = TRUE;
	}

	return success;
}

short unit_get_animation_frames_remaining(
	long unit_index,
	short *animation_state)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
	struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1848, animation_state);

	*animation_state = unit->unit.animation.state;

	return animation->frame_count - unit->object.animation.state.frame_index;
}

void unit_postprocess_node_matrices(
	long object_index,
	struct real_matrix4x3 *node_matrices)
{
	struct unit_datum *unit = unit_get(object_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit) && unit->unit.animation.seat_index != NONE)
	{
		struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
		struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);

		if (unit->object.parent_object_index != NONE && unit_animation_vehicle_ik(&unit->unit.animation))
		{
			short ik_point_index;

			for (ik_point_index = 0; ik_point_index < unit_seat->ik_points.count; ik_point_index++)
			{
				struct animation_graph_ik_point *ik_point = TAG_BLOCK_GET_ELEMENT(&unit_seat->ik_points, ik_point_index, struct animation_graph_ik_point);

				object_inverse_kinematics(
					object_index,
					ik_point->marker_name,
					unit->object.parent_object_index,
					ik_point->attached_to_marker_name,
					node_matrices);
			}
		}

		if (unit->unit.current_weapon_index != NONE && unit_animation_weapon_ik(&unit->unit.animation))
		{
			short ik_point_index;

			for (ik_point_index = 0; ik_point_index < weapon_class->ik_points.count; ik_point_index++)
			{
				struct animation_graph_ik_point *ik_point = TAG_BLOCK_GET_ELEMENT(&weapon_class->ik_points, ik_point_index, struct animation_graph_ik_point);

				object_inverse_kinematics(
					object_index,
					ik_point->marker_name,
					unit_get_current_weapon_index(object_index),
					ik_point->attached_to_marker_name,
					node_matrices);
			}

			SET_FLAG(unit->unit.animation.flags, _unit_animation_postpone_weapon_ik_until_interpolation_ends_bit, FALSE);
		}
	}

	return;
}

static void unit_euler_axis_buildplan(
	real current_location,
	real current_velocity,
	real velocity_limit,
	real acceleration_limit,
	struct euler_axis_plan *plan)
{
	plan->accel_a = REAL_MAX;
	plan->accel_t = REAL_MAX;
	plan->coast_t = REAL_MAX;
	plan->decel_a = REAL_MAX;
	plan->decel_t = REAL_MAX;
	plan->initial_p = current_location;
	plan->initial_v = current_velocity;
	plan->stopped = fabs(plan->initial_p) < 1e-3f && fabs(plan->initial_v) < 1e-3f;

	if (plan->stopped)
	{
		plan->accel_a = 0.f;
		plan->accel_t = 0.f;
		plan->coast_t = 0.f;
		plan->decel_a = 0.f;
		plan->decel_t = 0.f;
	}
	else
	{
		real stop_t = fabs(plan->initial_v) / acceleration_limit;
		boolean positive = plan->initial_v > 0.f;

		if (plan->initial_v * (stop_t / 2.f) / 2.f + plan->initial_p < 0.f)
		{
			unit_euler_axis_buildplan(-current_location, -current_velocity, velocity_limit, acceleration_limit, plan);
			plan->initial_p *= -1.f;
			plan->initial_v *= -1.f;
			plan->accel_a *= -1.f;
			plan->decel_a *= -1.f;
		}
		else
		{
			real stop_p = stop_t * (plan->initial_v / 2.f) + plan->initial_p;

			if (stop_p < 0.f)
			{
				match_assert("c:\\halo\\SOURCE\\units\\units.c", 1975, plan->initial_p > -1e-03f);
				match_assert("c:\\halo\\SOURCE\\units\\units.c", 1976, plan->initial_v < 0);

				plan->accel_a = 0.f;
				plan->accel_t = 0.f;
				plan->decel_a = plan->initial_v * plan->initial_v / (2.f * plan->initial_p);
				plan->decel_t = -plan->initial_v / plan->decel_a;
				plan->coast_t = 0.f;
			}
			else
			{
				real t;
				real actual_t;

				if (positive)
				{
					t = square_root(stop_p / acceleration_limit);
				}
				else
				{
					real t1;
					real t2;
					real a = -acceleration_limit;
					real b = 2.f * plan->initial_v;
					real c = stop_p;
					real disc = b * b - 4.f * a * c;

					match_assert("c:\\halo\\SOURCE\\units\\units.c", 2027, disc >= 0);

					t1 = (-b - square_root(disc)) / (2.f * a);
					t2 = (square_root(disc) - b) / (2.f * a);

					if (t1 >= 0.f && (t2 < 0.f || t1 < t2))
					{
						t = t1;
					}
					else
					{
						t = MAX(0.f, t2);
					}
				}

				match_assert("c:\\halo\\SOURCE\\units\\units.c", 2042, t >= 0);

				if (velocity_limit > 0.f)
				{
					real maximum_t = (positive ? velocity_limit : velocity_limit + plan->initial_v) / acceleration_limit;

					maximum_t = MAX(0.f, maximum_t);
					actual_t = MIN(t, maximum_t);
				}
				else
				{
					actual_t = t;
				}

				plan->accel_a = -acceleration_limit;
				plan->decel_a = acceleration_limit;

				if (positive)
				{
					plan->accel_t = actual_t + stop_t;
					plan->decel_t = actual_t;
				}
				else
				{
					plan->accel_t = actual_t;
					plan->decel_t = actual_t + stop_t;
				}

				if (actual_t < t)
				{
					real coasting_vel = plan->accel_a * plan->accel_t + plan->initial_v;
					real coast_p = (t - actual_t) * coasting_vel * 2.f - (t - actual_t) * (t - actual_t) * acceleration_limit;

					match_assert("c:\\halo\\SOURCE\\units\\units.c", 2128, coasting_vel < 0);
					plan->coast_t = coast_p / coasting_vel;
					match_assert("c:\\halo\\SOURCE\\units\\units.c", 2130, plan->coast_t >= 0);
					match_assert("c:\\halo\\SOURCE\\units\\units.c", 2131, plan->coast_t + actual_t >= t);
				}
				else
				{
					plan->coast_t = 0.f;
				}
			}
		}

		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2140, REAL_MAX != plan->accel_a);
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2141, REAL_MAX != plan->accel_t);
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2142, REAL_MAX != plan->coast_t);
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2143, REAL_MAX != plan->decel_a);
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2144, REAL_MAX != plan->decel_t);
	}

	return;
}

static void unit_euler_axis_couple(
	struct euler_axis_plan *plan1,
	struct euler_axis_plan *plan2,
	real velocity_limit,
	real acceleration_limit)
{
	real t_extension;
	struct euler_axis_plan *adjust_plan = NULL;

	if (!plan1->stopped && !plan2->stopped)
	{
		real plan1_t = plan1->decel_t + plan1->coast_t + plan1->accel_t;
		real plan2_t = plan2->decel_t + plan2->coast_t + plan2->accel_t;

		if (plan1->accel_t > 0.f && plan1_t < plan2_t)
		{
			adjust_plan = plan1;
			t_extension = plan2_t - plan1_t;
		}
		else if (plan2->accel_t > 0.f && plan2_t < plan1_t)
		{
			adjust_plan = plan2;
			t_extension = plan1_t - plan2_t;
		}
	}

	if (adjust_plan)
	{
		real a;
		real b;
		real c;
		real disc;
		real t;

		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2222, t_extension > 0);

		a = acceleration_limit;
		b = (adjust_plan->coast_t + t_extension) * acceleration_limit;
		c = -t_extension * fabs(adjust_plan->accel_a * adjust_plan->accel_t + adjust_plan->initial_v);
		disc = b * b - 4.f * a * c;
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2244, disc >= 0);
		t = (-b + square_root(disc)) / (2.f * a);
		t = MIN(t, MIN(adjust_plan->accel_t, adjust_plan->decel_t));

		if (t > 0.f)
		{
			real new_v = (adjust_plan->accel_t - t) * adjust_plan->accel_a + adjust_plan->initial_v;

			adjust_plan->accel_t -= t;
			adjust_plan->decel_t -= t;
			adjust_plan->coast_t = (2.f * new_v * t + adjust_plan->accel_a * t * t) / new_v;
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 2264, (adjust_plan->accel_t >= 0) && (adjust_plan->decel_t >= 0));
		}
	}

	return;
}

static boolean unit_euler_axis_doplan(
	struct euler_axis_plan *plan,
	real delta_t,
	real current_location,
	real *out_location,
	real current_velocity,
	real *out_velocity)
{
	boolean stopped = plan->stopped;
	real location = current_location;
	real velocity = current_velocity;

	if (!stopped)
	{
		real t = delta_t;

		if (t > 0.f && plan->accel_t > 0.f)
		{
			real dt = MIN(t, plan->accel_t);

			location = velocity * dt + plan->accel_a * dt * dt / 2.f + location;
			velocity = plan->accel_a * dt + velocity;
			t -= dt;
		}

		if (t > 0.f && plan->coast_t > 0.f)
		{
			real dt = MIN(t, plan->coast_t);

			location = velocity * dt + location;
			t -= dt;
		}

		if (t > 0.f && plan->decel_t > 0.f)
		{
			real dt = MIN(t, plan->decel_t);

			location = velocity * dt + plan->decel_a * dt * dt / 2.f + location;
			velocity = plan->decel_a * dt + velocity;
			t -= dt;
		}

		if (t > 0.f)
		{
			stopped = TRUE;
		}
	}

	*out_location = location;
	*out_velocity = velocity;

	return stopped;
}

void unit_euler_aiming_update(
	real_matrix4x3 const *orientation,
	real_vector3d *aiming_vector,
	real_vector3d const *desired_aiming_vector,
	real_vector3d *aiming_velocity,
	real_rectangle2d const *aiming_bounds,
	real angular_velocity_limit,
	real angular_acceleration_limit)
{
	real_vector3d achievable_desired_aiming_vector;
	real_vector3d relative_aiming_vector;
	real_vector3d relative_desired_aiming_vector;
	real_vector3d relative_final_vector;
	real_euler_angles2d aiming_angles;
	real_euler_angles2d relative_angles;
	real_euler_angles2d end_aiming_angles;
	real_euler_angles2d desired_aiming_angles;
	real_euler_angles2d angular_velocity;
	real_euler_angles2d end_angular_velocity;
	struct euler_axis_plan yaw_plan;
	struct euler_axis_plan pitch_plan;
	boolean circular;
	boolean achievable;
	boolean yaw_stopped;
	boolean pitch_stopped;

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 2402, angular_velocity_limit >= 0.0f);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 2403, (angular_acceleration_limit > 0.0f) && (angular_acceleration_limit < 10000.0f));

	if (orientation)
	{
		matrix4x3_inverse_transform_normal(orientation, aiming_vector, &relative_aiming_vector);
		matrix4x3_inverse_transform_normal(orientation, desired_aiming_vector, &relative_desired_aiming_vector);
	}
	else
	{
		relative_aiming_vector = *aiming_vector;
		relative_desired_aiming_vector = *desired_aiming_vector;
	}

	circular = aiming_bounds->x1 - aiming_bounds->x0 - 2.f * _pi > -0.0001f;
	euler_angles2d_from_vector3d(&aiming_angles, &relative_aiming_vector);
	euler_angles2d_from_vector3d(&desired_aiming_angles, &relative_desired_aiming_vector);
	achievable = TRUE;

	if (circular)
	{
		if (desired_aiming_angles.yaw < aiming_bounds->x0)
		{
			desired_aiming_angles.yaw += 2.f * _pi;
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 2433, desired_aiming_angles.yaw <= aiming_bounds->x1);
		}
		else if (desired_aiming_angles.yaw > aiming_bounds->x1)
		{
			desired_aiming_angles.yaw -= 2.f * _pi;
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 2438, desired_aiming_angles.yaw >= aiming_bounds->x0);
		}
	}
	else
	{
		if (desired_aiming_angles.yaw < aiming_bounds->x0)
		{
			desired_aiming_angles.yaw = aiming_bounds->x0;
			achievable = FALSE;
		}
		else if (desired_aiming_angles.yaw > aiming_bounds->x1)
		{
			desired_aiming_angles.yaw = aiming_bounds->x1;
			achievable = FALSE;
		}
	}

	if (desired_aiming_angles.pitch < aiming_bounds->y0)
	{
		desired_aiming_angles.pitch = aiming_bounds->y0;
		achievable = FALSE;
	}
	else if (desired_aiming_angles.pitch > aiming_bounds->y1)
	{
		desired_aiming_angles.pitch = aiming_bounds->y1;
		achievable = FALSE;
	}

	if (achievable)
	{
		achievable_desired_aiming_vector = *desired_aiming_vector;
	}
	else
	{
		vector3d_from_euler_angles2d(&achievable_desired_aiming_vector, &desired_aiming_angles);

		if (orientation)
		{
			matrix4x3_transform_normal(orientation, &achievable_desired_aiming_vector, &achievable_desired_aiming_vector);
			normalize3d(&achievable_desired_aiming_vector);
		}
	}

	{
		real_vector3d velocity_axis = *aiming_velocity;
		real angular_speed = normalize3d(&velocity_axis);

		if (angular_speed != 0.f)
		{
			real_vector3d rotated_aiming_vector = relative_aiming_vector;
			real_euler_angles2d rotated_aiming_angles;

			rotate_vector_about_axis(&rotated_aiming_vector, &velocity_axis, sine(angular_speed), cosine(angular_speed));
			euler_angles2d_from_vector3d(&rotated_aiming_angles, &rotated_aiming_vector);
			angular_velocity.yaw = rotated_aiming_angles.yaw - aiming_angles.yaw;
			angular_velocity.pitch = rotated_aiming_angles.pitch - aiming_angles.pitch;
		}
		else
		{
			angular_velocity.yaw = 0.f;
			angular_velocity.pitch = 0.f;
		}
	}

	relative_angles.yaw = aiming_angles.yaw - desired_aiming_angles.yaw;
	relative_angles.pitch = aiming_angles.pitch - desired_aiming_angles.pitch;

	if (circular)
	{
		if (relative_angles.yaw > _pi)
		{
			relative_angles.yaw -= 2.f * _pi;
		}
		else if (relative_angles.yaw < -_pi)
		{
			relative_angles.yaw += 2.f * _pi;
		}
	}

	unit_euler_axis_buildplan(relative_angles.yaw, angular_velocity.yaw, angular_velocity_limit, angular_acceleration_limit, &yaw_plan);
	unit_euler_axis_buildplan(
		relative_angles.pitch,
		angular_velocity.pitch,
		angular_velocity_limit,
		angular_acceleration_limit,
		&pitch_plan);
	unit_euler_axis_couple(&yaw_plan, &pitch_plan, angular_velocity_limit, angular_acceleration_limit);
	yaw_stopped = unit_euler_axis_doplan(
		&yaw_plan,
		1.f,
		relative_angles.yaw,
		&relative_angles.yaw,
		angular_velocity.yaw,
		&end_angular_velocity.yaw);
	pitch_stopped = unit_euler_axis_doplan(
		&pitch_plan,
		1.f,
		relative_angles.pitch,
		&relative_angles.pitch,
		angular_velocity.pitch,
		&end_angular_velocity.pitch);

	if (yaw_stopped && pitch_stopped)
	{
		*aiming_vector = achievable_desired_aiming_vector;
		*aiming_velocity = *global_zero_vector3d;
	}
	else
	{
		real cosine_angle;

		end_aiming_angles.yaw = relative_angles.yaw + desired_aiming_angles.yaw;
		end_aiming_angles.pitch = relative_angles.pitch + desired_aiming_angles.pitch;

		if (circular)
		{
			if (end_aiming_angles.yaw < aiming_bounds->x0)
			{
				end_aiming_angles.yaw += 2.f * _pi;
				match_assert("c:\\halo\\SOURCE\\units\\units.c", 2725, end_aiming_angles.yaw <= aiming_bounds->x1);
			}
			else if (end_aiming_angles.yaw > aiming_bounds->x1)
			{
				end_aiming_angles.yaw -= 2.f * _pi;
				match_assert("c:\\halo\\SOURCE\\units\\units.c", 2730, end_aiming_angles.yaw >= aiming_bounds->x0);
			}
		}
		else
		{
			end_aiming_angles.yaw = PIN(end_aiming_angles.yaw, aiming_bounds->x0, aiming_bounds->x1);
		}

		end_aiming_angles.pitch = PIN(end_aiming_angles.pitch, aiming_bounds->y0, aiming_bounds->y1);

		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2739, (end_aiming_angles.yaw >= aiming_bounds->x0) && (end_aiming_angles.yaw <= aiming_bounds->x1));
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2740, (end_aiming_angles.pitch >= aiming_bounds->y0) && (end_aiming_angles.pitch <= aiming_bounds->y1));

		vector3d_from_euler_angles2d(&relative_final_vector, &end_aiming_angles);

		{
			real_euler_angles2d further_angles;
			real_vector3d further_vector;

			further_angles.yaw = end_angular_velocity.yaw + end_aiming_angles.yaw;
			further_angles.pitch = end_angular_velocity.pitch + end_aiming_angles.pitch;
			vector3d_from_euler_angles2d(&further_vector, &further_angles);
			cosine_angle = dot_product3d(&relative_final_vector, &further_vector);
			cosine_angle = PIN(cosine_angle, -1.f, 1.f);
			cross_product3d(&relative_final_vector, &further_vector, aiming_velocity);
		}

		normalize3d(aiming_velocity);
		scale_vector3d(aiming_velocity, MIN(arccosine(cosine_angle), angular_velocity_limit), aiming_velocity);

		if (orientation)
		{
			matrix4x3_transform_normal(orientation, &relative_final_vector, aiming_vector);
			normalize3d(aiming_vector);
		}
		else
		{
			*aiming_vector = relative_final_vector;
		}
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 2793, aiming_vector);

	return;
}

boolean unit_has_animation_to_enter_seat(
	long unit_index,
	long vehicle_index,
	short seat_index)
{
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
	boolean result = FALSE;

	if (seat_index >= 0 && seat_index < vehicle_definition->unit.seats.count)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->object.type == _object_type_vehicle)
		{
			result = TRUE;
		}
		else
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

			if (unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, NULL, FALSE))
			{
				result = TRUE;
			}
		}
	}

	return result;
}

static boolean unit_set_or_test_seat_and_weapon_label(
	long object_index,
	char const *seat_label,
	char const *weapon_label,
	boolean change_flag)
{
	short seat_index;

	struct unit_datum *unit = unit_get(object_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	boolean result = FALSE;

	for (seat_index = 0; seat_index < animation_graph->unit_seats.count; seat_index++)
	{
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, seat_index, struct animation_graph_unit_seat);

		if (!seat_label || !_stricmp(seat_label, unit_seat->label))
		{
			short weapon_class_index;

			for (weapon_class_index = 0; weapon_class_index < unit_seat->weapon_classes.count; weapon_class_index++)
			{
				short weapon_type_index;

				struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, weapon_class_index, struct animation_graph_weapon_class);

				for (weapon_type_index = 0; weapon_type_index < weapon_class->weapon_types.count; weapon_type_index++)
				{
					struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, weapon_type_index, struct animation_graph_weapon_type);

					if (!weapon_label ||
						!strcmp(weapon_label, base_weapon_label_get(_unit_base_weapon_none)) && weapon_type->label[0] == '\0' ||
						!_stricmp(weapon_label, weapon_type->label))
					{
						if (change_flag)
						{
							boolean showing_acceleration =
								(unit_seat->animations.count > _unit_seat_animation_acceleration_front_back ?
								animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_acceleration_front_back].animation_index : NONE) != NONE ||
								(unit_seat->animations.count > _unit_seat_animation_acceleration_left_right ?
								animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_acceleration_left_right].animation_index : NONE) != NONE ||
								(unit_seat->animations.count > _unit_seat_animation_acceleration_up_down ?
								animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_acceleration_up_down].animation_index : NONE) != NONE;

							if (unit->unit.animation.state != _unit_state_user_animation)
							{
								unit->unit.animation.state = NONE;
							}

							unit->unit.animation.seat_index = (char)seat_index;
							unit->unit.animation.base_seat_index = (char)seat_label_to_base_seat_index(seat_label);
							unit->unit.animation.weapon_index = (char)weapon_class_index;
							unit->unit.animation.weapon_type_index = (char)weapon_type_index;
							SET_FLAG(unit->unit.animation.flags, _unit_animation_showing_acceleration_bit, showing_acceleration);
						}

						result = TRUE;
						break;
					}
				}
			}
		}
	}

	return result;
}

short unit_update_animation(
	long unit_index,
	struct unit_animation_update_data *data)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	short result = _unit_animation_result_none;
	short desired_state = data->state_desired;
	boolean force_state_change = FALSE;

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 2913, desired_state>=0 && desired_state<NUMBER_OF_UNIT_STATES);

	if (unit->object.parent_object_index == NONE && !TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		short desired_base_seat_index = NONE;

		switch (unit->unit.animation.desired_state)
		{
		case _unit_animation_state_asleep:
			desired_base_seat_index = _unit_base_seat_asleep;
			break;
		case _unit_animation_state_suspicious:
			desired_base_seat_index = _unit_base_seat_alert;
			break;
		case _unit_animation_state_alert:
			desired_base_seat_index = _unit_base_seat_alert;
			break;
		case _unit_animation_state_in_combat:
			desired_base_seat_index = data->crouching ? _unit_base_seat_crouch : _unit_base_seat_stand;
			break;
		case _unit_animation_state_wary:
			desired_base_seat_index = _unit_base_seat_stand;
			break;
		case _unit_animation_state_flee:
			desired_base_seat_index = _unit_base_seat_flee;
			break;
		case _unit_animation_state_flaming:
			desired_base_seat_index = _unit_base_seat_flaming;
			break;
		default:
			break;
		}

		match_assert("c:\\halo\\SOURCE\\units\\units.c", 2931, desired_base_seat_index!=NONE);

		if (unit->unit.player_index != NONE && magic_base_animation_seat_index != NONE)
		{
			desired_base_seat_index = PIN(magic_base_animation_seat_index, 0, NUMBER_OF_UNIT_BASE_SEATS);
		}

		if (unit->unit.magic_seat_index != NONE)
		{
			desired_base_seat_index = unit->unit.magic_seat_index;
		}

		if (TEST_FLAG(unit->unit.control_flags, _unit_control_force_alert_bit))
		{
			desired_base_seat_index = _unit_base_seat_alert;
		}

		if (unit->unit.flaming_death_delay > 0)
		{
			desired_base_seat_index = _unit_base_seat_flaming;
		}

		if (unit->unit.animation.base_seat_index != desired_base_seat_index &&
			unit_animation_state_interruptable(&unit->unit.animation, desired_state))
		{
			unit_set_or_test_seat_and_weapon_label(
			unit_index,
			base_seat_label_get(desired_base_seat_index),
			unit_get_current_weapon_label(unit_index),
			TRUE);
		}
	}

	if (unit->unit.animation.soft_ping_animation.index != NONE &&
		unit_animation_update(unit_index, unit_definition->object.animation_graph.index, &unit->unit.animation.soft_ping_animation) == _animation_will_restart_on_next_frame)
	{
		unit->unit.animation.soft_ping_animation.index = NONE;
	}

	if (unit->object.animation.state.index != NONE)
	{
		short animation_update_result = unit_animation_update(unit_index, unit->object.animation.animation_graph_index, &unit->object.animation.state);

		if (animation_update_result == _animation_key_frame)
		{
			switch (unit->unit.animation.state)
			{
			case _unit_state_throw_grenade:
				unit_throw_grenade_release(unit_index, FALSE);
				break;
			case _unit_state_melee_attack:
			case _unit_state_melee_airborne:
			case _unit_state_leap_melee:
				unit_cause_melee_damage(unit_index, FALSE, NONE, NONE, NONE, NONE, NULL);
				break;
			default:
				break;
			}
		}
		else if (animation_update_result == _animation_will_restart_on_next_frame)
		{
			switch (unit->unit.animation.state)
			{
			case _unit_state_leap_start:
				result = _unit_animation_result_jump;
				desired_state = _unit_state_leap_airborne;
				break;
			case _unit_state_opening:
			case _unit_state_closing:
				unit->object.animation.state.frame_index--;
				break;
			case _unit_state_dying:
			{
				boolean destroy = FALSE;

				if (TEST_FLAG(unit_definition->unit.flags, _unit_is_destroyed_after_dying_bit))
				{
					if (TEST_FLAG(unit->object.flags, _object_at_rest_bit))
					{
						destroy = TRUE;
					}
					else if (unit->object.type == _object_type_biped)
					{
						struct biped_datum *biped = biped_get(unit_index);
						struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);

						if (!TEST_FLAG(biped->biped.flags, _biped_airborne_bit) || TEST_FLAG(biped_definition->biped.flags, _biped_has_no_dying_airborne_bit))
						{
							destroy = TRUE;
						}
					}
				}

				if (destroy)
				{
					unit_destroy(unit_index);
				}
				else
				{
					if (unit->object.type == _object_type_biped)
					{
						biped_start_limp_body_physics(unit_index);
					}

					SET_FLAG(unit->unit.animation.flags, _unit_animation_ignore_translation_bit, TRUE);
					unit->object.animation.state.frame_index--;
				}
				break;
			}
			case _unit_state_exiting_seat:
			{
				real_vector3d exit_velocity;
				real_matrix4x3 old_world_matrix;
				struct model *model = model_definition_get(unit_definition->object.model.index);
				struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

				animation_get_root_velocity(model, animation, unit->object.animation.state.frame_index, &exit_velocity);
				matrix4x3_transform_vector(object_get_world_matrix(unit_index, &old_world_matrix), &exit_velocity, &exit_velocity);
				unit_exit_seat_end(unit_index);
				add_vectors3d(&unit->object.translational_velocity, &exit_velocity, &unit->object.translational_velocity);
				break;
			}
			case _unit_state_entering_seat:
			{
				struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
				struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
				struct unit_seat *parent_seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				object_set_visibility(unit_index, !TEST_FLAG(parent_seat->flags, _unit_seat_is_invisible_bit));

				if (parent_unit->unit.driver_object_index == unit_index)
				{
					unit_close(unit->object.parent_object_index);
				}
				break;
			}
			default:
				break;
			}

			if (!unit_animation_state_loops(&unit->unit.animation))
			{
				force_state_change = TRUE;
			}
		}
	}

	if (unit->unit.animation.action_animation.index != NONE &&
		unit_animation_update(unit_index, unit_definition->object.animation_graph.index, &unit->unit.animation.action_animation) == _animation_will_restart_on_next_frame)
	{
		object_start_interpolation(unit_index, 6);
		unit_animation_start_action(unit_index, _unit_action_none);
	}

	if (unit->unit.animation.overlay_action_animation.index != NONE)
	{
		switch (unit_animation_update(unit_index, unit_definition->object.animation_graph.index, &unit->unit.animation.overlay_action_animation))
		{
		case _animation_will_restart_on_next_frame:
		case _animation_looped:
			if (!unit_animation_overlay_action_loops(&unit->unit.animation))
			{
				unit->unit.animation.overlay_action = _unit_overlay_action_none;
				unit->unit.animation.overlay_action_animation.index = NONE;
			}
			break;
		default:
			break;
		}
	}

	if (force_state_change ||
		desired_state != unit->unit.animation.state && unit_animation_state_interruptable(&unit->unit.animation, desired_state))
	{
		unit_animation_set_state(unit_index, desired_state);
	}

	return result;
}

void unit_adjust_projectile_ray(
	long unit_index,
	real_point3d *origin,
	real_vector3d *forward,
	real *inherited_velocity,
	boolean offset_origin,
	boolean offset_aim)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (offset_aim)
	{
		*forward = unit->unit.aiming_vector;
	}

	if (offset_origin)
	{
		real_point3d camera_position;
		real_vector3d camera_to_origin;

		unit_get_camera_position(unit_index, &camera_position);
		vector_from_points3d(&camera_position, origin, &camera_to_origin);
		point_from_line3d(&camera_position, forward, (dot_product3d(&camera_to_origin, forward)), origin);
	}

	{
		real_vector3d owner_velocity;

		object_get_velocities(unit_index, &owner_velocity, NULL);
		*inherited_velocity = dot_product3d(forward, &owner_velocity);
	}

	return;
}

void unit_unzoom(
	long unit_index)
{
	boolean play_sound;
	boolean current_weapon_zooms;
	long current_weapon_index;
	long local_player_index;

	struct unit_datum *unit = unit_get(unit_index);

	if (player_index_from_unit_index(unit_index) == NONE)
	{
		local_player_index = NONE;
	}
	else
	{
		local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
	}

	current_weapon_zooms = unit->unit.current_zoom_level != NONE;

	if (local_player_index != NONE && current_weapon_zooms)
	{
		current_weapon_index = unit_get_current_weapon_index(unit_index);

		if (current_weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(current_weapon_index);
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
			long zoom_out_sound_index = weapon_definition->weapon.zoom_out_sound.index;

			play_sound = zoom_out_sound_index != NONE;

			if (play_sound)
			{
				unspatialized_impulse_sound_new(zoom_out_sound_index, 1.f);
			}
		}
	}

	unit->unit.current_zoom_level = NONE;
	unit->unit.desired_zoom_level = NONE;
	unit->unit.integrated_night_vision_power = 0.f;
	player_control_unzoom(unit_index);

	return;
}

short unit_get_zoom_level(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	return unit->unit.current_zoom_level;
}

real unit_get_zoom_magnification(
	long unit_index,
	short zoom_level)
{
	real result = 1.f;
	long current_weapon_index = unit_get_current_weapon_index(unit_index);

	if (current_weapon_index != NONE)
	{
		result = weapon_get_zoom_magnification(current_weapon_index, zoom_level);
	}

	return result;
}

boolean unit_solo_player_integrated_night_vision_is_active(
	void)
{
	boolean result = FALSE;

	if (local_player_count() == 1)
	{
		long player_index = local_player_get_player_index(local_player_get_next(NONE));

		if (player_index != NONE)
		{
			long unit_index = player_get(player_index)->unit_index;

			if (unit_index != NONE)
			{
				result = unit_integrated_night_vision_is_active(unit_index);
			}
		}
	}

	return result;
}

static boolean unit_integrated_night_vision_is_active(
	long unit_index)
{
	boolean active = FALSE;
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.current_zoom_level != NONE)
	{
		long current_weapon_index = unit_get_current_weapon_index(unit_index);

		if (current_weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(current_weapon_index);
			struct weapon_definition* weapon_definition = weapon_definition_get(weapon->definition_index);
			
			if (TEST_FLAG(weapon_definition->weapon.flags, _weapon_enables_unit_integrated_night_vision_bit))
			{
				active = TRUE;
			}
		}
	}

	return active;
}

void unit_render_debug(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

#ifdef DEBUG
	if (debug_objects_unit_vectors)
	{
		real_point3d head_position;
		real_point3d body_position;

		unit_get_camera_position(unit_index, &head_position);
		object_get_origin(unit_index, &body_position);
		body_position.z += 0.1f;
		render_debug_vector(TRUE, &head_position, &unit->unit.aiming_vector, 1.f, global_real_argb_white);
		render_debug_vector(TRUE, &head_position, &unit->unit.desired_aiming_vector, 0.5f, global_real_argb_red);
		render_debug_vector(TRUE, &body_position, &unit->object.forward, 1.f, global_real_argb_white);
		render_debug_vector(TRUE, &body_position, &unit->unit.desired_facing_vector, 0.5f, global_real_argb_red);
	}

	if (debug_objects_unit_seats)
	{
		long player_unit_index = player_get(local_player_get_player_index(render.local_player_index))->unit_index;

		if (player_unit_index != NONE)
		{
			short seat_index;

			for (seat_index = 0; seat_index < unit_definition->unit.seats.count; seat_index++)
			{
				real_point3d entrance_point;
				real_point3d seat_point;
				real_point3d hint_point;

				if (unit_get_seat_entrance_point(player_unit_index, unit_index, seat_index, &entrance_point, &seat_point, &hint_point))
				{
					render_debug_point(TRUE, &entrance_point, 0.25f, global_real_argb_red);
					render_debug_point(TRUE, &seat_point, 0.25f, global_real_argb_blue);
					render_debug_point(TRUE, &hint_point, 0.25f, global_real_argb_yellow);
				}
			}
		}
	}

	if (debug_objects_unit_mouth_apeture)
	{
		real_point3d head_point;

		unit_get_head_position(unit_index, &head_point);
		render_debug_string_at_point(FALSE, &head_point, csprintf(temporary, "%.2f", unit->unit.mouth_aperture), global_real_argb_orange);
	}
#endif

	return;
}

static boolean unit_animation_state_interruptable(
	struct unit_animation *animation,
	short new_state)
{
	boolean result = TRUE;

	switch (animation->state)
	{
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		if (new_state != _unit_state_hard_ping)
		{
			result = FALSE;
		}
		break;
	case _unit_state_dying_airborne:
	case _unit_state_dying:
		switch (new_state)
		{
		case _unit_state_dying_airborne:
		case _unit_state_dying:
			result = TRUE;
			break;
		default:
			result = FALSE;
			break;
		}
		break;
	case _unit_state_hard_ping:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_user_animation:
		result = FALSE;
		break;
	case _unit_state_turn_left:
	case _unit_state_turn_right:
	case _unit_state_opening:
	case _unit_state_closing:
		if (new_state == _unit_state_idle)
		{
			result = FALSE;
		}
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_busy(
	struct unit_animation *animation)
{
	boolean result = FALSE;

	switch (animation->state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = TRUE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_overlay_action_loops(
	struct unit_animation *animation)
{
	boolean result = FALSE;

	if (animation->state >= _unit_overlay_action_weapon_primary_charged && animation->state <= _unit_overlay_action_weapon_secondary_charged)
	{
		result = TRUE;
	}

	return result;
}

static boolean unit_animation_state_loops(
	struct unit_animation *animation)
{
	boolean result = TRUE;

	switch (animation->state)
	{
	case _unit_state_gesture:
	case _unit_state_turn_left:
	case _unit_state_turn_right:
	case _unit_state_hard_ping:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_user_animation:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_weapon_ik(
	struct unit_animation *animation)
{
	boolean result = animation->soft_ping_animation.index == NONE;

	if (animation->action != _unit_action_none)
	{
		result = FALSE;
	}

	switch (animation->state)
	{
	case _unit_state_flying_front:
	case _unit_state_flying_back:
	case _unit_state_flying_left:
	case _unit_state_flying_right:
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_user_animation:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_airborne:
	case _unit_state_leap_melee:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_vehicle_ik(
	struct unit_animation *animation)
{
	boolean result = TRUE;

	switch (animation->state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_aiming_screen(
	struct unit_animation *animation)
{
	boolean result = animation->action == _unit_action_none;

	if (animation->state >= _unit_state_hard_ping &&
		(animation->state <= _unit_state_resurrect_back || animation->state == _unit_state_leap_melee))
	{
		result = FALSE;
	}

	return result;
}

static short unit_animation_state_get_aiming_screen_index(
	short state)
{
	short result = NONE;

	switch (state)
	{
	case _unit_state_idle:
	case _unit_state_turn_left:
	case _unit_state_turn_right:
	case _unit_state_flying_front:
	case _unit_state_flying_back:
	case _unit_state_flying_left:
	case _unit_state_flying_right:
	case _unit_state_airborne:
	case _unit_state_land_soft:
	case _unit_state_land_hard:
	case _unit_state_opening:
	case _unit_state_closing:
		result = _unit_weapon_class_animation_aiming_still;
		break;
	case _unit_state_move_front:
	case _unit_state_move_back:
	case _unit_state_move_left:
	case _unit_state_move_right:
	case _unit_state_stunned_move_front:
	case _unit_state_stunned_move_back:
	case _unit_state_stunned_move_left:
	case _unit_state_stunned_move_right:
	case _unit_state_slide_front:
	case _unit_state_slide_back:
	case _unit_state_slide_left:
	case _unit_state_slide_right:
		result = _unit_weapon_class_animation_aiming_moving;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_state_can_be_entered_without_animation(
	short new_state)
{
	boolean result = TRUE;

	switch (new_state)
	{
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}

static short unit_animation_compute_interpolation_frame_count(
	short old_state,
	short new_state)
{
	short result = 6;

	if ((new_state == _unit_state_idle || new_state == _unit_state_turn_left || new_state == _unit_state_turn_right) &&
		(old_state == _unit_state_idle || old_state == _unit_state_turn_left || old_state == _unit_state_turn_right))
	{
		result = 1;
	}

	if (new_state == _unit_state_land_hard || new_state == _unit_state_land_soft)
	{
		result = 2;
	}

	return result;
}

void unit_animation_start_action(
	long unit_index,
	short action)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (action == _unit_action_none)
	{
		unit->unit.animation.action = _unit_action_none;
		unit->unit.animation.action_animation.index = NONE;
	}
	else
	{
		short interpolation_frame_count;
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
		struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
		struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, unit->unit.animation.weapon_type_index, struct animation_graph_weapon_type);
		short weapon_class_animation_index = NONE;
		short weapon_type_animation_index = NONE;
		short animation_index = NONE;

		switch (action)
		{
		case _unit_action_disarm_weapon:
			weapon_class_animation_index = _unit_weapon_class_animation_disarm;
			break;
		case _unit_action_drop_weapon:
			weapon_class_animation_index = _unit_weapon_class_animation_drop;
			break;
		case _unit_action_ready_weapon:
			weapon_class_animation_index = _unit_weapon_class_animation_ready;
			break;
		case _unit_action_put_away_weapon:
			weapon_class_animation_index = _unit_weapon_class_animation_put_away;
			break;
		case _unit_action_primary_reload_weapon:
			weapon_type_animation_index = _unit_weapon_type_animation_primary_reload;
			break;
		case _unit_action_secondary_reload_weapon:
			weapon_type_animation_index = _unit_weapon_type_animation_secondary_reload;
			break;
		case _unit_action_melee:
			weapon_type_animation_index = _unit_weapon_type_animation_melee;
			break;
		case _unit_action_throw_grenade:
			weapon_class_animation_index = _unit_weapon_class_animation_throw_grenade;
			break;
		case _unit_action_overheat:
			weapon_type_animation_index = _unit_weapon_type_animation_overheat;
			break;
		default:
			break;
		}

		if (weapon_type_animation_index != NONE)
		{
			animation_index = weapon_type_animation_index >= 0 && weapon_type_animation_index < weapon_type->animations.count ?
				animation_graph_animation_index_get(&weapon_type->animations)[weapon_type_animation_index].animation_index : NONE;
		}
		else if (weapon_class_animation_index != NONE)
		{
			animation_index = weapon_class_animation_index >= 0 && weapon_class_animation_index < weapon_class->animations.count ?
				animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index : NONE;
		}

		switch (action)
		{
		case _unit_action_melee:
			interpolation_frame_count = 0;
			break;
		default:
			interpolation_frame_count = 6;
			break;
		}

		if (animation_index != NONE)
		{
			if (interpolation_frame_count > 0)
			{
				object_start_interpolation(unit_index, interpolation_frame_count);
			}

			unit->unit.animation.action_animation.index = animation_choose_random_permutation(unit_definition->object.animation_graph.index, animation_index);
			unit->unit.animation.action_animation.frame_index = 0;
			unit->unit.animation.action = (char)action;
		}
	}

	return;
}

static boolean unit_animation_set_state(
	long unit_index,
	short new_state)
{
	short interpolation_frame_count;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
	struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
	struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, unit->unit.animation.weapon_type_index, struct animation_graph_weapon_type);
	boolean first_state = unit->unit.animation.state == NONE;
	boolean interpolate = FALSE;
	boolean result = TRUE;

	if (first_state || new_state != unit->unit.animation.state)
	{
		short animation_index;
		short weapon_class_animation_index = NONE;
		short unit_seat_animation_index = NONE;

		if (unit->unit.animation.state == _unit_state_throw_grenade)
		{
			unit_throw_grenade_release(unit_index, TRUE);
		}

		switch (new_state)
		{
		case _unit_state_idle:
			weapon_class_animation_index = _unit_weapon_class_animation_idle;
			break;
		case _unit_state_gesture:
			weapon_class_animation_index = _unit_weapon_class_animation_gesture;
			break;
		case _unit_state_turn_left:
			weapon_class_animation_index = _unit_weapon_class_animation_turning_left;
			break;
		case _unit_state_turn_right:
			weapon_class_animation_index = _unit_weapon_class_animation_turning_right;
			break;
		case _unit_state_move_front:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_front;
			break;
		case _unit_state_move_back:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_back;
			break;
		case _unit_state_move_left:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_left;
			break;
		case _unit_state_move_right:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_right;
			break;
		case _unit_state_stunned_move_front:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_front;
			break;
		case _unit_state_stunned_move_back:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_back;
			break;
		case _unit_state_stunned_move_left:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_left;
			break;
		case _unit_state_stunned_move_right:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_right;
			break;
		case _unit_state_slide_front:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_front;
			break;
		case _unit_state_slide_back:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_back;
			break;
		case _unit_state_slide_left:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_left;
			break;
		case _unit_state_slide_right:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_right;
			break;
		case _unit_state_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_airborne;
			break;
		case _unit_state_land_soft:
			weapon_class_animation_index = _unit_weapon_class_animation_land_soft;
			break;
		case _unit_state_land_hard:
			weapon_class_animation_index = _unit_weapon_class_animation_land_hard;
			break;
		case _unit_state_throw_grenade:
			weapon_class_animation_index = _unit_weapon_class_animation_throw_grenade;
			break;
		case _unit_state_melee_attack:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_attack;
			break;
		case _unit_state_melee_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_airborne;
			break;
		case _unit_state_melee_continuous:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_continuous;
			break;
		case _unit_state_resurrect_front:
			weapon_class_animation_index = _unit_weapon_class_animation_resurrect_front;
			break;
		case _unit_state_resurrect_back:
			weapon_class_animation_index = _unit_weapon_class_animation_resurrect_back;
			break;
		case _unit_state_feeding:
			weapon_class_animation_index = _unit_weapon_class_animation_feeding;
			break;
		case _unit_state_leap_start:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_start;
			break;
		case _unit_state_leap_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_airborne;
			break;
		case _unit_state_leap_melee:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_melee;
			break;
		case _unit_state_flying_front:
			unit_seat_animation_index = _unit_seat_animation_flying_front;
			break;
		case _unit_state_flying_back:
			unit_seat_animation_index = _unit_seat_animation_flying_back;
			break;
		case _unit_state_flying_left:
			unit_seat_animation_index = _unit_seat_animation_flying_left;
			break;
		case _unit_state_flying_right:
			unit_seat_animation_index = _unit_seat_animation_flying_right;
			break;
		case _unit_state_dying_airborne:
			unit_seat_animation_index = _unit_seat_animation_airborne_dead;
			break;
		case _unit_state_dying:
			unit_seat_animation_index = _unit_seat_animation_landing_dead;
			break;
		case _unit_state_opening:
			unit_seat_animation_index = _unit_seat_animation_opening;
			break;
		case _unit_state_closing:
			unit_seat_animation_index = _unit_seat_animation_closing;
			break;
		case _unit_state_hovering:
			unit_seat_animation_index = _unit_seat_animation_hovering;
			break;
		default:
			break;
		}

		if (weapon_class_animation_index != NONE)
		{
			animation_index = weapon_class_animation_index >= 0 && weapon_class_animation_index < weapon_class->animations.count ?
				animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index : NONE;
		}
		else if (unit_seat_animation_index != NONE)
		{
			animation_index = unit_seat_animation_index >= 0 && unit_seat_animation_index < unit_seat->animations.count ?
				animation_graph_animation_index_get(&unit_seat->animations)[unit_seat_animation_index].animation_index : NONE;
		}
		else
		{
			animation_index = NONE;
		}

#ifdef DEBUG
		if (debug_unit_animations && unit->object.type == _object_type_biped && animation_index == NONE)
		{
			console_warning("MISSING: %s '%s %s %s'",
				tag_name_strip_path(unit_definition->object.animation_graph.name),
				unit_seat->label,
				unit_seat_animation_index != NONE ? animation_list_get_string(&unit_seat_animation_list, unit_seat_animation_index) : weapon_class->label,
				weapon_class_animation_index != NONE ? animation_list_get_string(&weapon_class_animation_list, weapon_class_animation_index) : "");
		}
#endif

		if (animation_index != NONE || unit_animation_state_can_be_entered_without_animation(new_state))
		{
			unit_set_animation(unit_index, unit_definition->object.animation_graph.index, animation_choose_random_permutation(unit_definition->object.animation_graph.index, animation_index));
			interpolation_frame_count = unit_animation_compute_interpolation_frame_count(unit->unit.animation.state, new_state);
			interpolate = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}

	if (result)
	{
		short aiming_screen_index = unit_animation_state_get_aiming_screen_index(new_state);

		if (first_state || aiming_screen_index != unit_animation_state_get_aiming_screen_index(unit->unit.animation.state))
		{
			unit->unit.animation.aiming_screen_index = animation_choose_random_permutation(unit_definition->object.animation_graph.index,
				aiming_screen_index >= 0 && aiming_screen_index < weapon_class->animations.count ?
				animation_graph_animation_index_get(&weapon_class->animations)[aiming_screen_index].animation_index : NONE);

#ifdef DEBUG
			if (debug_unit_animations && unit->object.type == _object_type_biped && unit->unit.animation.aiming_screen_index == NONE && aiming_screen_index != NONE)
			{
				console_warning("MISSING: %s '%s %s %s'",
					tag_name_strip_path(unit_definition->object.animation_graph.name),
					unit_seat->label,
					weapon_class->label,
					animation_list_get_string(&weapon_class_animation_list, aiming_screen_index));
			}
#endif

			interpolation_frame_count = 6;
			interpolate = TRUE;
		}

		if (first_state)
		{
			unit->unit.animation.looking_screen_index = animation_choose_random_permutation(unit_definition->object.animation_graph.index,
				unit_seat->animations.count > _unit_seat_animation_looking ?
				animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_looking].animation_index : NONE);

#ifdef DEBUG
			if (debug_unit_animations && unit->object.type == _object_type_biped && unit->unit.animation.looking_screen_index == NONE)
			{
				console_warning("MISSING: %s '%s %s'",
					tag_name_strip_path(unit_definition->object.animation_graph.name),
					unit_seat->label,
					animation_list_get_string(&unit_seat_animation_list, _unit_seat_animation_looking));
			}
#endif
		}

		if (interpolate)
		{
			object_start_interpolation(unit_index, interpolation_frame_count);
		}

		unit->unit.animation.state = (char)new_state;
	}

	return result;
}

void unit_abort_animation(
	long unit_index)
{
	unit_animation_set_state(unit_index, _unit_state_idle);

	return;
}

static void unit_animation_start_overlay_action(
	long unit_index,
	short overlay_action)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (overlay_action >= unit->unit.animation.overlay_action && !unit_animation_busy(&unit->unit.animation))
	{
		short animation_index;
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
		struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
		struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, unit->unit.animation.weapon_type_index, struct animation_graph_weapon_type);
		short weapon_type_animation_index = NONE;

		switch (overlay_action)
		{
		case _unit_overlay_action_weapon_fire_primary:
			weapon_type_animation_index = _unit_weapon_type_animation_primary_recoil;
			break;
		case _unit_overlay_action_weapon_fire_secondary:
			weapon_type_animation_index = _unit_weapon_type_animation_secondary_recoil;
			break;
		case _unit_overlay_action_weapon_primary_charged:
			weapon_type_animation_index = _unit_weapon_type_animation_primary_charged;
			break;
		case _unit_overlay_action_weapon_secondary_charged:
			weapon_type_animation_index = _unit_weapon_type_animation_secondary_charged;
			break;
		case _unit_overlay_action_weapon_primary_chamber:
			weapon_type_animation_index = _unit_weapon_type_animation_primary_chamber;
			break;
		case _unit_overlay_action_weapon_secondary_chamber:
			weapon_type_animation_index = _unit_weapon_type_animation_secondary_chamber;
			break;
		}

		if (weapon_type_animation_index >= 0 && weapon_type_animation_index < weapon_type->animations.count)
		{
			animation_index = animation_graph_animation_index_get(&weapon_type->animations)[weapon_type_animation_index].animation_index;
		}
		else
		{
			animation_index = NONE;
		}

		if (animation_index != NONE)
		{
			unit->unit.animation.overlay_action_animation.index = animation_choose_random_permutation(unit_definition->object.animation_graph.index, animation_index);
			unit->unit.animation.overlay_action_animation.frame_index = 0;
			unit->unit.animation.overlay_action = (char)overlay_action;
		}
#ifdef DEBUG
		else if (debug_unit_animations && unit->object.type == _object_type_biped && unit->unit.animation.aiming_screen_index == NONE)
		{
			console_warning("MISSING: %s '%s %s'",
				tag_name_strip_path(unit_definition->object.animation_graph.name),
				unit_seat->label,
				weapon_class->label,
				weapon_type->label,
				animation_list_get_string(&weapon_type_animation_list, weapon_type_animation_index));
		}
#endif
	}

	return;
}

short unit_find_nearby_seat(
	long unit_index,
	long parent_unit_index,
	short *parent_seat_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *parent_unit = unit_get(parent_unit_index);
	struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
	short result = _unit_nearby_seat_result_none;
	short best_seat_index = NONE;

	if (!TEST_FLAG(parent_unit->object.damage_flags, _object_dead_bit) &&
		!TEST_FLAG(parent_unit->unit.flags, _unit_not_enterable_by_player_bit))
	{
		short seat_index;
		real best_distance = REAL_MAX;
		boolean best_seat_is_driver = FALSE;

		for (seat_index = 0; seat_index < parent_unit_definition->unit.seats.count; seat_index++)
		{
			real_point3d entrance_point;
			real_point3d seat_point;
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, seat_index, struct unit_seat);

			if (unit_get_seat_entrance_point(unit_index, parent_unit_index, seat_index, &entrance_point, &seat_point, NULL))
			{
				real entrance_distance = distance3d(&entrance_point, &unit->object.bounding_sphere_center);
				real seat_distance = distance3d(&seat_point, &unit->object.bounding_sphere_center);
				real distance = MIN(entrance_distance, seat_distance);

				if (distance < 1.f)
				{
					short seat_result = _unit_nearby_seat_result_none;

					if ((!TEST_FLAG(seat->flags, _unit_seat_not_valid_without_driver_bit) || parent_unit->unit.driver_object_index != NONE) &&
						seat->label[0] != '\0' &&
						unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, NULL, FALSE))
					{
						long seat_occupant_unit_index = NONE;

						if (unit_can_enter_seat(unit_index, parent_unit_index, seat_index, &seat_occupant_unit_index))
						{
							seat_result = _unit_nearby_seat_result_empty;
						}
						else if (seat_occupant_unit_index != NONE)
						{
							struct unit_datum *seat_occupant_unit = unit_get(seat_occupant_unit_index);

							if (seat_occupant_unit->unit.actor_index != NONE)
							{
								if (ai_try_vehicle_eviction(seat_occupant_unit->unit.actor_index, unit_index, FALSE))
								{
									seat_result = _unit_nearby_seat_result_occupied_by_friend;
								}
							}
						}
					}

					if (seat_result > _unit_nearby_seat_result_none)
					{
						boolean seat_is_driver = TEST_FLAG(seat->flags, _unit_seat_is_driver_bit);
						real distance_scale = 1.f;
						boolean better = FALSE;

						if (best_seat_is_driver && !seat_is_driver)
						{
							distance_scale = 1.5f;
						}

						if (best_seat_index == NONE)
						{
							better = TRUE;
						}
						else if (seat_result > result)
						{
							better = TRUE;
						}
						else if (distance * distance_scale < best_distance)
						{
							better = TRUE;
						}

						if (better)
						{
							result = seat_result;
							best_seat_index = seat_index;
							best_seat_is_driver = seat_is_driver;
							best_distance = distance;
						}
					}
				}
			}
		}
	}

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 4088, parent_seat_index != NULL);
	*parent_seat_index = best_seat_index;

	return result;
}

boolean unit_can_enter_seat(
	long unit_index,
	long parent_unit_index,
	short seat_index,
	long *occupant_unit_index_reference)
{
	long child_object_index;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *parent_unit = unit_get(parent_unit_index);
	long occupant_unit_index = NONE;
	boolean result = TRUE;

	if (unit_index == parent_unit_index)
	{
		result = FALSE;
	}

	child_object_index = parent_unit->object.first_child_object_index;

	while (child_object_index != NONE)
	{
		struct object_datum *child_object = object_get(child_object_index);

		if (TEST_FLAG(_object_mask_unit, child_object->object.type))
		{
			if (unit_get(child_object_index)->unit.parent_seat_index == seat_index)
			{
				result = FALSE;
				occupant_unit_index = child_object_index;
			}
			else if (unit->unit.player_index != NONE && game_team_is_enemy(unit->object.owner_team_index, child_object->object.owner_team_index))
			{
				result = FALSE;
			}
		}

		child_object_index = child_object->object.next_object_index;
	}

	if (occupant_unit_index_reference)
	{
		*occupant_unit_index_reference = occupant_unit_index;
	}

	return result;
}

boolean unit_enter_seat(
	long unit_index,
	long parent_unit_index,
	short seat_index)
{
	boolean result = FALSE;

	if (unit_can_enter_seat(unit_index, parent_unit_index, seat_index, NULL))
	{
		real_vector3d offset;
		real_point3d old_origin;
		struct object_marker marker;
		struct unit_datum *unit = unit_get(unit_index);
		struct unit_datum *parent_unit = unit_get(parent_unit_index);
		struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, seat_index, struct unit_seat);

		match_assert("c:\\halo\\SOURCE\\units\\units.c", 4245, unit->object.parent_object_index==NONE);

		object_get_origin(unit_index, &old_origin);
		object_get_marker_by_name(parent_unit_index, seat->marker_name, &marker, 1);
		vector_from_points3d(&marker.matrix.position, &old_origin, &offset);
		matrix4x3_inverse_transform_vector(&marker.matrix, &offset, &offset);
		object_attach_to_marker(parent_unit_index, seat->marker_name, unit_index, "");
		unit->unit.parent_seat_index = seat_index;
		unit->object.parent_object_index = parent_unit_index;
		unit_update_driver_and_gunner(parent_unit_index);
		unit_adjust_for_seat_change(unit_index);

		if (!unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, unit_get_current_weapon_label(unit_index), TRUE))
		{
			unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, NULL, TRUE);
		}

		{
			struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
			struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
			short animation_index = unit_seat->animations.count <= _unit_seat_animation_seat_enter ?
				NONE :
				animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_seat_enter].animation_index;

			if (animation_index != NONE)
			{
				object_start_interpolation(unit_index, 6);
				unit_set_animation(unit_index, unit_definition->object.animation_graph.index, animation_choose_random_permutation(unit_definition->object.animation_graph.index, animation_index));
				unit->unit.animation.state = _unit_state_entering_seat;
				object_offset_interpolation(unit_index, &offset);
				object_compute_node_matrices_recursive(unit_index);
			}
		}

		ai_handle_enter_vehicle(unit_index, parent_unit_index);
		unit_unzoom(unit_index);
		result = TRUE;
	}

	return result;
}

void unit_ping_animation(
	long unit_index,
	boolean died,
	boolean feign_death,
	boolean died_flying,
	boolean ignore_hard_pings,
	boolean force_hard_pings,
	real angle,
	short damage_part,
	real_vector2d *alignment_vector)
{
	short damage_direction;
	boolean soft_ping;
	boolean hard_ping;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (died)
	{
		feign_death = FALSE;
		soft_ping = TRUE;
		hard_ping = unit_definition->unit.hard_death_threshold > 0.f && unit->object.current_body_damage > unit_definition->unit.hard_death_threshold;
	}
	else if (feign_death)
	{
		died = TRUE;
		soft_ping = TRUE;
		hard_ping = FALSE;
	}
	else
	{
		soft_ping = unit->object.current_body_damage > unit_definition->unit.soft_ping_threshold || unit->object.current_shield_damage > unit_definition->unit.soft_ping_threshold;
		hard_ping = unit->object.current_body_damage > unit_definition->unit.hard_ping_threshold;
	}

	if (!died && (ignore_hard_pings || TEST_FLAG(unit->unit.flags, _unit_ignore_hard_pings_bit)))
	{
		hard_ping = FALSE;
	}

	if (force_hard_pings)
	{
		hard_ping = TRUE;
		soft_ping = TRUE;
	}

	if (damage_part == NONE)
	{
		damage_part = 0;
	}

	if (fabs(angle) < _pi/4.f)
	{
		damage_direction = _animation_damage_direction_back;
	}
	else if (fabs(angle) > _pi*0.6875)
	{
		damage_direction = _animation_damage_direction_front;
	}
	else
	{
		damage_direction = angle > 0.f ? _animation_damage_direction_left : _animation_damage_direction_right;
	}

	if (game_engine_running() && damage_part == _damage_part_head && hard_ping && died)
	{
		damage_direction = _animation_damage_direction_left;
	}

	if (soft_ping || died)
	{
		struct unit_definition *definition = unit_definition_get(unit->definition_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(definition->object.animation_graph.index);

		if (hard_ping || died)
		{
			boolean dying_airborne = FALSE;
			short state = died ? _unit_state_dying : _unit_state_hard_ping;
			short damage_type = died ? (hard_ping ? _animation_damage_type_hard_kill : _animation_damage_type_soft_kill) : _animation_damage_type_hard_ping;
			boolean interrupt = died || unit_animation_state_interruptable(&unit->unit.animation, state);

			if (unit->unit.animation.state == _unit_state_hard_ping && unit->object.animation.state.frame_index > definition->unit.runtime_hard_ping_minimum_interrupt_ticks)
			{
				interrupt = TRUE;
			}

			if (!died && TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				interrupt = FALSE;
			}

			if (!died && unit->object.parent_object_index != NONE)
			{
				interrupt = FALSE;
			}

			if (interrupt)
			{
				if (died)
				{
					unit_set_or_test_seat_and_weapon_label(
					unit_index,
					base_seat_label_get(_unit_base_seat_stand),
					unit_get_current_weapon_label(unit_index),
					TRUE);
				}

				if (state == _unit_state_dying && unit->object.type == _object_type_biped)
				{
					struct biped_datum *biped = biped_get(unit_index);
					struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);

					if (TEST_FLAG(biped->biped.flags, _biped_airborne_bit) && !TEST_FLAG(biped_definition->biped.flags, _biped_has_no_dying_airborne_bit))
					{
						state = _unit_state_dying_airborne;
						dying_airborne = TRUE;
					}
				}

				if (!dying_airborne || !unit_animation_set_state(unit_index, state))
				{
					short damage_animation_index = build_damage_animation_index(damage_type, damage_direction, damage_part);
					short animation_index = animation_choose_random_permutation(definition->object.animation_graph.index,
						damage_animation_index >= 0 && damage_animation_index < animation_graph->unit_damage_animations.count ?
						animation_graph_animation_index_get(&animation_graph->unit_damage_animations)[damage_animation_index].animation_index :
						NONE);

					if (animation_index != NONE)
					{
						if (unit->unit.animation.state == _unit_state_throw_grenade)
						{
							unit_throw_grenade_release(unit_index, TRUE);
						}

						object_start_interpolation(unit_index, 3);
						unit->unit.animation.state = (char)state;
						unit_set_animation(unit_index, definition->object.animation_graph.index, animation_index);
						SET_FLAG(unit->unit.animation.flags, _unit_animation_postpone_weapon_ik_until_interpolation_ends_bit, TRUE);

						if (died)
						{
							if (!died_flying && !feign_death)
							{
								struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

								unit->unit.weapon_drop_delay_ticks = (char)random_range(animation->frame_count >> 2, (animation->frame_count >> 1) + (animation->frame_count >> 2));
								unit->unit.weapon_drop_delay_ticks = MAX(unit->unit.weapon_drop_delay_ticks, 1);
							}
							else
							{
								unit->unit.weapon_drop_delay_ticks = 0;
							}
						}

						if (damage_direction != _animation_damage_direction_front)
						{
							struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

							if (animation->runtime_parent_animation_index ==
								(build_damage_animation_index(damage_type, _animation_damage_direction_front, damage_part) >= 0 && build_damage_animation_index(damage_type, _animation_damage_direction_front, damage_part) < animation_graph->unit_damage_animations.count ?
								animation_graph_animation_index_get(&animation_graph->unit_damage_animations)[build_damage_animation_index(damage_type, _animation_damage_direction_front, damage_part)].animation_index :
								NONE))
							{
								damage_direction = _animation_damage_direction_front;
							}
						}

						if (died)
						{
							SET_FLAG(unit->unit.animation.flags, _unit_animation_fallen_on_front_bit, damage_direction == _animation_damage_direction_back);
						}
					}
					else if (died)
					{
						SET_FLAG(unit->unit.animation.flags, _unit_animation_ignore_translation_bit, TRUE);
						SET_FLAG(unit->unit.animation.flags, _unit_animation_fallen_on_front_bit, FALSE);

						if (TEST_FLAG(definition->unit.flags, _unit_is_destroyed_after_dying_bit))
						{
							unit_destroy(unit_index);
						}
					}
				}

				if (alignment_vector &&
					!TEST_FLAG(definition->unit.flags, _unit_does_not_reorient_during_pings_bit) &&
					unit->object.type == _object_type_biped &&
					unit->object.parent_object_index == NONE &&
					(hard_ping || died))
				{
					real_vector2d adjusted_alignment_vector;

					switch (damage_direction)
					{
					case _animation_damage_direction_back:
						adjusted_alignment_vector = *alignment_vector;
						break;
					case _animation_damage_direction_front:
						negate_vector2d(alignment_vector, &adjusted_alignment_vector);
						break;
					case _animation_damage_direction_left:
						set_real_vector2d(&adjusted_alignment_vector, -alignment_vector->j, alignment_vector->i);
						break;
					case _animation_damage_direction_right:
						set_real_vector2d(&adjusted_alignment_vector, alignment_vector->j, -alignment_vector->i);
						break;
					default:
						match_halt("c:\\halo\\SOURCE\\units\\units.c", 4562);
					}

					unit_align_facing(unit_index, &adjusted_alignment_vector);
				}
			}
		}
		else if (unit->unit.animation.soft_ping_animation.index == NONE || unit->unit.animation.soft_ping_animation.frame_index > definition->unit.runtime_soft_ping_minimum_interrupt_ticks)
		{
			short damage_animation_index = build_damage_animation_index(_animation_damage_type_soft_ping, damage_direction, damage_part);
			short animation_index = animation_choose_random_permutation(definition->object.animation_graph.index,
				damage_animation_index >= 0 && damage_animation_index < animation_graph->unit_damage_animations.count ?
				animation_graph_animation_index_get(&animation_graph->unit_damage_animations)[damage_animation_index].animation_index :
				NONE);

			if (animation_index != NONE)
			{
				unit->unit.animation.soft_ping_animation.index = animation_index;
				unit->unit.animation.soft_ping_animation.frame_index = 0;
			}
		}
	}

	return;
}

void unit_impulse(
	long unit_index,
	real_point3d const *origin,
	real_vector3d const *velocity,
	real mass)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit_definition->object.physics.index != NONE)
	{
		struct physics_definition *physics = physics_definition_get(unit_definition->object.physics.index);
		real scale = mass / physics->mass;

		vector_from_line3d(&unit->object.translational_velocity, velocity, scale, &unit->object.translational_velocity);
	}

	return;
}

void unit_handle_weapon_state_change(
	long unit_index,
	short new_weapon_state)
{
	short action = NONE;
	short overlay_action = NONE;

	switch (new_weapon_state)
	{
	case _weapon_state_primary_reload:
		action = _unit_action_primary_reload_weapon;
		break;
	case _weapon_state_secondary_reload:
		action = _unit_action_secondary_reload_weapon;
		break;
	case _weapon_state_primary_chamber:
		overlay_action = _unit_overlay_action_weapon_primary_chamber;
		break;
	case _weapon_state_secondary_chamber:
		overlay_action = _unit_overlay_action_weapon_secondary_chamber;
		break;
	case _weapon_state_primary_recoil:
		overlay_action = _unit_overlay_action_weapon_fire_primary;
		break;
	case _weapon_state_secondary_recoil:
		overlay_action = _unit_overlay_action_weapon_fire_secondary;
		break;
	case _weapon_state_primary_charged:
		overlay_action = _unit_overlay_action_weapon_primary_charged;
		break;
	case _weapon_state_secondary_charged:
		overlay_action = _unit_overlay_action_weapon_secondary_charged;
		break;
	}

	if (action != NONE)
	{
		unit_animation_start_action(unit_index, action);
	}
	else if (overlay_action != NONE)
	{
		unit_animation_start_overlay_action(unit_index, overlay_action);
	}

	return;
}

void unit_damage_aftermath(
	long unit_index,
	struct damage_data *damage_data,
	unsigned long being_damaged_flags,
	real shield_damage,
	real body_damage,
	real body_damage_multiplier,
	short body_part)
{
	boolean died_flying;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct damage_definition *damage = &damage_effect_definition_get(damage_data->definition_index)->damage;
	real total_damage = shield_damage + body_damage;
	boolean died = TEST_FLAG(being_damaged_flags, _object_being_damaged_body_depleted_bit);
	boolean feigned = FALSE;

#ifdef DEBUG
	if (debug_damage_taken && unit->unit.player_index != NONE)
	{
		char const *owner_name = "<unknown>";
		char const *damage_name = "<unknown>";

		if (damage_data->owner_object_index != NONE)
		{
			struct object_datum *owner_object = object_get(damage_data->owner_object_index);
			struct object_definition *owner_definition = object_definition_get(owner_object->definition_index);
			char const *name = strrchr(owner_definition->object.model.name, '\\');

			if (name)
			{
				owner_name = name + 1;
			}
		}

		if (damage_data->definition_index != NONE)
		{
			char const *tag_name = tag_get_name(damage_data->definition_index);
			char const *name = strrchr(tag_name, '\\');

			if (name)
			{
				damage_name = name + 1;
			}
		}

		console_printf(
		FALSE,
		"p%d: body %.2f shld %.2f from %s %s",
		DATUM_INDEX_TO_ABSOLUTE_INDEX(unit->unit.player_index),
		body_damage,
		shield_damage,
		owner_name,
		damage_name);
	}
#endif

	{
		real recent_damage = unit->object.recent_shield_damage + unit->object.recent_body_damage;

		if (recent_damage > 0.f)
		{
			unit->unit.last_damage_category = damage->category;
			unit->unit.delayed_damage_timer = 45;
			unit->unit.delayed_damage_peak = MAX(unit->unit.delayed_damage_peak, recent_damage);

			if (damage_data->owner_object_index != NONE)
			{
				unit->unit.delayed_damage_attacker_object_index = damage_data->owner_object_index;
			}
		}
	}

	if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
	{
		unit->unit.active_camouflage -= damage->active_camouflage_damage;

		if (unit->unit.active_camouflage < 0.f)
		{
			unit->unit.active_camouflage = 0.f;
		}
	}

	died_flying = died && damage->instantaneous_acceleration >= 2.f;

	if (!died &&
		TEST_FLAG(unit->unit.flags, _unit_feign_death_allowed_bit) &&
		unit_definition->unit.feign_death_threshold > 0.f &&
		unit_definition->unit.feign_death_time > 0.f &&
		unit->object.body_vitality > 0.f &&
		unit->object.recent_body_damage > unit_definition->unit.feign_death_threshold)
	{
		real feign_death_time = real_random_range(0.f, 1.f) + unit_definition->unit.feign_death_time;

		SET_FLAG(unit->object.damage_flags, _object_dead_bit, TRUE);
		feigned = TRUE;
		unit->unit.feign_death_timer = (short)MAX(1.f, feign_death_time * TICKS_PER_SECOND);
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 4740, unit->unit.feign_death_timer > 0);
	}

	if (!TEST_FLAG(damage_data->flags, _damage_silent_bit) &&
		(died || feigned || !TEST_FLAG(unit->object.damage_flags, _object_dead_bit)) &&
		!TEST_FLAG(unit->unit.flags, _unit_impervious_bit) &&
		!TEST_FLAG(damage->flags, _damage_does_not_ping_units_bit))
	{
		real_vector2d direction;
		real_vector2d facing;
		boolean ignore_hard_pings;
		boolean force_hard_pings;
		boolean direction_valid;
		real angle;

		direction.i = damage_data->direction.i;
		direction.j = damage_data->direction.j;
		facing.i = unit->object.forward.i;
		facing.j = unit->object.forward.j;
		ignore_hard_pings = FALSE;
		force_hard_pings = FALSE;
		direction_valid = FALSE;
		angle = 0.f;

		if (normalize2d(&direction) > 0.f && normalize2d(&facing) > 0.f)
		{
			angle = signed_angle_between_vectors2d(&direction, &facing);
			direction_valid = TRUE;
		}

		if (TEST_FLAG(unit_definition->unit.flags, _unit_resists_pings_bit) && !TEST_FLAG(damage->flags, _damage_pings_resistant_units_bit))
		{
			ignore_hard_pings = TRUE;
		}

		if (unit->unit.flaming_death_delay > 0)
		{
			ignore_hard_pings = TRUE;
		}

		if (TEST_FLAG(being_damaged_flags, _object_being_damaged_region_destroyed_bit) ||
			TEST_FLAG(being_damaged_flags, _object_being_damaged_shield_depleted_bit) ||
			TEST_FLAG(being_damaged_flags, _object_being_damaged_force_hard_ping_bit))
		{
			force_hard_pings = TRUE;
		}

		unit_ping_animation(
			unit_index,
			died,
			feigned,
			died_flying,
			ignore_hard_pings,
			force_hard_pings,
			angle,
			body_part,
			direction_valid ? &direction : NULL);
	}

	{
		long damaging_player_index = damage_data->owner_player_index;
		long damaged_player_index = unit->unit.player_index;

		if (damaging_player_index != NONE && damaged_player_index != NONE)
		{
			game_engine_player_damaged_player(damaging_player_index, damaged_player_index, TEST_FLAG(being_damaged_flags, _object_being_damaged_by_friendly_bit));
		}
	}

	if (damage_data->owner_player_index != NONE || damage_data->owner_object_index != NONE)
	{
		unit_record_damage(
		unit_index,
		total_damage,
		damage->category,
		died,
		damage_data->owner_player_index,
		damage_data->owner_team_index,
		damage_data->owner_object_index);
	}

	if (!TEST_FLAG(damage_data->flags, _damage_silent_bit) &&
		(TEST_FLAG(being_damaged_flags, _object_being_damaged_body_depleted_bit) || body_damage > 0.f || shield_damage > 0.f))
	{
		unit_make_damage_sound(
		unit_index,
		damage_data,
		feigned | died,
		TEST_FLAG(being_damaged_flags, _object_being_damaged_killed_instantly_bit),
		body_damage,
		shield_damage);
	}

	if (body_damage > 0.f || shield_damage > 0.f)
	{
		unit_unzoom(unit_index);
	}

	if (unit->object.type == _object_type_biped)
	{
		if (died)
		{
			ai_handle_death(unit_index, damage_data->owner_object_index, damage->category);
		}
		else if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			ai_handle_damage(unit_index, damage_data->owner_object_index, damage->category, total_damage, &damage_data->direction, FALSE);
		}
	}

	if (unit->unit.player_index != NONE && damage->stun > 0.f && (game_engine_running() || stun_enable))
	{
		short stun_ticks;
		short minimum_stun_ticks;
		short maximum_stun_ticks;
		struct game_globals_player_information *player_information = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information);
		real stun = damage->stun * damage_data->scale;
		real maximum_stun = damage->maximum_stun * damage_data->scale;

		if (stun < 0.f)
		{
			stun = 0.f;
		}

		if (maximum_stun < 0.f)
		{
			maximum_stun = 0.f;
		}

		if (maximum_stun >= 1.f)
		{
			maximum_stun = 1.f;
		}

		if (maximum_stun > unit->unit.body_stun)
		{
			unit->unit.body_stun += stun;

			if (unit->unit.body_stun > maximum_stun)
			{
				unit->unit.body_stun = maximum_stun;
			}
		}

		stun_ticks = (short)(damage->stun_time * TICKS_PER_SECOND);
		minimum_stun_ticks = (short)(player_information->minimum_stun_time * TICKS_PER_SECOND);
		maximum_stun_ticks = (short)(player_information->maximum_stun_time * TICKS_PER_SECOND);

		if (unit->unit.body_stun_ticks < minimum_stun_ticks)
		{
			unit->unit.body_stun_ticks = minimum_stun_ticks;
		}

		unit->unit.body_stun_ticks += stun_ticks;

		if (unit->unit.body_stun_ticks > maximum_stun_ticks)
		{
			unit->unit.body_stun_ticks = maximum_stun_ticks;
		}
	}

	if (died || feigned)
	{
		unit_died(unit_index, feigned);
	}

	return;
}

void unit_record_damage(
	long unit_index,
	real damage,
	short damage_category,
	boolean died,
	long owner_player_index,
	short owner_team_index,
	long owner_object_index)
{
	short attacker_index;

	struct unit_datum *unit = unit_get(unit_index);
	boolean found = FALSE;
	long game_time = game_time_get();

	for (attacker_index = 0; attacker_index < MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++)
	{
		struct unit_attacker *attacker = &unit->unit.attackers[attacker_index];

		if ((owner_player_index != NONE && attacker->player_index == owner_player_index) ||
			attacker->object_index == owner_object_index)
		{
			attacker->damage_inflicted += damage;
			attacker->game_time_stamp = game_time;
			found = TRUE;
		}
	}

	if (!found)
	{
		short best_new_attacker_index = NONE;

		for (attacker_index = 0; attacker_index < MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++)
		{
			if (unit->unit.attackers[attacker_index].game_time_stamp == NONE)
			{
				best_new_attacker_index = attacker_index;
				break;
			}
		}

		if (best_new_attacker_index == NONE)
		{
			short most_damaging_attacker_index = 0;

			for (attacker_index = 1; attacker_index < MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++)
			{
				if (unit->unit.attackers[attacker_index].damage_inflicted > unit->unit.attackers[most_damaging_attacker_index].damage_inflicted)
				{
					most_damaging_attacker_index = attacker_index;
				}
			}

			best_new_attacker_index = NONE;

			for (attacker_index = 0; attacker_index < MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++)
			{
				if (attacker_index != most_damaging_attacker_index &&
					(best_new_attacker_index == NONE || unit->unit.attackers[attacker_index].game_time_stamp < unit->unit.attackers[best_new_attacker_index].game_time_stamp))
				{
					best_new_attacker_index = attacker_index;
				}
			}

			match_assert("c:\\halo\\SOURCE\\units\\units.c", 4972, best_new_attacker_index!=NONE);
		}

		unit->unit.attackers[best_new_attacker_index].object_index = owner_object_index;
		unit->unit.attackers[best_new_attacker_index].player_index = owner_player_index;
		unit->unit.attackers[best_new_attacker_index].damage_inflicted = damage;
		unit->unit.attackers[best_new_attacker_index].game_time_stamp = game_time;
	}

	if (died && owner_team_index != NONE && game_team_is_enemy(unit->object.owner_team_index, owner_team_index))
	{
		struct unit_datum *killer_unit = NULL;
		long killer_unit_index = NONE;

		if (owner_player_index != NONE)
		{
			struct player_datum *player = player_get(owner_player_index);

			if (player->unit_index != NONE)
			{
				killer_unit_index = player->unit_index;
				killer_unit = unit_get(player->unit_index);
			}
		}

		if (!killer_unit)
		{
			killer_unit_index = owner_object_index;
			killer_unit = unit_try_and_get(owner_object_index);
		}

		if (killer_unit)
		{
			if (damage_category == _damage_category_vehicle)
			{
				if (killer_unit->unit.driver_object_index != NONE)
				{
					killer_unit_index = killer_unit->unit.driver_object_index;
					killer_unit = unit_get(killer_unit->unit.driver_object_index);
				}
			}
			else if (killer_unit->unit.gunner_object_index != NONE)
			{
				killer_unit_index = killer_unit->unit.gunner_object_index;
				killer_unit = unit_get(killer_unit->unit.gunner_object_index);
			}

			if (!TEST_FLAG(killer_unit->object.damage_flags, _object_dead_bit))
			{
				long time = game_time_get();

				if (killer_unit->unit.killing_spree_last_time == NONE || killer_unit->unit.killing_spree_last_time + 120 < time)
				{
					killer_unit->unit.killing_spree_count = 0;
				}

				killer_unit->unit.killing_spree_count++;
				killer_unit->unit.killing_spree_last_time = time;

				if (ai_handle_killing_spree(killer_unit_index, killer_unit->unit.killing_spree_count))
				{
					killer_unit->unit.killing_spree_count = 0;
				}
			}
		}
	}

	return;
}

static short unit_test_spawning(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short result = 0;

	if (!TEST_FLAG(unit->unit.flags, _unit_spawned_actors_bit))
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

		if (unit_definition->unit.spawned_actor_variant.index != NONE)
		{
			result = random_range(unit_definition->unit.spawn_actor_lower_bound, unit_definition->unit.spawn_actor_upper_bound + 1);

			if (result > 0)
			{
				result = actors_spawn_from_unit(
				unit_index,
				unit_definition->unit.spawned_actor_variant.index,
				result,
				unit_definition->unit.spawn_throw_velocity * (1.f / TICKS_PER_SECOND));
			}

			SET_FLAG(unit->unit.flags, _unit_spawned_actors_bit, TRUE);
		}
	}

	return result;
}

void unit_destroy(
	long unit_index)
{
	object_destroy(unit_index);
	unit_test_spawning(unit_index);

	return;
}

void unit_died(
	long unit_index,
	boolean feigned)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (!feigned)
	{
		unit->unit.feign_death_timer = 0;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 5099, unit->unit.feign_death_timer > 0);
	}

	if (feigned)
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

		SET_FLAG(unit->unit.flags, _unit_feign_death_allowed_bit, real_random() < unit_definition->unit.feign_death_repeat_chance);
	}
	else
	{
		object_set_garbage(unit_index, TRUE);

		if (unit->unit.player_index != NONE)
		{
			player_died(unit->unit.player_index);
			unit->unit.player_index = NONE;
		}

		if (unit->unit.actor_index != NONE)
		{
			struct actor_datum *actor = actor_get(unit->unit.actor_index);

			unit->unit.fake_encounter_index = (short)actor->meta.encounter_index;
			unit->unit.fake_squad_index = actor->meta.squad_index;
			actor_died(unit->unit.actor_index);
			unit->unit.actor_index = NONE;
		}

		if (unit->unit.swarm_actor_index != NONE)
		{
			struct actor_datum *actor = actor_get(unit->unit.swarm_actor_index);

			unit->unit.fake_encounter_index = (short)actor->meta.encounter_index;
			unit->unit.fake_squad_index = actor->meta.squad_index;
			actor_swarm_unit_died(unit->unit.swarm_actor_index, unit_index);
			unit->unit.swarm_actor_index = NONE;
		}

		unit->unit.time_of_death = game_time_get();
	}

	SET_FLAG(unit->unit.flags, _unit_actively_controlled_bit, FALSE);
	SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, FALSE);
	unit->unit.control_flags = 0;

	if (unit->unit.current_weapon_index != NONE)
	{
		weapon_owner_update(unit_get_current_weapon_index(unit_index), 0, 0.f);
	}

	unit_stop_running_blindly(unit_index);

	if (unit->object.parent_object_index != NONE)
	{
		if (unit->unit.parent_seat_index != NONE)
		{
			unit_exit_seat_end(unit_index);
		}
		else
		{
			unit_detach_from_parent(unit_index);
		}
	}

	unit->unit.speech.queued.priority = 0;
	unit_drop_inventory_weapons(unit_index);
	unit_drop_current_equipment(unit_index);
	unit_drop_grenades(unit_index);

	if (!unit->unit.weapon_drop_delay_ticks)
	{
		unit_drop_current_weapon(unit_index, TRUE);
	}

	unit->unit.animation.overlay_action_animation.index = NONE;
	unit->unit.animation.action_animation.index = NONE;
	unit->unit.melee_attack_state = 0;

	if (unit->unit.grenade_throw_state == _unit_grenade_throw_wind_up)
	{
		unit->unit.grenade_throw_state = _unit_grenade_throw_idle;
	}

	return;
}

void unit_get_head_position(
	long unit_index,
	union real_point3d *head_position)
{
	struct object_marker head_marker;

	object_get_marker_by_name(unit_index, "head", &head_marker, 1);
	*head_position = head_marker.matrix.position;

	return;
}

void unit_get_camera_position(
	long unit_index,
	real_point3d *camera_position)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->object.parent_object_index == NONE && !TEST_FLAG(unit->object.damage_flags, _object_dead_bit) && unit->object.type == _object_type_biped)
	{
		biped_get_sight_position(unit_index, 0.f, NULL, NULL, NULL, camera_position);
	}
	else if (unit->object.parent_object_index == NONE)
	{
		if (unit->unit.gunner_object_index == NONE)
		{
			struct object_marker head_marker;

			object_get_marker_by_name(unit_index, "head", &head_marker, 1);
			*camera_position = head_marker.matrix.position;
		}
		else
		{
			struct object_marker camera_marker;
			struct unit_datum *gunner_unit = unit_get(unit->unit.gunner_object_index);
			struct unit_seat *parent_seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, gunner_unit->unit.parent_seat_index, struct unit_seat);

			object_get_marker_by_name(unit_index, parent_seat->marker_name, &camera_marker, 1);
			*camera_position = camera_marker.matrix.position;
		}
	}
	else
	{
		struct object_datum *parent_object = object_get(unit->object.parent_object_index);

		*camera_position = parent_object->object.position;

		if (TEST_FLAG(_object_mask_unit, parent_object->object.type) && unit->unit.parent_seat_index != NONE)
		{
			struct unit_datum *parent_unit = (struct unit_datum *)parent_object;
			struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
			struct unit_seat *parent_seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

			if (parent_unit->object.type != _object_type_vehicle || parent_seat->camera.marker_name[0])
			{
				struct object_marker camera_marker;

				object_get_marker_by_name(unit->object.parent_object_index, parent_seat->camera.marker_name, &camera_marker, 1);
				*camera_position = camera_marker.matrix.position;
			}
		}
	}

	return;
}

void unit_estimate_position(
	long unit_index,
	short estimate_mode,
	real_point3d const *body_position,
	real_vector3d *desired_facing,
	real_vector3d *desired_gun_offset,
	real_point3d *estimated_position)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 5297, body_position && estimated_position);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 5298, (estimate_mode >= 0) && (estimate_mode < NUMBER_OF_UNIT_ESTIMATE_POSITION_MODES));

	if (unit->object.parent_object_index == NONE &&
		!TEST_FLAG(unit->object.damage_flags, _object_dead_bit) &&
		unit->object.type == _object_type_biped)
	{
		biped_get_sight_position(unit_index, estimate_mode, body_position, desired_facing, desired_gun_offset, estimated_position);
	}
	else
	{
		real_point3d current_body_position;
		real_vector3d offset_vector;
		boolean found_position = FALSE;

		if (unit->object.type == _object_type_biped && unit->object.parent_object_index != NONE)
		{
			struct object_datum *parent_object = object_get(unit->object.parent_object_index);

			if (parent_object->object.type == _object_type_vehicle)
			{
				long surface_index = vehicle_find_pathfinding_surface_index(unit->object.parent_object_index, &current_body_position);

				if (surface_index != NONE)
				{
					found_position = TRUE;
				}
			}
		}

		if (!found_position)
		{
			object_get_origin(unit_index, &current_body_position);
			found_position = TRUE;
		}

		unit_get_camera_position(unit_index, estimated_position);
		vector_from_points3d(&current_body_position, body_position, &offset_vector);
		point_from_line3d(estimated_position, &offset_vector, 1.f, estimated_position);
	}

	return;
}

void unit_get_center_of_mass(
	long unit_index,
	real_point3d *center_of_mass)
{
	struct object_marker center_of_mass_marker;

	object_get_marker_by_name(unit_index, "body", &center_of_mass_marker, 1);
	*center_of_mass = center_of_mass_marker.matrix.position;

	return;
}

static short unit_animation_impulse_get_index(
	short animation_impulse,
	short *interpolation_frame_count)
{
	short index = NONE;

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 5364, animation_impulse>=0 && animation_impulse<NUMBER_OF_UNIT_ANIMATION_IMPULSES);

	switch (animation_impulse)
	{
	case _unit_animation_impulse_berserk:
		index = _unit_weapon_class_animation_berserk;
		break;
	case _unit_animation_impulse_signal_move:
		index = _unit_weapon_class_animation_signal_move;
		break;
	case _unit_animation_impulse_signal_attack:
		index = _unit_weapon_class_animation_signal_attack;
		break;
	case _unit_animation_impulse_signal_warn:
		index = _unit_weapon_class_animation_signal_warn;
		break;
	case _unit_animation_impulse_surprise_front:
		index = _unit_weapon_class_animation_surprise_front;
		break;
	case _unit_animation_impulse_surprise_back:
		index = _unit_weapon_class_animation_surprise_back;
		break;
	case _unit_animation_impulse_evade_left:
		index = _unit_weapon_class_animation_evade_left;
		break;
	case _unit_animation_impulse_evade_right:
		index = _unit_weapon_class_animation_evade_right;
		break;
	case _unit_animation_impulse_dive_front:
		index = _unit_weapon_class_animation_diving_front;
		break;
	case _unit_animation_impulse_dive_back:
		index = _unit_weapon_class_animation_diving_back;
		break;
	case _unit_animation_impulse_dive_left:
		index = _unit_weapon_class_animation_diving_left;
		break;
	case _unit_animation_impulse_dive_right:
		index = _unit_weapon_class_animation_diving_right;
		break;
	case _unit_animation_impulse_vehicle_celebrate:
		index = _unit_weapon_class_animation_celebrate;
		break;
	case _unit_animation_impulse_vehicle_panic:
		index = _unit_weapon_class_animation_panic;
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\units\\units.c", 5383);
	}

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 5385, index!=NONE);

	if (interpolation_frame_count)
	{
		switch (animation_impulse)
		{
		case _unit_animation_impulse_surprise_front:
		case _unit_animation_impulse_surprise_back:
		case _unit_animation_impulse_dive_front:
		case _unit_animation_impulse_dive_back:
		case _unit_animation_impulse_dive_left:
		case _unit_animation_impulse_dive_right:
			*interpolation_frame_count = 3;
			break;
		case _unit_animation_impulse_berserk:
		case _unit_animation_impulse_signal_move:
		case _unit_animation_impulse_signal_attack:
		case _unit_animation_impulse_signal_warn:
		case _unit_animation_impulse_evade_left:
		case _unit_animation_impulse_evade_right:
		case _unit_animation_impulse_vehicle_celebrate:
		case _unit_animation_impulse_vehicle_panic:
			*interpolation_frame_count = 6;
			break;
		default:
			match_halt("c:\\halo\\SOURCE\\units\\units.c", 5412);
		}
	}

	return index;
}

static boolean unit_can_play_animation_impulse(
	long unit_index,
	short animation_impulse)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (unit_animation_busy(&unit->unit.animation))
	{
		result = FALSE;
	}
	else if (unit->object.parent_object_index != NONE)
	{
		if (unit->unit.parent_seat_index != NONE)
		{
			struct unit_datum *parent_unit = unit_try_and_get(unit->object.parent_object_index);

			if (parent_unit)
			{
				struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				switch (animation_impulse)
				{
				case _unit_animation_impulse_vehicle_celebrate:
				case _unit_animation_impulse_vehicle_panic:
					result = TEST_FLAG(seat->flags, _unit_seat_allow_vehicle_communication_animations_bit);
					break;
				}
			}
		}
	}
	else
	{
		switch (animation_impulse)
		{
		case _unit_animation_impulse_vehicle_celebrate:
		case _unit_animation_impulse_vehicle_panic:
			result = FALSE;
			break;
		default:
			result = TRUE;
			break;
		}
	}

	return result;
}

boolean unit_test_animation_impulse(
	long unit_index,
	short animation_impulse)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (unit_can_play_animation_impulse(unit_index, animation_impulse))
	{
		short interpolation_frame_count;
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
		struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
		short weapon_class_animation_index = unit_animation_impulse_get_index(animation_impulse, &interpolation_frame_count);
		short animation_index = weapon_class_animation_index >= 0 && weapon_class_animation_index < weapon_class->animations.count ?
			animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index :
			NONE;

		result = animation_index != NONE;
	}

	return result;
}

boolean unit_start_animation_impulse(
	long unit_index,
	short animation_impulse,
	real_vector2d *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (unit_can_play_animation_impulse(unit_index, animation_impulse))
	{
		short interpolation_frame_count;
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
		struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
		short weapon_class_animation_index = unit_animation_impulse_get_index(animation_impulse, &interpolation_frame_count);
		short animation_index = weapon_class_animation_index >= 0 && weapon_class_animation_index < weapon_class->animations.count ?
			animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index :
			NONE;

		if (animation_index != NONE)
		{
			object_start_interpolation(unit_index, interpolation_frame_count);
			unit_set_animation(unit_index, unit_definition->object.animation_graph.index, animation_choose_random_permutation(unit_definition->object.animation_graph.index, animation_index));
			unit->unit.animation.state = _unit_state_ai_impulse;
			SET_FLAG(unit->unit.animation.flags, _unit_animation_postpone_weapon_ik_until_interpolation_ends_bit, TRUE);

			if (alignment_vector && unit->object.type == _object_type_biped && unit->object.parent_object_index == NONE)
			{
				unit_align_facing(unit_index, alignment_vector);
			}

			result = TRUE;
		}
	}

	return result;
}

long unit_get_aiming_unit_index(
	long unit_index)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
		{
			struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
			struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

			if (TEST_FLAG(seat->flags, _unit_seat_is_invisible_bit) || TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit))
			{
				unit_index = unit->object.parent_object_index;
			}
		}
	}

	return unit_index;
}

void unit_get_aiming_vector(
	long unit_index,
	real_vector3d *aiming_vector)
{
	*aiming_vector = unit_get(unit_index)->unit.aiming_vector;

	return;
}

void unit_get_looking_vector(
	long unit_index,
	real_vector3d *looking_vector)
{
	*looking_vector = unit_get(unit_index)->unit.looking_vector;

	return;
}

void unit_get_facing_vector(
	long unit_index,
	real_vector3d *facing_vector)
{
	object_get_orientation(unit_index, facing_vector, NULL);

	return;
}

boolean unit_clip_to_aiming_bounds(
	long unit_index,
	real_vector3d *vector,
	boolean use_aiming_screen)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;
	boolean aiming = use_aiming_screen ? unit->unit.animation.aiming_with_euler_screen : unit->unit.animation.looking_with_euler_screen;
	real_rectangle2d *bounds = use_aiming_screen ? &unit->unit.animation.aiming_screen_bounds : &unit->unit.animation.looking_screen_bounds;

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 5608, vector);

	if (aiming)
	{
		real_matrix4x3 matrix;
		real_euler_angles2d relative_aiming_angles;

		matrix.scale = 1.f;
		object_get_orientation(unit_index, &matrix.forward, &matrix.up);
		cross_product3d(&matrix.up, &matrix.forward, &matrix.left);
		matrix.position = *global_origin3d;

		{
			real_vector3d relative_vector;

			matrix4x3_inverse_transform_normal(&matrix, vector, &relative_vector);

			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5626, &relative_vector);
			euler_angles2d_from_vector3d(&relative_aiming_angles, &relative_vector);
		}

		match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5629, relative_aiming_angles.pitch);
		match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5630, relative_aiming_angles.yaw);

		if (relative_aiming_angles.yaw < bounds->x0)
		{
			relative_aiming_angles.yaw = bounds->x0;
			result = TRUE;
		}
		else
		{
			if (relative_aiming_angles.yaw > bounds->x1)
			{
				relative_aiming_angles.yaw = bounds->x1;
				result = TRUE;
			}
		}

		if (relative_aiming_angles.pitch < bounds->y0)
		{
			result = TRUE;
			relative_aiming_angles.pitch = bounds->y0;
		}
		else
		{
			if (relative_aiming_angles.pitch > bounds->y1)
			{
				result = TRUE;
				relative_aiming_angles.pitch = bounds->y1;
			}
		}

		if (result)
		{
			real_vector3d relative_vector;

			match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5661, relative_aiming_angles.pitch);
			match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5662, relative_aiming_angles.yaw);
			vector3d_from_euler_angles2d(&relative_vector, &relative_aiming_angles);

			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5665, &relative_vector);
			matrix4x3_transform_normal(&matrix, &relative_vector, vector);
			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5667, vector);
		}
	}

	return result;
}

long unit_inventory_get_weapon(
	long unit_index,
	short index)
{
	struct unit_datum *unit = unit_get(unit_index);

	return unit_get_weapon(unit, index);
}

short unit_inventory_next_grenade(
	long unit_index,
	short current_index,
	short delta)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	short next_index = NONE;

	if (current_index == NONE)
	{
		current_index = 0;
	}

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 5694, current_index>=0 && current_index<NUMBER_OF_UNIT_GRENADE_TYPES);

	index = current_index;

	do
	{
		if (unit->unit.grenade_counts[index] > 0)
		{
			next_index = index;

			if (index != current_index || delta == 0)
			{
				break;
			}
		}

		if (delta < 0)
		{
			index = index == 0 ? NUMBER_OF_UNIT_GRENADE_TYPES - 1 : index - 1;
		}
		else
		{
			index = index == NUMBER_OF_UNIT_GRENADE_TYPES - 1 ? 0 : index + 1;
		}
	}
	while (index != current_index);

	return next_index;
}

short unit_inventory_next_weapon(
	long unit_index,
	short current_index,
	short delta)
{
	return unit_weapon_next_index(unit_index, current_index, delta);
}

boolean unit_melee_attack_begin(
	long unit_index,
	boolean continuous,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	boolean result = FALSE;

	if (!unit_animation_busy(&unit->unit.animation))
	{
		short state;
		boolean airborne = FALSE;

		if (unit->object.type == _object_type_biped)
		{
			struct biped_datum *biped = (struct biped_datum *)unit;

			airborne = TEST_FLAG(biped->biped.flags, _biped_airborne_bit);
		}

		if (continuous)
		{
			state = _unit_state_melee_continuous;
		}
		else if (unit->unit.animation.state == _unit_state_leap_airborne)
		{
			state = _unit_state_leap_melee;
		}
		else if (airborne)
		{
			state = _unit_state_melee_airborne;
		}
		else
		{
			state = _unit_state_melee_attack;
		}

		if (unit_animation_set_state(unit_index, state) || continuous)
		{
			if (TEST_FLAG(unit_definition->unit.flags, _unit_melee_attack_is_fatal_bit))
			{
				unit->unit.animation.state = _unit_state_dying;
			}

			if (alignment_vector)
			{
				unit_align_facing(unit_index, alignment_vector);
			}

			if (continuous)
			{
				unit->unit.melee_attack_state = 4;
				unit->unit.melee_continuous_damage_effect_timer = 0;
			}
			else
			{
				unit->unit.melee_attack_state = 1;
			}

			result = TRUE;
		}
	}

	return result;
}

boolean unit_leap_begin(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (!unit_animation_busy(&unit->unit.animation))
	{
		boolean airborne = FALSE;

		if (unit->object.type == _object_type_biped)
		{
			struct biped_datum *biped = (struct biped_datum *)unit;

			airborne = TEST_FLAG(biped->biped.flags, _biped_airborne_bit);
		}

		if (!airborne && unit_animation_set_state(unit_index, _unit_state_leap_start))
		{
			if (alignment_vector)
			{
				unit_align_facing(unit_index, alignment_vector);
			}

			result = TRUE;
		}
	}

	return result;
}

boolean unit_controllable(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	return TEST_FLAG(unit->unit.flags, _unit_controllable_bit);
}

void unit_set_controllable(
	long unit_index,
	boolean controllable)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->unit.flags, _unit_controllable_bit, controllable);

	return;
}

void unit_set_possessed(
	long unit_index,
	boolean possessed)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->unit.flags, _unit_possessed_by_recording_bit, possessed);

	return;
}

void unit_set_actively_controlled(
	long unit_index,
	boolean actively_controlled)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.actor_index != NONE || unit->unit.swarm_actor_index != NONE || unit->unit.player_index != NONE)
	{
		actively_controlled = TRUE;
	}

	if (TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		actively_controlled = FALSE;
	}

	SET_FLAG(unit->unit.flags, _unit_actively_controlled_bit, actively_controlled);
	SET_FLAG(unit->unit.flags, _unit_controllable_bit, actively_controlled);

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		if (unit->unit.weapon_object_indices[index] != NONE)
		{
			item_in_unit_inventory(unit->unit.weapon_object_indices[index], unit_index);
		}
	}

	unit_update_driver_and_gunner(unit_index);

	return;
}

boolean unit_is_busy(
	long object_index)
{
	struct unit_datum *unit = unit_get(object_index);

	return unit_animation_busy(&unit->unit.animation);
}

void unit_scripting_set_emotion_animation(
	long unit_index,
	char const *animation_name)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		short animation_index = animation_graph_get_animation_by_name(unit->object.animation.animation_graph_index, animation_name);

		if (animation_index != NONE)
		{
			unit->unit.override_emotion_animation_index = animation_index;
		}
		else
		{
			console_warning("couldn't find the emotion animation '%s'", animation_name);
		}
	}

	return;
}

void unit_scripting_enter_vehicle(
	long unit_index,
	long parent_unit_index,
	char const *seat_name)
{
	if (unit_index != NONE && parent_unit_index != NONE && strlen(seat_name) > 0)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
			{
				unit_exit_seat_end(unit_index);
			}

			if (unit->object.parent_object_index == NONE)
			{
				short seat_index;
				struct unit_datum *parent_unit = unit_get(parent_unit_index);
				struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);

				for (seat_index = 0; seat_index < parent_unit_definition->unit.seats.count; seat_index++)
				{
					struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, seat_index, struct unit_seat);

					if (!_stricmp(seat_name, seat->label) &&
						!unit_seat_filled(parent_unit_index, seat_index) &&
						(unit->object.type == _object_type_vehicle || unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, NULL, FALSE)))
					{
						unit_enter_seat(unit_index, parent_unit_index, seat_index);
						return;
					}
				}
			}
		}
	}

	return;
}

void unit_scripting_suspended(
	long unit_index,
	boolean suspend)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		SET_FLAG(unit->unit.flags, _unit_suspended_bit, suspend);
		unit->object.translational_velocity = *global_zero_vector3d;

		if (unit->object.type == _object_type_biped)
		{
			struct biped_datum *biped = biped_get(unit_index);

			SET_FLAG(biped->biped.flags, _biped_airborne_bit, FALSE);
		}
	}

	return;
}

void unit_scripting_can_blink(
	long unit_index,
	boolean can_blink)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		SET_FLAG(unit->unit.flags, _unit_cannot_blink_bit, !can_blink);
	}

	return;
}

short vehicle_scripting_find_available_seats(
	long unit_index,
	char const *seat_substring_name,
	short seat_desire_type,
	short *seat_indices,
	short seat_max_indices)
{
	boolean any_seat_name;
	short seat_count;
	short seat_index;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 6031, seat_substring_name);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 6032, (seat_desire_type == NONE) || ((seat_desire_type >= 0) && (seat_desire_type < NUMBER_OF_VEHICLE_SEAT_DESIRE_TYPES)));

	any_seat_name = !seat_substring_name || strlen(seat_substring_name) == 0;
	seat_count = 0;

	for (seat_index = 0; seat_index < unit_definition->unit.seats.count; seat_index++)
	{
		char labelname[256];
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, seat_index, struct unit_seat);

		if (seat_count >= seat_max_indices)
		{
			break;
		}

		strcpy(labelname, seat->label);
		strlwr(labelname);

		if (any_seat_name || strstr(labelname, seat_substring_name))
		{
			boolean desired = TRUE;

			switch (seat_desire_type)
			{
			case _vehicle_seat_desire_any_non_driver:
				desired = !TEST_FLAG(seat->flags, _unit_seat_is_driver_bit);
				break;
			case _vehicle_seat_desire_gunner:
				desired = TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit);
				break;
			case _vehicle_seat_desire_passenger:
				desired = !TEST_FLAG(seat->flags, _unit_seat_is_driver_bit) && !TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit);
				break;
			case _vehicle_seat_desire_driver:
				desired = TEST_FLAG(seat->flags, _unit_seat_is_driver_bit);
				break;
			}

			if (desired && !unit_seat_filled(unit_index, seat_index))
			{
				seat_indices[seat_count++] = seat_index;
			}
		}
	}

	return seat_count;
}

short vehicle_scripting_load_magic(
	long unit_index,
	char const *seat_substring_name,
	long object_list_index)
{
	short count = 0;

	if (unit_index != NONE)
	{
		short seat_indices[16];
		long object_list_reference_index;
		long object_index;
		struct unit_datum *vehicle = unit_get(unit_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
		short seat_count = vehicle_scripting_find_available_seats(
		unit_index,
		seat_substring_name,
		NONE,
		seat_indices,
		NUMBEROF(seat_indices));

		for (object_index = object_list_get_first(object_list_index, &object_list_reference_index);
			object_index != NONE;
			object_index = object_list_get_next(object_list_index, &object_list_reference_index))
		{
			struct unit_datum *object = (struct unit_datum *)object_get(object_index);

			if (TEST_FLAG(_object_mask_unit, object->object.type) && !TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
			{
				short index;

				for (index = 0; index < seat_count; index++)
				{
					short seat_index = seat_indices[index];

					if (seat_index != NONE)
					{
						struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

						if (object->object.type == _object_type_vehicle || unit_set_or_test_seat_and_weapon_label(object_index, seat->label, NULL, FALSE))
						{
							if (object->object.parent_object_index != NONE && object->unit.parent_seat_index != NONE)
							{
								unit_exit_seat_end(object_index);
							}

							if (object->object.parent_object_index == NONE && unit_enter_seat(object_index, unit_index, seat_index))
							{
								seat_indices[index] = NONE;
								count++;
								break;
							}
						}
					}
				}
			}
		}
	}

	return count;
}

void unit_open(
	long unit_index)
{
	if (unit_index != NONE)
	{
		unit_animation_set_state(unit_index, _unit_state_opening);
	}

	return;
}

void unit_close(
	long unit_index)
{
	if (unit_index != NONE)
	{
		unit_animation_set_state(unit_index, _unit_state_closing);
	}

	return;
}

short vehicle_scripting_unload(
	long unit_index,
	char const *seat_substring_name)
{
	short count = 0;

	if (unit_index != NONE)
	{
		struct object_iterator iterator;
		struct unit_datum *unit;
		struct unit_datum *vehicle = unit_get(unit_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
		boolean any_seat_name = !seat_substring_name || !strlen(seat_substring_name);

		object_iterator_new(&iterator, _object_mask_unit, 0);

		while ((unit = object_iterator_next(&iterator)) != NULL)
		{
			if (unit->object.parent_object_index == unit_index)
			{
				char labelname[256];
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				strcpy(labelname, seat->label);
				strlwr(labelname);

				if ((any_seat_name || strstr(labelname, seat_substring_name)) && unit_try_and_exit_seat(iterator.index))
				{
					count++;
				}
			}
		}
	}

	return count;
}

void unit_scripting_doesnt_drop_items(
	long object_list_index)
{
	long object_list_reference_index;
	long unit_index;

	for (unit_index = object_list_get_first(object_list_index, &object_list_reference_index);
		unit_index != NONE;
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index))
	{
		struct unit_datum *unit = unit_try_and_get(unit_index);

		if (unit)
		{
			SET_FLAG(unit->unit.flags, _unit_doesnt_drop_items_bit, TRUE);
		}
	}

	return;
}

boolean unit_scripting_vehicle_test_seat_list(
	long unit_index,
	char const *seat_name,
	long object_list_index)
{
	boolean result = FALSE;

	if (unit_index != NONE)
	{
		short seat_index;
		struct unit_datum *vehicle = unit_get(unit_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);

		for (seat_index = 0; seat_index < vehicle_definition->unit.seats.count; seat_index++)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

			if (!_stricmp(seat_name, seat->label))
			{
				struct object_iterator iterator;
				struct unit_datum *unit;
				boolean found = FALSE;

				object_iterator_new(&iterator, _object_mask_unit, 0);

				while ((unit = object_iterator_next(&iterator)) != NULL)
				{
					if (unit->object.parent_object_index == unit_index && unit->unit.parent_seat_index == seat_index)
					{
						found = TRUE;
						break;
					}
				}

				if (found)
				{
					long object_list_reference_index;
					long object_index;

					for (object_index = object_list_get_first(object_list_index, &object_list_reference_index);
						object_index != NONE && iterator.index != object_index;
						object_index = object_list_get_next(object_list_index, &object_list_reference_index));

					if (iterator.index == object_index)
					{
						result = TRUE;
					}
				}
			}
		}
	}

	return result;
}

boolean unit_scripting_vehicle_test_seat(
	long unit_index,
	char const *seat_name,
	long child_index)
{
	boolean result = FALSE;

	if (unit_index != NONE && child_index != NONE)
	{
		short seat_index;
		struct unit_datum *vehicle = unit_get(unit_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);

		for (seat_index = 0; seat_index < vehicle_definition->unit.seats.count; seat_index++)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

			if (!_stricmp(seat_name, seat->label))
			{
				struct unit_datum *child = unit_get(child_index);

				if (child->object.parent_object_index == unit_index && child->unit.parent_seat_index == seat_index)
				{
					result = TRUE;
					break;
				}
			}
		}
	}

	return result;
}

void unit_scripting_exit_vehicle(
	long unit_index)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
		{
			unit_try_and_exit_seat(unit_index);
		}
	}

	return;
}

long unit_scripting_unit_riders(
	long unit_index)
{
	long object_list_index = NONE;

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		object_list_index = object_list_new();

		if (object_list_index != NONE)
		{
			long child_object_index = unit->object.first_child_object_index;

			while (child_object_index != NONE)
			{
				struct object_datum *child_object = object_get(child_object_index);

				if (TEST_FLAG(_object_mask_unit, child_object->object.type))
				{
					struct unit_datum *child_unit = (struct unit_datum *)child_object;

					if (child_unit->unit.parent_seat_index != NONE)
					{
						object_list_add(object_list_index, child_object_index);
					}
				}

				child_object_index = child_object->object.next_object_index;
			}
		}
	}

	return object_list_index;
}

long unit_scripting_unit_driver(
	long unit_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);
	long driver_index = NONE;

	if (unit)
	{
		driver_index = unit->unit.driver_object_index;
	}

	return driver_index;
}

long unit_scripting_unit_gunner(
	long unit_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);
	long gunner_index = NONE;

	if (unit)
	{
		gunner_index = unit->unit.gunner_object_index;
	}

	return gunner_index;
}

boolean unit_unsuspecting(
	long unit_index,
	real_point3d const *attacker)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);

	if (unit && unit->object.type == _object_type_biped)
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

		if (!TEST_FLAG(unit_definition->unit.flags, _unit_never_unsuspecting_bit))
		{
			real_vector3d attacker_to_unit;

			vector_from_points3d(attacker, &unit->object.bounding_sphere_center, &attacker_to_unit);

			if (dot_product3d(&attacker_to_unit, &unit->unit.looking_vector) > 0.f)
			{
				return TRUE;
			}

			if (!strcmp(base_seat_label_get(_unit_base_seat_asleep), unit_get_seat_label(unit_index)))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

void unit_shield_sapping_update(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->unit.animation.state == _unit_state_shield_sapping)
	{
		struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

		// the original compares the animation index (not the frame index) against the loop frame
		if (unit->object.animation.state.index >= animation_loop_frame_index(animation))
		{
			struct data_iterator iterator;
			struct player_datum *player;
			boolean damaged = FALSE;
			real radius = 4.f;

			data_iterator_new(&iterator, player_data);

			while ((player = data_iterator_next(&iterator)) != NULL)
			{
				if (player->unit_index != NONE)
				{
					struct unit_datum *player_unit = unit_get(player->unit_index);

					if (distance_squared3d(&player_unit->object.bounding_sphere_center, &unit->object.bounding_sphere_center) < radius * radius)
					{
						struct damage_data damage;

						damage_data_new(&damage, unit_definition->unit.melee_damage.index);
						damage.owner_object_index = unit_index;
						object_cause_damage(&damage, player->unit_index, NONE, NONE, NONE, NULL);
						damaged = TRUE;
					}
				}
			}

			if (!damaged)
			{
				unit->unit.shield_sap_timeout++;
			}
			else
			{
				unit->unit.shield_sap_timeout = 0;
			}
		}
	}

	return;
}

boolean unit_overcharged(
	long unit_index)
{
	long weapon_index = unit_get_current_weapon_index(unit_index);

	if (weapon_index != NONE)
	{
		return weapon_overcharged(weapon_index);
	}

	return FALSE;
}

long units_debug_get_next_unit(
	long unit_index)
{
	struct unit_datum *unit;
	long next_unit_index = NONE;

	if (unit_index != NONE)
	{
		struct object_iterator iterator;

		object_iterator_new(&iterator, _object_mask_unit, 0);

		while (object_iterator_next(&iterator))
		{
			if (iterator.index == unit_index)
			{
				break;
			}
		}

		while ((unit = object_iterator_next(&iterator)) != NULL)
		{
			if (unit->unit.actor_index == NONE &&
				unit->unit.swarm_actor_index == NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				next_unit_index = iterator.index;
				break;
			}
		}
	}

	if (next_unit_index == NONE)
	{
		struct object_iterator iterator;

		object_iterator_new(&iterator, _object_mask_unit, 0);

		while ((unit = object_iterator_next(&iterator)) != NULL)
		{
			if (unit->unit.actor_index == NONE &&
				unit->unit.swarm_actor_index == NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				next_unit_index = iterator.index;
				break;
			}
		}
	}

	return next_unit_index;
}

long units_debug_get_closest_unit(
	long unit_index)
{
	struct object_iterator iterator;
	struct unit_datum *unit;
	long closest_unit_index = NONE;
	real closest_distance = REAL_MAX;

	object_iterator_new(&iterator, _object_mask_biped, 0);

	while ((unit = object_iterator_next(&iterator)) != NULL)
	{
		if (iterator.index != unit_index && !TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			real_point3d current_origin;
			real_point3d other_origin;
			real distance;

			if (unit_index != NONE)
			{
				object_get_origin(unit_index, &current_origin);
				object_get_origin(iterator.index, &other_origin);
				distance = distance3d(&current_origin, &other_origin);
			}
			else
			{
				distance = 0.f;
			}

			if (distance < closest_distance)
			{
				closest_unit_index = iterator.index;
				closest_distance = distance;
			}
		}
	}

	return closest_unit_index;
}

void unit_debug_ninja_rope(
	long unit_index)
{
	struct collision_result collision;
	real_vector3d vector;
	real_point3d point;
	struct unit_datum *unit = unit_get(unit_index);

	unit_get_camera_position(unit_index, &point);
	scale_vector3d(&unit->unit.aiming_vector, 25.f, &vector);

	match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\units.c", 6648, _collision_user_debugging);

	if (collision_test_vector(FLAG(_collision_test_back_facing_surfaces_bit) | FLAG(_collision_test_structure_bit), &point, &vector, unit_index, &collision) &&
		collision.plane.n.k > 0.95f)
	{
		collision.point.z += 0.25f;
		object_translate(unit_index, &collision.point, NULL);
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\units\\units.c", 6657);

	return;
}

boolean unit_set_seat(
	long unit_index,
	char const *name)
{
	boolean result = FALSE;

	if (unit_set_or_test_seat_and_weapon_label(unit_index, name, NULL, TRUE))
	{
		result = TRUE;
	}

	return result;
}

boolean unit_set_user_animation(
	long unit_index,
	char const *name,
	short index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 6689, index>=0 && index<NUMBER_OF_UNIT_USER_ANIMATIONS);

	return FALSE;
}

boolean any_unit_is_dangerous(
	void)
{
	struct object_iterator iterator;
	struct unit_datum *unit;

	object_iterator_new(&iterator, _object_mask_unit, 1);

	while ((unit = (struct unit_datum *)object_iterator_next(&iterator)) != NULL)
	{
		if (unit->unit.animation.state == _unit_state_throw_grenade && unit->unit.grenade_throw_state != _unit_grenade_throw_ending)
		{
			return TRUE;
		}

		if ((unit->unit.animation.state == _unit_state_dying || unit->unit.animation.state == _unit_state_dying_airborne) &&
			!TEST_FLAG(unit->unit.animation.flags, _unit_animation_ignore_translation_bit))
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean unit_can_see_point(
	long unit_index,
	real_point3d const *point,
	real theta)
{
	boolean result = FALSE;

	if (unit_index != NONE)
	{
		real_point3d head_position;
		real_vector3d unit_to_point;
		struct unit_datum *unit = unit_get(unit_index);

		unit_get_head_position(unit_index, &head_position);
		vector_from_points3d(&head_position, point, &unit_to_point);
		normalize3d(&unit_to_point);

		if (dot_product3d(&unit_to_point, &unit->unit.looking_vector) > cosine(theta))
		{
			result = TRUE;
		}
	}

	return result;
}

boolean unit_driven_by_ai(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.driver_object_index != NONE)
	{
		unit = unit_get(unit->unit.driver_object_index);
	}

	return unit->unit.actor_index != NONE;
}

boolean unit_gunned_by_ai(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.gunner_object_index != NONE)
	{
		unit = unit_get(unit->unit.gunner_object_index);
	}

	return unit->unit.actor_index != NONE;
}

void units_set_desired_flashlight_state(
	long object_list_index,
	boolean state)
{
	long reference_index;
	long object_index;

	for (object_index = object_list_get_first(object_list_index, &reference_index); object_index != NONE; object_index = object_list_get_next(object_list_index, &reference_index))
	{
		if (unit_try_and_get(object_index))
		{
			unit_set_desired_flashlight_state(object_index, state);
		}
	}

	return;
}

void unit_set_desired_flashlight_state(
	long unit_index,
	boolean state)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (state)
		{
			SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit, TRUE);
		}
		else
		{
			SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit, TRUE);
		}
	}

	return;
}

boolean unit_get_current_flashlight_state(
	long unit_index)
{
	boolean result = FALSE;

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		result = TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit);
	}

	return result;
}

static char const *unit_get_seat_label(
	long object_index)
{
	char const *result;

	struct unit_datum *unit = unit_get(object_index);

	if (unit->object.parent_object_index == NONE || unit->unit.parent_seat_index == NONE)
	{
		result = base_seat_label_get(unit->unit.animation.base_seat_index);
	}
	else
	{
		struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
		struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
		result = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat)->label;
	}

	return result;
}

boolean unit_try_and_exit_seat(
	long object_index)
{
	struct unit_datum *unit = unit_get(object_index);
	boolean result = FALSE;

	if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
	{
		if (unit->object.type == _object_type_vehicle)
		{
			unit_exit_seat_end(object_index);
		}
		else if (!unit_animation_busy(&unit->unit.animation))
		{
			struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
			struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
			short animation_index = unit_seat->animations.count <= _unit_seat_animation_seat_exit ?
				NONE :
				animation_graph_animation_index_get(&unit_seat->animations)[_unit_seat_animation_seat_exit].animation_index;

			if (animation_index != NONE)
			{
				if (unit_get(unit->object.parent_object_index)->unit.driver_object_index == object_index)
				{
					unit_open(unit->object.parent_object_index);
				}

				unit_set_animation(object_index, unit_definition->object.animation_graph.index, animation_choose_random_permutation(unit_definition->object.animation_graph.index, animation_index));
				object_set_visibility(object_index, TRUE);
				unit->unit.animation.state = _unit_state_exiting_seat;
				ai_handle_exit_vehicle(object_index, unit->object.parent_object_index);
				result = TRUE;
			}
		}
	}

	return result;
}

void unit_exit_seat_end(
	long object_index)
{
	struct unit_datum *unit = unit_get(object_index);

	if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
	{
		real_vector3d exit_offset;
		real_vector3d transformed_exit_offset;
		real_point3d default_root_node_translation;
		real_point3d new_position;
		struct object_marker marker;
		long parent_object_index = unit->object.parent_object_index;
		struct unit_datum *parent_unit = unit_get(parent_object_index);
		real_matrix4x3 const *default_matrix = NULL;
		struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);
		real_matrix4x3 *node_matrix = object_get_node_matrix(object_index, 0);

		object_get_marker_by_name(parent_object_index, seat->marker_name, &marker, 1);
		vector_from_points3d(&marker.matrix.position, &node_matrix->position, &exit_offset);
		matrix4x3_inverse_transform_vector(&marker.matrix, &exit_offset, &transformed_exit_offset);

		{
			struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
			struct model *model = model_definition_get(unit_definition->object.model.index);
			struct model_node *root_node = TAG_BLOCK_GET_ELEMENT(&model->nodes, 0, struct model_node);

			default_matrix = &root_node->runtime_default_inverse_matrix;
			default_root_node_translation = root_node->default_translation;
		}

		if (parent_unit->unit.driver_object_index == object_index && parent_unit->unit.animation.state != _unit_state_opening)
		{
			unit_open(unit->object.parent_object_index);
		}

		unit->unit.last_vehicle_index = parent_object_index;
		unit->unit.game_time_at_last_vehicle_exit = game_time_get();

		if (unit->unit.driver_object_index == object_index)
		{
			unit->unit.driver_object_index = NONE;
		}

		if (unit->unit.gunner_object_index == object_index)
		{
			unit->unit.gunner_object_index = NONE;
		}

		object_detach(object_index);
		new_position.x = exit_offset.i + unit->object.position.x;
		new_position.y = exit_offset.j + unit->object.position.y;
		new_position.z = exit_offset.k + unit->object.position.z - default_root_node_translation.z;
		object_set_position(object_index, &new_position, NULL, NULL);

		{
			real_matrix4x3 matrix;
			real_matrix4x3 *new_node_matrix = object_get_node_matrix(object_index, 0);

			matrix4x3_multiply(new_node_matrix, default_matrix, &matrix);
			unit->object.forward = matrix.forward;
			unit->object.up = matrix.up;
		}

		object_set_visibility(object_index, TRUE);
		unit->unit.parent_seat_index = NONE;
		unit->unit.animation.base_seat_index = _unit_base_seat_stand;

		if (parent_unit->unit.driver_object_index == object_index)
		{
			parent_unit->unit.driver_object_index = NONE;
		}

		if (parent_unit->unit.gunner_object_index == object_index)
		{
			parent_unit->unit.gunner_object_index = NONE;
		}

		unit_update_driver_and_gunner(parent_object_index);
		unit_adjust_for_seat_change(object_index);

		{
			struct unit_animation_update_data data = {_unit_state_airborne, FALSE};

			unit_update_animation(object_index, &data);
		}

		((real_orientation *)object_header_block_get(object_index, &unit->object.original_node_orientations))->translation = default_root_node_translation;

		if (unit->object.type == _object_type_biped)
		{
			biped_exit_seat_end(object_index, parent_object_index);
		}

		object_compute_node_matrices_recursive(object_index);
	}

	return;
}

void unit_detach_from_parent(
	long object_index)
{
	struct unit_datum *unit = unit_get(object_index);

	if (unit->object.parent_object_index != NONE)
	{
		real_vector3d detach_velocity;
		struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);

		{
			real_point3d parent_origin;
			real_point3d unit_origin;

			object_get_origin(unit->object.parent_object_index, &parent_origin);
			object_get_origin(object_index, &unit_origin);
			vector_from_points3d(&parent_origin, &unit_origin, &detach_velocity);
		}

		if (normalize3d(&detach_velocity) == 0.f)
		{
			detach_velocity = unit->object.forward;
		}

		scale_vector3d(&detach_velocity, 0.6f / TICKS_PER_SECOND, &detach_velocity);
		object_detach(object_index);

		{
			real_point3d new_position = unit->object.position;

			scenario_ensure_point_within_world(&new_position);
			object_set_position(object_index, &new_position, NULL, NULL);
		}

		SET_FLAG(unit->object.flags, _object_at_rest_bit, FALSE);
		SET_FLAG(unit->unit.flags, _unit_attached_melee_attack_bit, FALSE);
		add_vectors3d(&unit->object.translational_velocity, &detach_velocity, &unit->object.translational_velocity);
		object_set_visibility(object_index, TRUE);
		object_compute_node_matrices(object_index);
	}

	return;
}

boolean unit_seat_filled(
	long parent_object_index,
	short seat_index)
{
	struct object_iterator iterator;
	struct unit_datum *unit;
	boolean result = FALSE;

	object_iterator_new(&iterator, _object_mask_unit, 0);

	while ((unit = (struct unit_datum *)object_iterator_next(&iterator)) != NULL)
	{
		if (unit->object.parent_object_index == parent_object_index && unit->unit.parent_seat_index == seat_index)
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

boolean unit_seat_is_driver(
	long vehicle_index,
	short seat_index)
{
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
	boolean result = FALSE;

	if (seat_index >= 0 && seat_index < vehicle_definition->unit.seats.count)
	{
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

		result = TEST_FLAG(seat->flags, _unit_seat_is_driver_bit);
	}

	return result;
}

boolean unit_seat_is_gunner(
	long vehicle_index,
	short seat_index)
{
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
	boolean result = FALSE;

	if (seat_index >= 0 && seat_index < vehicle_definition->unit.seats.count)
	{
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

		result = TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit);
	}

	return result;
}

boolean unit_seat_allow_noncombatants(
	long vehicle_index,
	short seat_index)
{
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
	boolean result = FALSE;

	if (seat_index >= 0 && seat_index < vehicle_definition->unit.seats.count)
	{
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

		result = TEST_FLAG(seat->flags, _unit_seat_allow_noncombatants_bit);
	}

	return result;
}

static void unit_update_driver_and_gunner(
	long parent_object_index)
{
	struct unit_datum *parent_unit = unit_get(parent_object_index);
	struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
	long child_object_index = parent_unit->object.first_child_object_index;

	while (child_object_index != NONE)
	{
		struct object_datum *child_object = object_get(child_object_index);

		if (TEST_FLAG(_object_mask_unit, child_object->object.type))
		{
			struct unit_datum *child_unit = (struct unit_datum *)child_object;

			if (child_unit->unit.parent_seat_index != NONE)
			{
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, child_unit->unit.parent_seat_index, struct unit_seat);

				if (TEST_FLAG(seat->flags, _unit_seat_is_driver_bit) &&
					!TEST_FLAG(parent_unit->unit.flags, _unit_actively_controlled_bit) &&
					parent_unit->unit.driver_object_index == NONE)
				{
					parent_unit->unit.driver_object_index = child_object_index;

					if (TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit) && parent_unit->unit.gunner_object_index == NONE)
					{
						parent_unit->unit.gunner_object_index = child_object_index;
					}
				}
				else if (TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit) &&
					(parent_unit->unit.gunner_object_index == NONE || parent_unit->unit.gunner_object_index == parent_unit->unit.driver_object_index))
				{
					parent_unit->unit.gunner_object_index = child_object_index;
				}
			}
		}

		child_object_index = child_object->object.next_object_index;
	}

	return;
}

static void unit_add_initial_weapons(
	long unit_index)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	for (index = 0; index < unit_definition->unit.initial_weapons.count; index++)
	{
		struct unit_initial_weapon *initial_weapon = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.initial_weapons, index, struct unit_initial_weapon);

		if (initial_weapon->reference.index != NONE)
		{
			struct object_placement_data placement_data;
			long weapon_index;

			object_placement_data_new(&placement_data, initial_weapon->reference.index, unit_index);
			weapon_index = object_new(&placement_data);

			if (weapon_index != NONE)
			{
				struct weapon_datum *weapon = weapon_get(weapon_index);

				if (game_engine_running() && unit_has_weapon_definition_index(unit_index, weapon->definition_index))
				{
					object_delete(weapon_index);
				}
				else if (unit_add_weapon_to_inventory(unit_index, weapon_index, _unit_add_weapon_normally))
				{
				}
				else
				{
					object_delete(weapon_index);
				}
			}
		}
	}

	return;
}

long unit_get_current_equipment(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	return unit->unit.equipment_object_index;
}

boolean unit_add_grenade_to_inventory(
	long unit_index,
	long equipment_index)
{
	struct equipment_datum *equipment = equipment_get(equipment_index);
	struct equipment_definition *equipment_definition = equipment_definition_get(equipment->definition_index);
	struct unit_datum *unit = unit_get(unit_index);
	struct game_globals_grenade *grenade = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, equipment_definition->equipment.grenade_type, struct game_globals_grenade);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7282, equipment_definition->equipment.powerup_type==_equipment_powerup_grenade);

	if (grenade && unit->unit.grenade_counts[equipment_definition->equipment.grenade_type] < grenade->maximum_count)
	{
		long local_player_index;

		unit->unit.grenade_counts[equipment_definition->equipment.grenade_type]++;

		if (player_index_from_unit_index(unit_index) == NONE)
		{
			local_player_index = NONE;
		}
		else
		{
			local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
		}

		if (local_player_index != NONE)
		{
			equipment_handle_pickup(equipment_index);
		}

		object_delete(equipment_index);
		result = TRUE;
	}

	return result;
}

short unit_add_grenade_type_to_inventory(
	long unit_index,
	short grenade_type,
	short grenade_count)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7309, grenade_count>=0);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7310, (grenade_type >= 0) && (grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES));

	unit->unit.grenade_counts[grenade_type] += (char)grenade_count;
	unit->unit.desired_grenade_index = (char)grenade_type;
	unit->unit.current_grenade_index = (char)grenade_type;

	return unit->unit.grenade_counts[grenade_type];
}

boolean unit_add_equipment_to_inventory(
	long unit_index,
	long equipment_index,
	short mode)
{
	struct equipment_datum *equipment = equipment_get(equipment_index);
	struct equipment_definition *equipment_definition = equipment_definition_get(equipment->definition_index);
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7329, equipment_definition->equipment.powerup_type!=_equipment_powerup_none);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7330, equipment_definition->equipment.powerup_type!=_equipment_powerup_grenade);

	if (unit->unit.equipment_object_index != NONE && mode == _unit_add_weapon_as_current_weapon)
	{
		object_delete(unit->unit.equipment_object_index);
		unit->unit.equipment_object_index = NONE;
	}

	if (unit->unit.equipment_object_index == NONE)
	{
		long local_player_index;

		object_disconnect_from_map(equipment_index);
		object_set_visibility(equipment_index, FALSE);

		if (player_index_from_unit_index(unit_index) == NONE)
		{
			local_player_index = NONE;
		}
		else
		{
			local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
		}

		if (local_player_index != NONE)
		{
			equipment_handle_pickup(equipment_index);
		}

		item_in_unit_inventory(equipment_index, unit_index);
		unit->unit.equipment_object_index = equipment_index;
		result = TRUE;
	}

	return result;
}

void unit_drop_current_equipment(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.equipment_object_index != NONE)
	{
		unit_drop_item(unit_index, unit->unit.equipment_object_index);
		unit->unit.equipment_object_index = NONE;
	}

	return;
}

void unit_delete_current_equipment(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.equipment_object_index != NONE)
	{
		object_delete(unit->unit.equipment_object_index);
		unit->unit.equipment_object_index = NONE;
	}

	return;
}

void unit_delete_all_weapons(
	long unit_index)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		long weapon_index = unit->unit.weapon_object_indices[index];

		if (weapon_index != NONE && index != unit->unit.current_weapon_index)
		{
			object_delete(weapon_index);
			unit->unit.weapon_object_indices[index] = NONE;

			if (index == unit->unit.desired_weapon_index)
			{
				unit->unit.desired_weapon_index = NONE;
			}

			if (index == unit->unit.current_weapon_index)
			{
				unit->unit.current_weapon_index = NONE;
			}
		}
	}

	return;
}

boolean unit_add_weapon_to_inventory(
	long unit_index,
	long weapon_index,
	short mode)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (TEST_FLAG(weapon->object.flags, _object_connected_to_map_bit) &&
		weapon->object.parent_object_index == NONE &&
		unit_can_use_weapon(unit_index, weapon_index) &&
		game_engine_picking_up(unit_index, weapon_index))
	{
		short index;

		if (mode == _unit_add_weapon_as_only_weapon)
		{
			unit_delete_all_weapons(unit_index);
		}

		index = unit_first_free_weapon_index(unit_index);

		if (index != NONE)
		{
			object_disconnect_from_map(weapon_index);
			object_set_visibility(weapon_index, FALSE);
			item_in_unit_inventory(weapon_index, unit_index);
			unit->unit.weapon_object_indices[index] = weapon_index;
			unit->unit.weapon_last_used_at_game_time[index] = 0;

			switch (mode)
			{
			case _unit_add_weapon_normally:
				unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 0);
				break;
			case _unit_add_weapon_as_current_weapon:
				if (!TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit))
				{
					player_control_set_desired_weapon(unit_index, index);
				}
			case _unit_add_weapon_as_only_weapon:
				unit->unit.desired_weapon_index = index;
				break;
			}

			result = TRUE;
		}
	}

	return result;
}

boolean unit_has_weapon_definition_index(
	long unit_index,
	long weapon_definition_index)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		long weapon_index = unit->unit.weapon_object_indices[index];

		if (weapon_index != NONE && weapon_get(weapon_index)->definition_index == weapon_definition_index)
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

static short unit_first_free_weapon_index(
	long unit_index)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	short result = NONE;

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		long weapon_index = unit->unit.weapon_object_indices[index];

		if (weapon_index == NONE)
		{
			result = index;
			break;
		}
	}

	return result;
}

static void unit_adjust_for_seat_change(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 0);
	unit_ready_desired_weapon(unit_index, TRUE);

	return;
}

static void unit_ready_desired_weapon(
	long unit_index,
	boolean immediate)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long desired_weapon_index = unit_get_desired_weapon_index(unit_index);
	long current_weapon_index = unit_get_current_weapon_index(unit_index);

	if (current_weapon_index != NONE && weapon_put_away(current_weapon_index, immediate))
	{
		object_detach(current_weapon_index);
		object_disconnect_from_map(current_weapon_index);
		object_activate(current_weapon_index);
		object_set_visibility(current_weapon_index, FALSE);
		item_in_unit_inventory(current_weapon_index, unit_index);
		unit->unit.current_weapon_index = NONE;
	}

	if (unit->unit.current_weapon_index == NONE)
	{
		if (desired_weapon_index != NONE)
		{
			struct animation_graph *animation_graph;
			struct animation_graph_unit_seat *unit_seat;
			struct animation_graph_weapon_class *weapon_class;

			unit_set_or_test_seat_and_weapon_label(
			unit_index,
			unit_get_seat_label(unit_index),
			weapon_get_label(desired_weapon_index),
			TRUE);

			animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
			weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);

			object_reconnect_to_map(desired_weapon_index, NULL);
			object_set_visibility(desired_weapon_index, TRUE);
			object_attach_to_marker(unit_index, weapon_class->hand_marker_name, desired_weapon_index, weapon_class->grip_marker_name);
			unit->unit.current_weapon_index = unit->unit.desired_weapon_index;

			if (unit->unit.desired_weapon_index != NONE)
			{
				unit->unit.weapon_last_used_at_game_time[unit->unit.current_weapon_index] = game_time_get();
			}

			weapon_ready(desired_weapon_index);
		}
		else
		{
			unit_set_or_test_seat_and_weapon_label(
			unit_index,
			unit_get_seat_label(unit_index),
			base_weapon_label_get(_unit_base_weapon_none),
			TRUE);
			unit->unit.current_weapon_index = NONE;
		}
	}

	unit_unzoom(unit_index);

	return;
}

short unit_get_weapon_count(
	long unit_index)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	short count = 0;

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		if (unit->unit.weapon_object_indices[index] != NONE)
		{
			struct weapon_datum *weapon = weapon_get(unit->unit.weapon_object_indices[index]);
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

			if (!TEST_FLAG(weapon_definition->weapon.flags, _weapon_doesnt_count_toward_maximum_bit))
			{
				count++;
			}
		}
	}

	return count;
}

boolean unit_can_use_weapon(
	long unit_index,
	long weapon_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct weapon_datum *weapon = weapon_get(weapon_index);
	char const *seat_label = unit_get_seat_label(unit_index);
	char const *weapon_label = weapon_get_label(weapon_index);
	boolean result = unit_set_or_test_seat_and_weapon_label(unit_index, seat_label, weapon_label, FALSE);

	if (result)
	{
		result = game_engine_allow_pick_up(unit_index, weapon_index);
	}

	return result;
}

boolean unit_approve_weapon_pickup(
	long unit_index,
	long new_weapon_index)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	struct weapon_datum *new_weapon = weapon_get(new_weapon_index);
	struct weapon_definition *new_weapon_definition = weapon_definition_get(new_weapon->definition_index);
	boolean result = TRUE;

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		if (unit->unit.weapon_object_indices[index] != NONE)
		{
			struct weapon_datum *weapon = weapon_get(unit->unit.weapon_object_indices[index]);

			if (new_weapon->definition_index == weapon->definition_index)
			{
				result = FALSE;
			}
		}
	}

	return result;
}

boolean unit_approve_weapon_swap(
	long unit_index,
	long new_weapon_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct weapon_datum *new_weapon = weapon_get(new_weapon_index);
	struct weapon_definition *new_weapon_definition = weapon_definition_get(new_weapon->definition_index);
	long current_weapon_index = unit_get_current_weapon_index(unit_index);
	boolean result = TRUE;

	if (current_weapon_index != NONE)
	{
		short index;

		for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
		{
			if (unit->unit.weapon_object_indices[index] != NONE)
			{
				struct weapon_datum *weapon = weapon_get(unit->unit.weapon_object_indices[index]);

				if (new_weapon->definition_index == weapon->definition_index &&
					!(index == unit->unit.current_weapon_index && weapon->weapon.age > 0.f && new_weapon->weapon.age < weapon->weapon.age))
				{
					result = FALSE;
				}
			}
		}
	}
	else
	{
		result = FALSE;
	}

	return result;
}

static short unit_weapon_next_index(
	long unit_index,
	short current_index,
	short delta)
{
	short index;
	struct unit_datum *unit = unit_get(unit_index);
	short selected_index = NONE;

	if (current_index == NONE)
	{
		current_index = 0;
	}

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7744, current_index>=0 && current_index<MAXIMUM_WEAPONS_PER_UNIT);

	index = current_index;

	do
	{
		if (unit->unit.weapon_object_indices[index] != NONE && unit_can_use_weapon(unit_index, unit->unit.weapon_object_indices[index]))
		{
			if (delta || selected_index == NONE || unit->unit.weapon_last_used_at_game_time[selected_index] < unit->unit.weapon_last_used_at_game_time[index])
			{
				selected_index = index;
			}

			if (weapon_must_be_readied(unit->unit.weapon_object_indices[index]))
			{
				break;
			}

			if (index != current_index)
			{
				break;
			}
		}

		if (delta < 0)
		{
			index = index == 0 ? MAXIMUM_WEAPONS_PER_UNIT - 1 : index - 1;
		}
		else
		{
			index = index == MAXIMUM_WEAPONS_PER_UNIT - 1 ? 0 : index + 1;
		}
	}
	while (index != current_index);

	return selected_index;
}

boolean unit_drop_current_weapon(
	long unit_index,
	boolean immediate)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long current_weapon_index = unit_get_current_weapon_index(unit_index);
	short next_index = unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 1);

	boolean result = FALSE;

	if (current_weapon_index != NONE &&
		(next_index != unit->unit.current_weapon_index || immediate) &&
		!TEST_FLAG(object_get(current_weapon_index)->object.flags, _object_invisible_bit) &&
		weapon_put_away(current_weapon_index, immediate))
	{
		first_person_weapon_message_from_unit(unit_index, 13);
		unit_drop_item(unit_index, current_weapon_index);
		unit->unit.weapon_object_indices[unit->unit.current_weapon_index] = NONE;
		unit->unit.current_weapon_index = NONE;
		unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, NONE, 0);
		
		if (!weapon_can_be_fired(current_weapon_index))
		{
			object_delete(current_weapon_index);
		}

		result = TRUE;
	}

	return result;
}

static char const *unit_get_current_weapon_label(
	long unit_index)
{
	char const *result;
	long weapon_index = unit_get_current_weapon_index(unit_index);

	if (weapon_index == NONE)
	{
		result = base_weapon_label_get(_unit_base_weapon_none);
	}
	else
	{
		result = weapon_get_label(weapon_index);
	}

	return result;
}

short unit_get_grenade_count(
	long unit_index,
	short grenade_type)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7847, grenade_type==NONE || (grenade_type>=0 && grenade_type<NUMBER_OF_UNIT_GRENADE_TYPES));

	if (grenade_type != NONE)
	{
		return unit->unit.grenade_counts[grenade_type];
	}
	else
	{
		return 0;
	}
}

short unit_get_current_grenade_type(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7864, unit->unit.current_grenade_index==NONE || (unit->unit.current_grenade_index>=0 && unit->unit.current_grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES));

	return unit->unit.current_grenade_index;
}

boolean unit_throw_grenade_begin(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long weapon_index = unit_get_current_weapon_index(unit_index);
	boolean result = FALSE;

	if (unit_get_grenade_count(unit_index, unit_get_current_grenade_type(unit_index)) > 0 &&
		!unit_animation_busy(&unit->unit.animation) &&
		!weapon_prevents_grenade_throwing(weapon_index))
	{
		if (weapon_index != NONE)
		{
			weapon_stop_reload(weapon_index);
		}

		biped_stop_melee_attack(unit_index);
		unit->unit.animation.action = _unit_action_none;
		unit->unit.animation.action_animation.index = NONE;

		if (unit_animation_set_state(unit_index, _unit_state_throw_grenade))
		{
			struct animation_graph *animation_graph;
			struct animation *animation;
			struct game_globals_grenade *grenade;

			unit->unit.grenade_throw_state = _unit_grenade_throw_wind_up;
			unit->unit.grenade_throw_ticks = 0;
			animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);
			unit->unit.grenade_throw_full_power_ticks = animation_key_frame_index(animation) - unit->object.animation.state.frame_index + 1;

			if (alignment_vector)
			{
				unit_align_facing(unit_index, alignment_vector);
			}
			else
			{
				real_vector2d aiming_alignment;

				aiming_alignment.i = unit->unit.aiming_vector.i;
				aiming_alignment.j = unit->unit.aiming_vector.j;

				if (normalize2d(&aiming_alignment) > 0.f)
				{
					unit_align_facing(unit_index, &aiming_alignment);
				}
			}

			first_person_weapon_message_from_unit(unit_index, 17);
			player_control_unzoom(unit_index);
			grenade = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, unit->unit.current_grenade_index, struct game_globals_grenade);

			if (grenade->throwing_effect.index != NONE)
			{
				effect_new_from_object(grenade->throwing_effect.index, unit_index, unit_index, NONE, 0.f, 0.f, NULL, NULL);
			}

			result = TRUE;
		}
	}

	return result;
}

static void unit_throw_grenade_move_to_hand(
	long unit_index)
{
	long object_index;

	struct object_placement_data placement_data;
	struct object_marker marker;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct game_globals_grenade *grenades = TAG_BLOCK_GET_ELEMENT(
		&scenario_get_game_globals()->grenades,
		unit->unit.current_grenade_index,
		struct game_globals_grenade);

	if ((
		unit->unit.player_index == NONE ||
		!cheat.infinite_ammo && !game_engine_infinite_grenades(unit->unit.player_index)) &&
		(unit->unit.actor_index == NONE ||
		!actor_has_unlimited_grenades(unit->unit.actor_index)))
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 7966, unit->unit.current_grenade_index>=0 && unit->unit.current_grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES);
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 7967, unit->unit.grenade_counts[unit->unit.current_grenade_index]>0);
		--unit->unit.grenade_counts[unit->unit.current_grenade_index];
	}

	object_get_marker_by_name(unit_index, "left hand", &marker, 1);
	object_placement_data_new(&placement_data, grenades->projectile.index, unit_index);
	SET_FLAG(placement_data.flags, 1, TRUE);
	unit_get_aiming_vector(unit_index, &placement_data.forward);
	normalize3d(perpendicular3d(&placement_data.forward, &placement_data.up));

	placement_data.position = marker.matrix.position;
	object_index = object_new(&placement_data);

	if (object_index != NONE)
	{
		object_attach_to_node(unit_index, object_index, marker.node_index);
		unit->unit.grenade_object_index = object_index;
		unit->unit.grenade_throw_state = _unit_grenade_throw_in_hand;
	}
	else
	{
		unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
	}

	return;
}

static void unit_throw_grenade_release(
	long unit_index,
	boolean premature)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->unit.grenade_throw_state == _unit_grenade_throw_in_hand)
	{
		if (unit->unit.grenade_object_index != NONE)
		{
			real_vector3d initial_velocity;

			long grenade_index = unit->unit.grenade_object_index;

			object_detach(grenade_index);

			if (unit->unit.actor_index != NONE)
			{
				real_point3d origin;

				object_get_origin(unit->unit.grenade_object_index, &origin);
				actor_aim_grenade(unit->unit.actor_index, &origin, &initial_velocity);
			}
			else
			{
				if (unit->unit.player_index != NONE)
				{
					real_point3d new_grenade_origin;
					real_vector3d forward;
					real_vector3d up;
					real_vector3d left;

					struct game_globals_player_information *player_information = TAG_BLOCK_GET_ELEMENT(
						&scenario_get_game_globals()->player_information,
						0,
						struct game_globals_player_information);

					forward = unit->unit.aiming_vector;

					if (normalize3d(cross_product3d(global_up3d, &forward, &left)) == 0.f)
					{
						left = *global_up3d;
					}

					normalize3d(cross_product3d(&forward, &left, &up));

					unit_get_camera_position(unit_index, &new_grenade_origin);
					point_from_line3d(&new_grenade_origin, &forward, (player_information->grenade_origin.i), &new_grenade_origin);
					point_from_line3d(&new_grenade_origin, &left, (player_information->grenade_origin.j), &new_grenade_origin);
					point_from_line3d(&new_grenade_origin, &up, (player_information->grenade_origin.k), &new_grenade_origin);

					object_translate(grenade_index, &new_grenade_origin, NULL);
				}

				{
					real scale = unit_definition->unit.grenade_velocity/TICKS_PER_SECOND;

					scale_vector3d(&unit->unit.aiming_vector, scale, &initial_velocity);
				}
			}

			if (premature)
			{
				real power = (real)unit->unit.grenade_throw_ticks/(real)unit->unit.grenade_throw_full_power_ticks;

				if (power < 1.f)
				{
					real_vector3d fumble_velocity;

					real range = real_random_range(0.020000001f, 0.046666667f);

					scale_vector3d(&unit->unit.aiming_vector, range, &fumble_velocity);
					scale_vector3d(&initial_velocity, power, &initial_velocity);
					point_from_line3d((real_point3d *)&initial_velocity, &fumble_velocity, 1.f-power, (real_point3d *)&initial_velocity);
				}
			}

			{
				struct object_datum *grenade = object_get(grenade_index);

				subtract_vectors3d(&initial_velocity, &grenade->object.translational_velocity, &initial_velocity);
				projectile_accelerate(grenade_index, &initial_velocity);
			}

			unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
			unit->unit.grenade_object_index = NONE;

			{
				real_point3d camera_position;

				unit_get_camera_position(unit_index, &camera_position);

				if (!object_force_inside_bsp(grenade_index, &camera_position))
				{
					object_delete(grenade_index);
				}
			}
		}
		else
		{
			unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
		}
	}

	return;
}

void scripting_magic_melee_attack(
	void)
{
	unit_melee_attack_begin(player_get(0)->unit_index, FALSE, NULL);

	return;
}

static void unit_seat_update(
	long object_index)
{
	real_vector3d *acceleration_scale;
	struct object_marker seat_marker;
	real_vector3d left;
	real_vector3d new_velocity;

	struct unit_datum *unit = unit_get(object_index);
	boolean found = FALSE;

	if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
	{
		struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
		struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

		if (object_get_marker_by_name(unit->object.parent_object_index, seat->marker_name, &seat_marker, 1))
		{
			object_get_origin(unit->object.parent_object_index, &seat_marker.matrix.position);
			acceleration_scale = &seat->acceleration_scale;
			found = TRUE;
		}
	}
	else
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

		seat_marker.matrix.position = unit->object.position;
		seat_marker.matrix.forward = unit->object.forward;
		seat_marker.matrix.up = unit->object.up;
		acceleration_scale = &unit_definition->unit.acceleration_scale;
		found = TRUE;
	}

	if (found)
	{
		real_vector3d acceleration;

		new_velocity.i = seat_marker.matrix.position.x-unit->unit.seat_last_position.x;
		new_velocity.j = seat_marker.matrix.position.y-unit->unit.seat_last_position.y;
		new_velocity.k = seat_marker.matrix.position.z-unit->unit.seat_last_position.z;

		acceleration.i = new_velocity.i - unit->unit.seat_last_velocity.i;
		acceleration.j = new_velocity.j - unit->unit.seat_last_velocity.j;
		acceleration.k = new_velocity.k - unit->unit.seat_last_velocity.k;

		cross_product3d(&seat_marker.matrix.up, &seat_marker.matrix.forward, &left);

		unit->unit.seat_desired_acceleration.i = (dot_product3d(&acceleration, &seat_marker.matrix.forward) * acceleration_scale->i) + 0.5f;
		unit->unit.seat_desired_acceleration.j = (dot_product3d(&acceleration, &left) * acceleration_scale->j) + 0.5f;
		unit->unit.seat_desired_acceleration.k = (dot_product3d(&acceleration, &seat_marker.matrix.up) * acceleration_scale->k) + 0.5f;

		unit->unit.seat_desired_acceleration.i = PIN(unit->unit.seat_desired_acceleration.i, 0.f, 1.f);
		unit->unit.seat_desired_acceleration.j = PIN(unit->unit.seat_desired_acceleration.j, 0.f, 1.f);
		unit->unit.seat_desired_acceleration.k = PIN(unit->unit.seat_desired_acceleration.k, 0.f, 1.f);

		unit->unit.seat_last_position = seat_marker.matrix.position;
		unit->unit.seat_last_velocity = new_velocity;
	}
	else
	{
		unit->unit.seat_desired_acceleration.k = 0.5f;
		unit->unit.seat_desired_acceleration.j = 0.5f;
		unit->unit.seat_desired_acceleration.i = 0.5f;
	}

	return;
}

static char const *base_seat_label_get(
	short base_seat_index)
{
	match_assert("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(8207, 8214), base_seat_index>=0 && base_seat_index<NUMBER_OF_UNIT_BASE_SEATS);
	return base_seat_labels[base_seat_index];
}

void scripting_set_magic_base_seat(
	char const *string)
{
	magic_base_animation_seat_index = seat_label_to_base_seat_index(string);

	return;
}

void unit_scripting_set_seat(
	long unit_index,
	char const *string)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		unit->unit.magic_seat_index = seat_label_to_base_seat_index(string);
	}

	return;
}

static short seat_label_to_base_seat_index(
	char const *seat_label)
{
	short seat_index;
	short result = NONE;

	for (seat_index = 0; seat_index < NUMBER_OF_UNIT_BASE_SEATS; ++seat_index)
	{
		if (!_stricmp(seat_label, base_seat_labels[seat_index]))
		{
			result = seat_index;
			break;
		}
	}

	return result;
}

static char const *base_weapon_label_get(
	short base_weapon_index)
{
	char const *base_weapon_labels[NUMBER_OF_UNIT_BASE_WEAPONS] = {"unarmed"};

	match_assert("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(8259, 8266), base_weapon_index>=0 && base_weapon_index<NUMBER_OF_UNIT_BASE_WEAPONS);

	return base_weapon_labels[base_weapon_index];
}

static void unit_set_animation(
	long unit_index,
	long animation_graph_index,
	short animation_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	unit->object.animation.animation_graph_index = animation_graph_index;
	unit->object.animation.state.index = animation_index;
	unit->object.animation.state.frame_index = 0;

#ifdef DEBUG
	if (debug_unit_all_animations)
	{
		char const *animation_name = "<none>";
		boolean print = TRUE;

		if (animation_graph_index != NONE)
		{
			struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);

			if (animation_index != NONE)
			{
				animation_name = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation)->name;
			}
		}

		if (ai_debug.selected_actor_index != NONE &&
			unit->unit.actor_index != ai_debug.selected_actor_index &&
			unit->unit.swarm_actor_index != ai_debug.selected_actor_index)
		{
			print = FALSE;
		}

		if (print)
		{
			console_printf(FALSE, "%s: animation %s", tag_name_strip_path(tag_get_name(unit->definition_index)), animation_name);
		}
	}
#endif

	return;
}

static short unit_animation_update(
	long unit_index,
	long animation_graph_index,
	struct animation_state *state)
{
	long sound_definition_index;

	short result = animation_update(animation_graph_index, state, &sound_definition_index);

	if (sound_definition_index != NONE)
	{
		object_impulse_sound_new(unit_index, sound_definition_index, 0, global_origin3d, global_forward3d, 1.f);
	}

	return result;
}

static void unit_refresh_illumination(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *parent_unit = unit_try_and_get(unit->object.parent_object_index);

	if (parent_unit == NULL)
	{
		real_rgb_color illumination_color;

		lights_illumination_at_point(&unit->object.position, &unit->object.location, &illumination_color);
		unit->unit.ambient_illumination = real_rgb_color_brightness(&illumination_color);
		unit->unit.self_illumination = object_get_self_illumination(unit_index);
	}
	else
	{
		unit->unit.ambient_illumination = parent_unit->unit.ambient_illumination;
		unit->unit.self_illumination = parent_unit->unit.self_illumination;
	}

	return;
}

static long unit_get_weapon(
	struct unit_datum *unit,
	short index)
{
	long result = NONE;

	if (index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(8364, 8371), index>=0 && index<MAXIMUM_WEAPONS_PER_UNIT);
		result = unit->unit.weapon_object_indices[index];
	}

	return result;
}

static void unit_drop_item(
	long unit_index,
	long item_index)
{
	real_vector3d item_velocity;
	real speed;

	struct unit_datum *unit = unit_get(unit_index);
	struct item_datum *item = item_get(item_index);

	if (item->object.parent_object_index == NONE)
	{
		object_reconnect_to_map(item_index, NULL);
		object_set_visibility(item_index, TRUE);
		object_attach_to_marker(unit_index, "left hand", item_index, "");
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(8389, 8396), item->object.parent_object_index==unit_index);
	}

	item_in_unit_inventory(item_index, NONE);
	object_detach(item_index);

	item->object.translational_velocity = *global_zero_vector3d;
	item->object.angular_velocity = *global_zero_vector3d;

	random_vector_in_cone3d(&unit->unit.aiming_vector, 0.f, DEGREES_TO_RADIANS(22.5f), &item_velocity);
	speed = real_random_range(0.8f / TICKS_PER_SECOND, 1.2f / TICKS_PER_SECOND);
	scale_vector3d(&item_velocity, speed, &item_velocity);

	{
		real_vector3d unit_velocity;

		object_get_velocities(unit_index, &unit_velocity, NULL);
		item_velocity.i = item_velocity.i + unit_velocity.i;
		item->item.ignore_object_index = unit_index;
		item_velocity.j = item_velocity.j + unit_velocity.j;
		item_velocity.k = item_velocity.k + unit_velocity.k;
	}

	item_accelerate(item_index, &item_velocity, FALSE);

	{
		real_point3d camera_position;

		unit_get_camera_position(unit_index, &camera_position);

		if (object_force_inside_bsp(item_index, &camera_position))
		{
		}
		else if (!game_engine_running())
		{
			object_delete(item_index);
		}
	}

	if (TEST_FLAG(unit->unit.flags, _unit_doesnt_drop_items_bit))
	{
		object_delete(item_index);
	}

	return;
}

static void unit_drop_grenades(
	long unit_index)
{
	short grenade_type;

	struct unit_datum *unit = unit_get(unit_index);

	for (grenade_type = 0; grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_type++)
	{
		struct game_globals_grenade *grenade = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, grenade_type, struct game_globals_grenade);

		while (unit->unit.grenade_counts[grenade_type] > 0)
		{
			struct object_placement_data placement_data;
			long grenade_index;

			object_placement_data_new(&placement_data, grenade->item.index, unit_index);
			grenade_index = object_new(&placement_data);

			if (grenade_index != NONE)
			{
				object_disconnect_from_map(grenade_index);
				unit_drop_item(unit_index, grenade_index);
			}

			unit->unit.grenade_counts[grenade_type]--;
		}
	}

	return;
}

static void unit_drop_inventory_weapons(
	long unit_index)
{
	short index;

	struct unit_datum *unit = unit_get(unit_index);

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		long weapon_index = unit->unit.weapon_object_indices[index];

		if (weapon_index != NONE && index != unit->unit.current_weapon_index)
		{
			struct weapon_datum *weapon = weapon_get(weapon_index);

			match_vassert("c:\\halo\\SOURCE\\units\\units.c", 8505, !TEST_FLAG(weapon->object.flags, _object_connected_to_map_bit),
				csprintf(temporary, "a %s tried to drop a %s which was connected to the map.", tag_get_name(unit->definition_index), tag_get_name(weapon->definition_index)));

			unit_drop_item(unit_index, weapon_index);

			if (index == unit->unit.desired_weapon_index)
			{
				unit->unit.desired_weapon_index = unit->unit.current_weapon_index;
			}

			unit->unit.weapon_object_indices[index] = NONE;

			if (!weapon_can_be_fired(weapon_index))
			{
				object_delete(weapon_index);
			}
		}
	}

	return;
}

void unit_handle_deleted_object(
	long unit_index,
	long deleted_object_index)
{
	short index;

	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.grenade_object_index == deleted_object_index)
	{
		unit->unit.grenade_object_index = NONE;
	}

	if (unit->unit.driver_object_index == deleted_object_index)
	{
		unit->unit.driver_object_index = NONE;
	}

	if (unit->unit.gunner_object_index == deleted_object_index)
	{
		unit->unit.gunner_object_index = NONE;
	}

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		if (unit->unit.weapon_object_indices[index] == deleted_object_index)
		{
			unit->unit.weapon_object_indices[index] = NONE;

			if (index == unit->unit.desired_weapon_index)
			{
				unit->unit.desired_weapon_index = NONE;
			}

			if (index == unit->unit.current_weapon_index)
			{
				unit->unit.current_weapon_index = NONE;
			}
		}
	}

	if (unit->unit.current_weapon_index == NONE)
	{
		unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 0);
	}

	if (unit->unit.equipment_object_index == deleted_object_index)
	{
		unit->unit.equipment_object_index = NONE;
	}

	if (unit->unit.delayed_damage_attacker_object_index == deleted_object_index)
	{
		unit->unit.delayed_damage_attacker_object_index = NONE;
	}

	return;
}

void unit_handle_region_destroyed(
	long unit_index,
	short region_index,
	unsigned long region_damage_flags)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		unit_scream(unit_index, TEST_FLAG(region_damage_flags, _object_region_head_destroyed_scream_bit) ? _unit_scream_destroyed_head : _unit_scream_destroyed_limb);
	}

	return;
}

static void unit_melee_sound(
	long unit_index,
	long damage_definition_index,
	short material_type)
{
	struct material_definition *material = scenario_material_definition_get(material_type);

	if (material->melee_hit_sound.index != NONE)
	{
		object_impulse_sound_new(unit_index, material->melee_hit_sound.index, NONE, global_origin3d, global_forward3d, 1.f);
	}

	if (damage_definition_index != NONE)
	{
		struct damage_effect_definition *damage_definition = damage_effect_definition_get(damage_definition_index);

		if (damage_definition->sound.index != NONE)
		{
			object_impulse_sound_new(unit_index, damage_definition->sound.index, NONE, global_origin3d, global_forward3d, 1.f);
		}
	}

	return;
}

void unit_cause_melee_damage(
	long unit_index,
	boolean silent,
	long target_object_index,
	short target_node_index,
	short target_region_index,
	short target_material_index,
	real_vector3d const *target_object_normal)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit_definition->unit.melee_damage.index != NONE)
	{
		real_point3d melee_origin;
		long damage_definition_index;
		long weapon_index;

		{
			struct object_marker marker;

			if (object_get_marker_by_name(unit_index, "melee", &marker, 1) == 1)
			{
				struct collision_result collision;

				melee_origin = marker.matrix.position;

				match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\units.c", 8640, _collision_user_bipeds);

				if (collision_test_line(_collision_test_for_projectiles_flags, &unit->object.bounding_sphere_center, &melee_origin, NONE, &collision))
				{
					melee_origin = unit->object.bounding_sphere_center;
				}

				match_collision_log_end_user("c:\\halo\\SOURCE\\units\\units.c", 8648);
			}
			else
			{
				melee_origin = unit->object.bounding_sphere_center;
			}
		}

		damage_definition_index = unit_definition->unit.melee_damage.index;
		weapon_index = unit_get_current_weapon_index(unit_index);

		if (weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(weapon_index);
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

			if (TEST_FLAG(weapon_definition->weapon.flags, _weapon_non_players_use_melee_damage_bit))
			{
				damage_definition_index = weapon_definition->weapon.melee_attack_damage.index;
			}
		}

		{
			struct damage_data damage_data;

			damage_data_new(&damage_data, damage_definition_index);
			damage_data.location = unit->object.location;
			damage_data.owner_object_index = unit_index;
			damage_data.owner_team_index = unit->object.owner_team_index;
			damage_data.owner_player_index = unit->unit.player_index;
			damage_data.origin = melee_origin;
			damage_data.epicenter = unit->object.bounding_sphere_center;

			if (target_object_index == NONE)
			{
				area_of_effect_cause_damage(&damage_data, NONE);
			}
			else
			{
				object_cause_damage(
					&damage_data,
					target_object_index,
					target_node_index,
					target_region_index,
					target_material_index,
					target_object_normal);
			}

			if (!silent && damage_data.material_type != NONE)
			{
				unit_melee_sound(unit_index, damage_definition_index, damage_data.material_type);
			}
		}
	}

	unit->unit.melee_attack_state = 0;

	return;
}

void unit_cause_player_melee_damage(
	long unit_index)
{
	short target_object_type;
	real target_t;
	real_point3d head_position;
	long target_surface_index;
	real_vector3d x_axis;
	real_vector3d y_axis;
	short i;
	short j;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long target_object_index = NONE;
	short material_type = NONE;
	short breakable_surface_index = NONE;
	long damage_definition_index = NONE;
	long response_definition_index = NONE;

	unit_get_head_position(unit_index, &head_position);

	match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\units.c", 8729, _collision_user_melee);

	normalize3d(perpendicular3d(&unit->unit.aiming_vector, &x_axis));
	cross_product3d(&unit->unit.aiming_vector, &x_axis, &y_axis);

	for (i = -2; i <= 2; i++)
	{
		for (j = -2; j <= 2; j++)
		{
			struct collision_result collision;
			real_vector3d vector;

			vector.i = (i*x_axis.i + j*y_axis.i)*0.1f + unit->unit.aiming_vector.i*0.8f;
			vector.j = (i*x_axis.j + j*y_axis.j)*0.1f + unit->unit.aiming_vector.j*0.8f;
			vector.k = (i*x_axis.k + j*y_axis.k)*0.1f + unit->unit.aiming_vector.k*0.8f;

			if (collision_test_vector(_collision_test_for_projectiles_flags, &head_position, &vector, unit_index, &collision))
			{
				switch (collision.type)
				{
				case _collision_result_structure:
					if (target_object_index == NONE)
					{
						material_type = collision.material_type;

						if (TEST_FLAG(collision.flags, _collision_surface_breakable_bit))
						{
							breakable_surface_index = collision.breakable_surface_index;
							target_surface_index = collision.surface_index;
						}
					}
					break;

				case _collision_result_object:
				{
					long object_index = collision.object_index;
					struct object_datum *object = object_get(object_index);

					if (object->object.type != _object_type_weapon && object->object.parent_object_index != NONE)
					{
						object_index = object->object.parent_object_index;
						object = object_get(object_index);
					}

					if (target_object_index == NONE ||
						(object->object.type == _object_type_biped && !target_object_type && target_t > collision.t) ||
						(object->object.type == _object_type_biped && target_object_type))
					{
						target_object_index = object_index;
						target_object_type = object->object.type;
						material_type = collision.material_type;
						target_t = collision.t;
					}
					break;
				}
				}
			}
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\units\\units.c", 8797);

	{
		long weapon_index = unit_get_current_weapon_index(unit_index);

		if (weapon_index != NONE)
		{
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon_get(weapon_index)->definition_index);

			damage_definition_index = weapon_definition->weapon.melee_attack_damage.index;
			response_definition_index = weapon_definition->weapon.melee_attack_response.index;
		}
	}

	if (damage_definition_index == NONE)
	{
		damage_definition_index = unit_definition->unit.melee_damage.index;
	}

	if (target_object_index != NONE)
	{
		struct object_datum *target_object = object_get(target_object_index);

		if (target_object->object.type == _object_type_vehicle)
		{
			real_vector3d acceleration;

			struct object_definition *target_object_definition = object_definition_get(target_object->definition_index);

			scale_vector3d(&unit->unit.aiming_vector, target_object_definition->object.acceleration_scale*0.035f, &acceleration);
			vehicle_accelerate(target_object_index, &acceleration);
		}
	}

	if (damage_definition_index != NONE)
	{
		struct damage_data damage_data;

		damage_data_new(&damage_data, damage_definition_index);
		SET_FLAG(damage_data.flags, _damage_area_of_effect_bit, TRUE);
		damage_data.location = unit->object.location;
		damage_data.owner_object_index = unit_index;
		damage_data.owner_team_index = unit->object.owner_team_index;
		damage_data.owner_player_index = unit->unit.player_index;
		damage_data.origin = head_position;
		damage_data.epicenter = unit->object.bounding_sphere_center;
		damage_data.direction = unit->unit.aiming_vector;
		damage_data.material_type = material_type;

		if (target_object_index == NONE)
		{
			if (breakable_surface_index != NONE)
			{
				breakable_surface_damage(breakable_surface_index, &damage_data, target_surface_index);
			}
		}
		else
		{
			struct game_globals_player_information *player_information;
			struct object_datum *target_object = object_get(target_object_index);

			if (target_object->object.type == _object_type_machine)
			{
				machine_try_to_open_with_damage(target_object_index);
			}

			player_information = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information);

			if (player_information->run_forward_speed > 0.f)
			{
				real forward_speed = dot_product3d(&unit->object.forward, &unit->object.translational_velocity);

				damage_data.scale = (forward_speed*30.f)/player_information->run_forward_speed;
				damage_data.scale = PIN(damage_data.scale, 0.f, 1.f);
			}

			if (unit->object.type == _object_type_biped)
			{
				struct biped_datum *biped = biped_get(unit_index);

				if (biped->biped.airborne_ticks > 15)
				{
					damage_data.scale = 1.5f;
				}
			}

			if (object_get(target_object_index)->object.type == _object_type_biped)
			{
				object_cause_damage(&damage_data, target_object_index, NONE, NONE, NONE, NULL);
			}
		}
	}

	if (material_type != NONE)
	{
		unit_melee_sound(unit_index, damage_definition_index, material_type);

		if (response_definition_index != NONE)
		{
			struct damage_data damage_data;

			damage_data_new(&damage_data, response_definition_index);
			SET_FLAG(damage_data.flags, _damage_from_weapon_bit, TRUE);
			negate_vector3d(&unit->unit.aiming_vector, &damage_data.direction);
			damage_data.epicenter = unit->object.bounding_sphere_center;
			damage_data.origin = damage_data.epicenter;
			object_cause_damage(&damage_data, unit_index, NONE, NONE, NONE, NULL);
		}
	}

	unit->unit.melee_attack_state = 0;

	return;
}

static void unit_cause_continuous_melee_damage(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->unit.melee_attack_state == 4 &&
		unit->object.parent_object_index != NONE &&
		unit_definition->unit.melee_damage.index != NONE)
	{
		struct collision_model_test_vector_result vector_result;
		struct damage_data damage_data;
		real_point3d collision_point;
		real_plane3d collision_plane;
		boolean collision_passed = FALSE;

		if (unit->unit.melee_continuous_damage_effect_timer == 0)
		{
			struct collision_model_instance instance;
			real_point3d collision_origin;
			real_vector3d collision_vector;

			match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(8934, 8941), _collision_user_melee);

			if (collision_model_instance_new(&instance, unit->object.parent_object_index))
			{
				object_get_origin(unit_index, &collision_origin);
				scale_vector3d(&unit->object.forward, 0.2f, &collision_vector);
				point_from_line3d(&collision_origin, &collision_vector, -0.5f, &collision_origin);

				if (collision_model_test_vector(
					&instance,
					FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit),
					&collision_origin,
					&collision_vector,
					&vector_result))
				{
					point_from_line3d(&collision_origin, &collision_vector, vector_result.bsp_result.t, &collision_point);
					matrix4x3_transform_plane(&instance.matrices[vector_result.node_index], vector_result.bsp_result.plane, &collision_plane);

					if (vector_result.bsp_result.plane_designator & LONG_MIN)
					{
						plane3d_negate(&collision_plane, &collision_plane);
					}

					collision_passed = TRUE;
				}
			}

			match_collision_log_end_user("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(8958, 8965));
		}

		damage_data_new(&damage_data, unit_definition->unit.melee_damage.index);
		damage_data.owner_object_index = unit_index;
		damage_data.owner_team_index = unit->object.owner_team_index;
		damage_data.owner_player_index = unit->unit.player_index;
		damage_data.scale = 1.f/30.f;

		if (collision_passed)
		{
			damage_data.epicenter = collision_point;
			damage_data.origin = damage_data.epicenter;
			damage_data.direction = unit->object.forward;
			SET_FLAG(damage_data.flags, _damage_create_localized_effect_bit, TRUE);
			unit->unit.melee_continuous_damage_effect_timer = 10;

			object_cause_damage(
				&damage_data,
				unit->object.parent_object_index,
				vector_result.node_index,
				vector_result.region_index,
				vector_result.bsp_result.material_index,
				&collision_plane.n);
		}
		else
		{
			object_cause_damage(
				&damage_data,
				unit->object.parent_object_index,
				NONE,
				NONE,
				NONE,
				NULL);
		}

		--unit->unit.melee_continuous_damage_effect_timer;
	}

	return;
}

void unit_impact_melee_damage(
	long unit_index,
	long impact_object_index,
	short impact_node_index,
	short impact_region_index,
	short impact_material_index,
	real_point3d const *impact_point,
	real_vector3d const *impact_normal,
	struct location const *impact_location)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct object_datum *impact_object = object_get(impact_object_index);

	if (TEST_FLAG(unit_definition->unit.flags, _unit_impact_melee_die_on_shield_bit) &&
		impact_object->object.type == _object_type_biped &&
		impact_object->object.shield_vitality > 0.f &&
		TEST_FLAG(unit_definition_get(impact_object->definition_index)->unit.flags, _unit_shields_fry_infection_forms_bit))
	{
		unit_cause_melee_damage(
		unit_index,
		TRUE,
		impact_object_index,
		impact_node_index,
		impact_region_index,
		impact_material_index,
		impact_normal);
		object_deplete_body(unit_index);
		object_delete(unit_index);
	}
	else if (TEST_FLAG(unit_definition->unit.flags, _unit_impact_melee_attaches_bit))
	{
		if (TEST_FLAG(_object_mask_unit, impact_object->object.type) && !TEST_FLAG(impact_object->object.damage_flags, _object_dead_bit))
		{
			long parent_object_index = impact_object->object.parent_object_index;
			boolean attach = TRUE;

			while (parent_object_index != NONE)
			{
				struct object_datum *parent_object = object_get(parent_object_index);

				if (parent_object_index == unit_index)
				{
					attach = FALSE;
					break;
				}

				if (parent_object->object.type != _object_type_vehicle)
				{
					attach = FALSE;
					break;
				}

				parent_object_index = parent_object->object.parent_object_index;
			}

			if (attach)
			{
				real_vector3d left_vector;

				unit->object.translational_velocity = *global_zero_vector3d;
				unit->object.angular_velocity = *global_zero_vector3d;
				unit->object.forward = *impact_normal;
				negate_vector3d(&unit->object.forward, &unit->object.forward);

				cross_product3d(&unit->object.up, &unit->object.forward, &left_vector);

				if (normalize3d(&left_vector) == 0.f)
				{
					cross_product3d(global_up3d, &unit->object.forward, &left_vector);

					if (normalize3d(&left_vector) == 0.f)
					{
						left_vector = *global_forward3d;
					}
				}

				cross_product3d(&unit->object.forward, &left_vector, &unit->object.up);
				object_translate(unit_index, impact_point, impact_location);
				object_attach_to_node(impact_object_index, unit_index, impact_node_index);
				SET_FLAG(unit->object.flags, _object_at_rest_bit, TRUE);
				SET_FLAG(unit->unit.flags, _unit_attached_melee_attack_bit, TRUE);
				unit_melee_attack_begin(unit_index, TRUE, NULL);
			}
		}
	}

	return;
}

void unit_set_enterable_by_player(
	long unit_index,
	boolean enterable_by_player)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		SET_FLAG(unit->unit.flags, _unit_not_enterable_by_player_bit, !enterable_by_player);
	}

	return;
}

void unit_set_emotion(
	long unit_index,
	short emotion_index)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		unit->unit.animation.emotion_index = (char)emotion_index;
		object_start_interpolation(unit_index, 6);
	}

	return;
}

void unit_aim_without_turning(
	long unit_index,
	boolean aim_without_turning)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		SET_FLAG(unit->unit.flags, _unit_aim_without_turning_bit, aim_without_turning);
	}

	return;
}

short unit_get_custom_animation_time(
	long unit_index)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->unit.animation.state == _unit_state_user_animation)
		{
			struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
			struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

			return MAX(animation->frame_count - unit->object.animation.state.frame_index - 2, 0);
		}
	}

	return 0;
}

void unit_stop_custom_animation(
	long unit_index)
{
	if (unit_index != NONE && unit_get(unit_index)->unit.animation.state == _unit_state_user_animation)
	{
		unit_animation_set_state(unit_index, _unit_state_idle);
	}

	return;
}

boolean unit_is_playing_custom_animation(
	long unit_index)
{
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		return unit->unit.animation.state == _unit_state_user_animation;
	}

	return FALSE;
}

boolean unit_custom_animation_at_frame(
	long unit_index,
	long animation_graph_index,
	char const *animation_name,
	boolean interpolate,
	short frame_index)
{
	boolean success = FALSE;

	if (unit_start_user_animation(unit_index, animation_graph_index, animation_name, interpolate))
	{
		struct unit_datum *unit = unit_get(unit_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

		if (frame_index >= 0 && frame_index < animation->frame_count)
		{
			unit->object.animation.state.frame_index = frame_index;
			success = TRUE;
		}
	}

	return success;
}

boolean unit_start_user_animation(
	long unit_index,
	long animation_graph_index,
	char const *animation_name,
	boolean interpolate)
{
	boolean success = FALSE;

	if (unit_index != NONE && animation_graph_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);
		short animation_index = animation_graph_get_animation_by_name(animation_graph_index, animation_name);

		if (animation_index != NONE)
		{
			struct animation *animation;

			animation_index = animation_choose_random_permutation(animation_graph_index, animation_index);
			animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

			if (animation->type != _animation_overlay && animation->type == _animation_base)
			{
				boolean start = TRUE;

				if (unit->unit.animation.state == _unit_state_user_animation && unit->object.animation.state.index != NONE)
				{
					struct animation *current_animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

					if (current_animation->runtime_parent_animation_index == animation->runtime_parent_animation_index)
					{
						if (unit->object.animation.state.frame_index + 2 == animation_key_frame_index(current_animation))
						{
							unit->object.animation.state.frame_index--;
							start = FALSE;
						}
						else if (unit->object.animation.state.frame_index < animation_key_frame_index(current_animation))
						{
							start = FALSE;
						}
					}
				}

				if (start)
				{
					if (interpolate)
					{
						object_start_interpolation(unit_index, 6);
					}

					unit->unit.animation.state = _unit_state_user_animation;
					unit_set_animation(unit_index, animation_graph_index, animation_index);
					SET_FLAG(unit->unit.animation.flags, _unit_animation_postpone_weapon_ik_until_interpolation_ends_bit, TRUE);
					object_compute_node_matrices_recursive(unit_index);
					success = TRUE;
				}
			}
		}
		else
		{
			console_warning("the animation '%s' doesn't exist in the graph '%s'", animation_name, tag_get_name(animation_graph_index));
		}
	}

	return success;
}

void unit_set_mouth_aperture(
	long unit_index,
	real aperture)
{
	struct unit_datum *unit = unit_get(unit_index);
	real *mouth_aperture = &unit->unit.mouth_aperture;
	real delta = aperture - *mouth_aperture;

	*mouth_aperture += PIN(delta, -0.3f, 0.3f);

	return;
}

short unit_inventory_get_must_be_readied_weapon(
	long unit_index)
{
	short index;
	short result = NONE;
	struct unit_datum *unit = unit_get(unit_index);

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		long weapon_index = unit->unit.weapon_object_indices[index];

		if (weapon_index != NONE && weapon_must_be_readied(weapon_index))
		{
			result = index;
			break;
		}
	}

	return result;
}

static void unit_align_facing(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->object.parent_object_index == NONE)
	{
		match_assert_valid_real_normal2d("c:\\halo\\SOURCE\\units\\units.c", 9353, alignment_vector);
		set_real_vector3d(&unit->object.forward, alignment_vector->i, alignment_vector->j, 0.f);
		unit->object.up = *global_up3d;

		match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\units\\units.c", 9357, &unit->object.forward, &unit->object.up);
	}

	return;
}

boolean unit_has_weapon(
	long unit_index,
	long weapon_index)
{
	long index;
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		long current_weapon_index = unit->unit.weapon_object_indices[index];

		if (current_weapon_index == weapon_index)
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

boolean unit_has_weapon_with_flag(
	long unit_index,
	long flag)
{
	long index;
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	for (index = 0; index < MAXIMUM_WEAPONS_PER_UNIT; index++)
	{
		long weapon_index = unit->unit.weapon_object_indices[index];

		if (weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(weapon_index);

			if (TEST_FLAG(weapon->weapon.flags, flag))
			{
				result = TRUE;
				break;
			}
		}
	}

	return result;
}

void unit_start_flaming_to_death(
	long unit_index,
	long attacker_object_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	unit_drop_current_weapon(unit_index, TRUE);
	SET_FLAG(unit->unit.flags, _unit_ignore_hard_pings_bit, TRUE);
	SET_FLAG(unit->object.damage_flags, _object_dead_bit, FALSE);
	SET_FLAG(unit->object.damage_flags, _object_cannot_take_damage_bit, TRUE);

	if (unit->unit.flaming_death_delay == 0)
	{
		short delay = seed_random_range(get_global_random_seed_address(), 60, 150);

		unit->unit.flaming_death_delay = PIN(delay, 1, UNSIGNED_CHAR_MAX);
		unit->unit.flaming_death_attacker_object_index = attacker_object_index;
		unit_start_running_blindly(unit_index);
	}

	return;
}

void unit_start_running_blindly(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (!TEST_FLAG(unit->unit.flags, _unit_running_blindly_bit))
	{
		real_vector3d initial_run_vector;
		real angle_range;

		SET_FLAG(unit->unit.flags, _unit_running_blindly_bit, TRUE);

		if (unit->unit.actor_index != NONE && actor_get_running_blind_vector(unit->unit.actor_index, &initial_run_vector))
		{
			unit->unit.run_blindly_angle = 0.f;
			angle_range = DEGREES_TO_RADIANS(25.f);
		}
		else
		{
			real_euler_angles2d angles;

			euler_angles2d_from_vector3d(&angles, &unit->object.forward);

			if (angles.yaw > _pi)
			{
				angles.yaw -= 2.f * _pi;
			}

			unit->unit.run_blindly_angle = angles.yaw;
			angle_range = DEGREES_TO_RADIANS(100.f);
		}

		unit->unit.run_blindly_angle += real_seed_random_range(get_global_random_seed_address(), -angle_range, angle_range);
	}

	return;
}

void unit_stop_running_blindly(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->unit.flags, _unit_running_blindly_bit, FALSE);

	return;
}

static void unit_flame_to_death(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct game_globals_falling_damage *falling_damage = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->falling_damage, 0, struct game_globals_falling_damage);

	unit_stop_running_blindly(unit_index);
	SET_FLAG(unit->unit.flags, _unit_ignore_hard_pings_bit, FALSE);
	SET_FLAG(unit->object.damage_flags, _object_cannot_take_damage_bit, FALSE);

	if (falling_damage && falling_damage->flaming_death_damage.index != NONE)
	{
		struct damage_data damage;
		struct object_datum *attacker_object = object_try_and_get(unit->unit.flaming_death_attacker_object_index);

		damage_data_new(&damage, falling_damage->flaming_death_damage.index);

		if (attacker_object)
		{
			damage.owner_player_index = attacker_object->object.owner_player_index;

			if (attacker_object->object.owner_object_index == NONE)
			{
				damage.owner_object_index = unit->unit.flaming_death_attacker_object_index;
			}
			else
			{
				damage.owner_object_index = attacker_object->object.owner_object_index;
			}

			damage.owner_team_index = attacker_object->object.owner_team_index;
		}

		object_cause_damage(&damage, unit_index, NONE, NONE, NONE, NULL);
	}

	if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		error(_error_silent, "WARNING: %s tried to die from flaming to death but couldn't", tag_name_strip_path(tag_get_name(unit->definition_index)));
		SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_bit, TRUE);
	}

	return;
}

static void unit_running_blind(
	long unit_index,
	real_vector3d *run_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean actor_controlled = FALSE;
	
	if (unit->unit.actor_index == NONE || !actor_get_running_blind_vector(unit->unit.actor_index, run_vector))
	{
		*run_vector = *global_forward3d;
	}
	else
	{
		actor_controlled = TRUE;
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(9534, 9541), run_vector);

	{
		real angular_acceleration_this_tick;

		real negative_angle_allowed = 1.f;
		real positive_angle_allowed = 1.f;

		if (actor_controlled)
		{
			real negative_angle_bounds_dist = RUN_BLINDLY_MAXIMUM_ANGLE - unit->unit.run_blindly_angle;
			real positive_angle_bounds_dist = unit->unit.run_blindly_angle + RUN_BLINDLY_MAXIMUM_ANGLE;

			negative_angle_allowed = MIN(negative_angle_allowed, negative_angle_bounds_dist / (RUN_BLINDLY_MAXIMUM_ANGLE * 0.3f));
			positive_angle_allowed = MIN(positive_angle_allowed, positive_angle_bounds_dist / (RUN_BLINDLY_MAXIMUM_ANGLE * 0.3f));
		}

		{
			real negative_velocity_bounds_dist = RUN_BLINDLY_MAXIMUM_ANGULAR_VELOCITY - unit->unit.run_blindly_angle_delta;
			real positive_velocity_bounds_dist = unit->unit.run_blindly_angle_delta + RUN_BLINDLY_MAXIMUM_ANGULAR_VELOCITY;

			negative_angle_allowed = MIN(negative_angle_allowed, negative_velocity_bounds_dist / (RUN_BLINDLY_MAXIMUM_ANGULAR_VELOCITY * 0.3f));
			positive_angle_allowed = MIN(positive_angle_allowed, positive_velocity_bounds_dist / (RUN_BLINDLY_MAXIMUM_ANGULAR_VELOCITY * 0.3f));
		}

		if (negative_angle_allowed < positive_angle_allowed)
		{
			if (negative_angle_allowed < -1.f)
			{
				angular_acceleration_this_tick = -RUN_BLINDLY_ANGULAR_ACCELERATION;
			}
			else
			{
				angular_acceleration_this_tick = real_random_range(-RUN_BLINDLY_ANGULAR_ACCELERATION, RUN_BLINDLY_ANGULAR_ACCELERATION * MIN(1.f, negative_angle_allowed));
			}
		}
		else if (positive_angle_allowed < -1.f)
		{
			angular_acceleration_this_tick = RUN_BLINDLY_ANGULAR_ACCELERATION;
		}
		else
		{
			positive_angle_allowed = MIN(1.f, positive_angle_allowed);
			angular_acceleration_this_tick = real_random_range(-RUN_BLINDLY_ANGULAR_ACCELERATION * positive_angle_allowed, RUN_BLINDLY_ANGULAR_ACCELERATION);
		}

		unit->unit.run_blindly_angle_delta += angular_acceleration_this_tick;
	}

	unit->unit.run_blindly_angle += unit->unit.run_blindly_angle_delta;

	if (unit->unit.run_blindly_angle < -_pi)
	{
		unit->unit.run_blindly_angle += 2.f * _pi;
	}
	else if (unit->unit.run_blindly_angle > _pi)
	{
		unit->unit.run_blindly_angle -= 2.f * _pi;
	}

	rotate_vector_about_axis(run_vector, global_up3d, sine(unit->unit.run_blindly_angle), cosine(unit->unit.run_blindly_angle));

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", MATCH_LINE_NTSC(9605, 9612), run_vector)

	return;
}

boolean unit_flying_through_air(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (unit->object.type == _object_type_biped)
	{
		result = biped_flying_through_air(unit_index);
	}

	return result;
}

/* ---------- private code */
