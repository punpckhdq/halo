/*
REAL_MATH.C

symbols in this file:
000FADE0 0010:
	_real_math_initialize (0000)
000FADF0 0010:
	_real_math_dispose (0000)
000FAE00 0020:
	_real_math_reset_precision (0000)
000FAE20 0020:
	_perpendicular2d (0000)
000FAE40 0090:
	_perpendicular3d (0000)
000FAED0 0030:
	_perpendicular4d (0000)
000FAF00 00a0:
	_rotate_vector_about_axis (0000)
000FAFA0 0050:
	_vectors_interpolate (0000)
000FAFF0 0050:
	_points_interpolate (0000)
000FB040 0020:
	_scalars_interpolate (0000)
000FB060 0060:
	_scalars_interpolate_and_clamp_0_to_1 (0000)
000FB0C0 0070:
	_component_vectors_from_normal3d (0000)
000FB130 00b0:
	_component_vectors_from_direction3d (0000)
000FB1E0 00d0:
	_quaternions_multiply (0000)
000FB2B0 0090:
	_quaternions_interpolate (0000)
000FB340 00a0:
	_quaternion_transform_point (0000)
000FB3E0 00b0:
	_vectors3d_from_euler_angles3d (0000)
000FB490 0100:
	_fast_vector_intersects_sphere (0000)
000FB590 0050:
	_point_in_rectangle2d (0000)
000FB5E0 0060:
	_point_in_rectangle3d (0000)
000FB640 01d0:
	_vector_intersects_rectangle2d (0000)
000FB810 0290:
	_vector_intersects_rectangle3d (0000)
000FBAA0 0010:
	_pill_intersects_rectangle3d (0000)
000FBAB0 0050:
	_fast_normalize2d (0000)
000FBB00 0060:
	_fast_normalize3d (0000)
000FBB60 0050:
	_cross_product_magnitude3d (0000)
000FBBB0 0030:
	_dequantize_byte_to_real (0000)
000FBBE0 0080:
	_signed_angle_between_vectors2d (0000)
000FBC60 00d0:
	_angle_between_vectors2d (0000)
000FBD30 00f0:
	_angle_between_vectors3d (0000)
000FBE20 0090:
	_angle_between_normals3d (0000)
000FBEB0 0070:
	_yaw_vectors (0000)
000FBF20 0080:
	_pitch_vectors (0000)
000FBFA0 0050:
	_fast_normals_interpolate (0000)
000FBFF0 0110:
	_normals_interpolate (0000)
000FC100 0040:
	_reflect_vector3d (0000)
000FC140 0110:
	_refract_vector3d (0000)
000FC250 0080:
	_quaternion_normalize (0000)
000FC2D0 0040:
	_quaternion_from_angle_and_vector (0000)
000FC310 0070:
	_quaternion_to_angle_and_vector (0000)
000FC380 0030:
	_quaternions_interpolate_and_normalize (0000)
000FC3B0 0070:
	_orientations_interpolate (0000)
000FC420 0040:
	_euler_angles2d_from_vector3d (0000)
000FC460 0030:
	_vector3d_from_euler_angles2d (0000)
000FC490 0020:
	_vector3d_from_angle (0000)
000FC4B0 00b0:
	_point_in_pill2d (0000)
000FC560 00d0:
	_point_to_line_distance_squared3d (0000)
000FC630 0570:
	_vector_to_line_distance_squared3d (0000)
000FCBA0 0140:
	_sphere_test_vector3d (0000)
000FCCE0 01c0:
	_pill_test_vector3d (0000)
000FCEA0 00f0:
	_fast_vector_intersection_with_sphere (0000)
000FCF90 00c0:
	_point_in_triangle2d (0000)
000FD050 01b0:
	_point_in_triangle3d (0000)
000FD200 00b0:
	_point_in_cone2d (0000)
000FD2B0 00c0:
	_point_in_cone3d (0000)
000FD370 00a0:
	_point_in_sector2d (0000)
000FD410 00c0:
	_point_in_sector3d (0000)
000FD4D0 0390:
	_vector_intersects_pill2d (0000)
000FD860 0490:
	_vector_intersects_pill3d (0000)
000FDCF0 0220:
	_vector_intersects_triangle2d (0000)
000FDF10 01b0:
	_vector_intersects_triangle3d (0000)
000FE0C0 0090:
	_circle_intersects_rectangle2d (0000)
000FE150 00c0:
	_sphere_intersects_rectangle3d (0000)
000FE210 0120:
	_circle_intersects_triangle2d (0000)
000FE330 0260:
	_sphere_intersects_triangle3d (0000)
000FE590 0140:
	_pill_intersects_rectangle2d (0000)
000FE6D0 0120:
	_pill_intersects_triangle2d (0000)
000FE7F0 0340:
	_pill_intersects_triangle3d (0000)
000FEB30 0170:
	_point_from_planes3d (0000)
000FECA0 0130:
	_line_from_planes3d (0000)
000FEDD0 01c0:
	_accelerate_to_position (0000)
000FEF90 0240:
	_angular_accelerate_to_position (0000)
000FF1D0 0080:
	_accelerate_to_velocity3d (0000)
000FF250 00f0:
	_accerate_to_position3d (0000)
000FF340 0140:
	_quantize_real_to_byte_lower_bound (0000)
000FF480 0130:
	_quantize_real_to_byte_upper_bound (0000)
000FF5B0 00f0:
	_quantize_real_to_byte_rectangle3d (0000)
000FF6A0 0050:
	_valid_real_sine_cosine (0000)
000FF6F0 01f0:
	_pin_normal_to_cone3d (0000)
000FF8E0 0150:
	_circle_intersects_cone2d (0000)
000FFA30 0170:
	_sphere_intersects_cone3d (0000)
000FFBA0 0160:
	_circle_intersects_sector2d (0000)
000FFD00 0170:
	_sphere_intersects_sector3d (0000)
000FFE70 00e0:
	_accelerate_to_velocity (0000)
0027AFB0 00a8:
	_rdata_0027afb0 (0000)
	_global_projection3d_mappings (0090)
0027B058 0003:
	??_C@_02HNLLPFKA@up?$AA@ (0000)
0027B05C 0008:
	??_C@_07CJMBKHEI@forward?$AA@ (0000)
0027B064 0020:
	??_C@_0CA@CJADJNKK@c?3?2halo?2SOURCE?2math?2real_math?4c?$AA@ (0000)
0027B084 001d:
	??_C@_0BN@CGIKENHM@?$CIt1?5?$DO?$DN?50?40f?$CJ?5?$CG?$CG?5?$CIt1?5?$DM?$DN?51?40f?$CJ?$AA@ (0000)
0027B0A4 001d:
	??_C@_0BN@IJIIIJPN@?$CIt0?5?$DO?$DN?50?40f?$CJ?5?$CG?$CG?5?$CIt0?5?$DM?$DN?51?40f?$CJ?$AA@ (0000)
0027B0C4 0023:
	??_C@_0CD@KMDBEJKN@?$CId0?5?$DM?5REAL_MAX?$CJ?5?$HM?$HM?5?$CId1?5?$DM?5REAL_MA@ (0000)
0027B0E8 000d:
	??_C@_0N@OJFEJAHD@cosine?$DO?$DN0?40f?$AA@ (0000)
0027B0F8 007c:
	??_C@_0HM@KOBDCKBG@dequantize_byte_to_real?$CImin?0?5max@ (0000)
0027B174 001f:
	??_C@_0BP@GNFGOPKP@?$CFlf?5is?5not?5between?5?$CFlf?5and?5?$CFlf?$AA@ (0000)
0027B198 008c:
	??_C@_0IM@POGCMEOK@dequantize_byte_to_real?$CImin?0?5max@ (0000)
0027B228 0084:
	??_C@_0IE@ICHPOHNI@rectangle?9?$DOx1?$DN?$DNREAL_MIN?5?$CG?$CG?5recta@ (0000)
0027B2AC 002e:
	??_C@_0CO@CNINPCHF@?$CFs?0?5?$CFs?3?5assert_valid_real_sine_c@ (0000)
0027B2DC 0005:
	??_C@_04JLECFDLM@sine?$AA@ (0000)
0027B2E4 001a:
	??_C@_0BK@BKKKOIMF@sine?$DO0?40f?5?$CG?$CG?5cosine?$DO?$DN0?40f?$AA@ (0000)
0030797C 007c:
	_global_origin2d (0000)
	_global_x_axis2d (0004)
	_global_y_axis2d (0008)
	_global_negative_x_axis2d (000c)
	_global_negative_y_axis2d (0010)
	_global_zero_vector2d (0014)
	_global_forward2d (0018)
	_global_left2d (001c)
	_global_backward2d (0020)
	_global_right2d (0024)
	_global_origin3d (0028)
	_global_x_axis3d (002c)
	_global_y_axis3d (0030)
	_global_z_axis3d (0034)
	_global_negative_x_axis3d (0038)
	_global_negative_y_axis3d (003c)
	_global_negative_z_axis3d (0040)
	_global_zero_vector3d (0044)
	_global_forward3d (0048)
	_global_left3d (004c)
	_global_up3d (0050)
	_global_backward3d (0054)
	_global_right3d (0058)
	_global_down3d (005c)
	_global_zero_angles2d (0060)
	_global_zero_angles3d (0064)
	_global_identity_quaternion (0068)
	_global_identity4x3 (006c)
	_global_negative_identity4x3 (0070)
	_global_null_rectangle2d (0074)
	_global_null_rectangle3d (0078)
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static const real_matrix4x3 private_identity4x3 =
{
	1.f,
	{
		{ 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f }
	},
};

static const real_matrix4x3 private_negative_identity4x3 =
{
	1.f,
	{
		{ -1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f }
	},
};


const real_point3d *const global_origin3d= (const real_point3d *const)&private_identity4x3.position;

const real_point3d *const global_x_axis3d= (const real_point3d *const)&private_identity4x3.forward;
const real_point3d *const global_y_axis3d= (const real_point3d *const)&private_identity4x3.left;
const real_point3d *const global_z_axis3d= (const real_point3d *const)&private_identity4x3.up;
const real_point3d *const global_negative_x_axis3d= (const real_point3d *const)&private_negative_identity4x3.forward;
const real_point3d *const global_negative_y_axis3d= (const real_point3d *const)&private_negative_identity4x3.left;
const real_point3d *const global_negative_z_axis3d= (const real_point3d *const)&private_negative_identity4x3.up;

const real_vector3d *const global_zero_vector3d= (const real_vector3d *const)&private_identity4x3.position;

const real_vector3d *const global_forward3d= (const real_vector3d *const)&private_identity4x3.forward;
const real_vector3d *const global_left3d= (const real_vector3d *const)&private_identity4x3.left;
const real_vector3d *const global_up3d= (const real_vector3d *const)&private_identity4x3.up;

const real_vector3d *const global_backward3d= (const real_vector3d *const)&private_negative_identity4x3.forward;
const real_vector3d *const global_right3d= (const real_vector3d *const)&private_negative_identity4x3.left;
const real_vector3d *const global_down3d= (const real_vector3d *const)&private_negative_identity4x3.up;

/* ---------- public code */

/* ---------- private code */
