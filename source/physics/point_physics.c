/*
POINT_PHYSICS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"
#include "point_physics.h"
#include "physics.h"
#include "collisions.h"
#include "collision_usage.h"
#include "objects.h"
#include "render_debug.h"
#include "scenario.h"

/* ---------- constants */

enum
{
	MAXIMUM_POINT_PHYSICS_COLLISIONS = 3
};

/* ---------- prototypes */

static void render_debug_point_physics(
	struct point_physics_definition const *definition,
	real_point3d const *position,
	real radius);

/* ---------- globals */

struct tag_reference_definition global_point_physics_reference = { 0, POINT_PHYSICS_DEFINITION_TAG, NULL };

boolean debug_point_physics = FALSE;

static real global_air_mass_over_radius_cubed = 0.0f;
static real global_water_mass_over_radius_cubed = 0.0f;

/* ---------- public code */

void point_physics_initialize_for_new_map(
	void)
{
	global_air_mass_over_radius_cubed = global_air_density * 118613.34f;
	global_water_mass_over_radius_cubed = global_water_density * 118613.34f;

	return;
}

void point_physics_dispose_from_old_map(
	void)
{
	return;
}

real point_physics_definition_get_mass(
	struct point_physics_definition *definition,
	real radius)
{
	return definition->runtime_mass_over_radius_cubed * radius * radius * radius;
}

unsigned long point_physics_update(
	unsigned long flags,
	struct point_physics_definition const *definition,
	struct location *location,
	short force_weather_palette_index,
	real_point3d *position,
	real_vector3d *translational_velocity,
	real_vector3d const *translational_force,
	real_vector3d *collision_normal,
	short *collision_material_type,
	real radius,
	real dt)
{
	unsigned long result = 0;

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\point_physics.c", 185, position);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\point_physics.c", 186, translational_velocity);
	match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 187, !translational_force || valid_real_vector3d(translational_force));
	match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 188, radius>=0.f);

	if (dt != 0.0f)
	{
		struct collision_result collision;
		real_vector3d wind_vector;
		real_vector3d delta;
		real_vector3d parallel;
		real_vector3d perpendicular;
		real radius_squared = radius * radius;
		real radius_cubed = radius_squared * radius;
		real mass = definition->runtime_mass_over_radius_cubed;
		real buoyancy_scale;
		real friction;
		real dt_over_mass;
		real offset;
		real t;
		unsigned long collision_flags;
		long wind_flags;
		boolean underwater;
		short i;

		wind_flags = 0;

		SET_FLAG(wind_flags, _scenario_current_simple_bit, TEST_FLAG(definition->flags, _point_physics_simple_wind_bit));
		SET_FLAG(wind_flags, _scenario_current_damped_bit, TEST_FLAG(definition->flags, _point_physics_damped_wind_bit));

		if (TEST_FLAG(flags, _point_physics_ignore_position_bit))
		{
			underwater = TEST_FLAG(flags, _point_physics_ignore_position_under_water_bit);
			scenario_get_current_from_weather_palette(position, &wind_vector, wind_flags, force_weather_palette_index);
		}
		else
		{
			underwater = scenario_get_current(location, position, &wind_vector, wind_flags);
		}

		if (underwater)
		{
			mass += global_water_mass_over_radius_cubed;
			buoyancy_scale = definition->runtime_water_buoyancy_scale;
			friction = definition->water_friction * radius_squared;
			SET_FLAG(result, _point_physics_in_water_bit, TRUE);
		}
		else
		{
			mass += global_air_mass_over_radius_cubed;
			buoyancy_scale = definition->runtime_air_buoyancy_scale;
			friction = definition->air_friction * radius_squared;
			SET_FLAG(result, _point_physics_in_air_bit, TRUE);
		}

		mass = mass * radius_cubed;
		dt_over_mass = dt / mass;

		if (TEST_FLAG(definition->flags, _point_physics_no_gravity_bit))
		{
			buoyancy_scale = 0.0f;
		}

		if (translational_force && mass != 0.0f)
		{
			translational_velocity->i += dt_over_mass * translational_force->i;
			translational_velocity->j += dt_over_mass * translational_force->j;
			translational_velocity->k += dt_over_mass * translational_force->k;
		}

		translational_velocity->k = ((global_gravity * TICKS_PER_SECOND) * TICKS_PER_SECOND) * buoyancy_scale * dt + translational_velocity->k;

		if (mass == 0.0f)
		{
			t = (friction == 0.0f) ? 0.0f : 1.0f;
		}
		else
		{
			t = dt_over_mass * friction;
			t = PIN(t, 0.0f, 1.0f);
		}

		translational_velocity->i += (wind_vector.i - translational_velocity->i) * t;
		translational_velocity->j += (wind_vector.j - translational_velocity->j) * t;
		translational_velocity->k += (wind_vector.k - translational_velocity->k) * t;

		collision_flags = FLAG(_collision_test_front_facing_surfaces_bit);
		SET_FLAG(collision_flags, _collision_test_media_bit, TEST_FLAG(definition->flags, _point_physics_water_collisions_bit) && !TEST_FLAG(flags, _point_physics_force_no_collisions_bit));
		SET_FLAG(collision_flags, _collision_test_structure_bit, TEST_FLAG(definition->flags, _point_physics_structure_collisions_bit) && !TEST_FLAG(flags, _point_physics_force_no_collisions_bit));

		match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 269, global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
		global_current_collision_users[global_current_collision_user_depth++] = _collision_user_point_physics;

		for (i = 0; dt != 0.0f && i < MAXIMUM_POINT_PHYSICS_COLLISIONS; i++)
		{
			delta.i = translational_velocity->i * dt;
			delta.j = translational_velocity->j * dt;
			delta.k = translational_velocity->k * dt;

			if (!collision_test_vector(collision_flags, position, &delta, NONE, &collision))
			{
				if (collision.location.leaf_index != NONE)
				{
					*location = collision.location;
				}

				*position = collision.point;
				break;
			}

			offset = MIN(radius, 0.005f);

			if (collision.type == _collision_result_media)
			{
				SET_FLAG(result, _point_physics_collided_with_water_bit, TRUE);
			}
			else if (collision.type == _collision_result_structure)
			{
				SET_FLAG(result, _point_physics_collided_with_structure_bit, TRUE);
			}

			if (collision_normal)
			{
				*collision_normal = collision.plane.n;
			}

			if (collision_material_type)
			{
				*collision_material_type = collision.material_type;
			}

			component_vectors_from_normal3d(translational_velocity, &collision.plane.n, &parallel, &perpendicular);
			translational_velocity->i = (1.0f - definition->contact_friction) * perpendicular.i - parallel.i * definition->elasticity;
			translational_velocity->j = (1.0f - definition->contact_friction) * perpendicular.j - parallel.j * definition->elasticity;
			translational_velocity->k = (1.0f - definition->contact_friction) * perpendicular.k - parallel.k * definition->elasticity;

			if (collision.location.leaf_index != NONE)
			{
				*location = collision.location;
			}

			position->x = collision.plane.n.i * offset + collision.point.x;
			position->y = collision.plane.n.j * offset + collision.point.y;
			position->z = collision.plane.n.k * offset + collision.point.z;

			dt -= collision.t * dt;
		}

		match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 312, global_current_collision_user_depth > 1);
		--global_current_collision_user_depth;
	}

	if (debug_point_physics)
	{
		render_debug_point_physics(definition, position, radius);
	}

	return result;
}

struct point_physics_definition *point_physics_definition_interpolate(
	struct point_physics_definition const *physics1,
	struct point_physics_definition const *physics2,
	real t,
	struct point_physics_definition *result)
{
	real t0 = 1.0f - t;

	match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 332, physics1);
	match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 333, physics2);
	match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 334, t>=0.f && t<=1.f);
	match_assert("c:\\halo\\SOURCE\\physics\\point_physics.c", 335, result);

	result->flags = physics1->flags;

	result->density = t0*physics1->density + t*physics2->density;
	result->runtime_water_buoyancy_scale = t0*physics1->runtime_water_buoyancy_scale + t*physics2->runtime_water_buoyancy_scale;
	result->runtime_air_buoyancy_scale = t0*physics1->runtime_air_buoyancy_scale + t*physics2->runtime_air_buoyancy_scale;
	result->runtime_mass_over_radius_cubed = t0*physics1->runtime_mass_over_radius_cubed + t*physics2->runtime_mass_over_radius_cubed;
	result->air_friction = t0*physics1->air_friction + t*physics2->air_friction;
	result->water_friction = t0*physics1->water_friction + t*physics2->water_friction;
	result->contact_friction = t0*physics1->contact_friction + t*physics2->contact_friction;
	result->elasticity = t0*physics1->elasticity + t*physics2->elasticity;

	return result;
}

/* ---------- private code */

static void render_debug_point_physics(
	struct point_physics_definition const *definition,
	real_point3d const *position,
	real radius)
{
	real_argb_color const *color = TEST_FLAG(definition->flags, _point_physics_structure_collisions_bit)
		? global_real_argb_red
		: global_real_argb_green;

	render_debug_point(TRUE, position, radius, color);

	return;
}
