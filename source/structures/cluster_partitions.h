/*
CLUSTER_PARTITIONS.H

header included in hcex build.
*/

#ifndef __CLUSTER_PARTITIONS_H
#define __CLUSTER_PARTITIONS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct cluster_partition
{
	long *cluster_first_data_references;
	struct data_array *data_reference_data;
	struct data_array *cluster_reference_data;
};

/* ---------- prototypes/CLUSTER_PARTITIONS.C */

void cluster_partition_new(struct cluster_partition *partition, char const *name);
void cluster_partition_make_valid(struct cluster_partition *partition);
void cluster_partition_make_invalid(struct cluster_partition *partition);
void cluster_partition_delete(struct cluster_partition *partition);
void cluster_partition_reconnect(
	struct cluster_partition *partition,
	long datum_index,
	long *first_cluster_reference,
	real_point3d const *position,
	float radius,
	struct location const *location);
void cluster_partition_disconnect(struct cluster_partition *partition, long datum_index, long *first_cluster_reference);
long cluster_partition_get_first_datum(struct cluster_partition const *partition, long *reference_index, short cluster_index);
long cluster_partition_get_next_datum(struct cluster_partition const *partition, long *reference_index);
long cluster_partition_get_first_cluster(struct cluster_partition const *partition, long *reference_index, long first_cluster_reference);
long cluster_partition_get_next_cluster(struct cluster_partition const *partition, long *reference_index);


/* ---------- globals */

/* ---------- public code */

#endif // __CLUSTER_PARTITIONS_H
