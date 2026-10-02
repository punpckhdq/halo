/*
ERROR_GEOMETRY.C

*/

/* ---------- headers */

#include "cseries.h"
#include "error_geometry.h"

/* ---------- constants */

#define ERROR_GEOMETRY_FILE_EXTENSION ".wrl" /* fake name */
#define MAXIMUM_ERROR_GEOMETRY_NAME_LENGTH (sizeof(error_geometry_file_name)-sizeof(ERROR_GEOMETRY_FILE_EXTENSION)) /* fake name */

#define IMPORT_SCALE 100.f /* fake name */

#define ERROR_GEOMETRY_POINT_RADIUS 0.01f /* fake name */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean error_geometry_file_is_open(void);

/* ---------- globals */

static FILE *error_geometry_file= NULL;

static char error_geometry_file_name[64]= "debug.wrl"; /* fake name */

static real_matrix4x3 error_geometry_transform= /* fake name */
{
	1.f,
	{
		{
			{ 1.f, 0.f, 0.f },
			{ 0.f, 1.f, 0.f },
			{ 0.f, 0.f, 1.f },
			{ 0.f, 0.f, 0.f }
		}
	}
};

/* ---------- public code */

static boolean error_geometry_file_is_open(
	void)
{
	if (!error_geometry_file)
	{
		error_geometry_file= fopen(error_geometry_file_name, "w");
		if (error_geometry_file)
		{
			fprintf(error_geometry_file, "#VRML V1.0 ascii\n\n");
			fflush(error_geometry_file);
		}
	}

	return error_geometry_file!=NULL;
}

void error_geometry_initialize(
	void)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 68, error_geometry_file==NULL);

	remove(error_geometry_file_name);

	return;
}

void error_geometry_dispose(
	void)
{
	if (error_geometry_file)
	{
		fclose(error_geometry_file);
		error_geometry_file= NULL;
	}

	return;
}

void error_geometry_set_name(
	char const *name)
{
	if (strncmp(error_geometry_file_name, name, MAXIMUM_ERROR_GEOMETRY_NAME_LENGTH))
	{
		error_geometry_dispose();

		strncpy(error_geometry_file_name, name, MAXIMUM_ERROR_GEOMETRY_NAME_LENGTH);
		error_geometry_file_name[MAXIMUM_ERROR_GEOMETRY_NAME_LENGTH]= 0;
		strcat(error_geometry_file_name, ERROR_GEOMETRY_FILE_EXTENSION);

		error_geometry_initialize();
	}

	return;
}

void error_geometry_set_transform(
	real_matrix4x3 const *matrix)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 107, matrix);
	match_assert_valid_real_matrix4x3("c:\\halo\\SOURCE\\tool\\error_geometry.c", 108, matrix);

	error_geometry_transform= *matrix;

	return;
}

void error_geometry_point(
	real_point3d const *point,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 119, point);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 120, color);

	if (error_geometry_file_is_open())
	{
		real_rectangle3d bounds;

		bounds.x0= point->x-ERROR_GEOMETRY_POINT_RADIUS;
		bounds.x1= point->x+ERROR_GEOMETRY_POINT_RADIUS;
		bounds.y0= point->y-ERROR_GEOMETRY_POINT_RADIUS;
		bounds.y1= point->y+ERROR_GEOMETRY_POINT_RADIUS;
		bounds.z0= point->z-ERROR_GEOMETRY_POINT_RADIUS;
		bounds.z1= point->z+ERROR_GEOMETRY_POINT_RADIUS;
		error_geometry_rectangle3d(&bounds, color);
	}

	return;
}

void error_geometry_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 143, p0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 144, p1);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 145, color);

	if (error_geometry_file_is_open())
	{
		real_point3d transformed_p0, transformed_p1;

		matrix4x3_transform_point(&error_geometry_transform, p0, &transformed_p0);
		matrix4x3_transform_point(&error_geometry_transform, p1, &transformed_p1);

		fprintf(error_geometry_file, "Separator\n{\n");
		fprintf(error_geometry_file, "\tCoordinate3 { point[%f %f %f, %f %f %f] }\n",
			transformed_p0.x*IMPORT_SCALE, transformed_p0.y*IMPORT_SCALE, transformed_p0.z*IMPORT_SCALE,
			transformed_p1.x*IMPORT_SCALE, transformed_p1.y*IMPORT_SCALE, transformed_p1.z*IMPORT_SCALE);
		fprintf(error_geometry_file, "\tMaterialBinding { value PER_VERTEX }\n");
		fprintf(error_geometry_file, "\tMaterial { diffuseColor[%f %f %f, %f %f %f] transparency[%f, %f] }\n",
			color->red, color->green, color->blue,
			color->red, color->green, color->blue,
			1.f-color->alpha, 1.f-color->alpha);
		fprintf(error_geometry_file, "\tIndexedLineSet { coordIndex[0,1,-1] }\n");
		fprintf(error_geometry_file, "}\n");
		fflush(error_geometry_file);
	}

	return;
}

void error_geometry_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 177, p0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 178, p1);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 179, p2);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 180, color);

	if (error_geometry_file_is_open())
	{
		real_point3d transformed_p0, transformed_p1, transformed_p2;

		matrix4x3_transform_point(&error_geometry_transform, p0, &transformed_p0);
		matrix4x3_transform_point(&error_geometry_transform, p1, &transformed_p1);
		matrix4x3_transform_point(&error_geometry_transform, p2, &transformed_p2);

		fprintf(error_geometry_file, "Separator\n{\n");
		fprintf(error_geometry_file, "\tCoordinate3 { point[%f %f %f, %f %f %f, %f %f %f] }\n",
			transformed_p0.x*IMPORT_SCALE, transformed_p0.y*IMPORT_SCALE, transformed_p0.z*IMPORT_SCALE,
			transformed_p1.x*IMPORT_SCALE, transformed_p1.y*IMPORT_SCALE, transformed_p1.z*IMPORT_SCALE,
			transformed_p2.x*IMPORT_SCALE, transformed_p2.y*IMPORT_SCALE, transformed_p2.z*IMPORT_SCALE);
		fprintf(error_geometry_file, "\tMaterialBinding { value PER_FACE }\n");
		fprintf(error_geometry_file, "\tMaterial { diffuseColor[%f %f %f] transparency[%f] }\n",
			color->red, color->green, color->blue, 1.f-color->alpha);
		fprintf(error_geometry_file, "\tIndexedFaceSet { coordIndex[0,1,2,-1] }\n");
		fprintf(error_geometry_file, "}\n");
		fflush(error_geometry_file);
	}

	return;
}

void error_geometry_polygon(
	short point_count,
	real_point3d const *points,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 211, point_count>=0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 212, points);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 213, color);

	if (point_count>=3 && error_geometry_file_is_open())
	{
		short point_index;

		fprintf(error_geometry_file, "Separator\n{\n");
		fprintf(error_geometry_file, "\tCoordinate3 { point[");
		for (point_index= 0; point_index<point_count; point_index++)
		{
			real_point3d transformed_point;

			matrix4x3_transform_point(&error_geometry_transform, &points[point_index], &transformed_point);
			fprintf(error_geometry_file, "%f %f %f%s",
				transformed_point.x*IMPORT_SCALE, transformed_point.y*IMPORT_SCALE, transformed_point.z*IMPORT_SCALE,
				(point_index<point_count-1) ? ", " : "] }\n");
		}
		fprintf(error_geometry_file, "\tMaterialBinding { value PER_FACE }\n");
		fprintf(error_geometry_file, "\tMaterial { diffuseColor[%f %f %f] transparency[%f] }\n",
			color->red, color->green, color->blue, 1.f-color->alpha);
		fprintf(error_geometry_file, "\tIndexedFaceSet { coordIndex[");
		for (point_index= 0; point_index<point_count; point_index++)
		{
			fprintf(error_geometry_file, "%d,", point_index);
		}
		fprintf(error_geometry_file, "-1] }\n");
		fprintf(error_geometry_file, "}\n");
		fflush(error_geometry_file);
	}

	return;
}

void error_geometry_polygon_list(
	long polygon_count,
	short const *point_counts,
	real_point3d const *points,
	real_argb_color const *colors)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 332, polygon_count>=0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 333, point_counts);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 334, points);

	if (polygon_count>0 && error_geometry_file_is_open())
	{
		long polygon_index;
		long point_index;

		fprintf(error_geometry_file, "Separator\n{\n");

		fprintf(error_geometry_file, "\tCoordinate3\n\t{\n\t\tpoint\n\t\t[\n");
		point_index= 0;
		for (polygon_index= 0; polygon_index<polygon_count; polygon_index++)
		{
			short polygon_point_index;

			for (polygon_point_index= 0; polygon_point_index<point_counts[polygon_index]; polygon_point_index++, point_index++)
			{
				real_point3d transformed_point;

				matrix4x3_transform_point(&error_geometry_transform, &points[point_index], &transformed_point);
				fprintf(error_geometry_file, "\t\t\t%f %f %f,\n",
					transformed_point.x*IMPORT_SCALE, transformed_point.y*IMPORT_SCALE, transformed_point.z*IMPORT_SCALE);
			}
		}
		fprintf(error_geometry_file, "\t\t]\n\t}\n");

		fprintf(error_geometry_file, "\tMaterialBinding\n\t{\n\t\tvalue PER_FACE\n\t}\n");

		if (colors)
		{
			fprintf(error_geometry_file, "\tMaterial\n\t{\n\t\tdiffuseColor\n\t\t[\n");
			for (polygon_index= 0; polygon_index<polygon_count; polygon_index++)
			{
				short triangle_index;

				for (triangle_index= 2; triangle_index<point_counts[polygon_index]; triangle_index++)
				{
					fprintf(error_geometry_file, "\t\t\t%f %f %f, ",
						colors[polygon_index].red, colors[polygon_index].green, colors[polygon_index].blue);
				}
				fprintf(error_geometry_file, "\n");
			}
			fprintf(error_geometry_file, "\t\t]\n\t\ttransparency[%f]\n\t}\n", 1.f-colors->alpha);
		}

		fprintf(error_geometry_file, "\tIndexedFaceSet\n\t{\n\t\tcoordIndex\n\t\t[\n");
		point_index= 0;
		for (polygon_index= 0; polygon_index<polygon_count; polygon_index++)
		{
			short triangle_index;

			fprintf(error_geometry_file, "\t\t\t");
			for (triangle_index= 2; triangle_index<point_counts[polygon_index]; triangle_index++)
			{
				fprintf(error_geometry_file, "%d,%d,%d,-1, ", point_index, point_index+triangle_index-1, point_index+triangle_index);
			}
			fprintf(error_geometry_file, "\n");
			point_index+= point_counts[polygon_index];
		}
		fprintf(error_geometry_file, "\t\t]\n\t}\n}\n");
		fflush(error_geometry_file);
	}

	return;
}

void error_geometry_polygon_mesh__textured_with_no_import_scale(
	long width,
	long height,
	real_point3d const *points,
	real_point2d const *texcoords)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 428, width>0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 429, height>0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 430, points);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 431, texcoords);

	if (error_geometry_file_is_open())
	{
		long point_index;
		long row;

		fprintf(error_geometry_file, "Separator\n{\n");

		fprintf(error_geometry_file, "\tCoordinate3\n\t{\n\t\tpoint\n\t\t[\n");
		for (point_index= 0; point_index<width*height; point_index++)
		{
			fprintf(error_geometry_file, "\t\t\t%f %f %f,\n", points[point_index].x, points[point_index].y, points[point_index].z);
		}
		fprintf(error_geometry_file, "\t\t]\n\t}\n");

		fprintf(error_geometry_file, "\tTextureCoordinate\n\t{\n\t\tpoint\n\t\t[\n");
		for (point_index= 0; point_index<width*height; point_index++)
		{
			fprintf(error_geometry_file, "\t\t\t%f %f,\n", texcoords[point_index].x, texcoords[point_index].y);
		}
		fprintf(error_geometry_file, "\t\t]\n\t}\n");

		fprintf(error_geometry_file, "\tMaterialBinding\n\t{\n\t\tvalue PER_FACE\n\t}\n");

		fprintf(error_geometry_file, "\tIndexedFaceSet\n\t{\n\t\tcoordIndex\n\t\t[\n");
		for (row= 0; row<height-1; row++)
		{
			long column;

			for (column= 0; column<width-1; column++)
			{
				fprintf(error_geometry_file, "\t\t\t");
				fprintf(error_geometry_file, "%d,%d,%d,%d,-1,",
					row*width+column, row*width+column+1, (row+1)*width+column+1, (row+1)*width+column);
				fprintf(error_geometry_file, "\n");
			}
		}
		fprintf(error_geometry_file, "\t\t]\n\t}\n}\n");
		fflush(error_geometry_file);
	}

	return;
}

void error_geometry_rectangle3d(
	real_rectangle3d const *bounds,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 491, bounds);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 492, color);

	if (error_geometry_file_is_open())
	{
		real_point3d points[4];

		set_real_point3d(&points[0], bounds->x0, bounds->y0, bounds->z0);
		set_real_point3d(&points[1], bounds->x0, bounds->y1, bounds->z0);
		set_real_point3d(&points[2], bounds->x0, bounds->y1, bounds->z1);
		set_real_point3d(&points[3], bounds->x0, bounds->y0, bounds->z1);
		error_geometry_polygon(4, points, color);

		set_real_point3d(&points[0], bounds->x1, bounds->y0, bounds->z0);
		set_real_point3d(&points[1], bounds->x1, bounds->y1, bounds->z0);
		set_real_point3d(&points[2], bounds->x1, bounds->y1, bounds->z1);
		set_real_point3d(&points[3], bounds->x1, bounds->y0, bounds->z1);
		error_geometry_polygon(4, points, color);

		set_real_point3d(&points[0], bounds->x0, bounds->y0, bounds->z0);
		set_real_point3d(&points[1], bounds->x1, bounds->y0, bounds->z0);
		set_real_point3d(&points[2], bounds->x1, bounds->y0, bounds->z1);
		set_real_point3d(&points[3], bounds->x0, bounds->y0, bounds->z1);
		error_geometry_polygon(4, points, color);

		set_real_point3d(&points[0], bounds->x0, bounds->y1, bounds->z0);
		set_real_point3d(&points[1], bounds->x1, bounds->y1, bounds->z0);
		set_real_point3d(&points[2], bounds->x1, bounds->y1, bounds->z1);
		set_real_point3d(&points[3], bounds->x0, bounds->y1, bounds->z1);
		error_geometry_polygon(4, points, color);

		set_real_point3d(&points[0], bounds->x0, bounds->y0, bounds->z0);
		set_real_point3d(&points[1], bounds->x0, bounds->y1, bounds->z0);
		set_real_point3d(&points[2], bounds->x1, bounds->y1, bounds->z0);
		set_real_point3d(&points[3], bounds->x1, bounds->y0, bounds->z0);
		error_geometry_polygon(4, points, color);

		set_real_point3d(&points[0], bounds->x0, bounds->y0, bounds->z1);
		set_real_point3d(&points[1], bounds->x0, bounds->y1, bounds->z1);
		set_real_point3d(&points[2], bounds->x1, bounds->y1, bounds->z1);
		set_real_point3d(&points[3], bounds->x1, bounds->y0, bounds->z1);
		error_geometry_polygon(4, points, color);
	}

	return;
}

void error_geometry_bounded_point(
	real_point3d const *point,
	real radius,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 539, point);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 540, color);

	if (error_geometry_file_is_open())
	{
		real_rectangle3d bounds;
		real_argb_color bounds_color;

		bounds.x0= point->x-radius;
		bounds.x1= point->x+radius;
		bounds.y0= point->y-radius;
		bounds.y1= point->y+radius;
		bounds.z0= point->z-radius;
		bounds.z1= point->z+radius;
		bounds_color.alpha= color->alpha*0.5f;
		bounds_color.rgb= color->rgb;
		error_geometry_rectangle3d(&bounds, &bounds_color);

		error_geometry_point(point, color);
	}

	return;
}

void error_geometry_bounded_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real radius,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 567, p0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 568, p1);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 569, color);

	if (error_geometry_file_is_open())
	{
		real_rectangle3d bounds;
		real_argb_color bounds_color;

		bounds.x0= MIN(p0->x, p1->x)-radius;
		bounds.x1= MAX(p0->x, p1->x)+radius;
		bounds.y0= MIN(p0->y, p1->y)-radius;
		bounds.y1= MAX(p0->y, p1->y)+radius;
		bounds.z0= MIN(p0->z, p1->z)-radius;
		bounds.z1= MAX(p0->z, p1->z)+radius;
		bounds_color.alpha= color->alpha*0.5f;
		bounds_color.rgb= color->rgb;
		error_geometry_rectangle3d(&bounds, &bounds_color);

		error_geometry_line(p0, p1, color);
	}

	return;
}

void error_geometry_bounded_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real radius,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 597, p0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 598, p1);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 599, p2);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 600, color);

	if (error_geometry_file_is_open())
	{
		real_rectangle3d bounds;
		real_argb_color bounds_color;

		bounds.x0= MIN(p0->x, MIN(p1->x, p2->x))-radius;
		bounds.x1= MAX(p0->x, MAX(p1->x, p2->x))+radius;
		bounds.y0= MIN(p0->y, MIN(p1->y, p2->y))-radius;
		bounds.y1= MAX(p0->y, MAX(p1->y, p2->y))+radius;
		bounds.z0= MIN(p0->z, MIN(p1->z, p2->z))-radius;
		bounds.z1= MAX(p0->z, MAX(p1->z, p2->z))+radius;
		bounds_color.alpha= color->alpha*0.5f;
		bounds_color.rgb= color->rgb;
		error_geometry_rectangle3d(&bounds, &bounds_color);

		error_geometry_triangle(p0, p1, p2, color);
	}

	return;
}

void error_geometry_bounded_polygon(
	short point_count,
	real_point3d const *points,
	real radius,
	real_argb_color const *color)
{
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 627, point_count>=0);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 628, points);
	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 629, color);

	if (point_count>=3 && error_geometry_file_is_open())
	{
		real_rectangle3d bounds;
		real_argb_color bounds_color;
		short point_index;

		bounds.x0= REAL_MAX;
		bounds.x1= -REAL_MAX;
		bounds.y0= REAL_MAX;
		bounds.y1= -REAL_MAX;
		bounds.z0= REAL_MAX;
		bounds.z1= -REAL_MAX;
		for (point_index= 0; point_index<point_count; point_index++)
		{
			bounds.x0= MIN(bounds.x0, points[point_index].x);
			bounds.x1= MAX(bounds.x1, points[point_index].x);
			bounds.y0= MIN(bounds.y0, points[point_index].y);
			bounds.y1= MAX(bounds.y1, points[point_index].y);
			bounds.z0= MIN(bounds.z0, points[point_index].z);
			bounds.z1= MAX(bounds.z1, points[point_index].z);
		}
		bounds.x0-= radius;
		bounds.x1+= radius;
		bounds.y0-= radius;
		bounds.y1+= radius;
		bounds.z0-= radius;
		bounds.z1+= radius;
		bounds_color.alpha= color->alpha*0.5f;
		bounds_color.rgb= color->rgb;
		error_geometry_rectangle3d(&bounds, &bounds_color);

		error_geometry_polygon(point_count, points, color);
	}

	return;
}

void error_geometry_comment(
	char const *format,
	...)
{
	va_list arglist;

	va_start(arglist, format);

	match_assert("c:\\halo\\SOURCE\\tool\\error_geometry.c", 669, format);

	if (error_geometry_file_is_open())
	{
		fprintf(error_geometry_file, "#");
		vfprintf(error_geometry_file, format, arglist);
		fprintf(error_geometry_file, "\n");
		fflush(error_geometry_file);
	}

	va_end(arglist);

	return;
}

/* ---------- private code */
