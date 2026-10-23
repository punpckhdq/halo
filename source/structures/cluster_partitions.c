/*
CLUSTER_PARTITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cluster_partitions.h"
#include "structures.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "game_state.h"
#include "reference_lists.h"

/* ---------- constants */

enum
{
	MAXIMUM_CLUSTER_PARTITION_REFERENCES = 2048, /* fake name */
	MAXIMUM_CLUSTERS_SPANNED_PER_DATUM = 64, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static long *cluster_partition_get_first_reference(struct cluster_partition const *partition, short cluster_index);

/* ---------- globals */

/* ---------- public code */

void cluster_partition_new(
	struct cluster_partition *partition,
	char const *name)
{
	char buffer[256];

	partition->cluster_first_data_references = game_state_malloc(
		name,
		"cluster references",
		MAXIMUM_CLUSTERS_PER_STRUCTURE * sizeof(*partition->cluster_first_data_references));
	sprintf(buffer, "cluster %s", name);
	partition->data_reference_data = reference_list_new(buffer, MAXIMUM_CLUSTER_PARTITION_REFERENCES);
	sprintf(buffer, "%s cluster", name);
	partition->cluster_reference_data = reference_list_new(buffer, MAXIMUM_CLUSTER_PARTITION_REFERENCES);

	if (!partition->cluster_first_data_references || !partition->cluster_reference_data || !partition->data_reference_data)
	{
		error(_error_immediate, "couldn't allocate %s cluster partition globals", name);
	}

	return;
}

void cluster_partition_make_valid(
	struct cluster_partition *partition)
{
	memset(
		partition->cluster_first_data_references,
		NONE,
		MAXIMUM_CLUSTERS_PER_STRUCTURE * sizeof(*partition->cluster_first_data_references));
	data_make_valid(partition->cluster_reference_data);
	data_make_valid(partition->data_reference_data);

	return;
}

void cluster_partition_make_invalid(
	struct cluster_partition *partition)
{
	if (partition->cluster_reference_data->valid)
	{
		data_make_invalid(partition->cluster_reference_data);
	}

	if (partition->data_reference_data->valid)
	{
		data_make_invalid(partition->data_reference_data);
	}

	return;
}

void cluster_partition_delete(
	struct cluster_partition *partition)
{
	if (partition->cluster_first_data_references)
	{
		partition->cluster_first_data_references = NULL;
	}

	if (partition->cluster_reference_data)
	{
		partition->cluster_reference_data = NULL;
	}

	if (partition->data_reference_data)
	{
		partition->data_reference_data = NULL;
	}

	return;
}

void cluster_partition_reconnect(
	struct cluster_partition *partition,
	long datum_index,
	long *first_cluster_reference,
	real_point3d const *position,
	real radius,
	struct location const *location)
{
	short spanning_cluster_indices[MAXIMUM_CLUSTERS_SPANNED_PER_DATUM];
	short spanning_cluster_count;
	short spanning_cluster_index;

	match_assert("c:\\halo\\SOURCE\\structures\\cluster_partitions.c", 111, partition);
	match_assert("c:\\halo\\SOURCE\\structures\\cluster_partitions.c", 112, first_cluster_reference);
	match_assert("c:\\halo\\SOURCE\\structures\\cluster_partitions.c", 113, *first_cluster_reference==NONE);
	match_assert("c:\\halo\\SOURCE\\structures\\cluster_partitions.c", 114, position);
	match_assert("c:\\halo\\SOURCE\\structures\\cluster_partitions.c", 115, location);

	spanning_cluster_count = structure_clusters_in_sphere(
		location->cluster_index,
		position,
		radius,
		MAXIMUM_CLUSTERS_SPANNED_PER_DATUM,
		spanning_cluster_indices);

	if (spanning_cluster_count > MAXIMUM_CLUSTERS_SPANNED_PER_DATUM)
	{
		error(_error_silent, "an object or light spanned %d clusters.", spanning_cluster_count);
		spanning_cluster_count = MAXIMUM_CLUSTERS_SPANNED_PER_DATUM;
	}

	for (spanning_cluster_index = 0; spanning_cluster_index < spanning_cluster_count; spanning_cluster_index++)
	{
		short cluster_index = spanning_cluster_indices[spanning_cluster_index];

		reference_list_add(partition->cluster_reference_data, first_cluster_reference, cluster_index);
		reference_list_add(
			partition->data_reference_data,
			cluster_partition_get_first_reference(partition, cluster_index),
			datum_index);
	}

	return;
}

void cluster_partition_disconnect(
	struct cluster_partition *partition,
	long datum_index,
	long *first_cluster_reference)
{
	long reference_index;
	struct data_reference *reference;
	short cluster_index;

	for (reference_index = *first_cluster_reference; reference_index != NONE; reference_index = reference->next_reference_index)
	{
		reference = (struct data_reference *)datum_get(partition->cluster_reference_data, reference_index);
		cluster_index = (short)reference->datum_index;
		datum_delete(partition->cluster_reference_data, reference_index);
		reference_list_remove(
			partition->data_reference_data,
			cluster_partition_get_first_reference(partition, cluster_index),
			datum_index);
	}

	*first_cluster_reference = NONE;

	return;
}

long cluster_partition_get_first_datum(
	struct cluster_partition const *partition,
	long *reference_index,
	short cluster_index)
{
	*reference_index = *cluster_partition_get_first_reference(partition, cluster_index);

	return reference_list_get_next_datum_index(partition->data_reference_data, reference_index);
}

long cluster_partition_get_next_datum(
	struct cluster_partition const *partition,
	long *reference_index)
{
	return reference_list_get_next_datum_index(partition->data_reference_data, reference_index);
}

long cluster_partition_get_first_cluster(
	struct cluster_partition const *partition,
	long *reference_index,
	long first_cluster_reference)
{
	*reference_index = first_cluster_reference;

	return reference_list_get_next_datum_index(partition->cluster_reference_data, reference_index);
}

long cluster_partition_get_next_cluster(
	struct cluster_partition const *partition,
	long *reference_index)
{
	return reference_list_get_next_datum_index(partition->cluster_reference_data, reference_index);
}

void cluster_partition_copy(
	struct cluster_partition *destination,
	struct cluster_partition const *source)
{
	memcpy(
		destination->cluster_first_data_references,
		source->cluster_first_data_references,
		global_structure_bsp_get()->clusters.count * sizeof(*destination->cluster_first_data_references));
	reference_list_copy(destination->cluster_reference_data, source->cluster_reference_data);
	reference_list_copy(destination->data_reference_data, source->data_reference_data);

	return;
}

/* ---------- private code */

static long *cluster_partition_get_first_reference(
	struct cluster_partition const *partition,
	short cluster_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\structures\\cluster_partitions.c",
		213,
		cluster_index>=0 && cluster_index<global_structure_bsp_get()->clusters.count);

	return &partition->cluster_first_data_references[cluster_index];
}
