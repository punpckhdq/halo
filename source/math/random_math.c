/*
RANDOM_MATH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"
#include "geometry.h"
#include "game_engine.h"

/* ---------- constants */

enum
{
	RANDOM_DIRECTION_TABLE_GEOSPHERE_SEGMENT_COUNT = 16,
};

enum
{
	RANDOM_A = 1664525,
	RANDOM_C = 1013904223,
};

/* ---------- macros */


/* ---------- structures */

/* ---------- prototypes */

static __inline real_vector3d *direction3d_from_table(real_vector3d *direction, short index);

/* ---------- globals */

static struct
{
	real_vector3d *random_direction_table;
	short random_direction_table_size;
} random_math_globals;

static long random_seed_lock_count;
static unsigned long global_random_seed;
static unsigned long global_local_random_seed;

/* ---------- public code */

void lock_global_random_seed(
	void)
{
	random_seed_lock_count++;

	return;
}

void unlock_global_random_seed(
	void)
{
	match_vassert("c:\\halo\\SOURCE\\math\\random_math.c", 41, random_seed_lock_count>0, "unmatched call to unlock_random_seed() somewhere");
	random_seed_lock_count--;

	return;
}

unsigned long *get_global_random_seed_address(
	void)
{
	if (game_engine_running())
	{
		match_vassert("c:\\halo\\SOURCE\\math\\random_math.c", 56, !random_seed_lock_count, "you should not be using global random(); use local random() instead");
	}

	return &global_random_seed;
}

unsigned long get_random_seed(
	void)
{
	return global_random_seed;
}

unsigned long *get_global_local_random_seed_address(
	void)
{
	return &global_local_random_seed;
}

void random_seed_debug_log(
	boolean enable)
{
	return;
}

long get_number_suitable_for_initializing_random_seed(
	void)
{
	long initial_seed_value = system_seconds() ^ system_milliseconds() ^ rand();

	return initial_seed_value;
}

void random_math_initialize(
	void)
{
	struct geosphere *random_direction_geosphere;
	short vertex_index;

	*get_global_local_random_seed_address() = get_number_suitable_for_initializing_random_seed();

	random_direction_geosphere = geosphere_new(RANDOM_DIRECTION_TABLE_GEOSPHERE_SEGMENT_COUNT);
	match_assert("c:\\halo\\SOURCE\\math\\random_math.c", 174, random_direction_geosphere);

	random_math_globals.random_direction_table = match_malloc("c:\\halo\\SOURCE\\math\\random_math.c", 176, random_direction_geosphere->vertex_count*sizeof(real_vector3d));
	random_math_globals.random_direction_table_size = random_direction_geosphere->vertex_count;
	for (vertex_index = 0; vertex_index<random_direction_geosphere->vertex_count; vertex_index++)
	{
		real_vector3d *direction = &random_math_globals.random_direction_table[vertex_index];

		*direction = *(real_vector3d *)&random_direction_geosphere->vertices[vertex_index];
	}

	geosphere_dispose(random_direction_geosphere);

	return;
}

void random_math_dispose(
	void)
{
	match_free("c:\\halo\\SOURCE\\math\\random_math.c", 200, random_math_globals.random_direction_table);

	return;
}

real real_seed_random(
	unsigned long *seed)
{
	*seed = RANDOM_A*(*seed) + RANDOM_C;

	return (real)(*seed>>16)*(1.f/UNSIGNED_SHORT_MAX);
}

real real_seed_random_range(
	unsigned long *seed,
	real lower_bound,
	real upper_bound)
{
	*seed = RANDOM_A*(*seed) + RANDOM_C;

	return ((real)(*seed>>16)*(1.f/UNSIGNED_SHORT_MAX))*(upper_bound-lower_bound) + lower_bound;
}

word seed_random(
	unsigned long *seed)
{
	*seed = RANDOM_A*(*seed) + RANDOM_C;

	return (word)(*seed>>16);
}

short seed_random_range(
	unsigned long *seed,
	short lower_bound,
	short upper_bound)
{
	*seed = RANDOM_A*(*seed) + RANDOM_C;

	return (short)(lower_bound + ((unsigned long)((upper_bound-lower_bound)*(word)(*seed>>16))>>16));
}

static __inline real_vector3d *direction3d_from_table(
	real_vector3d *direction,
	short index)
{
	real_vector3d *table_direction = &random_math_globals.random_direction_table[index];

	match_assert("c:\\halo\\SOURCE\\math\\random_math.c", 250, random_math_globals.random_direction_table);
	match_assert("c:\\halo\\SOURCE\\math\\random_math.c", 251, index>=0 && index<random_math_globals.random_direction_table_size);

	*direction = *table_direction;

	return direction;
}

real_vector3d *seed_random_direction3d(
	unsigned long *seed,
	real_vector3d *direction)
{
	return direction3d_from_table(direction, seed_random_range(seed, 0, random_math_globals.random_direction_table_size));
}

void seed_random_orientation(
	unsigned long *seed,
	real_vector3d *forward,
	real_vector3d *up)
{
	real yaw;
	real pitch;
	real roll;
	real cosine_yaw;
	real sine_yaw;
	real cosine_pitch;
	real sine_pitch;

	*seed = RANDOM_A*(*seed) + RANDOM_C;
	yaw = ((real)(*seed>>16)*(1.f/UNSIGNED_SHORT_MAX))*(2.f*_pi);
	*seed = RANDOM_A*(*seed) + RANDOM_C;
	pitch = ((real)(*seed>>16)*(1.f/UNSIGNED_SHORT_MAX))*_pi - _pi/2.f;
	*seed = RANDOM_A*(*seed) + RANDOM_C;
	roll = ((real)(*seed>>16)*(1.f/UNSIGNED_SHORT_MAX))*(2.f*_pi);
	cosine_yaw = cosine(yaw);
	sine_yaw = sine(yaw);
	cosine_pitch = cosine(pitch);
	sine_pitch = sine(pitch);

	forward->i = cosine_pitch*cosine_yaw;
	forward->j = cosine_pitch*sine_yaw;
	forward->k = sine_pitch;
	up->i = -sine_pitch*cosine_yaw;
	up->j = -sine_pitch*sine_yaw;
	up->k = cosine_pitch;
	yaw_vectors(up, forward, sine(roll), cosine(roll));

	return;
}

real_vector3d *seed_random_vector_in_cone3d(
	unsigned long *seed,
	real_vector3d const *axis,
	real inner_cone_angle,
	real outer_cone_angle,
	real_vector3d *result)
{
	real_vector3d random_vector;
	real_vector3d random_axis;

	*result = *axis;
	seed_random_direction3d(seed, &random_vector);
	cross_product3d(axis, &random_vector, &random_axis);
	if (normalize3d(&random_axis)>0.f)
	{
		real error_angle = real_seed_random_range(seed, inner_cone_angle, outer_cone_angle);

		rotate_vector_about_axis(result, &random_axis, sine(error_angle), cosine(error_angle));
	}

	return result;
}

/* ---------- private code */
