/*
VECTOR_TREE.H

header included in hcex build.
*/

#ifndef __VECTOR_TREE_H
#define __VECTOR_TREE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

typedef void *(*vector_tree_get_vector_proc)(void *data, long vector_index); /* fake name */
typedef long (*vector_tree_compare_component_proc)(void *data, void *vector0, void *vector1, short component); /* fake name */

struct vector_tree /* fake name */
{
	long root_node_index; /* fake name */
	struct dynamic_array nodes; /* fake name */
	short component_count;
	void *data; /* fake name */
	vector_tree_get_vector_proc get_vector;
	vector_tree_compare_component_proc compare_component;
};

/* ---------- prototypes/VECTOR_TREE.C */

void vector_tree_new(struct vector_tree *tree, short component_count, void *data, vector_tree_get_vector_proc get_vector, vector_tree_compare_component_proc compare_component);
void vector_tree_delete(struct vector_tree *tree);
boolean vector_tree_find(struct vector_tree *tree, void *vector, long **index_reference);

#endif // __VECTOR_TREE_H
