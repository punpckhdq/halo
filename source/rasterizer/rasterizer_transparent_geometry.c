/*
RASTERIZER_TRANSPARENT_GEOMETRY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "bitmaps_inlines.h"
#include "light_definitions.h"
#include "shaders.h"
#include "shader_definitions.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer/xbox/rasterizer_xbox.h"
#include "rasterizer_geometry.h"

/* ---------- prototypes */

__inline static void rasterizer_sort_internal(struct transparent_geometry_group *group);
static long group_sorted_indices_cmpfn(short const *group_index1, short const *group_index2);
static void rasterizer_sort_external(void);

/* ---------- globals */

static unsigned long transparent_geometry_group_pending_flags[BIT_VECTOR_SIZE_IN_LONGS(RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS)]; /* fake name */
static struct transparent_geometry_group *transparent_geometry_groups;
static struct transparent_geometry_group *transparent_geometry_groups2;
static long transparent_geometry_group_count;
static long transparent_geometry_group_count2;
static short *transparent_geometry_group_sorted_indices;
static short transparent_geometry_attached_group_count = 0;

/* ---------- public code */

boolean rasterizer_transparent_geometry_initialize(
	void)
{
	boolean success = TRUE;

	transparent_geometry_groups = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 41, RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS*sizeof(struct transparent_geometry_group));
	transparent_geometry_group_sorted_indices = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 43, RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS*sizeof(short));
	transparent_geometry_groups2 = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 46, RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS2*sizeof(struct transparent_geometry_group));
	transparent_geometry_group_count2 = 0;
	transparent_geometry_group_count = 0;

	if (!transparent_geometry_groups || !transparent_geometry_group_sorted_indices || !transparent_geometry_groups2)
	{
		error(_error_silent, "### ERROR failed to allocate transparent geometry buffer");
		success = FALSE;
	}

	if (success && !rasterizer_transparent_geometry_initialize_aux_buffer())
	{
		success = FALSE;
	}

	return success;
}

void rasterizer_transparent_geometry_begin(
	void)
{
	transparent_geometry_group_count = 0;
	transparent_geometry_attached_group_count = 0;
	memset(transparent_geometry_group_pending_flags, 0, sizeof(transparent_geometry_group_pending_flags));
	transparent_geometry_group_count2 = 0;

	return;
}

struct transparent_geometry_group *rasterizer_transparent_geometry_new_group(
	void)
{
	struct transparent_geometry_group *group = NULL;

	if (transparent_geometry_group_count<RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS)
	{
		group = &transparent_geometry_groups[transparent_geometry_group_count];
		group->sorted_index = transparent_geometry_group_count;
		transparent_geometry_group_count++;
	}

	return group;
}

struct transparent_geometry_group *rasterizer_transparent_geometry_new_group2(
	void)
{
	struct transparent_geometry_group *group = NULL;

	if (transparent_geometry_group_count2<RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS2)
	{
		group = &transparent_geometry_groups2[transparent_geometry_group_count2];
		group->sorted_index = transparent_geometry_group_count2;
		transparent_geometry_group_count2++;
	}

	return group;
}

struct transparent_geometry_group const *rasterizer_transparent_geometry_get_groups2(
	short *count)
{
	if (count)
	{
		*count = (short)transparent_geometry_group_count2;
	}

	return transparent_geometry_groups2;
}

struct transparent_geometry_group const *rasterizer_transparent_geometry_next_group(
	struct transparent_geometry_group const *group)
{
	if (group)
	{
		short next_group_sorted_index = (short)group->sorted_index+1;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 137, group->sorted_index>=0 && group->sorted_index<transparent_geometry_group_count);
		if (next_group_sorted_index<transparent_geometry_group_count)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 141, next_group_sorted_index>=0);
			group = &transparent_geometry_groups[transparent_geometry_group_sorted_indices[next_group_sorted_index]];
		}
		else
		{
			group = NULL;
		}
	}

	return group;
}

struct transparent_geometry_group *rasterizer_transparent_geometry_get_groups(
	void)
{
	return transparent_geometry_groups;
}

struct transparent_geometry_group const *rasterizer_transparent_geometry_get_group_from_presorted_index(
	short group_presorted_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 188, group_presorted_index>=0 && group_presorted_index<transparent_geometry_group_count);

	return &transparent_geometry_groups[group_presorted_index];
}

short rasterizer_transparent_geometry_get_group_presorted_index(
	struct transparent_geometry_group const *group)
{
	short group_presorted_index = NONE;

	if (group>=transparent_geometry_groups && group<&transparent_geometry_groups[transparent_geometry_group_count])
	{
		group_presorted_index = (short)(group-transparent_geometry_groups);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 203, group_presorted_index>=0 && group_presorted_index<transparent_geometry_group_count);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 204, ((unsigned long)group-(unsigned long)transparent_geometry_groups)%sizeof(struct transparent_geometry_group)==0);
	}

	return group_presorted_index;
}

boolean rasterizer_transparent_geometry_get_group_pending_status(
	struct transparent_geometry_group const *group)
{
	short group_presorted_index = rasterizer_transparent_geometry_get_group_presorted_index(group);
	boolean pending = TRUE;

	if (group_presorted_index!=NONE)
	{
		pending = !BIT_VECTOR_TEST_FLAG(transparent_geometry_group_pending_flags, group_presorted_index);
	}

	return pending;
}

void rasterizer_transparent_geometry_set_group_pending_status(
	struct transparent_geometry_group const *group,
	boolean status)
{
	short group_presorted_index = rasterizer_transparent_geometry_get_group_presorted_index(group);

	if (group_presorted_index!=NONE)
	{
		BIT_VECTOR_SET_FLAG(transparent_geometry_group_pending_flags, group_presorted_index, !status);
	}

	return;
}

short rasterizer_transparent_geometry_get_primary_vertex_type(
	struct transparent_geometry_group const *group)
{
	short vertex_type = NONE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 244, group);

	if (group->vertex_buffers)
	{
		vertex_type = group->vertex_buffers->type;
	}
	else if (group->dynamic_vertex_buffer_index!=NONE)
	{
		vertex_type = rasterizer_dynamic_vertices_get_type(group->dynamic_vertex_buffer_index);
	}
	else
	{
		error(_error_silent, "### ERROR transparent geometry group has no vertices");
	}

	return vertex_type;
}

void rasterizer_transparent_geometry_end(
	void)
{
	return;
}

void rasterizer_transparent_geometry_dispose(
	void)
{
	rasterizer_transparent_geometry_dispose_aux_buffer();

	if (transparent_geometry_groups)
	{
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 273, transparent_geometry_groups);
	}
	transparent_geometry_groups = NULL;

	if (transparent_geometry_group_sorted_indices)
	{
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 276, transparent_geometry_group_sorted_indices);
	}
	transparent_geometry_group_sorted_indices = NULL;

	if (transparent_geometry_groups2)
	{
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 280, transparent_geometry_groups2);
	}
	transparent_geometry_groups2 = NULL;

	transparent_geometry_group_count2 = 0;
	transparent_geometry_group_count = 0;

	return;
}

void rasterizer_transparent_geometry_draw(
	boolean water)
{
	static short group_index;
	boolean first_person_flag;

	rasterizer_profile_begin(water ? _rasterizer_profile_water : _rasterizer_profile_queued_transparents);

	if (transparent_geometry_group_count>0)
	{
		first_person_flag = FALSE;
		if (water)
		{
			rasterizer_sort_external();
			group_index = 0;
		}

		if (water && global_window_parameters.window_index!=NONE)
		{
			rasterizer_debug_options.__unknown88 = TRUE;
		}

		rasterizer_transparent_geometry_groups_begin();
		rasterizer_debug_options.__unknown88 = FALSE;

		for (; group_index<transparent_geometry_group_count; group_index++)
		{
			struct transparent_geometry_group *group = &transparent_geometry_groups[transparent_geometry_group_sorted_indices[group_index]];

			if (water && (!group->shader || group->shader->base.type!=_shader_type_transparent_water && !shader_is_water_decal(group->shader)))
			{
				break;
			}

			if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_first_person_bit))
			{
				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 340, !water);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 341, global_window_parameters.rasterizer_target==_rasterizer_target_render_primary);
				if (!first_person_flag)
				{
					rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);
					rasterizer_set_frustum_z(rasterizer_globals.z_near_first_person, rasterizer_globals.z_far_first_person);
					first_person_flag = TRUE;
				}
			}
			else
			{
				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 355, !first_person_flag);
			}

			rasterizer_transparent_geometry_group_draw(group, FALSE);
		}

		if (!water && global_window_parameters.window_index!=NONE)
		{
			rasterizer_debug_options.__unknown88 = TRUE;
		}
		rasterizer_transparent_geometry_groups_end();
		rasterizer_debug_options.__unknown88 = FALSE;

		if (first_person_flag)
		{
			rasterizer_set_frustum_z(0.f, 0.f);
		}
	}

	rasterizer_profile_end(water ? _rasterizer_profile_water : _rasterizer_profile_queued_transparents);

	return;
}

void rasterizer_transparent_geometry_stop(
	void)
{
	rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);

	return;
}

__inline static void rasterizer_sort_internal(
	struct transparent_geometry_group *group)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 402, group);

	return;
}

static long group_sorted_indices_cmpfn(
	short const *group_index1,
	short const *group_index2)
{
	struct transparent_geometry_group *group1;
	struct transparent_geometry_group *group2;
	long result = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 426, group_index1 && (*group_index1)>=0 && (*group_index1)<transparent_geometry_group_count);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_transparent_geometry.c", 427, group_index2 && (*group_index2)>=0 && (*group_index2)<transparent_geometry_group_count);

	group1 = &transparent_geometry_groups[*group_index1];
	group2 = &transparent_geometry_groups[*group_index2];

	if (shader_is_water_decal(group1->shader))
	{
		result = -1;
	}
	else if (shader_is_water_decal(group2->shader))
	{
		result = 1;
	}
	else if (group1->shader && group1->shader->base.type==_shader_type_transparent_water)
	{
		result = -1;
	}
	else if (group2->shader && group2->shader->base.type==_shader_type_transparent_water)
	{
		result = 1;
	}
	else if (TEST_FLAG(group1->geometry_flags, _rasterizer_geometry_first_person_bit) && !TEST_FLAG(group2->geometry_flags, _rasterizer_geometry_first_person_bit))
	{
		result = 1;
	}
	else if (TEST_FLAG(group2->geometry_flags, _rasterizer_geometry_first_person_bit) && !TEST_FLAG(group1->geometry_flags, _rasterizer_geometry_first_person_bit))
	{
		result = -1;
	}
	else if (group1->z_sort>group2->z_sort)
	{
		result = 1;
	}
	else if (group1->z_sort<group2->z_sort)
	{
		result = -1;
	}
	else if (group1->source_object_index>group2->source_object_index)
	{
		result = 1;
	}
	else if (group1->source_object_index<group2->source_object_index)
	{
		result = -1;
	}

	if (group1->cortana_hack && !group2->cortana_hack)
	{
		result = 1;
	}
	else if (group2->cortana_hack && !group1->cortana_hack)
	{
		result = -1;
	}

	return result;
}

static void rasterizer_sort_external(
	void)
{
	short group_index;

	for (group_index = 0; group_index<transparent_geometry_group_count; group_index++)
	{
		struct transparent_geometry_group *group = &transparent_geometry_groups[group_index];

		rasterizer_sort_internal(group);
		transparent_geometry_group_sorted_indices[group_index] = group_index;
	}

	qsort(transparent_geometry_group_sorted_indices, transparent_geometry_group_count, sizeof(short), (int(__cdecl *)(const void *, const void *))group_sorted_indices_cmpfn);

	for (group_index = 0; group_index<transparent_geometry_group_count; group_index++)
	{
		transparent_geometry_groups[transparent_geometry_group_sorted_indices[group_index]].sorted_index = group_index;
	}

	return;
}
