/*
REAL_MATH.H

file has inline function assertions.
*/

#ifndef __REAL_MATH_H
#define __REAL_MATH_H
#pragma once

#include <float.h>

/* ---------- constants */

#define _real_epsilon 0.0001f
#define _pi ((real)M_PI)

#define REAL_MIN -3.4028235e38f
#define REAL_MAX 3.4028235e38f
#define DOUBLE_MIN 3.62314807E-315

/* ---------- macros */

#define DEGREES_TO_RADIANS(angle) ((real)(((real)angle) * _pi / 180.f))
#define RADIANS_TO_DEGREES(angle) ((real)(((real)angle) * 180.f / _pi))

#define assert_valid_real(v)				\
vassert(									\
	valid_real(v),							\
	csprintf(								\
		temporary,							\
		"%s: assert_valid_real(0x%08X %f)",	\
		#v,									\
		*((long*)&v), v						\
	)										\
)

#define assert_valid_real_point3d(point)\
vassert(												\
	valid_real_point3d(point),							\
	csprintf(											\
		temporary,										\
		"%s: assert_valid_real_point3d(%f, %f, %f)",	\
		#point, (*point).x, (*point).y, (*point).z		\
	)													\
)

#define assert_valid_real_vector3d(vector)				\
vassert(												\
	valid_real_vector3d(vector),						\
	csprintf(											\
		temporary,										\
		"%s: assert_valid_real_vector2d(%f, %f, %f)",	\
		#vector, (*vector).i, (*vector).j, (*vector).k	\
	)													\
)

#define assert_valid_real_normal3d(vector)				\
vassert(												\
	valid_real_normal3d(vector),						\
	csprintf(											\
		temporary,										\
		"%s: assert_valid_real_normal3d(%f, %f, %f)",	\
		#vector, (*vector).i, (*vector).j, (*vector).k	\
	)													\
)

#define assert_valid_real_vector3d_axes2(forward, up)						\
vassert(																	\
	valid_real_vector3d_axes2(forward, up),									\
	csprintf(																\
		temporary,															\
		"%s, %s: assert_valid_real_vector3d_axes2(%f, %f, %f / %f, %f, %f)",\
		#forward, #up,														\
		(*forward).i, (*forward).j, (*forward).k, (*up).i, (*up).j, (*up).k	\
	)																		\
)

#define assert_valid_real_matrix4x3_internal(matrix, string)				\
vassert(valid_real((*matrix).scale), csprintf(temporary, "%s had a bad scale %f", string, (*matrix).scale));	\
vassert(valid_real_normal3d(&(*matrix).forward), csprintf(temporary, "%s had a bad forward (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k));		\
vassert(valid_real_normal3d(&(*matrix).left), csprintf(temporary, "%s had a bad left (%f,%f,%f)", string, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k));						\
vassert(valid_real_normal3d(&(*matrix).up), csprintf(temporary, "%s had a bad up (%f,%f,%f)", string, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k));								\
vassert(valid_real_point3d(&(*matrix).position), csprintf(temporary, "%s had a bad position (%f,%f,%f)", string, (*matrix).position.x, (*matrix).position.y, (*matrix).position.z));	\
match_vassert(																\
	valid_realcmp(dot_product3d(&(*matrix).forward, &(*matrix).left), 0.f),	\
	csprintf(																\
		temporary,															\
		"%s had a forward (%f,%f,%f) not perpendicular to left (%f,%f,%f)",	\
		string,																\
		(*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k,		\
		(*matrix).left.i, (*matrix).left.j, (*matrix).left.k));				\
vassert(																	\
	valid_realcmp(dot_product3d(&(*matrix).up, &(*matrix).left), 0.f),		\
	csprintf(																\
		temporary,															\
		"%s had a up (%f,%f,%f) not perpendicular to left (%f,%f,%f)",		\
		string,																\
		(*matrix).up.i, (*matrix).up.j, (*matrix).up.k,						\
		(*matrix).left.i, (*matrix).left.j, (*matrix).left.k));				\
vassert(																	\
	valid_realcmp(dot_product3d(&(*matrix).forward, &(*matrix).up), 0.f),	\
	csprintf(																\
		temporary,															\
		"%s had a forward (%f,%f,%f) not perpendicular to up (%f,%f,%f)",	\
		string,																\
		(*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k,		\
		(*matrix).up.i, (*matrix).up.j, (*matrix).up.k));					\
vassert(valid_real_matrix4x3(matrix), csprintf(temporary, "%s: assert_valid_real_matrix4x3", string));	\

#define assert_valid_real_matrix4x3(file, line, matrix)								\
if (!valid_real_matrix4x3(matrix))													\
{																					\
	assert_valid_real_matrix4x3_internal(file, line, matrix, STRINGIFY(matrix));	\
}																					\

#define assert_valid_real_matrix4x3_custom_string(matrix, custom_string)	\
if (!valid_real_matrix4x3(matrix))											\
{																			\
	char* string = custom_string;											\
	assert_valid_real_matrix4x3_internal(file, line, matrix, string);		\
}																			\

#define match_assert_valid_real(file, line, v)	\
match_vassert(									\
	file,										\
	line,										\
	valid_real(v),								\
	csprintf(									\
		temporary,								\
		"%s: assert_valid_real(0x%08X %f)",		\
		#v,										\
		*((long*)&v), v							\
	)											\
)

#define match_assert_valid_real_point3d(file, line, point)	\
match_vassert(												\
	file,													\
	line,													\
	valid_real_point3d(point),								\
	csprintf(												\
		temporary,											\
		"%s: assert_valid_real_point3d(%f, %f, %f)",		\
		#point, (*point).x, (*point).y, (*point).z			\
	)														\
)

#define match_assert_valid_real_vector3d(file, line, vector)	\
match_vassert(													\
	file,														\
	line,														\
	valid_real_vector3d(vector),								\
	csprintf(													\
		temporary,												\
		"%s: assert_valid_real_vector2d(%f, %f, %f)",			\
		#vector, (*vector).i, (*vector).j, (*vector).k			\
	)															\
)

#define match_assert_valid_real_normal3d(file, line, vector)	\
match_vassert(													\
	file,														\
	line,														\
	valid_real_normal3d(vector),								\
	csprintf(													\
		temporary,												\
		"%s: assert_valid_real_normal3d(%f, %f, %f)",			\
		#vector, (*vector).i, (*vector).j, (*vector).k			\
	)															\
)

#define match_assert_valid_real_vector3d_axes2(file, line, forward, up)		\
match_vassert(																\
	file,																\
	line,																	\
	valid_real_vector3d_axes2(forward, up),									\
	csprintf(																\
		temporary,															\
		"%s, %s: assert_valid_real_vector3d_axes2(%f, %f, %f / %f, %f, %f)",\
		#forward, #up,														\
		(*forward).i, (*forward).j, (*forward).k, (*up).i, (*up).j, (*up).k	\
	)																		\
)

#define match_assert_valid_real_matrix4x3_internal(file, line, matrix, string)	\
match_vassert(file, line, valid_real((*matrix).scale), csprintf(temporary, "%s had a bad scale %f", string, (*matrix).scale));	\
match_vassert(file, line, valid_real_normal3d(&(*matrix).forward), csprintf(temporary, "%s had a bad forward (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k));		\
match_vassert(file, line, valid_real_normal3d(&(*matrix).left), csprintf(temporary, "%s had a bad left (%f,%f,%f)", string, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k));						\
match_vassert(file, line, valid_real_normal3d(&(*matrix).up), csprintf(temporary, "%s had a bad up (%f,%f,%f)", string, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k));								\
match_vassert(file, line, valid_real_point3d(&(*matrix).position), csprintf(temporary, "%s had a bad position (%f,%f,%f)", string, (*matrix).position.x, (*matrix).position.y, (*matrix).position.z));	\
match_vassert(file, line,													\
	valid_realcmp(dot_product3d(&(*matrix).forward, &(*matrix).left), 0.f),	\
	csprintf(																\
		temporary,															\
		"%s had a forward (%f,%f,%f) not perpendicular to left (%f,%f,%f)",	\
		string,																\
		(*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k,		\
		(*matrix).left.i, (*matrix).left.j, (*matrix).left.k));				\
match_vassert(file, line, 													\
	valid_realcmp(dot_product3d(&(*matrix).up, &(*matrix).left), 0.f),		\
	csprintf(																\
		temporary,															\
		"%s had a up (%f,%f,%f) not perpendicular to left (%f,%f,%f)",		\
		string,																\
		(*matrix).up.i, (*matrix).up.j, (*matrix).up.k,						\
		(*matrix).left.i, (*matrix).left.j, (*matrix).left.k));				\
match_vassert(file, line, 													\
	valid_realcmp(dot_product3d(&(*matrix).forward, &(*matrix).up), 0.f),	\
	csprintf(																\
		temporary,															\
		"%s had a forward (%f,%f,%f) not perpendicular to up (%f,%f,%f)",	\
		string,																\
		(*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k,		\
		(*matrix).up.i, (*matrix).up.j, (*matrix).up.k));					\
match_vassert(file, line, valid_real_matrix4x3(matrix), csprintf(temporary, "%s: assert_valid_real_matrix4x3", string));	\


#define match_assert_valid_real_matrix4x3(file, line, matrix)							\
if (!valid_real_matrix4x3(matrix))														\
{																						\
	match_assert_valid_real_matrix4x3_internal(file, line, matrix, STRINGIFY(matrix));	\
}																						\

#define match_assert_valid_real_matrix4x3_custom_string(file, line, matrix, custom_string)	\
if (!valid_real_matrix4x3(matrix))															\
{																							\
	char const* string = (custom_string);													\
	match_assert_valid_real_matrix4x3_internal(file, line, matrix, string);					\
}

/* ---------- structures */

typedef float real;

union real_euler_angles2d
{
	real n[2];
	struct { real yaw; real pitch; };
};
typedef union real_euler_angles2d real_euler_angles2d;

union real_euler_angles3d
{
	real n[3];
	struct { real yaw, pitch, roll; };
};
typedef union real_euler_angles3d real_euler_angles3d;

union real_point2d
{
	real n[2];
	struct { real x; real y; };
	struct { real u; real v; };
};
typedef union real_point2d real_point2d;

union real_point3d
{
	real n[3];
	struct { real x, y, z; };
	struct { real u, v, w; };
};
typedef union real_point3d real_point3d;

union real_vector2d
{
	real n[2];
	struct { real i; real j; };
};
typedef union real_vector2d real_vector2d;

union real_vector3d
{
	real n[3];
	struct { real i, j, k; };
};
typedef union real_vector3d real_vector3d;

union real_vector4d
{
	real n[4];
	struct { real i; real j; real k; real l; };
};
typedef union real_vector4d real_vector4d;

struct real_quaternion
{
	real_vector3d v;
	real w;
};
typedef struct real_quaternion real_quaternion;

struct real_orientation
{
	real_quaternion rotation;
	real_point3d translation;
	real scale;
};
typedef struct real_orientation real_orientation;

struct real_matrix3x3
{
	union
	{
		real n[3][3];
		struct
		{
			real_vector3d forward;
			real_vector3d left;
			real_vector3d up;
		};
	};
};
typedef struct real_matrix3x3 real_matrix3x3;

struct real_matrix4x3
{
	real scale;
	union
	{
		real n[4][3];
		struct
		{
			real_vector3d forward;
			real_vector3d left;
			real_vector3d up;
			real_point3d position;
		};
	};
};
typedef struct real_matrix4x3 real_matrix4x3;

struct real_plane2d
{
	real_vector2d n;
	real d;
};
typedef struct real_plane2d real_plane2d;

struct real_plane3d
{
	real_vector3d n;
	real d;
};
typedef struct real_plane3d real_plane3d;

union real_rectangle2d
{
	real n[4];
	struct { real x0; real x1; real y0; real y1; };
};
typedef union real_rectangle2d real_rectangle2d;

union real_rectangle3d
{
	real n[6];
	struct { real x0; real x1; real y0; real y1; real z0; real z1; };
};
typedef union real_rectangle3d real_rectangle3d;

union real_rgb_color
{
	real n[3];
	struct { real red, green, blue; };
};
typedef union real_rgb_color real_rgb_color;

union real_argb_color
{
	real n[4];
	struct
	{
		real alpha;
		union
		{
			real_rgb_color rgb;
			struct { real red, green, blue; };
		};
	};
};
typedef union real_argb_color real_argb_color;

/* ---------- prototypes/REAL_MATH.C */

void real_math_initialize(void);
void real_math_dispose(void);
void real_math_reset_precision(void);

real_vector2d *perpendicular2d(real_vector2d const *a, real_vector2d *result);
real_vector3d *perpendicular3d(real_vector3d const *a, real_vector3d *result);
real_vector4d *perpendicular4d(real_vector4d const *a, real_vector4d *result);

real_vector3d *rotate_vector_about_axis(real_vector3d *v, real_vector3d const *n, real sine, real cosine);

void vectors_interpolate(real_vector3d const *a, real_vector3d const *b, real t, real_vector3d *result);
void points_interpolate(real_point3d const *a, real_point3d const *b, real t, real_point3d *result);
void scalars_interpolate(real a, real b, real t, real *result);
void scalars_interpolate_and_clamp_0_to_1(real a, real b, real t, real *result);

void component_vectors_from_normal3d(real_vector3d const *vector, real_vector3d const *normal, real_vector3d *parallel, real_vector3d *perpendicular);
void component_vectors_from_direction3d(real_vector3d const *vector, real_vector3d const *direction, real_vector3d *parallel, real_vector3d *perpendicular);

void quaternions_multiply(real_quaternion const *q0, real_quaternion const *q1, real_quaternion *result);
void quaternions_interpolate(real_quaternion const *q0, real_quaternion const *q1, real t, real_quaternion *result);
void quaternion_transform_point(real_quaternion const *q, real_point3d const *p, real_point3d *result);

void vectors3d_from_euler_angles3d(real_vector3d *forward, real_vector3d *up, real_euler_angles3d const *angles);
boolean fast_vector_intersects_sphere(real_point3d const *point, real_vector3d const *vector, real_point3d const *center, real radius);

boolean point_in_rectangle2d(real_point2d const *point, real_rectangle2d const *bounds);
boolean point_in_rectangle3d(real_point3d const *point, real_rectangle3d const *bounds);

boolean vector_intersects_rectangle2d(real_point2d const *point, real_vector2d const *vector, real_rectangle2d const *bounds);
boolean vector_intersects_rectangle3d(real_point3d const *point, real_vector3d const *vector, real_rectangle3d const *bounds);
boolean pill_intersects_rectangle3d(real_point3d const *base, real_vector3d const *height, real width, real_rectangle3d const *bounds);

real_vector2d *fast_normalize2d(real_vector2d *v);
real_vector3d *fast_normalize3d(real_vector3d *v);

real cross_product_magnitude3d(real_vector3d const *a, real_vector3d const *b);

real dequantize_byte_to_real(real min, real max, unsigned char value);

real signed_angle_between_vectors2d(real_vector2d const *a, real_vector2d const *b);
real angle_between_vectors2d(real_vector2d const *a, real_vector2d const *b);
real angle_between_vectors3d(real_vector3d const *a, real_vector3d const *b);
real angle_between_normals3d(real_vector3d const *a, real_vector3d const *b);

void yaw_vectors(real_vector3d *forward, real_vector3d const *up, real sine, real cosine);
void pitch_vectors(real_vector3d *forward, real_vector3d *up, real sine, real cosine);

void fast_normals_interpolate(real_vector3d const *a, real_vector3d const *b, real t, real_vector3d *result);
void normals_interpolate(real_vector3d const *a, real_vector3d const *b, real t, real_vector3d *result);

real_vector3d *reflect_vector3d(real_vector3d const *incident, real_vector3d const *normal, real_vector3d *reflection);
real_vector3d *refract_vector3d(real_vector3d const *incident, real_vector3d const *normal, real coefficient_of_refraction, real_vector3d *refraction);

void quaternion_normalize(real_quaternion *q);
void quaternion_from_angle_and_vector(real_quaternion *q, real a, real_vector3d const *v);
void quaternion_to_angle_and_vector(real_quaternion const *q, real *a, real_vector3d *v);
void quaternions_interpolate_and_normalize(real_quaternion const *q0, real_quaternion const *q1, real t, real_quaternion *result);

void orientations_interpolate(real_orientation const *a, real_orientation const *b, real t, real_orientation *result);

real_euler_angles2d *euler_angles2d_from_vector3d(real_euler_angles2d *angles, real_vector3d const *vector);
real_vector3d *vector3d_from_angle(real_vector3d *vector, real angle);
real_vector3d *vector3d_from_euler_angles2d(real_vector3d *vector, real_euler_angles2d const *angles);

boolean point_in_pill2d(real_point2d const *point, real_point2d const *base, real_vector2d const *height, real width);
real point_to_line_distance_squared3d(real_point3d const *point, real_point3d const *base, real_vector3d const *height);

boolean sphere_test_vector3d(real_point3d const *center, real radius, real_point3d const *point, real_vector3d const *vector, real *t, real_vector3d *normal);

boolean valid_real_sine_cosine(real sine, real cosine);

void angular_accelerate_to_position(
	real_vector3d *position,
	real_vector3d const *position_desired,
	real_vector3d *angular_velocity,
	real angular_velocity_magnitude_maximum,
	real angular_acceleration_magnitude_maximum);

/* ---------- prototypes/MATRIX_MATH.C */

void matrix4x3_identity(real_matrix4x3 *matrix);
void matrix4x3_transpose(real_matrix4x3 *matrix);
void matrix4x3_inverse(real_matrix4x3 const *matrix, real_matrix4x3 *result);
void matrix4x3_scale(real_matrix4x3 *matrix, real scale);
void matrix4x3_translation(real_matrix4x3 *matrix, real_point3d const *point);
void matrix4x3_rotation_from_axis_and_angle(real_matrix4x3 *matrix, real_vector3d const *axis, real sine, real cosine);
void matrix4x3_rotation_from_vectors(real_matrix4x3 *matrix, real_vector3d const *forward, real_vector3d const *up);
void matrix4x3_rotation_from_angles(real_matrix4x3 *matrix, real yaw, real pitch, real roll);
void matrix4x3_rotation_to_angles(real_matrix4x3 *matrix, real_euler_angles3d *angles);
void matrix4x3_rotation_from_quaternion(real_matrix4x3 *matrix, real_quaternion const *quaternion);
void matrix4x3_rotation_to_quaternion(real_matrix4x3 const *matrix, real_quaternion *quaternion);
void matrix4x3_from_point_and_vectors(real_matrix4x3 *matrix, real_point3d const *point, real_vector3d const *forward, real_vector3d const *up);
void matrix4x3_from_point_and_quaternion(real_matrix4x3 *matrix, real_point3d const *point, real_quaternion const *quaternion);
void matrix4x3_from_orientation(real_matrix4x3 *matrix, real_orientation const *orientation);
void matrix4x3_from_plane(real_matrix4x3 *matrix, real_plane3d const *plane);
void matrix4x3_to_point_and_vectors(real_matrix4x3 const *matrix, real_point3d *point, real_vector3d *forward, real_vector3d *up);
real_vector3d *vector_from_matrices4x3(real_matrix4x3 const *a, real_matrix4x3 const *b, real_vector3d *rotation);
real_point3d *matrix4x3_transform_point(real_matrix4x3 const *matrix, real_point3d const *point, real_point3d *result);
real_vector3d *matrix4x3_transform_vector(real_matrix4x3 const *matrix, real_vector3d const *vector, real_vector3d *result);
real_vector3d *matrix4x3_transform_normal(real_matrix4x3 const *matrix, real_vector3d const *normal, real_vector3d *result);
real_plane3d *matrix4x3_transform_plane(real_matrix4x3 const *matrix, real_plane3d const *plane, real_plane3d *result);
real_point3d *matrix4x3_inverse_transform_point(real_matrix4x3 const *matrix, real_point3d const *point, real_point3d *result);
real_vector3d *matrix4x3_inverse_transform_vector(real_matrix4x3 const *matrix, real_vector3d const *vector, real_vector3d *result);
real_vector3d *matrix4x3_inverse_transform_normal(real_matrix4x3 const *matrix, real_vector3d const *normal, real_vector3d *result);
real_plane3d *matrix4x3_inverse_transform_plane(real_matrix4x3 const *matrix, real_plane3d const *plane, real_plane3d *result);
void matrix4x3_multiply(real_matrix4x3 const *a, real_matrix4x3 const *b, real_matrix4x3 *result);

/* ---------- prototypes/RANDOM_MATH.C */

unsigned long *get_global_random_seed_address(void);
unsigned long *get_global_local_random_seed_address(void);

unsigned short seed_random(unsigned long *seed);
short seed_random_range(unsigned long *seed, short lower_bound, short upper_bound);

real real_seed_random(unsigned long *seed);
real real_seed_random_range(unsigned long *seed, real lower_bound, real upper_bound);

real_vector3d *seed_random_direction3d(unsigned long *seed, real_vector3d *direction);
real_vector3d *seed_random_vector_in_cone3d(unsigned long *seed, real_vector3d const *axis, real inner_cone_angle, real outer_cone_angle, real_vector3d *result);

/* ---------- globals */

extern short const global_projection3d_mappings[3][2][2];

extern const real_point3d *const global_origin3d;

extern const real_point3d *const global_x_axis3d;
extern const real_point3d *const global_y_axis3d;
extern const real_point3d *const global_z_axis3d;
extern const real_point3d *const global_negative_x_axis3d;
extern const real_point3d *const global_negative_y_axis3d;
extern const real_point3d *const global_negative_z_axis3d;

extern const real_vector3d *const global_zero_vector3d;

extern const real_vector3d *const global_forward3d;
extern const real_vector3d *const global_left3d;
extern const real_vector3d *const global_up3d;

extern const real_vector3d *const global_backward3d;
extern const real_vector3d *const global_right3d;
extern const real_vector3d *const global_down3d;

/* ---------- public code */

__inline real sine(
	real angle)
{
	return sin(angle);
}

__inline real cosine(
	real angle)
{
	return cos(angle);
}

__inline real tangent(
	real angle)
{
	return tan(angle);
}

__inline real arctangent(
	real y,
	real x)
{
	return atan2(y, x);
}

__inline real arccosine(
	real x)
{
	return acos(x);
}

__inline real arcsine(
	real x)
{
	return asin(x);
}

__inline real signed_angular_difference(
	real angle1,
	real angle2)
{
	real result = angle2-angle1;
	
	if (result>=_pi)
	{
		result-= (_pi*2.f);
	}
	
	if (result<=(-_pi))
	{
		result+= (_pi*2.f);
	}

	return result;
}

__inline real square_root(
	real x)
{
	return sqrt(x);
}

__inline real reciprocal_square_root(
	real x)
{
	return 1.f / square_root(x);
}

__inline real power(
	real a, 
	real b)
{
	return pow(a, b);
}

__inline real_point2d *set_real_point2d(
	real_point2d *p, 
	real x,
	real y)
{
	p->x = x;
	p->y = y;
	return p;
}

__inline real_vector2d *set_real_vector2d(
	real_vector2d *v,
	real i,
	real j)
{
	v->i = i;
	v->j = j;
	return v;
}

__inline real_point2d *point_from_line2d(
	real_point2d const *p, 
	real_vector2d const *v,
	real t, 
	real_point2d *result)
{
	result->x = t*v->i + p->x;
	result->y = t*v->j + p->y;
	return result;
}

__inline real_vector2d *vector_from_points2d(
	real_point2d const *a,
	real_point2d const *b, 
	real_vector2d *result)
{
	result->i = b->x-a->x;
	result->j = b->y-a->y;
	return result;
}

__inline real_vector2d *scale_vector2d(
	real_vector2d const *a,
	real c,
	real_vector2d *result)
{
	result->i = c*a->i;
	result->j = c*a->j;
	return result;
}

__inline real_vector2d *rotate_vector2d(
	real_vector2d const *v,
	real sine, 
	real cosine,
	real_vector2d *result)
{
	result->i = cosine*v->i - sine*v->j;
	result->j = sine*v->i + cosine*v->j;
	return result;
}

__inline real magnitude_squared2d(
	real_vector2d const *v)
{
	return v->i*v->i + v->j*v->j;
}

__inline real magnitude2d(
	real_vector2d const *v)
{
	return square_root(magnitude_squared2d(v));
}

__inline real normalize2d(
	real_vector2d *v)
{
	real magnitude = magnitude2d(v);

	if (!(_real_epsilon>fabs(magnitude-0.f)))
	{
		scale_vector2d(v, 1.f / magnitude, v);
	}
	else
	{
		magnitude = 0.f;
	}

	return magnitude;
}

__inline boolean limit2d(
	real_vector2d *vector, 
	real length)
{
	real dot = vector->i*vector->i + vector->j*vector->j;
	if (dot <= length*length)
		return FALSE;

	scale_vector2d(vector, length / square_root(dot), vector);
	return TRUE;
}

__inline real distance_squared2d(
	real_point2d const *a,
	real_point2d const *b)
{
	real_vector2d v;
	return magnitude_squared2d(vector_from_points2d(a, b, &v));
}

__inline real distance2d(
	real_point2d const *a,
	real_point2d const *b)
{
	real_vector2d v;
	return magnitude2d(vector_from_points2d(a, b, &v));
}

__inline real dot_product2d(
	real_vector2d const *a,
	real_vector2d const *b)
{
	return a->i*b->i + a->j*b->j;
}

__inline real cross_product2d(
	real_vector2d const *a,
	real_vector2d const *b)
{
	return a->i*b->j - a->j*b->i;
}

__inline real_vector2d *negate_vector2d(
	real_vector2d const *a, 
	real_vector2d *result)
{
	result->i = -a->i;
	result->j = -a->j;
	return result;
}

__inline short projection_from_vector3d(
	real_vector3d const *n)
{
	real i = fabs(n->i);
	real j = fabs(n->j);
	real k = fabs(n->k);

	if (k >= j && k >= i)
	{
		return _z;
	}
	else if (j >= i)
	{
		return _y;
	}

	return _x;
}

__inline boolean projection_sign_from_vector3d(
	real_vector3d const *n, 
	short projection)
{
	match_assert("..\\math\\real_math.h", 848, projection>=_x && projection<=_z);
	return n->n[projection] > 0.f;
}

__inline real_point2d *project_point3d(
	real_point3d const *p3d, 
	short projection, 
	boolean sign,
	real_point2d *p2d)
{
	match_assert("..\\math\\real_math.h", 859, projection>=_x && projection<=_z);
	match_assert("..\\math\\real_math.h", 860, ~(sign&~1));

	set_real_point2d(
		p2d,
		p3d->n[global_projection3d_mappings[projection][sign][0]],
		p3d->n[global_projection3d_mappings[projection][sign][1]]);
	return p2d;
}

__inline real_point3d *project_point2d(
	real_point2d const *p2d,
	real_plane3d const *plane,
	short projection, 
	boolean sign,
	real_point3d *p3d)
{
	long x = global_projection3d_mappings[projection][sign][0];
	long y = global_projection3d_mappings[projection][sign][1];

	match_assert("..\\math\\real_math.h", 879, projection>=_x && projection<=_z);
	match_assert("..\\math\\real_math.h", 880, ~(sign&~1));

	p3d->n[x] = p2d->x;
	p3d->n[y] = p2d->y;

	if (fabs(plane->n.n[projection])>=_real_epsilon)
	{
		p3d->n[projection] = ((plane->d - (p2d->x * plane->n.n[x])) - (p2d->y * plane->n.n[y])) / plane->n.n[projection];
	}
	else
	{
		p3d->n[projection] = 0.f;
	}

	return p3d;
}

__inline real_point3d *set_real_point3d(
	real_point3d *p,
	real x,
	real y,
	real z)
{
	p->x = x;
	p->y = y;
	p->z = z;

	return p;
}

__inline real_vector3d *set_real_vector3d(
	real_vector3d *v,
	real i,
	real j,
	real k)
{
	v->i = i;
	v->j = j;
	v->k = k;

	return v;
}

__inline real_point3d *point_from_line3d(
	real_point3d const *p, 
	real_vector3d const *v, 
	real t,
	real_point3d *result)
{
	result->x = v->i*t + p->x;
	result->y = v->j*t + p->y;
	result->z = v->k*t + p->z;
	return result;
}

__inline real_vector3d *vector_from_points3d(
	real_point3d const *a,
	real_point3d const *b,
	real_vector3d *result)
{
	result->i = b->x-a->x;
	result->j = b->y-a->y;
	result->k = b->z-a->z;
	return result;
}

__inline real_vector3d *scale_vector3d(
	real_vector3d const *a,
	real c,
	real_vector3d *result)
{
	result->i = c*a->i;
	result->j = c*a->j;
	result->k = c*a->k;
	return result;
}

__inline real magnitude_squared3d(
	real_vector3d const *v)
{
	return v->i*v->i + v->j*v->j + v->k*v->k;
}

__inline real magnitude3d(
	real_vector3d const *v)
{
	return square_root(magnitude_squared3d(v));
}

__inline real normalize3d(
	real_vector3d *v)
{
	real magnitude = magnitude3d(v);

	if (!(_real_epsilon>fabs(magnitude-0.f)))
	{
		scale_vector3d(v, 1.f / magnitude, v);
	}
	else
	{
		magnitude = 0.f;
	}

	return magnitude;
}

__inline boolean limit3d(
	real_vector3d *vector,
	real length)
{
	real dot = vector->i*vector->i + vector->j*vector->j + vector->k*vector->k;
	if (dot <= length*length)
		return FALSE;

	scale_vector3d(vector, length / square_root(dot), vector);
	return TRUE;
}

__inline real distance_squared3d(
	real_point3d const *a,
	real_point3d const *b)
{
	real_vector3d v;
	return magnitude_squared3d(vector_from_points3d(a, b, &v));
}

__inline real distance3d(
	real_point3d const *a,
	real_point3d const *b)
{
	real_vector3d v;
	return magnitude3d(vector_from_points3d(a, b, &v));
}

__inline real_point3d *midpoint3d(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d *result)
{
	result->x = (p0->x + p1->x) * 0.5f;
	result->y = (p0->y + p1->y) * 0.5f;
	result->z = (p0->z + p1->z) * 0.5f;
	return result;
}

__inline real dot_product3d(
	real_vector3d const *a,
	real_vector3d const *b)
{
	return a->i*b->i + a->j*b->j + a->k*b->k;
}

__inline real_vector3d *cross_product3d(
	real_vector3d const *a,
	real_vector3d const *b,
	real_vector3d *result)
{
	real k = a->i*b->j - a->j*b->i;
	real j = a->k*b->i - a->i*b->k;
	result->i = a->j*b->k - a->k*b->j;
	result->j = j;
	result->k = k;

	return result;
}

__inline real triple_product3d(
	real_vector3d const *a,
	real_vector3d const *b,
	real_vector3d const *n)
{
	// TODO: doesn't match
	return (n->i * ((b->k * a->j) - (b->j * a->k)))
		 + (n->k * ((a->i * b->j) - (b->i * a->j)))
		 + (n->j * ((b->i * a->k) - (a->i * b->k)));
}

__inline real_vector3d *add_vectors3d(
	real_vector3d const *a,
	real_vector3d const *b,
	real_vector3d *result)
{
	result->i = a->i+b->i;
	result->j = a->j+b->j;
	result->k = a->k+b->k;

	return result;
}

__inline real_vector3d *subtract_vectors3d(
	real_vector3d const *a,
	real_vector3d const *b,
	real_vector3d *result)
{
	result->i = a->i-b->i;
	result->j = a->j-b->j;
	result->k = a->k-b->k;

	return result;
}

__inline real_vector3d *negate_vector3d(
	real_vector3d const *a,
	real_vector3d *result)
{
	result->i = -a->i;
	result->j = -a->j;
	result->k = -a->k;
	return result;
}

__inline real dot_product4d(
	real_vector4d const *a, 
	real_vector4d const *b)
{
	return a->i*b->i + a->j*b->j + a->k*b->k + a->l*b->l;
}

__inline real_plane2d *plane2d_from_points(
	real_plane2d *plane,
	real_point2d const *point0,
	real_point2d const *point1)
{
	vector_from_points2d(point0, point1, &plane->n);

	if (normalize2d(&plane->n) == 0.0f)
	{
		plane->d = 0.0f;
	}
	else
	{
		plane->d = dot_product2d(&plane->n, (real_vector2d const *)point0);
	}

	return plane;
}

__inline real plane2d_distance_to_point(
	real_plane2d const *plane,
	real_point2d const *point)
{
	return (plane->n.i*point->x + plane->n.j*point->y) - plane->d;
}
	
__inline real point_to_line_distance3d(
	real_point3d const *point,
	real_point3d const *base, 
	real_vector3d const *height)
{
	return square_root(point_to_line_distance_squared3d(point, base, height));
}

__inline real_plane3d *plane3d_from_point_and_normal(
	real_plane3d *plane,
	real_point3d const *point,
	real_vector3d const *normal
)
{
	plane->n = *normal;
	plane->d = dot_product3d(&plane->n, (real_vector3d const *)point);
	return plane;
}

__inline real_plane3d *plane3d_from_points(
	real_plane3d *plane,
	real_point3d const *point0,
	real_point3d const *point1,
	real_point3d const *point2)
{
	real_vector3d v0;
	real_vector3d v1;

	vector_from_points3d(point0, point1, &v0);
	vector_from_points3d(point0, point2, &v1);

	cross_product3d(&v0, &v1, &plane->n);

	if (normalize3d(&plane->n) == 0.0f)
	{
		plane->d = 0.0f;
	}
	else
	{
		plane->d = dot_product3d(&plane->n, (real_vector3d const *)point0);
	}

	return plane;
}

__inline real_plane3d *plane3d_negate(
	const real_plane3d *p1,
	real_plane3d *plane
)
{
	plane->n.i = -p1->n.i;
	plane->n.j = -p1->n.j;
	plane->n.k = -p1->n.k;
	plane->d = -p1->d;

	return plane;
}

__inline real plane3d_distance_to_point(
	real_plane3d const *plane,
	real_point3d const *point)
{
	return dot_product3d(&plane->n, (real_vector3d const *)point) - plane->d;
}

__inline real vector_intersect_plane3d(
	real_point3d const *point,
	real_vector3d const *vector,
	real_plane3d const *plane)
{
	return -(plane3d_distance_to_point(plane, point) / dot_product3d(vector, &plane->n));
}

__inline boolean point_in_circle(
	real_point2d const *point,
	real_point2d const *center, 
	real radius)
{
	return distance_squared2d(point, center) <= (radius*radius);
}

__inline boolean point_in_sphere(
	real_point3d const *point,
	real_point3d const *center,
	real radius)
{
	return distance_squared3d(point, center) <= (radius*radius);
}

__inline real_quaternion *set_real_quaternion(
	real_quaternion *q,
	real i,
	real j,
	real k,
	real w)
{
	q->v.i = i;
	q->v.j = j;
	q->v.k = k;
	q->w = w;
	return q;
}

__inline real_euler_angles2d *set_real_euler_angles2d(
	real_euler_angles2d *angles,
	real pitch, real yaw)
{
	angles->pitch = pitch;
	angles->yaw = yaw;
	return angles;
}

__inline void set_random_seed(
	unsigned long seed)
{
	*get_global_random_seed_address() = seed;
}

__inline word random(
	void)
{
	return seed_random(get_global_random_seed_address());
}

__inline short random_range(
	short lower_bound,
	short upper_bound)
{
	return seed_random_range(get_global_random_seed_address(), lower_bound, upper_bound);
}

__inline boolean random_boolean(
	void)
{
	return random() > 0x8000;
}

__inline real real_random(
	void)
{
	return real_seed_random(get_global_random_seed_address());
}

__inline real real_random_range(
	real lower_bound,
	real upper_bound)
{
	return real_seed_random_range(get_global_random_seed_address(), lower_bound, upper_bound);
}

__inline real_vector3d *random_direction3d(
	real_vector3d *direction)
{
	return seed_random_direction3d(get_global_random_seed_address(), direction);
}

__inline real_vector3d *random_vector_in_cone3d(
	real_vector3d const *axis,
	real inner_cone_angle,
	real outer_cone_angle,
	real_vector3d *result)
{
	return seed_random_vector_in_cone3d(get_global_random_seed_address(), axis, inner_cone_angle, outer_cone_angle, result);
}

__inline word local_random(
	void)
{
	return seed_random(get_global_local_random_seed_address());
}

__inline short local_random_range(
	short lower_bound,
	short upper_bound)
{
	return seed_random_range(get_global_local_random_seed_address(), lower_bound, upper_bound);
}

__inline real real_local_random(
	void)
{
	return real_seed_random(get_global_local_random_seed_address());
}

__inline real real_local_random_range(
	real lower_bound,
	real upper_bound)
{
	return real_seed_random_range(get_global_local_random_seed_address(), lower_bound, upper_bound);
}

__inline real_vector3d *local_random_direction3d(
	real_vector3d *direction)
{
	return seed_random_direction3d(get_global_local_random_seed_address(), direction);
}

__inline real_vector3d *local_random_vector_in_cone3d(
	real_vector3d const *axis,
	real inner_cone_angle,
	real outer_cone_angle,
	real_vector3d *result)
{
	return seed_random_vector_in_cone3d(get_global_local_random_seed_address(), axis, inner_cone_angle, outer_cone_angle, result);
}

// TODO: doesn't match, needs cleanup
__inline real uniform_cubic_spline(
	real f0, 
	real f1, 
	real f2, 
	real f3, 
	real t0, 
	real h, 
	real t)
{
	real v7, v8, v9, v10, v11;
	v7 = f2 - f1;
	v8 = f1 - f0;
	v9 = (f3 - f2) - (f2 - f1);
	v10 = v7 - (f1 - f0);
	v11 = ((((((((t - ((h * 2.f) + t0)) * (v9 - v10)) 
		/ (h * 3.f)) + v10) * (t - (t0 + h))) 
		/ (h * 2.f)) + v8)
		* ((t - t0) / h))
		 + f0;
	return v11;
}

// TODO: doesn't match, needs cleanup
__inline real nonuniform_cubic_spline(
	real f0,
	real f1,
	real f2, 
	real f3,
	real t0, 
	real t1, 
	real t2, 
	real t3, 
	real t)
{
	real v9, v10, v11;
	v10 = ((f2 - f1) / (t2 - t1)) - ((f1 - f0) / (t1 - t0));
	v11 = (((((((((((f3 - f2) / (t3 - t2)) - ((f2 - f1) 
		/ (t2 - t1))) / (t3 - t1)) - (v10 / (t2 - t0))) 
		/ (t3 - t0))
		* (v9 - t2))
		+ (v10 / (t2 - t0)))
		* (v9 - t1))
		+ ((f1 - f0) / (t1 - t0)))
		* (v9 - t0))
		 + f0;
	return v11;
}

// TODO: doesn't match, needs cleanup. uses above inlines
__inline void uniform_cubic_spline_vector3d(
	real_vector3d *result, 
	real_vector3d const *f0,
	real_vector3d const *f1, 
	real_vector3d const *f2, 
	real_vector3d const *f3,
	real t0,
	real h,
	real t)
{
	real v8, v9, v10, v11, v12, v13;
	v8 = h * 2.f;
	v9 = h * 3.f;
	v11 = (h * 2.f) + t0;
	v12 = t - (t0 + h);
	v13 = ((((((t - ((h * 2.f) + t0))
		* (((f3->i - f2->i) - (f2->i - f1->i)) - ((f2->i - f1->i) - (f1->i - f0->i))))
		/ (h * 3.f))
		+ ((f2->i - f1->i) - (f1->i - f0->i)))
		* (t - (t0 + h)))
		 / (h * 2.f));
	v10 = 1.0 / h;

	result->i = (((v13 + (f1->i - f0->i)) * v10) * (t - t0)) + f0->i;

	result->j = (((((((((t - v11)
		* (((f3->j - f2->j) - (f2->j - f1->j))
		- ((f2->j - f1->j) - (f1->j - f0->j))))
		/ v9)
		+ ((f2->j - f1->j) - (f1->j - f0->j)))
		* v12)
		/ v8)
		+ (f1->j - f0->j))
		* v10)
		* (t - t0))
		+ f0->j;

	result->k = (((((((((t - v11)
		* (((f3->k - f2->k) - (f2->k - f1->k))
		- ((f2->k - f1->k) - (f1->k - f0->k))))
		/ v9)
		+ ((f2->k - f1->k) - (f1->k - f0->k)))
		* v12)
		/ v8)
		+ (f1->k - f0->k))
		* v10)
		* (t - t0))
		+ f0->k;
}

// TODO: doesn't match, needs cleanup. uses above inlines
__inline void nonuniform_cubic_spline_vector3d(
	real_vector3d *result,
	real_vector3d const *f0, 
	real_vector3d const *f1,
	real_vector3d const *f2,
	real_vector3d const *f3, 
	real t0, 
	real t1, 
	real t2,
	real t3,
	real t)
{
	real v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
	v10 = t3 - t2;
	v11 = t2 - t1;
	v12 = t1 - t0;
	v13 = t3 - t1;
	v14 = t2 - t0;
	v15 = t3 - t0;
	v16 = t - t2;
	v17 = t - t1;
	v18 = t - t0;

	result->i = (((((((((((f3->i - f2->i) / v10) - ((f2->i - f1->i) / v11)) / v13)
		- ((((f2->i - f1->i) / v11) - ((f1->i - f0->i) / v12)) / v14))
		/ v15)
		* v16)
		+ ((((f2->i - f1->i) / v11) - ((f1->i - f0->i) / v12)) / v14))
		* v17)
		+ ((f1->i - f0->i) / v12))
		* v18)
		+ f0->i;

	result->j = (((((((((((f3->j - f2->j) / v10) - ((f2->j - f1->j) / v11)) / v13)
		- ((((f2->j - f1->j) / v11) - ((f1->j - f0->j) / v12)) / v14))
		/ v15)
		* v16)
		+ ((((f2->j - f1->j) / v11) - ((f1->j - f0->j) / v12)) / v14))
		* v17)
		+ ((f1->j - f0->j) / v12))
		* v18)
		+ f0->j;

	v19 = (((f3->k - f2->k) / v10) - ((f2->k - f1->k) / v11));
	v20 = (((f2->k - f1->k) / v11) - ((f1->k - f0->k) / v12));

	result->k = ((((((((v19 / v13) - (v20 / v14)) / v15) * v16) + (v20 / v14)) * v17) + ((f1->k - f0->k) / v12))
				* v18)
				+ f0->k;
}

__inline boolean valid_real(
	real n)
{
	return (*(long *)&n & 0x7F800000) != 0x7F800000;
}

__inline boolean valid_realcmp(
	real a,
	real b)
{
	real y = a - b;
	return valid_real(y) && fabs(y) < 0.001f;
}

__inline boolean valid_real_point2d(
	real_point2d const *p)
{
	return valid_real(p->x) && valid_real(p->y);
}

__inline boolean valid_real_point3d(
	real_point3d const *p)
{
	return valid_real(p->x) && valid_real(p->y) && valid_real(p->z);
}

__inline boolean valid_real_vector3d(
	real_vector3d const *v)
{
	return valid_real(v->i) && valid_real(v->j) && valid_real(v->k);
}

__inline boolean valid_real_normal3d(
	real_vector3d const *n)
{
	return valid_realcmp(magnitude_squared3d(n), 1.f);
}

__inline boolean valid_real_vector3d_axes2(
	real_vector3d const *forward,
	real_vector3d const *up)
{
	boolean result = FALSE;
	if (valid_real_normal3d(forward) && valid_real_normal3d(up))
	{
		real product = dot_product3d(forward, up);
		if (valid_realcmp(product, 0.f))
		{
			result = TRUE;
		}
	}
	return result;
}

__inline boolean valid_real_vector3d_axes3(
	real_vector3d const *f,
	real_vector3d const *l,
	real_vector3d const *u)
{
	return
		valid_real_normal3d(f) &&
		valid_real_normal3d(l) &&
		valid_real_normal3d(u) &&
		valid_realcmp(dot_product3d(f, l), 0.f) &&
		valid_realcmp(dot_product3d(l, u), 0.f) &&
		valid_realcmp(dot_product3d(u, f), 0.f);
}

__inline boolean valid_real_matrix4x3(
	real_matrix4x3 const *matrix)
{
	return
		valid_real(matrix->scale) &&
		valid_real_vector3d_axes3(&matrix->forward, &matrix->left, &matrix->up) &&
		valid_real_point3d(&matrix->position);
}

// TODO: depends on byte_rectangle3d, need to include integer_math?
/*
__inline real_rectangle3d *dequantize_byte_to_real_rectangle3d(
	real_rectangle3d const *parent,
	byte_rectangle3d const *compressed_rectangle,
	real_rectangle3d *result)
{
	result->x0 = dequantize_byte_to_real(parent->x0, parent->x1, compressed_rectangle->x0);
	result->x1 = dequantize_byte_to_real(parent->x0, parent->x1, compressed_rectangle->x1);
	result->y0 = dequantize_byte_to_real(parent->y0, parent->y1, compressed_rectangle->y0);
	result->y1 = dequantize_byte_to_real(parent->y0, parent->y1, compressed_rectangle->y1);
	result->z0 = dequantize_byte_to_real(parent->z0, parent->z1, compressed_rectangle->z0);
	result->z1 = dequantize_byte_to_real(parent->z0, parent->z1, compressed_rectangle->z1);
}
*/

__inline void interpolate_scalar(
	real *current, 
	real desired,
	real maximum_speed)
{
	real difference = desired - *current;
	*current += PIN(difference, -maximum_speed, maximum_speed);
	return;
}

#endif // __REAL_MATH_H
