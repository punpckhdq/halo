/*
REAL_MATH.H

file has inline function assertions.
*/

#ifndef __REAL_MATH_H
#define __REAL_MATH_H
#pragma once

/* ---------- constants */

#define _real_epsilon 0.0001f

/* ---------- macros */

#define assert_valid_real_point3d(point)\
vassert(												\
	valid_real_point3d(point),							\
	csprintf(											\
		temporary,										\
		"%s: assert_valid_real_point3d(%f, %f, %f)",	\
		#point, (*point).x, (*point).y, (*point).z		\
	)													\
)

#define assert_valid_real_vector3d(file, line, vector)	\
vassert(														\
	valid_real_vector3d(vector),								\
	csprintf(													\
		temporary,												\
		"%s: assert_valid_real_vector2d(%f, %f, %f)",			\
		#vector, (*vector).i, (*vector).j, (*vector).k			\
	)															\
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

real normalize3d(real_vector3d *v);

void vectors3d_from_euler_angles3d(real_vector3d *forward, real_vector3d *up, real_euler_angles3d const *angles);

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
	real result= angle2-angle1;
	
	if (result>=M_PI)
	{
		result-= (M_PI*2.f);
	}
	
	if (result<=(-M_PI))
	{
		result+= (M_PI*2.f);
	}

	return result;
}

__inline real_point3d *point_from_line3d(real_point3d const *p, real_vector3d const *v, real t, real_point3d *result)
{
	result->x= (v->i*t) + p->x;
	result->y= (v->j*t) + p->y;
	result->z= (v->k*t) + p->z;
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
	result->i= b->x-a->x;
	result->j= b->y-a->y;
	result->k= b->z-a->z;
	return result;
}

__inline real magnitude_squared3d(
	real_vector3d const *vector)
{
	return vector->i*vector->i + vector->j*vector->j + vector->k*vector->k;
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
	result->i= -a->i;
	result->j= -a->j;
	result->k= -a->k;
	return result;
}

__inline real_point3d *set_real_point3d(
	real_point3d *p,
	real x,
	real y,
	real z)
{
	p->x= x;
	p->y= y;
	p->z= z;

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
	real result= a - b;
	return valid_real(result) && fabs(result) < 0.01f;
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
	boolean result= FALSE;
	if (valid_real_normal3d(forward) && valid_real_normal3d(up))
	{
		real product= dot_product3d(forward, up);
		if (valid_realcmp(product, 0.f))
		{
			result= TRUE;
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
	real k= (a->i*b->j) - (a->j*b->i);
	real j= (a->k*b->i) - (a->i*b->k);
	real i= (a->j*b->k) - (a->k*b->j);
	result->i= i;
	result->j= j;
	result->k= k;

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

#endif // __REAL_MATH_H
