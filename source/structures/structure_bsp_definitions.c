/*
STRUCTURE_BSP_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "structure_bsp_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

unsigned long *structure_bsp_get_cluster_pvs(
	struct structure_bsp *structure_bsp,
	short cluster_index)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 36, cluster_index>=0 && cluster_index<structure_bsp->clusters.count);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c",
		37,
		(cluster_index+1)*BIT_VECTOR_SIZE_IN_LONGS(structure_bsp->clusters.count)<=structure_bsp->cluster_data.size);

	// Get pointer to bitvector starting at the cluster index
	return (unsigned long *)(
		(byte *)structure_bsp->cluster_data.address +
		sizeof(unsigned long) * cluster_index *
		BIT_VECTOR_SIZE_IN_LONGS(structure_bsp->clusters.count));
}

void structure_bsp_find_material_for_surface(
	struct structure_bsp *structure,
	long surface_index,
	short *lightmap_index,
	short *material_index)
{
	struct structure_lightmap *lightmap;
	struct structure_material *material;
	short lightmap_last_index;
	short material_last_index;
	short i;

	i=0;
	*lightmap_index= 0;
	lightmap_last_index= structure->lightmaps.count-1;

	while (lightmap_last_index>i)
	{
		struct structure_lightmap *curr_lightmap;

		*lightmap_index= (lightmap_last_index-i) / 2+i;
		curr_lightmap= TAG_BLOCK_GET_ELEMENT(&structure->lightmaps, *lightmap_index, struct structure_lightmap);

		if (surface_index<TAG_BLOCK_GET_ELEMENT(&curr_lightmap->materials, 0, struct structure_material)->first_surface_index)
		{
			lightmap_last_index= *lightmap_index-1;
			*lightmap_index= lightmap_last_index;
		}
		else
		{
			if (surface_index<
				TAG_BLOCK_GET_ELEMENT(&curr_lightmap->materials, curr_lightmap->materials.count-1, struct structure_material)->surface_count+
				TAG_BLOCK_GET_ELEMENT(&curr_lightmap->materials, curr_lightmap->materials.count-1, struct structure_material)->first_surface_index)
			{
				break;
			}

			i= *lightmap_index+1;
			*lightmap_index= i;
		}
	}

	lightmap= TAG_BLOCK_GET_ELEMENT(&structure->lightmaps, *lightmap_index, struct structure_lightmap);

	i=0;
	*material_index= 0;
	material_last_index= lightmap->materials.count;

	while (i<material_last_index)
	{
		const struct structure_material *curr_material;

		*material_index= (material_last_index-i) / 2+i;
		curr_material= TAG_BLOCK_GET_ELEMENT(&lightmap->materials, *material_index, struct structure_material);

		if (surface_index<curr_material->first_surface_index)
		{
			material_last_index= *material_index-1;
			*material_index= material_last_index;
		}
		else
		{
			if (surface_index<curr_material->first_surface_index+curr_material->surface_count)
			{
				break;
			}

			i= *material_index+1;
			*material_index= i;
		}
		
	}

	material= TAG_BLOCK_GET_ELEMENT(&lightmap->materials, *material_index, struct structure_material);

	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 102, surface_index>=material->first_surface_index);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 103, surface_index<material->first_surface_index+material->surface_count);

	return;
}

void vertex_type_from_shader_tag(
	unsigned long group_tag,
	short *vertex_type,
	short *lightmap_vertex_type,
	boolean compressed)
{
	if (compressed)
	{
		*vertex_type= _rasterizer_vertex_type_environment_compressed;
		*lightmap_vertex_type= _rasterizer_vertex_type_environment_lightmap_compressed;
	}
	else
	{
		*vertex_type= _rasterizer_vertex_type_environment_uncompressed;
		*lightmap_vertex_type= _rasterizer_vertex_type_environment_lightmap_uncompressed;
	}
	
	return;
}

byte *structure_bsp_get_cluster_encoded_sound_data(
	struct structure_bsp *structure_bsp,
	short row_index,
	short column_index)
{
	short offset= row_index * (structure_bsp->clusters.count-1)-row_index*(row_index+1)/2+column_index-1;

	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 1202, row_index<column_index);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 1203, offset>=0 && offset<structure_bsp->sound_cluster_data.size);

	return &((byte *)structure_bsp->sound_cluster_data.address)[offset];
}

byte structure_bsp_get_cluster_encoded_sound_distance(
	struct structure_bsp *structure_bsp,
	short from_cluster_index,
	short to_cluster_index)
{
	byte result;

	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 1215, from_cluster_index>=0 && from_cluster_index<structure_bsp->clusters.count);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 1216, to_cluster_index>=0 && to_cluster_index<structure_bsp->clusters.count);

	if (from_cluster_index!=to_cluster_index)
	{
		if (from_cluster_index>to_cluster_index)
		{
			short temp= from_cluster_index;
			from_cluster_index= to_cluster_index;
			to_cluster_index= temp;
		}

		result= *structure_bsp_get_cluster_encoded_sound_data(
			structure_bsp, 
			from_cluster_index,
			to_cluster_index);
	}
	else
	{
		result= 0;
	}

	return result;
}


/* ---------- private code */
