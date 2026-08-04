/*
BREAKABLE_SURFACES.C

*/

/* ---------- headers */

#include "cseries.h"
#include "breakable_surfaces.h"

#include "collision_bsp_definitions.h"

#include "bitmaps/bitmaps.h"
#include "cseries/errors.h"
#include "effects/material_effect_definitions.h"
#include "effects/particles.h"
#include "game/game_globals.h"
#include "math/geometry.h"
#include "math/real_math.h"
#include "objects/damage.h"
#include "objects/damage_effect_definitions.h"
#include "objects/objects.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "sound/game_sound.h"
#include "structures/structure_bsp_definitions.h"
#include "tag_files/tag_groups.h"

/* ---------- structures */

struct breakable_surface_globals
{
	boolean enabled;
	byte breakable_surface_flags[MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO][32];
	struct breakable_surface_datum breakable_surfaces[MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO][MAXIMUM_BREAKABLE_SURFACES_PER_MAP];
};

/* ---------- prototypes */

static void breakable_surface_effect(
	short breakable_surface_index,
	const struct damage_data *damage_data,
	long seed_surface_index);

/* ---------- globals */

static struct breakable_surface_globals *globals;

boolean breakable_surface_effect_enabled = TRUE;

/* ---------- public code */

struct breakable_surface_datum *breakable_surface_get(
	short breakable_surface_index)
{
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 61, globals);
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 62, global_structure_bsp_index>=0 && global_structure_bsp_index<MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO);
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 63, breakable_surface_index>=0 && breakable_surface_index<MAXIMUM_BREAKABLE_SURFACES_PER_MAP);

	return &globals->breakable_surfaces[global_structure_bsp_index][breakable_surface_index];
}

void breakable_surfaces_initialize(
	void)
{
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 73, !globals);
	
	globals = (struct breakable_surface_globals *)game_state_malloc("breakable surface globals", 0, sizeof(struct breakable_surface_globals));
	return;
}

void breakable_surfaces_dispose(
	void)
{
	return;
}

void breakable_surfaces_initialize_for_new_map(
	void)
{
	short bsp_index;

	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 90, globals);

	globals->enabled = TRUE;

	for (bsp_index = 0; bsp_index<MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO; ++bsp_index)
	{
		short surface_index;

		csmemset(globals->breakable_surface_flags[bsp_index], NONE, sizeof(globals->breakable_surface_flags[bsp_index]));

		for (surface_index = 0; surface_index < MAXIMUM_BREAKABLE_SURFACES_PER_MAP; ++surface_index)
		{
			globals->breakable_surfaces[bsp_index][surface_index].vitality = 1.0f;
		}
	}

	return;
}

void breakable_surfaces_dispose_from_old_map(
	void)
{
	return;
}

void breakable_surfaces_enable(
	boolean state)
{
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 120, globals);
	
	globals->enabled = state;
	return;
}

void breakable_surfaces_reset(
	void)
{
	breakable_surfaces_initialize_for_new_map();
	return;
}

byte *breakable_surface_flags_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 138, globals);
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 139, global_structure_bsp_index>=0 && global_structure_bsp_index<MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO);

	return globals->breakable_surface_flags[global_structure_bsp_index];
}

boolean breakable_surface_extant(
	short breakable_surface_index)
{
	boolean result;

	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 147, breakable_surface_index==NONE || (breakable_surface_index>=0 && breakable_surface_index<MAXIMUM_BREAKABLE_SURFACES_PER_MAP));

	result = breakable_surface_index==NONE || BIT_VECTOR_TEST_FLAG((long *)breakable_surface_flags_get(), breakable_surface_index);
	return result;
}

void breakable_surface_damage(
	short breakable_surface_index,
	struct damage_data *damage_data,
	long seed_surface_index)
{
	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 163, globals);

	if (globals->enabled)
	{
		if (breakable_surface_index != NONE &&
			damage_data->definition_index != NONE &&
			damage_data->material_type != NONE)
		{
			struct breakable_surface_datum *surface = breakable_surface_get(breakable_surface_index);

			if (surface->vitality > 0.0f)
			{
				struct material_definition *material = scenario_material_definition_get(damage_data->material_type);

				if (material)
				{
					if (material->breakable_surface.maximum_vitality > 0.0f)
					{
						struct damage_effect_definition *definition = damage_effect_definition_get(damage_data->definition_index);
						struct damage_definition *damage = &definition->damage;
						real damage_amount = real_random_range(damage->damage_lower_bound, damage->damage_upper_bound);

						real damage_fraction = ((damage_amount - damage->damage_minimum) * damage_data->scale + damage->damage_minimum);

						damage_fraction *= damage->material_modifiers[damage_data->material_type];
						damage_fraction /= material->breakable_surface.maximum_vitality;

						surface->vitality -= damage_fraction;

						if (surface->vitality <= 0.0f)
						{
							BIT_VECTOR_SET_FLAG((long *)breakable_surface_flags_get(), breakable_surface_index, FALSE);

							breakable_surface_effect(breakable_surface_index, damage_data, seed_surface_index);
						}
					}
				}
			}
		}
	}

	return;
}


void breakable_surface_damage_area_of_effect(
	const struct damage_data *damage_data)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct damage_effect_definition *damage_effect_definition = damage_effect_definition_get(damage_data->definition_index);

	if (globals->enabled &&
		(damage_effect_definition->damage.damage_lower_bound!= 0.0f || damage_effect_definition->damage.damage_upper_bound!=0.0f))
	{
		short breakable_surface_index;

		real cutoff_radius = damage_effect_definition->cutoff_radius;

		if (cutoff_radius > 4.0f)
		{
			error(_error_silent, "WARNING: area of effect breakable surface damage with radius %d", cutoff_radius);
		}

		for (breakable_surface_index = 0; breakable_surface_index < structure_bsp->breakable_surfaces.count; breakable_surface_index++)
		{
			if (breakable_surface_extant(breakable_surface_index))
			{
				struct structure_breakable_surface *breakable_surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->breakable_surfaces, breakable_surface_index, struct structure_breakable_surface);
				real radius = breakable_surface->bounding_radius;
				radius += cutoff_radius;

				if (point_in_sphere(&breakable_surface->centroid, &damage_data->epicenter, radius))
				{
					breakable_surface_get(breakable_surface_index)->vitality = 0.0f;
					BIT_VECTOR_SET_FLAG((long *)breakable_surface_flags_get(), breakable_surface_index, FALSE);
					breakable_surface_effect(breakable_surface_index, damage_data, breakable_surface->collision_surface_index);
				}
			}
		}
	}
	
	return;
}

/* ---------- private code */

static void breakable_surface_effect(
	short breakable_surface_index,
	const struct damage_data *damage_data,
	long seed_surface_index)
{
	struct breakable_surface const *breakable_surface;
	short surface_queue_read_index;
	short surface_queue_write_index;
	boolean total_bounds_valid;
	real_rectangle3d total_bounds;
	long surface_queue[MAXIMUM_BREAKABLE_SURFACE_QUEUE_SIZE];
	struct collision_surface const *collision_surface;
	long surface_index;
	real_plane3d s_plane;
	real_point3d origin;
	real_plane3d surface_plane;
	real_rectangle2d surface_bounds;
	real_plane3d t_plane;
	real_point3d surface_vertices3d[MAXIMUM_VERTICES_PER_COLLISION_SURFACE];
	real_point3d position;
	real_vector3d velocity;
	
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct collision_bsp *collision_bsp = global_collision_bsp_get();

	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 251, damage_data);

	if (breakable_surface_effect_enabled)
	{
		long material_type;
		struct material_definition *material_def;

		collision_surface = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, seed_surface_index, struct collision_surface);
		material_type = TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_materials, collision_surface->material_index, struct structure_collision_material)->runtime_physics_material_type;
		material_def = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->materials, material_type, struct material_definition);
		breakable_surface = &material_def->breakable_surface;

		total_bounds_valid = FALSE;
		surface_queue_read_index = 0;
		surface_queue_write_index = 0;

		match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 266, collision_surface->breakable_surface_index==breakable_surface_index);

		surface_queue[surface_queue_write_index++] = seed_surface_index;

		while (surface_queue_read_index<surface_queue_write_index)
		{
			real_point2d surface_vertices2d[MAXIMUM_VERTICES_PER_COLLISION_SURFACE];
			boolean projection_sign;
			short projection_axis;
			short particle_index;
			struct collision_surface const *surface;
			long edge_index;
			short surface_vertex_index;
			short surface_queue_index;

			surface_index = surface_queue[surface_queue_read_index++];
			surface = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, surface_index, struct collision_surface);
			
			edge_index = surface->first_edge_index;
			surface_vertex_index = 0;

			bsp3d_get_plane_from_designator(&collision_bsp->bsp3d, surface->plane_designator, &surface_plane);
			projection_axis = projection_from_vector3d(&surface_plane.n);
			projection_sign = projection_sign_from_vector3d(&surface_plane.n, projection_axis);
			
			do
			{
				struct collision_edge const *collision_edge = TAG_BLOCK_GET_ELEMENT(&collision_bsp->edges, edge_index, struct collision_edge);
				boolean reverse = (collision_edge->surface_indices[1] == surface_index);
				real_point3d const *vertex_point = &TAG_BLOCK_GET_ELEMENT(&collision_bsp->vertices, collision_edge->vertex_indices[!reverse], struct collision_vertex)->point;
				long adjacent_surface_index = collision_edge->surface_indices[!reverse];

				if (surface_vertex_index==0)
				{
					real_point3d const *next_vertex_point = &TAG_BLOCK_GET_ELEMENT(&collision_bsp->vertices, collision_edge->vertex_indices[reverse], struct collision_vertex)->point;
					real_vector3d t_normal;
					real_vector3d s_normal;

					if (surface_index==seed_surface_index)
					{
						real_point2d temp_2d;
						project_point3d(&damage_data->epicenter, projection_axis, projection_sign, &temp_2d);
						project_point2d(&temp_2d, &surface_plane, projection_axis, projection_sign, &origin);
					}
					else
					{
						origin = *vertex_point;
					}

					vector_from_points3d(vertex_point, next_vertex_point, &s_normal);
					normalize3d(&s_normal);

					cross_product3d(&s_normal, &surface_plane.n, &t_normal);

					plane3d_from_point_and_normal(&s_plane, &origin, &s_normal);
					plane3d_from_point_and_normal(&t_plane, &origin, &t_normal);

					surface_bounds.x1 = plane3d_distance_to_point(&s_plane, vertex_point);
					surface_bounds.x0 = surface_bounds.x1;
					surface_bounds.y1 = plane3d_distance_to_point(&t_plane, vertex_point);
					surface_bounds.y0 = surface_bounds.y1;
				}
				else
				{
					real s = plane3d_distance_to_point(&s_plane, vertex_point);
					real t = plane3d_distance_to_point(&t_plane, vertex_point);

					surface_bounds.x0 = MIN(s, surface_bounds.x0);
					surface_bounds.y0 = MIN(t, surface_bounds.y0);
					surface_bounds.x1 = MAX(s, surface_bounds.x1);
					surface_bounds.y1 = MAX(t, surface_bounds.y1);
				}
				
				match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 348, surface_vertex_index<MAXIMUM_VERTICES_PER_COLLISION_SURFACE);

				surface_vertices3d[surface_vertex_index] = *vertex_point;
				project_point3d(vertex_point, projection_axis, projection_sign, &surface_vertices2d[surface_vertex_index]);

				if (total_bounds_valid)
				{
					total_bounds.x0 = MIN(vertex_point->x, total_bounds.x0);
					total_bounds.y0 = MIN(vertex_point->y, total_bounds.y0);
					total_bounds.z0 = MIN(vertex_point->z, total_bounds.z0);
					total_bounds.x1 = MAX(vertex_point->x, total_bounds.x1);
					total_bounds.y1 = MAX(vertex_point->y, total_bounds.y1);
					total_bounds.z1 = MAX(vertex_point->z, total_bounds.z1);
				}
				else
				{
					total_bounds.x1 = vertex_point->x;
					total_bounds.x0 = total_bounds.x1;
					total_bounds.y1 = vertex_point->y;
					total_bounds.y0 = total_bounds.y1;
					total_bounds.z1 = vertex_point->z;
					total_bounds.z0 = total_bounds.z1;
					total_bounds_valid = TRUE;
				}

				for (surface_queue_index = 0; adjacent_surface_index!=NONE && surface_queue_index<surface_queue_write_index; surface_queue_index++)
				{
					if (surface_queue[surface_queue_index]==adjacent_surface_index)
					{
						adjacent_surface_index = NONE;
					}
				}

				if (adjacent_surface_index!=NONE)
				{
					struct collision_surface const *adjacent = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, adjacent_surface_index, struct collision_surface);

					if (adjacent->breakable_surface_index==breakable_surface_index &&
						adjacent->material_index==collision_surface->material_index)
					{
						match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 388, surface_queue_write_index<MAXIMUM_BREAKABLE_SURFACE_QUEUE_SIZE);
						surface_queue[surface_queue_write_index++] = adjacent_surface_index;
					}
				}

				edge_index = collision_edge->edge_indices[reverse];
				++surface_vertex_index;
			}
			while (edge_index!=surface->first_edge_index);

			for (particle_index = 0; particle_index < breakable_surface->particle_effects.count; ++particle_index)
			{
				struct breakable_surface_particle_effect const *particle_effect = TAG_BLOCK_GET_ELEMENT(&breakable_surface->particle_effects, particle_index, struct breakable_surface_particle_effect);
				
				if (particle_effect->particle.index!=NONE)
				{
					rectangle2d bounds;
					short t_index;

					if (particle_effect->density!=0.0f)
					{
						bounds.x0 = fast_ftol(ceil(PIN(surface_bounds.x0 / particle_effect->density, -1000.f, 1000.f)));
						bounds.y0 = fast_ftol(ceil(PIN(surface_bounds.y0 / particle_effect->density, -1000.f, 1000.f)));
						bounds.x1 = fast_ftol(floor(PIN(surface_bounds.x1 / particle_effect->density, -1000.f, 1000.f)));
						bounds.y1 = fast_ftol(floor(PIN(surface_bounds.y1 / particle_effect->density, -1000.f, 1000.f)));
					}
					else
					{
						if (surface_index==seed_surface_index)
						{
							bounds.y1 = 0;
							bounds.x1 = 0;
							bounds.y0 = 0;
							bounds.x0 = 0;
						}
						else
						{
							bounds.y0 = 1;
							bounds.x0 = 1;
							bounds.y1 = 0;
							bounds.x1 = 0;
						}
					}

					for (t_index = bounds.y0; t_index<=bounds.y1; ++t_index)
					{
						short s_index;

						for (s_index = bounds.x0; s_index<=bounds.x1; ++s_index)
						{
							real_point2d position_2d_test;

							position = origin;

							{
								real_point2d jitter;
								jitter.x = real_local_random_range(-0.75f, 0.75f);
								jitter.y = real_local_random_range(-0.75f, 0.75f);

								point_from_line3d(&position, &s_plane.n, ((real)s_index + jitter.x) * particle_effect->density, &position);
								point_from_line3d(&position, &t_plane.n, ((real)t_index + jitter.y) * particle_effect->density, &position);
							}

							project_point3d(&position, projection_axis, projection_sign, &position_2d_test);

							if (convex_hull2d_test_point(surface_vertex_index, surface_vertices2d, &position_2d_test, 0.0f))
							{
								struct damage_breaking_effect_definition const *breaking_effect;
								real_vector3d outward_vector;
								real distance;
								struct new_particle_data particle;

								velocity = *global_zero_vector3d;
								breaking_effect = &damage_effect_definition_get(damage_data->definition_index)->breaking_effect;
								vector_from_points3d(&damage_data->epicenter, &position, &outward_vector);
								distance = normalize3d(&outward_vector);

								if (breaking_effect->outward_radius>0.0f)
								{
									real factor = PIN(1.0f - distance / breaking_effect->outward_radius, 0.0f, 1.0f);

									if (breaking_effect->outward_exponent != 0.0f)
									{
										factor = power(factor, breaking_effect->outward_exponent);
									}

									factor *= breaking_effect->outward_velocity;

									point_from_line3d((real_point3d *)&velocity, &outward_vector, factor, (real_point3d *)&velocity);
								}

								if (breaking_effect->forward_radius > 0.0f)
								{
									real factor = PIN(1.0f - distance / breaking_effect->forward_radius, 0.0f, 1.0f);

									if (breaking_effect->forward_exponent != 0.0f)
									{
										factor = power(factor, breaking_effect->forward_exponent);
									}

									factor *= breaking_effect->forward_velocity;

									point_from_line3d((real_point3d *)&velocity, &damage_data->direction, factor, (real_point3d *)&velocity);
								}

								if (particle_effect->velocity_scale_upper_bound > 0.0f)
								{
									real scale = real_local_random_range(particle_effect->velocity_scale_lower_bound, particle_effect->velocity_scale_upper_bound);
									scale_vector3d(&velocity, scale, &velocity);
								}

								particle.definition_index = particle_effect->particle.index;
								particle.object_index = NONE;
								particle.node_index = NONE;
								particle.local_player_index = NONE;
								particle.attached_to_local_player = FALSE;
								particle.dont_draw_first_person = FALSE;
								particle.dont_draw_third_person = FALSE;
								particle.position = position;
								particle.velocity = velocity;
								particle.direction = velocity;
								particle.initial_impulse = *global_zero_vector3d;
								particle.rotation = real_local_random_range(0.0f, 2*M_PI);
								particle.angular_velocity = real_local_random_range(particle_effect->angular_velocity_lower_bound, particle_effect->angular_velocity_upper_bound);
								particle.radius = real_local_random_range(particle_effect->radius_lower_bound, particle_effect->radius_upper_bound);

								{
									real u = real_local_random();

									rgb_colors_interpolate(
										&particle.color.rgb,
										particle_effect->flags & (
											FLAG(_breakable_surface_particle_effect_color_interpolate_in_hsv_bit) |
											FLAG(_breakable_surface_particle_effect_color_interpolate_along_farthest_hue_path_bit)
										),
										&particle_effect->tint_lower_bound.rgb,
										&particle_effect->tint_upper_bound.rgb,
										u
									);
								}

								particle.color.alpha = PIN(particle_effect->tint_lower_bound.alpha + (particle_effect->tint_upper_bound.alpha - particle_effect->tint_lower_bound.alpha) * real_local_random(), 0.0f, 1.0f);

								if (normalize3d(&particle.direction)==0.0f)
								{
									local_random_direction3d(&particle.direction);
								}

								particle_new(&particle);
							}
						}
					}
				}
			}
		}

		if (breakable_surface->sound.index != NONE && total_bounds_valid)
		{
			struct sound_location sound_location;

			set_real_point3d(&sound_location.position, (total_bounds.x0 + total_bounds.x1) * 0.5f, (total_bounds.y0 + total_bounds.y1) * 0.5f, (total_bounds.z0 + total_bounds.z1) * 0.5f);
			sound_location.forward = *global_forward3d;
			sound_location.translational_velocity = *global_zero_vector3d;
			sound_location.game_location = damage_data->location;
			unattached_impulse_sound_new(breakable_surface->sound.index, &sound_location, 1.0f);
		}
	}

	return;
}
