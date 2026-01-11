/*
MATRIX_MATH.C

symbols in this file:
000F8900 0040:
	_matrix4x3_identity (0000)
000F8940 0030:
	_matrix4x3_transpose (0000)
000F8970 00f0:
	_matrix4x3_inverse (0000)
000F8A60 0040:
	_matrix4x3_scale (0000)
000F8AA0 0050:
	_matrix4x3_translation (0000)
000F8AF0 00e0:
	_matrix4x3_rotation_from_axis_and_angle (0000)
000F8BD0 0120:
	_matrix4x3_rotation_from_quaternion (0000)
000F8CF0 0030:
	_matrix4x3_from_point_and_quaternion (0000)
000F8D20 0040:
	_matrix4x3_from_orientation (0000)
000F8D60 0050:
	_matrix4x3_to_point_and_vectors (0000)
000F8DB0 0080:
	_matrix4x3_transform_point (0000)
000F8E30 0070:
	_matrix4x3_transform_vector (0000)
000F8EA0 0060:
	_matrix4x3_transform_normal (0000)
000F8F00 00a0:
	_matrix4x3_inverse_transform_point (0000)
000F8FA0 0070:
	_matrix4x3_inverse_transform_vector (0000)
000F9010 0060:
	_matrix4x3_inverse_transform_normal (0000)
000F9070 0150:
	_matrix4x3_multiply (0000)
000F91C0 0050:
	_matrix3x3_determinant (0000)
000F9210 0070:
	_matrix3x3_transpose (0000)
000F9280 0140:
	_matrix3x3_inverse (0000)
000F93C0 00d0:
	_matrix3x3_from_axis_and_angle (0000)
000F9490 0120:
	_matrix3x3_multiply (0000)
000F95B0 0080:
	_matrix3x3_transform_vector (0000)
000F9630 0080:
	_matrix4x3_rotation_from_vectors (0000)
000F96B0 00b0:
	_matrix4x3_rotation_from_angles (0000)
000F9760 0080:
	_matrix4x3_rotation_to_angles (0000)
000F97E0 0150:
	_matrix4x3_rotation_to_quaternion (0000)
000F9930 0040:
	_matrix4x3_from_point_and_vectors (0000)
000F9970 0070:
	_vector_from_matrices4x3 (0000)
000F99E0 0080:
	_matrix4x3_transform_plane (0000)
000F9A60 0080:
	_matrix4x3_inverse_transform_plane (0000)
000F9AE0 0070:
	_matrix3x3_from_forward_and_up (0000)
000F9B50 0150:
	_matrix3x3_rotation_to_quaternion (0000)
000F9CA0 0040:
	_valid_real_plane3d (0000)
000F9CE0 00b0:
	_matrix4x3_from_plane (0000)
0027AC18 001c:
	??_C@_0BM@OIPNJJOM@?$CBrealcmp?$CIdeterminant?0?50?40f?$CJ?$AA@ (0000)
0027AC34 0022:
	??_C@_0CC@FNOHDKIK@c?3?2halo?2SOURCE?2math?2matrix_math?4@ (0000)
0027AC58 0008:
	__real@3f1a36e2eb1c432d (0000)
0027AC60 001a:
	??_C@_0BK@BHMHOMPE@valid_real_plane3d?$CIplane?$CJ?$AA@ (0000)
0030790C 000e:
	_data_0030790c (0000)
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

real_vector3d *matrix4x3_transform_normal(
	real_matrix4x3 const *matrix,
	real_vector3d const *normal,
	real_vector3d *result)
{
	real i= normal->i;
	real j= normal->j;
	real k= normal->k;

	result->i = i*matrix->forward.i + j*matrix->left.i + k*matrix->up.i;
	result->j = i*matrix->forward.j + j*matrix->left.j + k*matrix->up.j;
	result->k = i*matrix->forward.k + j*matrix->left.k + k*matrix->up.k;
	return result;
}

/* ---------- private code */
