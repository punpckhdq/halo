/*
REAL_MATH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "shell.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static real_matrix4x3 const private_identity4x3 =
{
	1.f,
	{
		{ 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f }
	},
};

static real_matrix4x3 const private_negative_identity4x3 =
{
	1.f,
	{
		{ -1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f }
	},
};

static real_quaternion const private_identity_quaternion = /* fake name */
{
	{ 0.f, 0.f, 0.f },
	1.f
};

static real_rectangle3d const private_null_rectangle =
{
	REAL_MAX, REAL_MIN, REAL_MAX, REAL_MIN, REAL_MAX, REAL_MIN
};

short const global_projection3d_mappings[3][2][2] =
{
	{
		{ 2, 1 },
		{ 1, 2 },
	},
	{
		{ 0, 2 },
		{ 2, 0 },
	},
	{
		{ 1, 0 },
		{ 0, 1 },
	}
};

real_point2d const *global_origin2d = (real_point2d const *)&private_identity4x3.up;
real_point2d const *global_x_axis2d = (real_point2d const *)&private_identity4x3.forward;
real_point2d const *global_y_axis2d = (real_point2d const *)&private_identity4x3.left;
real_point2d const *global_negative_x_axis2d = (real_point2d const *)&private_negative_identity4x3.forward;
real_point2d const *global_negative_y_axis2d = (real_point2d const *)&private_negative_identity4x3.left;

real_vector2d const *global_zero_vector2d = (real_vector2d const *)&private_identity4x3.up;
real_vector2d const *global_forward2d = (real_vector2d const *)&private_identity4x3.forward;
real_vector2d const *global_left2d = (real_vector2d const *)&private_identity4x3.left;
real_vector2d const *global_backward2d = (real_vector2d const *)&private_negative_identity4x3.forward;
real_vector2d const *global_right2d = (real_vector2d const *)&private_negative_identity4x3.left;

real_point3d const *global_origin3d = (real_point3d const *)&private_identity4x3.position;
real_point3d const *global_x_axis3d = (real_point3d const *)&private_identity4x3.forward;
real_point3d const *global_y_axis3d = (real_point3d const *)&private_identity4x3.left;
real_point3d const *global_z_axis3d = (real_point3d const *)&private_identity4x3.up;
real_point3d const *global_negative_x_axis3d = (real_point3d const *)&private_negative_identity4x3.forward;
real_point3d const *global_negative_y_axis3d = (real_point3d const *)&private_negative_identity4x3.left;
real_point3d const *global_negative_z_axis3d = (real_point3d const *)&private_negative_identity4x3.up;

real_vector3d const *global_zero_vector3d = (real_vector3d const *)&private_identity4x3.position;
real_vector3d const *global_forward3d = &private_identity4x3.forward;
real_vector3d const *global_left3d = &private_identity4x3.left;
real_vector3d const *global_up3d = &private_identity4x3.up;
real_vector3d const *global_backward3d = &private_negative_identity4x3.forward;
real_vector3d const *global_right3d = &private_negative_identity4x3.left;
real_vector3d const *global_down3d = &private_negative_identity4x3.up;

real_euler_angles2d const *global_zero_angles2d = (real_euler_angles2d const *)&private_identity4x3.up;
real_euler_angles3d const *global_zero_angles3d = (real_euler_angles3d const *)&private_identity4x3.up;

real_quaternion const *global_identity_quaternion = &private_identity_quaternion;
real_matrix4x3 const *global_identity4x3 = &private_identity4x3;
real_matrix4x3 const *global_negative_identity4x3 = &private_negative_identity4x3;

real_rectangle2d const *global_null_rectangle2d = (real_rectangle2d const *)&private_null_rectangle;
real_rectangle3d const *global_null_rectangle3d = &private_null_rectangle;

/* ---------- public code */

void real_math_initialize(
	void)
{
	random_math_initialize();
	periodic_functions_initialize();
	return;
}

void real_math_dispose(
	void)
{
	random_math_dispose();
	periodic_functions_dispose();
	return;
}

void real_math_reset_precision(
	void)
{
	_control87(CW_DEFAULT, 0xFFFFF);
	return;
}

real signed_angle_between_vectors2d(
	real_vector2d const *a,
	real_vector2d const *b)
{
	real dot_product = dot_product2d(a, b);
	real result = arccosine(PIN(dot_product, -1.f, 1.f));

	if ((a->i*b->j) - (a->j*b->i) < 0.f)
	{
		result = -result;
	}

	return result;
}

real angle_between_vectors2d(
	real_vector2d const *a,
	real_vector2d const *b)
{
	real angle = 0.f;
	real aa_bb = (a->i*a->i + a->j*a->j) * (b->i*b->i + b->j*b->j);
	
	if (aa_bb != 0.f)
	{
		real ab = dot_product2d(a, b);
		real c = PIN((ab/aa_bb*ab) + (ab/aa_bb*ab) - 1.f, -1.f, 1.f);

		angle = arccosine(c) * 0.5f;
		
		if (ab < 0.f)
		{
			angle = _pi - angle;
		}
	}

	return angle;
}

real angle_between_vectors3d(
	real_vector3d const *a,
	real_vector3d const *b)
{

	real angle = 0.f;
	real aa_bb = (a->i*a->i + a->j*a->j + a->k*a->k) * (b->i*b->i + b->j*b->j + b->k*b->k);

	if (aa_bb != 0.f)
	{
		real ab = dot_product3d(a, b);
		real c = PIN((ab/aa_bb*ab) + (ab/aa_bb*ab) - 1.f, -1.f, 1.f);

		angle = arccosine(c) * 0.5f;

		if (ab < 0.f)
		{
			angle = _pi - angle;
		}
	}

	return angle;
}

real angle_between_normals3d(
	real_vector3d const *a,
	real_vector3d const *b)
{
	real result = 0.f;

	if ((*(long *)&a->i) == (*(long *)&b->i)
		&& (*(long *)&a->j) == (*(long *)&b->j)
		&& (*(long *)&a->k) == (*(long *)&b->k))
	{
		return result;
	}

	result = PIN(dot_product3d(a, b), -1.f, 1.f);
	return arccosine(result);
}

real_vector2d *perpendicular2d(
	real_vector2d const *a,
	real_vector2d *result)
{
	result->i = -a->j;
	result->j = a->i;
	return result;
}

real_vector3d *perpendicular3d(
	real_vector3d const *a,
	real_vector3d *result)
{
	real i = fabs(a->i);
	real j = fabs(a->j);
	real k = fabs(a->k);

	if (i <= j && i <= k)
	{
		result->i = 0.f;
		result->j = a->k;
		result->k = -a->j;
	}
	else
	{
		if (j <= k)
		{
			result->i = -a->k;
			result->j = 0.f;
			result->k = a->i;
		}
		else
		{
			result->i = a->j;
			result->j = -a->i;
			result->k = 0.f;
		}
	}

	return result;
}

real_vector4d *perpendicular4d(
	real_vector4d const *a,
	real_vector4d *result)
{
	result->i = a->k;
	result->j = a->l;
	result->k = -a->i;
	result->l = -a->j;
	return result;
}

void yaw_vectors(
	real_vector3d *forward,
	real_vector3d const  *up,
	real sine,
	real cosine)
{
	real_vector3d cross;
	cross_product3d(up, forward, &cross);

	forward->i = forward->i*cosine + cross.i*sine;
	forward->j = forward->j*cosine + cross.j*sine;
	forward->k = forward->k*cosine + cross.k*sine;
	return;
}

void pitch_vectors(
	real_vector3d *forward,
	real_vector3d *up,
	real sine,
	real cosine)
{
	real_vector3d backward;
	negate_vector3d(forward, &backward);

	forward->i = forward->i*cosine + up->i*sine;
	forward->j = forward->j*cosine + up->j*sine;
	forward->k = forward->k*cosine + up->k*sine;

	up->i = up->i*cosine + backward.i*sine;
	up->j = up->j*cosine + backward.j*sine;
	up->k = up->k*cosine + backward.k*sine;
	return;
}

real_vector3d *rotate_vector_about_axis(
	real_vector3d *v,
	real_vector3d const *n,
	real sine,
	real cosine)
{
	real one_minus_cosine_times_v_dot_n = (n->i*v->i + n->j*v->j + n->k*v->k) * (1-cosine);

	real v_cross_n_i = n->k*v->j - v->k*n->j;
	real v_cross_n_j = n->i*v->k - v->i*n->k;
	real v_cross_n_k = n->j*v->i - v->j*n->i;

	v->i = cosine*v->i + one_minus_cosine_times_v_dot_n*n->i - sine*v_cross_n_i;
	v->j = cosine*v->j + one_minus_cosine_times_v_dot_n*n->j - sine*v_cross_n_j;
	v->k = cosine*v->k + one_minus_cosine_times_v_dot_n*n->k - sine*v_cross_n_k;

	return v;
}

void fast_normals_interpolate(
	real_vector3d const *a,
	real_vector3d const *b,
	real t,
	real_vector3d *result)
{
	real one_minus_t = 1-t;
	result->i = t*b->i + one_minus_t * a->i;
	result->j = t*b->j + one_minus_t * a->j;
	result->k = t*b->k + one_minus_t * a->k;
	fast_normalize3d(result);
	return;
}

void normals_interpolate(
	real_vector3d const *a,
	real_vector3d const *b,
	real t,
	real_vector3d *result)
{
	real angle;
	real_vector3d c;

	angle = angle_between_vectors3d(a, b);

	if (t <= 0.5f)
	{
		angle *= t;
		cross_product3d(a, b, &c);
		*result = *a;
	}
	else
	{
		angle *= (1-t);
		cross_product3d(b, a, &c);
		*result = *b;
	}

	if (normalize3d(&c) != 0.0f)
	{
		rotate_vector_about_axis(result, &c, sine(angle), cosine(angle));
	}

	return;
}

void vectors_interpolate(
	real_vector3d const *a,
	real_vector3d const *b,
	real t,
	real_vector3d *result)
{
	real one_minus_t = 1-t;
	result->i = one_minus_t*a->i + t*b->i;
	result->j = one_minus_t*a->j + t*b->j;
	result->k = one_minus_t*a->k + t*b->k;
	return;
}

void points_interpolate(
	real_point3d const *a,
	real_point3d const *b,
	real t,
	real_point3d *result)
{
	real one_minus_t = 1-t;
	result->x = one_minus_t*a->x + t*b->x;
	result->y = one_minus_t*a->y + t*b->y;
	result->z = one_minus_t*a->z + t*b->z;
	return;
}

void scalars_interpolate(
	real a,
	real b,
	real t,
	real *result)
{
	*result = (1-t) * a+b * t;
	return;
}

void scalars_interpolate_and_clamp_0_to_1(
	real a,
	real b,
	real t,
	real *result)
{
	*result = PIN((1.f-t)*a + t*b, 0.f, 1.f);
	return;
}

boolean pin_normal_to_cone3d(
	real_vector3d const *normal,
	real_vector3d const *direction,
	real sine,
	real cosine,
	real_vector3d *result)
{
	boolean pinned;

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\math\\real_math.c", 515, normal);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\math\\real_math.c", 516, direction);

	if (dot_product3d(normal, direction)>=cosine)
	{
		*result = *normal;
		pinned = FALSE;
	}
	else
	{
		real_vector3d axis;

		cross_product3d(direction, normal, &axis);

		if (normalize3d(&axis)==0.f)
		{
			perpendicular3d(direction, &axis);
			normalize3d(&axis);
		}

		*result = *direction;
		rotate_vector_about_axis(result, &axis, sine, cosine);
		pinned = TRUE;
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\math\\real_math.c", 540, result);

	return pinned;
}

real_vector3d *reflect_vector3d(
	real_vector3d const *incident,
	real_vector3d const *normal,
	real_vector3d *reflection)
{
	real incident_dot_normal = dot_product3d(incident, normal);
	real twice_incident_dot_normal = incident_dot_normal + incident_dot_normal;
	reflection->i = incident->i - normal->i*twice_incident_dot_normal;
	reflection->j = incident->j - normal->j*twice_incident_dot_normal;
	reflection->k = incident->k - normal->k*twice_incident_dot_normal;
	return reflection;
}

real_vector3d *refract_vector3d(
	real_vector3d const *incident,
	real_vector3d const *normal,
	real coefficient_of_refraction,
	real_vector3d *refraction)
{
	real refraction_scale;

	real incident_magnitude = magnitude3d(incident);
	real one_over_incident_magnitude = 1.f / incident_magnitude;

	real sine_incident = cross_product_magnitude3d(incident, normal) * one_over_incident_magnitude;
	real cosine_incident = dot_product3d(incident, normal) * one_over_incident_magnitude;
	real sine_refraction = coefficient_of_refraction * sine_incident;

	real cosine_refraction = square_root(1.f - sine_refraction * sine_refraction);
	
	real delta = (sine_incident * cosine_refraction + cosine_incident * sine_refraction) / sine_refraction;

	refraction->i = delta * normal->i + incident->i;
	refraction->j = delta * normal->j + incident->j;
	refraction->k = delta * normal->k + incident->k;

	refraction_scale = incident_magnitude / magnitude3d(refraction);
	
	refraction->i *= refraction_scale;
	refraction->j *= refraction_scale;
	refraction->k *= refraction_scale;

	return refraction;
}

void component_vectors_from_normal3d(
	real_vector3d const *vector,
	real_vector3d const *normal,
	real_vector3d *parallel,
	real_vector3d *perpendicular)
{
	real_vector3d storage;
	real vn = vector->i*normal->i + vector->j*normal->j + vector->k*normal->k;

	if (parallel== NULL)
	{
		parallel = &storage;
	}

	parallel->i = vn*normal->i;
	parallel->j = vn*normal->j;
	parallel->k = vn*normal->k;

	if (perpendicular!= NULL)
	{
		perpendicular->i = vector->i-parallel->i;
		perpendicular->j = vector->j-parallel->j;
		perpendicular->k = vector->k-parallel->k;
	}
	return;
}

void component_vectors_from_direction3d(
	real_vector3d const *vector,
	real_vector3d const *direction,
	real_vector3d *parallel,
	real_vector3d *perpendicular)
{
	real dd = direction->i*direction->i + direction->j*direction->j + direction->k*direction->k;
	if (dd != 0.f)
	{
		real vd_dd = (vector->i*direction->i + vector->j*direction->j + vector->k*direction->k) / dd;
		parallel->i = vd_dd*direction->i;
		parallel->j = vd_dd*direction->j;
		parallel->k = vd_dd*direction->k;
		perpendicular->i = vector->i-parallel->i;
		perpendicular->j = vector->j-parallel->j;
		perpendicular->k = vector->k-parallel->k;
	}
	else
	{
		parallel->i = 0.f;
		parallel->j = 0.f;
		parallel->k = 0.f;
		*perpendicular = *vector;
	}
	return;
}

void quaternion_normalize(
	real_quaternion *q)
{
	real magnitude_squared = q->v.i*q->v.i + q->v.j*q->v.j + q->v.k*q->v.k + q->w*q->w;
	if (magnitude_squared > 0.f)
	{
		real one_over_magnitude = reciprocal_square_root(magnitude_squared);
		set_real_quaternion(q, one_over_magnitude*q->v.i, one_over_magnitude*q->v.j, one_over_magnitude*q->v.k, one_over_magnitude*q->w);
	}
	else
	{
		set_real_quaternion(q, 0.f, 0.f, 0.f, 1.f);
	}
	return;
}

void quaternion_from_angle_and_vector(
	real_quaternion *q,
	real a,
	real_vector3d const *v)
{
	real half_angle = a * 0.5f;
	real sine_half_angle = sine(half_angle);

	q->w = cosine(half_angle);
	q->v.i = sine_half_angle*v->i;
	q->v.j = sine_half_angle*v->j;
	q->v.k = sine_half_angle*v->k;

	return;
}

void quaternion_to_angle_and_vector(
	real_quaternion const *q,
	real *a,
	real_vector3d *v)
{
	*v = q->v;
	*a = 2 * arctangent(normalize3d(v), q->w) ;
	
	if (*a > _pi)
	{
		negate_vector3d(v, v);
		*a = _pi * 2.0f - *a;
	}

	return;
}

void quaternions_multiply(
	real_quaternion const *q0,
	real_quaternion const *q1,
	real_quaternion *result)
{
	real_quaternion temporary;

	if (q0== result)
	{
		temporary = *q0;
		q0= &temporary;
	}

	if (q1== result)
	{
		temporary = *q1;
		q1= &temporary;
	}

	result->v.i =
					+ q0->v.j*q1->v.k
					+ q0->v.i*q1->w
					+ q0->w  *q1->v.i 
					- q0->v.k*q1->v.j
					;

	result->v.j =
					+ q0->w  *q1->v.j 
					+ q0->v.j*q1->w
					+ q0->v.k*q1->v.i 
					- q0->v.i*q1->v.k
					;

	result->v.k =
					+ q0->v.k*q1->w 
					+ q0->v.i*q1->v.j
					+ q0->w  *q1->v.k 
					- q0->v.j*q1->v.i
					;

	result->w =
					+ q0->w  *q1->w 
					- q0->v.i*q1->v.i 
					- q0->v.j*q1->v.j 
					- q0->v.k*q1->v.k
					;

	return;
}

void quaternions_interpolate(
	real_quaternion const *q0,
	real_quaternion const *q1,
	real t,
	real_quaternion *result)
{
	real v = 1.f-t;
	
	if (q0->v.i*q1->v.i + q0->v.j*q1->v.j + q0->v.k*q1->v.k + q0->w*q1->w < 0.f)
	{
		t = -t;
	}

	result->v.i = q0->v.i*v + q1->v.i*t;
	result->v.j = q0->v.j*v + q1->v.j*t;
	result->v.k = q0->v.k*v + q1->v.k*t;
	result->w = q0->w*v + q1->w*t;
	return;
}

void quaternions_interpolate_and_normalize(
	real_quaternion const *q0,
	real_quaternion const *q1,
	real t,
	real_quaternion *result)
{
	quaternions_interpolate(q0, q1, t, result);
	quaternion_normalize(result);
	return;
}

void quaternion_transform_point(
	real_quaternion const *q,
	real_point3d const *p,
	real_point3d *result)
{

	real w_squared = q->w*q->w;
	real two_w_squared_minus_one = 2.f * w_squared - 1.f;
	real v_dot_p = (q->v.i*p->x + q->v.k*p->z + q->v.j*p->y);
	
	real two_v_dot_p = v_dot_p+v_dot_p;
	real two_w = q->w+q->w;

	real v_cross_r_i = (q->v.k*p->x) - (p->z*q->v.i);
	real v_cross_r_j = (q->v.i*p->y) - (q->v.j*p->x);
	real v_cross_r_k = (q->v.j*p->z) - (q->v.k*p->y);

	result->x = v_cross_r_k * two_w+two_v_dot_p*q->v.i + two_w_squared_minus_one*p->x;
	result->y = two_v_dot_p*q->v.j + two_w_squared_minus_one*p->y + v_cross_r_i*two_w;
	result->z = two_v_dot_p*q->v.k + two_w_squared_minus_one*p->z + v_cross_r_j*two_w;
	return;
}

void orientations_interpolate(
	real_orientation const *a,
	real_orientation const *b,
	real t,
	real_orientation *result)
{
	quaternions_interpolate(&a->rotation, &b->rotation, t, &result->rotation);
	quaternion_normalize(&result->rotation);

	result->translation.x = a->translation.x*(1.f - t) + b->translation.x*t;
	result->translation.y = a->translation.y*(1.f - t) + b->translation.y*t;
	result->translation.z = a->translation.z*(1.f - t) + b->translation.z*t;
	result->scale = a->scale*(1.f - t) + b->scale*t;
	return;
}

real_euler_angles2d *euler_angles2d_from_vector3d(
	real_euler_angles2d *angles,
	real_vector3d const *vector)
{
	angles->yaw = arctangent(vector->j, vector->i);
	angles->pitch = arctangent(vector->k, square_root(vector->i*vector->i+vector->j*vector->j));
	return angles;
}

void vectors3d_from_euler_angles3d(
	real_vector3d *forward,
	real_vector3d *up,
	real_euler_angles3d const *angles)
{
	real_matrix4x3 matrix;
	real_point3d dummy;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 832, forward);
	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 833, up);
	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 834, angles);

	matrix4x3_rotation_from_angles(&matrix, angles->yaw, angles->pitch, angles->roll);
	matrix4x3_to_point_and_vectors(&matrix, &dummy, forward, up);
	return;
}

real_vector3d *vector3d_from_euler_angles2d(
	real_vector3d *vector,
	real_euler_angles2d const *angles)
{
	real cosine_pitch = cosine(angles->pitch);
	vector->i = cosine(angles->yaw)* cosine_pitch;
	vector->j = sine(angles->yaw)* cosine_pitch;
	vector->k = sine(angles->pitch);
	return vector;
}

real_vector3d *vector3d_from_angle(
	real_vector3d *vector,
	real angle)
{
	vector->k = 0.f;
	vector->i = cosine(angle);
	vector->j = sine(angle);
	return vector;
}

boolean point_in_pill2d(
	real_point2d const *point,
	real_point2d const *base,
	real_vector2d const *height,
	real width)
{
	real_vector2d v;
	real height_magnitude_squared;
	real t;

	vector_from_points2d(base, point, &v);
	height_magnitude_squared = dot_product2d(&v, height) / magnitude_squared2d(height);
	t = -PIN(height_magnitude_squared, 0.f, 1.f);
	point_from_line2d((real_point2d *)&v, height, t, (real_point2d *)&v);

	return magnitude_squared2d(&v)<=width*width;
}

real point_to_line_distance_squared3d(
	real_point3d const *point,
	real_point3d const *base,
	real_vector3d const *height)
{
	real t;
	real distance_squared;
	real_vector3d v;
	
	vector_from_points3d(base, point, &v);
	t = PIN(dot_product3d(&v, height) / magnitude_squared3d(height), 0.f, 1.f);
	point_from_line3d((real_point3d *)&v, height, -t, (real_point3d *)&v);
	distance_squared = magnitude_squared3d(&v);

	return distance_squared;
}

real vector_to_line_distance_squared3d(
	real_point3d const *point,
	real_vector3d const *vector,
	real_point3d const *base,
	real_vector3d const *height)
{
	real_vector3d v;
	real_vector3d n;
	real nn;
	real t0;
	real t1;
	real_point3d p0;
	real_point3d p1;

	vector_from_points3d(point, base, &v);
	cross_product3d(vector, height, &n);
	nn = magnitude_squared3d(&n);

	if (!realcmp(nn, 0.f))
	{
		boolean pin_t0 = FALSE;
		boolean pin_t1 = FALSE;

		scale_vector3d(&n, 1.f/nn, &n);
		t0 = triple_product3d(&v, height, &n);
		t1 = triple_product3d(&v, vector, &n);
		pin_t0 = t0<0.f || t0>1.f;
		pin_t1 = t1<0.f || t1>1.f;

		if (pin_t0 || pin_t1)
		{
			real d0 = REAL_MAX;
			real d1 = REAL_MAX;

			if (pin_t0)
			{
				point_from_line3d(point, vector, t0<0.f ? 0.f : 1.f, &p0);
				d0 = point_to_line_distance_squared3d(&p0, base, height);
			}

			if (pin_t1)
			{
				point_from_line3d(base, height, t1<0.f ? 0.f : 1.f, &p1);
				d1 = point_to_line_distance_squared3d(&p1, point, vector);
			}

			match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 942, (d0 < REAL_MAX) || (d1 < REAL_MAX));

			return MIN(d0, d1);
		}
	}
	else
	{
		real vh = dot_product3d(vector, height);

		{
			real vv = magnitude_squared3d(vector);

			if (vv>_real_epsilon)
			{
				real oovv = 1.f/vv;
				real tmin = dot_product3d(&v, vector)*oovv;
				real tmax = vh*oovv + tmin;

				t0 = (PIN(tmin, 0.f, 1.f) + PIN(tmax, 0.f, 1.f))*0.5f;
			}
			else
			{
				t0 = 0.f;
			}
		}

		{
			real hh = magnitude_squared3d(height);

			if (hh>_real_epsilon)
			{
				real oohh = 1.f/hh;
				real tmin = -dot_product3d(&v, height)*oohh;
				real tmax = vh*oohh + tmin;

				t1 = (PIN(tmin, 0.f, 1.f) + PIN(tmax, 0.f, 1.f))*0.5f;
			}
			else
			{
				t1 = 0.f;
			}
		}
	}

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 986, (t0 >= 0.0f) && (t0 <= 1.0f));
	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 987, (t1 >= 0.0f) && (t1 <= 1.0f));

	point_from_line3d(point, vector, t0, &p0);
	point_from_line3d(base, height, t1, &p1);

	return distance_squared3d(&p0, &p1);
}

boolean sphere_test_vector3d(
	real_point3d const *center,
	real radius,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t,
	real_vector3d *normal)
{
	real_vector3d x;
	real xv;

	vector_from_points3d(center, point, &x);
	xv = dot_product3d(&x, vector);

	if (xv < 0)
	{
		real xx = magnitude_squared3d(&x);
		real xx_rr = xx - radius*radius;

		if (xx_rr <= 0)
		{
			*t = 0;
			scale_vector3d(&x, reciprocal_square_root(xx), normal);
			return TRUE;
		}
		else
		{
			real d = vector->i*vector->i + vector->j*vector->j + vector->k*vector->k;
			real vv = xv*xv - d*xx_rr;

			if (vv >= 0)
			{
				real n = -(square_root(vv) + xv);

				if (n <= d)
				{
					*t = n/d;
					fast_normalize3d((real_vector3d *)point_from_line3d((real_point3d *)&x, vector, *t, (real_point3d *)normal));
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

boolean pill_test_vector3d(
	real_point3d const *base,
	real height,
	real width,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t_reference,
	real_vector3d *normal)
{
	real t;
	real_vector2d x;
	real xv;
	real xx_rr;
	boolean hit = FALSE;

	vector_from_points2d((real_point2d const *)base, (real_point2d const *)point, &x);
	xv = dot_product2d(&x, (real_vector2d const *)vector);
	xx_rr = magnitude_squared2d(&x) - width*width;

	if (xx_rr<=0.f)
	{
		t = 0.f;
		hit = TRUE;
	}
	else
	{
		real vv = magnitude_squared2d((real_vector2d const *)vector);
		real d = xv*xv - vv*xx_rr;

		if (d>=0.f)
		{
			real n = -(square_root(d) + xv);

			if (n<=vv)
			{
				t = n/vv;
				hit = TRUE;
			}
		}
	}

	if (hit)
	{
		real u = (point->z - base->z + t*vector->k) / (height*height);

		if (u<0.f)
		{
			hit = sphere_test_vector3d(base, width, point, vector, t_reference, normal);
		}
		else if (u>1.f)
		{
			real_point3d p;

			set_real_point3d(&p, base->x, base->y, base->z + height);
			hit = sphere_test_vector3d(&p, width, point, vector, t_reference, normal);
		}
		else if (xv<0.f)
		{
			*t_reference = t;
			normal->i = t*vector->i + x.i;
			normal->j = t*vector->j + x.j;
			fast_normalize2d((real_vector2d *)normal);
			normal->k = 0.f;
		}
		else
		{
			hit = FALSE;
		}
	}

	if (hit)
	{
		if (dot_product3d(vector, normal)>0.f)
		{
			hit = FALSE;
		}
	}

	return hit;
}

boolean fast_vector_intersects_sphere(
	real_point3d const *point,
	real_vector3d const *vector,
	real_point3d const *center,
	real radius)
{
	real_point3d p = {point->x-center->x, point->y-center->y, point->z - center->z};
	real c = (p.x*p.x) + (p.y*p.y) + (p.z*p.z) - (radius*radius);

	if (c < 0.f)
	{
		return TRUE;
	}
	else
	{
		real_vector3d v = *vector;
		real b = p.x*v.i + p.y*v.j + p.z*v.k;
		
		if (b >= 0.f)
		{
			return FALSE;
		}
		else
		{
			real a = v.i*v.i + v.j*v.j + v.k*v.k;
			real disc = b * b - a * c;
			
			if (disc <= 0.f)
			{
				return FALSE;
			}
			else
			{
				real neg_a_minus_b = -a - b;

				if (neg_a_minus_b < 0.f)
				{
					return TRUE;
				}
				else
				{
					return neg_a_minus_b * neg_a_minus_b < disc;
				}
			}
		}
	}
}

real fast_vector_intersection_with_sphere(
	real_point3d const *point,
	real_vector3d const *vector,
	real_point3d const *center,
	real radius)
{
	real_point3d p = {point->x-center->x, point->y-center->y, point->z - center->z};
	real c = (p.x*p.x) + (p.y*p.y) + (p.z*p.z) - (radius*radius);

	if (c < 0.f)
	{
		return 0.f;
	}
	else
	{
		real_vector3d v = *vector;
		real b_over_2 = p.x*v.i + p.y*v.j + p.z*v.k;

		if (b_over_2 >= 0.f)
		{
			return REAL_MAX;
		}
		else
		{
			real a = v.i*v.i + v.j*v.j + v.k*v.k;
			real disc_over_4 = b_over_2*b_over_2 - a*c;

			if (disc_over_4 <= 0.f)
			{
				return REAL_MAX;
			}
			else
			{
				real t = (-b_over_2 - square_root(disc_over_4)) / a;
				return t;
			}
		}
	}
}

boolean point_in_rectangle2d(
	real_point2d const *point,
	real_rectangle2d const *bounds)
{
	return point->x >= bounds->x0
		&& point->x <= bounds->x1
		&& point->y >= bounds->y0
		&& point->y <= bounds->y1;
}

boolean point_in_rectangle3d(
	real_point3d const *point,
	real_rectangle3d const *bounds)
{
	return point->x >= bounds->x0
		&& point->x <= bounds->x1
		&& point->y >= bounds->y0
		&& point->y <= bounds->y1
		&& point->z >= bounds->z0
		&& point->z <= bounds->z1;
}

boolean point_in_triangle2d(
	real_point2d const *point,
	real_point2d const *p0,
	real_point2d const *p1,
	real_point2d const *p2,
	real *s,
	real *t)
{
	real_vector2d u;
	real_vector2d p;

	vector_from_points2d(p0, p1, &u);
	vector_from_points2d(p0, point, &p);
	{
		real up = cross_product2d(&u, &p);

		if (up>=0.f)
		{
			real_vector2d v;
			real pv;

			vector_from_points2d(p0, p2, &v);
			pv = cross_product2d(&p, &v);

			if (pv>=0.f)
			{
				real uv = cross_product2d(&u, &v);

				if (up+pv<=uv)
				{
					real oouv = 1.f/uv;
					*s = pv*oouv;
					*t = up*oouv;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

boolean point_in_triangle3d(
	real_point3d const *point,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real *s,
	real *t)
{
	real_vector3d x;
	real_vector3d e0;
	real_vector3d e1;
	real_vector3d n;
	real xn;

	vector_from_points3d(p0, point, &x);
	vector_from_points3d(p0, p1, &e0);
	vector_from_points3d(p0, p2, &e1);
	cross_product3d(&e0, &e1, &n);
	xn = dot_product3d(&x, &n);

	if (xn*xn<magnitude_squared3d(&n)*_real_epsilon)
	{
		short projection_axis = projection_from_vector3d(&n);
		boolean projection_sign = projection_sign_from_vector3d(&n, projection_axis);
		real_vector2d u;
		real_vector2d p;
		real up;

		project_point3d((real_point3d *)&e0, projection_axis, projection_sign, (real_point2d *)&u);
		project_point3d((real_point3d *)&x, projection_axis, projection_sign, (real_point2d *)&p);
		up = cross_product2d(&u, &p);

		if (up>=0.f)
		{
			real_vector2d v;
			real pv;

			project_point3d((real_point3d *)&e1, projection_axis, projection_sign, (real_point2d *)&v);
			pv = cross_product2d(&p, &v);

			if (pv>=0.f)
			{
				real uv = cross_product2d(&u, &v);

				if (up+pv<=uv)
				{
					real oouv = 1.f/uv;
					*s = pv*oouv;
					*t = up*oouv;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

boolean point_in_cone2d(
	real_point2d const *point,
	real_point2d const *base,
	real_vector2d const *direction,
	real distance,
	real cosine)
{
	real_vector2d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 1375, cosine>=0.0f);

	vector_from_points2d(base, point, &v);
	{
		real vd = dot_product2d(&v, direction);

		if (vd>=0.f && vd<=distance)
		{
			real vv = magnitude_squared2d(&v);

			if (vd*vd>=vv*cosine*cosine)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean point_in_cone3d(
	real_point3d const *point,
	real_point3d const *base,
	real_vector3d const *direction,
	real distance,
	real cosine)
{
	real_vector3d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 1404, cosine>=0.0f);

	vector_from_points3d(base, point, &v);
	{
		real vd = dot_product3d(&v, direction);

		if (vd>=0.f && vd<=distance)
		{
			real vv = magnitude_squared3d(&v);

			if (vd*vd>=vv*cosine*cosine)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean point_in_sector2d(
	real_point2d const *point,
	real_point2d const *base,
	real_vector2d const *direction,
	real distance,
	real cosine)
{
	real_vector2d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 1433, cosine>=0.0f);

	vector_from_points2d(base, point, &v);
	{
		real vv = magnitude_squared2d(&v);

		if (distance*distance>=vv)
		{
			real vd = dot_product2d(&v, direction);

			if (vd>=0.f && vd*vd>=vv*cosine*cosine)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean point_in_sector3d(
	real_point3d const *point,
	real_point3d const *base,
	real_vector3d const *direction,
	real distance,
	real cosine)
{
	real_vector3d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 1462, cosine>=0.0f);

	vector_from_points3d(base, point, &v);
	{
		real vv = magnitude_squared3d(&v);

		if (distance*distance>=vv)
		{
			real vd = dot_product3d(&v, direction);

			if (vd>=0.f && vd*vd>=vv*cosine*cosine)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean vector_intersects_pill2d(
	real_point2d const *point,
	real_vector2d const *vector,
	real_point2d const *base,
	real_vector2d const *height,
	real width)
{
	real_vector2d v;
	real n;
	real t0;
	real t1;
	real_point2d p0;
	real_point2d p1;

	vector_from_points2d(point, base, &v);
	n = cross_product2d(vector, height);

	if (!realcmp(n, 0.f))
	{
		real oon = 1.f/n;
		boolean pin_t0;
		boolean pin_t1;

		t0 = cross_product2d(&v, height)*oon;
		t1 = cross_product2d(&v, vector)*oon;
		pin_t0 = t0<0.f || t0>1.f;
		pin_t1 = t1<0.f || t1>1.f;

		if (!pin_t0 && !pin_t1)
		{
			return TRUE;
		}

		if (pin_t0)
		{
			point_from_line2d(point, vector, t0<0.f ? 0.f : 1.f, &p0);
		}

		if (pin_t1)
		{
			point_from_line2d(base, height, t1<0.f ? 0.f : 1.f, &p1);
		}

		return (pin_t0 && point_in_pill2d(&p0, base, height, width)) || (pin_t1 && point_in_pill2d(&p1, point, vector, width));
	}
	else
	{
		real vh = dot_product2d(vector, height);

		{
			real vv = magnitude_squared2d(vector);

			if (vv>_real_epsilon)
			{
				real oovv = 1.f/vv;
				real tmin = dot_product2d(&v, vector)*oovv;
				real tmax = vh*oovv + tmin;

				t0 = (PIN(tmin, 0.f, 1.f) + PIN(tmax, 0.f, 1.f))*0.5f;
			}
			else
			{
				t0 = 0.f;
			}
		}

		{
			real hh = magnitude_squared2d(height);

			if (hh>_real_epsilon)
			{
				real oohh = 1.f/hh;
				real tmin = -dot_product2d(&v, height)*oohh;
				real tmax = vh*oohh + tmin;

				t1 = (PIN(tmin, 0.f, 1.f) + PIN(tmax, 0.f, 1.f))*0.5f;
			}
			else
			{
				t1 = 0.f;
			}
		}
	}

	point_from_line2d(point, vector, t0, &p0);
	point_from_line2d(base, height, t1, &p1);

	return distance_squared2d(&p0, &p1)<=width*width;
}

boolean vector_intersects_pill3d(
	real_point3d const *point,
	real_vector3d const *vector,
	real_point3d const *base,
	real_vector3d const *height,
	real width)
{
	real_vector3d v;
	real_vector3d n;
	real nn;
	real t0;
	real t1;
	real_point3d p0;
	real_point3d p1;

	vector_from_points3d(point, base, &v);
	cross_product3d(vector, height, &n);
	nn = magnitude_squared3d(&n);

	if (!realcmp(nn, 0.f))
	{
		boolean pin_t0 = FALSE;
		boolean pin_t1 = FALSE;

		scale_vector3d(&n, 1.f/nn, &n);
		t0 = triple_product3d(&v, height, &n);
		t1 = triple_product3d(&v, vector, &n);
		pin_t0 = t0<0.f || t0>1.f;
		pin_t1 = t1<0.f || t1>1.f;

		if (pin_t0 || pin_t1)
		{
			if (pin_t0)
			{
				point_from_line3d(point, vector, t0<0.f ? 0.f : 1.f, &p0);
			}

			if (pin_t1)
			{
				point_from_line3d(base, height, t1<0.f ? 0.f : 1.f, &p1);
			}

			return (pin_t0 && fast_vector_intersects_sphere(base, height, &p0, width)) || (pin_t1 && fast_vector_intersects_sphere(point, vector, &p1, width));
		}
	}
	else
	{
		real vh = dot_product3d(vector, height);

		{
			real vv = magnitude_squared3d(vector);

			if (vv>_real_epsilon)
			{
				real oovv = 1.f/vv;
				real tmin = dot_product3d(&v, vector)*oovv;
				real tmax = vh*oovv + tmin;

				t0 = (PIN(tmin, 0.f, 1.f) + PIN(tmax, 0.f, 1.f))*0.5f;
			}
			else
			{
				t0 = 0.f;
			}
		}

		{
			real hh = magnitude_squared3d(height);

			if (hh>_real_epsilon)
			{
				real oohh = 1.f/hh;
				real tmin = -dot_product3d(&v, height)*oohh;
				real tmax = vh*oohh + tmin;

				t1 = (PIN(tmin, 0.f, 1.f) + PIN(tmax, 0.f, 1.f))*0.5f;
			}
			else
			{
				t1 = 0.f;
			}
		}
	}

	point_from_line3d(point, vector, t0, &p0);
	point_from_line3d(base, height, t1, &p1);

	return distance_squared3d(&p0, &p1)<=width*width;
}

boolean vector_intersects_rectangle2d(
	real_point2d const *point,
	real_vector2d const *vector,
	real_rectangle2d const *bounds)
{
	real t_in = -FLT_MAX;
	real t_out = FLT_MAX;

	if (!realcmp(vector->i, 0.f))
	{
		real ooi = 1.f / vector->i;
		real t0 = (bounds->x0-point->x) * ooi;
		real t1 = (bounds->x1-point->x) * ooi;

		if (vector->i > 0.f)
		{
			if (t_in < t0) t_in = t0;
			if (t_out > t1) t_out = t1;
		}
		else
		{
			if (t_in < t1) t_in = t1;
			if (t_out > t0) t_out = t0;
		}

		if (t_in > t_out)
		{
			return FALSE;
		}
	}
	else
	{
		if (point->x < bounds->x0 || point->x > bounds->x1)
		{
			return FALSE;
		}
	}

	if (!realcmp(vector->j, 0.f))
	{
		real ooj = 1.f  / vector->j;
		real t0 = (bounds->y0-point->y) * ooj;
		real t1 = (bounds->y1-point->y) * ooj;

		if (vector->j > 0.f)
		{
			if (t_in < t0) t_in = t0;
			if (t_out > t1) t_out = t1;
		}
		else
		{
			if (t_in < t1) t_in = t1;
			if (t_out > t0) t_out = t0;
		}

		if (t_in > t_out)
		{
			return FALSE;
		}
	}
	else
	{
		if (point->y < bounds->y0 || point->y > bounds->y1)
		{
			return FALSE;
		}
	}

	return t_out >= 0.f && t_in <= 1.f;
}

boolean vector_intersects_rectangle3d(
	real_point3d const *point,
	real_vector3d const *vector,
	real_rectangle3d const *bounds)
{
	real t_in = -FLT_MAX;
	real t_out = FLT_MAX;

	if (!realcmp(vector->i, 0.f))
	{
		real ooi = 1.f / vector->i;
		real t0 = (bounds->x0-point->x) * ooi;
		real t1 = (bounds->x1-point->x) * ooi;

		if (vector->i > 0.f)
		{
			if (t_in < t0) t_in = t0;
			if (t_out > t1) t_out = t1;
		}
		else
		{
			if (t_in < t1) t_in = t1;
			if (t_out > t0) t_out = t0;
		}

		if (t_in > t_out)
		{
			return FALSE;
		}
	}
	else
	{
		if (point->x < bounds->x0 || point->x > bounds->x1)
			return FALSE;
	}

	if (!realcmp(vector->j, 0.f))
	{
		real ooj = 1.f / vector->j;
		real t0 = (bounds->y0-point->y) * ooj;
		real t1 = (bounds->y1-point->y) * ooj;

		if (vector->j > 0.f)
		{
			if (t_in < t0) t_in = t0;
			if (t_out > t1) t_out = t1;
		}
		else
		{
			if (t_in < t1) t_in = t1;
			if (t_out > t0) t_out = t0;
		}

		if (t_in > t_out)
		{
			return FALSE;
		}
	}
	else
	{
		if (point->y < bounds->y0 || point->y > bounds->y1)
		{
			return FALSE;
		}
	}

	if (!realcmp(vector->k, 0.f))
	{
		real ook = 1 / vector->k;
		real t0 = (bounds->z0-point->z) * ook;
		real t1 = (bounds->z1-point->z) * ook;

		if (vector->k > 0.f)
		{
			if (t_in < t0) t_in = t0;
			if (t_out > t1) t_out = t1;
		}
		else
		{
			if (t_in < t1) t_in = t1;
			if (t_out > t0) t_out = t0;
		}

		if (t_in > t_out)
		{
			return FALSE;
		}
	}
	else
	{
		if (point->z < bounds->z0 || point->z > bounds->z1)
		{
			return FALSE;
		}
	}

	return t_out >= 0.f && t_in <= 1.f;
}

boolean vector_intersects_triangle2d(
	real_point2d const *point,
	real_vector2d const *vector,
	real_point2d const *p0,
	real_point2d const *p1,
	real_point2d const *p2)
{
	real_vector2d v;
	real_vector2d e;
	real c;
	real x;
	real t_in = 0.f;
	real t_out = 1.f;

	vector_from_points2d(p0, point, &v);
	vector_from_points2d(p0, p1, &e);
	c = cross_product2d(&v, &e);
	x = cross_product2d(&e, vector);

	if (!realcmp(x, 0.f))
	{
		real t = c/x;

		if (x>0.f)
		{
			if (t_in<t)
			{
				t_in = t;
			}
		}
		else
		{
			if (t_out>t)
			{
				t_out = t;
			}
		}

		if (t_in>t_out)
		{
			return FALSE;
		}
	}
	else if (c>0.f)
	{
		return FALSE;
	}

	vector_from_points2d(p1, point, &v);
	vector_from_points2d(p1, p2, &e);
	c = cross_product2d(&v, &e);
	x = cross_product2d(&e, vector);

	if (!realcmp(x, 0.f))
	{
		real t = c/x;

		if (x>0.f)
		{
			if (t_in<t)
			{
				t_in = t;
			}
		}
		else
		{
			if (t_out>t)
			{
				t_out = t;
			}
		}

		if (t_in>t_out)
		{
			return FALSE;
		}
	}
	else if (c>0.f)
	{
		return FALSE;
	}

	vector_from_points2d(p2, point, &v);
	vector_from_points2d(p2, p0, &e);
	c = cross_product2d(&v, &e);
	x = cross_product2d(&e, vector);

	if (!realcmp(x, 0.f))
	{
		real t = c/x;

		if (x>0.f)
		{
			if (t_in<t)
			{
				t_in = t;
			}
		}
		else
		{
			if (t_out>t)
			{
				t_out = t;
			}
		}

		if (t_in>t_out)
		{
			return FALSE;
		}
	}
	else if (c>0.f)
	{
		return FALSE;
	}

	return TRUE;
}

boolean vector_intersects_triangle3d(
	real_point3d const *point,
	real_vector3d const *vector,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real *t_reference)
{
	real_vector3d e0;
	real_vector3d e1;
	real_vector3d n;
	real d;
	boolean hit = FALSE;

	vector_from_points3d(p0, p1, &e0);
	vector_from_points3d(p0, p2, &e1);
	cross_product3d(&e0, &e1, &n);
	d = dot_product3d(vector, &n);

	if (!realcmp(d, 0.f))
	{
		real ood = 1.f/d;
		real_vector3d x;
		real t;

		vector_from_points3d(point, p0, &x);
		t = dot_product3d(&x, &n)*ood;

		if (t>=0.f && t<=1.f)
		{
			real_vector3d w;
			real u;

			cross_product3d(&x, vector, &w);
			u = dot_product3d(&e1, &w)*ood;

			if (u>=0.f && u<=1.f)
			{
				real v = -dot_product3d(&e0, &w)*ood;

				if (v>=0.f && u+v<=1.f)
				{
					*t_reference = t;
					hit = TRUE;
				}
			}
		}
	}

	return hit;
}

boolean circle_intersects_rectangle2d(
	real_point2d const *center,
	real radius,
	real_rectangle2d const *bounds)
{
	real_vector2d v;

	v.i = center->x<=bounds->x1 ? (center->x<bounds->x0 ? bounds->x0-center->x : 0.f) : center->x-bounds->x1;
	v.j = center->y<=bounds->y1 ? (center->y<bounds->y0 ? bounds->y0-center->y : 0.f) : center->y-bounds->y1;

	return magnitude_squared2d(&v)<=radius*radius;
}

boolean sphere_intersects_rectangle3d(
	real_point3d const *center,
	real radius,
	real_rectangle3d const *bounds)
{
	real_vector3d v;

	v.i = center->x<=bounds->x1 ? (center->x<bounds->x0 ? bounds->x0-center->x : 0.f) : center->x-bounds->x1;
	v.j = center->y<=bounds->y1 ? (center->y<bounds->y0 ? bounds->y0-center->y : 0.f) : center->y-bounds->y1;
	v.k = center->z<=bounds->z1 ? (center->z<bounds->z0 ? bounds->z0-center->z : 0.f) : center->z-bounds->z1;

	return magnitude_squared3d(&v)<radius*radius;
}

boolean circle_intersects_triangle2d(
	real_point2d const *center,
	real radius,
	real_point2d const *p0,
	real_point2d const *p1,
	real_point2d const *p2)
{
	real_vector2d v;
	real_vector2d e;
	boolean in = TRUE;

	vector_from_points2d(p0, center, &v);
	vector_from_points2d(p0, p1, &e);

	if (cross_product2d(&v, &e)>0.f)
	{
		if (point_in_pill2d(center, p0, &e, radius))
		{
			return TRUE;
		}

		in = FALSE;
	}

	vector_from_points2d(p1, center, &v);
	vector_from_points2d(p1, p2, &e);

	if (cross_product2d(&v, &e)<0.f)
	{
		if (point_in_pill2d(center, p1, &e, radius))
		{
			return TRUE;
		}

		in = FALSE;
	}

	vector_from_points2d(p2, center, &v);
	vector_from_points2d(p2, p0, &e);

	if (cross_product2d(&v, &e)>0.f)
	{
		if (point_in_pill2d(center, p2, &e, radius))
		{
			return TRUE;
		}

		in = FALSE;
	}

	return in;
}

boolean sphere_intersects_triangle3d(
	real_point3d const *center,
	real radius,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2)
{
	real_vector3d v;
	real_vector3d e0;
	real_vector3d e1;
	real_vector3d e2;
	real_vector3d n;
	real vn;
	boolean in = TRUE;

	vector_from_points3d(p0, center, &v);
	vector_from_points3d(p0, p1, &e0);
	vector_from_points3d(p1, p2, &e1);
	cross_product3d(&e0, &e1, &n);
	vn = dot_product3d(&v, &n);

	if (vn*vn>magnitude_squared3d(&n)*radius*radius)
	{
		return FALSE;
	}

	if (triple_product3d(&v, &e0, &n)>0.f)
	{
		if (fast_vector_intersects_sphere(p0, &e0, center, radius))
		{
			return TRUE;
		}

		in = FALSE;
	}

	vector_from_points3d(p1, center, &v);

	if (triple_product3d(&v, &e1, &n)>0.f)
	{
		if (fast_vector_intersects_sphere(p1, &e1, center, radius))
		{
			return TRUE;
		}

		in = FALSE;
	}

	vector_from_points3d(p2, center, &v);
	vector_from_points3d(p2, p0, &e2);

	if (triple_product3d(&v, &e2, &n)<0.f)
	{
		if (fast_vector_intersects_sphere(p2, &e2, center, radius))
		{
			return TRUE;
		}

		in = FALSE;
	}

	return in;
}

boolean circle_intersects_cone2d(
	real_point2d const *center,
	real radius,
	real_point2d const *base,
	real_vector2d const *direction,
	real distance,
	real sine,
	real cosine)
{
	real_vector2d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 2167, sine>0.0f && cosine>=0.0f);
	match_assert_valid_real_sine_cosine("c:\\halo\\SOURCE\\math\\real_math.c", 2168, sine, cosine);

	vector_from_points2d(base, center, &v);
	{
		real vd = dot_product2d(&v, direction);

		if (-radius<=vd && distance+radius>=vd)
		{
			real vv = magnitude_squared2d(&v);

			if ((2.f*radius*sine + vd)*vd + radius*radius>=vv*cosine*cosine)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean sphere_intersects_cone3d(
	real_point3d const *center,
	real radius,
	real_point3d const *base,
	real_vector3d const *direction,
	real distance,
	real sine,
	real cosine)
{
	real_vector3d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 2199, sine>0.0f && cosine>=0.0f);
	match_assert_valid_real_sine_cosine("c:\\halo\\SOURCE\\math\\real_math.c", 2200, sine, cosine);

	vector_from_points3d(base, center, &v);
	{
		real vd = dot_product3d(&v, direction);

		if (-radius<=vd && distance+radius>=vd)
		{
			real vv = magnitude_squared3d(&v);

			if ((2.f*radius*sine + vd)*vd + radius*radius>=vv*cosine*cosine)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean circle_intersects_sector2d(
	real_point2d const *center,
	real radius,
	real_point2d const *base,
	real_vector2d const *direction,
	real distance,
	real sine,
	real cosine)
{
	real_vector2d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 2231, sine>0.0f && cosine>=0.0f);
	match_assert_valid_real_sine_cosine("c:\\halo\\SOURCE\\math\\real_math.c", 2232, sine, cosine);

	vector_from_points2d(base, center, &v);
	{
		real vd = dot_product2d(&v, direction);

		if (vd>=-radius && vd<=distance+radius)
		{
			real offset_radius = radius/sine;
			real_point2d offset_base;
			real offset_distance;

			point_from_line2d(base, direction, -offset_radius, &offset_base);
			offset_distance = offset_radius + distance + radius;

			if (point_in_sector2d(center, &offset_base, direction, offset_distance, cosine))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean sphere_intersects_sector3d(
	real_point3d const *center,
	real radius,
	real_point3d const *base,
	real_vector3d const *direction,
	real distance,
	real sine,
	real cosine)
{
	real_vector3d v;

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 2269, sine>0.0f && cosine>=0.0f);
	match_assert_valid_real_sine_cosine("c:\\halo\\SOURCE\\math\\real_math.c", 2270, sine, cosine);

	vector_from_points3d(base, center, &v);
	{
		real vd = dot_product3d(&v, direction);

		if (vd>=-radius && vd<=distance+radius)
		{
			real offset_radius = radius/sine;
			real_point3d offset_base;
			real offset_distance;

			point_from_line3d(base, direction, -offset_radius, &offset_base);
			offset_distance = offset_radius + distance + radius;

			if (point_in_sector3d(center, &offset_base, direction, offset_distance, cosine))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean pill_intersects_rectangle2d(
	real_point2d const *base,
	real_vector2d const *height,
	real width,
	real_rectangle2d const *bounds)
{
	real_point2d p;
	real_vector2d e;
	boolean in = TRUE;

	if (base->x<bounds->x0)
	{
		set_real_point2d(&p, bounds->x0, bounds->y0);
		set_real_vector2d(&e, 0.f, bounds->y1-bounds->y0);

		if (vector_intersects_pill2d(&p, &e, base, height, width))
		{
			return TRUE;
		}

		in = FALSE;
	}

	if (base->y<bounds->y0)
	{
		set_real_point2d(&p, bounds->x0, bounds->y0);
		set_real_vector2d(&e, bounds->x1-bounds->x0, 0.f);

		if (vector_intersects_pill2d(&p, &e, base, height, width))
		{
			return TRUE;
		}

		in = FALSE;
	}

	if (base->x>bounds->x1)
	{
		set_real_point2d(&p, bounds->x1, bounds->y0);
		set_real_vector2d(&e, 0.f, bounds->y1-bounds->y0);

		if (vector_intersects_pill2d(&p, &e, base, height, width))
		{
			return TRUE;
		}

		in = FALSE;
	}

	if (base->y>bounds->y1)
	{
		set_real_point2d(&p, bounds->x0, bounds->y1);
		set_real_vector2d(&e, bounds->x1-bounds->x0, 0.f);

		if (vector_intersects_pill2d(&p, &e, base, height, width))
		{
			return TRUE;
		}

		in = FALSE;
	}

	return in;
}

boolean pill_intersects_rectangle3d(
	real_point3d const *base,
	real_vector3d const *height,
	real width,
	real_rectangle3d const *bounds)
{
	/* originally stubbed */
	return FALSE;
}

boolean pill_intersects_triangle2d(
	real_point2d const *base,
	real_vector2d const *height,
	real width,
	real_point2d const *p0,
	real_point2d const *p1,
	real_point2d const *p2)
{
	real_vector2d v;
	real_vector2d e;
	boolean in = TRUE;

	vector_from_points2d(p0, base, &v);
	vector_from_points2d(p0, p1, &e);

	if (cross_product2d(&v, &e)>0.f)
	{
		if (vector_intersects_pill2d(p0, &e, base, height, width))
		{
			return TRUE;
		}

		in = FALSE;
	}

	vector_from_points2d(p1, base, &v);
	vector_from_points2d(p1, p2, &e);

	if (cross_product2d(&v, &e)>0.f)
	{
		if (vector_intersects_pill2d(p1, &e, base, height, width))
		{
			return TRUE;
		}

		in = FALSE;
	}

	vector_from_points2d(p2, base, &v);
	vector_from_points2d(p2, p0, &e);

	if (cross_product2d(&v, &e)>0.f)
	{
		if (vector_intersects_pill2d(p2, &e, base, height, width))
		{
			return TRUE;
		}

		in = FALSE;
	}

	return in;
}

boolean pill_intersects_triangle3d(
	real_point3d const *base,
	real_vector3d const *height,
	real width,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2)
{
	real_vector3d e0;
	real_vector3d e1;
	real_vector3d e2;
	real_vector3d n;
	real_vector3d v;
	real_point3d p;
	real t;
	real vn;
	boolean outside = FALSE;

	vector_from_points3d(p0, p1, &e0);
	vector_from_points3d(p1, p2, &e1);
	vector_from_points3d(base, p0, &v);
	cross_product3d(&e0, &e1, &n);
	t = dot_product3d(&v, &n)/dot_product3d(height, &n);
	point_from_line3d(base, height, PIN(t, 0.f, 1.f), &p);

	vector_from_points3d(p0, &p, &v);

	if (triple_product3d(&e0, &v, &n)<0.f)
	{
		if (vector_intersects_pill3d(p0, &e0, base, height, width))
		{
			return TRUE;
		}

		outside = TRUE;
	}

	vector_from_points3d(p1, &p, &v);

	if (triple_product3d(&e1, &v, &n)<0.f)
	{
		if (vector_intersects_pill3d(p1, &e1, base, height, width))
		{
			return TRUE;
		}

		outside = TRUE;
	}

	vector_from_points3d(p2, p0, &e2);
	vector_from_points3d(p2, &p, &v);

	if (triple_product3d(&e2, &v, &n)<0.f)
	{
		if (vector_intersects_pill3d(p2, &e2, base, height, width))
		{
			return TRUE;
		}

		outside = TRUE;
	}

	if (outside)
	{
		return FALSE;
	}

	if (t>0.f && t<1.f)
	{
		return TRUE;
	}

	vn = dot_product3d(&v, &n);

	return vn*vn<=magnitude_squared3d(&n)*width*width;
}

boolean point_from_planes3d(
	real_plane3d const *plane0,
	real_plane3d const *plane1,
	real_plane3d const *plane2,
	real_point3d *point)
{
	real t = triple_product3d(&plane0->n, &plane1->n, &plane2->n);

	if (!realcmp(t, 0.f))
	{
		real_vector3d cross;

		cross_product3d(&plane1->n, &plane2->n, &cross);
		scale_vector3d(&cross, plane0->d, (real_vector3d *)point);
		cross_product3d(&plane2->n, &plane0->n, &cross);
		point_from_line3d(point, &cross, (plane1->d), point);
		cross_product3d(&plane0->n, &plane1->n, &cross);
		point_from_line3d(point, &cross, (plane2->d), point);
		scale_vector3d((real_vector3d *)point, 1.f/t, (real_vector3d *)point);

		return TRUE;
	}

	return FALSE;
}

boolean line_from_planes3d(
	real_plane3d const *plane0,
	real_plane3d const *plane1,
	real_point3d *point,
	real_vector3d *vector)
{
	real vv;

	cross_product3d(&plane0->n, &plane1->n, vector);
	vv = magnitude_squared3d(vector);

	if (!realcmp(vv, 0.f))
	{
		real_vector3d cross;

		cross_product3d(&plane1->n, vector, &cross);
		scale_vector3d(&cross, plane0->d, (real_vector3d *)point);
		cross_product3d(vector, &plane0->n, &cross);
		point_from_line3d(point, &cross, (plane1->d), point);
		scale_vector3d((real_vector3d *)point, 1.f/vv, (real_vector3d *)point);

		return TRUE;
	}

	return FALSE;
}

void accelerate_to_velocity(
	real *position_reference,
	real *velocity_reference,
	real acceleration_maximum,
	real velocity_desired,
	real position_lower_bound,
	real position_upper_bound,
	boolean circular_position)
{
	if (circular_position)
	{
		real position = *position_reference;
		real velocity = *velocity_reference;
		real velocity_delta = velocity_desired - velocity;

		if (fabs(velocity_delta)>acceleration_maximum)
		{
			velocity_delta = velocity_delta<0.f ? -acceleration_maximum : acceleration_maximum;
		}

		velocity += velocity_delta;
		position += 0.5f*velocity_delta + velocity;

		if (position<position_lower_bound || position>position_upper_bound)
		{
			position = fmod(position - position_lower_bound, position_upper_bound - position_lower_bound) - position_lower_bound;
		}

		*position_reference = position;
		*velocity_reference = velocity;
	}
	else
	{
		accelerate_to_position(position_reference, velocity_reference, velocity_desired<0.f ? position_lower_bound : position_upper_bound, acceleration_maximum, fabs(velocity_desired), position_lower_bound, position_upper_bound, FALSE);
	}

	return;
}

boolean accelerate_to_position(
	real *position_reference,
	real *velocity_reference,
	real position_desired,
	real acceleration_maximum,
	real velocity_maximum,
	real position_lower_bound,
	real position_upper_bound,
	boolean circular_position)
{
	real position = *position_reference;
	real velocity = *velocity_reference;
	real velocity_maximum_squared = velocity_maximum*velocity_maximum;
	real position_delta_desired = position_desired - position;
	boolean done = FALSE;

	if (circular_position)
	{
		real position_midpoint = (position_upper_bound - position_lower_bound)*0.5f;

		if (position_delta_desired>position_midpoint)
		{
			position_delta_desired -= 2.f*position_midpoint;
		}
		else if (-position_midpoint>position_delta_desired)
		{
			position_delta_desired += 2.f*position_midpoint;
		}
	}

	if (fabs(position_delta_desired - velocity)<=MIN(acceleration_maximum, velocity_maximum))
	{
		velocity = 0.f;
		position = position_desired;

		if (position<position_lower_bound)
		{
			position = position_lower_bound;
		}
		else if (position>position_upper_bound)
		{
			position = position_upper_bound;
		}

		done = TRUE;
	}
	else
	{
		real velocity_desired = 2.f*acceleration_maximum*fabs(position_delta_desired);
		real acceleration;

		velocity_desired = velocity_desired>=velocity_maximum_squared ? velocity_maximum : square_root(velocity_desired);

		if (position_delta_desired<0.f)
		{
			velocity_desired = -velocity_desired;
		}

		acceleration = velocity_desired - velocity;

		if (fabs(acceleration)>acceleration_maximum)
		{
			acceleration = acceleration<0.f ? -acceleration_maximum : acceleration_maximum;
		}

		velocity += acceleration;
		position += 0.5f*acceleration + velocity;

		if (circular_position)
		{
			if (position<position_lower_bound)
			{
				position += position_upper_bound - position_lower_bound;
			}
			else if (position>position_upper_bound)
			{
				position -= position_upper_bound - position_lower_bound;
			}
		}

		if (position<position_lower_bound)
		{
			position = position_lower_bound;
		}
		else if (position>position_upper_bound)
		{
			position = position_upper_bound;
		}
	}

	*velocity_reference = velocity;
	*position_reference = position;

	return done;
}

void angular_accelerate_to_position(
	real_vector3d *position,
	real_vector3d const *position_desired,
	real_vector3d *angular_velocity,
	real angular_velocity_magnitude_maximum,
	real angular_acceleration_magnitude_maximum)
{
	if (angular_acceleration_magnitude_maximum<=0.f && angular_velocity_magnitude_maximum<=0.f)
	{
		*angular_velocity = *global_zero_vector3d;
		*position = *position_desired;
	}
	else
	{
		boolean exact = FALSE;
		real dot_product = dot_product3d(position, position_desired);
		real angle;
		real angular_velocity_magnitude_desired;
		real_vector3d angular_velocity_desired;
		real_vector3d instantaneous_acceleration_desired;
		real instantaneous_acceleration_magnitude_desired_squared;

		if (dot_product<-1.f)
		{
			dot_product = -1.f;
		}

		if (dot_product>1.f)
		{
			dot_product = 1.f;
		}

		angle = arccosine(dot_product);
		angular_velocity_magnitude_desired = 2.f*angular_acceleration_magnitude_maximum*angle;
		angular_velocity_magnitude_desired = angular_velocity_magnitude_desired>=angular_velocity_magnitude_maximum*angular_velocity_magnitude_maximum ? angular_velocity_magnitude_maximum : square_root(angular_velocity_magnitude_desired);

		cross_product3d(position, position_desired, &angular_velocity_desired);
		normalize3d(&angular_velocity_desired);
		scale_vector3d(&angular_velocity_desired, angular_velocity_magnitude_desired, &angular_velocity_desired);
		subtract_vectors3d(&angular_velocity_desired, angular_velocity, &instantaneous_acceleration_desired);
		instantaneous_acceleration_magnitude_desired_squared = magnitude_squared3d(&instantaneous_acceleration_desired);

		if (instantaneous_acceleration_magnitude_desired_squared<angular_acceleration_magnitude_maximum*angular_acceleration_magnitude_maximum)
		{
			if (angular_velocity_magnitude_desired<0.001f*0.001f)
			{
				*angular_velocity = *global_zero_vector3d;
				*position = *position_desired;
				exact = TRUE;
			}
			else
			{
				*angular_velocity = angular_velocity_desired;
			}
		}
		else
		{
			real instantaneous_acceleration_magnitude_desired = square_root(instantaneous_acceleration_magnitude_desired_squared);

			point_from_line3d((real_point3d *)angular_velocity, &instantaneous_acceleration_desired, angular_acceleration_magnitude_maximum/instantaneous_acceleration_magnitude_desired, (real_point3d *)angular_velocity);
		}

		if (!exact)
		{
			real_vector3d velocity_axis = *angular_velocity;
			real velocity_angle = normalize3d(&velocity_axis);

			if (velocity_angle!=0.f)
			{
				rotate_vector_about_axis(position, &velocity_axis, sine(velocity_angle), cosine(velocity_angle));
				normalize3d(position);
			}
		}
	}

	return;
}

boolean accelerate_to_velocity3d(
	real_vector3d *velocity,
	real_vector3d const *velocity_desired,
	real acceleration_maximum)
{
	real_vector3d acceleration;

	subtract_vectors3d(velocity_desired, velocity, &acceleration);

	if (limit3d(&acceleration, acceleration_maximum))
	{
		add_vectors3d(velocity, &acceleration, velocity);
		return FALSE;
	}
	else
	{
		*velocity = *velocity_desired;
		return TRUE;
	}
}

boolean accerate_to_position3d(
	real_point3d *position,
	real_vector3d *velocity,
	real_point3d *position_desired,
	real velocity_maximum,
	real acceleration_maximum)
{
	real_vector3d velocity_desired;
	real distance;

	vector_from_points3d(position, position_desired, &velocity_desired);
	distance = normalize3d(&velocity_desired);

	if (distance==0.f)
	{
		set_real_vector3d(&velocity_desired, 0.f, 0.f, 0.f);
	}
	else
	{
		scale_vector3d(&velocity_desired, square_root(2.f*acceleration_maximum*distance), &velocity_desired);
	}

	if (accelerate_to_velocity3d(velocity, &velocity_desired, acceleration_maximum) && distance==0.f)
	{
		*position = *position_desired;
		return TRUE;
	}

	add_vectors3d((real_vector3d *)position, velocity, (real_vector3d *)position);

	return FALSE;
}

byte quantize_real_to_byte_lower_bound(
	real min,
	real max,
	real value)
{
	byte test = (byte)((value-min)/(max-min)*255.f);

	match_vassert("c:\\halo\\SOURCE\\math\\real_math.c", 2803, min-_real_epsilon<=value && max+_real_epsilon>=value, csprintf(temporary, "%lf is not between %lf and %lf", value, min, max));

	while (test>0 && value<dequantize_byte_to_real(min, max, test))
	{
		test--;
	}

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 2809, dequantize_byte_to_real(min, max, test)<=value || (test==0 && dequantize_byte_to_real(min, max, test)<=value+_real_epsilon));

	return test;
}

byte quantize_real_to_byte_upper_bound(
	real min,
	real max,
	real value)
{
	byte test = (byte)((value-min)/(max-min)*255.f);

	match_vassert("c:\\halo\\SOURCE\\math\\real_math.c", 2823, min-_real_epsilon<=value && max+_real_epsilon>=value, csprintf(temporary, "%lf is not between %lf and %lf", value, min, max));

	while (test<UNSIGNED_CHAR_MAX && value>dequantize_byte_to_real(min, max, test))
	{
		test++;
	}

	match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 2829, dequantize_byte_to_real(min, max, test)>=value || (test==UNSIGNED_CHAR_MAX && dequantize_byte_to_real(min, max, test)>=value-_real_epsilon));

	return test;
}

byte_rectangle3d *quantize_real_to_byte_rectangle3d(
	real_rectangle3d const *parent,
	real_rectangle3d const *rectangle,
	byte_rectangle3d *result)
{
	if (rectangle->x0==REAL_MAX)
	{
		match_assert("c:\\halo\\SOURCE\\math\\real_math.c", 2843, rectangle->x1==REAL_MIN && rectangle->y0==REAL_MAX && rectangle->y1==REAL_MIN && rectangle->z0==REAL_MAX && rectangle->z1==REAL_MIN);
		memset(result, 0, sizeof(*result));
	}
	else
	{
		result->x0 = quantize_real_to_byte_lower_bound(parent->x0, parent->x1, rectangle->x0);
		result->x1 = quantize_real_to_byte_upper_bound(parent->x0, parent->x1, rectangle->x1);
		result->y0 = quantize_real_to_byte_lower_bound(parent->y0, parent->y1, rectangle->y0);
		result->y1 = quantize_real_to_byte_upper_bound(parent->y0, parent->y1, rectangle->y1);
		result->z0 = quantize_real_to_byte_lower_bound(parent->z0, parent->z1, rectangle->z0);
		result->z1 = quantize_real_to_byte_upper_bound(parent->z0, parent->z1, rectangle->z1);
	}

	return result;
}

/* ---------- private code */
