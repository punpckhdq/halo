/*
RENDER_CAMERAS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render_cameras.h"
#include "render.h"
#include "render_debug.h"
#include "objects.h"
#include "collision_bsp.h"
#include "rasterizer.h"
#include "input.h"
#include "editor_stubs.h"

/* ---------- constants */

enum
{
	MAXIMUM_RENDER_CAMERA_WARNING_CONDITIONS = 64,
};

enum
{
	_frustum_left_bit = 0,	/* fake name */
	_frustum_right_bit,		/* fake name */
	_frustum_top_bit,		/* fake name */
	_frustum_bottom_bit,	/* fake name */
	_frustum_near_bit,		/* fake name */
	_frustum_far_bit,		/* fake name */
	NUMBER_OF_FRUSTUM_BITS	/* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void render_camera_warn_once(short id, real value);

/* ---------- globals */

static boolean render_camera_warnings_initialized;
static real render_camera_warning_values[MAXIMUM_RENDER_CAMERA_WARNING_CONDITIONS];
static real previous_projection_coefficients[4];

boolean debug_no_frustum_clip = FALSE;
boolean render_camera_debug_this_fucking_frustum = FALSE;

/* ---------- public code */

static void render_camera_warn_once(
	short id,
	real value)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 135, id>=0 && id<MAXIMUM_RENDER_CAMERA_WARNING_CONDITIONS);

	if (!render_camera_warnings_initialized)
	{
		memset(render_camera_warning_values, 0, sizeof(render_camera_warning_values));
		render_camera_warnings_initialized = TRUE;
	}

	if (value >= 0.05f && value > render_camera_warning_values[id])
	{
		error(_error_silent, "### ERROR cameras: frustum-integrity condition #%d violated by %f", id, value);
		render_camera_warning_values[id] = value;
	}

	return;
}

void render_camera_new(
	struct render_camera *camera)
{
	memset(camera, 0, sizeof(*camera));

	return;
}

void render_camera_mirror(
	struct render_camera const *camera,
	struct render_mirror const *mirror,
	struct render_camera *result)
{
	real distance;
	real_plane3d plane = mirror->plane;

	*result = *camera;

	if (mirror->index_of_refraction == 0.f)
	{
		real_plane3d adjusted_plane;
		real const minimum_dot = 0.0125f;
		real const offset = 0.005859375f;

		if (fabs(dot_product3d(&plane.n, &camera->forward)) < minimum_dot)
		{
			real_vector3d adjusted_normal;
			real_point3d point;

			distance = -plane3d_distance_to_point(&plane, &camera->position);
			point_from_line3d(&camera->position, &plane.n, distance, &point);
			vector_from_line3d(&plane.n, &camera->forward, offset, &adjusted_normal);
			normalize3d(&adjusted_normal);
			plane3d_from_point_and_normal(&adjusted_plane, &point, &adjusted_normal);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 207, valid_real_plane3d(&adjusted_plane));
		}
		else
		{
			adjusted_plane = plane;
		}

		reflect_vector3d(&camera->forward, &adjusted_plane.n, &result->forward);
		reflect_vector3d(&camera->up, &adjusted_plane.n, &result->up);
		match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 217, valid_real_vector3d(&result->forward));
		match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 218, valid_real_vector3d(&result->up));
		point_from_line3d(
			&camera->position,
			&adjusted_plane.n,
			plane3d_distance_to_point(&adjusted_plane, &camera->position) * -2.f,
			&result->position);
		result->mirrored = !camera->mirrored;
		negate_vector3d(&result->up, &result->up);
	}
	else
	{
		real_vector3d cross;
		real magnitude = magnitude3d(&camera->forward);
		real inverse_magnitude = 1.f / magnitude;
		real sine_angle = magnitude3d(cross_product3d(&camera->forward, &plane.n, &cross)) * inverse_magnitude;
		real cosine_angle = dot_product3d(&camera->forward, &plane.n) * inverse_magnitude;
		real refracted_sine = mirror->index_of_refraction * sine_angle;
		real refracted_cosine = square_root(1.f - refracted_sine * refracted_sine);

		if (sine_angle != 0.f)
		{
			distance = -mirror->depth * (refracted_sine * cosine_angle) / (refracted_cosine * sine_angle);
		}
		else
		{
			distance = 0.f;
		}

		point_from_line3d(&camera->position, &plane.n, distance, &result->position);
	}

	result->z_near = 0.f;
	result->mirror_plane = plane;

	return;
}

void render_camera_hack_frustum_z(
	struct render_frustum *frustum,
	real z_near,
	real z_far)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 271, frustum && frustum->projection_valid);

	if (z_near == -1.f && z_far == -1.f)
	{
		previous_projection_coefficients[0] = frustum->projection_matrix[0][2];
		previous_projection_coefficients[1] = frustum->projection_matrix[1][2];
		previous_projection_coefficients[2] = frustum->projection_matrix[2][2];
		previous_projection_coefficients[3] = frustum->projection_matrix[3][2];
	}
	else if (z_near == 0.f && z_far == 0.f)
	{
		frustum->projection_matrix[0][2] = previous_projection_coefficients[0];
		frustum->projection_matrix[1][2] = previous_projection_coefficients[1];
		frustum->projection_matrix[2][2] = previous_projection_coefficients[2];
		frustum->projection_matrix[3][2] = previous_projection_coefficients[3];
	}
	else
	{
		frustum->projection_matrix[0][2] = 0.f;
		frustum->projection_matrix[1][2] = 0.f;
		frustum->projection_matrix[2][2] = -((z_near + z_far) / (z_far - z_near));
		frustum->projection_matrix[3][2] = z_near * z_far * -2.f / (z_far - z_near);
	}

	return;
}

void render_camera_build_frustum_bounds(
	struct render_camera const *camera,
	real_rectangle2d *frustum_bounds)
{
	rectangle2d const *viewport_bounds;
	rectangle2d const *window_bounds;
	real aspect_ratio;
	real scale;
	real y0;

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 305, camera);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 306, frustum_bounds);

	viewport_bounds = &camera->viewport_bounds;
	window_bounds = &camera->window_bounds;
	aspect_ratio = (real)(viewport_bounds->y1 - viewport_bounds->y0) / (viewport_bounds->x1 - viewport_bounds->x0);
	scale = 1.f / (window_bounds->y1 - window_bounds->y0);
	frustum_bounds->x0 = (2 * viewport_bounds->x0 - window_bounds->x0 - window_bounds->x1) * scale;
	frustum_bounds->x1 = (2 * viewport_bounds->x1 - window_bounds->x0 - window_bounds->x1) * scale;
	frustum_bounds->y0 = (2 * viewport_bounds->y0 - window_bounds->y0 - window_bounds->y1) * scale;
	frustum_bounds->y1 = (2 * viewport_bounds->y1 - window_bounds->y0 - window_bounds->y1) * scale;
	frustum_bounds->x0 *= aspect_ratio;
	frustum_bounds->x1 *= aspect_ratio;
	y0 = frustum_bounds->y0;
	frustum_bounds->y0 = -frustum_bounds->y1;
	frustum_bounds->y1 = -y0;

	return;
}

boolean render_camera_build_clipped_frustum_bounds(
	struct render_camera const *camera,
	real_rectangle2d const *clip,
	real_rectangle2d *frustum_bounds)
{
	boolean empty = TRUE;

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 342, camera);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 343, clip);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 344, frustum_bounds);

	if (!debug_no_frustum_clip && clip->x0 < clip->x1 && clip->y0 < clip->y1)
	{
		real scale = 1.f / tangent(camera->vertical_field_of_view * 0.5f);
		real aspect_ratio = (real)(camera->viewport_bounds.y1 - camera->viewport_bounds.y0) / (camera->viewport_bounds.x1 - camera->viewport_bounds.x0);

		frustum_bounds->x0 = clip->x0 * (scale * aspect_ratio);
		frustum_bounds->x1 = clip->x1 * (scale * aspect_ratio);
		frustum_bounds->y0 = clip->y0 * scale;
		frustum_bounds->y1 = clip->y1 * scale;
		empty = frustum_bounds->x0 >= frustum_bounds->x1 || frustum_bounds->y0 >= frustum_bounds->y1;
	}

	if (empty)
	{
		frustum_bounds->y0 = -1.f;
		frustum_bounds->x0 = -1.f;
		frustum_bounds->y1 = 1.f;
		frustum_bounds->x1 = 1.f;
	}

	return !empty;
}

void render_camera_build_frustum(
	struct render_camera const *camera,
	real_rectangle2d const *frustum_bounds,
	struct render_frustum *frustum,
	boolean build_projection)
{
	real half_width;
	real half_height;
	real center_x;
	real center_y;
	real x_scale;
	real y_scale;
	real tangent_value;
	real viewport_width = (real)(camera->viewport_bounds.x1 - camera->viewport_bounds.x0);
	real viewport_height = (real)(camera->viewport_bounds.y1 - camera->viewport_bounds.y0);

	if (frustum_bounds)
	{
		frustum->frustum_bounds = *frustum_bounds;
	}
	else
	{
		frustum->frustum_bounds.y0 = -1.f;
		frustum->frustum_bounds.x0 = -1.f;
		frustum->frustum_bounds.y1 = 1.f;
		frustum->frustum_bounds.x1 = 1.f;
	}

	half_width = (frustum->frustum_bounds.x1 - frustum->frustum_bounds.x0) * 0.5f;
	half_height = (frustum->frustum_bounds.y1 - frustum->frustum_bounds.y0) * 0.5f;
	center_x = -(frustum->frustum_bounds.x1 + frustum->frustum_bounds.x0) * 0.5f / half_width;
	center_y = -(frustum->frustum_bounds.y1 + frustum->frustum_bounds.y0) * 0.5f / half_height;
	tangent_value = tangent(camera->vertical_field_of_view * 0.5f);
	half_width = viewport_width / viewport_height * half_width;
	x_scale = 1.f / (half_width * tangent_value);
	y_scale = 1.f / (half_height * tangent_value);

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 430, camera->vertical_field_of_view<_pi - _real_epsilon);
	match_vassert(
		"c:\\halo\\SOURCE\\render\\render_cameras.c",
		432,
		camera->vertical_field_of_view > _real_epsilon,
		csprintf(
			temporary,
			"### FATAL ERROR: field of view set to %f (0x%x)",
			camera->vertical_field_of_view,
			*(long *)&camera->vertical_field_of_view));
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 433, camera->z_near>=0.0f);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 434, camera->z_far>camera->z_near);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 435, camera->viewport_bounds.x0<camera->viewport_bounds.x1);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 436, camera->viewport_bounds.y0<camera->viewport_bounds.y1);

	{
		real_vector3d x_axis;
		real_vector3d y_axis;
		real_vector3d z_axis;

		cross_product3d(&camera->forward, &camera->up, &x_axis);
		cross_product3d(&x_axis, &camera->forward, &y_axis);
		negate_vector3d(&camera->forward, &z_axis);
		normalize3d(&x_axis);
		normalize3d(&y_axis);
		normalize3d(&z_axis);
		frustum->view_to_world.forward = x_axis;
		frustum->view_to_world.left = y_axis;
		frustum->view_to_world.up = z_axis;
		frustum->view_to_world.position = camera->position;
		frustum->view_to_world.scale = 1.f;
		matrix4x3_inverse(&frustum->view_to_world, &frustum->world_to_view);
		match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 458, valid_real_matrix4x3(&frustum->world_to_view));
		match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 459, valid_real_matrix4x3(&frustum->view_to_world));
	}

	{
		real_vector3d viewspace_normal;
		real_plane3d viewspace_plane;

		set_real_vector3d(&viewspace_normal, -x_scale, 0.f, center_x + 1.f);
		normalize3d(&viewspace_normal);
		plane3d_from_point_and_normal(&viewspace_plane, global_origin3d, &viewspace_normal);
		matrix4x3_transform_plane(&frustum->view_to_world, &viewspace_plane, &frustum->world_planes[0]);
		set_real_vector3d(&viewspace_normal, x_scale, 0.f, 1.f - center_x);
		normalize3d(&viewspace_normal);
		plane3d_from_point_and_normal(&viewspace_plane, global_origin3d, &viewspace_normal);
		matrix4x3_transform_plane(&frustum->view_to_world, &viewspace_plane, &frustum->world_planes[1]);
		set_real_vector3d(&viewspace_normal, 0.f, -y_scale, center_y + 1.f);
		normalize3d(&viewspace_normal);
		plane3d_from_point_and_normal(&viewspace_plane, global_origin3d, &viewspace_normal);
		matrix4x3_transform_plane(&frustum->view_to_world, &viewspace_plane, &frustum->world_planes[2]);
		set_real_vector3d(&viewspace_normal, 0.f, y_scale, 1.f - center_y);
		normalize3d(&viewspace_normal);
		plane3d_from_point_and_normal(&viewspace_plane, global_origin3d, &viewspace_normal);
		matrix4x3_transform_plane(&frustum->view_to_world, &viewspace_plane, &frustum->world_planes[3]);
		set_real_vector3d(&viewspace_plane.n, 0.f, 0.f, 1.f);
		viewspace_plane.d = -camera->z_near;
		matrix4x3_transform_plane(&frustum->view_to_world, &viewspace_plane, &frustum->world_planes[4]);
		set_real_vector3d(&viewspace_plane.n, 0.f, 0.f, -1.f);
		viewspace_plane.d = camera->z_far;
		matrix4x3_transform_plane(&frustum->view_to_world, &viewspace_plane, &frustum->world_planes[5]);
		frustum->z_near = camera->z_near;
		frustum->z_far = camera->z_far;
	}

	{
		real_rectangle2d projected_bounds;
		real_point3d frustum_vertex;
		real_point3d frustum_midpoint;
		real far_x = -camera->z_far / x_scale;
		real far_y = -camera->z_far / y_scale;
		real midpoint_distance = (camera->z_far + camera->z_near) * 0.5f;

		projected_bounds.x0 = (center_x + 1.f) * far_x;
		projected_bounds.x1 = (center_x - 1.f) * far_x;
		projected_bounds.y0 = (center_y + 1.f) * far_y;
		projected_bounds.y1 = (center_y - 1.f) * far_y;
		set_real_point3d(&frustum_vertex, projected_bounds.x0, projected_bounds.y0, -camera->z_far);
		matrix4x3_transform_point(&frustum->view_to_world, &frustum_vertex, &frustum->world_vertices[0]);
		set_real_point3d(&frustum_vertex, projected_bounds.x1, projected_bounds.y0, -camera->z_far);
		matrix4x3_transform_point(&frustum->view_to_world, &frustum_vertex, &frustum->world_vertices[1]);
		set_real_point3d(&frustum_vertex, projected_bounds.x0, projected_bounds.y1, -camera->z_far);
		matrix4x3_transform_point(&frustum->view_to_world, &frustum_vertex, &frustum->world_vertices[2]);
		set_real_point3d(&frustum_vertex, projected_bounds.x1, projected_bounds.y1, -camera->z_far);
		matrix4x3_transform_point(&frustum->view_to_world, &frustum_vertex, &frustum->world_vertices[3]);
		frustum->world_vertices[4] = camera->position;
		set_real_point3d(
			&frustum_midpoint,
			-midpoint_distance * center_x / x_scale,
			-midpoint_distance * center_y / y_scale,
			-midpoint_distance);
		matrix4x3_transform_point(&frustum->view_to_world, &frustum_midpoint, &frustum->world_midpoint);
	}

	{
		short vertex_index;

		frustum->world_bounds.x1 = frustum->world_vertices[0].x;
		frustum->world_bounds.x0 = frustum->world_bounds.x1;
		frustum->world_bounds.y1 = frustum->world_vertices[0].y;
		frustum->world_bounds.y0 = frustum->world_bounds.y1;
		frustum->world_bounds.z1 = frustum->world_vertices[0].z;
		frustum->world_bounds.z0 = frustum->world_bounds.z1;

		for (vertex_index = 1; vertex_index < NUMBEROF(frustum->world_vertices); vertex_index++)
		{
			frustum->world_bounds.x0 = MIN(frustum->world_bounds.x0, frustum->world_vertices[vertex_index].x);
			frustum->world_bounds.y0 = MIN(frustum->world_bounds.y0, frustum->world_vertices[vertex_index].y);
			frustum->world_bounds.z0 = MIN(frustum->world_bounds.z0, frustum->world_vertices[vertex_index].z);
			frustum->world_bounds.x1 = MAX(frustum->world_bounds.x1, frustum->world_vertices[vertex_index].x);
			frustum->world_bounds.y1 = MAX(frustum->world_bounds.y1, frustum->world_vertices[vertex_index].y);
			frustum->world_bounds.z1 = MAX(frustum->world_bounds.z1, frustum->world_vertices[vertex_index].z);
		}
	}

	if (build_projection)
	{
		real_plane3d clip_plane;
		real near_distance;
		real far_distance;
		real projection_scale;

		if (camera->z_near == 0.f)
		{
			matrix4x3_transform_plane(&frustum->world_to_view, &camera->mirror_plane, &clip_plane);
		}
		else
		{
			clip_plane.n.i = 0.f;
			clip_plane.n.j = 0.f;
			clip_plane.n.k = 1.f;
			clip_plane.d = -camera->z_near;
		}

		near_distance = -clip_plane.d / clip_plane.n.k;
		far_distance = camera->z_far;
		projection_scale = far_distance / ((far_distance - near_distance) * (fabs(clip_plane.n.i / clip_plane.n.k) + 1.f + fabs(clip_plane.n.j / clip_plane.n.k)));
		clip_plane.n.i = projection_scale * clip_plane.n.i / clip_plane.n.k;
		clip_plane.n.j = projection_scale * clip_plane.n.j / clip_plane.n.k;
		clip_plane.n.k = projection_scale;
		clip_plane.d = -projection_scale * near_distance;

		if (clip_plane.d > 0.f && camera->z_near == 0.f)
		{
			clip_plane.n.i = -clip_plane.n.i;
			clip_plane.n.j = -clip_plane.n.j;
			clip_plane.n.k = -clip_plane.n.k;
			clip_plane.d = -clip_plane.d;
		}

		memset(frustum->projection_matrix, 0, sizeof(frustum->projection_matrix));
		frustum->projection_matrix[0][0] = x_scale;
		frustum->projection_matrix[0][2] = -clip_plane.n.i;
		frustum->projection_matrix[1][1] = y_scale;
		frustum->projection_matrix[1][2] = -clip_plane.n.j;
		frustum->projection_matrix[2][0] = -center_x;
		frustum->projection_matrix[2][1] = -center_y;
		frustum->projection_matrix[2][2] = -clip_plane.n.k;
		frustum->projection_matrix[2][3] = -1.f;
		frustum->projection_matrix[3][2] = clip_plane.d;
		frustum->projection_valid = TRUE;
		frustum->projection_world_to_screen.i = viewport_width * x_scale * 0.5f;
		frustum->projection_world_to_screen.j = viewport_height * y_scale * 0.5f;
	}
	else
	{
		memset(frustum->projection_matrix, 0, sizeof(frustum->projection_matrix));
		memset(&frustum->projection_world_to_screen, 0, sizeof(frustum->projection_world_to_screen));
		frustum->projection_valid = FALSE;
	}

	{
		short condition_index = 0;

		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[0], &frustum->world_vertices[0])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[0], &frustum->world_vertices[2])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[0], &frustum->world_vertices[4])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[1], &frustum->world_vertices[1])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[1], &frustum->world_vertices[3])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[1], &frustum->world_vertices[4])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[2], &frustum->world_vertices[0])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[2], &frustum->world_vertices[1])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[2], &frustum->world_vertices[4])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[3], &frustum->world_vertices[2])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[3], &frustum->world_vertices[3])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[3], &frustum->world_vertices[4])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[5], &frustum->world_vertices[0])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[5], &frustum->world_vertices[1])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[5], &frustum->world_vertices[2])));
		render_camera_warn_once(
			condition_index++,
			(real)fabs(plane3d_distance_to_point(&frustum->world_planes[5], &frustum->world_vertices[3])));
		render_camera_warn_once(
			condition_index++,
			plane3d_distance_to_point(&frustum->world_planes[0], &frustum->world_midpoint));
		render_camera_warn_once(
			condition_index++,
			plane3d_distance_to_point(&frustum->world_planes[1], &frustum->world_midpoint));
		render_camera_warn_once(
			condition_index++,
			plane3d_distance_to_point(&frustum->world_planes[2], &frustum->world_midpoint));
		render_camera_warn_once(
			condition_index++,
			plane3d_distance_to_point(&frustum->world_planes[3], &frustum->world_midpoint));
		render_camera_warn_once(
			condition_index++,
			plane3d_distance_to_point(&frustum->world_planes[4], &frustum->world_midpoint));
		render_camera_warn_once(
			condition_index++,
			plane3d_distance_to_point(&frustum->world_planes[5], &frustum->world_midpoint));
	}

	return;
}

boolean render_camera_triangle_frontfacing(
	struct render_camera const *camera,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2)
{
	real_vector3d view_to_triangle;
	real_vector3d e0;
	real_vector3d e1;
	real_vector3d normal;

	vector_from_points3d(&camera->position, p0, &view_to_triangle);
	vector_from_points3d(p0, p1, &e0);
	vector_from_points3d(p1, p2, &e1);
	cross_product3d(&e0, &e1, &normal);

	return dot_product3d(&normal, &view_to_triangle) > -_real_epsilon;
}

__inline short render_frustum_build_point_flags(
	struct render_frustum const *frustum,
	real_point3d const *point)
{
	short flags = plane3d_distance_to_point(&frustum->world_planes[0], point) > 0.f ? FLAG(_frustum_left_bit) : 0;

	flags |= plane3d_distance_to_point(&frustum->world_planes[1], point) > 0.f ? FLAG(_frustum_right_bit) : 0;
	flags |= plane3d_distance_to_point(&frustum->world_planes[2], point) > 0.f ? FLAG(_frustum_bottom_bit) : 0;
	flags |= plane3d_distance_to_point(&frustum->world_planes[3], point) > 0.f ? FLAG(_frustum_top_bit) : 0;

	return flags;
}

boolean render_frustum_triangle_visible(
	struct render_frustum const *frustum,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2)
{
	short and_flags = MASK(NUMBER_OF_FRUSTUM_BITS);
	short flags = render_frustum_build_point_flags(frustum, p0);

	if (!flags)
	{
		return TRUE;
	}

	and_flags &= flags;
	flags = render_frustum_build_point_flags(frustum, p1);

	if (!flags)
	{
		return TRUE;
	}

	and_flags &= flags;
	flags = render_frustum_build_point_flags(frustum, p2);

	if (!flags)
	{
		return TRUE;
	}

	return !(and_flags & flags);
}

short render_frustum_cube_visible(
	struct render_frustum const *frustum,
	real_rectangle3d const *bounds,
	boolean test_frustum_against_cube)
{
	real_point3d bounding_vertices[8];
	short point_index;
	short and_flags;
	short or_flags;

	if (frustum->world_bounds.x1 < bounds->x0)
	{
		return 0;
	}

	if (frustum->world_bounds.y1 < bounds->y0)
	{
		return 0;
	}

	if (frustum->world_bounds.z1 < bounds->z0)
	{
		return 0;
	}

	if (frustum->world_bounds.x0 > bounds->x1)
	{
		return 0;
	}

	if (frustum->world_bounds.y0 > bounds->y1)
	{
		return 0;
	}

	if (frustum->world_bounds.z0 > bounds->z1)
	{
		return 0;
	}

	and_flags = MASK(NUMBER_OF_FRUSTUM_BITS);
	or_flags = 0;
	bounding_vertices[6].x = bounds->x0;
	bounding_vertices[4].x = bounding_vertices[6].x;
	bounding_vertices[2].x = bounding_vertices[4].x;
	bounding_vertices[0].x = bounding_vertices[2].x;
	bounding_vertices[7].x = bounds->x1;
	bounding_vertices[5].x = bounding_vertices[7].x;
	bounding_vertices[3].x = bounding_vertices[5].x;
	bounding_vertices[1].x = bounding_vertices[3].x;
	bounding_vertices[5].y = bounds->y0;
	bounding_vertices[4].y = bounding_vertices[5].y;
	bounding_vertices[1].y = bounding_vertices[4].y;
	bounding_vertices[0].y = bounding_vertices[1].y;
	bounding_vertices[7].y = bounds->y1;
	bounding_vertices[6].y = bounding_vertices[7].y;
	bounding_vertices[3].y = bounding_vertices[6].y;
	bounding_vertices[2].y = bounding_vertices[3].y;
	bounding_vertices[3].z = bounds->z0;
	bounding_vertices[2].z = bounding_vertices[3].z;
	bounding_vertices[1].z = bounding_vertices[2].z;
	bounding_vertices[0].z = bounding_vertices[1].z;
	bounding_vertices[7].z = bounds->z1;
	bounding_vertices[6].z = bounding_vertices[7].z;
	bounding_vertices[5].z = bounding_vertices[6].z;
	bounding_vertices[4].z = bounding_vertices[5].z;

	for (point_index = 0; point_index < NUMBEROF(bounding_vertices); point_index++)
	{
		real_point3d const *point = &bounding_vertices[point_index];
		short flags = plane3d_distance_to_point(&frustum->world_planes[0], point) > 0.f ? FLAG(_frustum_left_bit) : 0;

		flags |= plane3d_distance_to_point(&frustum->world_planes[1], point) > 0.f ? FLAG(_frustum_right_bit) : 0;
		flags |= plane3d_distance_to_point(&frustum->world_planes[2], point) > 0.f ? FLAG(_frustum_bottom_bit) : 0;
		flags |= plane3d_distance_to_point(&frustum->world_planes[3], point) > 0.f ? FLAG(_frustum_top_bit) : 0;
		and_flags &= flags;
		or_flags |= flags;
	}

	if (!or_flags)
	{
		return 2;
	}

	if (and_flags)
	{
		return 0;
	}

	if (test_frustum_against_cube)
	{
		short vertex_index;
		short vertex_and_flags = MASK(NUMBER_OF_FRUSTUM_BITS);

		for (vertex_index = 0; vertex_index < NUMBEROF(frustum->world_vertices); vertex_index++)
		{
			real_point3d const *vertex = &frustum->world_vertices[vertex_index];
			short flags = vertex->x <= bounds->x0 ? FLAG(_frustum_left_bit) : 0;

			flags |= vertex->x >= bounds->x1 ? FLAG(_frustum_right_bit) : 0;
			flags |= vertex->y <= bounds->y0 ? FLAG(_frustum_bottom_bit) : 0;
			flags |= vertex->y >= bounds->y1 ? FLAG(_frustum_top_bit) : 0;
			flags |= vertex->z <= bounds->z0 ? FLAG(_frustum_near_bit) : 0;
			flags |= vertex->z >= bounds->z1 ? FLAG(_frustum_far_bit) : 0;
			vertex_and_flags &= flags;
		}

		if (vertex_and_flags)
		{
			return 0;
		}
	}

	return 1;
}

short render_frustum_sphere_visible(
	struct render_frustum const *frustum,
	real_point3d const *point,
	real radius)
{
	real distance0;
	real distance1;
	real distance2;
	real distance3;
	real distance4;
	real distance5;

	if (point->x - radius > frustum->world_bounds.x1)
	{
		return 0;
	}

	if (point->y - radius > frustum->world_bounds.y1)
	{
		return 0;
	}

	if (point->z - radius > frustum->world_bounds.z1)
	{
		return 0;
	}

	if (frustum->world_bounds.x0 > point->x + radius)
	{
		return 0;
	}

	if (frustum->world_bounds.y0 > point->y + radius)
	{
		return 0;
	}

	if (frustum->world_bounds.z0 > point->z + radius)
	{
		return 0;
	}

	distance0 = plane3d_distance_to_point(&frustum->world_planes[0], point);

	if (distance0 > radius)
	{
		return 0;
	}

	distance1 = plane3d_distance_to_point(&frustum->world_planes[1], point);

	if (distance1 > radius)
	{
		return 0;
	}

	distance2 = plane3d_distance_to_point(&frustum->world_planes[2], point);

	if (distance2 > radius)
	{
		return 0;
	}

	distance3 = plane3d_distance_to_point(&frustum->world_planes[3], point);

	if (distance3 > radius)
	{
		return 0;
	}

	distance4 = plane3d_distance_to_point(&frustum->world_planes[4], point);

	if (distance4 > radius)
	{
		return 0;
	}

	distance5 = plane3d_distance_to_point(&frustum->world_planes[5], point);

	if (distance5 > radius)
	{
		return 0;
	}

	if (distance0 < -radius && distance1 < -radius && distance2 < -radius && distance3 < -radius && distance5 < -radius)
	{
		return 2;
	}

	return 1;
}

real render_frustum_sphere_diameter_in_pixels(
	struct render_frustum const *frustum,
	real_point3d const *point,
	real radius)
{
	real distance = ABS(frustum->world_to_view.forward.k * point->x + frustum->world_to_view.left.k * point->y + frustum->world_to_view.up.k * point->z + frustum->world_to_view.position.z);

	return frustum->projection_world_to_screen.j / MAX(distance, 0.1f) * radius * 2.f;
}

real render_frustum_cube_view_fraction(
	struct render_frustum const *frustum,
	real_rectangle3d const *bounds)
{
	real_rectangle2d bounds2d;
	real x_scale;
	real x_offset;
	real y_scale;
	real y_offset;
	real inverse_z0;
	real inverse_z1;
	real fraction;

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 876, frustum);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 877, bounds);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 878, bounds->x0<=bounds->x1);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 879, bounds->y0<=bounds->y1);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 880, bounds->z0<=bounds->z1);

	if (!(bounds->z0 >= 0.f))
	{
		if (!(bounds->z1 >= 0.f))
		{
			x_scale = frustum->projection_matrix[0][0];
			x_offset = frustum->projection_matrix[2][0];
			y_scale = frustum->projection_matrix[1][1];
			y_offset = frustum->projection_matrix[2][1];
			inverse_z0 = 1.f / bounds->z0;
			inverse_z1 = 1.f / bounds->z1;

			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 900, frustum->projection_valid);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 903, frustum->projection_matrix[1][0]==0.0f);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 904, frustum->projection_matrix[3][0]==0.0f);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 905, frustum->projection_matrix[0][1]==0.0f);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 906, frustum->projection_matrix[3][1]==0.0f);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 907, frustum->projection_matrix[0][3]==0.0f);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 908, frustum->projection_matrix[1][3]==0.0f);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 909, frustum->projection_matrix[2][3]==-1.0f);
			match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 910, frustum->projection_matrix[3][3]==0.0f);

			bounds2d.x0 = MAX(
				MIN(
					(x_scale * bounds->x0 + x_offset * bounds->z0) * (-inverse_z0),
					(x_scale * bounds->x0 + x_offset * bounds->z1) * (-inverse_z1)),
				-1.f);
			bounds2d.y0 = MAX(
				MIN(
					(y_scale * bounds->y0 + y_offset * bounds->z0) * (-inverse_z0),
					(y_scale * bounds->y0 + y_offset * bounds->z1) * (-inverse_z1)),
				-1.f);
			bounds2d.x1 = MIN(
				MAX(
					(x_scale * bounds->x1 + x_offset * bounds->z0) * (-inverse_z0),
					(x_scale * bounds->x1 + x_offset * bounds->z1) * (-inverse_z1)),
				1.f);
			bounds2d.y1 = MIN(
				MAX(
					(y_scale * bounds->y1 + y_offset * bounds->z0) * (-inverse_z0),
					(y_scale * bounds->y1 + y_offset * bounds->z1) * (-inverse_z1)),
				1.f);
			fraction = MAX((bounds2d.x1 - bounds2d.x0) * (bounds2d.y1 - bounds2d.y0) / 4.f, 0.f);
		}
		else
		{
			fraction = 1.f;
		}
	}
	else
	{
		fraction = 0.f;
	}

	return fraction;
}

void render_frustum_get_projection_bounds(
	struct render_frustum const *frustum,
	real_rectangle2d *bounds)
{
	real x_offset;
	real y_offset;
	real x_scale;
	real y_scale;

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 935, frustum && frustum->projection_valid);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 936, bounds);

	x_offset = -frustum->projection_matrix[2][0];
	y_offset = -frustum->projection_matrix[2][1];
	x_scale = -1.f / frustum->projection_matrix[0][0];
	y_scale = -1.f / frustum->projection_matrix[1][1];
	bounds->x0 = (x_offset - 1.f) * x_scale;
	bounds->x1 = (x_offset + 1.f) * x_scale;
	bounds->y0 = (y_offset - 1.f) * y_scale;
	bounds->y1 = (y_offset + 1.f) * y_scale;

	return;
}

boolean render_camera_world_to_screen(
	struct render_camera const *camera,
	struct render_frustum const *frustum,
	real_point3d const *world_point,
	real_point2d *screen_point)
{
	real_point3d view_point;

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 961, camera);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 962, frustum);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 963, world_point);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 964, screen_point);

	matrix4x3_transform_point(&frustum->world_to_view, world_point, &view_point);

	return render_camera_view_to_screen(camera, frustum, &view_point, screen_point);
}

boolean render_camera_view_to_screen(
	struct render_camera const *camera,
	struct render_frustum const *frustum,
	real_point3d const *view_point,
	real_point2d *screen_point)
{
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 978, camera);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 979, frustum);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 980, view_point);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 981, screen_point);

	if (view_point->z < 0.f)
	{
		real scale = -1.f / view_point->z;

		match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 988, frustum->projection_valid);

		screen_point->x = (frustum->projection_matrix[0][0] * view_point->x + frustum->projection_matrix[2][0] * view_point->z) * scale;
		screen_point->y = -((frustum->projection_matrix[1][1] * view_point->y + frustum->projection_matrix[2][1] * view_point->z) * scale);

		if (screen_point->x >= -1.f && screen_point->x <= 1.f && screen_point->y >= -1.f && screen_point->y <= 1.f)
		{
			real viewport_width = (real)(camera->viewport_bounds.x1 - camera->viewport_bounds.x0);
			real viewport_height = (real)(camera->viewport_bounds.y1 - camera->viewport_bounds.y0);

			screen_point->x = viewport_width * ((screen_point->x + 1.f) * 0.5f) + camera->viewport_bounds.x0;
			screen_point->y = viewport_height * ((screen_point->y + 1.f) * 0.5f) + camera->viewport_bounds.y0;
			result = TRUE;
		}
	}

	return result;
}

void render_camera_screen_to_view(
	struct render_camera const *camera,
	struct render_frustum const *frustum,
	real_point2d const *screen_point,
	real_vector3d *view_vector)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1018, camera);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1019, frustum);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1020, screen_point);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1021, view_vector);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1028, frustum->projection_valid);

	view_vector->i = (screen_point->x - frustum->projection_matrix[2][0]) / frustum->projection_matrix[0][0];
	view_vector->j = -(frustum->projection_matrix[2][1] + screen_point->y) / frustum->projection_matrix[1][1];
	view_vector->k = -1.f;

	return;
}

void render_camera_screen_to_world(
	struct render_camera const *camera,
	struct render_frustum const *frustum,
	real_point2d const *screen_point,
	real_point3d *world_point,
	real_vector3d *world_vector)
{
	real_vector3d view_vector;

	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1052, camera);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1053, frustum);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1054, screen_point);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1055, world_point);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1056, world_vector);
	match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1058, frustum->projection_valid);

	render_camera_screen_to_view(camera, frustum, screen_point, &view_vector);
	*world_point = camera->position;
	matrix4x3_transform_vector(&frustum->view_to_world, &view_vector, world_vector);

	return;
}

real render_camera_get_adjusted_field_of_view_tangent(
	real field_of_view)
{
	return tangent(field_of_view * 0.5f) * 0.85f;
}

void render_camera_debug_frustum(
	struct render_camera const *camera,
	struct render_frustum const *frustum)
{
	if (render_camera_debug_this_fucking_frustum)
	{
		real_point3d frustum_grid[3][3];
		short i;
		short j;

		match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1083, camera);
		match_assert("c:\\halo\\SOURCE\\render\\render_cameras.c", 1084, frustum);

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				set_real_point3d(
					&frustum_grid[j][i],
					(j - 1) * tangent(camera->vertical_field_of_view * 0.5f) * (camera->window_bounds.x1 - camera->window_bounds.x0) / (camera->window_bounds.y1 - camera->window_bounds.y0),
					(i - 1) * tangent(camera->vertical_field_of_view * 0.5f),
					-1.f);
				matrix4x3_transform_point(&frustum->view_to_world, &frustum_grid[j][i], &frustum_grid[j][i]);
			}
		}

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				render_debug_line(TRUE, &frustum_grid[j][0], &frustum_grid[i][2], global_real_argb_red);
				render_debug_line(TRUE, &frustum_grid[0][j], &frustum_grid[2][i], global_real_argb_red);
			}
		}
	}

	return;
}
