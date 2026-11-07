/*
PHYSICS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "physics.h"
#include "collisions.h"
#include "physics_constants.h"
#include "collision_models.h"
#include "terrain_definitions.h"
#include "render.h"
#include "vehicles.h"
#include "render_debug.h"
#include "bipeds.h"
#include "structures.h"

/* ---------- constants */

/* ---------- macros */

#define SECONDS_PER_TICK (1.f/TICKS_PER_SECOND) /* fake name */

/* ---------- structures */

/* ---------- prototypes */

static short get_material_type(long object_index, short material_index);
static void compute_ground_plane(long object_index, struct mass_point_datum *mass_point, struct mass_point_definition const *mass_point_definition);
static void friction_evaluate(short type, real parallel_scale, real perpendicular_scale, struct friction_datum *friction, real_vector3d *forward, real_vector3d *up);
static void rotate_vectors3d_by_angular_velocity(real_vector3d const *forward, real_vector3d const *up, real_vector3d const *angular_velocity, real_vector3d *rotated_forward, real_vector3d *rotated_up);
static boolean physics_compute_vehicle_collision(struct physics_instance const *instance0, struct physics_instance const *instance1);
static boolean physics_compute_biped_collision(struct collision_model_instance *instance, long biped_index);
static void physics_compute_unit_collisions(long vehicle_index);
static void physics_update_old(long object_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points, real_vector3d const *magic_force, real_vector3d const *magic_torque);
/* ---------- globals */

real global_gravity = /*0.0035651792f*/(GRAVITY/METERS_PER_UNIT)*(SECONDS_PER_TICK*SECONDS_PER_TICK);
real global_water_density = 1.f;
real global_air_density = 0.0011f;
real global_physics_collision_depth = 0.2f;
real_plane3d depths_of_hell = { { 0.f, 0.f, 1.f }, -256.f };
boolean debug_physics_disable_penetration_freeze = FALSE;

/* ---------- public code */

__inline real pin_fraction(
	real value,
	real value0,
	real value1)
{
	real fraction;

	if (value0 < value1)
	{
		if (value <= value0) fraction = 0.f;
		else if (value >= value1) fraction = 1.f;
		else fraction = (value-value0)/(value1-value0);
	}
	else
	{
		if (value <= value1) fraction = 1.f;
		else if (value >= value0) fraction = 0.f;
		else fraction = (value0-value)/(value0-value1);
	}

	return fraction;
}

boolean physics_instance_new(
	struct physics_instance *instance,
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (object_definition->object.physics.index != NONE)
	{
		real_point3d negative_center_of_mass;

		instance->object_index = object_index;
		instance->physics = physics_definition_get(object_definition->object.physics.index);
		instance->world_matrix.scale = 1.f;
		object_get_origin(object_index, &instance->world_matrix.position);
		object_get_orientation(object_index, &instance->world_matrix.forward, &instance->world_matrix.up);
		cross_product3d(&instance->world_matrix.up, &instance->world_matrix.forward, &instance->world_matrix.left);
		set_real_point3d(&negative_center_of_mass, -instance->physics->center_of_mass.x, -instance->physics->center_of_mass.y, -instance->physics->center_of_mass.z);
		matrix4x3_transform_point(&instance->world_matrix, &negative_center_of_mass, &negative_center_of_mass);
		instance->world_matrix.position = negative_center_of_mass;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

boolean physics_test_point(
	struct physics_instance const *instance,
	real_point3d const *point)
{
	real_point3d transformed_point;
	short mass_point_index;

	matrix4x3_inverse_transform_point(&instance->world_matrix, point, &transformed_point);
	for (mass_point_index = 0; mass_point_index < instance->physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&instance->physics->mass_points, mass_point_index, struct mass_point_definition);

		if (point_in_sphere(&transformed_point, &mass_point_definition->position, mass_point_definition->radius))
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean physics_test_vector(
	struct physics_instance const *instance,
	real_point3d const *point,
	real_vector3d const *vector,
	struct physics_test_vector_result *result)
{
	boolean hit = FALSE;
	real_point3d transformed_point;
	real_vector3d transformed_vector;
	short mass_point_index;

	result->t = FLT_MAX;

	matrix4x3_inverse_transform_point(&instance->world_matrix, point, &transformed_point);
	matrix4x3_inverse_transform_vector(&instance->world_matrix, vector, &transformed_vector);
	for (mass_point_index = 0; mass_point_index < instance->physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&instance->physics->mass_points, mass_point_index, struct mass_point_definition);
		real t;
		real_vector3d normal;

		if (sphere_test_vector3d(&mass_point_definition->position, mass_point_definition->radius, &transformed_point, &transformed_vector, &t, &normal) && result->t > t)
		{
			real_point3d intersection;

			point_from_line3d(&transformed_point, &transformed_vector, t, &intersection);
			result->t = t;
			plane3d_from_point_and_normal(&result->plane, &intersection, &normal);
			hit = TRUE;
		}
	}

	if (hit)
	{
		matrix4x3_transform_plane(&instance->world_matrix, &result->plane, &result->plane);
	}

	return hit;
}

boolean physics_get_features_in_sphere(
	struct physics_instance const *instance,
	real_point3d const *center,
	real radius,
	real height,
	real width,
	struct collision_feature_list *features)
{
	short mass_point_index;

	for (mass_point_index = 0; mass_point_index < instance->physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&instance->physics->mass_points, mass_point_index, struct mass_point_definition);
		real_point3d point;
		real point_radius;

		matrix4x3_transform_point(&instance->world_matrix, &mass_point_definition->position, &point);
		point_radius = mass_point_definition->radius*instance->world_matrix.scale;
		collision_features_from_point(&point, height, point_radius+width, instance->object_index, NONE, 0, NONE, NONE, features);
	}

	return features->count[_collision_feature_sphere] || features->count[_collision_feature_cylinder] || features->count[_collision_feature_prism];
}

void physics_update(
	long object_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points,
	real_vector3d const *magic_force,
	real_vector3d const *magic_torque)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	struct physics_definition *physics = physics_definition_get(object_definition->object.physics.index);

	if (physics->radius > 0.f)
	{
		physics_update_old(object_index, powered_mass_points, mass_points, magic_force, magic_torque);
	}
	else
	{
		struct physics_instance instance;
		real_vector3d total_force, total_torque;
		struct vehicle_datum *vehicle;

		physics_instance_new(&instance, object_index);
		if (powered_mass_points)
		{
			short powered_mass_point_index;

			for (powered_mass_point_index = 0; powered_mass_point_index < physics->powered_mass_points.count; powered_mass_point_index++)
			{
				struct powered_mass_point_datum *powered_mass_point = &powered_mass_points[powered_mass_point_index];

				matrix4x3_rotation_from_quaternion(&powered_mass_point->rotation_matrix, &powered_mass_point->rotation);
				matrix4x3_transpose(&powered_mass_point->rotation_matrix);
			}
		}
		physics_compute_new(&instance, powered_mass_points, mass_points, &total_force, &total_torque);
		vehicle = vehicle_get(object_index);
		add_vectors3d(&total_force, &vehicle->vehicle.collision_force, &total_force);
		add_vectors3d(&total_torque, &vehicle->vehicle.collision_torque, &total_torque);
		set_real_vector3d(&vehicle->vehicle.collision_force, 0.f, 0.f, 0.f);
		set_real_vector3d(&vehicle->vehicle.collision_torque, 0.f, 0.f, 0.f);
		if (magic_force)
		{
			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 270, magic_force);
			add_vectors3d(&total_force, magic_force, &total_force);
		}
		if (magic_torque)
		{
			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 276, magic_torque);
			add_vectors3d(&total_torque, magic_torque, &total_torque);
		}
		physics_update_new(&instance, powered_mass_points, mass_points, &total_force, &total_torque);
		physics_compute_unit_collisions(object_index);
	}

	return;
}

/* ---------- private code */

static short get_material_type(
	long object_index,
	short material_index)
{
	short material_type;

	if (material_index != NONE)
	{
		if (object_index != NONE)
		{
			struct object_datum *object = object_get(object_index);
			struct object_definition *definition = object_definition_get(object->definition_index);
			struct collision_model *model = collision_model_definition_get(definition->object.collision_model.index);
			struct damage_material *material = TAG_BLOCK_GET_ELEMENT(&model->resistance.materials, material_index, struct damage_material);

			material_type = material->type;
		}
		else
		{
			struct structure_collision_material *material = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->collision_materials, material_index, struct structure_collision_material);

			material_type = material->runtime_physics_material_type;
		}
	}
	else
	{
		return NONE;
	}

	return material_type;
}

static void compute_ground_plane(
	long object_index,
	struct mass_point_datum *mass_point,
	struct mass_point_definition const *mass_point_definition)
{
	struct collision_feature_list features;
	struct collision_plane collision;

	mass_point->ground_plane = depths_of_hell;
	mass_point->ground_material_type = NONE;
	mass_point->ground_depth = mass_point_definition->radius-plane3d_distance_to_point(&mass_point->ground_plane, &mass_point->position);

	if (collision_get_features_in_sphere(_collision_test_for_vehicles_flags, &mass_point->position, mass_point_definition->radius, 0.f, mass_point_definition->radius, object_index, &features) &&
		collision_features_test_point(&features, &mass_point->position, &collision))
	{
		mass_point->ground_plane = collision.plane;
		mass_point->ground_depth = collision.t;
		mass_point->ground_material_type = get_material_type(collision.object_index, collision.material_index);
		SET_FLAG(mass_point->flags, _point_on_volatile_surface_bit, TEST_FLAG(collision.flags, _collision_surface_breakable_bit) ||
			(collision.object_index != NONE && !TEST_FLAG(_object_mask_scenery, object_get_type(collision.object_index))));

		if (collision.object_index != NONE)
		{
			object_deplete_shield(collision.object_index);
		}
	}

	match_assert("c:\\halo\\SOURCE\\physics\\physics.c", 338, mass_point->ground_material_type==NONE || (mass_point->ground_material_type>=0 && mass_point->ground_material_type<NUMBER_OF_MATERIAL_TYPES));
	return;
}

static void friction_evaluate(
	short type,
	real parallel_scale,
	real perpendicular_scale,
	struct friction_datum *friction,
	real_vector3d *forward,
	real_vector3d *up)
{
	if (type==_friction_type_point)
	{
		friction->parallel = friction->friction;
		set_real_vector3d(&friction->perpendicular, 0.f, 0.f, 0.f);
	}
	else
	{
		switch (type)
		{
		case _friction_type_forward:
			component_vectors_from_normal3d(&friction->friction, forward, &friction->parallel, &friction->perpendicular);
			break;
		case _friction_type_left:
			{
				real_vector3d left;

				cross_product3d(up, forward, &left);
				component_vectors_from_normal3d(&friction->friction, &left, &friction->parallel, &friction->perpendicular);
			}
			break;
		case _friction_type_up:
			component_vectors_from_normal3d(&friction->friction, up, &friction->parallel, &friction->perpendicular);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\physics\\physics.c", 383, FALSE, NULL);
			break;
		}

		scale_vector3d(&friction->parallel, parallel_scale, &friction->parallel);
		scale_vector3d(&friction->perpendicular, perpendicular_scale, &friction->perpendicular);
		add_vectors3d(&friction->parallel, &friction->perpendicular, &friction->friction);
	}

	return;
}

void physics_compute_new(
	struct physics_instance const *instance,
	struct powered_mass_point_datum const *powered_mass_points,
	struct mass_point_datum *mass_points,
	real_vector3d *total_force,
	real_vector3d *total_torque)
{
	struct object_datum *object = object_get(instance->object_index);
	struct physics_definition const *physics = instance->physics;
	real gravity = global_gravity*physics->gravity_scale;
	short mass_point_index;

	set_real_vector3d(total_force, 0.f, 0.f, -gravity*physics->mass);
	set_real_vector3d(total_torque, 0.f, 0.f, 0.f);
	csmemset(mass_points, 0, sizeof(struct mass_point_datum)*physics->mass_points.count);

	for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&physics->mass_points, mass_point_index, struct mass_point_definition);
		struct mass_point_datum *mass_point = &mass_points[mass_point_index];
		struct powered_mass_point_definition const *powered_mass_point_definition = mass_point_definition->powered_mass_point_index != NONE && powered_mass_points ? TAG_BLOCK_GET_ELEMENT(&physics->powered_mass_points, mass_point_definition->powered_mass_point_index, struct powered_mass_point_definition) : NULL;
		struct powered_mass_point_datum const *powered_mass_point = powered_mass_point_definition ? &powered_mass_points[mass_point_definition->powered_mass_point_index] : NULL;
		real weight = mass_point_definition->mass*gravity; // unused, but sapien computes it and it affects codegen

		mass_point->flags = 0;
		matrix4x3_transform_point(&instance->world_matrix, &mass_point_definition->position, &mass_point->position);
		if (powered_mass_point)
		{
			real_matrix4x3 powered_world_matrix;

			matrix4x3_multiply(&instance->world_matrix, &powered_mass_point->rotation_matrix, &powered_world_matrix);
			matrix4x3_transform_normal(&powered_world_matrix, &mass_point_definition->forward, &mass_point->forward);
			matrix4x3_transform_normal(&powered_world_matrix, &mass_point_definition->up, &mass_point->up);
		}
		else
		{
			matrix4x3_transform_normal(&instance->world_matrix, &mass_point_definition->forward, &mass_point->forward);
			matrix4x3_transform_normal(&instance->world_matrix, &mass_point_definition->up, &mass_point->up);
		}
		scenario_location_from_point(&mass_point->location, &mass_point->position);
		vector_from_points3d(&object->object.position, &mass_point->position, &mass_point->radius);
		cross_product3d(&object->object.angular_velocity, &mass_point->radius, &mass_point->velocity);
		add_vectors3d(&mass_point->velocity, &object->object.translational_velocity, &mass_point->velocity);
		compute_ground_plane(instance->object_index, mass_point, mass_point_definition);
		mass_point->water_depth = scenario_location_water_depth(&mass_point->location, &mass_point->position);

		if (mass_point->ground_depth > 0.f)
		{
			struct material_definition *material = scenario_material_definition_get(mass_point->ground_material_type);
			real ground_friction = material->physics_ground_friction_scale > 0.f && physics->mass <= 7500.f ? physics->ground_friction*material->physics_ground_friction_scale : physics->ground_friction;
			real ground_normal_k1 = material->physics_ground_friction_normal_k1_scale > 0.f ? physics->ground_normal_k1*material->physics_ground_friction_normal_k1_scale : physics->ground_normal_k1;
			real ground_normal_k0 = material->physics_ground_friction_normal_k0_scale > 0.f ? physics->ground_normal_k0*material->physics_ground_friction_normal_k0_scale : physics->ground_normal_k0;
			real ground_depth = material->physics_ground_depth_scale > 0.f ? physics->ground_depth*material->physics_ground_depth_scale : physics->ground_depth;
			real ground_damp_fraction = material->physics_ground_damp_fraction_scale > 0.f ? physics->ground_damp_fraction*material->physics_ground_damp_fraction_scale : physics->ground_damp_fraction;
			real normal_velocity = dot_product3d(&mass_point->velocity, &mass_point->ground_plane.n);
			real friction_scale;

			mass_point->normal_force_magnitude = (mass_point->ground_depth/ground_depth*global_gravity-normal_velocity*ground_damp_fraction)*physics->mass;
			scale_vector3d(&mass_point->ground_plane.n, mass_point->normal_force_magnitude, &mass_point->normal_force);
			friction_scale = -ground_friction*mass_point_definition->mass;
			point_from_line3d((real_point3d *)&mass_point->velocity, &mass_point->ground_plane.n, -normal_velocity, (real_point3d *)&mass_point->velocity_relative_to_ground);
			scale_vector3d(&mass_point->velocity_relative_to_ground, friction_scale, &mass_point->ground_friction.friction);

			if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_ground_friction_bit) && powered_mass_point->ground_friction_velocity != 0.f)
			{
				real ground_fraction = pin_fraction(mass_point->ground_plane.n.k, ground_normal_k0, ground_normal_k1);
				real up_fraction = dot_product3d(&mass_point->up, &mass_point->ground_plane.n);
				real up_fraction_clamped = PIN(up_fraction, 0.f, 1.f);
				real powered_friction_scale = ground_fraction*ground_fraction*up_fraction_clamped*up_fraction_clamped*friction_scale;
				real_vector3d powered_velocity, tangent_velocity;
				real powered_normal_velocity;

				scale_vector3d(&mass_point->forward, -powered_mass_point->ground_friction_velocity, &powered_velocity);
				powered_normal_velocity = dot_product3d(&powered_velocity, &mass_point->ground_plane.n);
				point_from_line3d((real_point3d *)&powered_velocity, &mass_point->ground_plane.n, -powered_normal_velocity, (real_point3d *)&tangent_velocity);
				add_vectors3d(&mass_point->velocity_relative_to_ground, &tangent_velocity, &mass_point->velocity_relative_to_ground);
				point_from_line3d((real_point3d *)&mass_point->ground_friction.friction, &tangent_velocity, powered_friction_scale, (real_point3d *)&mass_point->ground_friction.friction);
			}
			friction_evaluate(mass_point_definition->friction_type, mass_point_definition->friction_parallel_scale, mass_point_definition->friction_perpendicular_scale, &mass_point->ground_friction, &mass_point->forward, &mass_point->up);
		}

		if (mass_point->water_depth > 0.f)
		{
			real water_fraction = mass_point->water_depth < physics->water_depth ? mass_point->water_depth/physics->water_depth : 1.f;

			if (mass_point_definition->density > 0.f && physics->water_depth > 0.f)
			{
				mass_point->water_pressure_magnitude = physics->water_density/mass_point_definition->density*mass_point_definition->mass*water_fraction*gravity;
				set_real_vector3d(&mass_point->water_pressure, 0.f, 0.f, mass_point->water_pressure_magnitude);
			}
			if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_water_friction_bit) && powered_mass_point->water_friction_velocity != 0.f)
			{
				real_vector3d velocity;

				point_from_line3d((real_point3d *)&mass_point->velocity, &mass_point->forward, -powered_mass_point->water_friction_velocity, (real_point3d *)&velocity);
				scale_vector3d(&velocity, -physics->water_friction*mass_point_definition->mass, &mass_point->water_friction.friction);
			}
			else
			{
				scale_vector3d(&mass_point->velocity, -physics->water_friction*mass_point_definition->mass, &mass_point->water_friction.friction);
			}
			friction_evaluate(mass_point_definition->friction_type, mass_point_definition->friction_parallel_scale, mass_point_definition->friction_perpendicular_scale, &mass_point->water_friction, &mass_point->forward, &mass_point->up);

			if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_water_lift_bit) && powered_mass_point->water_lift_ratio != 0.f)
			{
				real speed = fabs(dot_product3d(&mass_point->velocity, &mass_point->forward));
				real lift = ((speed*physics->mass)*water_fraction)*powered_mass_point->water_lift_ratio;

				point_from_line3d((real_point3d *)&mass_point->powered_force, &mass_point->up, lift, (real_point3d *)&mass_point->powered_force);
			}
		}

		if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_air_friction_bit) && powered_mass_point->air_friction_velocity != 0.f)
		{
			real_vector3d velocity;

			point_from_line3d((real_point3d *)&mass_point->velocity, &mass_point->forward, -powered_mass_point->air_friction_velocity, (real_point3d *)&velocity);
			scale_vector3d(&velocity, -physics->air_friction*mass_point_definition->mass, &mass_point->air_friction.friction);
		}
		else
		{
			scale_vector3d(&mass_point->velocity, -physics->air_friction*mass_point_definition->mass, &mass_point->air_friction.friction);
		}
		friction_evaluate(mass_point_definition->friction_type, mass_point_definition->friction_parallel_scale, mass_point_definition->friction_perpendicular_scale, &mass_point->air_friction, &mass_point->forward, &mass_point->up);
		if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_air_lift_bit) && powered_mass_point->air_lift_ratio != 0.f)
		{
			real speed = fabs(dot_product3d(&mass_point->velocity, &mass_point->forward));
			real lift = ((speed*powered_mass_point->air_lift_ratio)*physics->mass);

			point_from_line3d((real_point3d *)&mass_point->powered_force, &mass_point->up, lift, (real_point3d *)&mass_point->powered_force);
		}

		SET_FLAG(mass_point->flags, _point_at_rest_bit, magnitude_squared3d(&mass_point->velocity) < SECONDS_PER_TICK*SECONDS_PER_TICK);
		SET_FLAG(mass_point->flags, _point_on_ground_bit, mass_point->ground_depth > 0.f);
		SET_FLAG(mass_point->flags, _point_in_water_bit, mass_point->water_depth > 0.f);
		if (powered_mass_point_definition)
		{
			if (TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_thrust_bit))
			{
				real thrust = powered_mass_point->thrust_fraction*physics->mass;

				point_from_line3d((real_point3d *)&mass_point->powered_force, &mass_point->forward, thrust, (real_point3d *)&mass_point->powered_force);
			}
			if (TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_antigrav_bit))
			{
				real_point3d point = mass_point->position;
				real_vector3d vector;
				struct collision_result collision;

				scale_vector3d(global_down3d, powered_mass_point_definition->antigrav_height+mass_point_definition->radius, &vector);
				if (collision_test_vector(_collision_test_for_vehicles_flags, &point, &vector, instance->object_index, &collision))
				{
					real distance = (powered_mass_point_definition->antigrav_height+mass_point_definition->radius)*collision.t-mass_point_definition->radius;
					real up_fraction = pin_fraction(mass_point->up.k, powered_mass_point_definition->antigrav_normal_k0, powered_mass_point_definition->antigrav_normal_k1);
					real height_fraction = distance > 0.f ? 1.f-distance/powered_mass_point_definition->antigrav_height : 1.f;
					real normal_velocity = dot_product3d(&mass_point->velocity, &collision.plane.n);
					real force = powered_mass_point->antigrav_fraction*powered_mass_point_definition->antigrav_strength*up_fraction*physics->mass*(global_gravity*height_fraction*height_fraction-powered_mass_point_definition->antigrav_damp_fraction*normal_velocity);

					point_from_line3d((real_point3d *)&mass_point->powered_force, &collision.plane.n, force, (real_point3d *)&mass_point->powered_force);
					SET_FLAG(mass_point->flags, _point_antigraving_bit, TRUE);
				}
			}
		}

		add_vectors3d(&mass_point->force, &mass_point->normal_force, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->ground_friction.friction, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->water_pressure, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->water_friction.friction, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->air_friction.friction, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->powered_force, &mass_point->force);
		cross_product3d(&mass_point->radius, &mass_point->force, &mass_point->torque);
		add_vectors3d(total_force, &mass_point->force, total_force);
		add_vectors3d(total_torque, &mass_point->torque, total_torque);
	}

	return;
}

static void physics_compute_unit_collisions(
	long vehicle_index)
{
	struct collision_model_instance model_instance;
	boolean model_instance_valid = collision_model_instance_new(&model_instance, vehicle_index);
	struct physics_instance physics_instance;

	if (physics_instance_new(&physics_instance, vehicle_index))
	{
		struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
		long unit_indices[MAXIMUM_OBJECTS_PER_MAP];
		short unit_count = objects_in_sphere(FLAG(_object_class_collideable), model_instance_valid ? _object_mask_unit : _object_mask_vehicle, &vehicle->object.location, &vehicle->object.bounding_sphere_center, vehicle->object.bounding_sphere_radius, unit_indices, NUMBEROF(unit_indices));
		short unit_index;

		for (unit_index = 0; unit_index < unit_count; unit_index++)
		{
			long other_unit_index = unit_indices[unit_index];

			switch (object_get_type(other_unit_index))
			{
			case _object_type_biped:
				{
					struct biped_datum *biped = biped_get(other_unit_index);

					match_assert("c:\\halo\\SOURCE\\physics\\physics.c", 669, model_instance_valid);
					if (!TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
					{
						physics_compute_biped_collision(&model_instance, other_unit_index);
					}
				}
				break;
			case _object_type_vehicle:
				if (other_unit_index != vehicle_index)
				{
					struct physics_instance other_physics_instance;

					if (physics_instance_new(&other_physics_instance, other_unit_index))
					{
						struct vehicle_datum *other_vehicle = vehicle_get(other_unit_index);

						if (DATUM_INDEX_TO_ABSOLUTE_INDEX(other_unit_index) < DATUM_INDEX_TO_ABSOLUTE_INDEX(vehicle_index) || TEST_FLAG(other_vehicle->object.flags, _object_at_rest_bit) || other_physics_instance.physics->radius > 0.f)
						{
							physics_compute_vehicle_collision(&physics_instance, &other_physics_instance);
						}
					}
				}
				break;
			}
		}
	}

	return;
}

static boolean physics_compute_biped_collision(
	struct collision_model_instance *instance,
	long biped_index)
{
	real_point3d base;
	real height, width;
	boolean hit = FALSE;

	biped_get_physics_pill(biped_index, &base, &height, &width);
	if (collision_model_test_point(instance, &base))
	{
		hit = TRUE;
	}
	else
	{
		struct collision_feature_list features;
		struct collision_plane collision;
		real_point3d test_center;
		real test_radius, test_width;

		collision_features_new(&features);
		set_real_point3d(&test_center, base.x, base.y, base.z+height*0.5f);
		test_radius = height*0.5f+width;
		test_width = MAX(width-0.015625f, 0.015625f);
		collision_model_get_features_in_sphere(instance, &test_center, test_radius, height, test_width, &features);
		if (collision_features_test_point(&features, &base, &collision))
		{
			hit = TRUE;
		}
	}

	if (hit)
	{
		struct vehicle_datum *vehicle = vehicle_get(instance->object_index);
		struct biped_datum *biped = biped_get(biped_index);
		real speed = magnitude3d(&vehicle->object.translational_velocity);
		real_vector3d acceleration_vector;
		real_point3d new_base;
		boolean fixed;

		vector_from_points3d(&vehicle->object.bounding_sphere_center, &biped->object.bounding_sphere_center, &acceleration_vector);
		normalize3d(&acceleration_vector);
		acceleration_vector.k += 0.8f;
		normalize3d(&acceleration_vector);
		scale_vector3d(&acceleration_vector, MAX(speed, 0.1f), &acceleration_vector);
		add_vectors3d(&acceleration_vector, &vehicle->object.translational_velocity, &acceleration_vector);
		scale_vector3d(&acceleration_vector, 0.5f, &acceleration_vector);
		biped_accelerate(biped_index, &acceleration_vector);
		point_from_line3d(&base, &acceleration_vector, 2.f, &base);
		fixed = collision_fix_pill(_collision_test_for_bipeds_living_flags, &base, width*2.f, height, width, biped_index, &new_base);
		if (fixed)
		{
			new_base.z -= width;
			object_translate(biped_index, &new_base, NULL);
		}

		if (!fixed || ((instance->object_index != biped->unit.last_vehicle_index || game_time_get() > biped->unit.game_time_at_last_vehicle_exit+3*TICKS_PER_SECOND) &&
			(speed > 2.f*SECONDS_PER_TICK || distance_squared3d((real_point3d *)&vehicle->object.translational_velocity, (real_point3d *)&biped->object.translational_velocity) > SECONDS_PER_TICK*SECONDS_PER_TICK)))
		{
			struct game_globals_falling_damage *falling_damage = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->falling_damage, 0, struct game_globals_falling_damage);

			if (falling_damage->vehicle_collision_damage.index != NONE)
			{
				long owner_index = instance->object_index;
				struct object_datum *owner = (struct object_datum *)vehicle;
				struct damage_data damage_data;

				if (vehicle->unit.driver_object_index != NONE)
				{
					owner_index = vehicle->unit.driver_object_index;
					owner = object_get(owner_index);
				}
				damage_data_new(&damage_data, falling_damage->vehicle_collision_damage.index);
				damage_data.scale = 1.f;
				SET_FLAG(damage_data.flags, _damage_area_of_effect_bit, TRUE);
				damage_data.owner_player_index = owner->object.owner_player_index;
				damage_data.owner_object_index = owner->object.owner_object_index != NONE ? owner->object.owner_object_index : owner_index;
				damage_data.owner_team_index = owner->object.owner_team_index;
				damage_data.origin = biped->object.bounding_sphere_center;
				damage_data.epicenter = vehicle->object.bounding_sphere_center;
				damage_data.direction = acceleration_vector;
				normalize3d(&damage_data.direction);
				object_cause_damage(&damage_data, biped_index, NONE, NONE, NONE, NULL);
			}
			if (falling_damage->vehicle_killed_unit_damage_effect.index != NONE)
			{
				static real scales[] = { 0.5f, 0.25f, 1.f };
				struct unit_definition *unit_definition = unit_definition_get(biped->definition_index);
				struct damage_data damage_data;

				damage_data_new(&damage_data, falling_damage->vehicle_killed_unit_damage_effect.index);
				match_assert("c:\\halo\\SOURCE\\physics\\physics.c", 830, unit_definition->unit.blip_type>=0 && unit_definition->unit.blip_type<NUMBEROF(scales));
				damage_data.scale = scales[unit_definition->unit.blip_type];
				damage_data.origin = biped->object.bounding_sphere_center;
				scale_vector3d(&acceleration_vector, -1.f, &damage_data.direction);
				object_cause_damage(&damage_data, instance->object_index, NONE, NONE, NONE, NULL);
			}
		}
	}

	return hit;
}

static boolean physics_compute_vehicle_collision(
	struct physics_instance const *instance0,
	struct physics_instance const *instance1)
{
	boolean hit = FALSE;
	struct vehicle_datum *vehicle0 = vehicle_get(instance0->object_index);
	struct vehicle_datum *vehicle1 = vehicle_get(instance1->object_index);
	real mass = square_root(instance0->physics->mass*instance1->physics->mass);
	real_vector3d total_force0, total_force1, total_torque0, total_torque1;
	short mass_point_index0;

	set_real_vector3d(&total_force0, 0.f, 0.f, 0.f);
	set_real_vector3d(&total_force1, 0.f, 0.f, 0.f);
	set_real_vector3d(&total_torque0, 0.f, 0.f, 0.f);
	set_real_vector3d(&total_torque1, 0.f, 0.f, 0.f);

	for (mass_point_index0 = 0; mass_point_index0 < instance0->physics->mass_points.count; mass_point_index0++)
	{
		struct mass_point_definition const *mass_point0 = TAG_BLOCK_GET_ELEMENT(&instance0->physics->mass_points, mass_point_index0, struct mass_point_definition);
		real_point3d center0;
		short mass_point_index1;

		matrix4x3_transform_point(&instance0->world_matrix, &mass_point0->position, &center0);
		for (mass_point_index1 = 0; mass_point_index1 < instance1->physics->mass_points.count; mass_point_index1++)
		{
			struct mass_point_definition const *mass_point1 = TAG_BLOCK_GET_ELEMENT(&instance1->physics->mass_points, mass_point_index1, struct mass_point_definition);
			real radius = mass_point0->radius+mass_point1->radius;
			real_point3d center1;
			real_vector3d normal;
			real distance;

			matrix4x3_transform_point(&instance1->world_matrix, &mass_point1->position, &center1);
			vector_from_points3d(&center0, &center1, &normal);
			distance = normalize3d(&normal);
			if (distance < radius && distance > 0.f)
			{
				real depth = (radius-distance)*0.5f;
				real force_magnitude = 2.f*mass*global_gravity/global_physics_collision_depth*depth;
				real_vector3d force0, force1, radius0, radius1, torque0, torque1;
				real_point3d point;

				scale_vector3d(&normal, -force_magnitude, &force0);
				scale_vector3d(&normal, force_magnitude, &force1);
				point_from_line3d(&center0, &normal, mass_point0->radius-depth, &point);
				vector_from_points3d(&vehicle0->object.position, &point, &radius0);
				vector_from_points3d(&vehicle1->object.position, &point, &radius1);
				cross_product3d(&radius0, &force0, &torque0);
				cross_product3d(&radius1, &force1, &torque1);
				add_vectors3d(&total_force0, &force0, &total_force0);
				add_vectors3d(&total_force1, &force1, &total_force1);
				add_vectors3d(&total_torque0, &torque0, &total_torque0);
				add_vectors3d(&total_torque1, &torque1, &total_torque1);
				hit = TRUE;
			}
		}
	}

	if (hit)
	{
		add_vectors3d(&vehicle0->vehicle.collision_force, &total_force0, &vehicle0->vehicle.collision_force);
		add_vectors3d(&vehicle0->vehicle.collision_torque, &total_torque0, &vehicle0->vehicle.collision_torque);
		SET_FLAG(vehicle0->object.flags, _object_at_rest_bit, FALSE);
		if (!(instance1->physics->radius > 0.f))
		{
			add_vectors3d(&vehicle1->vehicle.collision_force, &total_force1, &vehicle1->vehicle.collision_force);
			add_vectors3d(&vehicle1->vehicle.collision_torque, &total_torque1, &vehicle1->vehicle.collision_torque);
			SET_FLAG(vehicle1->object.flags, _object_at_rest_bit, FALSE);
		}
	}

	return hit;
}

static void rotate_vectors3d_by_angular_velocity(
	real_vector3d const *forward,
	real_vector3d const *up,
	real_vector3d const *angular_velocity,
	real_vector3d *rotated_forward,
	real_vector3d *rotated_up)
{
	real_vector3d axis = *angular_velocity;
	real angle = normalize3d(&axis);

	match_assert("c:\\halo\\SOURCE\\physics\\physics.c", 944, forward!=rotated_forward);
	match_assert("c:\\halo\\SOURCE\\physics\\physics.c", 945, up!=rotated_up);

	if (angle != 0.f)
	{
		real_matrix4x3 rotation;

		matrix4x3_rotation_from_axis_and_angle(&rotation, &axis, sine(angle), cosine(angle));
		matrix4x3_transform_vector(&rotation, forward, rotated_forward);
		matrix4x3_transform_vector(&rotation, up, rotated_up);
		normalize3d(rotated_forward);
		point_from_line3d((real_point3d *)rotated_up, rotated_forward, -dot_product3d(rotated_up, rotated_forward), (real_point3d *)rotated_up);
		normalize3d(rotated_up);
	}
	else
	{
		*rotated_forward = *forward;
		*rotated_up = *up;
	}

	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\physics\\physics.c", 965, rotated_forward, rotated_up);
	return;
}

void physics_update_new(
	struct physics_instance const *instance,
	struct powered_mass_point_datum const *powered_mass_points,
	struct mass_point_datum const *mass_points,
	real_vector3d const *total_force,
	real_vector3d const *total_torque)
{
	struct vehicle_datum *object = vehicle_get(instance->object_index);
	real_vector3d linear_acceleration, linear_velocity, angular_acceleration, angular_velocity;
	real_vector3d forward, up;
	real_point3d position;
	short mass_points_at_rest_count;
	short mass_points_on_ground_count;
	short mass_points_on_volatile_surface_count;
	short mass_points_in_water_count;
	short mass_point_index;

	match_assert("c:\\halo\\SOURCE\\physics\\physics.c", 986, instance->physics->mass>0.0f);
	scale_vector3d(total_force, 1.f/instance->physics->mass, &linear_acceleration);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 990, &linear_acceleration);
	add_vectors3d(&object->object.translational_velocity, &linear_acceleration, &linear_velocity);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 994, &linear_velocity);
	add_vectors3d((real_vector3d *)&object->object.position, &linear_velocity, (real_vector3d *)&position);

	{
		real_matrix3x3 rotation;
		real_matrix3x3 inertial_matrix_inverse;

		matrix3x3_from_forward_and_up(&rotation, &object->object.forward, &object->object.up);
		matrix3x3_multiply(&rotation, TAG_BLOCK_GET_ELEMENT(&instance->physics->inertial_matrix, _physics_inertial_matrix_inverse, real_matrix3x3), &inertial_matrix_inverse);
		matrix3x3_multiply(&inertial_matrix_inverse, matrix3x3_transpose(&rotation, &rotation), &inertial_matrix_inverse);
		matrix3x3_transform_vector(&inertial_matrix_inverse, total_torque, &angular_acceleration);
	}
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 1008, &angular_acceleration);
	add_vectors3d(&object->object.angular_velocity, &angular_acceleration, &angular_velocity);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 1012, &angular_velocity);
	rotate_vectors3d_by_angular_velocity(&object->object.forward, &object->object.up, &angular_velocity, &forward, &up);
	object->object.translational_velocity = linear_velocity;
	object->object.angular_velocity = angular_velocity;

	if (debug_physics_disable_penetration_freeze)
	{
		object_set_position(instance->object_index, &position, &forward, &up);
	}
	else
	{
		short iterations = 4;
		unsigned long stuck_mass_point_flags;

		do
		{
			real_matrix4x3 world_matrix;
			real_point3d negative_center_of_mass;
			boolean found_collision_valid = FALSE;
			struct collision_result found_collision;
			real_vector3d found_collision_vector;

			iterations--;
			stuck_mass_point_flags = 0;
			matrix4x3_from_point_and_vectors(&world_matrix, &position, &forward, &up);
			set_real_point3d(&negative_center_of_mass, -instance->physics->center_of_mass.x, -instance->physics->center_of_mass.y, -instance->physics->center_of_mass.z);
			matrix4x3_transform_point(&world_matrix, &negative_center_of_mass, &negative_center_of_mass);
			world_matrix.position = negative_center_of_mass;

			for (mass_point_index = 0; mass_point_index < instance->physics->mass_points.count; mass_point_index++)
			{
				struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&instance->physics->mass_points, mass_point_index, struct mass_point_definition);
				struct mass_point_datum const *mass_point = &mass_points[mass_point_index];
				real_point3d new_position;
				real_vector3d vector;
				struct collision_result collision;

				matrix4x3_transform_point(&world_matrix, &mass_point_definition->position, &new_position);
				vector_from_points3d(&mass_point->position, &new_position, &vector);
				if (collision_test_vector(_collision_test_for_vehicles_flags | FLAG(_collision_test_front_facing_surfaces_bit), &mass_point->position, &vector, instance->object_index, &collision))
				{
					SET_FLAG(stuck_mass_point_flags, mass_point_index, TRUE);
					if (!found_collision_valid || found_collision.t > collision.t)
					{
						found_collision_valid = TRUE;
						found_collision_vector = vector;
						found_collision = collision;
					}
				}
			}

			if (found_collision_valid)
			{
				real normal_distance = dot_product3d(&found_collision_vector, &found_collision.plane.n);
				real adjustment = normal_distance != 0.f ? 0.0078125f/fabs(normal_distance) : 0.03125f;
				real t = MAX(found_collision.t-adjustment, 0.f);
				real normal_velocity = dot_product3d(&linear_velocity, &found_collision.plane.n);

				if (normal_velocity < 0.f)
				{
					point_from_line3d((real_point3d *)&linear_velocity, &found_collision.plane.n, normal_velocity*(t-1.f), (real_point3d *)&linear_velocity);
					object->object.translational_velocity = linear_velocity;
					add_vectors3d((real_vector3d *)&object->object.position, &linear_velocity, (real_vector3d *)&position);
				}
				scale_vector3d(&angular_velocity, t, &angular_velocity);
				object->object.angular_velocity = angular_velocity;
				rotate_vectors3d_by_angular_velocity(&object->object.forward, &object->object.up, &angular_velocity, &forward, &up);
			}
			else
			{
				object_set_position(instance->object_index, &position, &forward, &up);
				break;
			}
		}
		while (iterations > 0);

		object->vehicle.stuck_mass_point_flags = stuck_mass_point_flags;
	}

	mass_points_at_rest_count = 0;
	mass_points_on_ground_count = 0;
	mass_points_on_volatile_surface_count = 0;
	mass_points_in_water_count = 0;
	for (mass_point_index = 0; mass_point_index < instance->physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_datum const *mass_point = &mass_points[mass_point_index];

		mass_points_at_rest_count += TEST_FLAG(mass_point->flags, _point_at_rest_bit);
		mass_points_on_ground_count += TEST_FLAG(mass_point->flags, _point_on_ground_bit);
		mass_points_on_volatile_surface_count += TEST_FLAG(mass_point->flags, _point_on_volatile_surface_bit);
		mass_points_in_water_count += TEST_FLAG(mass_point->flags, _point_in_water_bit);
	}

	SET_FLAG(object->object.flags, _object_at_rest_bit, mass_points_at_rest_count == instance->physics->mass_points.count && mass_points_on_ground_count >= 3 && !mass_points_on_volatile_surface_count &&
		magnitude_squared3d(&linear_velocity) <= SECONDS_PER_TICK*SECONDS_PER_TICK &&
		magnitude_squared3d(&angular_velocity) <= (_pi2*SECONDS_PER_TICK)*(_pi2*SECONDS_PER_TICK) &&
		magnitude_squared3d(&linear_acceleration) <= (0.5f*SECONDS_PER_TICK*SECONDS_PER_TICK)*(0.5f*SECONDS_PER_TICK*SECONDS_PER_TICK) &&
		magnitude_squared3d(&angular_acceleration) <= (_pi2*SECONDS_PER_TICK*SECONDS_PER_TICK)*(_pi2*SECONDS_PER_TICK*SECONDS_PER_TICK));
	SET_FLAG(object->object.flags, _object_on_ground_bit, mass_points_on_ground_count > 0);
	SET_FLAG(object->object.flags, _object_on_media_bit, mass_points_in_water_count > 0);
	SET_FLAG(object->object.flags, _object_partially_under_media_bit, mass_points_in_water_count > 0);
	SET_FLAG(object->object.flags, _object_wholly_under_media_bit, mass_points_in_water_count == instance->physics->mass_points.count);
	return;
}

static void physics_update_old(
	long object_index,
	struct powered_mass_point_datum *powered_mass_points,
	struct mass_point_datum *mass_points,
	real_vector3d const *magic_force,
	real_vector3d const *magic_torque)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	struct physics_definition *physics = physics_definition_get(object_definition->object.physics.index);
	real gravity = global_gravity*physics->gravity_scale;
	short mass_points_at_rest_count = 0;
	short mass_points_on_ground_count = 0;
	short mass_points_on_volatile_surface_count = 0;
	short mass_points_in_water_count = 0;
	real_matrix4x3 world_matrix;
	real_vector3d total_force, total_torque, translational_acceleration, angular_acceleration;
	short mass_point_index;

	matrix4x3_from_point_and_vectors(&world_matrix, &object->object.position, &object->object.forward, &object->object.up);
	set_real_vector3d(&total_force, 0.f, 0.f, -gravity*physics->mass);
	set_real_vector3d(&total_torque, 0.f, 0.f, 0.f);
	set_real_vector3d(&translational_acceleration, 0.f, 0.f, 0.f);
	set_real_vector3d(&angular_acceleration, 0.f, 0.f, 0.f);
	if (powered_mass_points)
	{
		short powered_mass_point_index;

		for (powered_mass_point_index = 0; powered_mass_point_index < physics->powered_mass_points.count; powered_mass_point_index++)
		{
			struct powered_mass_point_datum *powered_mass_point = &powered_mass_points[powered_mass_point_index];

			matrix4x3_rotation_from_quaternion(&powered_mass_point->rotation_matrix, &powered_mass_point->rotation);
			matrix4x3_transpose(&powered_mass_point->rotation_matrix);
		}
	}
	csmemset(mass_points, 0, sizeof(struct mass_point_datum)*physics->mass_points.count);
	if (magic_force)
	{
		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 1255, magic_force);
		add_vectors3d(&total_force, magic_force, &total_force);
	}
	if (magic_torque)
	{
		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 1261, magic_torque);
		add_vectors3d(&total_torque, magic_torque, &total_torque);
	}

	for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&physics->mass_points, mass_point_index, struct mass_point_definition);
		struct mass_point_datum *mass_point = &mass_points[mass_point_index];
		struct powered_mass_point_definition const *powered_mass_point_definition = mass_point_definition->powered_mass_point_index != NONE && powered_mass_points ? TAG_BLOCK_GET_ELEMENT(&physics->powered_mass_points, mass_point_definition->powered_mass_point_index, struct powered_mass_point_definition) : NULL;
		struct powered_mass_point_datum const *powered_mass_point = powered_mass_point_definition ? &powered_mass_points[mass_point_definition->powered_mass_point_index] : NULL;
		real weight = mass_point_definition->mass*gravity; // unused, but sapien computes it and it affects codegen
		real_point3d position;

		mass_point->flags = 0;
		vector_from_points3d(&physics->center_of_mass, &mass_point_definition->position, (real_vector3d *)&position);
		matrix4x3_transform_point(&world_matrix, &position, &mass_point->position);
		if (powered_mass_point)
		{
			real_matrix4x3 powered_world_matrix;

			matrix4x3_multiply(&world_matrix, &powered_mass_point->rotation_matrix, &powered_world_matrix);
			matrix4x3_transform_normal(&powered_world_matrix, &mass_point_definition->forward, &mass_point->forward);
			matrix4x3_transform_normal(&powered_world_matrix, &mass_point_definition->up, &mass_point->up);
		}
		else
		{
			matrix4x3_transform_normal(&world_matrix, &mass_point_definition->forward, &mass_point->forward);
			matrix4x3_transform_normal(&world_matrix, &mass_point_definition->up, &mass_point->up);
		}
		scenario_location_from_point(&mass_point->location, &mass_point->position);
		vector_from_points3d(&object->object.position, &mass_point->position, &mass_point->radius);
		cross_product3d(&object->object.angular_velocity, &mass_point->radius, &mass_point->velocity);
		add_vectors3d(&mass_point->velocity, &object->object.translational_velocity, &mass_point->velocity);
		compute_ground_plane(object_index, mass_point, mass_point_definition);
		mass_point->water_depth = scenario_location_water_depth(&mass_point->location, &mass_point->position);

		if (mass_point->ground_depth > 0.f && physics->ground_depth > 0.f)
		{
			real normal_velocity = dot_product3d(&mass_point->velocity, &mass_point->ground_plane.n);
			real friction_scale;

			mass_point->normal_force_magnitude = (global_gravity/physics->ground_depth*mass_point->ground_depth-normal_velocity*physics->ground_damp_fraction)*physics->mass;
			scale_vector3d(&mass_point->ground_plane.n, mass_point->normal_force_magnitude, &mass_point->normal_force);
			friction_scale = -physics->ground_friction*mass_point_definition->mass;
			point_from_line3d((real_point3d *)&mass_point->velocity, &mass_point->ground_plane.n, -normal_velocity, (real_point3d *)&mass_point->velocity_relative_to_ground);
			scale_vector3d(&mass_point->velocity_relative_to_ground, friction_scale, &mass_point->ground_friction.friction);

			if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_ground_friction_bit) && powered_mass_point->ground_friction_velocity != 0.f)
			{
				real ground_fraction = pin_fraction(mass_point->ground_plane.n.k, physics->ground_normal_k0, physics->ground_normal_k1);
				real up_fraction = dot_product3d(&mass_point->up, &mass_point->ground_plane.n);
				real up_fraction_clamped = PIN(up_fraction, 0.f, 1.f);
				real powered_friction_scale = ground_fraction*ground_fraction*up_fraction_clamped*up_fraction_clamped*friction_scale;
				real_vector3d powered_velocity, tangent_velocity;
				real powered_normal_velocity;

				scale_vector3d(&mass_point->forward, -powered_mass_point->ground_friction_velocity, &powered_velocity);
				powered_normal_velocity = dot_product3d(&powered_velocity, &mass_point->ground_plane.n);
				point_from_line3d((real_point3d *)&powered_velocity, &mass_point->ground_plane.n, -powered_normal_velocity, (real_point3d *)&tangent_velocity);
				add_vectors3d(&mass_point->velocity_relative_to_ground, &tangent_velocity, &mass_point->velocity_relative_to_ground);
				point_from_line3d((real_point3d *)&mass_point->ground_friction.friction, &tangent_velocity, powered_friction_scale, (real_point3d *)&mass_point->ground_friction.friction);
			}
			if (mass_point->ground_material_type!=_material_ice)
			{
				friction_evaluate(mass_point_definition->friction_type, mass_point_definition->friction_parallel_scale, mass_point_definition->friction_perpendicular_scale, &mass_point->ground_friction, &mass_point->forward, &mass_point->up);
			}
			else
			{
				real const ice_scale = 0.125f;

				friction_evaluate(mass_point_definition->friction_type, ice_scale*mass_point_definition->friction_parallel_scale, ice_scale*mass_point_definition->friction_perpendicular_scale, &mass_point->ground_friction, &mass_point->forward, &mass_point->up);
			}
		}

		if (mass_point->water_depth > 0.f)
		{
			real water_fraction = mass_point->water_depth < physics->water_depth ? mass_point->water_depth/physics->water_depth : 1.f;

			if (mass_point_definition->density > 0.f && physics->water_depth > 0.f)
			{
				mass_point->water_pressure_magnitude = physics->water_density/mass_point_definition->density*mass_point_definition->mass*water_fraction*gravity;
				set_real_vector3d(&mass_point->water_pressure, 0.f, 0.f, mass_point->water_pressure_magnitude);
			}
			if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_water_friction_bit) && powered_mass_point->water_friction_velocity != 0.f)
			{
				real_vector3d velocity;

				point_from_line3d((real_point3d *)&mass_point->velocity, &mass_point->forward, -powered_mass_point->water_friction_velocity, (real_point3d *)&velocity);
				scale_vector3d(&velocity, -physics->water_friction*mass_point_definition->mass, &mass_point->water_friction.friction);
			}
			else
			{
				scale_vector3d(&mass_point->velocity, -physics->water_friction*mass_point_definition->mass, &mass_point->water_friction.friction);
			}
			friction_evaluate(mass_point_definition->friction_type, mass_point_definition->friction_parallel_scale, mass_point_definition->friction_perpendicular_scale, &mass_point->water_friction, &mass_point->forward, &mass_point->up);

			if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_water_lift_bit) && powered_mass_point->water_lift_ratio != 0.f)
			{
				real speed = fabs(dot_product3d(&mass_point->velocity, &mass_point->forward));
				real lift = water_fraction*powered_mass_point->water_lift_ratio*physics->mass*speed;

				point_from_line3d((real_point3d *)&mass_point->powered_force, &mass_point->up, lift, (real_point3d *)&mass_point->powered_force);
			}
		}

		if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_air_friction_bit) && powered_mass_point->air_friction_velocity != 0.f)
		{
			real_vector3d velocity;

			point_from_line3d((real_point3d *)&mass_point->velocity, &mass_point->forward, -powered_mass_point->air_friction_velocity, (real_point3d *)&velocity);
			scale_vector3d(&velocity, -physics->air_friction*mass_point_definition->mass, &mass_point->air_friction.friction);
		}
		else
		{
			scale_vector3d(&mass_point->velocity, -physics->air_friction*mass_point_definition->mass, &mass_point->air_friction.friction);
		}
		friction_evaluate(mass_point_definition->friction_type, mass_point_definition->friction_parallel_scale, mass_point_definition->friction_perpendicular_scale, &mass_point->air_friction, &mass_point->forward, &mass_point->up);
		if (powered_mass_point_definition && TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_air_lift_bit) && powered_mass_point->air_lift_ratio != 0.f)
		{
			real speed = fabs(dot_product3d(&mass_point->velocity, &mass_point->forward));
			real lift = powered_mass_point->air_lift_ratio*physics->mass*speed;

			point_from_line3d((real_point3d *)&mass_point->powered_force, &mass_point->up, lift, (real_point3d *)&mass_point->powered_force);
		}

		SET_FLAG(mass_point->flags, _point_at_rest_bit, dot_product3d(&mass_point->velocity, &mass_point->velocity) < SECONDS_PER_TICK*SECONDS_PER_TICK);
		SET_FLAG(mass_point->flags, _point_on_ground_bit, mass_point->ground_depth > 0.f);
		SET_FLAG(mass_point->flags, _point_in_water_bit, mass_point->water_depth > 0.f);
		mass_points_at_rest_count += TEST_FLAG(mass_point->flags, _point_at_rest_bit);
		mass_points_on_ground_count += TEST_FLAG(mass_point->flags, _point_on_ground_bit);
		mass_points_on_volatile_surface_count += TEST_FLAG(mass_point->flags, _point_on_volatile_surface_bit);
		mass_points_in_water_count += TEST_FLAG(mass_point->flags, _point_in_water_bit);
		if (powered_mass_point_definition)
		{
			if (TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_thrust_bit))
			{
				real thrust = powered_mass_point->thrust_fraction*physics->mass;

				point_from_line3d((real_point3d *)&mass_point->powered_force, &mass_point->forward, thrust, (real_point3d *)&mass_point->powered_force);
			}
			if (TEST_FLAG(powered_mass_point_definition->flags, _powered_mass_point_antigrav_bit))
			{
				real_point3d point = mass_point->position;
				real_vector3d vector;
				struct collision_result collision;

				scale_vector3d(global_down3d, powered_mass_point_definition->antigrav_height+mass_point_definition->radius, &vector);
				if (collision_test_vector(_collision_test_for_vehicles_flags, &point, &vector, object_index, &collision))
				{
					real distance = (powered_mass_point_definition->antigrav_height+mass_point_definition->radius)*collision.t-mass_point_definition->radius;
					real up_fraction = pin_fraction(mass_point->up.k, powered_mass_point_definition->antigrav_normal_k0, powered_mass_point_definition->antigrav_normal_k1);
					real height_fraction = distance > 0.f ? 1.f-distance/powered_mass_point_definition->antigrav_height : 1.f;
					real normal_velocity = dot_product3d(&mass_point->velocity, &collision.plane.n);
					real force = powered_mass_point->antigrav_fraction*powered_mass_point_definition->antigrav_strength*up_fraction*physics->mass*(global_gravity*height_fraction*height_fraction-powered_mass_point_definition->antigrav_damp_fraction*normal_velocity);

					point_from_line3d((real_point3d *)&mass_point->powered_force, &collision.plane.n, force, (real_point3d *)&mass_point->powered_force);
				}
			}
		}

		add_vectors3d(&mass_point->force, &mass_point->normal_force, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->ground_friction.friction, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->water_pressure, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->water_friction.friction, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->air_friction.friction, &mass_point->force);
		add_vectors3d(&mass_point->force, &mass_point->powered_force, &mass_point->force);
		cross_product3d(&mass_point->radius, &mass_point->force, &mass_point->torque);
		add_vectors3d(&total_force, &mass_point->force, &total_force);
		add_vectors3d(&total_torque, &mass_point->torque, &total_torque);
	}

	if (physics->mass != 0.f)
	{
		scale_vector3d(&total_force, 1.f/physics->mass, &translational_acceleration);
	}
	{
		real_vector3d torque_axle = total_torque;

		if (normalize3d(&torque_axle) != 0.f)
		{
			real moment = 0.f;
			short mass_point_index;

			for (mass_point_index = 0; mass_point_index < physics->mass_points.count; mass_point_index++)
			{
				struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&physics->mass_points, mass_point_index, struct mass_point_definition);
				struct mass_point_datum *mass_point = &mass_points[mass_point_index];
				real_vector3d radius;

				point_from_line3d((real_point3d *)&mass_point->radius, &torque_axle, -dot_product3d(&mass_point->radius, &torque_axle), (real_point3d *)&radius);
				moment += (magnitude_squared3d(&radius)+0.4f*mass_point_definition->radius*mass_point_definition->radius)*mass_point_definition->mass*physics->moment;
			}
			if (moment != 0.f)
			{
				scale_vector3d(&total_torque, 1.f/moment, &angular_acceleration);
			}
		}
	}

	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 1539, &translational_acceleration);
	add_vectors3d(&object->object.translational_velocity, &translational_acceleration, &object->object.translational_velocity);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\physics.c", 1543, &angular_acceleration);
	add_vectors3d(&object->object.angular_velocity, &angular_acceleration, &object->object.angular_velocity);
	{
		real_point3d new_position;
		struct location new_location;

		add_vectors3d((real_vector3d *)&object->object.position, &object->object.translational_velocity, (real_vector3d *)&new_position);
		scenario_location_from_line(&new_location, &object->object.location, &object->object.position, &new_position);
		object_translate(object_index, &new_position, &new_location);
	}
	{
		real_vector3d axis = object->object.angular_velocity;
		real angle = normalize3d(&axis);

		if (angle != 0.f)
		{
			real sine = sin(angle);
			real cosine = cos(angle);

			rotate_vector_about_axis(&object->object.forward, &axis, sine, cosine);
			rotate_vector_about_axis(&object->object.up, &axis, sine, cosine);
			normalize3d(&object->object.forward);
			point_from_line3d((real_point3d *)&object->object.up, &object->object.forward, -dot_product3d(&object->object.up, &object->object.forward), (real_point3d *)&object->object.up);
			normalize3d(&object->object.up);
		}
	}

	{
		real const translational_velocity_threshold = SECONDS_PER_TICK*SECONDS_PER_TICK;
		real const angular_velocity_threshold = (_pi2*SECONDS_PER_TICK)*(_pi2*SECONDS_PER_TICK);
		real const translational_acceleration_threshold = (0.5f*SECONDS_PER_TICK*SECONDS_PER_TICK)*(0.5f*SECONDS_PER_TICK*SECONDS_PER_TICK);
		real const angular_acceleration_threshold = (_pi2*SECONDS_PER_TICK*SECONDS_PER_TICK)*(_pi2*SECONDS_PER_TICK*SECONDS_PER_TICK);
		real translational_velocity_squared = magnitude_squared3d(&object->object.translational_velocity);
		real angular_velocity_squared = magnitude_squared3d(&object->object.angular_velocity);
		real translational_acceleration_squared = magnitude_squared3d(&translational_acceleration);
		real angular_acceleration_squared = magnitude_squared3d(&angular_acceleration);

		SET_FLAG(object->object.flags, _object_at_rest_bit, mass_points_at_rest_count == physics->mass_points.count && mass_points_on_ground_count >= 3 && !mass_points_on_volatile_surface_count &&
			translational_velocity_squared <= translational_velocity_threshold && angular_velocity_squared <= angular_velocity_threshold &&
			translational_acceleration_squared <= translational_acceleration_threshold && angular_acceleration_squared <= angular_acceleration_threshold);
	}
	SET_FLAG(object->object.flags, _object_on_ground_bit, mass_points_on_ground_count > 0);
	SET_FLAG(object->object.flags, _object_on_media_bit, mass_points_in_water_count > 0);
	SET_FLAG(object->object.flags, _object_partially_under_media_bit, mass_points_in_water_count > 0);
	SET_FLAG(object->object.flags, _object_wholly_under_media_bit, mass_points_in_water_count == physics->mass_points.count);
	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\physics\\physics.c", 1595, &object->object.forward, &object->object.up);
	return;
}

void render_debug_physics(
	struct physics_instance *instance)
{
	struct object_datum *object = object_get(instance->object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	real_point3d center_of_mass;
	short mass_point_index;

	matrix4x3_transform_point(&instance->world_matrix, &instance->physics->center_of_mass, &center_of_mass);
	render_debug_vectors(TRUE, &center_of_mass, &instance->world_matrix.forward, &instance->world_matrix.up, object_definition->object.bounding_radius);

	for (mass_point_index = 0; mass_point_index < instance->physics->mass_points.count; mass_point_index++)
	{
		struct mass_point_definition const *mass_point_definition = TAG_BLOCK_GET_ELEMENT(&instance->physics->mass_points, mass_point_index, struct mass_point_definition);
		real_point3d position;
		real_vector3d forward, up;

		matrix4x3_transform_point(&instance->world_matrix, &mass_point_definition->position, &position);
		matrix4x3_transform_normal(&instance->world_matrix, &mass_point_definition->forward, &forward);
		matrix4x3_transform_normal(&instance->world_matrix, &mass_point_definition->up, &up);
		render_debug_sphere(TRUE, &position, mass_point_definition->radius, global_real_argb_white);
		render_debug_vectors(TRUE, &position, &forward, &up, mass_point_definition->radius*0.5f);
	}

	return;
}
