/*
BIPEDS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bipeds.h"
#include "vehicles.h"
#include "biped_limp_noodle.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "network_messages.h"
#include "actors.h"
#include "collisions.h"
#include "sound_manager.h"
#include "ai_debug.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "structures.h"
#include "observer.h"
#include "physics_constants.h"
#include "network_client_manager.h"
#include "effects.h"
#include "object_lights.h"
#include "object_types.h"
#include "cinematics.h"
#include "devices.h"
#include "physics.h"
#include "first_person_weapons.h"
#include "recorded_animations.h"
#include "device_definitions.h"
#include "material_effects.h"
#include "material_effect_definitions.h"
#include "collision_models.h"
#include "render_debug.h"
#include "damage_effect_definitions.h"
#include "collision_debug.h"

/* ---------- constants */

enum
{
	MAXIMUM_BIPED_PHYSICS_COLLISIONS = 16, /* fake name */
};

enum
{
	_biped_physics_in_airborne_bit = 0,
	_biped_physics_in_slipping_bit,
	_biped_physics_in_crouched_bit,
	_biped_physics_in_trying_to_stand_bit,
	_biped_physics_in_flying_bit,
	_biped_physics_in_absolute_movement_bit,
	_biped_physics_in_no_collision_bit,
	_biped_physics_in_dead_bit,
	_biped_physics_in_pass_through_bipeds_bit,
	_biped_physics_in_climb_anything_bit,
	NUMBER_OF_BIPED_PHYSICS_IN_FLAGS,
};

enum
{
	_biped_physics_out_airborne_bit = 0,
	_biped_physics_out_slipping_bit,
	_biped_physics_out_cannot_stand_bit,
	_biped_physics_out_splatter_bit,
	_biped_physics_out_volatile_collision_bit,
	NUMBER_OF_BIPED_PHYSICS_OUT_FLAGS,
};

/* ---------- macros */

#define match_assert_valid_real_vector3d_axes3(file, line, forward, left, up)		\
match_vassert(																		\
	file,																			\
	line,																			\
	valid_real_vector3d_axes3(forward, left, up),									\
	csprintf(																		\
		temporary,																	\
		"%s, %s, %s: assert_valid_real_vector3d_axes3(%f, %f, %f / %f, %f, %f / %f, %f, %f)",\
		#forward, #left, #up,														\
		(*forward).i, (*forward).j, (*forward).k,									\
		(*up).i, (*up).j, (*up).k,													\
		(*left).i, (*left).j, (*left).k												\
	)																				\
)

/* ---------- structures */

struct biped_physics
{
	long biped_index;
	word in_flags;
	real_point3d position;
	real_vector3d forward;
	real_vector3d aiming;
	real_vector3d velocity;
	real crouch_velocity;
	real_vector3d movement_desired;
	real movement_penalty;
	real acceleration_maximum;
	real airborne_acceleration_maximum;
	real height;
	real width;
	real ground_tangential_velocity_max;
	real ground_tangential_angle;
	real minimum_normal_k;
	real downhill_k0;
	real downhill_k1;
	real downhill_velocity_scale;
	real uphill_k0;
	real uphill_k1;
	real uphill_velocity_scale;
	real_plane3d ground_plane;
	long existing_support_surface_index;
	real gravity;
	long bumped_object_index;
	long elevator_object_index;
	word out_flags;
	long support_surface_index;
	long stick_surface_index;
	real_point3d new_position;
	real_vector3d new_velocity;
	real landing_velocity;
	real collision_velocity;
};

/* ---------- prototypes */

static long biped_find_ground_surface(long biped_index, real ground_distance, real_vector3d const *ground_direction, real_point3d *surface_point, real_vector3d *surface_normal);
static void biped_verify_object_vectors(long biped_index, char const *debug_string);
static void biped_update_turning(long biped_index, struct unit_animation_update_data *animation);
static void biped_update_moving(long biped_index, struct unit_animation_update_data *animation);
static void biped_bumped_object(long biped_index, long object_index, real_vector3d const *old_velocity);
static void biped_falling_danger(long biped_index);
static void biped_vehicle_speech(long biped_index);
static boolean biped_check_discard(long biped_index);
static void biped_falling_damage(long biped_index, real collision_velocity);
static void biped_apply_rotation(long biped_index);
static void biped_update_dead(long biped_index, struct unit_animation_update_data *animation);
static void biped_update_airborne(long biped_index, struct unit_animation_update_data *animation);
static void biped_start_landing(long biped_index, real landing_velocity);
static void biped_update_landing(long biped_index, struct unit_animation_update_data *animation);
static void biped_update_slipping(long biped_index, struct unit_animation_update_data *animation);
static void biped_update_jumping(long biped_index, struct unit_animation_update_data *animation);
static boolean biped_jump(long biped_index);
static void biped_update_physics(struct biped_physics *physics);
static void biped_try_to_make_footsteps(long biped_index);
static void biped_make_footstep(long biped_index, short event_index, short contact_point_index);
static void biped_find_nearby_support_surface(long biped_index);
static void biped_snap_facing(long biped_index);

/* ---------- globals */

boolean rider_ejection = TRUE;

static struct profile_section biped_update_section = {"biped_update", NONE, TRUE};

boolean debug_biped_physics = FALSE;
boolean debug_biped_skip_update = FALSE;
boolean debug_biped_skip_collision = FALSE;
boolean debug_biped_limp_body_disable = FALSE;

/* ---------- public code */

void bipeds_initialize(
	void)
{
	return;
}

void bipeds_initialize_for_new_map(
	void)
{
	return;
}

void bipeds_dispose_from_old_map(
	void)
{
	return;
}

void bipeds_dispose(
	void)
{
	return;
}

void biped_place(
	long biped_index,
	struct scenario_biped_datum *scenario_biped)
{
	unit_place(biped_index, &scenario_biped->unit);
	object_add_scenario_permutation(biped_index, &scenario_biped->permutation);

	return;
}

void biped_adjust_placement(
	long object_index,
	struct object_placement_data *data)
{
	struct biped_datum *biped = biped_get(object_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (TEST_FLAG(definition->biped.flags, _biped_pill_centered_at_origin_bit) &&
		!TEST_FLAG(definition->biped.flags, _biped_flying_bit))
	{
		point_from_line3d(&data->position, &data->up, (real)(definition->biped.collision_radius), &data->position);
	}

	return;
}

boolean biped_new(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	biped->biped.ground_plane = depths_of_hell;
	biped->biped.jump_recovery_timer = 127;
	biped->biped.support_surface_index = NONE;
	biped->biped.pathfinding_surface_index = NONE;
	object_get_origin(biped_index, &biped->biped.pathfinding_point);
	biped->biped.last_pathfinding_surface_index = NONE;
	biped->biped.last_pathfinding_attempt_time = NONE;
	biped->biped.impact_target_object_index = NONE;

	if (TEST_FLAG(definition->biped.flags, _biped_climbs_anything_bit))
	{
		biped_find_nearby_support_surface(biped_index);
	}
	biped_snap_facing(biped_index);

	biped->biped.elevator_object_index = NONE;
	biped->biped.elevator_ticks = 0;

	return TRUE;
}

void biped_delete(
	long biped_index)
{
	return;
}

boolean biped_update(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);
	struct unit_animation_update_data animation;

	if (!debug_biped_skip_update)
	{
		profile_enter(biped_update_section);

		biped_verify_object_vectors(biped_index, "pre-update");

		animation.state_desired = _unit_state_idle;
		animation.crouching = FALSE;

		if (biped->object.parent_object_index != NONE)
		{
			struct object_datum *object = object_get(biped->object.parent_object_index);

			if (object->object.type == _object_type_vehicle)
			{
				struct vehicle_datum *vehicle = vehicle_get(biped->object.parent_object_index);

				biped_vehicle_speech(biped_index);
				if (TEST_FLAG(biped->unit.control_flags, _unit_control_action_bit))
				{
					unit_try_and_exit_seat(biped_index);
				}
				if (rider_ejection && vehicle->object.up.k < 0.f && TEST_FLAG(vehicle->object.flags, _object_on_ground_bit))
				{
					unit_exit_seat_end(biped_index);
				}
			}
			else if (object->object.type == _object_type_biped)
			{
				animation.state_desired = TEST_FLAG(object->object.damage_flags, _object_dead_bit) ? _unit_state_feeding : _unit_state_melee_continuous;
			}
		}
		else
		{
			biped_snap_facing(biped_index);

			if (TEST_FLAG(biped->object.damage_flags, _object_dead_bit) ||
				(!TEST_FLAG(definition->biped.flags, _biped_flying_bit) && !TEST_FLAG(definition->biped.flags, _biped_climbs_anything_bit)))
			{
				biped->unit.desired_facing_vector.k = 0.f;
				if (normalize3d(&biped->unit.desired_facing_vector) == 0.f)
				{
					biped->unit.desired_facing_vector = *global_forward3d;
				}
			}

			switch (biped->unit.animation.state)
			{
			case _unit_state_move_front:
			case _unit_state_move_back:
			case _unit_state_move_left:
			case _unit_state_move_right:
				biped->biped.state = _biped_state_moving;
				break;
			case _unit_state_idle:
			case _unit_state_turn_left:
			case _unit_state_turn_right:
				biped->biped.state = _biped_state_idle;
				break;
			default:
				biped->biped.state = _biped_state_unknown;
				break;
			}

			if (magnitude_squared3d(&biped->unit.throttle) < 0.1f * 0.1f)
			{
				biped->unit.throttle = *global_zero_vector3d;
			}

			if (TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
			{
				if (biped->biped.airborne_ticks < SCHAR_MAX)
				{
					biped->biped.airborne_ticks++;
				}
			}
			else
			{
				biped->biped.airborne_ticks = 0;
			}

			if (TEST_FLAG(biped->biped.flags, _biped_slipping_bit))
			{
				if (biped->biped.slipping_ticks < SCHAR_MAX)
				{
					biped->biped.slipping_ticks++;
				}
			}
			else
			{
				biped->biped.slipping_ticks = 0;
			}

			animation.state_desired = _unit_state_idle;
			animation.crouching = TEST_FLAG(biped->unit.control_flags, _unit_control_crouch_modifier_bit);

			biped_verify_object_vectors(biped_index, "pre-turning");
			if (!TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				biped_update_turning(biped_index, &animation);
				biped_verify_object_vectors(biped_index, "post-turning");
			}
			biped_update_moving(biped_index, &animation);
			biped_verify_object_vectors(biped_index, "post-moving");

			if (TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				biped_update_dead(biped_index, &animation);
			}
			else if (TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
			{
				biped_update_airborne(biped_index, &animation);
			}
			else if (biped->biped.landing != NONE)
			{
				biped_update_landing(biped_index, &animation);
			}
			else if (TEST_FLAG(biped->biped.flags, _biped_slipping_bit))
			{
				biped_update_slipping(biped_index, &animation);
			}
			biped_verify_object_vectors(biped_index, "post-dead/air/land/slip");

			if (!biped->biped.player_melee_ticks)
			{
				if (biped->unit.player_index != NONE && TEST_FLAG(biped->unit.control_flags, _unit_control_use_equipment_bit))
				{
					struct unit_datum *unit = unit_get(biped_index);
					long weapon_index = unit_inventory_get_weapon(biped_index, unit->unit.current_weapon_index);

					if (!weapon_prevents_melee_attack(weapon_index) && biped->unit.current_zoom_level == NONE)
					{
						short reduction;

						unit_animation_start_action(biped_index, 7);
						weapon_stop_reload(weapon_index);
						first_person_weapon_message_from_unit(biped_index, _first_person_weapon_message_melee);
						biped->biped.player_melee_ticks = (char)weapon_get_first_person_animation_time(weapon_index, _first_person_animation_time_end, _first_person_weapon_animation_melee, NONE);
						biped->biped.player_melee_attack_tick = biped->biped.player_melee_ticks - (char)weapon_get_first_person_animation_time(weapon_index, _first_person_animation_time_key, _first_person_weapon_animation_melee, NONE);
						reduction = biped->biped.player_melee_ticks >> 2;
						biped->biped.player_melee_ticks -= reduction;
						biped->biped.player_melee_attack_tick -= reduction;
					}
				}
			}
			else
			{
				if (biped->biped.player_melee_ticks == biped->biped.player_melee_attack_tick)
				{
					unit_cause_player_melee_damage(biped_index);
				}
				biped->biped.player_melee_ticks--;
			}

			biped_try_to_make_footsteps(biped_index);
			biped_falling_danger(biped_index);
			biped_check_discard(biped_index);
		}

		if (unit_update_animation(biped_index, &animation) == _unit_animation_result_jump)
		{
			biped_jump(biped_index);
		}

		if (TEST_FLAG(biped->object.damage_flags, _object_dead_bit) && TEST_FLAG(biped->object.flags, _object_at_rest_bit))
		{
			biped->object.idle_ticks++;
		}
		else
		{
			biped->object.idle_ticks = 0;
		}

		biped_verify_object_vectors(biped_index, "post-update");

		profile_exit(biped_update_section);
	}

	return TRUE;
}

void biped_export_function_values(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);
	short function_index;

	for (function_index = 0; function_index < NUMBEROF(definition->biped.function_modes); function_index++)
	{
		if (definition->biped.function_modes[function_index] != _biped_function_none)
		{
			real value = 0.f;

			switch (definition->biped.function_modes[function_index])
			{
			case _biped_function_flying_speed:
				value = magnitude3d(&biped->object.translational_velocity) / (definition->biped.flying_velocity / TICKS_PER_SECOND);
				value = PIN(value, 0.f, 1.f);
				break;
			}

			biped->object.incoming_function_values[function_index] = value;
		}
	}

	return;
}

void biped_preprocess_node_orientations(
	long biped_index,
	struct real_orientation *node_orientations)
{
	biped_verify_object_vectors(biped_index, "preprocess-nodes");

	return;
}

void biped_reset(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);

	memset(&biped->biped, 0, sizeof(biped->biped));
	biped->biped.ground_plane = depths_of_hell;
	biped->biped.last_falling_communication_time = NONE;

	return;
}

void biped_disconnect_from_structure_bsp(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);

	biped->biped.support_surface_index = NONE;
	biped->biped.pathfinding_surface_index = NONE;
	biped->biped.last_pathfinding_surface_index = NONE;

	return;
}

void biped_accelerate(
	long biped_index,
	real_vector3d *acceleration)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (!TEST_FLAG(definition->unit.flags, _unit_is_special_bit))
	{
		biped_verify_object_vectors(biped_index, "pre-accel");

		if (!TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
		{
			scale_vector3d(acceleration, 0.5f, acceleration);
		}
		if (TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
		{
			biped_stop_limp_body_physics(biped_index);
		}

		add_vectors3d(&biped->object.translational_velocity, acceleration, &biped->object.translational_velocity);
		SET_FLAG(biped->object.flags, _object_at_rest_bit, FALSE);
		SET_FLAG(biped->biped.flags, _biped_slipping_bit, TRUE);
		SET_FLAG(biped->biped.flags, _biped_airborne_bit, TRUE);

		if (TEST_FLAG(biped->object.damage_flags, _object_dead_bit) ||
			TEST_FLAG(definition->biped.flags, _biped_flying_bit) ||
			TEST_FLAG(definition->biped.flags, _biped_climbs_anything_bit))
		{
			real_vector3d angular_acceleration_axis;

			normalize3d(cross_product3d(global_up3d, acceleration, &angular_acceleration_axis));
			vector_from_line3d(&biped->object.angular_velocity, &angular_acceleration_axis, real_random() * (_pi / 2.f) * magnitude3d(acceleration), &biped->object.angular_velocity);
		}

		if (biped->object.parent_object_index == NONE)
		{
			real_vector3d new_forward = *acceleration;

			if (normalize3d(&new_forward) > 0.f)
			{
				biped->object.forward = new_forward;
				biped_snap_facing(biped_index);
				biped_verify_object_vectors(biped_index, "post-accel");
			}
		}
	}

	return;
}

void biped_get_sight_position(
	long biped_index,
	short estimate_mode,
	real_point3d const *estimated_body_position,
	real_vector3d *desired_facing,
	real_vector3d const *desired_gun_offset,
	real_point3d *sight_position)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 759, (estimate_mode == _unit_estimate_none) || (estimated_body_position != NULL));
	match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 760, (estimate_mode != _unit_estimate_gun_position) || (desired_facing != NULL));

	if (estimate_mode == _unit_estimate_none)
	{
		object_get_origin(biped_index, sight_position);
	}
	else
	{
		*sight_position = *estimated_body_position;
	}

	if (estimate_mode == _unit_estimate_gun_position)
	{
		real_vector3d desired_left_vector;

		match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 775, desired_facing && desired_gun_offset);

		desired_left_vector.i = -desired_facing->j;
		desired_left_vector.j = desired_facing->i;
		desired_left_vector.k = 0.f;
		point_from_line3d(sight_position, desired_facing, desired_gun_offset->i, sight_position);
		vector_from_line3d(sight_position, &desired_left_vector, desired_gun_offset->j, sight_position);
		sight_position->z += desired_gun_offset->k;
	}
	else
	{
		real crouch;

		switch (estimate_mode)
		{
		case _unit_estimate_head_standing:
			crouch = 0.f;
			break;
		case _unit_estimate_head_crouching:
			crouch = 1.f;
			break;
		default:
			crouch = biped->biped.crouch;
			break;
		}

		sight_position->z += (1.f - crouch) * definition->biped.standing_camera_height + crouch * definition->biped.crouching_camera_height;
	}

	return;
}

void biped_get_physics_pill(
	long biped_index,
	real_point3d *base,
	real *height,
	real *width)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	object_get_origin(biped_index, base);
	if (!TEST_FLAG(definition->biped.flags, _biped_pill_centered_at_origin_bit))
	{
		base->z += definition->biped.collision_radius;
	}

	if (TEST_FLAG(definition->biped.flags, _biped_spherical_bit) ||
		(biped->unit.player_index == NONE && !TEST_FLAG(biped->object.flags, _object_movie_star_bit)))
	{
		*height = 0.f;
	}
	else
	{
		*height = (definition->biped.collision_height_crouching - definition->biped.collision_height_standing) * biped->biped.crouch + definition->biped.collision_height_standing - 2.f * definition->biped.collision_radius;
	}
	*width = definition->biped.collision_radius;

	return;
}

void biped_get_autoaim_pill(
	long biped_index,
	real_point3d *base,
	real_vector3d *height,
	real *width)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (definition->biped.runtime_pelvis_node_index == NONE || definition->biped.runtime_head_node_index == NONE)
	{
		real height_scalar;
		real unused_width_scalar;

		biped_get_physics_pill(biped_index, base, &height_scalar, &unused_width_scalar);
		base->z += 0.5f * height_scalar;
		scale_vector3d(global_up3d, 0.5f * height_scalar, height);
		*width = definition->biped.autoaim_width;
	}
	else
	{
		real_matrix4x3 *pelvis_matrix = object_get_node_matrix(biped_index, definition->biped.runtime_pelvis_node_index);
		real_matrix4x3 *head_matrix = object_get_node_matrix(biped_index, definition->biped.runtime_head_node_index);

		if (TEST_FLAG(definition->biped.flags, _biped_spherical_bit))
		{
			midpoint3d(&pelvis_matrix->position, &head_matrix->position, base);
			*height = *global_zero_vector3d;
		}
		else
		{
			*base = pelvis_matrix->position;
			vector_from_points3d(&pelvis_matrix->position, &head_matrix->position, height);
		}
		*width = definition->biped.autoaim_width;
	}

	return;
}

boolean biped_fix_position(
	long biped_index,
	long line_of_site_object_index,
	real_point3d const *new_position,
	real_point3d *final_position,
	real max_radius_fudge_factor,
	boolean fix_below_new_position,
	boolean dont_teleport,
	boolean use_radius_as_multiplier)
{
	boolean fixed = FALSE;

	match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 893, final_position || !dont_teleport);
	match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\bipeds.c", 895, _collision_user_bipeds);

	if (biped_index != NONE || line_of_site_object_index != NONE)
	{
		static real_vector3d const fudge_vectors[] =
		{
			{ 0.f, 0.f, 0.f },
			{ 1.f, 0.f, 0.f },
			{ 0.f, 0.f, 1.f },
			{ 0.70710678f, 0.f, 0.70710678f },
			{ 0.57735027f, 0.57735027f, 0.57735027f },
			{ 0.57735027f, -0.57735027f, 0.57735027f },
			{ 0.70710678f, 0.70710678f, 0.f },
			{ 0.70710678f, -0.70710678f, 0.f },
			{ 0.f, 0.70710678f, 0.70710678f },
			{ 0.f, -0.70710678f, 0.70710678f },
			{ -1.f, 0.f, 0.f },
			{ 0.f, 1.f, 0.f },
			{ 0.f, -1.f, 0.f },
			{ -0.70710678f, -0.70710678f, 0.f },
			{ -0.70710678f, 0.70710678f, 0.f },
			{ -0.70710678f, 0.f, 0.70710678f },
			{ -0.57735027f, -0.57735027f, 0.57735027f },
			{ -0.57735027f, 0.57735027f, 0.57735027f },
			{ 0.f, 0.f, -1.f },
			{ 0.70710678f, 0.f, -0.70710678f },
			{ -0.70710678f, 0.f, -0.70710678f },
			{ 0.f, 0.70710678f, -0.70710678f },
			{ 0.f, -0.70710678f, -0.70710678f },
			{ 0.57735027f, 0.57735027f, -0.57735027f },
			{ 0.57735027f, -0.57735027f, -0.57735027f },
			{ -0.57735027f, -0.57735027f, -0.57735027f },
			{ -0.57735027f, 0.57735027f, -0.57735027f },
		};
		struct biped_datum *biped;
		struct biped_definition *biped_definition;
		real_point3d fixed_position;
		real_point3d anchor_position;
		real width;
		real height;
		boolean swapped;
		unsigned long collision_flags;
		struct collision_model_instance instance;
		real_vector3d right;
		real_vector3d height_vector;
		short fudge_vector_count;
		short i;

		if (line_of_site_object_index != NONE)
		{
			real dummy;

			object_get_bounding_sphere(line_of_site_object_index, &anchor_position, &dummy);
		}

		swapped = FALSE;
		if (biped_index == NONE)
		{
			biped_index = line_of_site_object_index;
			swapped = TRUE;
		}

		biped = biped_get(biped_index);
		biped_definition = biped_definition_get(biped->definition_index);
		collision_flags = TEST_FLAG(biped_definition->biped.flags, _biped_passes_through_bipeds_bit) ? _collision_test_for_bipeds_passthrough_living_flags : _collision_test_for_bipeds_living_flags;

		if (new_position)
		{
			fixed_position = *new_position;
			biped_get_physics_pill(biped_index, (real_point3d *)&height_vector, &height, &width);
		}
		else
		{
			biped_get_physics_pill(biped_index, &fixed_position, &height, &width);
		}

		if (swapped)
		{
			biped_index = NONE;
		}

		fudge_vector_count = NUMBEROF(fudge_vectors) - (fix_below_new_position ? 0 : 9);

		if (line_of_site_object_index != NONE)
		{
			collision_model_instance_new(&instance, line_of_site_object_index);
		}

		cross_product3d(&biped->object.forward, &biped->object.up, &right);
		normalize3d(&right);
		scale_vector3d(global_up3d, height, &height_vector);

		if (use_radius_as_multiplier)
		{
			max_radius_fudge_factor *= width;
		}

		for (i = 0; !fixed && i < fudge_vector_count; i++)
		{
			real_point3d fudge_position;

			if (fix_below_new_position)
			{
				point_from_line3d(&fixed_position, &biped->object.forward, fudge_vectors[i].i * max_radius_fudge_factor, &fudge_position);
				point_from_line3d(&fudge_position, &right, fudge_vectors[i].j * max_radius_fudge_factor, &fudge_position);
				point_from_line3d(&fudge_position, &biped->object.up, fudge_vectors[i].k * max_radius_fudge_factor, &fudge_position);
			}
			else
			{
				point_from_line3d(&fixed_position, &fudge_vectors[i], max_radius_fudge_factor, &fudge_position);
			}

			if ((scenario_leaf_index_from_point(&fudge_position) == NONE ? NONE : TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, scenario_leaf_index_from_point(&fudge_position) & LONG_MAX, struct structure_leaf)->cluster_index) != NONE)
			{
				struct collision_result result;
				struct collision_model_test_pill_result model_pill_result;
				struct collision_result collision;

				if (collision_fix_pill(collision_flags, &fudge_position, 2.f * width, height, width, biped_index, &fudge_position) &&
					!collision_test_pill(collision_flags, &fudge_position, &height_vector, width, biped_index, &result) &&
					(line_of_site_object_index == NONE ||
					!collision_model_test_pill(&instance, &fudge_position, &height_vector, width, &model_pill_result) &&
					(!collision_test_line(collision_flags, &fudge_position, &anchor_position, biped_index, &collision) || collision.object_index == line_of_site_object_index) &&
					(!collision_test_line(collision_flags, &anchor_position, &fudge_position, line_of_site_object_index, &collision) || collision.object_index == biped_index)))
				{
					struct biped_definition *definition = biped_definition_get(biped->definition_index);
					struct location fixed_location;

					scenario_location_from_point(&fixed_location, &fudge_position);
					match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 1054, fixed_location.cluster_index!=NONE);

					if (!TEST_FLAG(definition->biped.flags, _biped_pill_centered_at_origin_bit))
					{
						fudge_position.z -= definition->biped.collision_radius;
					}

					if (biped_index != NONE && !dont_teleport)
					{
						biped->object.position = fudge_position;
						object_compute_node_matrices_recursive(biped_index);
						object_translate(biped_index, &fudge_position, &fixed_location);
					}

					if (final_position)
					{
						*final_position = fudge_position;
					}

					fixed = TRUE;
				}
			}
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\units\\bipeds.c", 1080);

	return fixed;
}

void biped_render_debug(
	long biped_index)
{
	real_point3d base;
	real_vector3d height_vector;
	real height;
	real width;

	if (debug_objects_biped_physics_pills)
	{
		biped_get_physics_pill(biped_index, &base, &height, &width);
		if (height > 0.f)
		{
			scale_vector3d(global_up3d, height, &height_vector);
			render_debug_pill(TRUE, &base, &height_vector, width, global_real_argb_white);
		}
		else
		{
			render_debug_sphere(TRUE, &base, width, global_real_argb_white);
		}
	}

	if (debug_objects_biped_autoaim_pills)
	{
		biped_get_autoaim_pill(biped_index, &base, &height_vector, &width);
		if (magnitude_squared3d(&height_vector) > 0.f)
		{
			render_debug_pill(TRUE, &base, &height_vector, width, global_real_argb_red);
		}
		else
		{
			render_debug_sphere(TRUE, &base, width, global_real_argb_red);
		}
	}

	return;
}

static long biped_find_ground_surface(
	long biped_index,
	real ground_distance,
	real_vector3d const *ground_direction,
	real_point3d *surface_point,
	real_vector3d *surface_normal)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct collision_bsp *bsp = global_collision_bsp_get();
	long surface_index = NONE;
	struct collision_bsp_test_vector_result result;
	real_point3d collision_origin;
	real_vector3d collision_vector;

	match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\bipeds.c", 1146, _collision_user_bipeds);

	object_get_origin(biped_index, &collision_origin);
	point_from_line3d(&collision_origin, global_up3d, 0.4f, &collision_origin);
	scale_vector3d(ground_direction, ground_distance, &collision_vector);
	if (collision_bsp_test_vector(FLAG(_collision_bsp_test_front_facing_surfaces_bit), bsp, 0, NULL, &collision_origin, &collision_vector, REAL_MAX, &result))
	{
		surface_index = result.surface_index;
		if (surface_point)
		{
			point_from_line3d(&collision_origin, &collision_vector, result.t, surface_point);
		}
		if (surface_normal)
		{
			*surface_normal = result.plane->n;
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\units\\bipeds.c", 1168);

	return surface_index;
}

long biped_approximate_surface_index(
	long biped_index,
	real_point3d *surface_point)
{
	return biped_find_ground_surface(biped_index, 2.f, global_down3d, surface_point, NULL);
}

long biped_find_pathfinding_surface_index(
	long biped_index,
	real_point3d *pathfinding_point)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (!TEST_FLAG(definition->biped.flags, _biped_flying_bit) || TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
	{
		if (biped->biped.pathfinding_surface_index == NONE)
		{
			long game_time = game_time_get();

			if (game_time > biped->biped.last_pathfinding_attempt_time)
			{
				struct collision_bsp *bsp = global_collision_bsp_get();
				real_point3d found_point = biped->biped.pathfinding_point;

				biped->biped.last_pathfinding_attempt_time = game_time;
				if (biped->biped.support_surface_index != NONE)
				{
					real_point2d closest_point2d;

					collision_surface_find_closest_point2d(bsp, biped->biped.support_surface_index, _z, TRUE, (real_point2d *)&biped->biped.pathfinding_point, &closest_point2d);
					collision_surface_project_point2d(bsp, biped->biped.support_surface_index, _z, TRUE, &closest_point2d, &found_point);
					biped->biped.pathfinding_surface_index = biped->biped.support_surface_index;
				}
				else if (biped->biped.last_pathfinding_surface_index != NONE &&
					collision_surface_test_point2d(bsp, biped->biped.last_pathfinding_surface_index, _z, TRUE, (real_point2d *)&biped->biped.pathfinding_point))
				{
					biped->biped.pathfinding_surface_index = biped->biped.last_pathfinding_surface_index;
					collision_surface_project_point2d(bsp, biped->biped.pathfinding_surface_index, _z, TRUE, (real_point2d *)&biped->biped.pathfinding_point, &found_point);
					biped->biped.pathfinding_surface_index = biped->biped.last_pathfinding_surface_index;
				}

				if (biped->biped.pathfinding_surface_index == NONE)
				{
					biped->biped.pathfinding_surface_index = biped_approximate_surface_index(biped_index, &found_point);
				}

				if (biped->biped.pathfinding_surface_index != NONE)
				{
					biped->biped.pathfinding_point = found_point;
					biped->biped.last_pathfinding_surface_index = biped->biped.pathfinding_surface_index;
				}
			}
		}
	}
	else
	{
		biped->biped.pathfinding_surface_index = NONE;
		object_get_origin(biped_index, pathfinding_point);
	}

	match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 1255, pathfinding_point);
	*pathfinding_point = biped->biped.pathfinding_point;

	return biped->biped.pathfinding_surface_index;
}

static void biped_verify_object_vectors(
	long biped_index,
	char const *debug_string)
{
	struct biped_datum *biped = biped_get(biped_index);

	if (!valid_real_vector3d_axes2(&biped->object.forward, &biped->object.up))
	{
		struct biped_definition *definition = biped_definition_get(biped->definition_index);

		match_vhalt("c:\\halo\\SOURCE\\units\\bipeds.c", 1373, csprintf(temporary, "biped %s (%s%s%s): %s vector failure: (%f, %f, %f) / (%f, %f, %f)",
			tag_name_strip_path(tag_get_name(biped->definition_index)),
			TEST_FLAG(definition->biped.flags, _biped_flying_bit) ? "flying" : TEST_FLAG(definition->biped.flags, _biped_uses_player_physics_bit) ? "player-physics" : TEST_FLAG(definition->biped.flags, _biped_climbs_anything_bit) ? "climb" : "normal",
			TEST_FLAG(biped->object.damage_flags, _object_dead_bit) ? "/dead" : "",
			TEST_FLAG(biped->biped.flags, _biped_limp_body_physics_active_bit) ? "/limping" : "",
			debug_string,
			biped->object.forward.i, biped->object.forward.j, biped->object.forward.k,
			biped->object.up.i, biped->object.up.j, biped->object.up.k));
	}

	return;
}

void biped_stop_melee_attack(
	long biped_index)
{
	struct biped_datum *biped = biped_try_and_get(biped_index);

	if (biped)
	{
		biped->biped.player_melee_ticks = 0;
	}

	return;
}

void biped_start_limp_body_physics(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	boolean uses_limp_body_physics = TEST_FLAG(biped_definition_get(biped->definition_index)->biped.flags, _biped_uses_limp_body_physics_bit);

	if (!debug_biped_limp_body_disable &&
		uses_limp_body_physics &&
		TEST_FLAG(biped->object.flags, _object_at_rest_bit) &&
		!TEST_FLAG(biped->biped.flags, _biped_airborne_bit) &&
		!TEST_FLAG(biped->biped.flags, _biped_limp_body_physics_active_bit))
	{
		biped->biped.limp_body_current_relaxation_iterations = 0;
		biped->biped.limp_body_max_relaxation_iterations = biped_limp_noodle_get_max_relaxation_iterations(biped_index);
		SET_FLAG(biped->object.flags, _object_do_not_recompute_node_matrices_bit, TRUE);
		SET_FLAG(biped->biped.flags, _biped_limp_body_physics_active_bit, TRUE);
	}

	return;
}

void biped_stop_limp_body_physics(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);

	if (TEST_FLAG(biped_definition_get(biped->definition_index)->biped.flags, _biped_uses_limp_body_physics_bit) &&
		TEST_FLAG(biped->biped.flags, _biped_limp_body_physics_active_bit))
	{
		SET_FLAG(biped->object.flags, _object_do_not_recompute_node_matrices_bit, FALSE);
		SET_FLAG(biped->biped.flags, _biped_limp_body_physics_active_bit, FALSE);
	}

	return;
}

void biped_exit_seat_end(
	long biped_index,
	long old_vehicle_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	biped->object.forward.k = 0.f;
	if (normalize3d(&biped->object.forward) == 0.f)
	{
		biped->object.forward = *global_forward3d;
	}
	biped->object.up = *global_up3d;
	SET_FLAG(biped->biped.flags, _biped_airborne_bit, TRUE);

	if (!biped_fix_position(biped_index, old_vehicle_index, NULL, NULL, 2.f, TRUE, FALSE, TRUE))
	{
		real_point3d new_position_please_dear_god_work;
		real radius;

		object_get_bounding_sphere(old_vehicle_index, &new_position_please_dear_god_work, &radius);
		if (!biped_fix_position(biped_index, old_vehicle_index, &new_position_please_dear_god_work, NULL, radius, TRUE, FALSE, FALSE))
		{
			error(_error_silent, "couldn't teleport the biped out far enough from the vehicle...");
		}
	}

	return;
}

static void biped_update_turning(
	long biped_index,
	struct unit_animation_update_data *animation)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);

	if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit) && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
	{
		real_vector3d final_facing_vector;
		real_rectangle2d facing_bounds;
		real angular_velocity_limit;
		real angular_acceleration_limit;
		boolean exact_facing = FALSE;
		real desired_bank;
		real bank_fraction;
		real bank_time;

		if (magnitude_squared3d(&biped->object.translational_velocity) < (0.5f / TICKS_PER_SECOND) * (0.5f / TICKS_PER_SECOND) &&
			magnitude_squared3d(&biped->object.angular_velocity) < (DEGREES_TO_RADIANS(2) / TICKS_PER_SECOND) * (DEGREES_TO_RADIANS(2) / TICKS_PER_SECOND) &&
			magnitude_squared3d(&biped->unit.throttle) < 0.1f * 0.1f)
		{
			real threshold = TEST_FLAG(biped->unit.control_flags, _unit_control_exact_facing_bit) ? 0.99f : biped_definition->biped.runtime_cosine_stationary_turning_threshold;

			exact_facing = dot_product3d(&biped->unit.desired_facing_vector, &biped->object.forward) > threshold;
		}

		if (exact_facing)
		{
			final_facing_vector = biped->object.forward;
		}
		else
		{
			real pitch = biped_definition->biped.flying_pitch_amount * biped->unit.throttle.k;

			final_facing_vector = biped->unit.desired_facing_vector;
			if (pitch != 0.f)
			{
				final_facing_vector.k += pitch;
				if (normalize3d(&final_facing_vector) == 0.f)
				{
					final_facing_vector = biped->unit.desired_facing_vector;
				}
			}
		}

		{
			real bank_scale;
			real_vector3d right_vector;

			cross_product3d(&biped->object.forward, &biped->object.up, &right_vector);
			bank_scale = dot_product3d(&right_vector, &biped->unit.desired_facing_vector) / 0.3f;
			desired_bank = biped->unit.throttle.i * bank_scale - biped->unit.throttle.j;
		}
		desired_bank = MIN(desired_bank, 1.5f) * biped_definition->biped.flying_bank_amount;

		if (desired_bank * biped->biped.bank <= 0.f)
		{
			bank_fraction = 1.f;
		}
		else
		{
			bank_fraction = 1.f - MIN(biped->biped.bank / desired_bank, 1.f);
		}

		bank_time = bank_fraction * biped_definition->biped.flying_bank_apply_time + (1.f - bank_fraction) * biped_definition->biped.flying_bank_decay_time;
		if (bank_time > 0.f)
		{
			desired_bank = (desired_bank - biped->biped.bank) / (TICKS_PER_SECOND * bank_time) + biped->biped.bank;
		}
		biped->biped.bank = desired_bank;

		facing_bounds.x0 = -_pi;
		facing_bounds.x1 = _pi;
		facing_bounds.y0 = -_pi / 2.f;
		facing_bounds.y1 = _pi / 2.f;
		angular_velocity_limit = biped_definition->biped.flying_angular_velocity / TICKS_PER_SECOND;
		angular_acceleration_limit = biped_definition->biped.flying_angular_acceleration / (TICKS_PER_SECOND * TICKS_PER_SECOND);
		if (angular_acceleration_limit == 0.f)
		{
			biped->object.forward = final_facing_vector;
		}
		else
		{
			unit_euler_aiming_update(NULL, &biped->object.forward, &final_facing_vector, &biped->object.angular_velocity, &facing_bounds, angular_velocity_limit, angular_acceleration_limit);
		}

		biped_snap_facing(biped_index);
		biped_verify_object_vectors(biped_index, "post-fly-turn");
	}
	else
	{
		boolean flaming = FALSE;

		if (biped->unit.animation.base_seat_index != _unit_base_seat_asleep)
		{
			real_vector3d desired_facing_vector;
			real facing_cross;
			real facing_dot;
			boolean turn_right;

			if (biped->unit.animation.base_seat_index == _unit_base_seat_flaming)
			{
				flaming = TRUE;
			}

			if (TEST_FLAG(biped_definition->biped.flags, _biped_climbs_anything_bit))
			{
				real_vector3d desired_left_vector;
				real_vector3d cross_vector;

				cross_product3d(&biped->object.up, &biped->unit.desired_facing_vector, &desired_left_vector);
				cross_product3d(&desired_left_vector, &biped->object.up, &desired_facing_vector);
				if (normalize3d(&desired_facing_vector) == 0.f)
				{
					desired_facing_vector = biped->object.forward;
				}
				cross_product3d(&desired_facing_vector, &biped->object.forward, &cross_vector);
				facing_cross = dot_product3d(&cross_vector, &biped->object.up);
				facing_dot = dot_product3d(&desired_facing_vector, &biped->object.forward);
			}
			else
			{
				desired_facing_vector = biped->unit.desired_facing_vector;
				desired_facing_vector.k = 0.f;
				if (normalize2d((real_vector2d *)&desired_facing_vector) == 0.f)
				{
					desired_facing_vector = biped->object.forward;
				}
				facing_cross = cross_product2d((real_vector2d *)&desired_facing_vector, (real_vector2d *)&biped->object.forward);
				facing_dot = dot_product2d((real_vector2d *)&desired_facing_vector, (real_vector2d *)&biped->object.forward);
			}

			turn_right = facing_cross > 0.f;
			if (facing_dot < -0.9f)
			{
				if (biped->unit.animation.state == _unit_state_turn_right)
				{
					turn_right = TRUE;
				}
				else if (biped->unit.animation.state == _unit_state_turn_left)
				{
					turn_right = FALSE;
				}
			}

			if (biped->biped.state == _biped_state_moving || TEST_FLAG(biped_definition->biped.flags, _biped_turns_without_animating_bit))
			{
				if (!TEST_FLAG(biped->unit.control_flags, _unit_control_look_dont_turn_bit))
				{
					real angle = biped_definition->biped.moving_turning_speed / TICKS_PER_SECOND;
					real angle_cosine = cosine(angle);
					real angle_sine = sine(angle);
					real remaining;

					if (turn_right)
					{
						angle_sine = -angle_sine;
					}

					if (TEST_FLAG(biped_definition->biped.flags, _biped_climbs_anything_bit))
					{
						real_vector3d cross_vector;

						rotate_vector_about_axis(&biped->object.forward, &biped->object.up, angle_sine, angle_cosine);
						cross_product3d(&desired_facing_vector, &biped->object.forward, &cross_vector);
						remaining = dot_product3d(&cross_vector, &biped->object.up);
					}
					else
					{
						rotate_vector2d((real_vector2d *)&biped->object.forward, angle_sine, angle_cosine, (real_vector2d *)&biped->object.forward);
						remaining = cross_product2d((real_vector2d *)&desired_facing_vector, (real_vector2d *)&biped->object.forward);
					}

					if ((turn_right && remaining < 0.f) || (!turn_right && remaining > 0.f))
					{
						if (TEST_FLAG(biped_definition->biped.flags, _biped_climbs_anything_bit))
						{
							real_vector3d left_vector;

							cross_product3d(&biped->object.up, &desired_facing_vector, &left_vector);
							if (normalize3d(&left_vector) > 0.f)
							{
								cross_product3d(&left_vector, &biped->object.up, &biped->object.forward);
							}
						}
						else
						{
							set_real_vector3d(&biped->object.forward, desired_facing_vector.i, desired_facing_vector.j, 0.f);
							biped->object.up = *global_up3d;
						}
						normalize3d(&biped->object.forward);
					}

					biped_verify_object_vectors(biped_index, "post-moving-turn");
				}
			}
			else if (biped->biped.state == _biped_state_idle)
			{
				if (!flaming &&
					!TEST_FLAG(biped->unit.flags, _unit_aim_without_turning_bit) &&
					!TEST_FLAG(biped->unit.control_flags, _unit_control_look_dont_turn_bit))
				{
					real threshold = TEST_FLAG(biped->unit.control_flags, _unit_control_exact_facing_bit) ? 0.99f : biped_definition->biped.runtime_cosine_stationary_turning_threshold;

					if (facing_dot < threshold && !TEST_FLAG(biped_definition->unit.flags, _unit_is_special_bit))
					{
						animation->state_desired = turn_right ? _unit_state_turn_right : _unit_state_turn_left;
					}

					biped_verify_object_vectors(biped_index, "post-standing-turn");
				}
			}
		}
	}

	return;
}

static void biped_update_moving(
	long biped_index,
	struct unit_animation_update_data *animation)
{
	struct biped_datum *biped = biped_get(biped_index);

	if (!TEST_FLAG(biped->object.flags, _object_at_rest_bit) ||
		!TEST_FLAG(biped->object.damage_flags, _object_dead_bit) ||
		!TEST_FLAG(biped->unit.animation.flags, _unit_animation_ignore_translation_bit))
	{
		struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);
		struct biped_physics physics;
		real crouch_velocity;

		physics.biped_index = biped_index;
		physics.in_flags = 0;
		physics.forward = biped->object.forward;
		if (TEST_FLAG(biped_definition->unit.flags, _unit_simple_creature_bit))
		{
			physics.aiming = biped->object.forward;
		}
		else
		{
			unit_get_aiming_vector(biped_index, &physics.aiming);
		}
		physics.velocity = biped->object.translational_velocity;
		physics.crouch_velocity = 0.f;
		physics.acceleration_maximum = 0.0053333333f;
		physics.airborne_acceleration_maximum = 0.f;
		biped_get_physics_pill(biped_index, &physics.position, &physics.height, &physics.width);
		physics.minimum_normal_k = biped_definition->biped.runtime_minimum_normal_k;
		physics.downhill_k0 = biped_definition->biped.runtime_downhill_k0;
		physics.downhill_k1 = biped_definition->biped.runtime_downhill_k1;
		physics.downhill_velocity_scale = biped_definition->biped.downhill_velocity_scale;
		physics.uphill_k0 = biped_definition->biped.runtime_uphill_k0;
		physics.uphill_k1 = biped_definition->biped.runtime_uphill_k1;
		physics.uphill_velocity_scale = biped_definition->biped.uphill_velocity_scale;
		physics.ground_plane = biped->biped.ground_plane;
		physics.existing_support_surface_index = biped->biped.support_surface_index;
		physics.ground_tangential_velocity_max = REAL_MAX;
		physics.ground_tangential_angle = 0.f;

		if (biped->biped.landing == _biped_landing_hard)
		{
			set_real_vector3d(&physics.movement_desired, 0.f, 0.f, 0.f);
			physics.movement_penalty = 1.f;
		}
		else
		{
			real random_speed_scale = 1.f;

			if (TEST_FLAG(biped_definition->biped.flags, _biped_random_speed_increase_bit) && !biped->unit.aiming_speed)
			{
				real infection_form_toughness = game_difficulty_get_value(_game_difficulty_infection_form_toughness);

				random_speed_scale = 1.f + ((real)((unsigned long)biped_index % 137) / 137.f) * infection_form_toughness;
			}

			if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit) && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				animation->state_desired = _unit_state_idle;
			}
			else if (biped->unit.throttle.i != 0.f || biped->unit.throttle.j != 0.f || biped->unit.throttle.k != 0.f)
			{
				boolean stunned = biped->unit.body_stun > 0.2f;

				if (biped_definition->unit.stunned_movement_threshold > 0.f && biped->object.recent_body_damage > biped_definition->unit.stunned_movement_threshold)
				{
					stunned = TRUE;
				}

				if (fabs(biped->unit.throttle.i) < fabs(biped->unit.throttle.j))
				{
					if (biped->unit.throttle.j < 0.f)
					{
						animation->state_desired = stunned ? _unit_state_stunned_move_right : _unit_state_move_right;
					}
					else
					{
						animation->state_desired = stunned ? _unit_state_stunned_move_left : _unit_state_move_left;
					}
				}
				else if (biped->unit.throttle.i < 0.f)
				{
					animation->state_desired = stunned ? _unit_state_stunned_move_back : _unit_state_move_back;
				}
				else
				{
					animation->state_desired = stunned ? _unit_state_stunned_move_front : _unit_state_move_front;
				}
			}

			physics.movement_desired = *global_zero_vector3d;
			if (biped->object.animation.state.index != NONE &&
				(TEST_FLAG(biped->object.damage_flags, _object_dead_bit) || !TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit)) &&
				!TEST_FLAG(biped->unit.animation.flags, _unit_animation_ignore_translation_bit))
			{
				struct model *model_definition = model_definition_get(biped_definition->object.model.index);
				struct animation_graph *animation_graph = animation_graph_definition_get(biped->object.animation.animation_graph_index);
				struct animation *animation_definition = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, biped->object.animation.state.index, struct animation);
				real yaw = 0.f;

				switch (animation_definition->frame_info_type)
				{
				case _animation_frame_info_xy_translation:
				{
					real *frame_info = animation_get_frame_info(animation_definition, biped->object.animation.state.frame_index, 2 * sizeof(real));

					physics.movement_desired.i = frame_info[0];
					physics.movement_desired.j = frame_info[1];
					break;
				}
				case _animation_frame_info_xy_translation_yaw_rotation:
				{
					real *frame_info = animation_get_frame_info(animation_definition, biped->object.animation.state.frame_index, 3 * sizeof(real));

					physics.movement_desired.i = frame_info[0];
					physics.movement_desired.j = frame_info[1];
					yaw = frame_info[2];
					break;
				}
				case _animation_frame_info_xyz_translation_yaw_rotation:
				{
					real *frame_info = animation_get_frame_info(animation_definition, biped->object.animation.state.frame_index, 4 * sizeof(real));

					physics.movement_desired.i = frame_info[0];
					physics.movement_desired.j = frame_info[1];
					physics.movement_desired.k = frame_info[2];
					yaw = frame_info[3];
					break;
				}
				}

				scale_vector3d(&physics.movement_desired, random_speed_scale, &physics.movement_desired);

				if (!realcmp(yaw, 0.f))
				{
					real_vector3d new_forward = biped->object.forward;

					rotate_vector_about_axis(&new_forward, &biped->object.up, sine(yaw), cosine(yaw));
					if (TEST_FLAG(biped->unit.control_flags, _unit_control_exact_facing_bit) &&
						(biped->unit.animation.state == _unit_state_turn_left || biped->unit.animation.state == _unit_state_turn_right) &&
						dot_product3d(&biped->unit.desired_facing_vector, &biped->object.forward) > 0.5f)
					{
						real_vector3d previous_cross_desired;
						real_vector3d current_cross_desired;

						cross_product3d(&biped->unit.desired_facing_vector, &biped->object.forward, &previous_cross_desired);
						cross_product3d(&biped->unit.desired_facing_vector, &new_forward, &current_cross_desired);
						if (dot_product3d(&previous_cross_desired, &biped->object.up) * dot_product3d(&current_cross_desired, &biped->object.up) <= 0.f)
						{
							new_forward = biped->unit.desired_facing_vector;
							unit_abort_animation(biped_index);
						}
					}
					biped->object.forward = new_forward;
					biped_snap_facing(biped_index);
				}
			}

			if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit) && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				real throttle_magnitude = MIN(1.f, magnitude3d(&biped->unit.throttle));
				real deceleration_fraction = 1.f - throttle_magnitude;
				real crouch_scale = 1.f;
				real acceleration;

				if (biped_definition->biped.flying_crouch_velocity_modifier > 0.f)
				{
					if (biped->biped.crouch == 1.f)
					{
						crouch_scale = biped_definition->biped.flying_crouch_velocity_modifier;
					}
					else if (biped->biped.crouch > 0.f)
					{
						crouch_scale = (biped_definition->biped.flying_crouch_velocity_modifier - 1.f) * biped->biped.crouch + 1.f;
					}
				}

				physics.movement_desired.i = crouch_scale * random_speed_scale * biped_definition->biped.flying_velocity;
				physics.movement_desired.j = crouch_scale * random_speed_scale * biped_definition->biped.flying_sidestep_velocity;
				physics.movement_desired.k = crouch_scale * random_speed_scale * biped_definition->biped.flying_sidestep_velocity;
				acceleration = (1.f - deceleration_fraction) * biped_definition->biped.flying_acceleration + deceleration_fraction * biped_definition->biped.flying_deceleration;
				acceleration = crouch_scale * random_speed_scale * acceleration;
				physics.movement_desired.i = physics.movement_desired.i * biped->unit.throttle.i / TICKS_PER_SECOND;
				physics.movement_desired.j = physics.movement_desired.j * biped->unit.throttle.j / TICKS_PER_SECOND;
				physics.movement_desired.k = physics.movement_desired.k * biped->unit.throttle.k / TICKS_PER_SECOND;
				physics.acceleration_maximum = acceleration / TICKS_PER_SECOND;
				physics.airborne_acceleration_maximum = physics.acceleration_maximum;
			}
			else if (TEST_FLAG(biped_definition->biped.flags, _biped_uses_player_physics_bit) && biped->unit.animation.state != _unit_state_user_animation)
			{
				struct game_globals_player_information *player_information = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information);
				struct game_globals_player_information player_information_block;
				real speed_scale = 1.f;

				if (cinematic_in_progress() || TEST_FLAG(biped_definition->biped.flags, _biped_uses_old_player_physics_bit))
				{
					player_information_block = *player_information;
					player_information = &player_information_block;
					player_information_block.walking_speed = 0.512f;
					player_information_block.run_forward_speed = 2.25f;
					player_information_block.run_backward_speed = 2.f;
					player_information_block.run_sideways_speed = 2.f;
					player_information_block.run_acceleration = 0.32f;
				}

				if (biped->unit.player_index != NONE)
				{
					struct player_datum *player = player_get(biped->unit.player_index);

					speed_scale = player->multiplayer.speed_multiplier;
				}
				speed_scale = (1.f - player_information->stun_movement_penalty * biped->unit.body_stun) * speed_scale * random_speed_scale;

				if (biped->unit.throttle.i > 0.f)
				{
					physics.movement_desired.i = (1.f - biped->biped.crouch) * player_information->run_forward_speed + player_information->sneak_forward_speed * biped->biped.crouch;
				}
				else
				{
					physics.movement_desired.i = (1.f - biped->biped.crouch) * player_information->run_backward_speed + player_information->sneak_backward_speed * biped->biped.crouch;
				}
				physics.movement_desired.j = (1.f - biped->biped.crouch) * player_information->run_sideways_speed + player_information->sneak_sideways_speed * biped->biped.crouch;
				physics.movement_desired.k = 0.f;
				physics.acceleration_maximum = (1.f - biped->biped.crouch) * player_information->run_acceleration + player_information->sneak_acceleration * biped->biped.crouch;

				if (biped->unit.animation.base_seat_index == _unit_base_seat_alert)
				{
					physics.movement_desired.i = biped->unit.throttle.i > 0.f ? player_information->walking_speed : 0.f;
					physics.movement_desired.j = 0.f;
				}

				physics.movement_desired.i = speed_scale * biped->unit.throttle.i / TICKS_PER_SECOND * physics.movement_desired.i;
				physics.movement_desired.j = speed_scale * biped->unit.throttle.j / TICKS_PER_SECOND * physics.movement_desired.j;
				physics.acceleration_maximum /= TICKS_PER_SECOND;
				physics.airborne_acceleration_maximum = player_information->airborne_acceleration / TICKS_PER_SECOND;

				if (!TEST_FLAG(biped->unit.control_flags, _unit_control_look_dont_turn_bit))
				{
					physics.forward = biped->unit.desired_facing_vector;
				}

				if (game_players_are_double_speed())
				{
					scale_vector3d(&physics.movement_desired, player_information->double_speed_multiplier, &physics.movement_desired);
				}
			}
			else if (!TEST_FLAG(biped->biped.flags, _biped_slipping_bit) && !TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
			{
				biped->object.translational_velocity.i = physics.movement_desired.i;
				biped->object.translational_velocity.j = physics.movement_desired.j;
				physics.acceleration_maximum = REAL_MAX;
			}

			if (TEST_FLAG(biped->biped.flags, _biped_airborne_bit) &&
				biped->biped.airborne_ticks < 22 &&
				biped->unit.actor_index != NONE &&
				actor_is_leaping(biped->unit.actor_index))
			{
				physics.ground_tangential_velocity_max = 0.1f;
				physics.ground_tangential_angle = 0.5f;
			}

			physics.movement_penalty = 0.f;
		}

		if (biped->unit.animation.base_seat_index == _unit_base_seat_crouch)
		{
			crouch_velocity = 1.f - biped->biped.crouch;
			if (crouch_velocity > biped_definition->biped.runtime_crouch_transition_velocity)
			{
				biped->biped.crouch += biped_definition->biped.runtime_crouch_transition_velocity;
				crouch_velocity = biped_definition->biped.runtime_crouch_transition_velocity;
			}
			else
			{
				biped->biped.crouch = 1.f;
			}
		}
		else
		{
			crouch_velocity = -biped->biped.crouch;
			if (crouch_velocity < -biped_definition->biped.runtime_crouch_transition_velocity)
			{
				biped->biped.crouch -= biped_definition->biped.runtime_crouch_transition_velocity;
				crouch_velocity = -biped_definition->biped.runtime_crouch_transition_velocity;
			}
			else
			{
				biped->biped.crouch = 0.f;
			}
		}

		if (fabs(crouch_velocity) > 0.01f && TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
		{
			physics.crouch_velocity = (biped_definition->biped.collision_height_standing - biped_definition->biped.collision_height_crouching) * crouch_velocity;
		}

		if (biped->biped.crouch != 0.f)
		{
			physics.in_flags |= FLAG(_biped_physics_in_crouched_bit);
			if (!animation->crouching)
			{
				physics.in_flags |= FLAG(_biped_physics_in_trying_to_stand_bit);
			}
		}
		if (TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_airborne_bit);
		}
		if (TEST_FLAG(biped->biped.flags, _biped_slipping_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_slipping_bit);
		}
		if (TEST_FLAG(biped->biped.flags, _biped_absolute_movement_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_absolute_movement_bit);
		}
		if (TEST_FLAG(biped->biped.flags, _biped_no_collision_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_no_collision_bit);
		}
		if (TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_dead_bit);
		}
		if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit) && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_flying_bit);
		}
		if (TEST_FLAG(biped_definition->biped.flags, _biped_passes_through_bipeds_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_pass_through_bipeds_bit);
		}
		if (TEST_FLAG(biped_definition->biped.flags, _biped_climbs_anything_bit) && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
		{
			physics.in_flags |= FLAG(_biped_physics_in_climb_anything_bit);
		}

		biped_update_physics(&physics);

		if (physics.elevator_object_index != NONE)
		{
			biped->biped.elevator_ticks = 60;
			biped->biped.elevator_object_index = physics.elevator_object_index;
		}
		else if (biped->biped.elevator_ticks > 0)
		{
			biped->biped.elevator_ticks--;
		}
		else
		{
			biped->biped.elevator_object_index = NONE;
		}

		if (TEST_FLAG(biped->unit.flags, _unit_suspended_bit))
		{
			physics.new_velocity = *global_zero_vector3d;
			physics.velocity = physics.new_velocity;
			physics.new_position = physics.position;
			physics.out_flags &= ~FLAG(_biped_physics_out_airborne_bit);
		}

		{
			real_point3d new_biped_position = physics.new_position;

			if (!TEST_FLAG(biped_definition->biped.flags, _biped_pill_centered_at_origin_bit))
			{
				new_biped_position.z -= physics.width;
			}
			object_translate(biped_index, &new_biped_position, NULL);
			biped->object.translational_velocity = physics.new_velocity;
			biped->biped.support_surface_index = physics.support_surface_index;
			biped->biped.pathfinding_surface_index = NONE;
			biped->biped.pathfinding_point = new_biped_position;
		}

		if (!animation->crouching && TEST_FLAG(physics.out_flags, _biped_physics_out_cannot_stand_bit))
		{
			animation->crouching = TRUE;
		}

		SET_FLAG(biped->biped.flags, _biped_airborne_bit, TEST_FLAG(physics.out_flags, _biped_physics_out_airborne_bit));
		SET_FLAG(biped->biped.flags, _biped_slipping_bit, TEST_FLAG(physics.out_flags, _biped_physics_out_slipping_bit));
		SET_FLAG(biped->biped.flags, _biped_movement_passes_through_bipeds_bit, TEST_FLAG(biped_definition->biped.flags, _biped_passes_through_bipeds_bit));
		biped->biped.ground_plane = physics.ground_plane;

		if (physics.landing_velocity > 0.f)
		{
			biped_start_landing(biped_index, physics.landing_velocity);
		}

		if (!TEST_FLAG(physics.out_flags, _biped_physics_out_cannot_stand_bit))
		{
			biped_update_jumping(biped_index, animation);
		}

		if (biped->unit.melee_attack_state == 3 && biped->biped.impact_target_object_index != NONE)
		{
			struct object_datum *target = object_get(biped->biped.impact_target_object_index);
			real_vector3d movement_vector;
			struct collision_model_instance instance;
			struct collision_model_test_vector_result vector_result;
			struct collision_result collision;

			match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\bipeds.c", 2170, _collision_user_melee);

			vector_from_points3d(&physics.position, &physics.new_position, &movement_vector);
			if (fast_vector_intersects_sphere(&physics.position, &movement_vector, &target->object.bounding_sphere_center, (real)(target->object.bounding_sphere_radius)) &&
				collision_model_instance_new(&instance, biped->biped.impact_target_object_index) &&
				collision_model_test_vector(&instance, FLAG(_collision_bsp_test_front_facing_surfaces_bit) | FLAG(_collision_bsp_test_back_facing_surfaces_bit), &physics.position, &movement_vector, &vector_result) &&
				!collision_test_vector(_collision_test_for_bipeds_passthrough_living_flags, &physics.position, &movement_vector, biped_index, &collision))
			{
				real_point3d collision_point;
				real_plane3d collision_plane;

				point_from_line3d(&physics.position, &movement_vector, vector_result.bsp_result.t, &collision_point);
				matrix4x3_transform_plane(&instance.matrices[vector_result.node_index], vector_result.bsp_result.plane, &collision_plane);
				if (vector_result.bsp_result.plane_designator & LONG_MIN)
				{
					plane3d_negate(&collision_plane, &collision_plane);
				}
				unit_impact_melee_damage(biped_index, biped->biped.impact_target_object_index, vector_result.node_index, vector_result.region_index, vector_result.bsp_result.material_index, &collision_point, &collision_plane.n, &collision.location);
			}

			match_collision_log_end_user("c:\\halo\\SOURCE\\units\\bipeds.c", 2213);
		}

		biped_bumped_object(biped_index, physics.bumped_object_index, &physics.velocity);
		biped_falling_damage(biped_index, physics.landing_velocity);

		if (TEST_FLAG(physics.in_flags, _biped_physics_in_flying_bit) || TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
		{
			SET_FLAG(biped->object.flags, _object_on_ground_bit, FALSE);
		}
		else
		{
			SET_FLAG(biped->object.flags, _object_on_ground_bit, TRUE);
		}

		if (TEST_FLAG(physics.in_flags, _biped_physics_in_flying_bit) ||
			TEST_FLAG(biped->biped.flags, _biped_airborne_bit) ||
			TEST_FLAG(physics.out_flags, _biped_physics_out_volatile_collision_bit) ||
			!(magnitude_squared3d(&biped->object.translational_velocity) < _real_epsilon))
		{
			SET_FLAG(biped->object.flags, _object_at_rest_bit, FALSE);
		}
		else
		{
			SET_FLAG(biped->object.flags, _object_at_rest_bit, TRUE);
		}

		if (TEST_FLAG(biped->object.flags, _object_on_ground_bit))
		{
			biped->object.angular_velocity = *global_zero_vector3d;
		}
	}

	return;
}

static void biped_bumped_object(
	long biped_index,
	long object_index,
	real_vector3d const *old_velocity)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (biped->biped.bump_ticks < 0)
	{
		if (object_index == NONE)
		{
			biped->biped.bump_ticks++;
		}
		else
		{
			biped->biped.bump_ticks = -15;
		}
	}
	else if (object_index != NONE)
	{
		struct object_datum *object = object_get(object_index);

		ai_handle_bump(biped_index, object_index, old_velocity);
		if (biped->unit.player_index != NONE || recorded_animation_controlling_unit(biped_index))
		{
			if (biped->biped.bump_object_index != object_index)
			{
				biped->biped.bump_object_index = object_index;
				biped->biped.bump_ticks = 0;
			}
			else
			{
				biped->biped.bump_ticks++;
				if (biped->biped.bump_ticks > 3)
				{
					switch (object->object.type)
					{
					case _object_type_biped:
						if (cheat.bump_possession)
						{
							short local_player_index = unit_get_local_player_index(biped_index);

							if (local_player_index != NONE)
							{
								struct biped_datum *bumped_biped = (struct biped_datum *)object;

								bumped_biped->biped.bump_ticks = -15;
								players_set_local_player_unit(local_player_index, object_index);
							}
						}
						break;
					}
					biped->biped.bump_ticks = -15;
				}
			}
		}
	}

	return;
}

static void biped_falling_danger(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (!TEST_FLAG(biped->object.damage_flags, _object_dead_bit) &&
		!TEST_FLAG(definition->biped.flags, _biped_flying_bit) &&
		!TEST_FLAG(definition->biped.flags, _biped_immune_to_falling_damage_bit) &&
		!TEST_FLAG(biped->unit.flags, _unit_no_falling_damage_bit) &&
		biped->unit.actor_index != NONE &&
		biped->unit.animation.state != _unit_state_ai_impulse)
	{
		long game_time = game_time_get();

		if (biped->biped.airborne_ticks > 30)
		{
			if (biped->biped.last_falling_communication_time == NONE || biped->biped.last_falling_communication_time + 15 < game_time)
			{
				struct game_globals_falling_damage *falling_damage = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->falling_damage, 0, struct game_globals_falling_damage);
				boolean scream = TRUE;
				real_point3d ground_point;
				real_point3d biped_position;

				biped->biped.last_falling_communication_time = game_time;
				if (biped_find_ground_surface(biped_index, 6.f, global_down3d, &ground_point, NULL) != NONE)
				{
					object_get_origin(biped_index, &biped_position);
					if (!(biped->object.translational_velocity.k > 0.f))
					{
						if (biped->object.translational_velocity.k * biped->object.translational_velocity.k + 2.f * global_gravity * (biped_position.z - ground_point.z) < falling_damage->runtime_maximum_damage_velocity * falling_damage->runtime_maximum_damage_velocity)
						{
							scream = FALSE;
						}
					}
					else
					{
						scream = FALSE;
					}
				}

				if (scream)
				{
					unit_scream(biped_index, _unit_scream_falling);
				}
			}
		}
	}

	return;
}

static void biped_vehicle_speech(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct vehicle_datum *vehicle = vehicle_get(biped->object.parent_object_index);
	struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

	if (TEST_FLAG(vehicle_definition->unit.flags, _unit_causes_passenger_dialogue_bit) &&
		biped->unit.actor_index != NONE &&
		biped->unit.animation.state != _unit_state_ai_impulse &&
		biped->unit.gunner_inactive_ticks > 120)
	{
		long game_time = game_time_get();

		if (vehicle->vehicle.airborne_ticks > 30)
		{
			if (biped->biped.last_falling_communication_time == NONE || biped->biped.last_falling_communication_time + 15 < game_time)
			{
				boolean falling = FALSE;
				real_vector3d approximate_direction_vector;
				real_vector3d ground_surface_normal;

				biped->biped.last_falling_communication_time = game_time;
				if (biped_find_ground_surface(biped_index, 8.f, global_down3d, NULL, NULL) == NONE)
				{
					falling = TRUE;
					scale_vector3d(&vehicle->object.translational_velocity, 60.f, &approximate_direction_vector);
					approximate_direction_vector.k -= 1800.f * global_gravity;
					if (normalize3d(&approximate_direction_vector) > 0.f &&
						biped_find_ground_surface(biped_index, 8.f, &approximate_direction_vector, NULL, &ground_surface_normal) != NONE &&
						ground_surface_normal.k > 0.3f)
					{
						falling = FALSE;
					}
				}

				if (falling)
				{
					ai_communication_event(_ai_communication_vehicle_falling, biped_index, NONE, NONE, NONE, NONE, NULL);
				}
				else if (vehicle->object.up.k > 0.6f && magnitude3d(&vehicle->object.angular_velocity) < DEGREES_TO_RADIANS(3))
				{
					ai_communication_event(_ai_communication_vehicle_woohoo, biped_index, NONE, NONE, NONE, NONE, NULL);
				}
				else
				{
					ai_communication_event(_ai_communication_vehicle_scared, biped_index, NONE, NONE, NONE, NONE, NULL);
				}
			}
		}
	}

	return;
}

static boolean biped_check_discard(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);

	if (!game_engine_running() &&
		(TEST_FLAG(biped->object.flags, _object_outside_of_map_bit) || biped->object.location.cluster_index == NONE) &&
		biped->object.position.z < -2000.f)
	{
		error(_error_silent, "WARNING: biped %s (%s) is in a bad place (%.1f %.1f %.1f), erasing",
			tag_name_strip_path(tag_get_name(biped->definition_index)),
			biped->object.position.x,
			biped->object.position.y,
			biped->object.position.z,
			ai_debug_describe_actor(biped->unit.swarm_actor_index == NONE ? biped->unit.actor_index : biped->unit.swarm_actor_index, biped_index, TRUE, temporary, 256));
		object_delete(biped_index);
	}

	return FALSE;
}

static void biped_falling_damage(
	long biped_index,
	real collision_velocity)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);
	struct game_globals_falling_damage *falling_damage = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->falling_damage, 0, struct game_globals_falling_damage);
	boolean immune = TEST_FLAG(biped->unit.flags, _unit_no_falling_damage_bit) || TEST_FLAG(definition->biped.flags, _biped_immune_to_falling_damage_bit);
	real vertical_velocity = biped->object.translational_velocity.k;

	if (!cheat.jetpack || biped->unit.player_index == NONE)
	{
		if (collision_velocity > falling_damage->runtime_minimum_damage_velocity)
		{
			if (!immune)
			{
				struct damage_data damage_data;

				damage_data_new(&damage_data, falling_damage->falling_damage.index);
				damage_data.scale = (collision_velocity - falling_damage->runtime_minimum_damage_velocity) / (falling_damage->runtime_maximum_damage_velocity - falling_damage->runtime_minimum_damage_velocity);
				damage_data.scale = PIN(damage_data.scale, 0.f, 1.f);
				object_cause_damage(&damage_data, biped_index, NONE, NONE, NONE, NULL);
			}
		}
		else if (!TEST_FLAG(definition->biped.flags, _biped_flying_bit) && -falling_damage->runtime_maximum_falling_velocity > vertical_velocity)
		{
			if (!immune && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				struct damage_data damage_data;

				damage_data_new(&damage_data, falling_damage->maximum_distance_damage.index);
				object_cause_damage(&damage_data, biped_index, NONE, NONE, NONE, NULL);
			}

			if (!game_engine_running() && TEST_FLAG(biped->object.flags, _object_outside_of_map_bit) && player_index_from_unit_index(biped_index) == NONE)
			{
				error(_error_silent, "WARNING: biped %s (%s) fell outside world and was erased",
					tag_name_strip_path(tag_get_name(biped->definition_index)),
					ai_debug_describe_actor(biped->unit.swarm_actor_index == NONE ? biped->unit.actor_index : biped->unit.swarm_actor_index, biped_index, TRUE, temporary, 256));
				object_delete(biped_index);
			}
		}
	}

	return;
}

static void biped_apply_rotation(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	real_vector3d axis = biped->object.angular_velocity;
	real angle = normalize3d(&axis);
	real angle_cosine = cosine(angle);
	real angle_sine = sine(angle);

	rotate_vector_about_axis(&biped->object.forward, &axis, angle_sine, angle_cosine);
	normalize3d(&biped->object.forward);
	{
		real_vector3d new_up;
		real_vector3d new_right;

		new_up = biped->object.up;
		rotate_vector_about_axis(&new_up, &axis, angle_sine, angle_cosine);
		cross_product3d(&biped->object.forward, &new_up, &new_right);
		cross_product3d(&new_right, &biped->object.forward, &biped->object.up);
	}
	if (normalize3d(&biped->object.up) == 0.f)
	{
		biped->object.forward = *global_forward3d;
		biped->object.up = *global_up3d;
	}

	return;
}

static void biped_update_dead(
	long biped_index,
	struct unit_animation_update_data *animation)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (TEST_FLAG(biped->biped.flags, _biped_limp_body_physics_active_bit) &&
		biped->biped.limp_body_current_relaxation_iterations < biped->biped.limp_body_max_relaxation_iterations)
	{
		if (!debug_biped_limp_body_disable)
		{
			biped_limp_noodle_relax_nodes_onto_environment(biped_index);
		}
		biped_verify_object_vectors(biped_index, "post-limp-noodle");
	}
	else if (biped->biped.airborne_ticks < 3 || TEST_FLAG(definition->biped.flags, _biped_has_no_dying_airborne_bit))
	{
		if (biped->unit.animation.state == _unit_state_dying_airborne)
		{
			biped->biped.bank = 0.f;
			biped_snap_facing(biped_index);
		}
		animation->state_desired = _unit_state_dying;
		biped_verify_object_vectors(biped_index, "post-dying");
	}
	else
	{
		if (biped->unit.animation.state == _unit_state_dying_airborne)
		{
			biped_apply_rotation(biped_index);
		}
		animation->state_desired = _unit_state_dying_airborne;
		biped_verify_object_vectors(biped_index, "post-dying-airborne");
	}
	animation->crouching = FALSE;

	return;
}

boolean biped_flying_through_air(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	return biped->biped.airborne_ticks > 3 && (!TEST_FLAG(definition->biped.flags, _biped_flying_bit) || TEST_FLAG(biped->object.damage_flags, _object_dead_bit));
}

static void biped_update_airborne(
	long biped_index,
	struct unit_animation_update_data *animation)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);
	boolean flying_through_air = FALSE;

	if (biped_flying_through_air(biped_index))
	{
		flying_through_air = TRUE;
		if (TEST_FLAG(definition->biped.flags, _biped_rotate_while_airborne_bit))
		{
			if (biped->unit.animation.state != _unit_state_melee_airborne && biped->unit.animation.state != _unit_state_leap_melee)
			{
				real angular_acceleration = real_random_range(DEGREES_TO_RADIANS(3), DEGREES_TO_RADIANS(5));
				boolean valid_axis = FALSE;
				real_vector3d angular_acceleration_axis;

				if (biped->object.up.k < 0.8f)
				{
					cross_product3d(&biped->object.up, global_up3d, &angular_acceleration_axis);
					valid_axis = normalize3d(&angular_acceleration_axis) > 0.f;
				}
				if (!valid_axis)
				{
					vector3d_from_angle(&angular_acceleration_axis, real_random_range(0.f, 2.f * _pi));
				}
				vector_from_line3d(&biped->object.angular_velocity, &angular_acceleration_axis, angular_acceleration, &biped->object.angular_velocity);
			}
			biped_apply_rotation(biped_index);
		}
	}

	match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 2687, TEST_FLAG(biped->biped.flags, _biped_airborne_bit));

	if (biped->unit.animation.state == _unit_state_leap_start || biped->unit.animation.state == _unit_state_leap_airborne)
	{
		animation->state_desired = _unit_state_leap_airborne;
	}
	else if (biped->unit.animation.state == _unit_state_airborne || flying_through_air)
	{
		animation->state_desired = _unit_state_airborne;
	}

	biped_verify_object_vectors(biped_index, "post-airborne");

	return;
}

static void biped_start_landing(
	long biped_index,
	real landing_velocity)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);
	real minimum_soft_landing_velocity = definition->biped.minimum_soft_landing_velocity / TICKS_PER_SECOND;
	real minimum_hard_landing_velocity = definition->biped.minimum_hard_landing_velocity / TICKS_PER_SECOND;
	real maximum_hard_landing_velocity = definition->biped.maximum_hard_landing_velocity / TICKS_PER_SECOND;

	if (!(landing_velocity < minimum_soft_landing_velocity))
	{
		short landing = NONE;
		real velocity;
		real velocity_range;
		real landing_time;

		if (landing_velocity < minimum_hard_landing_velocity)
		{
			velocity = landing_velocity - minimum_soft_landing_velocity;
			velocity_range = minimum_hard_landing_velocity - minimum_soft_landing_velocity;
			landing_time = definition->biped.maximum_soft_landing_time * TICKS_PER_SECOND;
			landing = _biped_landing_soft;
		}
		else
		{
			velocity = landing_velocity;
			velocity_range = maximum_hard_landing_velocity - minimum_hard_landing_velocity;
			landing_time = definition->biped.maximum_hard_landing_time * TICKS_PER_SECOND;
			landing = _biped_landing_hard;
		}

		if (velocity_range > 0.f)
		{
			velocity /= velocity_range;
			velocity = PIN(velocity, 0.f, 1.f);
			biped->biped.landing = landing;
			biped->biped.landing_recovery_counter = 0;
			biped->biped.landing_recovery_time = (char)(velocity * landing_time);
		}
	}

	return;
}

static void biped_update_landing(
	long biped_index,
	struct unit_animation_update_data *animation)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (++biped->biped.landing_recovery_counter >= biped->biped.landing_recovery_time)
	{
		biped->biped.landing = NONE;
	}

	if (!cinematic_in_progress() &&
		(biped->biped.landing_recovery_counter == 2 || (biped->biped.landing == NONE && biped->biped.landing_recovery_time < 2)))
	{
		biped_make_footstep(biped_index, _material_effect_biped_jump_land, _contact_point_left_foot);
		biped_make_footstep(biped_index, _material_effect_biped_jump_land, _contact_point_right_foot);
	}

	animation->state_desired = biped->biped.landing == _biped_landing_hard ? _unit_state_land_hard : _unit_state_land_soft;

	biped_verify_object_vectors(biped_index, "post-landing");

	return;
}

static void biped_update_slipping(
	long biped_index,
	struct unit_animation_update_data *animation)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (biped->biped.slipping_ticks > 3 && magnitude_squared3d(&biped->object.translational_velocity) > (1.f / TICKS_PER_SECOND) * (1.f / TICKS_PER_SECOND))
	{
		biped_make_footstep(biped_index, _material_effect_biped_sliding, _contact_point_left_foot);
		biped_make_footstep(biped_index, _material_effect_biped_sliding, _contact_point_right_foot);
	}

	biped_verify_object_vectors(biped_index, "post-slipping");

	return;
}

static void biped_update_jumping(
	long biped_index,
	struct unit_animation_update_data *animation)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	if (!TEST_FLAG(biped->biped.flags, _biped_airborne_bit) && biped->biped.landing != _biped_landing_hard)
	{
		if (biped->biped.jump_recovery_timer < SCHAR_MAX)
		{
			biped->biped.jump_recovery_timer++;
		}
		if (TEST_FLAG(biped->unit.control_flags, _unit_control_jump_bit) && biped->biped.jump_recovery_timer > 5)
		{
			biped_jump(biped_index);
		}
	}

	if (cheat.jetpack && biped->unit.player_index != NONE)
	{
		struct player_datum *player = player_get(biped->unit.player_index);
		real const jetpack_minimum_thrust = 0.01f;
		real const jetpack_maximum_thrust = 0.05f;
		real const jetpack_maximum_speed = 1.4f;
		real const jetpack_damping = 0.8f;
		boolean thrusting = FALSE;

		if (TEST_FLAG(biped->unit.control_flags, _unit_control_weapon_primary_trigger_bit) && TEST_FLAG(biped->unit.control_flags, _unit_control_throw_grenade_bit))
		{
			real speed = FLOOR(dot_product3d(&biped->object.translational_velocity, &biped->unit.aiming_vector), 0.f);
			real thrust = PIN(speed / jetpack_maximum_speed, 0.f, 1.f);
			real thrust_velocity = (jetpack_maximum_thrust - jetpack_minimum_thrust) * thrust - jetpack_maximum_thrust * thrust * thrust + jetpack_minimum_thrust;
			real_vector3d perpendicular_velocity;

			vector_from_line3d(&biped->object.translational_velocity, &biped->unit.aiming_vector, -speed, &perpendicular_velocity);
			vector_from_line3d(&biped->object.translational_velocity, &perpendicular_velocity, jetpack_damping - 1.f, &biped->object.translational_velocity);
			vector_from_line3d(&biped->object.translational_velocity, &biped->unit.aiming_vector, thrust_velocity, &biped->object.translational_velocity);
			SET_FLAG(biped->biped.flags, _biped_airborne_bit, TRUE);
			thrusting = TRUE;
		}
		else if (TEST_FLAG(biped->unit.control_flags, _unit_control_crouch_modifier_bit) && TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
		{
			scale_vector3d(&biped->object.translational_velocity, jetpack_damping - 1.f, &biped->object.translational_velocity);
		}

		if (player->local_player_index != NONE && thrusting)
		{
			struct vibrate_definition vibrate;

			memset(&vibrate, 0, sizeof(vibrate));
			vibrate.vibrate_frequencies[0].frequency = 1.f;
			vibrate.vibrate_frequencies[0].duration = 0.3f;
			vibrate.vibrate_frequencies[0].fade_function = _transition_function_late;
			rumble_player_impulse(player->local_player_index, &vibrate, 1.f, 1.f);
		}
	}

	return;
}

static boolean biped_jump(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);
	boolean jumped = FALSE;

	if (!TEST_FLAG(biped->biped.flags, _biped_airborne_bit) && biped->biped.landing != _biped_landing_hard)
	{
		real jump_velocity;
		real_vector3d new_velocity;
		real up_velocity;
		long actor_index;

		jumped = TRUE;
		jump_velocity = definition->biped.jump_velocity;
		if (biped->unit.player_index != NONE)
		{
			struct game_globals_player_information *player_information = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information);

			jump_velocity *= 1.f - biped->unit.body_stun * player_information->stun_turning_penalty;
		}
		if (cheat.super_jump && biped->unit.player_index != NONE)
		{
			jump_velocity *= 4.f;
		}

		new_velocity = biped->object.translational_velocity;
		up_velocity = dot_product3d(&new_velocity, &biped->object.up);
		if (up_velocity < jump_velocity)
		{
			vector_from_line3d(&new_velocity, &biped->object.up, jump_velocity - up_velocity, &new_velocity);
		}

		actor_index = biped->unit.swarm_actor_index == NONE ? biped->unit.actor_index : biped->unit.swarm_actor_index;
		if (actor_index != NONE)
		{
			boolean leaping = biped->unit.animation.state == _unit_state_leap_start || biped->unit.animation.state == _unit_state_leap_airborne;

			jumped = actor_aim_jump(actor_index, biped_index, leaping, jump_velocity, &new_velocity);
		}

		if (jumped)
		{
			biped->object.translational_velocity = new_velocity;
			SET_FLAG(biped->biped.flags, _biped_airborne_bit, TRUE);
			biped->biped.jump_recovery_timer = 0;
			biped->biped.support_surface_index = NONE;
			biped_make_footstep(biped_index, _material_effect_biped_jump, _contact_point_left_foot);
			biped_make_footstep(biped_index, _material_effect_biped_jump, _contact_point_right_foot);
		}
	}

	return jumped;
}

void biped_build_flying_axes(
	real_vector3d const *forward_vector,
	real_vector3d *left_vector,
	real_vector3d *up_vector)
{
	match_assert("c:\\halo\\SOURCE\\units\\bipeds.c", 2993, forward_vector && left_vector && up_vector);

	*up_vector = *global_up3d;
	cross_product3d(up_vector, forward_vector, left_vector);
	if (normalize3d(left_vector) == 0.f)
	{
		*up_vector = *global_forward3d;
		cross_product3d(up_vector, forward_vector, left_vector);
		normalize3d(left_vector);
	}
	cross_product3d(forward_vector, left_vector, up_vector);
	normalize3d(up_vector);

	match_assert_valid_real_vector3d_axes3("c:\\halo\\SOURCE\\units\\bipeds.c", 3008, forward_vector, left_vector, up_vector);

	return;
}

static void biped_update_physics(
	struct biped_physics *physics)
{
	boolean climbing = TEST_FLAG(physics->in_flags, _biped_physics_in_climb_anything_bit);
	real forward_i = 0.f;
	real forward_j = 0.f;

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\units\\bipeds.c", 3020, &physics->position);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\bipeds.c", 3021, &physics->velocity);

	physics->out_flags = 0;
	SET_FLAG(physics->out_flags, _biped_physics_out_cannot_stand_bit, FALSE);

	if (TEST_FLAG(physics->in_flags, _biped_physics_in_flying_bit))
	{
		real_vector3d desired_velocity;
		real_vector3d desired_acceleration;
		real_vector3d acceleration;
		real_vector3d left_vector;
		real_vector3d up_vector;
		real_vector3d desired_acceleration_direction;
		real desired_acceleration_magnitude;

		biped_build_flying_axes(&physics->forward, &left_vector, &up_vector);
		desired_velocity.i = physics->movement_desired.i * physics->forward.i + physics->movement_desired.j * left_vector.i + physics->movement_desired.k * up_vector.i;
		desired_velocity.j = physics->movement_desired.i * physics->forward.j + physics->movement_desired.j * left_vector.j + physics->movement_desired.k * up_vector.j;
		desired_velocity.k = physics->movement_desired.i * physics->forward.k + physics->movement_desired.j * left_vector.k + physics->movement_desired.k * up_vector.k;
		scale_vector3d(&desired_velocity, 1.f - physics->movement_penalty, &desired_velocity);
		subtract_vectors3d(&desired_velocity, &physics->velocity, &desired_acceleration);
		desired_acceleration_direction = desired_acceleration;
		desired_acceleration_magnitude = normalize3d(&desired_acceleration_direction);
		if (desired_acceleration_magnitude > physics->acceleration_maximum)
		{
			scale_vector3d(&desired_acceleration_direction, physics->acceleration_maximum, &acceleration);
		}
		else
		{
			acceleration = desired_acceleration;
		}
		add_vectors3d(&physics->velocity, &acceleration, &physics->new_velocity);
		SET_FLAG(physics->out_flags, _biped_physics_out_slipping_bit, FALSE);
		SET_FLAG(physics->out_flags, _biped_physics_out_airborne_bit, TRUE);
	}
	else if (TEST_FLAG(physics->in_flags, _biped_physics_in_absolute_movement_bit))
	{
		physics->new_velocity.i = physics->movement_desired.i * physics->forward.i - physics->movement_desired.j * physics->forward.j;
		forward_i = physics->new_velocity.i;
		physics->new_velocity.j = physics->movement_desired.i * physics->forward.j + physics->movement_desired.j * physics->forward.i;
		forward_j = physics->new_velocity.j;
		physics->new_velocity.k = physics->movement_desired.k;
	}
	else if (TEST_FLAG(physics->in_flags, _biped_physics_in_airborne_bit))
	{
		real_vector2d desired_velocity;
		real_vector2d desired_acceleration;
		real_vector2d acceleration;
		real_vector2d desired_acceleration_direction;
		real desired_acceleration_magnitude;

		desired_velocity.i = physics->movement_desired.i * physics->forward.i - physics->movement_desired.j * physics->forward.j;
		desired_velocity.j = physics->movement_desired.i * physics->forward.j + physics->movement_desired.j * physics->forward.i;
		scale_vector2d(&desired_velocity, 1.f - physics->movement_penalty, &desired_velocity);
		desired_acceleration.i = desired_velocity.i - physics->velocity.i;
		desired_acceleration.j = desired_velocity.j - physics->velocity.j;
		desired_acceleration_direction = desired_acceleration;
		desired_acceleration_magnitude = normalize2d(&desired_acceleration_direction);
		if (desired_acceleration_magnitude > physics->airborne_acceleration_maximum)
		{
			scale_vector2d(&desired_acceleration_direction, physics->airborne_acceleration_maximum, &acceleration);
		}
		else
		{
			acceleration = desired_acceleration;
		}
		physics->new_velocity.i = physics->velocity.i + acceleration.i;
		physics->new_velocity.j = physics->velocity.j + acceleration.j;
		physics->new_velocity.k = physics->velocity.k - global_gravity;
		SET_FLAG(physics->out_flags, _biped_physics_out_slipping_bit, TEST_FLAG(physics->in_flags, _biped_physics_in_slipping_bit));
	}
	else
	{
		real_vector3d desired_velocity;
		real_vector3d desired_acceleration;
		real_vector3d acceleration;
		real speed = magnitude3d(&physics->movement_desired);
		real_vector3d desired_velocity_direction;

		if (TEST_FLAG(physics->in_flags, _biped_physics_in_climb_anything_bit))
		{
			real_vector3d forward = physics->aiming;
			real_vector3d left;

			cross_product3d(&physics->ground_plane.n, &forward, &left);
			if (normalize3d(&left) == 0.f)
			{
				cross_product3d(&physics->ground_plane.n, global_up3d, &left);
				if (normalize3d(&left) == 0.f)
				{
					cross_product3d(&physics->ground_plane.n, global_forward3d, &left);
					normalize3d(&left);
				}
			}
			cross_product3d(&left, &physics->ground_plane.n, &forward);
			normalize3d(&forward);
			desired_velocity_direction.i = physics->movement_desired.i * forward.i + physics->movement_desired.j * left.i;
			desired_velocity_direction.j = physics->movement_desired.i * forward.j + physics->movement_desired.j * left.j;
			desired_velocity_direction.k = physics->movement_desired.i * forward.k + physics->movement_desired.j * left.k + physics->movement_desired.k;
			normalize3d(&desired_velocity_direction);
		}
		else if (physics->ground_plane.n.k > _real_epsilon)
		{
			desired_velocity_direction.i = physics->movement_desired.i * physics->forward.i - physics->movement_desired.j * physics->forward.j;
			forward_i = desired_velocity_direction.i;
			desired_velocity_direction.j = physics->movement_desired.i * physics->forward.j + physics->movement_desired.j * physics->forward.i;
			forward_j = desired_velocity_direction.j;
			desired_velocity_direction.k = physics->movement_desired.k - (desired_velocity_direction.i * physics->ground_plane.n.i + desired_velocity_direction.j * physics->ground_plane.n.j) / physics->ground_plane.n.k;
			normalize3d(&desired_velocity_direction);
		}
		else
		{
			real_vector3d forward = physics->aiming;
			real_vector3d left;

			cross_product3d(global_up3d, &physics->aiming, &left);
			normalize3d(&left);
			vector_from_line3d(&forward, &physics->ground_plane.n, -dot_product3d(&forward, &physics->ground_plane.n), &forward);
			vector_from_line3d(&left, &physics->ground_plane.n, -dot_product3d(&left, &physics->ground_plane.n), &left);
			forward_i = physics->movement_desired.i * physics->forward.i - physics->movement_desired.j * physics->forward.j;
			forward_j = physics->movement_desired.i * physics->forward.j + physics->movement_desired.j * physics->forward.i;
			desired_velocity_direction.i = physics->movement_desired.i * forward.i + physics->movement_desired.j * left.i;
			desired_velocity_direction.j = physics->movement_desired.i * forward.j + physics->movement_desired.j * left.j;
			desired_velocity_direction.k = physics->movement_desired.i * forward.k + physics->movement_desired.j * left.k + physics->movement_desired.k;
			if (!climbing)
			{
				desired_velocity_direction.k *= 5.f;
			}
			normalize3d(&desired_velocity_direction);
		}

		if (!TEST_FLAG(physics->in_flags, _biped_physics_in_climb_anything_bit))
		{
			if (desired_velocity_direction.k <= physics->downhill_k1)
			{
				speed *= physics->downhill_velocity_scale;
			}
			else if (desired_velocity_direction.k < physics->downhill_k0)
			{
				speed *= (physics->downhill_velocity_scale - 1.f) * (desired_velocity_direction.k - physics->downhill_k0) / (physics->downhill_k1 - physics->downhill_k0) + 1.f;
			}
			else if (desired_velocity_direction.k >= physics->uphill_k1)
			{
				speed *= physics->uphill_velocity_scale;
			}
			else if (desired_velocity_direction.k > physics->uphill_k0)
			{
				speed *= (physics->uphill_velocity_scale - 1.f) * (desired_velocity_direction.k - physics->uphill_k0) / (physics->uphill_k1 - physics->uphill_k0) + 1.f;
			}
		}

		speed *= 1.f - physics->movement_penalty;
		scale_vector3d(&desired_velocity_direction, speed, &desired_velocity);
		desired_acceleration.i = desired_velocity.i - physics->velocity.i;
		desired_acceleration.j = desired_velocity.j - physics->velocity.j;
		desired_acceleration.k = desired_velocity.k - physics->velocity.k;
		{
			real_vector3d desired_acceleration_direction = desired_acceleration;
			real desired_acceleration_magnitude;
			boolean slipping = FALSE;

			desired_acceleration_magnitude = normalize3d(&desired_acceleration_direction);
			if (desired_acceleration_magnitude > physics->acceleration_maximum)
			{
				if (!TEST_FLAG(physics->in_flags, _biped_physics_in_climb_anything_bit))
				{
					slipping = TEST_FLAG(physics->in_flags, _biped_physics_in_slipping_bit);
				}
				scale_vector3d(&desired_acceleration_direction, physics->acceleration_maximum, &acceleration);
			}
			else
			{
				acceleration = desired_acceleration;
			}
			vector_from_line3d(&acceleration, &physics->ground_plane.n, -1.f / 128.f, &acceleration);
			SET_FLAG(physics->out_flags, _biped_physics_out_slipping_bit, slipping);
		}
		add_vectors3d(&physics->velocity, &acceleration, &physics->new_velocity);
		if (TEST_FLAG(physics->out_flags, _biped_physics_out_slipping_bit))
		{
			physics->new_velocity.k -= global_gravity;
		}
	}

	{
		unsigned long collision_flags;
		real_point3d position;
		real_point3d new_position;
		real_vector3d velocity;
		real_vector3d new_velocity;
		short collision_count;
		struct collision_plane collisions[MAXIMUM_BIPED_PHYSICS_COLLISIONS];
		boolean skip_collision = FALSE;

		if (TEST_FLAG(physics->in_flags, _biped_physics_in_no_collision_bit))
		{
			collision_flags = 0;
		}
		else if (TEST_FLAG(physics->in_flags, _biped_physics_in_dead_bit))
		{
			collision_flags = _collision_test_for_bipeds_dead_flags;
		}
		else if (TEST_FLAG(physics->in_flags, _biped_physics_in_pass_through_bipeds_bit))
		{
			collision_flags = _collision_test_for_bipeds_passthrough_living_flags;
		}
		else
		{
			collision_flags = _collision_test_for_bipeds_living_flags;
		}

		position = physics->position;
		velocity = physics->new_velocity;
		velocity.k += physics->crouch_velocity;

		skip_collision = debug_biped_skip_collision;
		if (skip_collision)
		{
			collision_count = 1;
			collisions[0].t = 0.f;
			collisions[0].point = position;
			collisions[0].plane.n = *global_up3d;
			collisions[0].plane.d = -position.z;
			collisions[0].object_index = physics->biped_index;
			collisions[0].surface_index = NONE;
			collisions[0].flags = 0;
			collisions[0].breakable_surface_index = 0;
			collisions[0].material_index = NONE;
			new_velocity = velocity;
			new_velocity.k = 0.f;
			point_from_line3d(&position, &new_velocity, 1.f, &new_position);
		}
		else
		{
			collision_count = collision_move_pill(collision_flags, &position, &velocity, physics->height, physics->width, physics->biped_index, &new_position, &new_velocity, MAXIMUM_BIPED_PHYSICS_COLLISIONS, collisions);
		}

		if (debug_biped_physics && object_get(physics->biped_index)->object.owner_player_index != NONE)
		{
			collision_debug_point = position;
			collision_debug_repeat = TRUE;
			collision_debug_vector = velocity;
			collision_debug_length = 1.f;
			collision_debug_width = physics->width;
			collision_debug_height = physics->height;
		}

		SET_FLAG(physics->out_flags, _biped_physics_out_splatter_bit, collision_count >= MAXIMUM_BIPED_PHYSICS_COLLISIONS);

		physics->stick_surface_index = NONE;
		if (!collision_count && physics->existing_support_surface_index != NONE)
		{
			struct collision_bsp *bsp = global_collision_bsp_get();
			real_point3d original_plane_point;
			real_plane3d original_plane;
			long stick_surface_index = NONE;
			real_plane3d stick_surface_plane;
			real_point3d stick_edge_point;
			real stick_distance_squared = REAL_MAX;
			real stick_velocity;

			if (physics->existing_support_surface_index >= 0 && physics->existing_support_surface_index < bsp->surfaces.count)
			{
				struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, physics->existing_support_surface_index, struct collision_surface);
				long edge_index = surface->first_edge_index;
				real distance;

				bsp3d_get_plane_from_designator(&bsp->bsp3d, surface->plane_designator, &original_plane);
				distance = dot_product3d((real_vector3d *)&new_position, &original_plane.n) - original_plane.d;
				point_from_line3d(&new_position, &original_plane.n, -distance, &original_plane_point);
				do
				{
					struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
					boolean reverse = physics->existing_support_surface_index == edge->surface_indices[1];
					long adjacent_surface_index = edge->surface_indices[!reverse];

					if (adjacent_surface_index != NONE)
					{
						struct collision_surface *adjacent_surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, adjacent_surface_index, struct collision_surface);

						if (TEST_FLAG(physics->in_flags, _biped_physics_in_climb_anything_bit) || TEST_FLAG(adjacent_surface->flags, _collision_surface_climbable_bit))
						{
							real_plane3d adjacent_plane;
							real adjacent_velocity;
							real adjacent_distance;

							bsp3d_get_plane_from_designator(&bsp->bsp3d, adjacent_surface->plane_designator, &adjacent_plane);
							adjacent_velocity = dot_product3d(&new_velocity, &adjacent_plane.n);
							adjacent_distance = dot_product3d((real_vector3d *)&new_position, &adjacent_plane.n) - adjacent_plane.d;
							if (adjacent_velocity > 0.f && adjacent_distance > -0.5f * physics->width)
							{
								struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[0], struct collision_vertex);
								struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[1], struct collision_vertex);
								real_vector3d p0_p1;
								real_vector3d p0_q;
								real_point3d edge_point;
								real t;
								real distance_squared;

								vector_from_points3d(&vertex0->point, &vertex1->point, &p0_p1);
								vector_from_points3d(&vertex0->point, &original_plane_point, &p0_q);
								t = dot_product3d(&p0_q, &p0_p1) / magnitude_squared3d(&p0_p1);
								if (t < 0.f)
								{
									edge_point = vertex0->point;
								}
								else if (t > 1.f)
								{
									edge_point = vertex1->point;
								}
								else
								{
									point_from_line3d(&vertex0->point, &p0_p1, t, &edge_point);
								}

								distance_squared = distance_squared3d(&original_plane_point, &edge_point);
								if (distance_squared < stick_distance_squared)
								{
									stick_surface_index = adjacent_surface_index;
									stick_surface_plane = adjacent_plane;
									stick_distance_squared = distance_squared;
									stick_edge_point = edge_point;
									stick_velocity = adjacent_velocity;
								}
							}
						}
					}

					edge_index = edge->edge_indices[reverse];
				}
				while (edge_index != surface->first_edge_index);
			}

			if (stick_surface_index != NONE)
			{
				if (stick_distance_squared > (2.f * physics->width) * (2.f * physics->width))
				{
					stick_surface_index = NONE;
				}
				else if (stick_velocity > 1.6f / TICKS_PER_SECOND)
				{
					stick_surface_index = NONE;
				}
			}

			if (stick_surface_index != NONE)
			{
				real distance = dot_product3d(&stick_surface_plane.n, (real_vector3d *)&new_position) - (stick_surface_plane.d + physics->width);

				if (fabs(distance) > 0.5f * physics->width)
				{
					stick_surface_index = NONE;
				}
				else
				{
					real new_velocity_dot = dot_product3d(&new_velocity, &stick_surface_plane.n);

					point_from_line3d(&new_position, &stick_surface_plane.n, -distance, &new_position);
					if (new_velocity_dot > -1.f / TICKS_PER_SECOND)
					{
						point_from_line3d((real_point3d *)&new_velocity, &stick_surface_plane.n, -(new_velocity_dot + 1.f / TICKS_PER_SECOND), (real_point3d *)&new_velocity);
					}

					collision_count = 1;
					collisions[0].t = 0.f;
					point_from_line3d(&new_position, &stick_surface_plane.n, (real)(-physics->width), &collisions[0].point);
					collisions[0].plane = stick_surface_plane;
					collisions[0].object_index = NONE;
					collisions[0].surface_index = stick_surface_index;
					collisions[0].flags = 0;
					collisions[0].breakable_surface_index = 0;
					collisions[0].material_index = NONE;
					physics->stick_surface_index = stick_surface_index;
				}
			}
		}

		{
			real magnitude_squared = forward_i * forward_i + forward_j * forward_j;

			if (magnitude_squared > _real_epsilon * _real_epsilon)
			{
				real inverse_magnitude = 1.f / sqrt(magnitude_squared);

				forward_i *= inverse_magnitude;
				forward_j *= inverse_magnitude;
			}
		}

		{
			short ground_index = NONE;
			boolean ground_is_ground = FALSE;
			boolean ground_is_stick = FALSE;
			real ground_k = -REAL_MAX;
			real ground_velocity = -REAL_MAX;

			if (!TEST_FLAG(physics->in_flags, _biped_physics_in_flying_bit))
			{
				short i;

				for (i = 0; i < collision_count; i++)
				{
					boolean ground = !TEST_FLAG(physics->in_flags, _biped_physics_in_dead_bit) && (climbing || TEST_FLAG(collisions[i].flags, _collision_surface_climbable_bit));
					boolean stick = physics->stick_surface_index != NONE && collisions[ground_index].surface_index == physics->stick_surface_index;
					real plane_velocity = -dot_product3d(&physics->new_velocity, &collisions[i].plane.n);
					boolean select = FALSE;

					if (ground && !climbing && forward_i * collisions[i].plane.n.i + forward_j * collisions[i].plane.n.j > 0.5f)
					{
						select = FALSE;
					}
					else if (ground && !ground_is_ground)
					{
						select = TRUE;
					}
					else if (ground)
					{
						if (stick)
						{
							select = TRUE;
						}
						else if (!ground_is_stick)
						{
							select = plane_velocity > ground_velocity;
						}
					}
					else if (!ground_is_ground)
					{
						select = collisions[i].plane.n.k > ground_k;
					}

					if (select)
					{
						ground_is_ground = ground;
						ground_index = i;
						ground_is_stick = stick;
						ground_k = collisions[i].plane.n.k;
						ground_velocity = plane_velocity;
					}

					if (!TEST_FLAG(physics->out_flags, _biped_physics_out_volatile_collision_bit) &&
						(TEST_FLAG(collisions[i].flags, _collision_surface_breakable_bit) ||
						(collisions[i].object_index != NONE && !TEST_FLAG(_object_mask_scenery, object_get_type(collisions[i].object_index)))))
					{
						SET_FLAG(physics->out_flags, _biped_physics_out_volatile_collision_bit, TRUE);
					}
				}
			}

			{
				boolean on_ground = FALSE;
				real impact_velocity = 0.f;

				if (ground_index != NONE)
				{
					real_plane3d ground_plane = collisions[ground_index].plane;

					impact_velocity = -(dot_product3d(&velocity, &ground_plane.n));
					if (ground_is_ground || ground_is_stick)
					{
						on_ground = TRUE;
					}
					else if (ground_k >= physics->minimum_normal_k)
					{
						on_ground = TRUE;
						if (TEST_FLAG(physics->in_flags, _biped_physics_in_airborne_bit) && physics->ground_tangential_velocity_max < REAL_MAX)
						{
							real_vector3d tangential_velocity;

							vector_from_line3d(&velocity, &ground_plane.n, impact_velocity, &tangential_velocity);
							if (magnitude_squared3d(&tangential_velocity) > physics->ground_tangential_velocity_max * physics->ground_tangential_velocity_max)
							{
								real impact_angle = impact_velocity / magnitude3d(&velocity);

								if (impact_angle < physics->ground_tangential_angle)
								{
									on_ground = FALSE;
								}
							}
						}
					}
				}

				if (on_ground)
				{
					SET_FLAG(physics->out_flags, _biped_physics_out_airborne_bit, FALSE);
					physics->ground_plane = collisions[ground_index].plane;
					physics->support_surface_index = collisions[ground_index].surface_index;
					if (physics->support_surface_index != NONE && physics->support_surface_index == physics->stick_surface_index)
					{
						physics->landing_velocity = 0.f;
					}
					else
					{
						physics->landing_velocity = -dot_product3d(&velocity, &physics->ground_plane.n);
					}
				}
				else
				{
					SET_FLAG(physics->out_flags, _biped_physics_out_airborne_bit, TRUE);
					physics->ground_plane = depths_of_hell;
					physics->support_surface_index = NONE;
					physics->landing_velocity = 0.f;
				}
			}
		}

		{
			long bumped_object_index = NONE;
			short bumped_object_type;
			real bumped_object_velocity_magnitude_squared;
			short i;

			for (i = 0; i < collision_count; i++)
			{
				if (collisions[i].object_index != NONE)
				{
					struct object_datum *object = object_get(collisions[i].object_index);
					real_vector3d this_object_relative_velocity;
					real magnitude_squared;
					boolean select = FALSE;

					subtract_vectors3d(&object->object.translational_velocity, &new_velocity, &this_object_relative_velocity);
					magnitude_squared = magnitude_squared3d(&this_object_relative_velocity);
					if (bumped_object_index == NONE)
					{
						select = TRUE;
					}
					else if (bumped_object_type == _object_type_vehicle && object->object.type != _object_type_vehicle)
					{
						select = FALSE;
					}
					else if (bumped_object_type != _object_type_vehicle && object->object.type == _object_type_vehicle)
					{
						select = TRUE;
					}
					else
					{
						select = magnitude_squared > bumped_object_velocity_magnitude_squared;
					}

					if (select)
					{
						bumped_object_index = collisions[i].object_index;
						bumped_object_type = object->object.type;
						bumped_object_velocity_magnitude_squared = magnitude_squared;
					}
				}
			}
			physics->bumped_object_index = bumped_object_index;
		}

		{
			short i;

			physics->elevator_object_index = NONE;
			for (i = 0; i < collision_count; i++)
			{
				long object_index = collisions[i].object_index;

				if (object_index != NONE)
				{
					struct machine_datum *machine = machine_try_and_get(object_index);

					if (machine)
					{
						struct machine_definition *machine_definition = machine_definition_get(machine->definition_index);

						if (TEST_FLAG(machine_definition->machine.flags, _machine_is_elevator_bit) && machine_definition->machine.elevator_node_index != NONE)
						{
							physics->elevator_object_index = object_index;
						}
					}
				}
			}
		}

		physics->collision_velocity = distance3d((real_point3d *)&velocity, (real_point3d *)&new_velocity);
		physics->new_position = new_position;
		physics->new_velocity = new_velocity;
		physics->new_velocity.k -= physics->crouch_velocity;

		if (TEST_FLAG(physics->in_flags, _biped_physics_in_crouched_bit) && TEST_FLAG(physics->in_flags, _biped_physics_in_trying_to_stand_bit))
		{
			struct biped_datum *biped = biped_get(physics->biped_index);

			if (biped->unit.player_index != NONE)
			{
				struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);

				if (!TEST_FLAG(biped_definition->biped.flags, _biped_pill_centered_at_origin_bit) && !TEST_FLAG(biped_definition->biped.flags, _biped_spherical_bit))
				{
					unsigned long flags = TEST_FLAG(physics->in_flags, _biped_physics_in_dead_bit) ? _collision_test_for_bipeds_dead_flags : _collision_test_for_bipeds_living_flags;
					real_point3d center = physics->new_position;
					real radius = 0.5f * biped_definition->biped.collision_height_standing;
					struct collision_feature_list features;

					center.z += radius;
					if (collision_get_features_in_sphere(flags, &center, radius, 0.f, biped_definition->biped.collision_radius, physics->biped_index, &features))
					{
						real_vector3d vector;
						struct collision_plane collision;

						scale_vector3d(global_up3d, biped_definition->biped.collision_height_standing - 2.f * biped_definition->biped.collision_radius, &vector);
						if (collision_features_test_vector(&features, &physics->new_position, &vector, &collision))
						{
							SET_FLAG(physics->out_flags, _biped_physics_out_cannot_stand_bit, TRUE);
						}
					}
				}
			}
		}
	}

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\units\\bipeds.c", 3856, &physics->new_position);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\bipeds.c", 3857, &physics->new_velocity);

	return;
}

static void biped_try_to_make_footsteps(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);
	boolean moving = FALSE;
	boolean turning = FALSE;

	switch (biped->unit.animation.state)
	{
	case _unit_state_turn_left:
	case _unit_state_turn_right:
		turning = TRUE;
		break;
	case _unit_state_move_front:
	case _unit_state_move_back:
	case _unit_state_move_left:
	case _unit_state_move_right:
		if (magnitude_squared3d(&biped->unit.throttle) > 0.25f)
		{
			moving = TRUE;
		}
		break;
	}

	if (biped->object.animation.state.index != NONE)
	{
		struct animation_graph *animation_graph = animation_graph_definition_get(biped->object.animation.animation_graph_index);
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, biped->object.animation.state.index, struct animation);

		if (turning)
		{
			if (biped->object.animation.state.frame_index == 0)
			{
				biped_make_footstep(biped_index, _material_effect_biped_shuffle, _contact_point_left_foot);
				biped_make_footstep(biped_index, _material_effect_biped_shuffle, _contact_point_right_foot);
			}
		}
		else if (moving && (animation_left_foot_frame_index(animation) || animation_right_foot_frame_index(animation)))
		{
			boolean footstep = FALSE;
			boolean right_foot;

			if (biped->object.animation.state.frame_index == animation_left_foot_frame_index(animation))
			{
				right_foot = FALSE;
				footstep = TRUE;
			}
			else if (biped->object.animation.state.frame_index == animation_right_foot_frame_index(animation))
			{
				right_foot = TRUE;
				footstep = TRUE;
			}

			if (footstep)
			{
				biped_make_footstep(biped_index, biped->unit.animation.base_seat_index == _unit_base_seat_stand, right_foot != FALSE);
			}
		}
	}

	switch (biped->biped.state)
	{
	case _biped_state_idle:
		if (biped->biped.stop_ticks > 0)
		{
			biped->biped.stop_ticks++;
			if (biped->biped.stop_ticks > 3)
			{
				biped_make_footstep(biped_index, _material_effect_biped_shuffle, _contact_point_left_foot);
				biped_make_footstep(biped_index, _material_effect_biped_shuffle, _contact_point_right_foot);
				biped->biped.stop_ticks = 0;
			}
		}
		break;
	case _biped_state_moving:
		biped->biped.stop_ticks = 1;
		break;
	default:
		biped->biped.stop_ticks = 0;
		break;
	}

	return;
}

static void biped_make_footstep(
	long biped_index,
	short event_index,
	short contact_point_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *definition = biped_definition_get(biped->definition_index);

	match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\bipeds.c", 3965, _collision_user_bipeds);

	if (contact_point_index < definition->biped.contact_points.count && definition->biped.material_effects.index != NONE)
	{
		if (material_effect_visible(&biped->object.bounding_sphere_center))
		{
			struct biped_contact_point *contact_point = TAG_BLOCK_GET_ELEMENT(&definition->biped.contact_points, contact_point_index, struct biped_contact_point);
			struct object_marker marker;

			if (object_get_marker_by_name(biped_index, contact_point->marker_name, &marker, 1))
			{
				material_effect_new_from_point(definition->biped.material_effects.index, event_index, &marker.matrix.position, 0.f);
			}
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\units\\bipeds.c", 3982);

	return;
}

static void biped_find_nearby_support_surface(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct collision_bsp *bsp = global_collision_bsp_get();
	struct collision_bsp_test_sphere_result sphere_test_result;
	real_point3d center;
	real height;
	real width;

	biped_get_physics_pill(biped_index, &center, &height, &width);

	match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\bipeds.c", 3999, _collision_user_bipeds);

	if (collision_bsp_test_sphere(bsp, 256, breakable_surface_flags_get(), &center, width + 0.05f, &sphere_test_result))
	{
		long best_surface_index = NONE;
		real best_distance = REAL_MAX;
		real_plane3d best_plane;
		short i;

		for (i = 0; i < sphere_test_result.surface_count; i++)
		{
			struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, sphere_test_result.surface_indices[i], struct collision_surface);
			real_plane3d plane;
			real distance;

			bsp3d_get_plane_from_designator(&bsp->bsp3d, surface->plane_designator, &plane);
			distance = dot_product3d(&plane.n, (real_vector3d *)&center) - plane.d;
			if (distance < best_distance)
			{
				best_distance = distance;
				best_surface_index = sphere_test_result.surface_indices[i];
				best_plane = plane;
			}
		}

		if (best_surface_index != NONE)
		{
			biped->biped.support_surface_index = best_surface_index;
			biped->biped.ground_plane = best_plane;
			biped->object.up = best_plane.n;
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\units\\bipeds.c", 4034);

	return;
}

static void biped_snap_facing(
	long biped_index)
{
	struct biped_datum *biped = biped_get(biped_index);
	struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);

	if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit) && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
	{
		real_vector3d new_right;
		real_vector3d new_up;
		real bank_cosine;
		real bank_sine;

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\bipeds.c", 4051, &biped->object.forward);

		cross_product3d(&biped->object.forward, global_up3d, &new_right);
		cross_product3d(&new_right, &biped->object.forward, &new_up);
		if (normalize3d(&new_up) == 0.f)
		{
			new_up = *global_forward3d;
			new_right = *global_left3d;
		}
		bank_cosine = cosine(biped->biped.bank);
		bank_sine = sine(biped->biped.bank);
		scale_vector3d(&new_up, bank_cosine, &new_up);
		normalize3d(&new_right);
		point_from_line3d((real_point3d *)&new_up, &new_right, bank_sine, (real_point3d *)&biped->object.up);
		biped_verify_object_vectors(biped_index, "post-bank-snapfacing");
	}
	else if (TEST_FLAG(biped_definition->biped.flags, _biped_climbs_anything_bit) && !TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
	{
		boolean use_new_up = FALSE;
		real_vector3d rotation_axis;
		real_vector3d new_up;
		real_vector3d new_right;
		real_vector3d new_forward;

		if (biped->biped.support_surface_index == NONE)
		{
			new_up = biped->object.up;
			use_new_up = TRUE;
		}
		else
		{
			new_up = biped->biped.ground_plane.n;
			cross_product3d(&biped->object.up, &new_up, &rotation_axis);
			if (normalize3d(&rotation_axis) == 0.f)
			{
				if (dot_product3d(&new_up, &biped->object.up) > 0.f)
				{
					use_new_up = TRUE;
				}
				else
				{
					rotation_axis = biped->object.forward;
				}
			}
		}

		if (!use_new_up)
		{
			real_vector3d rotated_up = biped->object.up;
			real_vector3d continuing_rotation_axis;
			real angle_sine = sine(DEGREES_TO_RADIANS(10));
			real angle_cosine = cosine(DEGREES_TO_RADIANS(10));

			rotate_vector_about_axis(&rotated_up, &rotation_axis, angle_sine, angle_cosine);
			cross_product3d(&rotated_up, &new_up, &continuing_rotation_axis);
			if (dot_product3d(&rotation_axis, &continuing_rotation_axis) > 0.f)
			{
				new_up = rotated_up;
			}
		}

		cross_product3d(&biped->object.forward, &new_up, &new_right);
		cross_product3d(&new_up, &new_right, &new_forward);
		if (normalize3d(&new_forward) == 0.f)
		{
			cross_product3d(&new_up, &biped->object.up, &new_right);
			cross_product3d(&new_up, &new_right, &new_forward);
			if (normalize3d(&new_forward) == 0.f)
			{
				new_up = *global_up3d;
				new_forward = *global_forward3d;
			}
		}
		biped->object.forward = new_forward;
		biped->object.up = new_up;
		biped_verify_object_vectors(biped_index, "post-climb-snapfacing");
	}
	else if (TEST_FLAG(biped->object.damage_flags, _object_dead_bit) && !TEST_FLAG(biped->biped.flags, _biped_airborne_bit))
	{
		real cosine_angle;

		biped_verify_object_vectors(biped_index, "pre-deadplane-snapfacing");
		cosine_angle = dot_product3d(&biped->object.up, &biped->biped.ground_plane.n);
		if (!(fabs(cosine_angle - 1.f) < _real_epsilon))
		{
			real angle = arccosine(cosine_angle);

			if (angle != 0.f)
			{
				real_vector3d rotation_axis;

				cross_product3d(&biped->object.up, &biped->biped.ground_plane.n, &rotation_axis);
				if (normalize3d(&rotation_axis) != 0.f)
				{
					rotate_vector_about_axis(&biped->object.up, &rotation_axis, sine(angle), cosine(angle));
					rotate_vector_about_axis(&biped->object.forward, &rotation_axis, sine(angle), cosine(angle));
					normalize3d(&biped->object.up);
					normalize3d(&biped->object.forward);
				}
			}
		}
		biped_verify_object_vectors(biped_index, "post-deadplane-snapfacing");
	}
	else
	{
		biped->object.forward.k = 0.f;
		if (normalize3d(&biped->object.forward) == 0.f)
		{
			biped->object.forward = *global_forward3d;
		}
		biped->object.up = *global_up3d;
		biped_verify_object_vectors(biped_index, "post-normal-snapfacing");
	}

	return;
}

/* ---------- private code */
