/*
COLLISION_BSP_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "collision_bsp_definitions.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static struct tag_block_definition bsp3d_nodes_block;
static struct tag_block_definition planes_block;
static struct tag_block_definition leaves_block;
static struct tag_block_definition bsp2d_references_block;
static struct tag_block_definition bsp2d_nodes_block;
static struct tag_block_definition surfaces_block;
static struct tag_block_definition edges_block;
static struct tag_block_definition vertices_block;

struct tag_field global_collision_bsp_fields[] =
{
	{_field_block, "bsp3d nodes*", &bsp3d_nodes_block},
	{_field_block, "planes*", &planes_block},
	{_field_block, "leaves*", &leaves_block},
	{_field_block, "bsp2d references*", &bsp2d_references_block},
	{_field_block, "bsp2d nodes*", &bsp2d_nodes_block},
	{_field_block, "surfaces*", &surfaces_block},
	{_field_block, "edges*", &edges_block},
	{_field_block, "vertices*", &vertices_block},
	{_field_terminator}
};

TAG_BLOCK(bsp3d_nodes_block, "bsp3d node", MAXIMUM_NODES_PER_BSP3D, sizeof(struct bsp3d_node), NULL, NULL, NULL, NULL)
{
	{_field_long_integer, "plane*"},
	{_field_long_integer, "back child*"},
	{_field_long_integer, "front child*"},
	{_field_terminator}
};

TAG_BLOCK(planes_block, "plane", MAXIMUM_PLANES_PER_BSP3D, sizeof(real_plane3d), NULL, NULL, NULL, NULL)
{
	{_field_real_plane3d, "plane*"},
	{_field_terminator}
};

static char *collision_leaf_flags_strings[] = /* fake name */
{
	"contains double-sided surfaces"
};

static struct flags_definition collision_leaf_flags = /* fake name */
{
	NUMBEROF(collision_leaf_flags_strings),
	collision_leaf_flags_strings
};

TAG_BLOCK(leaves_block, "leaf", MAXIMUM_LEAVES_PER_BSP3D, sizeof(struct collision_leaf), NULL, NULL, NULL, NULL)
{
	{_field_word_flags, "flags*", &collision_leaf_flags},
	{_field_short_integer, "bsp2d reference count*"},
	{_field_long_integer, "first bsp2d reference*"},
	{_field_terminator}
};

TAG_BLOCK(bsp2d_references_block, "bsp2d reference", MAXIMUM_BSP2D_REFERENCES_PER_COLLISION_BSP, sizeof(struct bsp2d_reference), NULL, NULL, NULL, NULL)
{
	{_field_long_integer, "plane*", &planes_block},
	{_field_long_integer, "bsp2d node*", &bsp2d_nodes_block},
	{_field_terminator}
};

TAG_BLOCK(bsp2d_nodes_block, "bsp2d node", UNSIGNED_SHORT_MAX, sizeof(struct bsp2d_node), NULL, NULL, NULL, NULL)
{
	{_field_real_plane2d, "plane*"},
	{_field_long_integer, "left child*"},
	{_field_long_integer, "right child*"},
	{_field_terminator}
};

static char *collision_surface_flags_strings[] = /* fake name */
{
	"two sided",
	"invisible",
	"climbable",
	"breakable"
};

static struct flags_definition collision_surface_flags = /* fake name */
{
	NUMBEROF(collision_surface_flags_strings),
	collision_surface_flags_strings
};

TAG_BLOCK(surfaces_block, "surface", MAXIMUM_SURFACES_PER_COLLISION_BSP, sizeof(struct collision_surface), NULL, NULL, NULL, NULL)
{
	{_field_long_integer, "plane*"},
	{_field_long_integer, "first edge*"},
	{_field_byte_flags, "flags*", &collision_surface_flags},
	{_field_char_integer, "breakable surface*"},
	{_field_short_integer, "material*"},
	{_field_terminator}
};

TAG_BLOCK(edges_block, "edge", MAXIMUM_EDGES_PER_COLLISION_BSP, sizeof(struct collision_edge), NULL, NULL, NULL, NULL)
{
	{_field_long_integer, "start vertex*"},
	{_field_long_integer, "end vertex*"},
	{_field_long_integer, "forward edge*"},
	{_field_long_integer, "reverse edge*"},
	{_field_long_integer, "left surface*"},
	{_field_long_integer, "right surface*"},
	{_field_terminator}
};

TAG_BLOCK(vertices_block, "vertex", MAXIMUM_VERTICES_PER_COLLISION_BSP, sizeof(struct collision_vertex), NULL, NULL, NULL, NULL)
{
	{_field_real_point3d, "point*"},
	{_field_long_integer, "first edge*"},
	{_field_terminator}
};

/* ---------- public code */

/* ---------- private code */
