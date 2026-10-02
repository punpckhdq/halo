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

static struct tag_block_definition bsp3d_node_block; /* fake name */
static struct tag_block_definition plane_block; /* fake name */
static struct tag_block_definition collision_leaf_block; /* fake name */
static struct tag_block_definition bsp2d_reference_block; /* fake name */
static struct tag_block_definition bsp2d_node_block; /* fake name */
static struct tag_block_definition collision_surface_block; /* fake name */
static struct tag_block_definition collision_edge_block; /* fake name */
static struct tag_block_definition collision_vertex_block; /* fake name */

struct tag_field global_collision_bsp_fields[] =
{
	{ _field_block, "bsp3d nodes*", &bsp3d_node_block },
	{ _field_block, "planes*", &plane_block },
	{ _field_block, "leaves*", &collision_leaf_block },
	{ _field_block, "bsp2d references*", &bsp2d_reference_block },
	{ _field_block, "bsp2d nodes*", &bsp2d_node_block },
	{ _field_block, "surfaces*", &collision_surface_block },
	{ _field_block, "edges*", &collision_edge_block },
	{ _field_block, "vertices*", &collision_vertex_block },
	{ _field_terminator }
};

static struct tag_field bsp3d_node_fields[] = /* fake name */
{
	{ _field_long_integer, "plane*" },
	{ _field_long_integer, "back child*" },
	{ _field_long_integer, "front child*" },
	{ _field_terminator }
};

static struct tag_block_definition bsp3d_node_block = /* fake name */
{
	"bsp3d node",
	0,
	MAXIMUM_NODES_PER_BSP3D,
	sizeof(struct bsp3d_node),
	NULL,
	bsp3d_node_fields
};

static struct tag_field plane_fields[] = /* fake name */
{
	{ _field_real_plane3d, "plane*" },
	{ _field_terminator }
};

static struct tag_block_definition plane_block = /* fake name */
{
	"plane",
	0,
	MAXIMUM_PLANES_PER_BSP3D,
	sizeof(real_plane3d),
	NULL,
	plane_fields
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

static struct tag_field collision_leaf_fields[] = /* fake name */
{
	{ _field_word_flags, "flags*", &collision_leaf_flags },
	{ _field_short_integer, "bsp2d reference count*" },
	{ _field_long_integer, "first bsp2d reference*" },
	{ _field_terminator }
};

static struct tag_block_definition collision_leaf_block = /* fake name */
{
	"leaf",
	0,
	MAXIMUM_LEAVES_PER_BSP3D,
	sizeof(struct collision_leaf),
	NULL,
	collision_leaf_fields
};

static struct tag_field bsp2d_reference_fields[] = /* fake name */
{
	{ _field_long_integer, "plane*", &plane_block },
	{ _field_long_integer, "bsp2d node*", &bsp2d_node_block },
	{ _field_terminator }
};

static struct tag_block_definition bsp2d_reference_block = /* fake name */
{
	"bsp2d reference",
	0,
	MAXIMUM_BSP2D_REFERENCES_PER_COLLISION_BSP,
	sizeof(struct bsp2d_reference),
	NULL,
	bsp2d_reference_fields
};

static struct tag_field bsp2d_node_fields[] = /* fake name */
{
	{ _field_real_plane2d, "plane*" },
	{ _field_long_integer, "left child*" },
	{ _field_long_integer, "right child*" },
	{ _field_terminator }
};

static struct tag_block_definition bsp2d_node_block = /* fake name */
{
	"bsp2d node",
	0,
	UNSIGNED_SHORT_MAX,
	sizeof(struct bsp2d_node),
	NULL,
	bsp2d_node_fields
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

static struct tag_field collision_surface_fields[] = /* fake name */
{
	{ _field_long_integer, "plane*" },
	{ _field_long_integer, "first edge*" },
	{ _field_byte_flags, "flags*", &collision_surface_flags },
	{ _field_char_integer, "breakable surface*" },
	{ _field_short_integer, "material*" },
	{ _field_terminator }
};

static struct tag_block_definition collision_surface_block = /* fake name */
{
	"surface",
	0,
	MAXIMUM_SURFACES_PER_COLLISION_BSP,
	sizeof(struct collision_surface),
	NULL,
	collision_surface_fields
};

static struct tag_field collision_edge_fields[] = /* fake name */
{
	{ _field_long_integer, "start vertex*" },
	{ _field_long_integer, "end vertex*" },
	{ _field_long_integer, "forward edge*" },
	{ _field_long_integer, "reverse edge*" },
	{ _field_long_integer, "left surface*" },
	{ _field_long_integer, "right surface*" },
	{ _field_terminator }
};

static struct tag_block_definition collision_edge_block = /* fake name */
{
	"edge",
	0,
	MAXIMUM_EDGES_PER_COLLISION_BSP,
	sizeof(struct collision_edge),
	NULL,
	collision_edge_fields
};

static struct tag_field collision_vertex_fields[] = /* fake name */
{
	{ _field_real_point3d, "point*" },
	{ _field_long_integer, "first edge*" },
	{ _field_terminator }
};

static struct tag_block_definition collision_vertex_block = /* fake name */
{
	"vertex",
	0,
	MAXIMUM_VERTICES_PER_COLLISION_BSP,
	sizeof(struct collision_vertex),
	NULL,
	collision_vertex_fields
};

/* ---------- public code */

/* ---------- private code */
