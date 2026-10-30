/*
AIM_ASSIST.C

symbols in this file:
00093C00 0050:
	_compute_attenuation (0000)
00093C50 0030:
	_compute_composite_attenuation (0000)
00093C80 00f0:
	_unit_get_aim_assist_parameters (0000)
00093D70 00a0:
	_compare_targets (0000)
00093E10 0010:
	_reciprocal_square_root (0000)
00093E20 0060:
	_limit3d (0000)
00093E80 0020:
	_set_real_euler_angles2d (0000)
00093EA0 00f0:
	_aim_assist_clear_line_of_sight (0000)
00093F90 01a0:
	_object_compute_autoaim_target (0000)
00094130 01a0:
	_aim_assist_compute_target (0000)
000942D0 0110:
	_autoaim_compute_target (0000)
000943E0 0190:
	_find_aim_assist_targets_recursive (0000)
00094570 0130:
	_find_aim_assist_targets (0000)
000946A0 0100:
	_aim_assist (0000)
000947A0 0340:
	_player_aim_projectile (0000)
00094AE0 0170:
	_local_player_aim_assist (0000)
0025AC3C 0021:
	??_C@_0CB@CDDNCJJI@c?3?2halo?2SOURCE?2game?2aim_assist?4c@ (0000)
0025AC60 0004:
	__real@43000000 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "aim_assist.h"
#include "players.h"
#include "render.h"
#include "network_game_globals.h"
#include "actor_definitions.h"
#include "network_messages.h"
#include "collisions.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "director.h"
#include "weapons.h"
#include "bipeds.h"
#include "structures.h"
#include "observer.h"
#include "physics_constants.h"

/* ---------- constants */

enum
{
	MAXIMUM_AIM_ASSIST_TARGETS = 64 /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean unit_get_aim_assist_parameters(long unit_index, short zoom_level, struct aim_assist_parameters *parameters);
static void object_compute_autoaim_target(long object_index, real_point3d const *position, real_vector3d const *direction, real_point3d *target);
static int __cdecl compare_targets(void const *a, void const *b);
static short find_aim_assist_targets_recursive(struct aim_assist_parameters const *parameters, long object_index, real_point3d const *position, real_vector3d const *direction, real distance, real angle_sine, real angle_cosine, long ignore_object_index, short ignore_team_index, short maximum_target_count, struct aim_assist_target *targets);
static short find_aim_assist_targets(struct aim_assist_parameters const *parameters, short position_cluster_index, real_point3d const *position, real_vector3d const *direction, long ignore_object_index, short ignore_team_index, short maximum_target_count, struct aim_assist_target *targets);

/* ---------- globals */

/* ---------- public code */

__inline real compute_attenuation(
	real variable,
	real maximum)
{
	real minimum = 0.5f * maximum;

	if (variable >= maximum)
	{
		return 0.f;
	}

	if (variable <= minimum)
	{
		return 1.f;
	}

	return (maximum - variable) / (maximum - minimum);
}

__inline real compute_composite_attenuation(
	real angle,
	real maximum_angle,
	real distance,
	real maximum_distance)
{
	return compute_attenuation(angle, maximum_angle) * compute_attenuation(distance, maximum_distance);
}

long player_aim_projectile(
	long player_index,
	real_point3d const *position,
	real_vector3d *direction)
{
	long target_object_index = NONE;
	struct player_datum *player = player_get(player_index);
	long unit_index = unit_get_aiming_unit_index(player->unit_index);
	struct aim_assist_parameters parameters;

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\game\\aim_assist.c", 77, direction);
	match_collision_log_begin_user("c:\\halo\\SOURCE\\game\\aim_assist.c", 79, _collision_user_aim_assist);

	if (unit_get_aim_assist_parameters(unit_index, unit_get_zoom_level(unit_index), &parameters))
	{
		real_vector3d target_direction;
		real_vector3d collision_direction;
		real_point3d camera_position;
		real_vector3d camera_direction;
		real_vector3d camera_displacement;
		real_vector3d camera_vector;
		real_vector3d desired_direction;
		real autoaim_level;

		director_camera_deterministic(player->unit_index, &camera_position, &camera_direction);
		target_direction = *direction;
		autoaim_level = 0.f;

		{
			struct aim_assist_target target;

			if (aim_assist(&parameters, &camera_position, &camera_direction, player->unit_index, (short)player->team_index, &target))
			{
				vector_from_points3d(position, &target.position, &target_direction);
				if (normalize3d(&target_direction) == 0.f)
				{
					target_direction = *direction;
				}

				autoaim_level = target.autoaim_level;
				target_object_index = target.object_index;
			}
		}

		{
			struct unit_datum *unit = unit_get(unit_index);
			real camera_distance = magnitude3d(vector_from_points3d(&unit->object.position, &camera_position, &camera_displacement));
			struct collision_result collision;

			camera_vector = camera_direction;
			normalize3d(&camera_vector);
			scale_vector3d(&camera_vector, camera_distance, &camera_vector);
			set_real_point3d(&camera_position, camera_position.x + camera_vector.i, camera_position.y + camera_vector.j, camera_position.z + camera_vector.k);
			scale_vector3d(&camera_direction, 128.f, &desired_direction);
			collision_test_vector(_collision_test_for_projectiles_flags, &camera_position, &desired_direction, player->unit_index, &collision);

			vector_from_points3d(position, &collision.point, &collision_direction);
			if (normalize3d(&collision_direction) == 0.f)
			{
				collision_direction = *direction;
			}
		}

		fast_normals_interpolate(&collision_direction, &target_direction, autoaim_level, &desired_direction);
		pin_normal_to_cone3d(&desired_direction, direction, sine(parameters.deviation_angle), cosine(parameters.deviation_angle), direction);
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\game\\aim_assist.c", 140);

	player->aim_assist_unit_index = target_object_index;
	player->aim_assist_timestamp = game_time_get();

	return target_object_index;
}

long local_player_aim_assist(
	short local_player_index,
	real *autoaim_level,
	real *magnetism_level,
	real_euler_angles2d *target_angular_position,
	real_euler_angles2d *target_angular_velocity)
{
	long target_object_index = NONE;
	short perspective = director_get_perspective(local_player_index);

	*autoaim_level = 0.f;
	*magnetism_level = 0.f;
	set_real_euler_angles2d(target_angular_position, 0.f, 0.f);
	set_real_euler_angles2d(target_angular_velocity, 0.f, 0.f);

	if (perspective == _director_perspective_first_person || perspective == _director_perspective_third_person)
	{
		struct player_datum *player = player_get(local_player_get_player_index(local_player_index));
		long unit_index = unit_get_aiming_unit_index(player->unit_index);
		struct aim_assist_parameters parameters;

		if (unit_get_aim_assist_parameters(unit_index, player_control_get_zoom_level(local_player_index), &parameters))
		{
			struct observer_result const *camera = observer_get_camera(local_player_index);
			struct aim_assist_target target;

			if (aim_assist(&parameters, &camera->position, &camera->forward, unit_index, (short)player->team_index, &target))
			{
				real_vector3d velocity;
				real_vector3d target_velocity;
				real_vector3d relative_velocity;
				real_vector3d const *vector = &target.vector;
				real_vector3d const *velocity_delta = &relative_velocity;
				real xy_magnitude_squared;
				real xy_magnitude;

				*autoaim_level = target.autoaim_level;
				*magnetism_level = target.magnetism_level;
				euler_angles2d_from_vector3d(target_angular_position, &target.vector);
				object_get_velocities(player->unit_index, &velocity, NULL);
				object_get_velocities(target.object_index, &target_velocity, NULL);
				subtract_vectors3d(&target_velocity, &velocity, &relative_velocity);

				xy_magnitude_squared = vector->i * vector->i + vector->j * vector->j;
				xy_magnitude = square_root(xy_magnitude_squared);
				target_angular_velocity->yaw = (vector->i * velocity_delta->j - vector->j * velocity_delta->i) / xy_magnitude_squared;
				target_angular_velocity->pitch = (xy_magnitude * velocity_delta->k - vector->k / xy_magnitude * (vector->i * velocity_delta->i + vector->j * velocity_delta->j)) / (vector->k * vector->k + xy_magnitude_squared);
				target_object_index = target.object_index;
			}
		}
	}

	return target_object_index;
}

boolean aim_assist(
	struct aim_assist_parameters const *parameters,
	real_point3d const *position,
	real_vector3d const *direction,
	long ignore_object_index,
	short ignore_team_index,
	struct aim_assist_target *target)
{
	boolean result = FALSE;

	if (scenario_leaf_index_from_point(position) != NONE)
	{
		short cluster_index = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, scenario_leaf_index_from_point(position) & LONG_MAX, struct structure_leaf)->cluster_index;

		if (cluster_index != NONE)
		{
			struct aim_assist_target targets[MAXIMUM_AIM_ASSIST_TARGETS];
			short target_count = find_aim_assist_targets(parameters, cluster_index, position, direction, ignore_object_index, ignore_team_index, NUMBEROF(targets), targets);

			if (target_count > 0)
			{
				short target_index;

				qsort(targets, target_count, sizeof(struct aim_assist_target), compare_targets);
				for (target_index = 0; target_index < target_count; target_index++)
				{
					if (aim_assist_clear_line_of_sight(position, &targets[target_index].position, ignore_object_index, targets[target_index].object_index))
					{
						*target = targets[target_index];
						result = TRUE;
						break;
					}
				}
			}
		}
	}

	return result;
}

boolean aim_assist_compute_target(
	struct aim_assist_parameters const *parameters,
	long object_index,
	real_point3d const *position,
	real_vector3d const *direction,
	struct aim_assist_target *target)
{
	real cosine;

	target->object_index = object_index;
	object_compute_autoaim_target(object_index, position, direction, &target->position);
	vector_from_points3d(position, &target->position, &target->vector);
	target->direction = target->vector;
	target->distance = normalize3d(&target->direction);
	cosine = dot_product3d(&target->direction, direction);
	target->angle = arccosine(PIN(cosine, -1.f, 1.f));

	if (parameters)
	{
		target->autoaim_level = compute_composite_attenuation(target->angle, parameters->autoaim_angle, target->distance, parameters->autoaim_distance);
		target->magnetism_level = compute_composite_attenuation(target->angle, parameters->magnetism_angle, target->distance, parameters->magnetism_distance);

		if (target->magnetism_level > 0.f)
		{
			struct unit_datum *unit = unit_get(target->object_index);
			struct unit_definition *definition = unit_definition_get(unit->definition_index);

			if (TEST_FLAG(definition->unit.flags, _unit_is_inconsequential_bit))
			{
				target->magnetism_level *= TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_control, 0, struct game_globals_player_control)->magnetism_inconsequential_target_scale;
			}
		}
	}
	else
	{
		target->autoaim_level = 0.f;
		target->magnetism_level = 0.f;
	}

	return target->autoaim_level > 0.f || target->magnetism_level > 0.f;
}

boolean autoaim_compute_target(
	long object_index,
	real_point3d const *position,
	real_vector3d const *direction,
	long ignore_object_index,
	real_point3d *target_position,
	real_vector3d *target_direction,
	real *target_distance,
	real *target_angle)
{
	boolean result = FALSE;

	object_compute_autoaim_target(object_index, position, direction, target_position);
	if (aim_assist_clear_line_of_sight(position, target_position, ignore_object_index, object_index))
	{
		vector_from_points3d(position, target_position, target_direction);
		*target_distance = normalize3d(target_direction);
		if (*target_distance != 0.f)
		{
			*target_angle = arccosine(PIN(dot_product3d(direction, target_direction), -1.f, 1.f));
			result = TRUE;
		}
	}

	return result;
}

boolean aim_assist_clear_line_of_sight(
	real_point3d const *p0,
	real_point3d const *p1,
	long ignore_object_index,
	long target_object_index)
{
	boolean clear = FALSE;
	struct collision_result collision;

	match_collision_log_begin_user("c:\\halo\\SOURCE\\game\\aim_assist.c", 350, _collision_user_aim_assist);

	if (!collision_test_line(_collision_test_for_line_of_sight_flags, p0, p1, object_get_ultimate_parent(ignore_object_index), &collision))
	{
		clear = TRUE;
	}
	else if (collision.type == _collision_result_object && object_get_ultimate_parent(collision.object_index) == object_get_ultimate_parent(target_object_index))
	{
		clear = TRUE;
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\game\\aim_assist.c", 367);

	return clear;
}

/* ---------- private code */

static boolean unit_get_aim_assist_parameters(
	long unit_index,
	short zoom_level,
	struct aim_assist_parameters *parameters)
{
	if (unit_index != NONE)
	{
		long weapon_index = unit_inventory_get_weapon(unit_index, unit_get(unit_index)->unit.current_weapon_index);

		if (weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(weapon_index);
			struct weapon_definition *definition = weapon_definition_get(weapon->definition_index);

			if (zoom_level != NONE || !TEST_FLAG(definition->weapon.flags, _weapon_aim_assists_only_when_zoomed_bit))
			{
				real magnification = weapon_get_zoom_magnification(weapon_index, zoom_level);
				real angle_scale = 1.f / magnification;

				parameters->autoaim_angle = angle_scale * definition->weapon.aim_assist_parameters.autoaim_angle;
				parameters->autoaim_distance = magnification * definition->weapon.aim_assist_parameters.autoaim_distance;
				parameters->magnetism_angle = angle_scale * definition->weapon.aim_assist_parameters.magnetism_angle;
				parameters->magnetism_distance = magnification * definition->weapon.aim_assist_parameters.magnetism_distance;
				parameters->deviation_angle = angle_scale * MAX(definition->weapon.aim_assist_parameters.deviation_angle, definition->weapon.aim_assist_parameters.autoaim_angle);

				return TRUE;
			}
		}
	}

	return FALSE;
}

static short find_aim_assist_targets(
	struct aim_assist_parameters const *parameters,
	short position_cluster_index,
	real_point3d const *position,
	real_vector3d const *direction,
	long ignore_object_index,
	short ignore_team_index,
	short maximum_target_count,
	struct aim_assist_target *targets)
{
	short target_count = 0;
	real distance = MAX(parameters->autoaim_distance, parameters->magnetism_distance);
	real angle = MAX(parameters->autoaim_angle, parameters->magnetism_angle);

	if (distance > 0.f && angle > 0.f)
	{
		real angle_sine = sine(angle);
		real angle_cosine = cosine(angle);
		short cluster_indices[MAXIMUM_CLUSTERS_PER_STRUCTURE];
		short cluster_count = structure_clusters_in_cone(position_cluster_index, position, direction, distance, angle_sine, angle_cosine, NUMBEROF(cluster_indices), cluster_indices);
		long object_indices[MAXIMUM_OBJECTS_PER_MAP];
		short object_count = objects_in_clusters_by_indices(FLAG(_object_class_collideable), cluster_count, cluster_indices, NUMBEROF(object_indices), object_indices);
		short index;

		for (index = 0; index < object_count; index++)
		{
			long object_index = object_indices[index];

			target_count += find_aim_assist_targets_recursive(parameters, object_index, position, direction, distance, angle_sine, angle_cosine, ignore_object_index, ignore_team_index, maximum_target_count - target_count, &targets[target_count]);
			if (target_count >= maximum_target_count)
			{
				break;
			}
		}
	}

	return target_count;
}

static short find_aim_assist_targets_recursive(
	struct aim_assist_parameters const *parameters,
	long object_index,
	real_point3d const *position,
	real_vector3d const *direction,
	real distance,
	real angle_sine,
	real angle_cosine,
	long ignore_object_index,
	short ignore_team_index,
	short maximum_target_count,
	struct aim_assist_target *targets)
{
	short target_count = 0;

	do
	{
		struct object_datum *object = object_get(object_index);

		if (TEST_FLAG(_object_mask_unit, object->object.type) &&
			!TEST_FLAG(object->object.flags, _object_invisible_bit) &&
			unit_get(object_index)->unit.active_camouflage < 1.f &&
			sphere_intersects_cone3d(&object->object.bounding_sphere_center, object->object.bounding_sphere_radius, position, direction, distance, angle_sine, angle_cosine))
		{
			if (TEST_FLAG(_object_mask_biped, object->object.type) &&
				!TEST_FLAG(object->object.damage_flags, _object_dead_bit) &&
				object_index != ignore_object_index &&
				game_team_is_enemy(ignore_team_index, object->object.owner_team_index) &&
				!TEST_FLAG(biped_definition_get(object->definition_index)->unit.flags, _unit_ignored_by_autoaiming_bit))
			{
				struct aim_assist_target target;

				if (aim_assist_compute_target(parameters, object_index, position, direction, &target) && target_count < maximum_target_count)
				{
					targets[target_count++] = target;
				}
			}

			if (object->object.first_child_object_index != NONE && target_count < maximum_target_count)
			{
				target_count += find_aim_assist_targets_recursive(parameters, object->object.first_child_object_index, position, direction, distance, angle_sine, angle_cosine, ignore_object_index, ignore_team_index, maximum_target_count - target_count, &targets[target_count]);
			}
		}

		object_index = object->object.next_object_index;
	}
	while (object_index != NONE && target_count < maximum_target_count);

	return target_count;
}

static void object_compute_autoaim_target(
	long object_index,
	real_point3d const *position,
	real_vector3d const *direction,
	real_point3d *target)
{
	real_point3d base;
	real_vector3d height;
	real width;
	real_vector3d normal;
	real denominator;
	real_vector3d displacement;
	real_vector3d perpendicular;

	biped_get_autoaim_pill(object_index, &base, &height, &width);
	cross_product3d(&height, direction, &normal);
	denominator = magnitude_squared3d(&normal);

	if (denominator > 0.f)
	{
		real_vector3d base_to_position;
		real_vector3d cross;
		real t;

		vector_from_points3d(&base, position, &base_to_position);
		cross_product3d(&base_to_position, direction, &cross);
		t = dot_product3d(&cross, &normal) / denominator;
		point_from_line3d(&base, &height, PIN(t, 0.f, 1.f), target);
	}
	else
	{
		*target = base;
	}

	vector_from_points3d(position, target, &displacement);
	point_from_line3d((real_point3d *)&displacement, direction, -dot_product3d(&displacement, direction), (real_point3d *)&perpendicular);
	limit3d(&perpendicular, width);
	point_from_line3d(target, &perpendicular, -1.f, target);

	return;
}

static int __cdecl compare_targets(
	void const *a,
	void const *b)
{
	struct aim_assist_target const *target0 = (struct aim_assist_target const *)a;
	struct aim_assist_target const *target1 = (struct aim_assist_target const *)b;

	if (target0->autoaim_level > target1->autoaim_level)
	{
		return -1;
	}

	if (target0->autoaim_level < target1->autoaim_level)
	{
		return 1;
	}

	if (target0->magnetism_level > target1->magnetism_level)
	{
		return -1;
	}

	if (target0->magnetism_level < target1->magnetism_level)
	{
		return 1;
	}

	if (target0->distance < target1->distance)
	{
		return -1;
	}

	if (target0->distance > target1->distance)
	{
		return 1;
	}

	if (target0->angle < target1->angle)
	{
		return -1;
	}

	if (target0->angle > target1->angle)
	{
		return 1;
	}

	return DATUM_INDEX_TO_ABSOLUTE_INDEX(target0->object_index) - DATUM_INDEX_TO_ABSOLUTE_INDEX(target1->object_index);
}

