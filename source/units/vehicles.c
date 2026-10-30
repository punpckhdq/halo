/*
VEHICLES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "vehicles.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "network_messages.h"
#include "collisions.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "observer.h"
#include "physics_constants.h"
#include "network_client_manager.h"
#include "effects.h"
#include "game_engine_list.h"
#include "object_types.h"
#include "physics.h"
#include "material_effects.h"
#include "material_effect_definitions.h"

/* ---------- constants */

enum
{
	_vehicle_function_none = 0,
	_vehicle_function_speed_absolute,
	_vehicle_function_speed_forward,
	_vehicle_function_speed_backward,
	_vehicle_function_slide_absolute,
	_vehicle_function_slide_left,
	_vehicle_function_slide_right,
	_vehicle_function_speed_slide_maximum,
	_vehicle_function_turn_absolute,
	_vehicle_function_turn_left,
	_vehicle_function_turn_right,
	_vehicle_function_crouch,
	_vehicle_function_jump,
	_vehicle_function_walk,
	_vehicle_function_velocity_air,
	_vehicle_function_velocity_water,
	_vehicle_function_velocity_ground,
	_vehicle_function_velocity_forward,
	_vehicle_function_velocity_left,
	_vehicle_function_velocity_up,
	_vehicle_function_left_tread_position,
	_vehicle_function_right_tread_position,
	_vehicle_function_left_tread_velocity,
	_vehicle_function_right_tread_velocity,
	_vehicle_function_front_left_tire_position,
	_vehicle_function_front_right_tire_position,
	_vehicle_function_back_left_tire_position,
	_vehicle_function_back_right_tire_position,
	_vehicle_function_front_left_tire_velocity,
	_vehicle_function_front_right_tire_velocity,
	_vehicle_function_back_left_tire_velocity,
	_vehicle_function_back_right_tire_velocity,
	_vehicle_function_wingtip_contrail,
	_vehicle_function_hover,
	_vehicle_function_thrust,
	_vehicle_function_engine_hack,
	_vehicle_function_wingtip_contrail_new,
	NUMBER_OF_VEHICLE_FUNCTIONS
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

real_quaternion *matrix3x3_rotation_to_quaternion(real_matrix3x3 const *matrix, real_quaternion *quaternion);
byte quantize_real_to_byte_lower_bound(real min, real max, real value);

static real_vector3d *compute_acceleration(real_vector3d const *current_velocity, real_vector3d const *desired_velocity, real maximum_acceleration, real maximum_deceleration, real_vector3d *acceleration);
static void update_human_tank_physics(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_human_jeep_physics(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_human_boat_physics(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_human_plane_physics(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_alien_scout_physics(long vehicle_index, real steering, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_alien_fighter_physics(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_alien_fighter_physics_new(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_alien_fighter_physics_old(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void update_turret_physics(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void slowly_stop_vehicle(long vehicle_index);
static void create_pelican_effect(long vehicle_index);
static void create_ghost_effect(long vehicle_index);
static void create_crashing_effects(long vehicle_index, real_vector3d const *old_velocity, struct mass_point_datum *mass_points);
static boolean update_suspension(long vehicle_index);
static void create_slipping_effects(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);
static void compute_airborne_ticks(long vehicle_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points);

/* ---------- globals */

static struct profile_section vehicle_update_section = {"vehicle_update", NONE, TRUE}; /* fake name */

/* ---------- public code */

void vehicles_initialize(
	void)
{
	return;
}

void vehicles_initialize_for_new_map(
	void)
{
	return;
}

void vehicles_dispose_from_old_map(
	void)
{
	return;
}

void vehicles_dispose(
	void)
{
	return;
}

void vehicle_place(
	long vehicle_index,
	struct scenario_vehicle_datum *scenario_vehicle)
{
	unit_place(vehicle_index, &scenario_vehicle->unit);
	object_add_scenario_permutation(vehicle_index, &scenario_vehicle->permutation);

	return;
}

void vehicle_reset(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);

	vehicle->vehicle.flags = 0;
	vehicle->vehicle.stop_time = 0;
	vehicle->vehicle.airborne_ticks = 0;
	vehicle->vehicle.upending_type = _vehicle_not_upending;
	vehicle->vehicle.upending_ticks = 0;
	vehicle->vehicle.on_ground_ticks = 0;
	vehicle->vehicle.speed = 0.f;
	vehicle->vehicle.slide = 0.f;
	vehicle->vehicle.turn = 0.f;
	vehicle->vehicle.wheel = 0.f;
	vehicle->vehicle.left_tread = 0.f;
	vehicle->vehicle.right_tread = 0.f;
	vehicle->vehicle.thrust = 0.f;
	vehicle->vehicle.hover = 0.f;
	memset(vehicle->vehicle.suspension, 0, sizeof(vehicle->vehicle.suspension));
	set_real_vector3d(&vehicle->vehicle.collision_force, 0.f, 0.f, 0.f);
	set_real_vector3d(&vehicle->vehicle.collision_torque, 0.f, 0.f, 0.f);
	vehicle->vehicle.stuck_mass_point_flags = 0;

	return;
}

boolean vehicle_new(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	vehicle_reset(vehicle_index);

	if (definition->object.physics.index == NONE)
	{
		SET_FLAG(vehicle->object.flags, _object_at_rest_bit, TRUE);
	}
	else
	{
		SET_FLAG(vehicle->object.flags, _object_at_rest_bit, FALSE);
	}

	if (definition->object.physics.index != NONE)
	{
		vehicle->object.position.z += definition->object.bounding_radius * 0.5f;
	}

	return TRUE;
}

void vehicle_delete(
	long vehicle_index)
{
	return;
}

boolean vehicle_update(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	profile_enter(vehicle_update_section);

	if (vehicle->object.parent_object_index != NONE)
	{
		set_real_vector3d(&vehicle->object.angular_velocity, 0.f, 0.f, 0.f);
		set_real_vector3d(&vehicle->object.translational_velocity, 0.f, 0.f, 0.f);
		SET_FLAG(vehicle->object.flags, _object_at_rest_bit, FALSE);
	}
	else
	{
		real steering;
		boolean integrated_light = TEST_FLAG(vehicle->unit.control_flags, _unit_control_integrated_light_bit);

		SET_FLAG(vehicle->vehicle.flags, _vehicle_control_crouch_bit, TEST_FLAG(vehicle->unit.control_flags, _unit_control_crouch_modifier_bit));
		SET_FLAG(vehicle->vehicle.flags, _vehicle_control_jump_bit, TEST_FLAG(vehicle->unit.control_flags, _unit_control_jump_bit) ||
			(TEST_FLAG(definition->vehicle.flags, _vehicle_control_opposite_speed_sets_jump_bit) &&
			((vehicle->unit.throttle.i > 0.f && vehicle->vehicle.speed < 0.f) || (vehicle->unit.throttle.i < 0.f && vehicle->vehicle.speed > 0.f))));

		{
			real_vector3d left;

			cross_product3d(&vehicle->object.up, &vehicle->object.forward, &left);
			steering = arctangent(dot_product3d(&vehicle->unit.desired_facing_vector, &left), dot_product3d(&vehicle->unit.desired_facing_vector, &vehicle->object.forward));
		}

		if (TEST_FLAG(vehicle->vehicle.flags, _vehicle_upending_bit) &&
			vehicle->vehicle.upending_type != _vehicle_not_upending &&
			vehicle->vehicle.upending_ticks < 30 &&
			vehicle->object.up.k <= 0.9f)
		{
			real_vector3d axis;
			real_vector3d dummy;
			real angular_velocity = (vehicle->vehicle.upending_type == _vehicle_upending_along_forward || vehicle->vehicle.upending_type == _vehicle_upending_along_right) ? 0.3 : -0.3;
			boolean upright;
			boolean clockwise = vehicle->vehicle.upending_type == _vehicle_upending_along_forward || vehicle->vehicle.upending_type == _vehicle_upending_along_right;

			if (vehicle->vehicle.upending_type == _vehicle_upending_along_right || vehicle->vehicle.upending_type == _vehicle_upending_along_left)
			{
				cross_product3d(&vehicle->object.forward, &vehicle->object.up, &axis);
			}
			else
			{
				axis = vehicle->object.forward;
			}
			upright = dot_product3d(cross_product3d(&vehicle->object.up, global_up3d, &dummy), &axis) > 0.f;

			angular_velocity *= PIN(-2.f * vehicle->object.up.k, definition->vehicle.flipping_angular_velocity_min, definition->vehicle.flipping_angular_velocity_max);
			SET_FLAG(vehicle->object.flags, _object_at_rest_bit, FALSE);

			if (vehicle->vehicle.upending_type == _vehicle_upending_along_forward || vehicle->vehicle.upending_type == _vehicle_upending_along_back)
			{
				real_vector3d left;

				cross_product3d(&vehicle->object.forward, &vehicle->object.up, &left);
				vector_from_line3d(&axis, &left, -vehicle->object.forward.k, &axis);
			}

			scale_vector3d(&axis, angular_velocity, &vehicle->object.angular_velocity);

			switch (definition->vehicle.type)
			{
			case _vehicle_alien_fighter:
				vehicle->object.translational_velocity.k = MIN(-0.01f, vehicle->object.translational_velocity.k);
				break;
			case _vehicle_human_tank:
				scale_vector3d(&vehicle->object.forward, dot_product3d(&vehicle->object.forward, &vehicle->object.translational_velocity), &vehicle->object.translational_velocity);
				break;
			}

			vehicle->vehicle.upending_ticks++;
		}
		else
		{
			vehicle->vehicle.upending_ticks = 0;
			vehicle->vehicle.upending_type = _vehicle_not_upending;
			SET_FLAG(vehicle->vehicle.flags, _vehicle_upending_bit, FALSE);
		}

		if (TEST_FLAG(vehicle->vehicle.flags, _vehicle_control_jump_bit))
		{
			physics_variable_speed_update_seek(&vehicle->vehicle.speed, &definition->vehicle.speed, 0.f, 1.f);
		}
		else
		{
			physics_variable_speed_update_seek(&vehicle->vehicle.speed, &definition->vehicle.speed, vehicle->unit.throttle.i, 1.f);
			physics_variable_speed_update_seek(&vehicle->vehicle.slide, &definition->vehicle.slide, vehicle->unit.throttle.j, 1.f);
		}

		if (definition->vehicle.type != _vehicle_human_tank)
		{
			real desired_turn = vehicle->vehicle.speed < 0.f ? -steering : steering;

			desired_turn = PIN(desired_turn, definition->vehicle.turn.minimum_position * (_pi / 180.f), definition->vehicle.turn.maximum_position * (_pi / 180.f));
			physics_variable_position_update_seek(&vehicle->vehicle.turn, &definition->vehicle.turn, FALSE, desired_turn,
				(((real)definition->vehicle.turn_rate) * (_pi / 180.f)) * (1.f / TICKS_PER_SECOND));
		}
		else if (vehicle->vehicle.speed == 0.f)
		{
			physics_variable_speed_update_seek(&vehicle->vehicle.turn, &definition->vehicle.speed, 0.f, 1.f);
		}
		else
		{
			real desired_turn = steering / _pi2;

			desired_turn = PIN(desired_turn, -1.f, 1.f) * definition->vehicle.speed.maximum_forward_speed;
			physics_variable_speed_update_seek(&vehicle->vehicle.turn, &definition->vehicle.speed, desired_turn, 2.f);
		}

		if (definition->object.physics.index != NONE &&
			((TEST_FLAG(definition->vehicle.flags, _vehicle_speed_wakes_physics_bit) && vehicle->vehicle.speed != 0.f) ||
			(TEST_FLAG(definition->vehicle.flags, _vehicle_turn_wakes_physics_bit) && vehicle->vehicle.turn != 0.f) ||
			(TEST_FLAG(definition->vehicle.flags, _vehicle_driver_power_wakes_physics_bit) && vehicle->unit.seat_power[0] != 0.f) ||
			(TEST_FLAG(definition->vehicle.flags, _vehicle_gunner_power_wakes_physics_bit) && vehicle->unit.seat_power[1] != 0.f) ||
			(TEST_FLAG(definition->vehicle.flags, _vehicle_slide_wakes_physics_bit) && vehicle->vehicle.slide != 0.f)))
		{
			SET_FLAG(vehicle->object.flags, _object_at_rest_bit, FALSE);
		}

		match_collision_log_begin_user("c:\\halo\\SOURCE\\units\\vehicles.c", 308, _collision_user_vehicles);

		if (definition->object.physics.index == NONE || TEST_FLAG(vehicle->object.flags, _object_at_rest_bit))
		{
			if (vehicle->vehicle.stop_time > 0)
			{
				slowly_stop_vehicle(vehicle_index);
				update_suspension(vehicle_index);
			}
		}
		else
		{
			struct mass_point_datum mass_points[MAXIMUM_MASS_POINTS_PER_PHYSICS];
			struct powered_mass_point_datum powered_mass_points[MAXIMUM_POWERED_MASS_POINTS_PER_PHYSICS];
			real_vector3d old_velocity = vehicle->object.translational_velocity;

			switch (definition->vehicle.type)
			{
			case _vehicle_human_tank:
				update_human_tank_physics(vehicle_index, powered_mass_points, mass_points);
				break;
			case _vehicle_human_jeep:
				update_human_jeep_physics(vehicle_index, powered_mass_points, mass_points);
				break;
			case _vehicle_human_boat:
				update_human_boat_physics(vehicle_index, powered_mass_points, mass_points);
				break;
			case _vehicle_human_plane:
				update_human_plane_physics(vehicle_index, powered_mass_points, mass_points);
				break;
			case _vehicle_alien_scout:
				update_alien_scout_physics(vehicle_index, steering, powered_mass_points, mass_points);
				break;
			case _vehicle_alien_fighter:
				update_alien_fighter_physics(vehicle_index, powered_mass_points, mass_points);
				break;
			case _vehicle_turret:
				update_turret_physics(vehicle_index, powered_mass_points, mass_points);
				break;
			}

			create_slipping_effects(vehicle_index, powered_mass_points, mass_points);
			if (!update_suspension(vehicle_index))
			{
				create_crashing_effects(vehicle_index, &old_velocity, mass_points);
			}
			compute_airborne_ticks(vehicle_index, powered_mass_points, mass_points);

			if (TEST_FLAG(vehicle->object.flags, _object_at_rest_bit))
			{
				vehicle->vehicle.stop_time = 15;
			}

			if (!TEST_FLAG(vehicle->object.flags, _object_no_collisions_bit) && TEST_FLAG(_vehicle_mask_flying, definition->vehicle.type))
			{
				real vehicle_floor = global_structure_bsp_get()->vehicle_floor;
				real vehicle_ceiling = global_structure_bsp_get()->vehicle_ceiling;

				if (vehicle_floor != 0.f && vehicle->object.position.z < vehicle_floor)
				{
					vehicle->object.translational_velocity.k += ((vehicle_floor - vehicle->object.position.z) * (1.f / 64.f) -
						vehicle->object.translational_velocity.k * (1.f / 16.f)) * vehicle->unit.seat_power[0];
				}

				if (vehicle_ceiling != 0.f && vehicle->object.position.z > vehicle_ceiling)
				{
					vehicle->object.translational_velocity.k -= ((vehicle->object.position.z - vehicle_ceiling) * (1.f / 64.f) +
						vehicle->object.translational_velocity.k * (1.f / 16.f)) * vehicle->unit.seat_power[0];
				}
			}
		}

		match_collision_log_end_user("c:\\halo\\SOURCE\\units\\vehicles.c", 372);

		if (TEST_FLAG(definition->vehicle.flags, _vehicle_kills_riders_at_terminal_velocity_bit))
		{
			struct game_globals_falling_damage *falling_damage = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->falling_damage, 0, struct game_globals_falling_damage);

			if (-falling_damage->runtime_maximum_falling_velocity > vehicle->object.translational_velocity.k)
			{
				struct object_datum *object;
				long object_index;

				for (object_index = vehicle->object.first_child_object_index; object_index != NONE; object_index = object->object.next_object_index)
				{
					struct damage_data damage_data;

					object = object_get(object_index);
					damage_data_new(&damage_data, falling_damage->maximum_distance_damage.index);
					object_cause_damage(&damage_data, object_index, NONE, NONE, NONE, NULL);
				}
			}
		}
	}

	if (definition->object.animation_graph.index != NONE)
	{
		struct unit_animation_update_data data;

		data.state_desired = _unit_state_idle;
		data.crouching = FALSE;
		unit_update_animation(vehicle_index, &data);
	}

	{
		boolean blurred = fabs(vehicle->vehicle.speed) >= definition->vehicle.blur_speed;

		if (blurred != TEST_FLAG(vehicle->vehicle.flags, _vehicle_blurred_bit))
		{
			object_permute_region(vehicle_index, "~blur", NONE, blurred);
			SET_FLAG(vehicle->vehicle.flags, _vehicle_blurred_bit, blurred);
		}
	}

	profile_exit(vehicle_update_section);

	return TRUE;
}

void vehicle_preprocess_node_orientations(
	long vehicle_index,
	struct real_orientation *node_orientations)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	if (definition->object.animation_graph.index != NONE)
	{
		struct animation_graph *animation_graph = animation_graph_definition_get(definition->object.animation_graph.index);
		struct animation_graph_vehicle_animations *vehicle_animations = animation_graph->vehicle_animations.count ?
			TAG_BLOCK_GET_ELEMENT(&animation_graph->vehicle_animations, 0, struct animation_graph_vehicle_animations) : NULL;

		if (vehicle_animations)
		{
			short animation_index;
			short suspension_index;

			animation_index = vehicle_animations->animations.count > _vehicle_animation_steering ?
				animation_graph_animation_index_get(&vehicle_animations->animations)[_vehicle_animation_steering].animation_index : NONE;
			if (animation_index != NONE)
			{
				aiming_screen_apply(TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation),
					&vehicle_animations->steering_screen_bounds, vehicle->vehicle.turn, 0.f, node_orientations);
			}

			animation_index = vehicle_animations->animations.count > _vehicle_animation_roll ?
				animation_graph_animation_index_get(&vehicle_animations->animations)[_vehicle_animation_roll].animation_index : NONE;
			if (animation_index != NONE)
			{
				struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
				real roll = triple_product3d(&vehicle->object.up, &vehicle->object.forward, &vehicle->object.translational_velocity);
				real frame = (roll / definition->vehicle.speed.maximum_forward_speed + 1.f) * 0.5f;

				overlay_animation_apply_continuous(animation, (animation->frame_count - 1) * PIN(frame, 0.f, 1.f), node_orientations);
			}

			animation_index = vehicle_animations->animations.count > _vehicle_animation_throttle ?
				animation_graph_animation_index_get(&vehicle_animations->animations)[_vehicle_animation_throttle].animation_index : NONE;
			if (animation_index != NONE)
			{
				struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
				real frame;

				if (vehicle->vehicle.speed < 0.f)
				{
					frame = 0.5f - vehicle->vehicle.speed / definition->vehicle.speed.maximum_reverse_speed * 0.5f;
				}
				else
				{
					frame = (vehicle->vehicle.speed / definition->vehicle.speed.maximum_forward_speed + 1.f) * 0.5f;
				}

				overlay_animation_apply_continuous(animation, (animation->frame_count - 1) * frame, node_orientations);
			}

			animation_index = vehicle_animations->animations.count > _vehicle_animation_velocity ?
				animation_graph_animation_index_get(&vehicle_animations->animations)[_vehicle_animation_velocity].animation_index : NONE;
			if (animation_index != NONE)
			{
				struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
				real velocity = dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.forward);
				real frame = PIN(velocity, 0.f, 1.f) / (real)fabs(definition->vehicle.speed.maximum_forward_speed);

				overlay_animation_apply_continuous(animation, (animation->frame_count - 1) * PIN(frame, 0.f, 1.f), node_orientations);
			}

			animation_index = vehicle_animations->animations.count > _vehicle_animation_braking ?
				animation_graph_animation_index_get(&vehicle_animations->animations)[_vehicle_animation_braking].animation_index : NONE;
			if (animation_index != NONE)
			{
				TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
			}

			animation_index = vehicle_animations->animations.count > _vehicle_animation_ground_speed ?
				animation_graph_animation_index_get(&vehicle_animations->animations)[_vehicle_animation_ground_speed].animation_index : NONE;
			if (animation_index != NONE)
			{
				struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
				real frame;

				if (definition->vehicle.wheel_circumference > 0.f)
				{
					frame = vehicle->vehicle.wheel / definition->vehicle.wheel_circumference;
				}
				else
				{
					frame = 0.f;
				}

				overlay_animation_apply_continuous(animation, animation->frame_count * frame, node_orientations);
			}

			for (suspension_index = 0; suspension_index < vehicle_animations->suspension_animations.count; suspension_index++)
			{
				struct animation_graph_vehicle_suspension_animation *suspension = TAG_BLOCK_GET_ELEMENT(&vehicle_animations->suspension_animations,
					suspension_index, struct animation_graph_vehicle_suspension_animation);

				if (suspension->animation_index != NONE)
				{
					struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, suspension->animation_index, struct animation);
					real frame = dequantize_byte_to_real(0.f, 1.f, vehicle->vehicle.suspension[suspension_index]);

					overlay_animation_apply_continuous(animation, (animation->frame_count - 1) * frame, node_orientations);
				}
			}
		}
	}

	return;
}

boolean vehicle_causes_collision_damage(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	return TEST_FLAG(definition->vehicle.flags, _vehicle_causes_collision_damage_bit);
}

boolean vehicle_build_update(
	void)
{
	return TRUE;
}

boolean vehicle_parse_update(
	void)
{
	return TRUE;
}

void vehicle_accelerate(
	long vehicle_index,
	real_vector3d const *acceleration)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	if (definition->object.physics.index != NONE)
	{
		real_vector3d axis;
		real magnitude;

		physics_definition_get(definition->object.physics.index);

		add_vectors3d(&vehicle->object.translational_velocity, acceleration, &vehicle->object.translational_velocity);
		cross_product3d(global_up3d, acceleration, &axis);
		magnitude = normalize3d(&axis);
		if (magnitude > 0.f)
		{
			scale_vector3d(&axis, magnitude * _pi, &axis);
			add_vectors3d(&axis, &vehicle->object.angular_velocity, &vehicle->object.angular_velocity);
		}

		SET_FLAG(vehicle->object.flags, _object_at_rest_bit, FALSE);
	}

	return;
}

void vehicle_hover(
	long vehicle_index,
	boolean hover_flag)
{
	if (vehicle_index != NONE)
	{
		struct vehicle_datum *vehicle = vehicle_get(vehicle_index);

		if (hover_flag)
		{
			object_get_origin(vehicle_index, &vehicle->vehicle.hover_position);
			SET_FLAG(vehicle->vehicle.flags, _vehicle_hovering_bit, TRUE);
		}
		else
		{
			SET_FLAG(vehicle->vehicle.flags, _vehicle_hovering_bit, FALSE);
		}
	}

	return;
}

void vehicle_render_debug(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	if (definition->object.physics.index != NONE)
	{
		struct physics_definition *physics = physics_definition_get(definition->object.physics.index);

		if (debug_objects_vehicle_powered_mass_points)
		{
			short mass_point_index;

			for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
			{
			}
		}
	}

	return;
}

long vehicle_find_pathfinding_surface_index(
	long vehicle_index,
	real_point3d *pathfinding_point)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	long surface_index = NONE;

	object_get_origin(vehicle_index, pathfinding_point);

	switch (definition->vehicle.type)
	{
	case _vehicle_human_tank:
	case _vehicle_human_jeep:
	case _vehicle_alien_scout:
	case _vehicle_turret:
		{
			struct collision_bsp *collision_bsp = global_collision_bsp_get();
			struct collision_bsp_test_vector_result result;
			real_point3d collision_origin;
			real_vector3d collision_vector;

			object_get_origin(vehicle_index, &collision_origin);
			point_from_line3d(&collision_origin, global_up3d, 0.4f, &collision_origin);
			scale_vector3d(global_down3d, 2.f, &collision_vector);

			if (collision_bsp_test_vector(FLAG(_collision_bsp_test_front_facing_surfaces_bit), collision_bsp, 0, NULL, &collision_origin, &collision_vector, REAL_MAX, &result))
			{
				surface_index = result.surface_index;
				point_from_line3d(&collision_origin, &collision_vector, result.t, pathfinding_point);
			}
		}
		break;
	}

	return surface_index;
}

static real_vector3d *compute_acceleration(
	real_vector3d const *current_velocity,
	real_vector3d const *desired_velocity,
	real maximum_acceleration,
	real maximum_deceleration,
	real_vector3d *acceleration)
{
	real dot;

	subtract_vectors3d(desired_velocity, current_velocity, acceleration);
	acceleration->k += global_gravity;

	dot = dot_product3d(acceleration, desired_velocity);
	if (dot > _real_epsilon)
	{
		maximum_acceleration = (maximum_acceleration - maximum_deceleration) * ((dot * dot / magnitude_squared3d(acceleration)) / magnitude_squared3d(desired_velocity)) + maximum_deceleration;
	}
	else
	{
		maximum_acceleration = maximum_deceleration;
	}

	limit3d(acceleration, maximum_acceleration);

	return acceleration;
}

static void update_human_tank_physics(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);
	real left_tread_speed = vehicle->vehicle.speed - vehicle->vehicle.turn;
	real right_tread_speed = vehicle->vehicle.speed + vehicle->vehicle.turn;

	vehicle->vehicle.left_tread += left_tread_speed;
	vehicle->vehicle.left_tread = fmod(vehicle->vehicle.left_tread, definition->vehicle.wheel_circumference);
	if (vehicle->vehicle.left_tread < 0.f)
	{
		vehicle->vehicle.left_tread += definition->vehicle.wheel_circumference;
	}

	vehicle->vehicle.right_tread += right_tread_speed;
	vehicle->vehicle.right_tread = fmod(vehicle->vehicle.right_tread, definition->vehicle.wheel_circumference);
	if (vehicle->vehicle.right_tread < 0.f)
	{
		vehicle->vehicle.right_tread += definition->vehicle.wheel_circumference;
	}

	if (physics->powered_mass_points.count == 2)
	{
		powered_mass_points[0].ground_friction_velocity = left_tread_speed;
		set_real_quaternion(&powered_mass_points[0].rotation, 0.f, 0.f, 0.f, 1.f);
		powered_mass_points[1].ground_friction_velocity = right_tread_speed;
		set_real_quaternion(&powered_mass_points[1].rotation, 0.f, 0.f, 0.f, 1.f);

		physics_update(vehicle_index, powered_mass_points, mass_points, NULL, NULL);
	}
	else
	{
		physics_update(vehicle_index, NULL, mass_points, NULL, NULL);
	}

	return;
}

static void update_human_jeep_physics(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);

	vehicle->vehicle.wheel += vehicle->vehicle.speed;
	vehicle->vehicle.wheel = (real)fmod(vehicle->vehicle.wheel, definition->vehicle.wheel_circumference);
	if (vehicle->vehicle.wheel < 0.f)
	{
		vehicle->vehicle.wheel += definition->vehicle.wheel_circumference;
	}

	if (physics->powered_mass_points.count == 2)
	{
		real angle = 0.5f * vehicle->vehicle.turn;
		real cosine_angle = cosine(angle);
		real sine_angle = sine(angle);

		powered_mass_points[0].ground_friction_velocity = vehicle->vehicle.speed;
		set_real_quaternion(&powered_mass_points[0].rotation, 0.f, 0.f, sine_angle, cosine_angle);
		powered_mass_points[1].ground_friction_velocity = vehicle->vehicle.speed;
		set_real_quaternion(&powered_mass_points[1].rotation, 0.f, 0.f, -sine_angle, cosine_angle);

		physics_update(vehicle_index, powered_mass_points, mass_points, NULL, NULL);
	}
	else
	{
		physics_update(vehicle_index, NULL, mass_points, NULL, NULL);
	}

	return;
}

static void update_human_boat_physics(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);

	if (physics->powered_mass_points.count == 3)
	{
		real_vector3d magic_force;
		real_vector3d magic_torque;
		real_vector3d desired_up;
		real const maximum_speed = 0.4f;
		real speed = fabs(magnitude3d(&vehicle->object.translational_velocity) / maximum_speed);
		real angle = 0.5f * vehicle->vehicle.turn;

		if (speed > 1.f)
		{
			speed = 1.f;
		}
		angle = (1.f - speed) * angle;

		powered_mass_points[0].water_friction_velocity = vehicle->vehicle.speed;
		powered_mass_points[0].water_lift_ratio = 0.003f;
		set_real_quaternion(&powered_mass_points[0].rotation, 0.f, 0.f, sine(angle), cosine(angle));
		powered_mass_points[1].water_lift_ratio = 0.003f;
		set_real_quaternion(&powered_mass_points[1].rotation, 0.f, 0.f, 0.f, 1.f);
		powered_mass_points[2].water_lift_ratio = 0.005f;
		set_real_quaternion(&powered_mass_points[2].rotation, 0.f, 0.f, 0.f, 1.f);

		set_real_vector3d(&magic_force, 0.f, 0.f, 0.f);
		vector_from_line3d(global_up3d, &vehicle->object.forward, -vehicle->object.forward.k, &desired_up);
		if (normalize3d(&desired_up) != 0.f)
		{
			real const maximum_roll_acceleration = 4.f * _pi / (TICKS_PER_SECOND * TICKS_PER_SECOND);
			real_vector3d left;
			real_vector3d forward_cross_velocity;
			real roll;
			real angle;
			real roll_velocity;
			real desired_roll_velocity;
			real roll_acceleration;
			real torque;

			cross_product3d(&vehicle->object.up, &vehicle->object.forward, &left);
			roll = dot_product3d(cross_product3d(&vehicle->object.forward, &vehicle->object.translational_velocity, &forward_cross_velocity), global_up3d) * (2.f * _pi);
			rotate_vector_about_axis(&desired_up, &vehicle->object.forward, sine(roll), cosine(roll));
			angle = angle_between_vectors3d(&desired_up, &vehicle->object.up);
			if (dot_product3d(&desired_up, &left) > 0.f)
			{
				angle = -angle;
			}

			roll_velocity = dot_product3d(&vehicle->object.angular_velocity, &vehicle->object.forward);
			desired_roll_velocity = (angle != 0.f ? (angle < 0.f ? -1 : 1) : 0) * square_root(fabs(angle) * 2.f * maximum_roll_acceleration);
			roll_acceleration = PIN(desired_roll_velocity - roll_velocity, -maximum_roll_acceleration, maximum_roll_acceleration);
			torque = physics->xx_moment * roll_acceleration;
			scale_vector3d(&vehicle->object.forward, torque, &magic_torque);
		}
		else
		{
			set_real_vector3d(&magic_torque, 0.f, 0.f, 0.f);
		}

		physics_update(vehicle_index, powered_mass_points, mass_points, &magic_force, &magic_torque);
	}
	else
	{
		physics_update(vehicle_index, NULL, mass_points, NULL, NULL);
	}

	return;
}

static void update_human_plane_physics(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);

	if (TEST_FLAG(vehicle->vehicle.flags, _vehicle_hovering_bit))
	{
		memset(mass_points, 0, physics->mass_points.count * sizeof(struct mass_point_datum));
	}
	else
	{
		real throttle = PIN(vehicle->vehicle.speed, 0.f, definition->vehicle.speed.maximum_forward_speed) / definition->vehicle.speed.maximum_forward_speed;
		real desired_hover;
		real_vector3d desired_forward;
		real_vector3d desired_up;
		real_vector3d left;
		real_vector3d magic_force;
		real_vector3d magic_torque;

		throttle *= throttle;
		desired_hover = vehicle->unit.seat_power[0] *
			(!TEST_FLAG(vehicle->vehicle.flags, _vehicle_control_crouch_bit) ? (TEST_FLAG(vehicle->vehicle.flags, _vehicle_control_jump_bit) ? 1.f : 0.75f) : 0.25f) *
			(1.f - throttle);
		interpolate_scalar(&vehicle->vehicle.hover, desired_hover, 0.05f);
		vehicle->vehicle.thrust = vehicle->unit.seat_power[0] * throttle;

		desired_forward = vehicle->unit.desired_facing_vector;
		set_real_vector3d(&desired_up, -desired_forward.i * desired_forward.k, -desired_forward.j * desired_forward.k, 1.f - desired_forward.k * desired_forward.k);
		if (normalize3d(&desired_up) == 0.f)
		{
			set_real_vector3d(&desired_up, 1.f, 0.f, 0.f);
		}

		cross_product3d(&vehicle->object.up, &vehicle->object.forward, &left);

		{
			real const thrust_scale = 0.05f;
			real const hover_lift_scale = 1.3f;
			real const speed_lift_scale = 1.05f;
			real speed = dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.forward);
			real thrust = ((vehicle->vehicle.speed - speed) * vehicle->vehicle.thrust) * physics->mass * thrust_scale;
			real speed_fraction = fabs(speed / definition->vehicle.speed.maximum_forward_speed);
			real lift = (vehicle->vehicle.hover * hover_lift_scale + speed_fraction * speed_lift_scale) * physics->mass * global_gravity;

			magic_force.i = thrust * vehicle->object.forward.i + lift * vehicle->object.up.i;
			magic_force.j = thrust * vehicle->object.forward.j + lift * vehicle->object.up.j;
			magic_force.k = thrust * vehicle->object.forward.k + lift * vehicle->object.up.k;
		}

		{
			real_matrix4x3 current_matrix;
			real_matrix4x3 desired_matrix;
			real_matrix4x3 rotation_matrix;
			real const yaw_scale = _pi2;
			real const angular_velocity_scale = 1.f / 30.f;
			real const torque_scale = 0.05f;
			real_vector2d velocity2d;
			real yaw;
			real_quaternion rotation_quaternion;
			real_vector3d axis;
			real angle;
			real scale;
			real_vector3d desired_angular_velocity;
			real torque;

			velocity2d = *(real_vector2d const *)&vehicle->object.translational_velocity;
			yaw = cross_product2d((real_vector2d const *)&desired_forward, &velocity2d) * yaw_scale / (real)fabs(definition->vehicle.speed.maximum_forward_speed);
			yaw_vectors(&desired_up, &desired_forward, sine(yaw), cosine(yaw));

			matrix4x3_rotation_from_vectors(&current_matrix, &vehicle->object.forward, &vehicle->object.up);
			matrix4x3_rotation_from_vectors(&desired_matrix, &desired_forward, &desired_up);
			matrix4x3_inverse(&desired_matrix, &desired_matrix);
			matrix4x3_multiply(&current_matrix, &desired_matrix, &rotation_matrix);
			matrix4x3_rotation_to_quaternion(&rotation_matrix, &rotation_quaternion);
			quaternion_to_angle_and_vector(&rotation_quaternion, &angle, &axis);

			scale = angle * angular_velocity_scale;
			desired_angular_velocity.i = scale * axis.i;
			desired_angular_velocity.j = scale * axis.j;
			desired_angular_velocity.k = scale * axis.k;

			torque = torque_scale * physics->mass * physics->radius * physics->radius;
			magic_torque.i = (desired_angular_velocity.i - vehicle->object.angular_velocity.i) * torque;
			magic_torque.j = (desired_angular_velocity.j - vehicle->object.angular_velocity.j) * torque;
			magic_torque.k = (desired_angular_velocity.k - vehicle->object.angular_velocity.k) * torque;
		}

		scale_vector3d(&magic_force, vehicle->unit.seat_power[0], &magic_force);
		scale_vector3d(&magic_torque, vehicle->unit.seat_power[0], &magic_torque);

		physics_update(vehicle_index, NULL, mass_points, &magic_force, &magic_torque);
	}

	create_pelican_effect(vehicle_index);

	return;
}

static void update_alien_scout_physics(
	long vehicle_index,
	real steering,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);
	real const maximum_water_depth = 0.5f;
	real water_depth = scenario_location_water_depth(&vehicle->object.location, &vehicle->object.position);
	real_vector3d magic_force = *global_zero_vector3d;
	real_vector3d magic_torque = *global_zero_vector3d;
	real power = vehicle->unit.seat_power[0];
	short powered_mass_point_index;

	for (powered_mass_point_index = 0; powered_mass_point_index < physics->powered_mass_points.count; powered_mass_point_index++)
	{
		struct powered_mass_point_datum *powered_mass_point = &powered_mass_points[powered_mass_point_index];

		powered_mass_point->antigrav_fraction = power;
		set_real_quaternion(&powered_mass_point->rotation, 0.f, 0.f, 0.f, 1.f);
	}

	if (water_depth < maximum_water_depth && vehicle->object.up.k > -0.2f)
	{
		real_matrix4x3 vehicle_matrix;
		real_vector3d velocity;

		matrix4x3_from_point_and_vectors(&vehicle_matrix, &vehicle->object.position, &vehicle->object.forward, &vehicle->object.up);
		matrix4x3_inverse_transform_vector(&vehicle_matrix, &vehicle->object.translational_velocity, &velocity);

		if (vehicle->vehicle.hover > 0.f)
		{
			real maximum_acceleration;
			real_vector3d desired_velocity;
			real_vector3d desired_acceleration;
			real maximum_speed = definition->vehicle.speed.maximum_forward_speed;

			if (TEST_FLAG(vehicle->vehicle.flags, _vehicle_control_jump_bit))
			{
				real const boost_scale = 0.8f;

				maximum_speed *= boost_scale;
			}

			scale_vector3d(&vehicle->unit.throttle, maximum_speed, &desired_velocity);
			subtract_vectors3d(&desired_velocity, &velocity, &desired_acceleration);
			desired_acceleration.k = 0.f;
			maximum_acceleration = definition->vehicle.speed.acceleration;
			if (vehicle->vehicle.on_ground_ticks > 0 && fabs(steering) > _pi / 4.f)
			{
				real const decay_rate = 0.05f;
				real const maximum_decay = 0.98f;
				real decay = MIN(vehicle->vehicle.on_ground_ticks * decay_rate, maximum_decay);

				maximum_acceleration = (1.f - decay) * maximum_acceleration;
			}

			limit3d(&desired_acceleration, maximum_acceleration);
			matrix4x3_transform_vector(&vehicle_matrix, &desired_acceleration, &desired_acceleration);
			point_from_line3d((real_point3d *)&magic_force, &desired_acceleration, physics->mass * vehicle->vehicle.hover, (real_point3d *)&magic_force);
		}

		if (vehicle->vehicle.hover > 0.f)
		{
			real const maximum_angular_acceleration = _pi / (TICKS_PER_SECOND * TICKS_PER_SECOND);
			real desired_steering = steering;
			real angular_velocity = dot_product3d(&vehicle->object.angular_velocity, &vehicle->object.up);
			real desired_angular_velocity;
			real angular_acceleration;
			real torque;

			desired_angular_velocity = (desired_steering != 0.f ? (desired_steering < 0.f ? -1 : 1) : 0) *
				square_root(fabs(desired_steering) * 2.f * maximum_angular_acceleration);
			if (fabs(desired_angular_velocity) > _real_epsilon && desired_steering / desired_angular_velocity < 2.f)
			{
				desired_angular_velocity = 0.5f * desired_steering;
			}

			angular_acceleration = PIN(desired_angular_velocity - angular_velocity, -maximum_angular_acceleration, maximum_angular_acceleration);
			torque = physics->zz_moment * angular_acceleration;
			point_from_line3d((real_point3d *)&magic_torque, &vehicle->object.up, (real)(torque * vehicle->vehicle.hover), (real_point3d *)&magic_torque);
		}

		if (vehicle->vehicle.hover < 1.f)
		{
			real const maximum_alpha = DEGREES_TO_RADIANS(80.f) / (TICKS_PER_SECOND * TICKS_PER_SECOND);
			real_vector2d applied_alpha = *global_zero_vector2d;
			real_vector3d vehicle_left;
			real_vector2d vehicle_forward_horizontal;
			real_vector2d vehicle_left_horizontal;
			real_vector3d turning_torque;

			cross_product3d(&vehicle->object.up, &vehicle->object.forward, &vehicle_left);
			vehicle_forward_horizontal = *(real_vector2d const *)&vehicle->object.forward;
			vehicle_left_horizontal = *(real_vector2d const *)&vehicle_left;
			normalize2d(&vehicle_forward_horizontal);
			normalize2d(&vehicle_left_horizontal);

			if (vehicle->object.up.k > 0.f)
			{
				real_vector2d tilt;
				real_vector2d tilt_velocity;
				real_vector2d alpha_desire;
				real const alpha_sign = 1.f;
				real_vector2d alpha_scale;

				tilt.i = dot_product2d((real_vector2d const *)&vehicle->object.up, &vehicle_forward_horizontal);
				tilt.j = dot_product2d((real_vector2d const *)&vehicle->object.up, &vehicle_left_horizontal);
				tilt_velocity.i = dot_product2d((real_vector2d const *)&vehicle->object.angular_velocity, &vehicle_left_horizontal);
				tilt_velocity.j = -dot_product2d((real_vector2d const *)&vehicle->object.angular_velocity, &vehicle_forward_horizontal);

				alpha_desire = *global_zero_vector2d;
				point_from_line2d((real_point2d *)&alpha_desire, &tilt, -1.f, (real_point2d *)&alpha_desire);
				point_from_line2d((real_point2d *)&alpha_desire, &tilt_velocity, -15.f, (real_point2d *)&alpha_desire);

				alpha_scale.i = (alpha_desire.i * vehicle->unit.throttle.i != 0.f ? (alpha_desire.i * vehicle->unit.throttle.i < 0.f ? -1 : 1) : 0) * alpha_sign * fabs(alpha_desire.i);
				alpha_scale.j = (alpha_desire.j * vehicle->unit.throttle.j != 0.f ? (alpha_desire.j * vehicle->unit.throttle.j < 0.f ? -1 : 1) : 0) * alpha_sign * fabs(alpha_desire.j);
				applied_alpha.i += maximum_alpha * vehicle->unit.throttle.i * PIN(alpha_scale.i + 1.f, 0.3f, 2.5f);
				applied_alpha.j += maximum_alpha * vehicle->unit.throttle.j * PIN(alpha_scale.j + 1.f, 0.3f, 2.5f);

				{
					real const leveling_alpha = DEGREES_TO_RADIANS(200.f) / (TICKS_PER_SECOND * TICKS_PER_SECOND);

					applied_alpha.i += leveling_alpha * alpha_desire.i * (1.f - vehicle->object.up.k);
					applied_alpha.j += leveling_alpha * alpha_desire.j * (1.f - vehicle->object.up.k);
				}
			}
			else
			{
				applied_alpha.i += maximum_alpha * vehicle->unit.throttle.i;
				applied_alpha.j += maximum_alpha * vehicle->unit.throttle.j;
			}

			turning_torque = *global_zero_vector3d;
			point_from_line3d((real_point3d *)&turning_torque, &vehicle_left, applied_alpha.i * physics->yy_moment, (real_point3d *)&turning_torque);
			point_from_line3d((real_point3d *)&turning_torque, &vehicle->object.forward, -applied_alpha.j * physics->xx_moment, (real_point3d *)&turning_torque);
			point_from_line3d((real_point3d *)&magic_torque, &turning_torque, (real)(1.f - vehicle->vehicle.hover), (real_point3d *)&magic_torque);
		}

		if (TEST_FLAG(vehicle->vehicle.flags, _vehicle_control_jump_bit))
		{
			real speed = dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.forward);
			real_vector3d vehicle_left;

			speed /= definition->vehicle.speed.maximum_forward_speed;
			speed = PIN(speed, 0.f, 1.f);
			cross_product3d(&vehicle->object.up, &vehicle->object.forward, &vehicle_left);
			if (speed > 0.f)
			{
				real const boost_pitch_alpha = DEGREES_TO_RADIANS(300.f) / (TICKS_PER_SECOND * TICKS_PER_SECOND);
				real const boost_lift_acceleration = 0.004f;

				point_from_line3d((real_point3d *)&magic_torque, &vehicle_left, -boost_pitch_alpha * physics->yy_moment * speed * vehicle->vehicle.hover, (real_point3d *)&magic_torque);
				point_from_line3d((real_point3d *)&magic_force, global_up3d, boost_lift_acceleration * physics->mass * speed * vehicle->vehicle.hover, (real_point3d *)&magic_force);
			}

			if (vehicle->vehicle.airborne_ticks > 0)
			{
				real_vector3d vehicle_horizontal_forward;

				cross_product3d(&vehicle_left, global_up3d, &vehicle_horizontal_forward);
				if (normalize3d(&vehicle_horizontal_forward) > 0.f)
				{
					real const forward_acceleration = 0.002f;
					real const lift_acceleration = 0.001f;
					real const airborne_ticks = 30.f;
					real scale = PIN(1.f - vehicle->vehicle.airborne_ticks / airborne_ticks, 0.f, 1.f);

					point_from_line3d((real_point3d *)&magic_force, &vehicle_horizontal_forward, forward_acceleration * scale * physics->mass * (1.f - vehicle->vehicle.hover), (real_point3d *)&magic_force);
					point_from_line3d((real_point3d *)&magic_force, global_up3d, lift_acceleration * scale * physics->mass * (1.f - vehicle->vehicle.hover), (real_point3d *)&magic_force);
				}
			}
		}

		scale_vector3d(&magic_force, power, &magic_force);
		scale_vector3d(&magic_torque, power, &magic_torque);
	}

	physics_update(vehicle_index, powered_mass_points, mass_points, &magic_force, &magic_torque);

	{
		real maximum_hover = MAX(0.4f, vehicle->object.up.k);
		real hover = 0.f;
		short powered_mass_point_count = 0;
		short antigraving_mass_point_count = 0;
		short mass_point_index;

		for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
		{
			struct mass_point_definition *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&physics->mass_points, mass_point_index, struct mass_point_definition);

			if (mass_point_definition->powered_mass_point_index != NONE)
			{
				struct mass_point_datum *mass_point = &mass_points[mass_point_index];

				powered_mass_point_count++;
				if (TEST_FLAG(mass_point->flags, _point_antigraving_bit))
				{
					antigraving_mass_point_count++;
				}
			}
		}

		if (powered_mass_point_count > 0)
		{
			hover = (real)antigraving_mass_point_count / (real)powered_mass_point_count;
		}

		{
			real const maximum_hover_change = 0.1f;
			real desired_hover = PIN(maximum_hover * hover, 0.f, 1.f);
			real hover_change = desired_hover - vehicle->vehicle.hover;

			if (hover_change > maximum_hover_change)
			{
				vehicle->vehicle.hover += maximum_hover_change;
			}
			else if (hover_change < -maximum_hover_change)
			{
				vehicle->vehicle.hover -= maximum_hover_change;
			}
			else
			{
				vehicle->vehicle.hover = desired_hover;
			}
		}
	}

	create_ghost_effect(vehicle_index);

	return;
}

static void update_alien_fighter_physics(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	if (physics_definition_get(definition->object.physics.index)->radius > 0.f)
	{
		update_alien_fighter_physics_old(vehicle_index, powered_mass_points, mass_points);
	}
	else
	{
		update_alien_fighter_physics_new(vehicle_index, powered_mass_points, mass_points);
	}

	create_ghost_effect(vehicle_index);

	return;
}

static void update_alien_fighter_physics_new(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);

	if (physics->powered_mass_points.count == 2)
	{
		real_vector3d magic_force;
		real_vector3d magic_torque;
		real_vector3d desired_angular_velocity;
		real_vector3d angular_acceleration;
		real spin;
		real thrust_delta;

		{
			real_vector3d desired_velocity;
			real_vector3d acceleration;
			real throttle;

			scale_vector3d(&vehicle->object.forward, vehicle->vehicle.speed, &desired_velocity);
			if (vehicle->vehicle.speed > 0.f)
			{
				throttle = vehicle->vehicle.speed / definition->vehicle.speed.maximum_forward_speed;
			}
			else
			{
				throttle = -(vehicle->vehicle.speed / definition->vehicle.speed.maximum_reverse_speed);
			}

			compute_acceleration(&vehicle->object.translational_velocity, &desired_velocity,
				throttle * definition->vehicle.speed.acceleration, throttle * definition->vehicle.speed.deceleration, &acceleration);
			scale_vector3d(&acceleration, physics->mass, &magic_force);
			scale_vector3d(&magic_force, vehicle->unit.seat_power[0], &magic_force);
		}

		{
			real_matrix3x3 current_rotation;
			real_matrix3x3 desired_rotation;
			real_matrix3x3 rotation;
			real_quaternion rotation_quaternion;
			real_vector3d axis;
			real yaw;
			real angle;

			matrix3x3_from_forward_and_up(&current_rotation, &vehicle->object.forward, &vehicle->object.up);
			desired_rotation.forward = vehicle->unit.desired_facing_vector;
			vector_from_line3d(global_up3d, &desired_rotation.forward, -desired_rotation.forward.k, &desired_rotation.up);
			if (normalize3d(&desired_rotation.up) == 0.f)
			{
				desired_rotation.up = *global_forward3d;
			}

			if (!unit_driven_by_ai(vehicle_index))
			{
				pitch_vectors(&desired_rotation.forward, &desired_rotation.up, sine(definition->vehicle.fixed_gun_pitch), cosine(definition->vehicle.fixed_gun_pitch));
			}

			yaw = cross_product2d((real_vector2d const *)&desired_rotation.forward, (real_vector2d const *)&vehicle->object.translational_velocity) /
				definition->vehicle.speed.maximum_forward_speed * definition->vehicle.turn.maximum_position;
			yaw_vectors(&desired_rotation.up, &desired_rotation.forward, sine(yaw), cosine(yaw));
			cross_product3d(&desired_rotation.up, &desired_rotation.forward, &desired_rotation.left);

			matrix3x3_transpose(&current_rotation, &current_rotation);
			matrix3x3_multiply(&desired_rotation, &current_rotation, &rotation);
			matrix3x3_rotation_to_quaternion(&rotation, &rotation_quaternion);
			quaternion_to_angle_and_vector(&rotation_quaternion, &angle, &axis);
			angle = -angle;
			scale_vector3d(&axis, definition->vehicle.turn_rate * angle / _pi, &desired_angular_velocity);
		}

		subtract_vectors3d(&desired_angular_velocity, &vehicle->object.angular_velocity, &angular_acceleration);
		scale_vector3d(&angular_acceleration, (physics->xx_moment + physics->yy_moment + physics->zz_moment) / 3.f, &magic_torque);
		scale_vector3d(&magic_torque, vehicle->unit.seat_power[0], &magic_torque);

		spin = magnitude3d(&vehicle->object.angular_velocity) / definition->vehicle.turn_rate;
		if (spin > vehicle->vehicle.thrust)
		{
			thrust_delta = PIN((1.f - vehicle->vehicle.thrust) * (1.f - vehicle->vehicle.thrust) * 0.2f, 0.01f, 0.05f);
			thrust_delta = MIN(spin - vehicle->vehicle.thrust, thrust_delta);
		}
		else
		{
			thrust_delta = -MAX(vehicle->vehicle.thrust * vehicle->vehicle.thrust * 0.05f, 0.005f);
			thrust_delta = MAX(spin - vehicle->vehicle.thrust, thrust_delta);
		}
		vehicle->vehicle.thrust += thrust_delta;

		powered_mass_points[0].antigrav_fraction = vehicle->unit.seat_power[0];
		powered_mass_points[0].rotation = *global_identity_quaternion;
		powered_mass_points[1].antigrav_fraction = vehicle->unit.seat_power[0];
		powered_mass_points[1].rotation = *global_identity_quaternion;

		physics_update(vehicle_index, powered_mass_points, mass_points, &magic_force, &magic_torque);
	}
	else
	{
		physics_update(vehicle_index, NULL, mass_points, NULL, NULL);
	}

	return;
}

static void update_alien_fighter_physics_old(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);

	if (physics->powered_mass_points.count == 2)
	{
		real_vector3d desired_forward = vehicle->unit.desired_facing_vector;
		real_vector3d desired_up = *global_up3d;
		real_vector3d left;
		real_vector3d magic_force;
		real_vector3d magic_torque;

		set_real_vector3d(&desired_up, -desired_forward.i * desired_forward.k, -desired_forward.j * desired_forward.k, 1.f - desired_forward.k * desired_forward.k);
		if (normalize3d(&desired_up) == 0.f)
		{
			set_real_vector3d(&desired_up, 1.f, 0.f, 0.f);
		}

		cross_product3d(&vehicle->object.up, &vehicle->object.forward, &left);

		{
			real const thrust_scale = 0.05f;
			real const lift_scale = 1.05f;
			real speed = dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.forward);
			real thrust = thrust_scale * physics->mass * (vehicle->vehicle.speed - speed);
			real speed_fraction = fabs(speed / definition->vehicle.speed.maximum_forward_speed);
			real lift = speed_fraction * lift_scale * physics->mass * global_gravity;

			magic_force.i = thrust * vehicle->object.forward.i + lift * vehicle->object.up.i;
			magic_force.j = thrust * vehicle->object.forward.j + lift * vehicle->object.up.j;
			magic_force.k = thrust * vehicle->object.forward.k + lift * vehicle->object.up.k;
		}

		{
			real_matrix4x3 current_matrix;
			real_matrix4x3 desired_matrix;
			real_matrix4x3 rotation_matrix;
			real const yaw_scale = _pi2;
			real const angular_velocity_scale = 4.f / 30.f;
			real const torque_scale = 0.05f;
			real_vector2d forward2d;
			real_vector2d velocity2d;
			real yaw;
			real_quaternion rotation_quaternion;
			real_vector3d axis;
			real angle;
			real scale;
			real_vector3d desired_angular_velocity;
			real torque;

			forward2d.i = desired_forward.i;
			forward2d.j = desired_forward.j;
			velocity2d = *(real_vector2d const *)&vehicle->object.translational_velocity;
			yaw = (cross_product2d(&forward2d, &velocity2d) * yaw_scale) / (real)fabs(definition->vehicle.speed.maximum_forward_speed);
			yaw_vectors(&desired_up, &desired_forward, sine(yaw), cosine(yaw));

			matrix4x3_rotation_from_vectors(&current_matrix, &vehicle->object.forward, &vehicle->object.up);
			matrix4x3_rotation_from_vectors(&desired_matrix, &desired_forward, &desired_up);
			matrix4x3_inverse(&desired_matrix, &desired_matrix);
			matrix4x3_multiply(&current_matrix, &desired_matrix, &rotation_matrix);
			matrix4x3_rotation_to_quaternion(&rotation_matrix, &rotation_quaternion);
			quaternion_to_angle_and_vector(&rotation_quaternion, &angle, &axis);

			scale = angle * angular_velocity_scale;
			desired_angular_velocity.i = scale * axis.i;
			desired_angular_velocity.j = scale * axis.j;
			desired_angular_velocity.k = scale * axis.k;

			torque = torque_scale * physics->mass * physics->radius * physics->radius;
			magic_torque.i = (desired_angular_velocity.i - vehicle->object.angular_velocity.i) * torque;
			magic_torque.j = (desired_angular_velocity.j - vehicle->object.angular_velocity.j) * torque;
			magic_torque.k = (desired_angular_velocity.k - vehicle->object.angular_velocity.k) * torque;
		}

		powered_mass_points[0].antigrav_fraction = vehicle->unit.seat_power[0];
		powered_mass_points[0].rotation.w = 1.f;
		powered_mass_points[0].rotation.v.i = 0.f;
		powered_mass_points[0].rotation.v.j = 0.f;
		powered_mass_points[0].rotation.v.k = 0.f;

		powered_mass_points[1].antigrav_fraction = vehicle->unit.seat_power[0];
		powered_mass_points[1].rotation.w = 1.f;
		powered_mass_points[1].rotation.v.i = 0.f;
		powered_mass_points[1].rotation.v.j = 0.f;
		powered_mass_points[1].rotation.v.k = 0.f;

		scale_vector3d(&magic_force, vehicle->unit.seat_power[0], &magic_force);
		scale_vector3d(&magic_torque, vehicle->unit.seat_power[0], &magic_torque);

		physics_update(vehicle_index, powered_mass_points, mass_points, &magic_force, &magic_torque);
	}
	else
	{
		physics_update(vehicle_index, NULL, mass_points, NULL, NULL);
	}

	return;
}

static void update_turret_physics(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	physics_update(vehicle_index, NULL, mass_points, NULL, NULL);

	return;
}

boolean vehicle_is_flipped(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);

	return vehicle->object.up.k < 0.2f;
}

static void slowly_stop_vehicle(
	long vehicle_index)
{
	real const velocity_scale = 0.835f;
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real_matrix4x3 rotation;

	vehicle->vehicle.stop_time--;
	scale_vector3d(&vehicle->object.translational_velocity, velocity_scale, &vehicle->object.translational_velocity);
	scale_vector3d(&vehicle->object.angular_velocity, velocity_scale, &vehicle->object.angular_velocity);
	add_vectors3d((real_vector3d *)&vehicle->object.position, &vehicle->object.translational_velocity, (real_vector3d *)&position);

	{
		real_vector3d axis = vehicle->object.angular_velocity;
		real angle = normalize3d(&axis);

		if (angle != 0.f)
		{
			matrix4x3_rotation_from_axis_and_angle(&rotation, &axis, sine(angle), cosine(angle));
			matrix4x3_transform_vector(&rotation, &vehicle->object.forward, &forward);
			matrix4x3_transform_vector(&rotation, &vehicle->object.up, &up);
		}
		else
		{
			forward = vehicle->object.forward;
			up = vehicle->object.up;
		}
	}

	if (!vehicle->vehicle.stop_time)
	{
		vehicle->object.translational_velocity = *global_zero_vector3d;
		vehicle->object.angular_velocity = *global_zero_vector3d;
	}

	object_set_position(vehicle_index, &position, &forward, &up);

	return;
}

static void create_pelican_effect(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	if (definition->vehicle.effect.index != NONE)
	{
		struct object_marker thrusters[16];
		short hover_thruster_count = object_get_marker_by_name(vehicle_index, "hover thrusters", thrusters, 15);
		short jet_thruster_count = object_get_marker_by_name(vehicle_index, "jet thrusters", &thrusters[hover_thruster_count], (short)(16 - hover_thruster_count));
		short thruster_index;

		for (thruster_index = 0; thruster_index < hover_thruster_count + jet_thruster_count; thruster_index++)
		{
			struct object_marker *thruster = &thrusters[thruster_index];
			struct collision_result collision;
			real_vector3d direction;
			real_vector3d vector;

			local_random_vector_in_cone3d(&thruster->matrix.forward, 0.f, _pi / 12.f, &direction);
			scale_vector3d(&direction, (thruster_index < hover_thruster_count ? vehicle->vehicle.hover : vehicle->vehicle.thrust) * 6.f + 2.f, &vector);

			if (collision_test_vector(_collision_test_environment_flags, &thruster->matrix.position, &vector, vehicle_index, &collision))
			{
				char const *fake_marker_names[3];
				real_point3d fake_marker_points[3];
				real_vector3d fake_marker_forwards[3];

				fake_marker_names[0] = "incident";
				fake_marker_points[0] = collision.point;
				negate_vector3d(&direction, &fake_marker_forwards[0]);

				fake_marker_names[1] = "normal";
				fake_marker_points[1] = collision.point;
				fake_marker_forwards[1] = collision.plane.n;

				fake_marker_names[2] = "reflected";
				fake_marker_points[2] = collision.point;
				reflect_vector3d(&direction, &collision.plane.n, &fake_marker_forwards[2]);

				effect_new_unattached_from_markers(definition->vehicle.effect.index, NONE, NULL, 3, fake_marker_names, fake_marker_points, fake_marker_forwards,
					1.f - collision.t, 1.f - collision.t, NULL, NULL, TRUE);
			}
		}
	}

	return;
}

static void create_ghost_effect(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	if (definition->vehicle.effect.index != NONE && vehicle->unit.seat_power[0] > 0.f)
	{
		struct object_marker thrusters[16];
		short hover_thruster_count = object_get_marker_by_name(vehicle_index, "hover thrusters", thrusters, 15);
		short jet_thruster_count = 0;
		short thruster_index;

		for (thruster_index = 0; thruster_index < hover_thruster_count + jet_thruster_count; thruster_index++)
		{
			struct object_marker *thruster = &thrusters[thruster_index];
			struct collision_result collision;
			real_vector3d direction;
			real_vector3d vector;

			local_random_vector_in_cone3d(&thruster->matrix.forward, 0.f, 15.f, &direction);
			scale_vector3d(&direction, 1.f, &vector);

			if (collision_test_vector(_collision_test_environment_flags, &thruster->matrix.position, &vector, vehicle_index, &collision))
			{
				real scale = PIN((-thruster->matrix.forward.k) * (1.f - collision.t) * vehicle->unit.seat_power[0], 0.f, 1.f);

				if (scale > 0.f)
				{
					real_point3d midpoint;
					char const *fake_marker_names[4];
					real_point3d fake_marker_points[4];
					real_vector3d fake_marker_forwards[4];

					midpoint.x = (collision.point.x + thruster->matrix.position.x) * 0.5f;
					midpoint.y = (collision.point.y + thruster->matrix.position.y) * 0.5f;
					midpoint.z = (collision.point.z + thruster->matrix.position.z) * 0.5f;

					fake_marker_names[0] = "incident";
					fake_marker_points[0] = collision.point;
					negate_vector3d(&direction, &fake_marker_forwards[0]);

					fake_marker_names[1] = "normal";
					fake_marker_points[1] = collision.point;
					fake_marker_forwards[1] = collision.plane.n;

					fake_marker_names[2] = "reflected";
					fake_marker_points[2] = collision.point;
					reflect_vector3d(&direction, &collision.plane.n, &fake_marker_forwards[2]);

					fake_marker_names[3] = "midpoint";
					fake_marker_points[3] = midpoint;
					reflect_vector3d(&direction, &collision.plane.n, &fake_marker_forwards[3]);

					effect_new_unattached_from_markers(definition->vehicle.effect.index, NONE, NULL, 4, fake_marker_names, fake_marker_points, fake_marker_forwards,
						scale, scale, NULL, NULL, TRUE);
				}
			}
		}
	}

	return;
}

static void create_crashing_effects(
	long vehicle_index,
	real_vector3d const *old_velocity,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);
	struct game_globals_falling_damage *falling_damage = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->falling_damage, 0, struct game_globals_falling_damage);

	if (falling_damage->vehicle_hit_environment_damage_effect.index != NONE || definition->vehicle.crash_sound.index != NONE)
	{
		real const minimum_acceleration = 0.02f;
		real const maximum_acceleration = 0.042f;
		real_vector3d acceleration;
		real acceleration_magnitude;

		subtract_vectors3d(&vehicle->object.translational_velocity, old_velocity, &acceleration);
		acceleration_magnitude = magnitude3d(&acceleration);
		if (acceleration_magnitude > minimum_acceleration)
		{
			boolean on_ground = FALSE;
			short mass_point_index;

			for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
			{
				struct mass_point_datum *mass_point = &mass_points[mass_point_index];

				TAG_BLOCK_GET_ELEMENT(&physics->mass_points, mass_point_index, struct mass_point_definition);
				if (TEST_FLAG(mass_point->flags, _point_on_ground_bit))
				{
					on_ground = TRUE;
					break;
				}
			}

			if (on_ground)
			{
				real scale = (acceleration_magnitude - minimum_acceleration) / (maximum_acceleration - minimum_acceleration);

				if (falling_damage->vehicle_hit_environment_damage_effect.index != NONE)
				{
					struct damage_data damage_data;

					damage_data_new(&damage_data, falling_damage->vehicle_hit_environment_damage_effect.index);
					damage_data.scale = PIN(scale, 0.f, 1.f);
					damage_data.origin = vehicle->object.bounding_sphere_center;
					damage_data.direction = acceleration;
					object_cause_damage(&damage_data, vehicle_index, NONE, NONE, NONE, NULL);
				}

				if (definition->vehicle.crash_sound.index != NONE)
				{
					object_impulse_sound_new(vehicle_index, definition->vehicle.crash_sound.index, NONE, global_origin3d, global_forward3d, PIN(scale, 0.f, 1.f));
				}
			}
		}
	}

	return;
}

static boolean update_suspension(
	long vehicle_index)
{
	boolean result = FALSE;
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

	if (definition->object.animation_graph.index != NONE)
	{
		struct animation_graph *animation_graph = animation_graph_definition_get(definition->object.animation_graph.index);
		struct animation_graph_vehicle_animations *vehicle_animations = animation_graph->vehicle_animations.count ?
			TAG_BLOCK_GET_ELEMENT(&animation_graph->vehicle_animations, 0, struct animation_graph_vehicle_animations) : NULL;

		if (vehicle_animations)
		{
			struct physics_definition *physics = physics_definition_get(definition->object.physics.index);
			real maximum_compression = 0.f;
			real_matrix4x3 world_matrix;
			short suspension_index;

			matrix4x3_from_point_and_vectors(&world_matrix, &vehicle->object.position, &vehicle->object.forward, &vehicle->object.up);
			for (suspension_index = 0; suspension_index < vehicle_animations->suspension_animations.count; suspension_index++)
			{
				struct animation_graph_vehicle_suspension_animation *suspension = TAG_BLOCK_GET_ELEMENT(&vehicle_animations->suspension_animations,
					suspension_index, struct animation_graph_vehicle_suspension_animation);

				if (suspension->mass_point_index >= 0 && suspension->mass_point_index < physics->mass_points.count && suspension->animation_index != NONE)
				{
					struct mass_point_definition *mass_point;
					struct collision_result collision;
					real_point3d point;
					real_vector3d up;
					real_point3d position;
					real_vector3d vector;
					real old_compression;
					real compression;
					real offset;
					real extension;

					TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, suspension->animation_index, struct animation);
					mass_point = TAG_BLOCK_GET_ELEMENT(&physics->mass_points, suspension->mass_point_index, struct mass_point_definition);
					old_compression = dequantize_byte_to_real(0.f, 1.f, vehicle->vehicle.suspension[suspension_index]);

					matrix4x3_transform_point(&world_matrix, &mass_point->position, &point);
					matrix4x3_transform_normal(&world_matrix, &mass_point->up, &up);
					extension = suspension->full_extension_ground_depth - suspension->full_compression_ground_depth;
					offset = suspension->full_compression_ground_depth - physics->center_of_mass.z;
					vector_from_line3d(&point, &up, offset - extension, &position);
					scale_vector3d(&up, 2.f * extension, &vector);
					collision_test_vector(_collision_test_for_vehicles_flags, &position, &vector, vehicle_index, &collision);

					compression = PIN((1.f - collision.t) * 2.f, 0.f, 1.f);
					maximum_compression = MAX(compression - old_compression, maximum_compression);
					vehicle->vehicle.suspension[suspension_index] = quantize_real_to_byte_lower_bound(0.f, 1.f, (compression + old_compression) * 0.5f);
				}
			}

			{
				real const minimum_sound_compression = 0.3f;
				real const maximum_sound_compression = 0.9f;

				if (definition->vehicle.suspension_sound.index != NONE && maximum_compression > minimum_sound_compression)
				{
					real scale = (maximum_compression - minimum_sound_compression) / (maximum_sound_compression - minimum_sound_compression);

					object_impulse_sound_new(vehicle_index, definition->vehicle.suspension_sound.index, NONE, global_origin3d, global_forward3d, PIN(scale, 0.f, 1.f));
					result = TRUE;
				}
			}
		}
	}

	return result;
}

static void create_slipping_effects(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	real const minimum_slip_velocity = 0.03f;
	real const maximum_slip_velocity = 0.25f;
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);

	if (definition->vehicle.material_effects.index != NONE)
	{
		short mass_point_index;

		for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
		{
			struct mass_point_datum *mass_point = &mass_points[mass_point_index];
			struct mass_point_definition *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&physics->mass_points, mass_point_index, struct mass_point_definition);

			if (TEST_FLAG(mass_point->flags, _point_on_ground_bit))
			{
				real slip_velocity = magnitude3d(&mass_point->velocity_relative_to_ground);

				if (slip_velocity > minimum_slip_velocity)
				{
					real const ground_offset = 0.003f;
					real scale = (slip_velocity - minimum_slip_velocity) / (maximum_slip_velocity - minimum_slip_velocity);
					real_point3d position;
					real_vector3d direction;

					point_from_line3d(&mass_point->position, &mass_point->ground_plane.n, mass_point->ground_depth - mass_point_definition->radius + ground_offset, &position);
					scale_vector3d(&mass_point->velocity_relative_to_ground, 0.8660254f / slip_velocity, &direction);
					point_from_line3d((real_point3d *)&direction, &mass_point->ground_plane.n, 0.5f, (real_point3d *)&direction);

					material_effect_new(definition->vehicle.material_effects.index,
						TEST_FLAG(mass_point_definition->flags, _mass_point_metallic_bit) ? _material_effect_vehicle_slip_metallic : _material_effect_vehicle_slip_nonmetallic,
						mass_point->ground_material_type, &position, &direction, &vehicle->object.location, PIN(scale, 0.f, 1.f));
				}
			}
		}
	}

	return;
}

static void compute_airborne_ticks(
	long vehicle_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	struct physics_definition *physics = physics_definition_get(definition->object.physics.index);
	boolean on_ground = FALSE;
	short mass_point_index;

	if (vehicle->vehicle.airborne_ticks < UNSIGNED_CHAR_MAX)
	{
		vehicle->vehicle.airborne_ticks++;
	}

	for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_datum *mass_point = &mass_points[mass_point_index];

		if (TEST_FLAG(mass_point->flags, _point_on_ground_bit))
		{
			on_ground = TRUE;
			vehicle->vehicle.airborne_ticks = 0;
			break;
		}

		if (TEST_FLAG(mass_point->flags, _point_antigraving_bit))
		{
			vehicle->vehicle.airborne_ticks = 0;
		}
	}

	if (on_ground)
	{
		if (vehicle->vehicle.on_ground_ticks < UNSIGNED_CHAR_MAX)
		{
			vehicle->vehicle.on_ground_ticks++;
		}
	}
	else
	{
		vehicle->vehicle.on_ground_ticks = 0;
	}

	return;
}

void vehicle_export_function_values(
	long vehicle_index)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
	real maximum_forward_speed = fabs(definition->vehicle.speed.maximum_forward_speed);
	real maximum_reverse_speed = fabs(definition->vehicle.speed.maximum_reverse_speed);
	real maximum_speed = MAX(maximum_forward_speed, maximum_reverse_speed);
	real maximum_left_slide = fabs(definition->vehicle.slide.maximum_forward_speed);
	real maximum_right_slide = fabs(definition->vehicle.slide.maximum_reverse_speed);
	real maximum_slide = MAX(maximum_left_slide, maximum_right_slide);
	real maximum_left_turn = fabs(definition->vehicle.turn.maximum_position);
	real maximum_right_turn = fabs(definition->vehicle.turn.minimum_position);
	real maximum_turn = MAX(maximum_left_turn, maximum_right_turn);
	short function_index;

	for (function_index = 0; function_index < NUMBER_OF_INCOMING_OBJECT_FUNCTIONS; function_index++)
	{
		if (definition->vehicle.function_modes[function_index] != _vehicle_function_none)
		{
			real value = 0.f;

			switch (definition->vehicle.function_modes[function_index])
			{
			case _vehicle_function_speed_absolute:
			case _vehicle_function_front_left_tire_velocity:
			case _vehicle_function_front_right_tire_velocity:
			case _vehicle_function_back_left_tire_velocity:
			case _vehicle_function_back_right_tire_velocity:
				value = fabs(vehicle->vehicle.speed) / maximum_speed;
				break;
			case _vehicle_function_speed_forward:
				value = (vehicle->vehicle.speed < 0.f ? 0.f : vehicle->vehicle.speed) / maximum_forward_speed;
				break;
			case _vehicle_function_speed_backward:
				value = fabs(vehicle->vehicle.speed > 0.f ? 0.f : vehicle->vehicle.speed) / maximum_reverse_speed;
				break;
			case _vehicle_function_slide_absolute:
				value = fabs(vehicle->vehicle.slide) / maximum_slide;
				break;
			case _vehicle_function_slide_left:
				value = fabs(vehicle->vehicle.slide) / maximum_left_slide;
				break;
			case _vehicle_function_slide_right:
				value = fabs(vehicle->vehicle.slide) / maximum_right_slide;
				break;
			case _vehicle_function_speed_slide_maximum:
				value = MAX(fabs(vehicle->vehicle.speed) / maximum_speed, fabs(vehicle->vehicle.slide) / maximum_slide);
				break;
			case _vehicle_function_turn_absolute:
				value = fabs(vehicle->vehicle.turn) / maximum_turn;
				break;
			case _vehicle_function_turn_left:
				value = fabs(vehicle->vehicle.turn) / maximum_left_turn;
				break;
			case _vehicle_function_turn_right:
				value = fabs(vehicle->vehicle.turn) / maximum_right_turn;
				break;
			case _vehicle_function_crouch:
				value = TEST_FLAG(vehicle->vehicle.flags, _vehicle_control_crouch_bit) ? 1.f : 0.f;
				break;
			case _vehicle_function_jump:
				value = TEST_FLAG(vehicle->vehicle.flags, _vehicle_control_jump_bit) ? 1.f : 0.f;
				break;
			case _vehicle_function_velocity_air:
				value = magnitude3d(&vehicle->object.translational_velocity) / maximum_speed;
				break;
			case _vehicle_function_velocity_water:
				value = (vehicle->object.flags & (FLAG(_object_on_media_bit) | FLAG(_object_partially_under_media_bit) | FLAG(_object_wholly_under_media_bit))) ?
					magnitude3d(&vehicle->object.translational_velocity) / maximum_speed : 0.f;
				break;
			case _vehicle_function_velocity_ground:
				value = TEST_FLAG(vehicle->object.flags, _object_on_ground_bit) ?
					magnitude3d(&vehicle->object.translational_velocity) / maximum_speed : 0.f;
				break;
			case _vehicle_function_velocity_forward:
				value = fabs(dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.forward)) / maximum_speed;
				break;
			case _vehicle_function_velocity_left:
				value = fabs(triple_product3d(&vehicle->object.up, &vehicle->object.forward, &vehicle->object.translational_velocity)) / maximum_speed;
			case _vehicle_function_velocity_up:
				value = fabs(dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.up)) / maximum_speed;
				break;
			case _vehicle_function_left_tread_position:
				value = vehicle->vehicle.left_tread / definition->vehicle.wheel_circumference;
				break;
			case _vehicle_function_right_tread_position:
				value = vehicle->vehicle.right_tread / definition->vehicle.wheel_circumference;
				break;
			case _vehicle_function_left_tread_velocity:
				value = fabs(vehicle->vehicle.speed - vehicle->vehicle.turn) / maximum_speed;
				break;
			case _vehicle_function_right_tread_velocity:
				value = fabs(vehicle->vehicle.turn + vehicle->vehicle.speed) / maximum_speed;
				break;
			case _vehicle_function_front_left_tire_position:
			case _vehicle_function_front_right_tire_position:
			case _vehicle_function_back_left_tire_position:
			case _vehicle_function_back_right_tire_position:
				value = vehicle->vehicle.wheel / definition->vehicle.wheel_circumference;
				break;
			case _vehicle_function_wingtip_contrail:
				{
					real_vector3d parallel_velocity;
					real_vector3d perpendicular_velocity;

					component_vectors_from_normal3d(&vehicle->object.translational_velocity, &vehicle->object.forward, &parallel_velocity, &perpendicular_velocity);
					value = magnitude3d(&perpendicular_velocity) / 0.3f;
					value = value * value;
				}
				break;
			case _vehicle_function_thrust:
				value = vehicle->vehicle.thrust;
				break;
			case _vehicle_function_hover:
				value = vehicle->vehicle.hover;
				break;
			case _vehicle_function_engine_hack:
				{
					real velocity = fabs(dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.forward)) / maximum_speed;
					real speed = fabs(vehicle->vehicle.speed) / maximum_forward_speed;
					real blend = PIN((vehicle->vehicle.airborne_ticks * 0.2f + 1.f) * 0.5f, 0.f, 1.f);

					value = velocity * (1.f - blend) + blend * speed;
				}
				break;
			case _vehicle_function_wingtip_contrail_new:
				value = magnitude3d(&vehicle->object.translational_velocity) / definition->vehicle.speed.maximum_forward_speed;
				value = (value * vehicle->vehicle.thrust - 0.05f) / (0.9f - 0.05f);
				break;
			}

			vehicle->object.incoming_function_values[function_index] = PIN(value, 0.f, 1.f);
		}
	}

	return;
}

boolean vehicle_moving_near_any_player(
	void)
{
	long unit_indices[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	real_point3d players_centers[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	short unit_count = 0;
	boolean no_vehicles_moving = TRUE;
	short local_player_index;

	for (local_player_index = local_player_get_next(NONE); local_player_index != NONE; local_player_index = local_player_get_next(local_player_index))
	{
		long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE :
			player_get(local_player_get_player_index(local_player_index))->unit_index;

		if (unit_index != NONE && unit_get(unit_index)->object.parent_object_index == NONE)
		{
			real radius;

			unit_indices[unit_count] = unit_index;
			object_get_bounding_sphere(unit_index, &players_centers[unit_count], &radius);
			unit_count++;
		}
	}

	if (unit_count)
	{
		struct object_iterator iterator;
		struct vehicle_datum *vehicle;

		object_iterator_new(&iterator, _object_mask_vehicle, 0);
		while (no_vehicles_moving && (vehicle = object_iterator_next(&iterator)) != NULL)
		{
			short unit_index;

			for (unit_index = 0; unit_index < unit_count; unit_index++)
			{
				if (unit_get(unit_indices[unit_index])->object.parent_object_index != iterator.index &&
					distance_squared3d(&players_centers[unit_index], &vehicle->object.bounding_sphere_center) < 100.f &&
					magnitude_squared3d(&vehicle->object.translational_velocity) >= (1.f / 30.f) * (1.f / 30.f))
				{
					no_vehicles_moving = FALSE;
					break;
				}
			}
		}
	}

	return !no_vehicles_moving;
}

boolean vehicle_stuck(
	long vehicle_index,
	real_vector3d *approximate_direction)
{
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
	struct physics_instance instance;
	boolean stuck = FALSE;

	if (vehicle->vehicle.stuck_mass_point_flags && physics_instance_new(&instance, vehicle_index))
	{
		real_point3d stuck_centroid = *global_origin3d;
		short stuck_count = 0;
		short mass_point_index;

		for (mass_point_index = 0; mass_point_index < instance.physics->mass_points.count; mass_point_index++)
		{
			if (TEST_FLAG(vehicle->vehicle.stuck_mass_point_flags, mass_point_index))
			{
				struct mass_point_definition *mass_point = TAG_BLOCK_GET_ELEMENT(&instance.physics->mass_points, mass_point_index, struct mass_point_definition);

				point_from_line3d(&stuck_centroid, (real_vector3d *)&mass_point->position, 1.f, &stuck_centroid);
				stuck_count++;
			}
		}

		if (stuck_count > 0)
		{
			real_point3d stuck_point;
			real_point3d vehicle_origin;

			scale_vector3d((real_vector3d *)&stuck_centroid, 1.f / stuck_count, (real_vector3d *)&stuck_centroid);
			matrix4x3_transform_point(&instance.world_matrix, &stuck_centroid, &stuck_point);
			object_get_origin(vehicle_index, &vehicle_origin);
			vector_from_points3d(&vehicle_origin, &stuck_point, approximate_direction);
			if (normalize3d(approximate_direction) != 0.f)
			{
				stuck = TRUE;
			}
		}
	}

	return stuck;
}

/* ---------- private code */
