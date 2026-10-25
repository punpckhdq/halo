/*
LEAF_MAP.H

header included in hcex build.
*/

#ifndef __LEAF_MAP_H
#define __LEAF_MAP_H
#pragma once

/* ---------- headers */


/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct leaf_map
{
	const struct bsp3d *bsp;
	struct tag_block leaves;	// map_leaf
	struct tag_block portals;	// leaf_portal
};

struct leaf_portal
{
	long plane_index;
	long leaf_indices[2];
	struct tag_block vertices;	// real_point3d
};

/* ---------- prototypes/LEAF_MAP.C */

boolean leaf_map_initialize_from_bsp(struct leaf_map *leaf_map, struct bsp3d const *bsp, long leaf_count);
void leaf_map_delete(struct leaf_map *leaf_map);
void leaf_map_close_portal(struct leaf_map *leaf_map, long portal_index);
boolean leaf_map_leaf_is_closed(struct leaf_map const *leaf_map, long leaf_index);
void leaf_map_get_leaf_bounds(struct leaf_map const *leaf_map, long leaf_index, real_point3d *bounding_sphere_center, real *bounding_sphere_radius);
boolean leaf_map_closure(struct leaf_map const *leaf_map, long *marked_leaves, long *result);
void render_debug_leaf_portal(struct leaf_map const *map, long portal_designator);
void render_debug_leaf_portals(struct leaf_map const *map, long leaf_index);
boolean leaf_map_leaf_spans_polygon(struct leaf_map const *leaf_map, long leaf_index, real_plane3d const *plane, short projection, boolean projection_sign, short vertex_count, real_point2d const *vertices2d);
void render_debug_leaf_faces(struct leaf_map const *map, long leaf_index);

/* ---------- globals */

/* ---------- public code */

#endif // __LEAF_MAP_H
