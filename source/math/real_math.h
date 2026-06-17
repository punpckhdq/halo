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
	char* string = custom_string;															\
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

real angle_between_vectors3d(real_vector3d const *a, real_vector3d const *b);

real_vector3d *perpendicular3d(real_vector3d const *a, real_vector3d *result);

real_vector3d *rotate_vector_about_axis(real_vector3d *v, real_vector3d const *n, float sine, float cosine);

real_euler_angles2d *euler_angles2d_from_vector3d(real_euler_angles2d *angles, real_vector3d const *vector);
void vectors3d_from_euler_angles3d(real_vector3d *forward, real_vector3d *up, real_euler_angles3d const *angles);
real_vector3d *vector3d_from_euler_angles2d(real_vector3d *vector, real_euler_angles2d const *angles);


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

short seed_random_range(unsigned long *seed, short lower_bound, short upper_bound);

real real_seed_random(unsigned long *seed);
real real_seed_random_range(unsigned long *seed, real lower_bound, real upper_bound);

/* ---------- globals */

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

__inline real arccosine(real x)
{
	return acos(x);
}

__inline real arcsine(real x)
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

__inline real_point3d *point_from_line3d(real_point3d const *p, real_vector3d const *v, real t, real_point3d *result)
{
	result->x = (v->i*t) + p->x;
	result->y = (v->j*t) + p->y;
	result->z = (v->k*t) + p->z;
	return result;
}

__inline real dot_product3d(
	real_vector3d const *a,
	real_vector3d const *b)
{
	return (a->i*b->i) + (a->j*b->j) + (a->k*b->k);
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
	real_vector3d const *vector)
{
	return vector->i*vector->i + vector->j*vector->j + vector->k*vector->k;
}

__inline real magnitude3d(
	real_vector3d const *v)
{
	return square_root(magnitude_squared3d(v));
}

__inline real normalize3d(
	real_vector3d *v)
{
	real result = magnitude3d(v);
	if (fabs(result-0.f)>=_real_epsilon)
	{
		scale_vector3d(v, 1.f / result, v);
	}
	else
	{
		result = 0.f;
	}

	return result;
}

__inline real distance_squared3d(
	real_point3d const *a,
	real_point3d const *b)
{
	real_vector3d v;

	return magnitude_squared3d(vector_from_points3d(a, b, &v));
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

__inline boolean valid_real(
	real n)
{
	return (*(long *)&n & 0x7F800000) != 0x7F800000;
}

__inline boolean valid_realcmp(
	real a,
	real b)
{
	real result = a - b;
	return valid_real(result) && fabs(result) < 0.001f;
}

__inline boolean valid_real_point3d(
	real_point3d const *p)
{
	return valid_real(p->x) && valid_real(p->y) && valid_real(p->z);
}

__inline boolean valid_real_vector3d(real_vector3d const *v)
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

__inline real_vector3d *cross_product3d(
	real_vector3d const *a,
	real_vector3d const *b,
	real_vector3d *result)
{
	real k = (a->i*b->j) - (a->j*b->i);
	real j = (a->k*b->i) - (a->i*b->k);
	real i = (a->j*b->k) - (a->k*b->j);
	result->i = i;
	result->j = j;
	result->k = k;

	return result;
}

__inline real_vector3d *subtract_vectors3d(
	real_vector3d const *a,
	real_vector3d const *b,
	real_vector3d *result)
{
	result->i= a->i-b->i;
	result->j= a->j-b->j;
	result->k= a->k-b->k;

	return result;
}

__inline boolean point_in_sphere(
	real_point3d const *point,
	real_point3d const *center,
	real radius)
{
	return distance_squared3d(point, center)<=(radius*radius);
}

__inline short random_range(
	short lower_bound,
	short upper_bound)
{
	return seed_random_range(get_global_random_seed_address(), lower_bound, upper_bound);
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

__inline real_plane3d *plane3d_from_point_and_normal(
	real_plane3d *plane,
	real_point3d const *point,
	real_vector3d const *normal
)
{
	plane->n = *normal;
	plane->d = dot_product3d((real_vector3d *)point, &plane->n);

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
	real_point3d const *point
)
{
	return plane->n.i*point->x +
		   plane->n.j*point->y +
		   plane->n.k*point->z -
		   plane->d;
}

__inline real_plane3d *plane3d_from_points(
	real_plane3d *plane,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2)
{
	real_vector3d v0;
	real_vector3d v1;

	vector_from_points3d(p0, p1, &v0);
	vector_from_points3d(p0, p2, &v1);

	cross_product3d(&v0, &v1, &plane->n);

	if (normalize3d(&plane->n) == 0.0f)
	{
		plane->d = 0.0f;
	}
	else
	{
		plane->d = dot_product3d((real_vector3d *)p0, &plane->n);
	}

	return plane;
}

#endif // __REAL_MATH_H
