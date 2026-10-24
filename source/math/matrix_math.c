/*
MATRIX_MATH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void matrix4x3_identity(
	real_matrix4x3 *matrix)
{
	matrix->scale = 1.f;
	matrix->forward.i = 1.f;
	matrix->forward.j = 0.f;
	matrix->forward.k = 0.f;
	matrix->left.i = 0.f;
	matrix->left.j = 1.f;
	matrix->left.k = 0.f;
	matrix->up.i = 0.f;
	matrix->up.j = 0.f;
	matrix->up.k = 1.f;
	matrix->position.x = 0.f;
	matrix->position.y = 0.f;
	matrix->position.z = 0.f;

	return;
}

void matrix4x3_transpose(
	real_matrix4x3 *matrix)
{
	real swap;

	swap = matrix->n[1][0];
	matrix->n[1][0] = matrix->n[0][1];
	matrix->n[0][1] = swap;

	swap = matrix->n[2][0];
	matrix->n[2][0] = matrix->n[0][2];
	matrix->n[0][2] = swap;

	swap = matrix->n[2][1];
	matrix->n[2][1] = matrix->n[1][2];
	matrix->n[1][2] = swap;

	return;
}

void matrix4x3_inverse(
	real_matrix4x3 const *matrix,
	real_matrix4x3 *result)
{
	if (matrix->scale!=0.f)
	{
		real x = -matrix->position.x;
		real y = -matrix->position.y;
		real z = -matrix->position.z;
		real swap;

		if (matrix->scale!=1.f)
		{
			result->scale = 1.f / matrix->scale;
			x *= result->scale;
			y *= result->scale;
			z *= result->scale;
		}
		else
		{
			result->scale = 1.f;
		}

		result->n[0][0] = matrix->n[0][0];
		result->n[1][1] = matrix->n[1][1];
		result->n[2][2] = matrix->n[2][2];

		swap = matrix->n[1][0];
		result->n[1][0] = matrix->n[0][1];
		result->n[0][1] = swap;

		swap = matrix->n[2][0];
		result->n[2][0] = matrix->n[0][2];
		result->n[0][2] = swap;

		swap = matrix->n[2][1];
		result->n[2][1] = matrix->n[1][2];
		result->n[1][2] = swap;

		result->n[3][0] = (x*result->n[0][0]) + y*result->n[1][0] + (z*result->n[2][0]);
		result->n[3][1] = (x*result->n[0][1]) + y*result->n[1][1] + (z*result->n[2][1]);
		result->n[3][2] = (x*result->n[0][2]) + y*result->n[1][2] + (z*result->n[2][2]);
	}
	else
	{
		memset(result, 0, sizeof(*result));
	}

	return;
}

void matrix4x3_scale(
	real_matrix4x3 *matrix,
	real scale)
{
	matrix->scale = scale;
	matrix->forward.i = 1.f;
	matrix->forward.j = 0.f;
	matrix->forward.k = 0.f;
	matrix->left.i = 0.f;
	matrix->left.j = 1.f;
	matrix->left.k = 0.f;
	matrix->up.i = 0.f;
	matrix->up.j = 0.f;
	matrix->up.k = 1.f;
	matrix->position.x = 0.f;
	matrix->position.y = 0.f;
	matrix->position.z = 0.f;

	return;
}

void matrix4x3_translation(
	real_matrix4x3 *matrix,
	real_point3d const *point)
{
	matrix->scale = 1.f;
	matrix->forward.i = 1.f;
	matrix->forward.j = 0.f;
	matrix->forward.k = 0.f;
	matrix->left.i = 0.f;
	matrix->left.j = 1.f;
	matrix->left.k = 0.f;
	matrix->up.i = 0.f;
	matrix->up.j = 0.f;
	matrix->up.k = 1.f;
	matrix->position = *point;

	return;
}

void matrix4x3_rotation_from_axis_and_angle(
	real_matrix4x3 *matrix,
	real_vector3d const *axis,
	real sine,
	real cosine)
{
	real_vector3d axis_squared;
	real_vector3d axis_sine;

	axis_squared.i = axis->i*axis->i;
	axis_squared.j = axis->j*axis->j;
	axis_squared.k = axis->k*axis->k;
	axis_sine.i = sine*axis->i;
	axis_sine.j = sine*axis->j;
	axis_sine.k = sine*axis->k;

	matrix->scale = 1.f;

	matrix->forward.i = (1.f-axis_squared.i)*cosine + axis_squared.i;
	matrix->forward.j = axis->i*axis->j*(1.f-cosine);
	matrix->left.i = matrix->forward.j;
	matrix->left.i-= axis_sine.k;
	matrix->forward.j+= axis_sine.k;

	matrix->left.j = (1.f-axis_squared.j)*cosine + axis_squared.j;
	matrix->forward.k = axis->k*axis->i*(1.f-cosine);
	matrix->up.i = matrix->forward.k;
	matrix->up.i+= axis_sine.j;
	matrix->forward.k-= axis_sine.j;

	matrix->up.k = (1.f-axis_squared.k)*cosine + axis_squared.k;
	matrix->left.k = axis->j*axis->k*(1.f-cosine);
	matrix->up.j = matrix->left.k;
	matrix->up.j-= axis_sine.i;
	matrix->left.k+= axis_sine.i;

	matrix->position.x = matrix->position.y = matrix->position.z = 0.f;

	return;
}

void matrix4x3_rotation_from_vectors(
	real_matrix4x3 *matrix,
	real_vector3d const *forward,
	real_vector3d const *up)

{
	matrix->scale = 1.f;
	matrix->forward = *forward;
	cross_product3d(up, forward, &matrix->left);

	matrix->up = *up;
	set_real_point3d(&matrix->position, 0.f, 0.f, 0.f);

	return;
}

void matrix4x3_rotation_from_angles(
	real_matrix4x3 *matrix,
	real yaw,
	real pitch,
	real roll)
{
	real a = cosine(roll);
	real b = sine(roll);
	real c = cosine(pitch);
	real d = sine(pitch);
	real e = cosine(yaw);
	real f = sine(yaw);
	real ad = d*a;
	real bd = d*b;

	matrix->scale = 1.f;
	matrix->forward.i = e*c;
	matrix->forward.j = f*a - bd*e;
	matrix->forward.k = ad*e + f*b;
	matrix->left.i = -(f*c);
	matrix->left.j = bd*f + e*a;
	matrix->left.k = e*b - ad*f;
	matrix->up.i = -d;
	matrix->up.j = -(c*b);
	matrix->up.k = c*a;
	matrix->position.x = 0.f;
	matrix->position.y = 0.f;
	matrix->position.z = 0.f;

	return;
}

void matrix4x3_rotation_to_angles(
	real_matrix4x3 *matrix,
	real_euler_angles3d *angles)
{
	real tr_x;
	real tr_y;
	real C;

	angles->pitch = -arcsine(matrix->up.i);
	C = cosine(angles->pitch);
	if (C>0.0001)
	{
		tr_x = matrix->up.k/C;
		tr_y = -matrix->up.j/C;
		angles->roll = arctangent(tr_y, tr_x);

		tr_x = matrix->forward.i/C;
		tr_y = -matrix->left.i/C;
		angles->yaw = arctangent(tr_y, tr_x);
	}
	else
	{
		angles->roll = 0.f;

		tr_x = matrix->left.j;
		tr_y = matrix->forward.j;
		angles->yaw = arctangent(tr_y, tr_x);
	}

	return;
}

void matrix4x3_rotation_from_quaternion(
	real_matrix4x3 *matrix,
	real_quaternion const *quaternion)
{
	real n = quaternion->v.i*quaternion->v.i + quaternion->v.j*quaternion->v.j + quaternion->v.k*quaternion->v.k + quaternion->w*quaternion->w;
	real scale = n!=0.f ? 2.f/n : 0.f;
	real xs = scale*quaternion->v.i;
	real ys = scale*quaternion->v.j;
	real zs = scale*quaternion->v.k;
	real wx = xs*quaternion->w;
	real wy = ys*quaternion->w;
	real wz = zs*quaternion->w;
	real xx = xs*quaternion->v.i;
	real xy = ys*quaternion->v.i;
	real xz = zs*quaternion->v.i;
	real yy = ys*quaternion->v.j;
	real yz = zs*quaternion->v.j;
	real zz = zs*quaternion->v.k;

	matrix->forward.i = 1.f - (yy+zz);
	matrix->forward.j = xy - wz;
	matrix->forward.k = xz + wy;
	matrix->left.i = xy + wz;
	matrix->left.j = 1.f - (xx+zz);
	matrix->left.k = yz - wx;
	matrix->up.i = xz - wy;
	matrix->up.j = yz + wx;
	matrix->up.k = 1.f - (xx+yy);
	matrix->scale = 1.f;
	matrix->position.x = 0.f;
	matrix->position.y = 0.f;
	matrix->position.z = 0.f;

	return;
}

void matrix4x3_rotation_to_quaternion(
	real_matrix4x3 const *matrix,
	real_quaternion *quaternion)
{
	real trace;
	real s;

	trace = matrix->n[0][0] + matrix->n[1][1] + matrix->n[2][2];
	if (trace>0.f)
	{
		s = square_root(trace + 1.f);
		quaternion->w = 0.5f*s;
		s = 0.5f/s;
		quaternion->v.i = (matrix->n[2][1]-matrix->n[1][2])*s;
		quaternion->v.j = (matrix->n[0][2]-matrix->n[2][0])*s;
		quaternion->v.k = (matrix->n[1][0]-matrix->n[0][1])*s;
	}
	else
	{
		static short next_field[3] = { 1, 2, 0 };
		short i = 0;
		short j, k;
		real q[3];

		if (matrix->n[1][1]>matrix->n[0][0])
		{
			i = 1;
		}

		if (matrix->n[2][2]>matrix->n[i][i])
		{
			i = 2;
		}

		j = next_field[i];
		k = next_field[j];

		s = square_root(matrix->n[i][i] - (matrix->n[j][j] + matrix->n[k][k]) + 1.f);
		q[i] = s*0.5f;
		if (s!=0.f)
		{
			s = 0.5f/s;
		}

		q[j] = (matrix->n[i][j] + matrix->n[j][i])*s;
		q[k] = (matrix->n[i][k] + matrix->n[k][i])*s;
		quaternion->w = (matrix->n[k][j] - matrix->n[j][k])*s;
		quaternion->v.i = q[0];
		quaternion->v.j = q[1];
		quaternion->v.k = q[2];
	}

	return;
}

void matrix4x3_from_point_and_vectors(
	real_matrix4x3 *matrix,
	real_point3d const *point,
	real_vector3d const *forward,
	real_vector3d const *up)
{
	matrix4x3_rotation_from_vectors(matrix, forward, up);
	matrix->position = *point;
	
	return;
}

void matrix4x3_from_point_and_quaternion(
	real_matrix4x3 *matrix,
	real_point3d const *point,
	real_quaternion const *quaternion)
{
	matrix4x3_rotation_from_quaternion(matrix, quaternion);
	matrix->position = *point;

	return;
}

void matrix4x3_from_orientation(
	real_matrix4x3 *matrix,
	real_orientation const *orientation)
{
	matrix4x3_rotation_from_quaternion(matrix, &orientation->rotation);
	matrix->scale = orientation->scale;
	matrix->position = orientation->translation;

	return;
}

void matrix4x3_from_plane(
	real_matrix4x3 *matrix,
	real_plane3d const *plane)
{
	real_vector3d forward;
	real_point3d origin;

	match_assert("c:\\halo\\SOURCE\\math\\matrix_math.c", 370, valid_real_plane3d(plane));

	perpendicular3d(&plane->n, &forward);
	normalize3d(&forward);
	origin.x = plane->n.i*plane->d;
	origin.y = plane->n.j*plane->d;
	origin.z = plane->n.k*plane->d;
	matrix4x3_from_point_and_vectors(matrix, &origin, &forward, &plane->n);

	return;
}

void matrix4x3_to_point_and_vectors(
	real_matrix4x3 const *matrix,
	real_point3d *point,
	real_vector3d *forward,
	real_vector3d *up)
{
	*forward = matrix->forward;
	*up = matrix->up;
	*point = matrix->position;

	return;
}

real_vector3d *vector_from_matrices4x3(
	real_matrix4x3 const *a,
	real_matrix4x3 const *b,
	real_vector3d *rotation)
{
	real_matrix4x3 b_inverse;
	real_matrix4x3 displacement_rotation_matrix;
	real_quaternion displacement_rotation_quaternion;
	real angle;

	matrix4x3_inverse(b, &b_inverse);
	matrix4x3_multiply(a, &b_inverse, &displacement_rotation_matrix);
	matrix4x3_rotation_to_quaternion(&displacement_rotation_matrix, &displacement_rotation_quaternion);
	quaternion_to_angle_and_vector(&displacement_rotation_quaternion, &angle, rotation);
	scale_vector3d(rotation, angle, rotation);

	return rotation;
}

real_point3d *matrix4x3_transform_point(
	real_matrix4x3 const *matrix,
	real_point3d const *point,
	real_point3d *result)
{
	real x = point->x;
	real y = point->y;
	real z = point->z;

	if (matrix->scale != 1.f)
	{
		x *= matrix->scale;
		y *= matrix->scale;
		z *= matrix->scale;
	}

	result->x = matrix->up.i*z + matrix->left.i*y + matrix->forward.i*x + matrix->position.x;
	result->y = matrix->up.j*z + matrix->left.j*y + matrix->forward.j*x + matrix->position.y;
	result->z = matrix->up.k*z + matrix->left.k*y + matrix->forward.k*x + matrix->position.z;
	return result;
}

real_vector3d *matrix4x3_transform_vector(
	real_matrix4x3 const *matrix,
	real_vector3d const *vector,
	real_vector3d *result)
{
	real i = vector->i;
	real j = vector->j;
	real k = vector->k;

	if (matrix->scale!=1.f)
	{
		i*= matrix->scale;
		j*= matrix->scale;
		k*= matrix->scale;
	}

	result->i = matrix->up.i*k + matrix->left.i*j + matrix->forward.i*i;
	result->j = matrix->up.j*k + matrix->left.j*j + matrix->forward.j*i;
	result->k = matrix->up.k*k + matrix->left.k*j + matrix->forward.k*i;
	return result;
}

real_vector3d *matrix4x3_transform_normal(
	real_matrix4x3 const *matrix,
	real_vector3d const *normal,
	real_vector3d *result)
{
	real i = normal->i;
	real j = normal->j;
	real k = normal->k;

	result->i = i*matrix->forward.i + j*matrix->left.i + k*matrix->up.i;
	result->j = i*matrix->forward.j + j*matrix->left.j + k*matrix->up.j;
	result->k = i*matrix->forward.k + j*matrix->left.k + k*matrix->up.k;
	return result;
}

real_plane3d *matrix4x3_transform_plane(
	real_matrix4x3 const *matrix,
	real_plane3d const *plane,
	real_plane3d *result)
{
	matrix4x3_transform_normal(matrix, &plane->n, &result->n);
	result->d = plane->d*matrix->scale + dot_product3d((real_vector3d const *)&matrix->position, &result->n);

	return result;
}

real_point3d *matrix4x3_inverse_transform_point(
	real_matrix4x3 const *matrix,
	real_point3d const *point,
	real_point3d *result)
{
	if (matrix->scale!=0.f)
	{
		real x = point->x - matrix->position.x;
		real y = point->y - matrix->position.y;
		real z = point->z - matrix->position.z;

		if (matrix->scale!=1.f)
		{
			real one_over_scale = 1.f/matrix->scale;

			x*= one_over_scale;
			y*= one_over_scale;
			z*= one_over_scale;
		}

		result->x = matrix->forward.k*z + matrix->forward.j*y + matrix->forward.i*x;
		result->y = matrix->left.k*z + matrix->left.j*y + matrix->left.i*x;
		result->z = matrix->up.k*z + matrix->up.j*y + matrix->up.i*x;
	}
	else
	{
		result->x = 0.f;
		result->y = 0.f;
		result->z = 0.f;
	}

	return result;
}

real_vector3d *matrix4x3_inverse_transform_vector(
	real_matrix4x3 const *matrix,
	real_vector3d const *vector,
	real_vector3d *result)
{
	real i = vector->i;
	real j = vector->j;
	real k = vector->k;

	if (matrix->scale!=1.f)
	{
		real one_over_scale = 1.f/matrix->scale;

		i*= one_over_scale;
		j*= one_over_scale;
		k*= one_over_scale;
	}

	result->i = matrix->forward.k*k + matrix->forward.j*j + matrix->forward.i*i;
	result->j = matrix->left.k*k + matrix->left.j*j + matrix->left.i*i;
	result->k = matrix->up.k*k + matrix->up.j*j + matrix->up.i*i;
	return result;
}

real_vector3d *matrix4x3_inverse_transform_normal(
	real_matrix4x3 const *matrix,
	real_vector3d const *normal,
	real_vector3d *result)
{
	real i = normal->i;
	real j = normal->j;
	real k = normal->k;

	result->i = matrix->forward.k*k + matrix->forward.j*j + matrix->forward.i*i;
	result->j = matrix->left.k*k + matrix->left.j*j + matrix->left.i*i;
	result->k = matrix->up.k*k + matrix->up.j*j + matrix->up.i*i;
	return result;
}

real_plane3d *matrix4x3_inverse_transform_plane(
	real_matrix4x3 const *matrix,
	real_plane3d const *plane,
	real_plane3d *result)
{
	if (matrix->scale!=0.f)
	{
		result->d = plane->d - dot_product3d((real_vector3d const *)&matrix->position, &plane->n);
		if (matrix->scale!=1.f)
		{
			result->d/= matrix->scale;
		}
	}
	else
	{
		result->d = 0.f;
	}

	matrix4x3_inverse_transform_vector(matrix, &plane->n, &result->n);

	return result;
}

void matrix4x3_multiply(
	real_matrix4x3 const *a,
	real_matrix4x3 const *b,
	real_matrix4x3 *result)
{
	real const *a_vectors = a->n[0];
	real const *b_vectors = b->n[0];
	real *result_vectors = result->n[0];
	real const *a_scale = &a->scale;

	__asm
	{
		mov ecx, a_vectors
		mov edx, b_vectors
		mov eax, result_vectors

		movss xmm2, dword ptr [ecx+0x20]
		movhps xmm2, qword ptr [ecx+0x18]
		movss xmm3, dword ptr [edx]
		movss xmm4, dword ptr [edx+0x4]
		movss xmm0, dword ptr [ecx]
		movhps xmm0, qword ptr [ecx+0x4]
		shufps xmm2, xmm2, 0x36
		shufps xmm3, xmm3, 0x0
		movss xmm1, dword ptr [ecx+0xc]
		movhps xmm1, qword ptr [ecx+0x10]
		shufps xmm4, xmm4, 0x0
		mulps xmm3, xmm0
		movss xmm5, dword ptr [edx+0x8]
		movss xmm6, dword ptr [edx+0xc]
		mulps xmm4, xmm1
		shufps xmm5, xmm5, 0x0
		mulps xmm5, xmm2
		shufps xmm6, xmm6, 0x0
		mulps xmm6, xmm0
		addps xmm3, xmm4
		movss xmm7, dword ptr [edx+0x10]
		movss xmm4, dword ptr [edx+0x1c]
		shufps xmm7, xmm7, 0x0
		addps xmm3, xmm5
		mulps xmm7, xmm1
		shufps xmm4, xmm4, 0x0
		movss xmm5, dword ptr [edx+0x14]
		shufps xmm5, xmm5, 0x0
		mulps xmm4, xmm1
		mulps xmm5, xmm2
		addps xmm6, xmm7
		movss xmm7, dword ptr [edx+0x18]
		movss dword ptr [eax], xmm3
		movhps qword ptr [eax+0x4], xmm3
		addps xmm6, xmm5
		shufps xmm7, xmm7, 0x0
		movss xmm5, dword ptr [edx+0x20]
		mulps xmm7, xmm0
		shufps xmm5, xmm5, 0x0
		movss dword ptr [eax+0xc], xmm6
		mulps xmm5, xmm2
		addps xmm7, xmm4
		movhps qword ptr [eax+0x10], xmm6
		addps xmm7, xmm5
		shufps xmm7, xmm7, 0x8f
		movhps qword ptr [eax+0x18], xmm7
		movss dword ptr [eax+0x20], xmm7

		movss xmm3, dword ptr [edx+0x24]
		movss xmm4, dword ptr [edx+0x28]
		movss xmm5, dword ptr [edx+0x2c]
		shufps xmm3, xmm3, 0x0
		shufps xmm4, xmm4, 0x0
		shufps xmm5, xmm5, 0x0
		mulps xmm3, xmm0
		mulps xmm4, xmm1
		mulps xmm5, xmm2
		movss xmm6, dword ptr [ecx+0x24]
		addps xmm3, xmm4
		movhps xmm6, qword ptr [ecx+0x28]
		addps xmm3, xmm5
		mov ecx, a_scale
		movss xmm5, dword ptr [ecx]
		shufps xmm5, xmm5, 0x0
		mulps xmm3, xmm5
		addps xmm3, xmm6
		movss dword ptr [eax+0x24], xmm3
		movhps qword ptr [eax+0x28], xmm3
	}

	result->scale = a->scale*b->scale;

	return;
}

real matrix3x3_determinant(
	real_matrix3x3 const *matrix)
{
	return matrix->n[0][0]*matrix->n[1][1]*matrix->n[2][2]
		+ matrix->n[0][1]*matrix->n[1][2]*matrix->n[2][0]
		+ matrix->n[0][2]*matrix->n[1][0]*matrix->n[2][1]
		- matrix->n[0][0]*matrix->n[1][2]*matrix->n[2][1]
		- matrix->n[0][1]*matrix->n[1][0]*matrix->n[2][2]
		- matrix->n[0][2]*matrix->n[1][1]*matrix->n[2][0];
}

real_matrix3x3 *matrix3x3_transpose(
	real_matrix3x3 const *matrix,
	real_matrix3x3 *result)
{
	if (matrix==result)
	{
		real swap;

		swap = matrix->n[0][1];
		result->n[0][1] = matrix->n[1][0];
		result->n[1][0] = swap;

		swap = matrix->n[0][2];
		result->n[0][2] = matrix->n[2][0];
		result->n[2][0] = swap;

		swap = matrix->n[1][2];
		result->n[1][2] = matrix->n[2][1];
		result->n[2][1] = swap;
	}
	else
	{
		result->n[0][0] = matrix->n[0][0];
		result->n[0][1] = matrix->n[1][0];
		result->n[0][2] = matrix->n[2][0];
		result->n[1][0] = matrix->n[0][1];
		result->n[1][1] = matrix->n[1][1];
		result->n[1][2] = matrix->n[2][1];
		result->n[2][0] = matrix->n[0][2];
		result->n[2][1] = matrix->n[1][2];
		result->n[2][2] = matrix->n[2][2];
	}

	return result;
}

real_matrix3x3 *matrix3x3_inverse(
	real_matrix3x3 const *matrix,
	real determinant,
	real_matrix3x3 *result)
{
	real_matrix3x3 copy;
	real oodeterminant;
	short i, j;

	if (matrix==result)
	{
		copy = *matrix;
		matrix = &copy;
	}

	match_assert("c:\\halo\\SOURCE\\math\\matrix_math.c", 787, !realcmp(determinant, 0.0f));

	oodeterminant = 1.f/determinant;
	for (i = 0; i<3; i++)
	{
		for (j = 0; j<3; j++)
		{
			short i1 = i<2 ? i+1 : 0;
			short i2 = i>0 ? i-1 : 2;
			short j1 = j<2 ? j+1 : 0;
			short j2 = j>0 ? j-1 : 2;

			result->n[j][i] = (matrix->n[i1][j1]*matrix->n[i2][j2] - matrix->n[i1][j2]*matrix->n[i2][j1])*oodeterminant;
		}
	}

	return result;
}

real_matrix3x3 *matrix3x3_from_forward_and_up(
	real_matrix3x3 *matrix,
	real_vector3d const *forward,
	real_vector3d const *up)
{
	matrix->forward = *forward;
	cross_product3d(up, forward, &matrix->left);
	matrix->up = *up;

	return matrix;
}

real_matrix3x3 *matrix3x3_from_axis_and_angle(
	real_matrix3x3 *matrix,
	real_vector3d const *axis,
	real sine,
	real cosine)
{
	real_vector3d axis_squared;
	real_vector3d axis_sine;

	axis_squared.i = axis->i*axis->i;
	axis_squared.j = axis->j*axis->j;
	axis_squared.k = axis->k*axis->k;
	axis_sine.i = sine*axis->i;
	axis_sine.j = sine*axis->j;
	axis_sine.k = sine*axis->k;

	matrix->forward.i = (1.f-axis_squared.i)*cosine + axis_squared.i;
	matrix->forward.j = axis->i*axis->j*(1.f-cosine);
	matrix->left.i = matrix->forward.j;
	matrix->left.i-= axis_sine.k;
	matrix->forward.j+= axis_sine.k;

	matrix->left.j = (1.f-axis_squared.j)*cosine + axis_squared.j;
	matrix->forward.k = axis->k*axis->i*(1.f-cosine);
	matrix->up.i = matrix->forward.k;
	matrix->up.i+= axis_sine.j;
	matrix->forward.k-= axis_sine.j;

	matrix->up.k = (1.f-axis_squared.k)*cosine + axis_squared.k;
	matrix->left.k = axis->j*axis->k*(1.f-cosine);
	matrix->up.j = matrix->left.k;
	matrix->up.j-= axis_sine.i;
	matrix->left.k+= axis_sine.i;

	return matrix;
}

real_matrix3x3 *matrix3x3_multiply(
	real_matrix3x3 const *a,
	real_matrix3x3 const *b,
	real_matrix3x3 *result)
{
	real_matrix3x3 copy;

	if (a==result)
	{
		copy = *a;
		a = &copy;
	}

	if (b==result)
	{
		copy = *b;
		b = &copy;
	}

	result->n[0][0] = a->n[0][0]*b->n[0][0] + a->n[1][0]*b->n[0][1] + a->n[2][0]*b->n[0][2];
	result->n[0][1] = a->n[0][1]*b->n[0][0] + a->n[1][1]*b->n[0][1] + a->n[2][1]*b->n[0][2];
	result->n[0][2] = a->n[0][2]*b->n[0][0] + a->n[1][2]*b->n[0][1] + a->n[2][2]*b->n[0][2];
	result->n[1][0] = a->n[0][0]*b->n[1][0] + a->n[1][0]*b->n[1][1] + a->n[2][0]*b->n[1][2];
	result->n[1][1] = a->n[0][1]*b->n[1][0] + a->n[1][1]*b->n[1][1] + a->n[2][1]*b->n[1][2];
	result->n[1][2] = a->n[0][2]*b->n[1][0] + a->n[1][2]*b->n[1][1] + a->n[2][2]*b->n[1][2];
	result->n[2][0] = a->n[0][0]*b->n[2][0] + a->n[1][0]*b->n[2][1] + a->n[2][0]*b->n[2][2];
	result->n[2][1] = a->n[0][1]*b->n[2][0] + a->n[1][1]*b->n[2][1] + a->n[2][1]*b->n[2][2];
	result->n[2][2] = a->n[0][2]*b->n[2][0] + a->n[1][2]*b->n[2][1] + a->n[2][2]*b->n[2][2];

	return result;
}

real_vector3d *matrix3x3_transform_vector(
	real_matrix3x3 const *matrix,
	real_vector3d const *vector,
	real_vector3d *result)
{
	real_vector3d copy;

	if (vector==result)
	{
		copy = *vector;
		vector = &copy;
	}

	result->i = matrix->n[0][0]*vector->i + matrix->n[1][0]*vector->j + matrix->n[2][0]*vector->k;
	result->j = matrix->n[0][1]*vector->i + matrix->n[1][1]*vector->j + matrix->n[2][1]*vector->k;
	result->k = matrix->n[0][2]*vector->i + matrix->n[1][2]*vector->j + matrix->n[2][2]*vector->k;

	return result;
}

real_quaternion *matrix3x3_rotation_to_quaternion(
	real_matrix3x3 const *matrix,
	real_quaternion *quaternion)
{
	real trace;
	real s;

	trace = matrix->n[0][0] + matrix->n[1][1] + matrix->n[2][2];
	if (trace>0.f)
	{
		s = square_root(trace + 1.f);
		quaternion->w = 0.5f*s;
		s = 0.5f/s;
		quaternion->v.i = (matrix->n[2][1]-matrix->n[1][2])*s;
		quaternion->v.j = (matrix->n[0][2]-matrix->n[2][0])*s;
		quaternion->v.k = (matrix->n[1][0]-matrix->n[0][1])*s;
	}
	else
	{
		static short next_field[3] = { 1, 2, 0 };
		short i = 0;
		short j, k;
		real q[3];

		if (matrix->n[1][1]>matrix->n[0][0])
		{
			i = 1;
		}

		if (matrix->n[2][2]>matrix->n[i][i])
		{
			i = 2;
		}

		j = next_field[i];
		k = next_field[j];

		s = square_root(matrix->n[i][i] - (matrix->n[j][j] + matrix->n[k][k]) + 1.f);
		q[i] = s*0.5f;
		if (s!=0.f)
		{
			s = 0.5f/s;
		}

		q[j] = (matrix->n[i][j] + matrix->n[j][i])*s;
		q[k] = (matrix->n[i][k] + matrix->n[k][i])*s;
		quaternion->w = (matrix->n[k][j] - matrix->n[j][k])*s;
		quaternion->v.i = q[0];
		quaternion->v.j = q[1];
		quaternion->v.k = q[2];
	}

	return quaternion;
}

/* ---------- private code */
