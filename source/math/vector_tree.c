/*
VECTOR_TREE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "array.h"
#include "vector_tree.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct vector_tree_node /* fake name */
{
	long vector_index; /* fake name */
	long lesser_node_index; /* fake name */
	long equal_node_index; /* fake name */
	long greater_node_index; /* fake name */
};

/* ---------- prototypes */

static boolean vectors_equal(struct vector_tree *tree, void *vector0, void *vector1, short component);
static void vector_tree_add_node(struct vector_tree *tree, long **index_reference);

/* ---------- globals */

/* ---------- public code */

void vector_tree_new(
	struct vector_tree *tree,
	short component_count,
	void *data,
	vector_tree_get_vector_proc get_vector,
	vector_tree_compare_component_proc compare_component)
{
	match_assert("c:\\halo\\SOURCE\\math\\vector_tree.c", 43, tree);
	match_assert("c:\\halo\\SOURCE\\math\\vector_tree.c", 44, get_vector);
	match_assert("c:\\halo\\SOURCE\\math\\vector_tree.c", 45, compare_component);
	match_assert("c:\\halo\\SOURCE\\math\\vector_tree.c", 46, component_count>0);

	dynamic_array_new(&tree->nodes, sizeof(struct vector_tree_node));
	tree->root_node_index = NONE;
	tree->data = data;
	tree->get_vector = get_vector;
	tree->compare_component = compare_component;
	tree->component_count = component_count;

	return;
}

void vector_tree_delete(
	struct vector_tree *tree)
{
	dynamic_array_delete(&tree->nodes);

	return;
}

static boolean vectors_equal(
	struct vector_tree *tree,
	void *vector0,
	void *vector1,
	short component)
{
	for (; component<tree->component_count; component++)
	{
		if (tree->compare_component(tree->data, vector0, vector1, component))
		{
			return FALSE;
		}
	}

	return TRUE;
}

static void vector_tree_add_node(
	struct vector_tree *tree,
	long **index_reference)
{
	long node_index = dynamic_array_add_element(&tree->nodes);

	if (node_index!=NONE)
	{
		struct vector_tree_node *node = dynamic_array_get_element(&tree->nodes, node_index, sizeof(struct vector_tree_node));

		node->lesser_node_index = NONE;
		node->equal_node_index = NONE;
		node->greater_node_index = NONE;
		*index_reference = &node->vector_index;
	}
	else
	{
		*index_reference = NULL;
	}

	return;
}

boolean vector_tree_find(
	struct vector_tree *tree,
	void *vector,
	long **index_reference)
{
	long *node_index_reference = &tree->root_node_index;
	short component = 0;

	match_assert("c:\\halo\\SOURCE\\math\\vector_tree.c", 74, tree);
	match_assert("c:\\halo\\SOURCE\\math\\vector_tree.c", 75, vector);
	match_assert("c:\\halo\\SOURCE\\math\\vector_tree.c", 76, index_reference);

	while (*node_index_reference!=NONE)
	{
		struct vector_tree_node *node = dynamic_array_get_element(&tree->nodes, *node_index_reference, sizeof(struct vector_tree_node));
		void *node_vector = tree->get_vector(tree->data, node->vector_index);
		long comparison = tree->compare_component(tree->data, vector, node_vector, component);

		if (comparison<0)
		{
			node_index_reference = &node->lesser_node_index;
		}
		else if (comparison>0)
		{
			node_index_reference = &node->greater_node_index;
		}
		else
		{
			component++;
			if (vectors_equal(tree, vector, node_vector, component))
			{
				*index_reference = &node->vector_index;
				return TRUE;
			}
			else
			{
				node_index_reference = &node->equal_node_index;
			}
		}
	}

	*node_index_reference = tree->nodes.count;
	vector_tree_add_node(tree, index_reference);

	return FALSE;
}

/* ---------- private code */
