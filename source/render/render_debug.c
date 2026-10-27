/*
RENDER_DEBUG.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render_debug.h"
#include "render.h"
#include "rasterizer.h"
#include "interface.h"
#include "draw_string.h"
#include "game.h"
#include "input.h"
#include "players.h"
#include "objects.h"
#include "units.h"
#include "bipeds.h"
#include "vehicles.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "structures.h"
#include "collisions.h"
#include "collision_bsp_definitions.h"
#include "bsp3d.h"
#include "collision_debug.h"
#include "collision_usage.h"
#include "decals.h"
#include "decal_definitions.h"
#include "damage.h"
#include "texture_cache.h"
#include "sound_cache.h"
#include "recorded_animations.h"
#include "ai_debug.h"
#include "ai_profile.h"
#include "path.h"
#include "hs.h"

#include <stdarg.h>

/* ---------- constants */

enum
{
	CIRCLE_DIVISIONS = 16,										/* fake name */
	NUMBER_OF_CIRCLE_POINTS = CIRCLE_DIVISIONS + 1,				/* fake name */
	NUMBER_OF_HALF_CIRCLE_POINTS = CIRCLE_DIVISIONS / 2 + 1,	/* fake name */

	MAXIMUM_RENDER_DEBUG_CACHE_ENTRIES = 512,					/* fake name */
	MAXIMUM_RENDER_DEBUG_CACHE_STRING_SIZE = 1024,				/* fake name */
};

enum
{
	_render_debug_cache_circle = 0,				/* fake name */
	_render_debug_cache_point,					/* fake name */
	_render_debug_cache_line,					/* fake name */
	_render_debug_cache_sphere,					/* fake name */
	_render_debug_cache_cylinder,				/* fake name */
	_render_debug_cache_pill,					/* fake name */
	_render_debug_cache_box,					/* fake name */
	_render_debug_cache_box_outline,			/* fake name */
	_render_debug_cache_string,					/* fake name */
	_render_debug_cache_string_at_point,		/* fake name */
	NUMBER_OF_RENDER_DEBUG_CACHE_TYPES			/* fake name */
};

#define CIRCLE_DIVISION_ANGLE ((2.f * _pi) / CIRCLE_DIVISIONS)				/* fake name */
#define DECAL_YAW_SCALE ((2.f * _pi) / (2 * CHAR_MAX))						/* fake name */
#define DECAL_PITCH_SCALE (_pi / (2 * CHAR_MAX))							/* fake name */
#define DEBUG_DECAL_SPHERE_RADIUS 0.1f										/* fake name */
#define DEBUG_CAMERA_GROUND_DISTANCE 1000.f									/* fake name */
#define DEBUG_CAMERA_SURFACE_DISTANCE 50.f									/* fake name */
#define DEBUG_BSP_FILE_NAME "d:\\debug_bsp.txt"								/* fake name */

/* ---------- macros */

/* ---------- structures */

struct render_debug_cache_entry	/* fake name */
{
	short type;

	union
	{
		struct
		{
			real_plane3d plane;
			short projection;
			boolean sign;
			real_point2d center;
			real radius;
			real_argb_color color;
			real offset;
		} circle;

		struct
		{
			real_point3d point;
			real size;
			real_argb_color color;
		} point;

		struct
		{
			real_point3d point0;
			real_point3d point1;
			real_argb_color color;
		} line;

		struct
		{
			real_point3d center;
			real radius;
			real_argb_color color;
		} sphere;

		struct
		{
			real_point3d base;
			real_vector3d height;
			real width;
			real_argb_color color;
		} cylinder;

		struct
		{
			real_point3d base;
			real_vector3d height;
			real width;
			real_argb_color color;
		} pill;

		struct
		{
			real_rectangle3d bounds;
			real_argb_color color;
		} box;

		struct
		{
			char *string;
		} string;

		struct
		{
			char *string;
			real_point3d point;
			real_argb_color color;
		} string_at_point;
	};
};

/* ---------- prototypes */

static char *render_debug_add_cache_string(char const *string);
static void build_circle_points(real radius, real_point2d *points);
static real build_height_matrix(real_point3d const *base, real_vector3d const *height, real_matrix4x3 *matrix);
static void build_pill_points(real_point3d const *base, real_vector3d const *height, real radius, real_point3d *top_points, real_point3d *bottom_points, real_point3d *yz_top_points, real_point3d *yz_bottom_points, real_point3d *zx_top_points, real_point3d *zx_bottom_points);
static void render_debug_add_cache_entry(short type, ...);
static void render_debug_camera(void);
static void render_debug_player(void);
static void render_debug_structure(void);
static void render_debug_bsp(void);
static void render_debug_input(void);
static void render_debug_structure_decals(void);

/* ---------- globals */

boolean debug_camera;
boolean debug_player;
boolean debug_structure;
boolean debug_bsp;
boolean debug_input;
boolean debug_permanent_decals;

static boolean render_debug_cache_string_overflow_warned;			/* fake name */
static boolean render_debug_cache_overflow_warned;					/* fake name */
static short render_debug_cache_string_length;						/* fake name */
static short render_debug_cache_entry_count;						/* fake name */
static short render_debug_cache_time;								/* fake name */
static struct render_debug_cache_entry render_debug_cache_entries[MAXIMUM_RENDER_DEBUG_CACHE_ENTRIES];	/* fake name */
static char render_debug_cache_strings[MAXIMUM_RENDER_DEBUG_CACHE_STRING_SIZE];							/* fake name */

/* ---------- public code */

void render_debug_point2d(
	boolean immediate,
	real_plane3d const *plane,
	short projection,
	boolean sign,
	real_point2d const *point,
	real size,
	real_argb_color const *color,
	real offset)
{
	real_point3d q;

	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 219, plane);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 220, point);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 221, color);

	project_point2d(point, plane, projection, sign, &q);
	q.n[projection] += sign ? offset : -offset;
	render_debug_point(immediate, &q, size, color);

	return;
}

void render_debug_line2d(
	boolean immediate,
	real_plane3d const *plane,
	short projection,
	boolean sign,
	real_point2d const *p0,
	real_point2d const *p1,
	real_argb_color const *color,
	real offset)
{
	real_point3d q0;
	real_point3d q1;

	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 242, plane);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 243, p0);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 244, p1);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 245, color);

	project_point2d(p0, plane, projection, sign, &q0);
	project_point2d(p1, plane, projection, sign, &q1);
	q0.n[projection] += sign ? offset : -offset;
	q1.n[projection] += sign ? offset : -offset;
	render_debug_line(immediate, &q0, &q1, color);

	return;
}

void render_debug_vector2d(
	boolean immediate,
	real_plane3d const *plane,
	short projection,
	boolean sign,
	real_point2d const *point,
	real_vector2d const *vector,
	real scale,
	real_argb_color const *color,
	real offset)
{
	real_point2d point1;

	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 269, plane);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 270, point);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 271, vector);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 272, color);

	point_from_line2d(point, vector, scale, &point1);
	render_debug_line2d(immediate, plane, projection, sign, point, &point1, color, offset);

	return;
}

void render_debug_circle(
	boolean immediate,
	real_plane3d const *plane,
	short projection,
	boolean sign,
	real_point2d const *center,
	real radius,
	real_argb_color const *color,
	real offset)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 290, plane);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 291, center);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 292, color);

	if (immediate)
	{
		real_point2d points[NUMBER_OF_CIRCLE_POINTS];
		real_point2d q0;
		real_point2d q1;
		short i;

		build_circle_points(radius, points);

		for (i = 0; i < CIRCLE_DIVISIONS; i++)
		{
			real_point2d const *p0 = &points[i];
			real_point2d const *p1 = &points[i + 1];

			set_real_point2d(&q0, center->x + p0->x, center->y + p0->y);
			set_real_point2d(&q1, center->x + p1->x, center->y + p1->y);
			render_debug_line2d(TRUE, plane, projection, sign, &q0, &q1, color, offset);
		}
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_circle, plane, projection, sign, center, radius, color);
	}

	return;
}

void render_debug_point(
	boolean immediate,
	real_point3d const *point,
	real size,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 327, point);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 328, color);

	if (immediate)
	{
		real_point3d points[6];
		short i;

		size *= 0.5f;
		set_real_point3d(&points[0], point->x - size, point->y, point->z);
		set_real_point3d(&points[1], point->x + size, point->y, point->z);
		set_real_point3d(&points[2], point->x, point->y - size, point->z);
		set_real_point3d(&points[3], point->x, point->y + size, point->z);
		set_real_point3d(&points[4], point->x, point->y, point->z - size);
		set_real_point3d(&points[5], point->x, point->y, point->z + size);

		for (i = 0; i < NUMBEROF(points); i += 2)
		{
			rasterizer_debug_line(&points[i], &points[i + 1], color);
		}
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_point, point, size, color);
	}

	return;
}

void render_debug_line(
	boolean immediate,
	real_point3d const *point0,
	real_point3d const *point1,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 363, point0);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 364, point1);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 365, color);

	if (immediate)
	{
		rasterizer_debug_line(point0, point1, color);
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_line, point0, point1, color);
	}

	return;
}

void render_debug_vector(
	boolean immediate,
	real_point3d const *point,
	real_vector3d const *vector,
	real size,
	real_argb_color const *color)
{
	real_point3d point1;

	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 388, point);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 389, vector);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 390, color);

	point_from_line3d(point, vector, size, &point1);
	render_debug_line(immediate, point, &point1, color);

	return;
}

void render_debug_vectors(
	boolean immediate,
	real_point3d const *point,
	real_vector3d const *forward,
	real_vector3d const *up,
	real size)
{
	real_matrix4x3 matrix;

	matrix4x3_from_point_and_vectors(&matrix, point, forward, up);
	render_debug_matrix(immediate, &matrix, size);

	return;
}

void render_debug_quaternion(
	boolean immediate,
	real_point3d const *point,
	real_quaternion const *quaternion,
	real size)
{
	real_matrix4x3 matrix;

	matrix4x3_from_point_and_quaternion(&matrix, point, quaternion);
	render_debug_matrix(immediate, &matrix, size);

	return;
}

void render_debug_tick(
	boolean immediate,
	real_point3d const *point,
	real_vector3d const *tick_vector,
	real tick_size,
	real_argb_color const *color)
{
	real_point3d q0;
	real_point3d q1;

	point_from_line3d(point, tick_vector, tick_size, &q0);
	point_from_line3d(point, tick_vector, -tick_size, &q1);
	render_debug_line(immediate, &q0, &q1, color);

	return;
}

void render_debug_line_offset(
	boolean immediate,
	real_point3d const *p0,
	real_point3d const *p1,
	real_argb_color const *color,
	real offset)
{
	real_point3d q0;
	real_point3d q1;

	point_from_line3d(p0, global_up3d, offset, &q0);
	point_from_line3d(p1, global_up3d, offset, &q1);
	render_debug_line(immediate, &q0, &q1, color);

	return;
}

void render_debug_matrix(
	boolean immediate,
	real_matrix4x3 const *matrix,
	real size)
{
	render_debug_vector(
		immediate,
		&matrix->position,
		&matrix->forward,
		size * matrix->scale,
		global_real_argb_red);
	render_debug_vector(
		immediate,
		&matrix->position,
		&matrix->left,
		size * matrix->scale,
		global_real_argb_green);
	render_debug_vector(
		immediate,
		&matrix->position,
		&matrix->up,
		size * matrix->scale,
		global_real_argb_blue);

	return;
}

void render_debug_triangle(
	boolean immediate,
	real_point3d const *point0,
	real_point3d const *point1,
	real_point3d const *point2,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 480, immediate);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 481, point0);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 482, point1);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 483, point2);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 484, color);

	rasterizer_debug_triangle(point0, point1, point2, color);

	return;
}

void render_debug_quadrilateral(
	boolean immediate,
	real_point3d const *point0,
	real_point3d const *point1,
	real_point3d const *point2,
	real_point3d const *point3,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 499, immediate);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 500, point0);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 501, point1);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 502, point2);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 503, point3);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 504, color);

	render_debug_triangle(immediate, point0, point1, point2, color);
	render_debug_triangle(immediate, point0, point2, point3, color);

	return;
}

void render_debug_sphere(
	boolean immediate,
	real_point3d const *center,
	real radius,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 518, center);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 519, color);

	if (immediate)
	{
		if (render_frustum_sphere_visible(&render.frustum, center, radius))
		{
			real_point2d points[NUMBER_OF_CIRCLE_POINTS];
			real_point3d q0;
			real_point3d q1;
			short i;

			build_circle_points(radius, points);

			for (i = 0; i < CIRCLE_DIVISIONS; i++)
			{
				real_point2d const *p0 = &points[i];
				real_point2d const *p1 = &points[i + 1];

				set_real_point3d(&q0, center->x + p0->x, center->y + p0->y, center->z);
				set_real_point3d(&q1, center->x + p1->x, center->y + p1->y, center->z);
				rasterizer_debug_line(&q0, &q1, color);
				set_real_point3d(&q0, center->x + p0->y, center->y, center->z + p0->x);
				set_real_point3d(&q1, center->x + p1->y, center->y, center->z + p1->x);
				rasterizer_debug_line(&q0, &q1, color);
				set_real_point3d(&q0, center->x, center->y + p0->x, center->z + p0->y);
				set_real_point3d(&q1, center->x, center->y + p1->x, center->z + p1->y);
				rasterizer_debug_line(&q0, &q1, color);
			}
		}
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_sphere, center, radius, color);
	}

	return;
}

void render_debug_cylinder(
	boolean immediate,
	real_point3d const *base,
	real_vector3d const *height,
	real width,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 645, base);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 646, height);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 647, color);

	if (immediate)
	{
		real_point3d top_points[NUMBER_OF_CIRCLE_POINTS];
		real_point3d bottom_points[NUMBER_OF_CIRCLE_POINTS];
		short i;

		build_pill_points(
			base,
			height,
			width,
			bottom_points,
			top_points,
			NULL,
			NULL,
			NULL,
			NULL);

		for (i = 0; i < CIRCLE_DIVISIONS; i++)
		{
			rasterizer_debug_line(&top_points[i], &top_points[i + 1], color);
			rasterizer_debug_line(&bottom_points[i], &bottom_points[i + 1], color);
		}

		for (i = 0; i < CIRCLE_DIVISIONS; i += 4)
		{
			rasterizer_debug_line(&top_points[i], &bottom_points[i], color);
		}

		for (i = 0; i < CIRCLE_DIVISIONS / 2; i += 4)
		{
			rasterizer_debug_line(&top_points[i], &top_points[i + CIRCLE_DIVISIONS / 2], color);
			rasterizer_debug_line(&bottom_points[i], &bottom_points[i + CIRCLE_DIVISIONS / 2], color);
		}
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_cylinder, base, height, width, color);
	}

	return;
}

void render_debug_pill(
	boolean immediate,
	real_point3d const *base,
	real_vector3d const *height,
	real width,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 693, base);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 694, height);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 695, color);

	if (immediate)
	{
		real_point3d xy_top_points[NUMBER_OF_CIRCLE_POINTS];
		real_point3d xy_bottom_points[NUMBER_OF_CIRCLE_POINTS];
		real_point3d yz_top_points[NUMBER_OF_HALF_CIRCLE_POINTS];
		real_point3d yz_bottom_points[NUMBER_OF_HALF_CIRCLE_POINTS];
		real_point3d zx_top_points[NUMBER_OF_HALF_CIRCLE_POINTS];
		real_point3d zx_bottom_points[NUMBER_OF_HALF_CIRCLE_POINTS];
		short i;

		build_pill_points(
			base,
			height,
			width,
			xy_top_points,
			xy_bottom_points,
			yz_top_points,
			yz_bottom_points,
			zx_top_points,
			zx_bottom_points);

		for (i = 0; i < CIRCLE_DIVISIONS; i++)
		{
			rasterizer_debug_line(&xy_top_points[i], &xy_top_points[i + 1], color);
			rasterizer_debug_line(&xy_bottom_points[i], &xy_bottom_points[i + 1], color);
		}

		for (i = 0; i < CIRCLE_DIVISIONS; i += 4)
		{
			rasterizer_debug_line(&xy_top_points[i], &xy_bottom_points[i], color);
		}

		for (i = 0; i < CIRCLE_DIVISIONS / 2; i++)
		{
			rasterizer_debug_line(&yz_top_points[i], &yz_top_points[i + 1], color);
			rasterizer_debug_line(&yz_bottom_points[i], &yz_bottom_points[i + 1], color);
			rasterizer_debug_line(&zx_top_points[i], &zx_top_points[i + 1], color);
			rasterizer_debug_line(&zx_bottom_points[i], &zx_bottom_points[i + 1], color);
		}
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_pill, base, height, width, color);
	}

	return;
}

void render_debug_box2d_outline(
	boolean immediate,
	real_rectangle2d const *bounds,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 745, bounds);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 746, color);

	if (immediate)
	{
		real_point3d box_points[4];

		set_real_point3d(&box_points[0], bounds->x0, bounds->y0, -1.f);
		set_real_point3d(&box_points[1], bounds->x1, bounds->y0, -1.f);
		set_real_point3d(&box_points[2], bounds->x1, bounds->y1, -1.f);
		set_real_point3d(&box_points[3], bounds->x0, bounds->y1, -1.f);
		matrix4x3_transform_point(&render.frustum.view_to_world, &box_points[0], &box_points[0]);
		matrix4x3_transform_point(&render.frustum.view_to_world, &box_points[1], &box_points[1]);
		matrix4x3_transform_point(&render.frustum.view_to_world, &box_points[2], &box_points[2]);
		matrix4x3_transform_point(&render.frustum.view_to_world, &box_points[3], &box_points[3]);
		render_debug_polygon_edges(box_points, NUMBEROF(box_points), color);
	}
	else
	{
		match_vhalt("c:\\halo\\SOURCE\\render\\render_debug.c", 765, "can't add box2d to debug cache");
	}

	return;
}

void render_debug_box(
	boolean immediate,
	real_rectangle3d const *bounds,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 776, bounds);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 777, color);

	if (immediate)
	{
		real_point3d box_points[8];

		set_real_point3d(&box_points[0], bounds->x0, bounds->y0, bounds->z0);
		set_real_point3d(&box_points[1], bounds->x1, bounds->y0, bounds->z0);
		set_real_point3d(&box_points[2], bounds->x0, bounds->y0, bounds->z1);
		set_real_point3d(&box_points[3], bounds->x1, bounds->y0, bounds->z1);
		set_real_point3d(&box_points[4], bounds->x0, bounds->y1, bounds->z1);
		set_real_point3d(&box_points[5], bounds->x1, bounds->y1, bounds->z1);
		set_real_point3d(&box_points[6], bounds->x0, bounds->y1, bounds->z0);
		set_real_point3d(&box_points[7], bounds->x1, bounds->y1, bounds->z0);
		render_debug_quadrilateral(TRUE, &box_points[0], &box_points[2], &box_points[4], &box_points[6], color);
		render_debug_quadrilateral(TRUE, &box_points[1], &box_points[3], &box_points[5], &box_points[7], color);
		render_debug_quadrilateral(TRUE, &box_points[0], &box_points[1], &box_points[3], &box_points[2], color);
		render_debug_quadrilateral(TRUE, &box_points[4], &box_points[5], &box_points[7], &box_points[6], color);
		render_debug_quadrilateral(TRUE, &box_points[0], &box_points[1], &box_points[7], &box_points[6], color);
		render_debug_quadrilateral(TRUE, &box_points[2], &box_points[3], &box_points[5], &box_points[4], color);
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_box, bounds, color);
	}

	return;
}

void render_debug_box_outline(
	boolean immediate,
	real_rectangle3d const *bounds,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 812, bounds);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 813, color);

	if (immediate)
	{
		real_point3d box_points[8];
		short i;

		set_real_point3d(&box_points[0], bounds->x0, bounds->y0, bounds->z0);
		set_real_point3d(&box_points[1], bounds->x1, bounds->y0, bounds->z0);
		set_real_point3d(&box_points[2], bounds->x1, bounds->y1, bounds->z0);
		set_real_point3d(&box_points[3], bounds->x0, bounds->y1, bounds->z0);
		set_real_point3d(&box_points[4], bounds->x0, bounds->y0, bounds->z1);
		set_real_point3d(&box_points[5], bounds->x1, bounds->y0, bounds->z1);
		set_real_point3d(&box_points[6], bounds->x1, bounds->y1, bounds->z1);
		set_real_point3d(&box_points[7], bounds->x0, bounds->y1, bounds->z1);
		render_debug_polygon_edges(&box_points[0], 4, color);
		render_debug_polygon_edges(&box_points[4], 4, color);

		for (i = 0; i < 4; i++)
		{
			render_debug_line(TRUE, &box_points[i], &box_points[i + 4], color);
		}
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_box_outline, bounds, color);
	}

	return;
}

void render_debug_polygon(
	real_point3d const *points,
	short point_count,
	real_argb_color const *color)
{
	short i;

	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 855, points);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 856, color);

	for (i = 1; i < point_count - 1; i++)
	{
		render_debug_triangle(TRUE, &points[0], &points[i], &points[i + 1], color);
	}

	return;
}

void render_debug_polygon_edges(
	real_point3d const *points,
	short point_count,
	real_argb_color const *color)
{
	short i;

	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 873, points);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 874, color);

	if (point_count > 2)
	{
		render_debug_line(TRUE, &points[point_count - 1], &points[0], color);

		for (i = 1; i < point_count; i++)
		{
			render_debug_line(TRUE, &points[i - 1], &points[i], color);
		}
	}

	return;
}

void render_debug_string(
	boolean immediate,
	char const *string)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 893, string);

	if (immediate)
	{
		interface_set_bitmap_text_draw_mode(
			_interface_font_terminal,
			_text_style_plain,
			_text_justification_left,
			0,
			_interface_color_table_dialog,
			0);
		rasterizer_draw_string(NULL, NULL, NULL, 0, string);
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_string, string);
	}

	return;
}

void render_debug_string_at_point(
	boolean immediate,
	real_point3d const *point,
	char const *string,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 914, point);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 915, string);
	match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 916, color);

	if (immediate)
	{
		real_point2d screen_point;
		rectangle2d bounds;

		if (render_camera_world_to_screen(&render.camera, &render.frustum, point, &screen_point))
		{
			bounds.x0 = (short)(screen_point.x - render.camera.viewport_bounds.x0);
			bounds.y0 = (short)(screen_point.y - render.camera.viewport_bounds.y0);
			bounds.x1 = SHORT_MAX;
			bounds.y1 = SHORT_MAX;
			interface_set_bitmap_text_draw_mode(
				_interface_font_terminal,
				_text_style_plain,
				_text_justification_left,
				0,
				_interface_color_table_dialog,
				0);
			draw_string_set_color(color);
			rasterizer_draw_string(&bounds, NULL, NULL, 0, string);
		}
	}
	else
	{
		render_debug_add_cache_entry(_render_debug_cache_string_at_point, string, point, color);
	}

	return;
}

void render_debug(
	void)
{
	short entry_index;

	ai_debug_render();
	ai_profile_render();
	render_debug_object_damage();
	render_debug_scripting();
	render_debug_trigger_volumes();
	texture_cache_debug_render();
	sound_cache_debug_render();
	render_debug_recording();
	render_debug_detail_objects();
	collision_debug_render();
	collision_log_render();
	render_debug_obstacle_path();
	render_debug_fog_planes();
	render_debug_decals();
	render_debug_camera();
	render_debug_player();
	render_debug_structure();
	render_debug_bsp();
	render_debug_input();
	render_debug_structure_decals();
	players_debug_render();

	for (entry_index = 0; entry_index < render_debug_cache_entry_count; entry_index++)
	{
		struct render_debug_cache_entry *entry = &render_debug_cache_entries[entry_index];

		switch (entry->type)
		{
		case _render_debug_cache_circle:
			render_debug_circle(
				TRUE,
				&entry->circle.plane,
				entry->circle.projection,
				entry->circle.sign,
				&entry->circle.center,
				entry->circle.radius,
				&entry->circle.color,
				entry->circle.offset);
			break;
		case _render_debug_cache_point:
			render_debug_point(TRUE, &entry->point.point, entry->point.size, &entry->point.color);
			break;
		case _render_debug_cache_line:
			render_debug_line(TRUE, &entry->line.point0, &entry->line.point1, &entry->line.color);
			break;
		case _render_debug_cache_sphere:
			render_debug_sphere(TRUE, &entry->sphere.center, entry->sphere.radius, &entry->sphere.color);
			break;
		case _render_debug_cache_cylinder:
			render_debug_cylinder(
				TRUE,
				&entry->cylinder.base,
				&entry->cylinder.height,
				entry->cylinder.width,
				&entry->cylinder.color);
			break;
		case _render_debug_cache_pill:
			render_debug_pill(
				TRUE,
				&entry->pill.base,
				&entry->pill.height,
				entry->pill.width,
				&entry->pill.color);
			break;
		case _render_debug_cache_box:
			render_debug_box(TRUE, &entry->box.bounds, &entry->box.color);
			break;
		case _render_debug_cache_box_outline:
			render_debug_box_outline(TRUE, &entry->box.bounds, &entry->box.color);
			break;
		case _render_debug_cache_string:
			render_debug_string(TRUE, entry->string.string);
			break;
		case _render_debug_cache_string_at_point:
			render_debug_string_at_point(
				TRUE,
				&entry->string_at_point.point,
				entry->string_at_point.string,
				&entry->string_at_point.color);
			break;
		default:
			match_halt("c:\\halo\\SOURCE\\render\\render_debug.c", 1269);
		}
	}

	if (render_debug_cache_time != (short)game_time_get() - 1)
	{
		render_debug_cache_time = (short)game_time_get();
		render_debug_cache_entry_count = 0;
		render_debug_cache_string_length = 0;
		render_debug_cache_strings[0] = '\0';
	}

	return;
}

/* ---------- private code */

static char *render_debug_add_cache_string(
	char const *string)
{
	short string_index;
	char *result = NULL;

	for (string_index = 0; string_index < render_debug_cache_string_length; string_index++)
	{
		if (!strcmp(string, &render_debug_cache_strings[string_index]))
		{
			result = &render_debug_cache_strings[string_index];
			break;
		}
	}

	if (!result)
	{
		if (render_debug_cache_string_length < MAXIMUM_RENDER_DEBUG_CACHE_STRING_SIZE - 1)
		{
			result = &render_debug_cache_strings[render_debug_cache_string_length];
			strncpy(result, string, MAXIMUM_RENDER_DEBUG_CACHE_STRING_SIZE - 1 - render_debug_cache_string_length);
			render_debug_cache_strings[MAXIMUM_RENDER_DEBUG_CACHE_STRING_SIZE - 1] = '\0';
			render_debug_cache_string_length += strlen(string) + 1;
			render_debug_cache_string_length = MIN(render_debug_cache_string_length, MAXIMUM_RENDER_DEBUG_CACHE_STRING_SIZE - 1);
		}
		else if (!render_debug_cache_string_overflow_warned)
		{
			error(_error_silent, "render debug cache string overflow");
			render_debug_cache_string_overflow_warned = TRUE;
		}
	}

	return result;
}

static void build_circle_points(
	real radius,
	real_point2d *points)
{
	short i;
	real s = sine(CIRCLE_DIVISION_ANGLE);
	real c = cosine(CIRCLE_DIVISION_ANGLE);

	set_real_point2d(&points[0], radius, 0.f);

	for (i = 0; i + 1 < CIRCLE_DIVISIONS; i++)
	{
		rotate_vector2d((real_vector2d const *)&points[i], s, c, (real_vector2d *)&points[i + 1]);
	}

	points[CIRCLE_DIVISIONS] = points[0];

	return;
}

static real build_height_matrix(
	real_point3d const *base,
	real_vector3d const *height,
	real_matrix4x3 *matrix)
{
	real length;

	matrix->scale = 1.f;
	matrix->up = *height;
	perpendicular3d(&matrix->up, &matrix->left);
	length = normalize3d(&matrix->up);
	normalize3d(&matrix->left);
	cross_product3d(&matrix->left, &matrix->up, &matrix->forward);
	matrix->position = *base;

	return length;
}

static void build_pill_points(
	real_point3d const *base,
	real_vector3d const *height,
	real radius,
	real_point3d *top_points,
	real_point3d *bottom_points,
	real_point3d *yz_top_points,
	real_point3d *yz_bottom_points,
	real_point3d *zx_top_points,
	real_point3d *zx_bottom_points)
{
	real_matrix4x3 matrix;
	real_point2d circle_points[NUMBER_OF_CIRCLE_POINTS];
	short i;
	real length = build_height_matrix(base, height, &matrix);

	build_circle_points(radius, circle_points);

	if (top_points && bottom_points)
	{
		for (i = 0; i < NUMBER_OF_CIRCLE_POINTS; i++)
		{
			real_point2d const *point = &circle_points[i];

			set_real_point3d(&top_points[i], point->x, point->y, length);
			set_real_point3d(&bottom_points[i], point->x, point->y, 0.f);
			matrix4x3_transform_point(&matrix, &top_points[i], &top_points[i]);
			matrix4x3_transform_point(&matrix, &bottom_points[i], &bottom_points[i]);
		}
	}

	if (yz_top_points && yz_bottom_points && zx_top_points && zx_bottom_points)
	{
		for (i = 0; i < NUMBER_OF_HALF_CIRCLE_POINTS; i++)
		{
			real_point2d const *p0 = &circle_points[i];
			real_point2d const *p1 = &circle_points[i + CIRCLE_DIVISIONS / 2];

			set_real_point3d(&yz_top_points[i], 0.f, p0->x, p0->y + length);
			set_real_point3d(&yz_bottom_points[i], 0.f, p1->x, p1->y);
			set_real_point3d(&zx_top_points[i], p0->x, 0.f, p0->y + length);
			set_real_point3d(&zx_bottom_points[i], p1->x, 0.f, p1->y);
			matrix4x3_transform_point(&matrix, &yz_top_points[i], &yz_top_points[i]);
			matrix4x3_transform_point(&matrix, &yz_bottom_points[i], &yz_bottom_points[i]);
			matrix4x3_transform_point(&matrix, &zx_top_points[i], &zx_top_points[i]);
			matrix4x3_transform_point(&matrix, &zx_bottom_points[i], &zx_bottom_points[i]);
		}
	}

	return;
}

static void render_debug_add_cache_entry(
	short type,
	...)
{
	if (render_debug_cache_time != (short)game_time_get())
	{
		render_debug_cache_time = (short)game_time_get();
		render_debug_cache_entry_count = 0;
		render_debug_cache_string_length = 0;
		render_debug_cache_strings[0] = '\0';
	}

	if (render_debug_cache_entry_count < MAXIMUM_RENDER_DEBUG_CACHE_ENTRIES)
	{
		va_list arguments;
		struct render_debug_cache_entry *entry = &render_debug_cache_entries[render_debug_cache_entry_count++];

		va_start(arguments, type);
		entry->type = type;

		switch (type)
		{
		case _render_debug_cache_circle:
			entry->circle.plane = *va_arg(arguments, real_plane3d const *);
			entry->circle.projection = va_arg(arguments, short);
			entry->circle.sign = va_arg(arguments, boolean);
			entry->circle.center = *va_arg(arguments, real_point2d const *);
			entry->circle.radius = (real)va_arg(arguments, double);
			entry->circle.color = *va_arg(arguments, real_argb_color const *);
			entry->circle.offset = (real)va_arg(arguments, double);
			break;
		case _render_debug_cache_point:
			entry->point.point = *va_arg(arguments, real_point3d const *);
			entry->point.size = (real)va_arg(arguments, double);
			entry->point.color = *va_arg(arguments, real_argb_color const *);
			break;
		case _render_debug_cache_line:
			entry->line.point0 = *va_arg(arguments, real_point3d const *);
			entry->line.point1 = *va_arg(arguments, real_point3d const *);
			entry->line.color = *va_arg(arguments, real_argb_color const *);
			break;
		case _render_debug_cache_sphere:
			entry->sphere.center = *va_arg(arguments, real_point3d const *);
			entry->sphere.radius = (real)va_arg(arguments, double);
			entry->sphere.color = *va_arg(arguments, real_argb_color const *);
			break;
		case _render_debug_cache_cylinder:
			entry->cylinder.base = *va_arg(arguments, real_point3d const *);
			entry->cylinder.height = *va_arg(arguments, real_vector3d const *);
			entry->cylinder.width = (real)va_arg(arguments, double);
			entry->cylinder.color = *va_arg(arguments, real_argb_color const *);
			break;
		case _render_debug_cache_pill:
			entry->pill.base = *va_arg(arguments, real_point3d const *);
			entry->pill.height = *va_arg(arguments, real_vector3d const *);
			entry->pill.width = (real)va_arg(arguments, double);
			entry->pill.color = *va_arg(arguments, real_argb_color const *);
			break;
		case _render_debug_cache_box:
		case _render_debug_cache_box_outline:
			entry->box.bounds = *va_arg(arguments, real_rectangle3d const *);
			entry->box.color = *va_arg(arguments, real_argb_color const *);
			break;
		case _render_debug_cache_string:
		{
			char *string = render_debug_add_cache_string(va_arg(arguments, char const *));

			if (string)
			{
				entry->string.string = string;
			}
			else
			{
				render_debug_cache_entry_count--;
			}
			break;
		}
		case _render_debug_cache_string_at_point:
		{
			char *string = render_debug_add_cache_string(va_arg(arguments, char const *));

			if (string)
			{
				entry->string_at_point.string = string;
				entry->string_at_point.point = *va_arg(arguments, real_point3d const *);
				entry->string_at_point.color = *va_arg(arguments, real_argb_color const *);
			}
			else
			{
				render_debug_cache_entry_count--;
			}
			break;
		}
		}

		va_end(arguments);
	}
	else if (!render_debug_cache_overflow_warned)
	{
		error(_error_silent, "render debug cache overflow.");
		render_debug_cache_overflow_warned = TRUE;
	}

	return;
}

static void render_debug_camera(
	void)
{
	if (debug_camera)
	{
		struct location camera_location;
		char textstring[2048];
		real_vector3d collision_vector;
		struct collision_result collision;
		real_vector3d v;
		real_point3d p;
		short lightmap_index;
		short material_index;
		long surface_index;
		real s;
		real t;

		scenario_location_from_point(&camera_location, &render.camera.position);
		_snprintf(
			textstring,
			sizeof(textstring),
			"point(%01.2f,%01.2f,%01.2f) leaf(#%d [%d]) cluster(#%d [%d])",
			render.camera.position.x,
			render.camera.position.y,
			render.camera.position.z,
			camera_location.leaf_index,
			render.leaf_index,
			camera_location.cluster_index,
			render.cluster_index);

		scale_vector3d(global_down3d, DEBUG_CAMERA_GROUND_DISTANCE, &collision_vector);

		if (collision_test_vector(
			FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_structure_bit),
			&render.camera.position,
			&collision_vector,
			NONE,
			&collision))
		{
			_snprintf(
				&textstring[strlen(textstring)],
				sizeof(textstring) - strlen(textstring),
				"|nground_point(%01.2f,%01.2f,%01.2f) facing(%01.2f) surface(#%d)",
				collision.point.x,
				collision.point.y,
				collision.point.z,
				arctangent(render.camera.forward.j, render.camera.forward.i) / (_pi / 180.f),
				collision.surface_index);
		}

		scale_vector3d(&render.camera.forward, DEBUG_CAMERA_SURFACE_DISTANCE, &v);

		if (structure_test_vector(
			&render.camera.position,
			&v,
			&p,
			&lightmap_index,
			&material_index,
			&surface_index,
			&s,
			&t))
		{
			_snprintf(
				&textstring[strlen(textstring)],
				sizeof(textstring) - strlen(textstring),
				"|n%s",
				TAG_BLOCK_GET_ELEMENT(
					&TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->lightmaps, lightmap_index, struct structure_lightmap)->materials,
					material_index,
					struct structure_material)->shader.name);
		}

		render_debug_string(TRUE, textstring);
	}

	return;
}

static void render_debug_player(
	void)
{
	if (debug_player && render.local_player_index != NONE)
	{
		long unit_index = local_player_get_player_index(render.local_player_index) == NONE ?
			NONE :
			player_get(local_player_get_player_index(render.local_player_index))->unit_index;

		if (unit_index != NONE)
		{
			struct biped_datum *biped = biped_try_and_get(unit_index);

			if (biped && biped->biped.elevator_object_index != NONE)
			{
				render_debug_string(TRUE, "riding an elevator");
			}

			if (biped->object.parent_object_index != NONE && biped->unit.parent_seat_index != NONE)
			{
				char string[1025];
				struct vehicle_datum *vehicle = (struct vehicle_datum *)object_try_and_get_and_verify_type(
					biped->object.parent_object_index,
					_object_mask_vehicle);

				sprintf(
					string,
					"speed %5f|nslide %5f|nturn  %5f%s",
					vehicle->vehicle.speed,
					vehicle->vehicle.slide,
					vehicle->vehicle.turn,
					vehicle->vehicle.stuck_mass_point_flags ? "|nstuck!" : "");
				render_debug_string(TRUE, string);
			}
		}
	}

	return;
}

static void render_debug_structure(
	void)
{
	if (debug_structure)
	{
		long edge_index;
		struct collision_bsp *bsp = global_collision_bsp_get();

		for (edge_index = 0; edge_index < bsp->edges.count; edge_index++)
		{
			struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
			struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(
				&bsp->vertices,
				edge->vertex_indices[0],
				struct collision_vertex);
			struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(
				&bsp->vertices,
				edge->vertex_indices[1],
				struct collision_vertex);

			render_debug_line(TRUE, &vertex0->point, &vertex1->point, global_real_argb_green);
		}
	}

	return;
}

static void render_debug_bsp(
	void)
{
	if (debug_bsp)
	{
		long plane_indices[MAXIMUM_BSP3D_DEPTH];
		char textstring[2048];
		long node_index = 0;
		short plane_count = 0;
		struct bsp3d *bsp3d = global_bsp3d_get();
		char *s = textstring + sprintf(textstring, " node plane|n");

		do
		{
			struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&bsp3d->nodes, node_index, struct bsp3d_node);
			boolean front = plane3d_distance_to_point(
				TAG_BLOCK_GET_ELEMENT(&bsp3d->planes, node->plane, real_plane3d),
				&render.camera.position) >= 0.f;

			match_assert("c:\\halo\\SOURCE\\render\\render_debug.c", 1618, plane_count<MAXIMUM_BSP3D_DEPTH);
			plane_indices[plane_count++] = node->plane;
			s += sprintf(s, "%5d %5d %c|n", node_index, node->plane, front ? '+' : '-');
			node_index = node->child_indices[front];
		}
		while (!(node_index & LONG_MIN));

		if (node_index != NONE)
		{
			sprintf(s, " leaf %5d", node_index & LONG_MAX);
		}
		else
		{
			sprintf(s, "solid");
		}

		render_debug_string(TRUE, textstring);

		if (input_key_is_down(_key_b))
		{
			FILE *file = fopen(DEBUG_BSP_FILE_NAME, "w");

			if (file)
			{
				short i;

				fprintf(file, "%d\n", plane_count);

				for (i = 0; i < plane_count; i++)
				{
					fprintf(file, "%d\n", plane_indices[i]);
				}

				fclose(file);
			}
		}
	}

	return;
}

static void render_debug_input(
	void)
{
	if (debug_input)
	{
		char temp[512];
		short tab_stops[] = { 200, 400, 550 };

		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		input_get_raw_data_string(temp, sizeof(temp) - 1);
		render_debug_string(TRUE, temp);
	}

	return;
}

static void render_debug_structure_decals(
	void)
{
	if (debug_permanent_decals)
	{
		long decal_index;
		struct structure_bsp *structure_bsp = global_structure_bsp_get();

		for (decal_index = 0; decal_index < structure_bsp->runtime_decals.count; decal_index++)
		{
			struct collision_result collision;
			struct structure_runtime_decal *decal = TAG_BLOCK_GET_ELEMENT(
				&structure_bsp->runtime_decals,
				decal_index,
				struct structure_runtime_decal);
			long definition_index = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->decal_palette,
				decal->palette_index,
				struct scenario_decal_palette_entry)->reference.index;
			struct decal_definition *definition = decal_definition_get(definition_index);
			long cluster_index = scenario_leaf_index_from_point(&decal->position) == NONE ?
				NONE :
				TAG_BLOCK_GET_ELEMENT(
					&global_structure_bsp_get()->leaves,
					scenario_leaf_index_from_point(&decal->position) & LONG_MAX,
					struct structure_leaf)->cluster_index;
			struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(
				&structure_bsp->clusters,
				cluster_index,
				struct structure_cluster);

			{
				real_vector3d direction;
				real_euler_angles2d angles;

				angles.yaw = decal->yaw * DECAL_YAW_SCALE;
				angles.pitch = decal->pitch * DECAL_PITCH_SCALE;
				vector3d_from_euler_angles2d(&direction, &angles);

				if (collision_test_vector(
					_collision_test_environment_flags,
					&decal->position,
					&direction,
					NONE,
					&collision))
				{
					if (cluster->first_runtime_decal_index == NONE || decal_index - cluster->first_runtime_decal_index >= cluster->runtime_decal_count)
					{
						render_debug_sphere(TRUE, &collision.point, DEBUG_DECAL_SPHERE_RADIUS, global_real_argb_orange);
					}
					else
					{
						render_debug_sphere(TRUE, &collision.point, DEBUG_DECAL_SPHERE_RADIUS, global_real_argb_yellow);
					}
				}
				else
				{
					render_debug_sphere(TRUE, &decal->position, DEBUG_DECAL_SPHERE_RADIUS, global_real_argb_red);
				}
			}

			render_debug_string_at_point(
				FALSE,
				&decal->position,
				tag_name_strip_path(tag_get_name(definition_index)),
				global_real_argb_green);
		}
	}

	return;
}
