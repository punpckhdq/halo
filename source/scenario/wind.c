/*
WIND.C
*/

/* ---------- headers */

#include "cseries.h"
#include "scenario.h"
#include "wind_definitions.h"
#include "fog_definitions.h"
#include "structure_bsp_definitions.h"

/* ---------- constants */

enum
{
	WIND_VARIANCE_BOX_RESOLUTION = 64,
	WIND_VARIANCE_BOX_SIZE = 8,
	WIND_VARIANCE_BOX_SCALE = 8,
	NUMBER_OF_WIND_VARIANCE_BOX_CONTROL_POINTS = 8,
	WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT = 8
};

/* ---------- macros */

/* ---------- structures */

struct wind_state
{
	boolean valid;
	real velocity_variance;
	real_euler_angles2d angular_variance;
	real velocity;
	real_vector3d velocity3d;
};

/* ---------- prototypes */

static void wind_variance_initialize(void);
static void wind_variance_get(real_point3d const *position, real_vector3d *wind, real wind_local_variation_rate, real max_magnitude);

/* ---------- globals */

long global_environment_index = NONE;
struct
{
	boolean initialized;
	real_vector3d variance[3][WIND_VARIANCE_BOX_RESOLUTION];
	short count;
	struct wind_state wind_states[MAXIMUM_WEATHER_PALETTE_ENTRIES_PER_STRUCTURE];
	long time;
} wind_globals;

/* ---------- public code */

void wind_initialize_for_new_map(
	void)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	match_assert("c:\\halo\\SOURCE\\scenario\\wind.c", 65, !wind_globals.initialized);
	memset(&wind_globals, 0, sizeof(wind_globals));
	wind_globals.initialized = TRUE;
	wind_variance_initialize();

	return;
}

void wind_dispose_from_old_map(
	void)
{
	wind_globals.initialized = FALSE;

	return;
}

void wind_update(
	void)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short i;

	match_assert("c:\\halo\\SOURCE\\scenario\\wind.c", 89, wind_globals.initialized);
	wind_globals.time++;
	for (i = 0; i < structure_bsp->weather_palette.count; i++)
	{
		struct structure_weather_palette_entry *weather = TAG_BLOCK_GET_ELEMENT(&structure_bsp->weather_palette, i, struct structure_weather_palette_entry);
		struct wind_state *state = &wind_globals.wind_states[i];

		if (weather->wind.index != NONE)
		{
			struct wind_definition *wind = tag_get(WIND_DEFINITION_TAG, weather->wind.index);
			real_euler_angles2d direction;

			state->velocity_variance += local_random_range(0, 2) ? 0.01f : -0.01f;
			state->velocity_variance = PIN(state->velocity_variance, 0.f, 1.f);
			state->angular_variance.pitch += local_random_range(0, 2) ? 0.01f : -0.01f;
			state->angular_variance.pitch = PIN(state->angular_variance.pitch, -1.f, 1.f);
			state->angular_variance.yaw += local_random_range(0, 2) ? 0.01f : -0.01f;
			state->angular_variance.yaw = PIN(state->angular_variance.yaw, -1.f, 1.f);
			state->velocity = wind->velocity_lower_bound + (wind->velocity_upper_bound - wind->velocity_lower_bound) * state->velocity_variance;
			euler_angles2d_from_vector3d(&direction, &weather->wind_direction);
			direction.pitch += wind->variation_area.pitch * state->angular_variance.pitch * 0.5f;
			direction.yaw += wind->variation_area.yaw * state->angular_variance.yaw * 0.5f;
			vector3d_from_euler_angles2d(&state->velocity3d, &direction);
			scale_vector3d(&state->velocity3d, weather->wind_magnitude * state->velocity, &state->velocity3d);
			state->valid = TRUE;
		}
		else
		{
			state->valid = FALSE;
		}
	}
	wind_globals.count = structure_bsp->weather_palette.count;

	return;
}

void scenario_get_wind(
	struct location const *location,
	real_point3d const *position,
	real_vector3d *wind_vector,
	unsigned long flags)
{
	scenario_get_current(location, position, wind_vector, flags | FLAG(_scenario_current_force_no_water_bit));

	return;
}

void scenario_get_water_current(
	struct location const *location,
	real_point3d const *position,
	real_vector3d *wind_vector,
	unsigned long flags)
{
	scenario_get_current(location, position, wind_vector, flags | FLAG(_scenario_current_force_water_bit));

	return;
}

boolean scenario_get_current(
	struct location const *location,
	real_point3d const *position,
	real_vector3d *wind_vector,
	unsigned long flags)
{
	boolean result = FALSE;
	short weather_palette_index = NONE;

	if (location->cluster_index != NONE)
	{
		struct structure_bsp *structure_bsp = global_structure_bsp_get();
		short fog_region_index = scenario_get_fog_region_index(location, TEST_FLAG(flags, _scenario_current_force_water_bit) ? NULL : position);
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, location->cluster_index, struct structure_cluster);

		weather_palette_index = cluster->weather_palette_index;
		if (fog_region_index != NONE)
		{
			struct structure_fog_region *region = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_regions, fog_region_index, struct structure_fog_region);

			if (region->fog_palette_index != NONE && region->weather_palette_index != NONE)
			{
				struct structure_fog_palette_entry *fog = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_palette, region->fog_palette_index, struct structure_fog_palette_entry);

				if (fog->fog.index != NONE)
				{
					struct fog_definition *definition = fog_definition_get(fog->fog.index);

					if (TEST_FLAG(definition->flags, _fog_definition_is_water_bit))
					{
						if (!TEST_FLAG(flags, _scenario_current_force_no_water_bit))
						{
							weather_palette_index = region->weather_palette_index;
							result = TRUE;
						}
					}
					else if (!TEST_FLAG(flags, _scenario_current_force_water_bit))
					{
						weather_palette_index = region->weather_palette_index;
					}
				}
			}
		}
	}
	scenario_get_current_from_weather_palette(position, wind_vector, flags, weather_palette_index);

	return result;
}

void scenario_get_current_from_weather_palette(
	real_point3d const *position,
	real_vector3d *current_vector,
	unsigned long flags,
	short weather_palette_index)
{
	if (weather_palette_index >= 0 && weather_palette_index < wind_globals.count)
	{
		struct wind_state *state = &wind_globals.wind_states[weather_palette_index];

		if (state->valid)
		{
			struct structure_weather_palette_entry *weather = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->weather_palette, weather_palette_index, struct structure_weather_palette_entry);
			struct wind_definition *wind = tag_get(WIND_DEFINITION_TAG, weather->wind.index);
			real local_variation_weight = TEST_FLAG(flags, _scenario_current_simple_bit) ? 0.f : wind->local_variation_weight;
			real_vector3d wind_variance;

			wind_variance_get(position, &wind_variance, wind->local_variation_rate, wind->local_variation_weight * state->velocity);
			current_vector->i = (1.f - local_variation_weight) * state->velocity3d.i + wind_variance.i;
			current_vector->j = (1.f - local_variation_weight) * state->velocity3d.j + wind_variance.j;
			current_vector->k = (1.f - local_variation_weight) * state->velocity3d.k + wind_variance.k;
			if (TEST_FLAG(flags, _scenario_current_damped_bit))
			{
				current_vector->i *= 1.f - wind->damping;
				current_vector->j *= 1.f - wind->damping;
				current_vector->k *= 1.f - wind->damping;
			}
		}
		else
		{
			*current_vector = *global_zero_vector3d;
		}
	}
	else
	{
		*current_vector = *global_zero_vector3d;
	}

	return;
}

/* ---------- private code */

static void wind_variance_initialize(
	void)
{
	short i, j, k;
	real const h = 1.f;
	real const sample_step = h / WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT;

	for (i = 0; i < NUMBER_OF_WIND_VARIANCE_BOX_CONTROL_POINTS; i++)
	{
		for (j = 0; j < 3; j++)
		{
			random_direction3d(&wind_globals.variance[j][i * WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT]);
		}
	}
	for (i = 0; i < NUMBER_OF_WIND_VARIANCE_BOX_CONTROL_POINTS; i++)
	{
		for (k = 1; k < WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT; k++)
		{
			for (j = 0; j < 3; j++)
			{
				real t = i * h + k * sample_step;
				word control_points[4];

				control_points[0] = (i - 1) & (NUMBER_OF_WIND_VARIANCE_BOX_CONTROL_POINTS - 1);
				control_points[1] = i;
				control_points[2] = (i + 1) & (NUMBER_OF_WIND_VARIANCE_BOX_CONTROL_POINTS - 1);
				control_points[3] = (i + 2) & (NUMBER_OF_WIND_VARIANCE_BOX_CONTROL_POINTS - 1);
				uniform_cubic_spline_vector3d(
					&wind_globals.variance[j][i * WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT + k],
					&wind_globals.variance[j][control_points[0] * WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT],
					&wind_globals.variance[j][control_points[1] * WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT],
					&wind_globals.variance[j][control_points[2] * WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT],
					&wind_globals.variance[j][control_points[3] * WIND_VARIANCE_RESOLUTION_PER_CONTROL_POINT],
					(control_points[1] - 1) * h,
					h,
					t);
			}
		}
	}

	return;
}

static void wind_variance_get(
	real_point3d const *position,
	real_vector3d *wind,
	real wind_local_variation_rate,
	real max_magnitude)
{
	real const slope[3] = {0.1f, 0.2f, 0.07f};
	real scale = max_magnitude / 3.f;
	short i;

	*wind = *global_zero_vector3d;
	for (i = 0; i < 3; i++)
	{
		real time_adjusted_position = (wind_globals.time * slope[i] * wind_local_variation_rate + position->n[i]) * WIND_VARIANCE_BOX_SCALE;
		short index;

		*(long *)&time_adjusted_position &= LONG_MAX;
		time_adjusted_position += 8388608.f;
		index = *(long *)&time_adjusted_position & (WIND_VARIANCE_BOX_RESOLUTION - 1);
		wind->i += wind_globals.variance[i][index].i;
		wind->j += wind_globals.variance[i][index].j;
		wind->k += wind_globals.variance[i][index].k;
	}
	scale_vector3d(wind, scale, wind);

	return;
}
