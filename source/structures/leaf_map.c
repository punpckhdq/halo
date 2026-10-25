/*
LEAF_MAP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "leaf_map.h"
#include "bsp3d.h"
#include "render_debug.h"

/* ---------- constants */

enum
{
	MAXIMUM_NODE_STACK_COUNT = 256,
	MAXIMUM_VERTICES_PER_LEAF_FACE = 64, /* fake name */
	MAXIMUM_FACES_PER_MAP_LEAF = 256,
	MAXIMUM_PORTALS_PER_MAP_LEAF = 256,
	MAXIMUM_MAP_LEAVES = 0x10000, /* fake name */
	MAXIMUM_VERTICES_PER_LEAF_PORTAL = 64,
	MAXIMUM_PORTALS_PER_LEAF_MAP = 0x80000,
};

enum
{
	_plane_intersection_none = 0, /* fake name */
	_plane_intersection_line, /* fake name */
	_plane_intersection_all, /* fake name */
};

#define LEAF_FACE_CLIP_EPSILON (1.0f / 4096.0f)				/* fake name */
#define LEAF_PORTAL_INTERSECTION_EPSILON (1.0f / 2048.0f)	/* fake name */
#define MINIMUM_LEAF_PORTAL_AREA 0.0025f					/* fake name */
#define MINIMUM_LEAF_PORTAL_COMPACTNESS 0.01f				/* fake name */

/* ---------- macros */

#define index_from_node(node) ((node)&LONG_MAX)

/* ---------- structures */

struct map_leaf_face
{
	long node_index;
	struct tag_block vertices;		// real_point2d
};

struct map_leaf
{
	struct tag_block faces;					// map_leaf_face
	struct tag_block portal_designators;	// long
};

struct temporary_face
{
	short vertex_count;
	real_point2d vertices[MAXIMUM_VERTICES_PER_LEAF_FACE];
};

/* ---------- prototypes */

static void node_stack_push(long node);
static long node_stack_pop(void);
static long node_stack_read(short levels_up);
static void leaf_map_build_leaf_faces(struct leaf_map *leaf_map, long node_index);
static void leaf_map_build_leaf_faces_for_leaf(struct leaf_map *leaf_map, long leaf_index);
static void leaf_map_build_leaf_face_for_leaf_on_node(struct leaf_map *leaf_map, long leaf_index, long node);
static short intersect_planes3d(real_plane3d const *plane, real_plane3d const *reference_plane, real_plane2d *result);
static void leaf_map_build_portals(struct leaf_map *leaf_map, long node_index);
static void leaf_map_build_portals_from_leaf(struct leaf_map *leaf_map, long ancestor_node_index, long leaf_index, long node_index, short stack_depth);
static boolean find_like_crossing(struct leaf_map const *leaf_map, long plane_index, boolean *side);
static short map_leaf_find_face_on_node(struct leaf_map const *leaf_map, struct map_leaf const *leaf, long node_index);
static void leaf_map_build_portal_from_leaves(struct leaf_map *leaf_map, long node_index, long leaf_index0, long leaf_index1);
static void leaf_map_family_mark(struct leaf_map const *leaf_map, long *marked_leaves, long leaf_index);
static void leaf_face_get_vertex3d(struct leaf_map const *map, struct map_leaf_face const *face, short vertex_index, real_point3d *result);
static real normalize_three_dee(real_vector3d *vector);

/* ---------- globals */

static struct profile_section leaf_map_initialize_section = {"leaf_map_initialize", NONE, TRUE}; /* fake name */

static struct temporary_face default_face =
{
	4,
	{
		{-1536.f, -1536.f},
		{1536.f, -1536.f},
		{1536.f, 1536.f},
		{-1536.f, 1536.f}
	}
};

TAG_BLOCK(map_leaf_face_vertex_block, MAXIMUM_VERTICES_PER_LEAF_FACE, sizeof(real_point2d), NULL, NULL, NULL, NULL)
{
	{_field_real_point2d, "vertex*"},
	{_field_terminator}
};

TAG_BLOCK(map_leaf_portal_index_block, MAXIMUM_PORTALS_PER_MAP_LEAF, sizeof(long), NULL, NULL, NULL, NULL)
{
	{_field_long_integer, "portal index*"},
	{_field_terminator}
};

TAG_BLOCK(map_leaf_face_block, MAXIMUM_FACES_PER_MAP_LEAF, sizeof(struct map_leaf_face), NULL, NULL, NULL, NULL)
{
	{_field_long_integer, "node index*"},
	{_field_block, "vertices*", &map_leaf_face_vertex_block},
	{_field_terminator}
};

TAG_BLOCK(global_map_leaf_block, MAXIMUM_MAP_LEAVES, sizeof(struct map_leaf), NULL, NULL, NULL, NULL)
{
	{_field_block, "faces*", &map_leaf_face_block},
	{_field_block, "portal indices*", &map_leaf_portal_index_block},
	{_field_terminator}
};

TAG_BLOCK(leaf_portal_vertex_block, MAXIMUM_VERTICES_PER_LEAF_PORTAL, sizeof(real_point3d), NULL, NULL, NULL, NULL)
{
	{_field_real_point3d, "point*"},
	{_field_terminator}
};

TAG_BLOCK(global_leaf_portal_block, MAXIMUM_PORTALS_PER_LEAF_MAP, sizeof(struct leaf_portal), NULL, NULL, NULL, NULL)
{
	{_field_long_integer, "plane index*"},
	{_field_long_integer, "back leaf index*"},
	{_field_long_integer, "front leaf index*"},
	{_field_block, "vertices*", &leaf_portal_vertex_block},
	{_field_terminator}
};

static struct
{
	long node_stack[MAXIMUM_NODE_STACK_COUNT];
	short node_stack_count;
	char const *error;
} leaf_map_globals;

static short portal_count = 0;

/* ---------- public code */

static void node_stack_push(
	long node)
{
	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 42, leaf_map_globals.node_stack_count<MAXIMUM_NODE_STACK_COUNT);

	leaf_map_globals.node_stack[leaf_map_globals.node_stack_count++] = node;

	return;
}

static long node_stack_pop(
	void)
{
	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 51, leaf_map_globals.node_stack_count>0);

	return leaf_map_globals.node_stack[--leaf_map_globals.node_stack_count];
}

static long node_stack_read(
	short levels_up)
{
	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 59, levels_up>=0 && levels_up<leaf_map_globals.node_stack_count);

	return leaf_map_globals.node_stack[leaf_map_globals.node_stack_count - levels_up - 1];
}

boolean leaf_map_initialize_from_bsp(
	struct leaf_map *leaf_map,
	struct bsp3d const *bsp,
	long leaf_count)
{
	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 86, leaf_map);
	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 87, bsp);
	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 88, leaf_map_globals.node_stack_count==0);

	leaf_map_globals.error = NULL;
	profile_enter(leaf_map_initialize_section);
	leaf_map->bsp = bsp;

	if (tag_block_resize(&leaf_map->leaves, leaf_count))
	{
		if (bsp->nodes.count > 0)
		{
			leaf_map_build_leaf_faces(leaf_map, 0);
			leaf_map_build_portals(leaf_map, 0);
		}
	}
	else if (!leaf_map_globals.error)
	{
		leaf_map_globals.error = "couldn't allocate leaf_map leaves.";
	}

	if (leaf_map_globals.error)
	{
		error(_error_delayed, leaf_map_globals.error);
	}

	profile_exit(leaf_map_initialize_section);

	return !leaf_map_globals.error;
}

void leaf_map_delete(
	struct leaf_map *leaf_map)
{
	tag_block_resize(&leaf_map->leaves, 0);
	tag_block_resize(&leaf_map->portals, 0);

	return;
}

static void leaf_map_build_leaf_faces(
	struct leaf_map *leaf_map,
	long node_index)
{
	short child_index;
	struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, node_index, struct bsp3d_node);

	for (child_index = 0; child_index < 2; child_index++)
	{
		node_stack_push(child_index == 0 ? node_index | LONG_MIN : node_index);

		if (node->child_indices[child_index] & LONG_MIN)
		{
			if (node->child_indices[child_index] != NONE)
			{
				leaf_map_build_leaf_faces_for_leaf(leaf_map, node->child_indices[child_index]);
			}
		}
		else
		{
			leaf_map_build_leaf_faces(leaf_map, node->child_indices[child_index]);
		}

		node_stack_pop();
	}

	return;
}

static void leaf_map_build_leaf_faces_for_leaf(
	struct leaf_map *leaf_map,
	long leaf_index)
{
	short levels_up;

	for (levels_up = 0; levels_up < leaf_map_globals.node_stack_count; levels_up++)
	{
		leaf_map_build_leaf_face_for_leaf_on_node(leaf_map, leaf_index, node_stack_read(levels_up));
	}

	return;
}

static void leaf_map_build_leaf_face_for_leaf_on_node(
	struct leaf_map *leaf_map,
	long leaf_index,
	long node)
{
	struct temporary_face result;
	short levels_up;
	real_plane3d *reference_plane = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->planes, TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, index_from_node(node), struct bsp3d_node)->plane, real_plane3d);

	result = default_face;

	for (levels_up = 0; levels_up < leaf_map_globals.node_stack_count && result.vertex_count; levels_up++)
	{
		long traversal_node = node_stack_read(levels_up);

		if (traversal_node != node)
		{
			real_plane2d halfspace;
			short intersection;
			real_plane3d other_plane = *TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->planes, TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, index_from_node(traversal_node), struct bsp3d_node)->plane, real_plane3d);

			if (traversal_node & LONG_MIN)
			{
				plane3d_negate(&other_plane, &other_plane);
			}

			intersection = intersect_planes3d(&other_plane, reference_plane, &halfspace);

			if (intersection == _plane_intersection_line)
			{
				result.vertex_count = convex_polygon2d_clip_to_plane(
					result.vertex_count,
					result.vertices,
					&halfspace,
					MAXIMUM_VERTICES_PER_LEAF_FACE,
					result.vertices,
					NULL,
					NULL,
					LEAF_FACE_CLIP_EPSILON);
				match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 227, result.vertex_count!=NONE);
			}
			else if (intersection == _plane_intersection_none)
			{
				result.vertex_count = 0;
			}
		}
	}

	if (result.vertex_count)
	{
		struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index), struct map_leaf);
		short face_index = (short)tag_block_add_element(&leaf->faces);

		if (face_index != NONE)
		{
			struct map_leaf_face *face = TAG_BLOCK_GET_ELEMENT(&leaf->faces, face_index, struct map_leaf_face);

			face->node_index = index_from_node(node);

			if (tag_block_resize(&face->vertices, result.vertex_count))
			{
				memcpy(face->vertices.address, result.vertices, result.vertex_count * sizeof(real_point2d));
			}
			else if (!leaf_map_globals.error)
			{
				leaf_map_globals.error = "couldn't allocate leaf vertices.";
			}
		}
		else if (!leaf_map_globals.error)
		{
			leaf_map_globals.error = "couldn't allocate leaf face.";
		}
	}

	return;
}

static short intersect_planes3d(
	real_plane3d const *plane,
	real_plane3d const *reference_plane,
	real_plane2d *result)
{
	real_plane3d perpendicular_intersection;
	real length;
	short projection = projection_from_vector3d(&reference_plane->n);

	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 299, reference_plane->n.n[projection]!=0.f);
	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 300, result);

	if (plane->n.n[projection] == 0.f)
	{
		perpendicular_intersection = *plane;
	}
	else
	{
		real scale = plane->n.n[projection] / reference_plane->n.n[projection];

		perpendicular_intersection.n.i = plane->n.i - scale * reference_plane->n.i;
		perpendicular_intersection.n.j = plane->n.j - scale * reference_plane->n.j;
		perpendicular_intersection.n.k = plane->n.k - scale * reference_plane->n.k;
		perpendicular_intersection.d = plane->d - scale * reference_plane->d;
	}

	length = normalize_three_dee(&perpendicular_intersection.n);

	if (length != 0.f)
	{
		project_point3d(
			(real_point3d const *)&perpendicular_intersection.n,
			projection,
			projection_sign_from_vector3d(&reference_plane->n, projection),
			(real_point2d *)&result->n);
		result->d = perpendicular_intersection.d / length;

		return _plane_intersection_line;
	}
	else
	{
		if (perpendicular_intersection.d > 0.f)
		{
			return _plane_intersection_none;
		}
		else
		{
			return _plane_intersection_all;
		}
	}
}

static void leaf_map_build_portals(
	struct leaf_map *leaf_map,
	long node_index)
{
	short child_index;
	struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, node_index, struct bsp3d_node);

	for (child_index = 0; child_index < 2; child_index++)
	{
		node_stack_push(child_index == 0 ? node_index | LONG_MIN : node_index);

		if (node->child_indices[child_index] & LONG_MIN)
		{
			if (node->child_indices[child_index] != NONE)
			{
				leaf_map_build_portals_from_leaf(
					leaf_map,
					NONE,
					index_from_node(node->child_indices[child_index]),
					0,
					(short)(leaf_map_globals.node_stack_count - 1));
			}
		}
		else
		{
			leaf_map_build_portals(leaf_map, node->child_indices[child_index]);
		}

		node_stack_pop();
	}

	return;
}

static void leaf_map_build_portals_from_leaf(
	struct leaf_map *leaf_map,
	long ancestor_node_index,
	long leaf_index,
	long node_index,
	short levels_up)
{
	short child_index;
	boolean side;
	struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, node_index, struct bsp3d_node);
	long first_traversal_node = ancestor_node_index == NONE ? node_stack_read(levels_up) : NONE;
	boolean plane_on_stack = find_like_crossing(leaf_map, node->plane, &side);

	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 415, ancestor_node_index!=NONE || index_from_node(first_traversal_node)==node_index);

	for (child_index = 0; child_index < 2; child_index++)
	{
		boolean descend = ancestor_node_index == NONE && child_index && (first_traversal_node & LONG_MIN);

		if (ancestor_node_index != NONE || child_index || (first_traversal_node & LONG_MIN))
		{
			if (!descend || map_leaf_find_face_on_node(leaf_map, TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index), struct map_leaf), node_index) != NONE)
			{
				if (ancestor_node_index == NONE || !plane_on_stack || side != child_index)
				{
					if (node->child_indices[child_index] & LONG_MIN)
					{
						if (node->child_indices[child_index] != NONE && index_from_node(node->child_indices[child_index]) != leaf_index)
						{
							leaf_map_build_portal_from_leaves(
								leaf_map,
								descend ? node_index : ancestor_node_index,
								leaf_index,
								node->child_indices[child_index]);
						}
					}
					else
					{
						leaf_map_build_portals_from_leaf(
							leaf_map,
							descend ? node_index : ancestor_node_index,
							leaf_index,
							node->child_indices[child_index],
							(short)(levels_up - 1));
					}
				}
			}
		}
	}

	return;
}

static boolean find_like_crossing(
	struct leaf_map const *leaf_map,
	long plane_index,
	boolean *side)
{
	short levels_up;
	boolean found = FALSE;

	for (levels_up = 0; levels_up < leaf_map_globals.node_stack_count; levels_up++)
	{
		long traversal_node = node_stack_read(levels_up);
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, index_from_node(traversal_node), struct bsp3d_node);

		if (node->plane == plane_index)
		{
			*side = (traversal_node & LONG_MIN) != 0;
			found = TRUE;
			break;
		}
	}

	return found;
}

static short map_leaf_find_face_on_node(
	struct leaf_map const *leaf_map,
	struct map_leaf const *leaf,
	long node_index)
{
	short face_index;

	for (face_index = 0; face_index < leaf->faces.count; face_index++)
	{
		if (TAG_BLOCK_GET_ELEMENT(&leaf->faces, face_index, struct map_leaf_face)->node_index == node_index)
		{
			return face_index;
		}
	}

	return NONE;
}

static void leaf_map_build_portal_from_leaves(
	struct leaf_map *leaf_map,
	long node_index,
	long leaf_index0,
	long leaf_index1)
{
	struct map_leaf *leaf0 = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index0), struct map_leaf);
	struct map_leaf *leaf1 = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index1), struct map_leaf);
	short face_index0 = map_leaf_find_face_on_node(leaf_map, leaf0, node_index);
	short face_index1 = map_leaf_find_face_on_node(leaf_map, leaf1, node_index);

	if (face_index0 != NONE && face_index1 != NONE)
	{
		short vertex_count;
		real_point2d vertices[MAXIMUM_VERTICES_PER_LEAF_PORTAL];
		struct map_leaf_face *face0 = TAG_BLOCK_GET_ELEMENT(&leaf0->faces, face_index0, struct map_leaf_face);
		struct map_leaf_face *face1 = TAG_BLOCK_GET_ELEMENT(&leaf1->faces, face_index1, struct map_leaf_face);

		match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 478, leaf_index0!=leaf_index1);

		vertex_count = convex_hull2d_intersect(
			face0->vertices.count,
			face0->vertices.address,
			face1->vertices.count,
			face1->vertices.address,
			MAXIMUM_VERTICES_PER_LEAF_PORTAL,
			vertices,
			LEAF_PORTAL_INTERSECTION_EPSILON);

		if (vertex_count > 0)
		{
			long portal_index = tag_block_add_element(&leaf_map->portals);
			short portal_designator_index0 = (short)tag_block_add_element(&leaf0->portal_designators);
			short portal_designator_index1 = (short)tag_block_add_element(&leaf1->portal_designators);

			match_vassert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 487, vertex_count!=NONE, "too many vertices in portal");

			if (portal_index != NONE && portal_designator_index0 != NONE && portal_designator_index1 != NONE)
			{
				short vertex_index;
				short projection;
				boolean sign;
				real area;
				struct leaf_portal *portal = TAG_BLOCK_GET_ELEMENT(&leaf_map->portals, portal_index, struct leaf_portal);
				real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->planes, TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, node_index, struct bsp3d_node)->plane, real_plane3d);

				portal->plane_index = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, node_index, struct bsp3d_node)->plane;
				projection = projection_from_vector3d(&plane->n);
				sign = projection_sign_from_vector3d(&plane->n, projection);
				portal->leaf_indices[0] = index_from_node(leaf_index0);
				portal->leaf_indices[1] = index_from_node(leaf_index1);
				*TAG_BLOCK_GET_ELEMENT(&leaf0->portal_designators, portal_designator_index0, long) = portal_index;
				*TAG_BLOCK_GET_ELEMENT(&leaf1->portal_designators, portal_designator_index1, long) = portal_index;

				if (tag_block_resize(&portal->vertices, vertex_count))
				{
					for (vertex_index = 0; vertex_index < vertex_count; vertex_index++)
					{
						project_point2d(
							&vertices[vertex_index],
							plane,
							projection,
							sign,
							TAG_BLOCK_GET_ELEMENT(&portal->vertices, vertex_index, real_point3d));
					}
				}

				area = convex_hull2d_area(vertex_count, vertices);

				if (area < MINIMUM_LEAF_PORTAL_AREA ||
					square_root(area) / convex_hull2d_perimeter(vertex_count, vertices) < MINIMUM_LEAF_PORTAL_COMPACTNESS)
				{
					leaf_map_close_portal(leaf_map, portal_index);
				}
			}
			else if (!leaf_map_globals.error)
			{
				if (portal_index == NONE)
				{
					leaf_map_globals.error = "couldn't allocate leaf map portal";
				}
				else
				{
					leaf_map_globals.error = "couldn't allocate leaf map portal designator.";
				}
			}
		}
		else if (vertex_count == NONE)
		{
			match_halt("c:\\halo\\SOURCE\\structures\\leaf_map.c", 541);
		}
	}

	return;
}

void leaf_map_close_portal(
	struct leaf_map *leaf_map,
	long portal_index)
{
	short side;
	struct leaf_portal *portal = TAG_BLOCK_GET_ELEMENT(&leaf_map->portals, portal_index, struct leaf_portal);

	for (side = 0; side < 2; side++)
	{
		short portal_designator_index;
		struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(portal->leaf_indices[side]), struct map_leaf);

		for (portal_designator_index = 0; portal_designator_index < leaf->portal_designators.count; portal_designator_index++)
		{
			long *portal_designator = TAG_BLOCK_GET_ELEMENT(&leaf->portal_designators, portal_designator_index, long);

			if (index_from_node(*portal_designator) == portal_index)
			{
				*portal_designator = index_from_node(*portal_designator) | LONG_MIN;
				break;
			}
		}

		match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 673, portal_designator_index!=leaf->portal_designators.count);
	}

	return;
}

boolean leaf_map_leaf_is_closed(
	struct leaf_map const *leaf_map,
	long leaf_index)
{
	short portal_designator_index;
	struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index), struct map_leaf);

	for (portal_designator_index = 0; portal_designator_index < leaf->portal_designators.count; portal_designator_index++)
	{
		long portal_designator = *TAG_BLOCK_GET_ELEMENT(&leaf->portal_designators, portal_designator_index, long);

		if (!(portal_designator & LONG_MIN))
		{
			return FALSE;
		}
	}

	return leaf->portal_designators.count;
}

void leaf_map_get_leaf_bounds(
	struct leaf_map const *leaf_map,
	long leaf_index,
	real_point3d *bounding_sphere_center,
	real *bounding_sphere_radius)
{
	real_point3d maximum;
	struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index), struct map_leaf);
	real_rectangle3d bounds = *global_null_rectangle3d;

	if (leaf->portal_designators.count)
	{
		short portal_designator_index;

		for (portal_designator_index = 0; portal_designator_index < leaf->portal_designators.count; portal_designator_index++)
		{
			short vertex_index;
			struct leaf_portal *portal = TAG_BLOCK_GET_ELEMENT(&leaf_map->portals, index_from_node(*TAG_BLOCK_GET_ELEMENT(&leaf->portal_designators, portal_designator_index, long)), struct leaf_portal);

			for (vertex_index = 0; vertex_index < portal->vertices.count; vertex_index++)
			{
				real_point3d *vertex = TAG_BLOCK_GET_ELEMENT(&portal->vertices, vertex_index, real_point3d);

				if (vertex->x < bounds.x0)
				{
					bounds.x0 = vertex->x;
				}

				if (vertex->x > bounds.x1)
				{
					bounds.x1 = vertex->x;
				}

				if (vertex->y < bounds.y0)
				{
					bounds.y0 = vertex->y;
				}

				if (vertex->y > bounds.y1)
				{
					bounds.y1 = vertex->y;
				}

				if (vertex->z < bounds.z0)
				{
					bounds.z0 = vertex->z;
				}

				if (vertex->z > bounds.z1)
				{
					bounds.z1 = vertex->z;
				}
			}
		}
	}
	else if (leaf->faces.count)
	{
		short face_index;

		for (face_index = 0; face_index < leaf->faces.count; face_index++)
		{
			short vertex_index;
			real_point3d point;
			struct map_leaf_face *face = TAG_BLOCK_GET_ELEMENT(&leaf->faces, face_index, struct map_leaf_face);
			real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->planes, TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, face->node_index, struct bsp3d_node)->plane, real_plane3d);
			short projection = projection_from_vector3d(&plane->n);
			boolean sign = projection_sign_from_vector3d(&plane->n, projection);

			for (vertex_index = 0; vertex_index < face->vertices.count; vertex_index++)
			{
				project_point2d(
					TAG_BLOCK_GET_ELEMENT(&face->vertices, vertex_index, real_point2d),
					plane,
					projection,
					sign,
					&point);

				if (point.x < bounds.x0)
				{
					bounds.x0 = point.x;
				}

				if (point.x > bounds.x1)
				{
					bounds.x1 = point.x;
				}

				if (point.y < bounds.y0)
				{
					bounds.y0 = point.y;
				}

				if (point.y > bounds.y1)
				{
					bounds.y1 = point.y;
				}

				if (point.z < bounds.z0)
				{
					bounds.z0 = point.z;
				}

				if (point.z > bounds.z1)
				{
					bounds.z1 = point.z;
				}
			}
		}
	}

	bounding_sphere_center->x = (bounds.x0 + bounds.x1) * 0.5f;
	bounding_sphere_center->y = (bounds.y0 + bounds.y1) * 0.5f;
	bounding_sphere_center->z = (bounds.z0 + bounds.z1) * 0.5f;
	maximum.x = bounds.x1;
	maximum.y = bounds.y1;
	maximum.z = bounds.z1;
	*bounding_sphere_radius = distance3d(bounding_sphere_center, &maximum);

	return;
}

boolean leaf_map_closure(
	struct leaf_map const *leaf_map,
	long *marked_leaves,
	long *result)
{
	long leaf_index;

	if (marked_leaves != result)
	{
		memcpy(result, marked_leaves, BIT_VECTOR_SIZE_IN_BYTES(leaf_map->leaves.count));
	}

	for (leaf_index = 0; leaf_index < leaf_map->leaves.count; leaf_index++)
	{
		if (BIT_VECTOR_TEST_FLAG(result, leaf_index))
		{
			leaf_map_family_mark(leaf_map, result, leaf_index);
		}
	}

	return TRUE;
}

static void leaf_map_family_mark(
	struct leaf_map const *leaf_map,
	long *marked_leaves,
	long leaf_index)
{
	short portal_designator_index;
	struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index), struct map_leaf);

	for (portal_designator_index = 0; portal_designator_index < leaf->portal_designators.count; portal_designator_index++)
	{
		struct leaf_portal *portal = TAG_BLOCK_GET_ELEMENT(&leaf_map->portals, index_from_node(*TAG_BLOCK_GET_ELEMENT(&leaf->portal_designators, portal_designator_index, long)), struct leaf_portal);
		long other_leaf_index = portal->leaf_indices[0] == leaf_index ? portal->leaf_indices[1] : portal->leaf_indices[0];

		if (!BIT_VECTOR_TEST_FLAG(marked_leaves, other_leaf_index))
		{
			BIT_VECTOR_SET_FLAG(marked_leaves, other_leaf_index, TRUE);
			leaf_map_family_mark(leaf_map, marked_leaves, other_leaf_index);
		}
	}

	return;
}

void render_debug_leaf_portal(
	struct leaf_map const *map,
	long portal_designator)
{
	short vertex_index;
	struct leaf_portal *portal = TAG_BLOCK_GET_ELEMENT(&map->portals, index_from_node(portal_designator), struct leaf_portal);
	boolean closed = (portal_designator & LONG_MIN) != 0;
	real_argb_color portal_colors[2] = {{0.1f, 0.f, 1.f, 0.f}, {0.1f, 1.f, 0.f, 0.f}};

	for (vertex_index = 2; vertex_index < portal->vertices.count; vertex_index++)
	{
		render_debug_triangle(
			TRUE,
			TAG_BLOCK_GET_ELEMENT(&portal->vertices, 0, real_point3d),
			TAG_BLOCK_GET_ELEMENT(&portal->vertices, vertex_index - 1, real_point3d),
			TAG_BLOCK_GET_ELEMENT(&portal->vertices, vertex_index, real_point3d),
			&portal_colors[closed]);
		render_debug_line(
			TRUE,
			TAG_BLOCK_GET_ELEMENT(&portal->vertices, vertex_index - 1, real_point3d),
			TAG_BLOCK_GET_ELEMENT(&portal->vertices, vertex_index, real_point3d),
			global_real_argb_red);
	}

	render_debug_line(
		TRUE,
		TAG_BLOCK_GET_ELEMENT(&portal->vertices, 0, real_point3d),
		TAG_BLOCK_GET_ELEMENT(&portal->vertices, 1, real_point3d),
		global_real_argb_red);
	render_debug_line(
		TRUE,
		TAG_BLOCK_GET_ELEMENT(&portal->vertices, 0, real_point3d),
		TAG_BLOCK_GET_ELEMENT(&portal->vertices, portal->vertices.count - 1, real_point3d),
		global_real_argb_red);

	return;
}

void render_debug_leaf_portals(
	struct leaf_map const *map,
	long leaf_index)
{
	short portal_designator_index;
	struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&map->leaves, index_from_node(leaf_index), struct map_leaf);

	for (portal_designator_index = 0; portal_designator_index < leaf->portal_designators.count; portal_designator_index++)
	{
		long portal_designator = *TAG_BLOCK_GET_ELEMENT(&leaf->portal_designators, portal_designator_index, long);

		render_debug_leaf_portal(map, portal_designator);
	}

	return;
}

boolean leaf_map_leaf_spans_polygon(
	struct leaf_map const *leaf_map,
	long leaf_index,
	real_plane3d const *plane,
	short projection,
	boolean projection_sign,
	short vertex_count,
	real_point2d const *vertices2d)
{
	short face_index;
	struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&leaf_map->leaves, index_from_node(leaf_index), struct map_leaf);

	for (face_index = 0; face_index < leaf->faces.count; face_index++)
	{
		short vertex_index;
		struct map_leaf_face *face = TAG_BLOCK_GET_ELEMENT(&leaf->faces, face_index, struct map_leaf_face);
		real_plane3d *face_plane = TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->planes, TAG_BLOCK_GET_ELEMENT(&leaf_map->bsp->nodes, face->node_index, struct bsp3d_node)->plane, real_plane3d);
		short face_projection = projection_from_vector3d(&face_plane->n);
		boolean face_sign = projection_sign_from_vector3d(&face_plane->n, face_projection);

		for (vertex_index = 0; vertex_index < face->vertices.count; vertex_index++)
		{
			real_point3d vertex0;
			real_point3d vertex1;
			real distance0;
			real distance1;

			project_point2d(
				TAG_BLOCK_GET_ELEMENT(&face->vertices, vertex_index, real_point2d),
				face_plane,
				face_projection,
				face_sign,
				&vertex0);
			project_point2d(
				TAG_BLOCK_GET_ELEMENT(&face->vertices, vertex_index + 1 == face->vertices.count ? 0 : vertex_index + 1, real_point2d),
				face_plane,
				face_projection,
				face_sign,
				&vertex1);
			distance0 = plane3d_distance_to_point(plane, &vertex0);
			distance1 = plane3d_distance_to_point(plane, &vertex1);

			if ((distance0 < -0.03f && distance1 > 0.03f) || (distance0 > 0.03f && distance1 < -0.03f))
			{
				real_vector3d edge;
				real t;
				real_point3d intersection_point;
				real_point2d intersection_point2d;

				vector_from_points3d(&vertex0, &vertex1, &edge);
				t = vector_intersect_plane3d(&vertex0, &edge, plane);
				point_from_line3d(&vertex0, &edge, t, &intersection_point);
				project_point3d(&intersection_point, projection, projection_sign, &intersection_point2d);

				if (convex_hull2d_test_point(vertex_count, vertices2d, &intersection_point2d, 0.05f))
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

static void leaf_face_get_vertex3d(
	struct leaf_map const *map,
	struct map_leaf_face const *face,
	short vertex_index,
	real_point3d *result)
{
	real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&map->bsp->planes, TAG_BLOCK_GET_ELEMENT(&map->bsp->nodes, face->node_index, struct bsp3d_node)->plane, real_plane3d);
	short projection = projection_from_vector3d(&plane->n);
	boolean sign = projection_sign_from_vector3d(&plane->n, projection);

	project_point2d(
		TAG_BLOCK_GET_ELEMENT(&face->vertices, vertex_index, real_point2d),
		plane,
		projection,
		sign,
		result);

	return;
}

void render_debug_leaf_faces(
	struct leaf_map const *map,
	long leaf_index)
{
	short face_index;
	real_point3d p0;
	real_point3d p1;
	real_point3d last_p;
	real_point3d p;
	struct map_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&map->leaves, index_from_node(leaf_index), struct map_leaf);
	real_argb_color face_color = {0.2f, 1.f, 0.f, 0.f};

	match_assert("c:\\halo\\SOURCE\\structures\\leaf_map.c", 940, map->bsp);

	for (face_index = 0; face_index < leaf->faces.count; face_index++)
	{
		short vertex_index;
		struct map_leaf_face *face = TAG_BLOCK_GET_ELEMENT(&leaf->faces, face_index, struct map_leaf_face);

		leaf_face_get_vertex3d(map, face, 0, &p0);
		leaf_face_get_vertex3d(map, face, 1, &p1);
		last_p = p1;
		render_debug_line(TRUE, &p0, &p1, global_real_argb_white);

		for (vertex_index = 2; vertex_index < face->vertices.count; vertex_index++)
		{
			leaf_face_get_vertex3d(map, face, vertex_index, &p);
			render_debug_triangle(TRUE, &p0, &last_p, &p, &face_color);
			render_debug_line(TRUE, &last_p, &p, global_real_argb_white);
			last_p = p;
		}
	}

	return;
}

static real normalize_three_dee(
	real_vector3d *vector)
{
	real length = magnitude3d(vector);
	real inverse_length = 1.f / length;

	if (valid_real(inverse_length))
	{
		scale_vector3d(vector, inverse_length, vector);

		if (valid_real_normal3d(vector))
		{
			return length;
		}
	}

	return 0.f;
}
