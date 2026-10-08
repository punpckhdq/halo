/*
RASTERIZER_GEOMETRY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_geometry.h"
#include "rasterizer.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "decals.h"

/* ---------- public code */

byte compress_real_to_int8(
	real z)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 42, z>=0.0f && z<=1.0f);

	return (byte)fast_ftol(z*255.f);
}

byte compress_real_to_int8_clamp(
	real z)
{
	return (byte)fast_ftol(PIN(z, 0.f, 1.f)*255.f);
}

short compress_real_to_int16(
	real z)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 55, z>=-1.0f && z<=1.0f);

	return (short)fast_ftol((real)floor(z*32767.5f));
}

short compress_real_to_int16_clamp(
	real z)
{
	return (short)fast_ftol((real)floor(PIN(z, -1.f, 1.f)*32767.5f));
}

unsigned long compress_real_vector3d_to_int32(
	real_vector3d const *v)
{
	long i;
	long j;
	long k;
	real_vector3d v2;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 69, v);
	match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 78, v->i>=-1.f && v->i<=1.f && v->j>=-1.f && v->j<=1.f && v->k>=-1.f && v->k<=1.f,
		csprintf(temporary, "invalid vector= [%f %f %f] 0x%x%x%x", v->i, v->j, v->k, *(unsigned long *)&v->i, *(unsigned long *)&v->j, *(unsigned long *)&v->k));

	i = fast_ftol((real)floor(v->i*1023.5f))&0x7ff;
	j = fast_ftol((real)floor(v->j*1023.5f))&0x7ff;
	k = fast_ftol((real)floor(v->k*511.5f))&0x3ff;

	v2 = uncompress_int32_to_real_vector3d((k<<22) | (j<<11) | i);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 92, fabs(v2.i - v->i)<0.01f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 93, fabs(v2.j - v->j)<0.01f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 94, fabs(v2.k - v->k)<0.01f);

	return (k<<22) | (j<<11) | i;
}

unsigned long compress_real_vector3d_to_int32_clamp(
	real_vector3d const *v)
{
	long i;
	long j;
	long k;
	real_vector3d v2;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 104, v);

	i = fast_ftol((real)floor(PIN(v->i, -1.f, 1.f)*1023.5f))&0x7ff;
	j = fast_ftol((real)floor(PIN(v->j, -1.f, 1.f)*1023.5f))&0x7ff;
	k = fast_ftol((real)floor(PIN(v->k, -1.f, 1.f)*511.5f))&0x3ff;

	v2 = uncompress_int32_to_real_vector3d((k<<22) | (j<<11) | i);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 118, fabs(v2.i - v->i)<0.01f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 119, fabs(v2.j - v->j)<0.01f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 120, fabs(v2.k - v->k)<0.01f);

	return (k<<22) | (j<<11) | i;
}

real uncompress_int8_to_real(
	byte i)
{
	return i/255.f;
}

real uncompress_int16_to_real(
	short i)
{
	return (2.f*i + 1.f)/65535.f;
}

real_vector3d uncompress_int32_to_real_vector3d(
	unsigned long i)
{
	real_vector3d v;

	v.i = (((real)(long)((i&0x7ff)<<21))/1048576.f + 1.f)/2047.f;
	i >>= 11;
	v.j = (((real)(long)((i&0x7ff)<<21))/1048576.f + 1.f)/2047.f;
	i >>= 11;
	v.k = (((real)(long)(i<<22))/2097152.f + 1.f)/1023.f;

	return v;
}

long rasterizer_geometry_get_vertex_size(
	short type)
{
	static short const rasterizer_vertex_type_sizes[NUMBER_OF_RASTERIZER_VERTEX_TYPES+1] =
	{
		sizeof(struct environment_vertex_uncompressed),
		sizeof(struct environment_vertex_compressed),
		sizeof(struct environment_lightmap_vertex_uncompressed),
		sizeof(struct environment_lightmap_vertex_compressed),
		sizeof(struct model_vertex_uncompressed),
		sizeof(struct model_vertex_compressed),
		sizeof(struct dynamic_unlit_vertex),
		sizeof(struct dynamic_lit_vertex),
		sizeof(struct dynamic_screen_vertex),
		sizeof(struct debug_vertex),
		sizeof(struct decal_vertex),
		sizeof(struct detail_object_vertex)
	};

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 170, type>=0 && type<NUMBER_OF_RASTERIZER_VERTEX_TYPES);

	return rasterizer_vertex_type_sizes[type];
}

void rasterizer_geometry_byte_swap_vertices(
	short type,
	long count,
	void *vertices,
	long buffer_size)
{
	return;
}

void rasterizer_geometry_compress_vertices(
	short type,
	long count,
	void *compressed,
	long compressed_size,
	void const *uncompressed,
	long uncompressed_size)
{
	long i;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 194, uncompressed);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 195, compressed);

	switch (type)
	{
	case _rasterizer_vertex_type_environment_uncompressed:
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 201, count*sizeof(struct environment_vertex_uncompressed)==uncompressed_size);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 202, count*sizeof(struct environment_vertex_compressed)==compressed_size);

		for (i = 0; i<count; i++)
		{
			struct environment_vertex_uncompressed const *src = &((struct environment_vertex_uncompressed const *)uncompressed)[i];
			struct environment_vertex_compressed *dst = &((struct environment_vertex_compressed *)compressed)[i];

			dst->position = src->position;
			dst->normal = compress_real_vector3d_to_int32_clamp(&src->normal);
			dst->binormal = compress_real_vector3d_to_int32_clamp(&src->binormal);
			dst->tangent = compress_real_vector3d_to_int32_clamp(&src->tangent);
			dst->texcoord = src->texcoord;
		}
		break;
	}
	case _rasterizer_vertex_type_environment_lightmap_uncompressed:
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 221, count*sizeof(struct environment_lightmap_vertex_uncompressed)==uncompressed_size);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 222, count*sizeof(struct environment_lightmap_vertex_compressed)==compressed_size);

		for (i = 0; i<count; i++)
		{
			struct environment_lightmap_vertex_uncompressed const *src = &((struct environment_lightmap_vertex_uncompressed const *)uncompressed)[i];
			struct environment_lightmap_vertex_compressed *dst = &((struct environment_lightmap_vertex_compressed *)compressed)[i];

			dst->incident_radiosity = compress_real_vector3d_to_int32_clamp(&src->incident_radiosity);
			dst->lightmap_u = compress_real_to_int16_clamp(src->texcoord.x);
			dst->lightmap_v = compress_real_to_int16_clamp(src->texcoord.y);
		}
		break;
	}
	case _rasterizer_vertex_type_model_uncompressed:
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 239, count*sizeof(struct model_vertex_uncompressed)==uncompressed_size);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 240, count*sizeof(struct model_vertex_compressed)==compressed_size);

		for (i = 0; i<count; i++)
		{
			struct model_vertex_uncompressed const *src = &((struct model_vertex_uncompressed const *)uncompressed)[i];
			struct model_vertex_compressed *dst = &((struct model_vertex_compressed *)compressed)[i];

			dst->position = src->position;
			dst->normal = compress_real_vector3d_to_int32_clamp(&src->normal);
			dst->binormal = compress_real_vector3d_to_int32_clamp(&src->binormal);
			dst->tangent = compress_real_vector3d_to_int32_clamp(&src->tangent);
			dst->texcoord_u = compress_real_to_int16_clamp(src->texcoord.x);
			dst->texcoord_v = compress_real_to_int16_clamp(src->texcoord.y);
			dst->nodes[0] = src->nodes[0]*3;
			dst->nodes[1] = src->nodes[1]*3;
			dst->weights[0] = compress_real_to_int16_clamp(src->weights[0]);
		}
		break;
	}
	default:
		error(_error_silent, "### ERROR can't compress this type of vertex buffer");
	}

	return;
}

void rasterizer_geometry_uncompress_vertices(
	short type,
	long count,
	void *uncompressed,
	long uncompressed_size,
	void const *compressed,
	long compressed_size)
{
	long i;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 280, uncompressed);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 281, compressed);

	switch (type)
	{
	case _rasterizer_vertex_type_environment_compressed:
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 287, count*sizeof(struct environment_vertex_uncompressed)==uncompressed_size);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 288, count*sizeof(struct environment_vertex_compressed)==compressed_size);

		for (i = 0; i<count; i++)
		{
			struct environment_vertex_uncompressed *dst = &((struct environment_vertex_uncompressed *)uncompressed)[i];
			struct environment_vertex_compressed const *src = &((struct environment_vertex_compressed const *)compressed)[i];

			dst->position = src->position;
			dst->normal = uncompress_int32_to_real_vector3d(src->normal);
			dst->binormal = uncompress_int32_to_real_vector3d(src->binormal);
			dst->tangent = uncompress_int32_to_real_vector3d(src->tangent);
			dst->texcoord = src->texcoord;
		}
		break;
	}
	case _rasterizer_vertex_type_environment_lightmap_compressed:
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 307, count*sizeof(struct environment_lightmap_vertex_uncompressed)==uncompressed_size);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 308, count*sizeof(struct environment_lightmap_vertex_compressed)==compressed_size);

		for (i = 0; i<count; i++)
		{
			struct environment_lightmap_vertex_uncompressed *dst = &((struct environment_lightmap_vertex_uncompressed *)uncompressed)[i];
			struct environment_lightmap_vertex_compressed const *src = &((struct environment_lightmap_vertex_compressed const *)compressed)[i];

			dst->incident_radiosity = uncompress_int32_to_real_vector3d(src->incident_radiosity);
			dst->texcoord.x = uncompress_int16_to_real(src->lightmap_u);
			dst->texcoord.y = uncompress_int16_to_real(src->lightmap_v);
		}
		break;
	}
	case _rasterizer_vertex_type_model_compressed:
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 325, count*sizeof(struct model_vertex_uncompressed)==uncompressed_size);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 326, count*sizeof(struct model_vertex_compressed)==compressed_size);

		for (i = 0; i<count; i++)
		{
			struct model_vertex_uncompressed *dst = &((struct model_vertex_uncompressed *)uncompressed)[i];
			struct model_vertex_compressed const *src = &((struct model_vertex_compressed const *)compressed)[i];

			dst->position = src->position;
			dst->normal = uncompress_int32_to_real_vector3d(src->normal);
			dst->binormal = uncompress_int32_to_real_vector3d(src->binormal);
			dst->tangent = uncompress_int32_to_real_vector3d(src->tangent);
			dst->texcoord.x = uncompress_int16_to_real(src->texcoord_u);
			dst->texcoord.y = uncompress_int16_to_real(src->texcoord_v);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 341, src->nodes[0]%3==0);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 342, src->nodes[1]%3==0);
			dst->nodes[0] = src->nodes[0]/3;
			dst->nodes[1] = src->nodes[1]/3;
			dst->weights[0] = uncompress_int8_to_real((byte)src->weights[0]);
			dst->weights[1] = 1.f - dst->weights[0];
		}
		break;
	}
	default:
		error(_error_silent, "### ERROR can't uncompress this type of vertex buffer");
	}

	return;
}

boolean rasterizer_geometry_stripify(
	struct triangle_buffer *triangle_buffer,
	struct vertex_buffer *vertex_buffer)
{
	boolean success = TRUE;

	return success;
}

void environment_vertex_compressed_get_point(
	struct environment_vertex_compressed const *vertex,
	real_point3d *point)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 438, vertex);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 439, point);

	*point = vertex->position;

	return;
}

void environment_vertex_compressed_get_normal(
	struct environment_vertex_compressed const *vertex,
	real_vector3d *normal)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 450, vertex);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 451, normal);

	*normal = uncompress_int32_to_real_vector3d(vertex->normal);

	return;
}

void environment_vertex_compressed_get_texcoord(
	struct environment_vertex_compressed const *vertex,
	real_point2d *texcoord)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 462, vertex);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 463, texcoord);

	*texcoord = vertex->texcoord;

	return;
}

void environment_lightmap_vertex_compressed_get_incident_radiosity(
	struct environment_lightmap_vertex_compressed const *vertex,
	real_vector3d *normal)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 474, vertex);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 475, normal);

	*normal = uncompress_int32_to_real_vector3d(vertex->incident_radiosity);

	return;
}

void environment_lightmap_vertex_compressed_get_texcoord(
	struct environment_lightmap_vertex_compressed const *vertex,
	real_point2d *texcoord)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 486, vertex);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_geometry.c", 487, texcoord);

	texcoord->x = uncompress_int16_to_real(vertex->lightmap_u);
	texcoord->y = uncompress_int16_to_real(vertex->lightmap_v);

	return;
}
