/*
BREAKABLE_SURFACES.C

*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "real_math.h"
#include "tag_groups.h"
#include "breakable_surfaces.h"
#include "collisions.h"
#include "collision_bsp_definitions.h"
#include "collision_model_definitions.h"
#include "damage.h"
#include "bitmaps.h"
#include "material_effect_definitions.h"
#include "particles.h"
#include "game_globals.h"
#include "structure_bsp_definitions.h"
#include "damage_effect_definitions.h"
#include "game_state.h"
#include "scenario.h"
#include "game_sound.h"

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

struct breakable_surface_globals *globals;

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

	for (bsp_index = 0; bsp_index < MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO; ++bsp_index)
	{
		short surface_index;
		struct breakable_surface_datum *surface;

		csmemset(globals->breakable_surface_flags[bsp_index], NONE, sizeof(globals->breakable_surface_flags[bsp_index]));

		surface = globals->breakable_surfaces[bsp_index];
		
		for (surface_index = 0; surface_index < MAXIMUM_BREAKABLE_SURFACES_PER_MAP; surface_index++)
		{
			surface[surface_index].vitality = 1.0f;
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

	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 147,
		breakable_surface_index==NONE || (breakable_surface_index>=0 && breakable_surface_index<MAXIMUM_BREAKABLE_SURFACES_PER_MAP));

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
	struct structure_bsp *structure_bsp;
	struct damage_effect_definition *damage_effect_def;
	struct structure_breakable_surface *breakable_surface;
	real_vector3d area;
	real cutoff_radius;
	real radius;
	short i;

	structure_bsp = global_structure_bsp_get();
	damage_effect_def = damage_effect_definition_get(damage_data->definition_index);

	if (globals->enabled
		&& (damage_effect_def->damage.damage_lower_bound != 0.0f || damage_effect_def->damage.damage_upper_bound != 0.0f))
	{
		cutoff_radius = damage_effect_def->cutoff_radius;

		if (cutoff_radius > 4.0f)
		{
			error(2, "WARNING: area of effect breakable surface damage with radius %d", cutoff_radius);
		}

		for (i = 0; i < structure_bsp->breakable_surfaces.count; i++)
		{
			if (breakable_surface_extant(i))
			{
				breakable_surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->breakable_surfaces, i, struct structure_breakable_surface);
				radius = breakable_surface->bounding_radius + cutoff_radius;

				vector_from_points3d(&breakable_surface->centroid, &damage_data->epicenter, &area);

				if (magnitude_squared3d(&area) <= radius * radius)
				{
					breakable_surface_get(i)->vitality = 0.0f;
					BIT_VECTOR_SET_FLAG((long *)breakable_surface_flags_get(), i, FALSE);
					breakable_surface_effect(i, damage_data, breakable_surface->collision_surface_index);
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
	real_plane3d *plane;
	real_rectangle2d surface_bounds;
	real_plane3d t_plane;
	real_point3d surface_vertices3d[MAXIMUM_VERTICES_PER_COLLISION_SURFACE];
	real_point2d surface_vertices2d[MAXIMUM_VERTICES_PER_COLLISION_SURFACE];
	real_vector3d t_normal;
	real_vector3d s_normal;
	real_point3d position;
	real_point2d position_2d_test;
	real_vector3d velocity;
	struct new_particle_data particle;
	struct sound_location sound_location;
	struct collision_surface const *surface;
	struct collision_edge const *collision_edge;
	struct collision_vertex const *vertex;
	struct collision_vertex const *next_vertex;
	struct collision_surface const *adjacent;
	real_point2d projected_epicenter;
	long edge_index;
	long adjacent_surface_index;
	boolean reverse;
	short projection_axis;
	boolean projection_sign;
	short surface_vertex_index;
	short i;
	real s, t;
	real ax, ay, az;
	long material_type;
	struct breakable_surface_particle_effect const *contrail;
	struct damage_breaking_effect_definition const *breaking_effect;
	real_vector3d outward;
	real distance, factor, scale, s_offset, t_offset;
	short contrail_index;
	short s_min, s_max, t_min, t_max;
	long s_index, t_index;
	struct material_definition *material_def;

	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct collision_bsp *collision_bsp = global_collision_bsp_get();

	match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 251, damage_data);

	if (breakable_surface_effect_enabled)
	{
		collision_surface = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, seed_surface_index, struct collision_surface);
		material_type = TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_materials, collision_surface->material_index, struct structure_collision_material)->runtime_physics_material_type;
		material_def = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->materials, material_type, struct material_definition);
		breakable_surface = &material_def->breakable_surface;

		total_bounds_valid = FALSE;
		surface_queue_read_index = 0;

		match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 266, collision_surface->breakable_surface_index==breakable_surface_index);

		surface_queue[0] = seed_surface_index;
		surface_queue_write_index = 1;

		while (TRUE)
		{
			surface_index = surface_queue[surface_queue_read_index++];
			surface = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, surface_index, struct collision_surface);
			plane = TAG_BLOCK_GET_ELEMENT(&collision_bsp->bsp3d.planes, surface->plane_designator & 0x7FFFFFFF, real_plane3d);

			if (surface->plane_designator >= 0)
			{
				surface_plane = *plane;
			}
			else
			{
				surface_plane.n.i = -plane->n.i;
				surface_plane.n.j = -plane->n.j;
				surface_plane.n.k = -plane->n.k;
				surface_plane.d = -plane->d;
			}

			ax = fabs(surface_plane.n.i);
			ay = fabs(surface_plane.n.j);
			az = fabs(surface_plane.n.k);

			if (ax >= ay && ax >= az)
			{
				projection_axis = 0;
			}
			else if (ay >= az)
			{
				projection_axis = 1;
			}
			else
			{
				projection_axis = 2;
			}

			projection_sign = (surface_plane.n.n[projection_axis] > 0.0f);

			surface_vertex_index = 0;
			edge_index = surface->first_edge_index;
			do
			{
				collision_edge = TAG_BLOCK_GET_ELEMENT(&collision_bsp->edges, edge_index, struct collision_edge);
				reverse = (collision_edge->surface_indices[1] == surface_index);
				vertex = TAG_BLOCK_GET_ELEMENT(&collision_bsp->vertices, collision_edge->vertex_indices[!reverse], struct collision_vertex);
				adjacent_surface_index = collision_edge->surface_indices[!reverse];

				if (surface_vertex_index == 0)
				{
					next_vertex = TAG_BLOCK_GET_ELEMENT(&collision_bsp->vertices, collision_edge->vertex_indices[reverse], struct collision_vertex);

					if (surface_index == seed_surface_index)
					{
						project_point3d(&damage_data->epicenter, projection_axis, projection_sign, &projected_epicenter);
						project_point2d(&projected_epicenter, &surface_plane, projection_axis, projection_sign, &origin);
					}
					else
					{
						origin = vertex->point;
					}

					vector_from_points3d(&vertex->point, &next_vertex->point, &s_normal);
					normalize3d(&s_normal);

					cross_product3d(&s_normal, &surface_plane.n, &t_normal);

					plane3d_from_point_and_normal(&s_plane, &origin, &s_normal);
					plane3d_from_point_and_normal(&t_plane, &origin, &t_normal);

					surface_bounds.x1 = dot_product3d((real_vector3d *)&vertex->point, &s_normal) - s_plane.d;
					surface_bounds.x0 = surface_bounds.x1;
					surface_bounds.y1 = dot_product3d((real_vector3d *)&vertex->point, &t_normal) - t_plane.d;
					surface_bounds.y0 = surface_bounds.y1;
				}
				else
				{
					s = dot_product3d((real_vector3d *)&vertex->point, &s_plane.n) - s_plane.d;
					t = dot_product3d((real_vector3d *)&vertex->point, &t_plane.n) - t_plane.d;
					if (s <= surface_bounds.x0) surface_bounds.x0 = s;
					if (t <= surface_bounds.y0) surface_bounds.y0 = t;
					if (s >  surface_bounds.x1) surface_bounds.x1 = s;
					if (t >  surface_bounds.y1) surface_bounds.y1 = t;

					match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 348, surface_vertex_index < MAXIMUM_VERTICES_PER_COLLISION_SURFACE);
				}

				surface_vertices3d[surface_vertex_index] = vertex->point;
				project_point3d(&vertex->point, projection_axis, projection_sign, &surface_vertices2d[surface_vertex_index]);

				if (total_bounds_valid)
				{
					if (vertex->point.x <= total_bounds.x0) total_bounds.x0 = vertex->point.x;
					if (vertex->point.y <= total_bounds.y0) total_bounds.y0 = vertex->point.y;
					if (vertex->point.z <= total_bounds.z0) total_bounds.z0 = vertex->point.z;
					if (vertex->point.x >  total_bounds.x1) total_bounds.x1 = vertex->point.x;
					if (vertex->point.y >  total_bounds.y1) total_bounds.y1 = vertex->point.y;
					if (vertex->point.z >  total_bounds.z1) total_bounds.z1 = vertex->point.z;
				}
				else
				{
					total_bounds.x0 = total_bounds.x1 = vertex->point.x;
					total_bounds.y0 = total_bounds.y1 = vertex->point.y;
					total_bounds.z0 = total_bounds.z1 = vertex->point.z;
					total_bounds_valid = TRUE;
				}

				if (adjacent_surface_index != NONE)
				{
					for (i = 0; i < surface_queue_write_index; i++)
					{
						if (surface_queue[i] == adjacent_surface_index)
						{
							adjacent_surface_index = NONE;
							break;
						}
					}
					if (adjacent_surface_index != NONE)
					{
						adjacent = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, adjacent_surface_index, struct collision_surface);
						if (adjacent->breakable_surface_index == breakable_surface_index
							&& adjacent->material_index == collision_surface->material_index)
						{
							match_assert("c:\\halo\\SOURCE\\physics\\breakable_surfaces.c", 388, surface_queue_write_index < MAXIMUM_BREAKABLE_SURFACE_QUEUE_SIZE);
							surface_queue[surface_queue_write_index++] = adjacent_surface_index;
						}
					}
				}

				edge_index = collision_edge->edge_indices[reverse];
				{
					surface_vertex_index++;
				}
			}
			while (edge_index != surface->first_edge_index);

			for (contrail_index = 0; contrail_index < breakable_surface->particle_effects.count; contrail_index++)
			{
				contrail = TAG_BLOCK_GET_ELEMENT(&breakable_surface->particle_effects, contrail_index, struct breakable_surface_particle_effect);
				if (contrail->particle.index == NONE)
				{
					continue;
				}

				if (contrail->density > 0.0f)
				{
					s_min = (short)ceil(PIN(surface_bounds.x0 / contrail->density, -1000.0f, 1000.0f));
					t_min = (short)ceil(PIN(surface_bounds.y0 / contrail->density, -1000.0f, 1000.0f));
					s_max = (short)floor(PIN(surface_bounds.x1 / contrail->density, -1000.0f, 1000.0f));
					t_max = (short)floor(PIN(surface_bounds.y1 / contrail->density, -1000.0f, 1000.0f));
				}
				else
				{
					if (surface_index != seed_surface_index)
					{
						continue;
					}
					s_min = s_max = t_min = t_max = 0;
				}

				for (t_index = t_min; t_index <= t_max; t_index++)
				{
					for (s_index = s_min; s_index <= s_max; s_index++)
					{
						s_offset = ((real)s_index + real_random_range(-0.75f, 0.75f)) * contrail->density;
						t_offset = ((real)t_index + real_random_range(-0.75f, 0.75f)) * contrail->density;

						position = origin;

						point_from_line3d(&position, &s_plane.n, s_offset, &position);
						point_from_line3d(&position, &t_plane.n, t_offset, &position);

						project_point3d(&position, projection_axis, projection_sign, &position_2d_test);

						if (!convex_hull2d_test_point(surface_vertex_index, surface_vertices2d, &position_2d_test, FALSE))
						{
							continue;
						}

						velocity = *global_zero_vector3d;

						vector_from_points3d(&position, &damage_data->epicenter, &outward);
						distance = normalize3d(&outward);

						breaking_effect = &damage_effect_definition_get(damage_data->definition_index)->breaking_effect;

						if (breaking_effect->outward_radius > 0.0f)
						{
							factor = PIN(1.0f - distance / breaking_effect->outward_radius, 0.0f, 1.0f);

							if (breaking_effect->outward_exponent != 0.0f)
							{
								factor = (real)pow(factor, breaking_effect->outward_exponent);
							}

							factor *= breaking_effect->outward_velocity;

							point_from_line3d((real_point3d *)&velocity, &outward, factor, (real_point3d *)&velocity);
						}
						if (breaking_effect->forward_radius > 0.0f)
						{
							factor = PIN(1.0f - distance / breaking_effect->forward_radius, 0.0f, 1.0f);

							if (breaking_effect->forward_exponent != 0.0f)
							{
								factor = (real)pow(factor, breaking_effect->forward_exponent);
							}

							factor *= breaking_effect->forward_velocity;

							point_from_line3d((real_point3d *)&velocity, &damage_data->direction, factor, (real_point3d *)&velocity);
						}
						if (contrail->velocity_scale_upper_bound > 0.0f)
						{
							scale = real_random_range(contrail->velocity_scale_lower_bound, contrail->velocity_scale_upper_bound);
							scale_vector3d(&velocity, scale, &velocity);
						}

						particle.definition_index = contrail->particle.index;
						particle.object_index = NONE;
						particle.node_index = NONE;
						particle.local_player_index = NONE;
						particle.attached_to_local_player = FALSE;
						particle.dont_draw_first_person = FALSE;
						particle.dont_draw_third_person = FALSE;
						particle.position = position;
						particle.direction = velocity;
						particle.velocity = velocity;
						particle.initial_impulse = *global_zero_vector3d;
						particle.rotation = real_random_range(0.0f, M_PI * 2.0);
						particle.angular_velocity = real_random_range(contrail->angular_velocity_lower_bound, contrail->angular_velocity_upper_bound);
						particle.radius = real_random_range(contrail->radius_lower_bound, contrail->radius_upper_bound);
						rgb_colors_interpolate(&particle.color.rgb, contrail->flags & 3, &contrail->tint_lower_bound.rgb, &contrail->tint_upper_bound.rgb, real_random());
						particle.color.alpha = PIN(contrail->tint_lower_bound.alpha + real_random() * (contrail->tint_upper_bound.alpha - contrail->tint_lower_bound.alpha), 0.0f, 1.0f);

						if (normalize3d(&particle.direction) == 0.0f)
						{
							seed_random_direction3d(get_global_local_random_seed_address(), &particle.direction);
						}

						particle_new(&particle);
					}
				}
			}

			if (surface_queue_read_index >= surface_queue_write_index)
			{
				break;
			}
		}

		if (breakable_surface->sound.index != NONE && total_bounds_valid)
		{
			sound_location.position.x = (total_bounds.x0 + total_bounds.x1) * 0.5f;
			sound_location.position.y = (total_bounds.y0 + total_bounds.y1) * 0.5f;
			sound_location.position.z = (total_bounds.z0 + total_bounds.z1) * 0.5f;
			sound_location.forward = *global_forward3d;
			sound_location.translational_velocity = *global_zero_vector3d;
			sound_location.game_location = damage_data->location;
			unattached_impulse_sound_new(breakable_surface->sound.index, &sound_location, 1.0f);
		}
	}

	return;
}
