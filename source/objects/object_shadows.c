/*
OBJECT_SHADOWS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "objects.h"
#include "object_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct object_shadow /* fake name */
{
	real bounding_radius;
	real_rectangle3d bounds;
	short count;
	short field_1E;
};

/* ---------- prototypes */

static void object_shadow_add_object(long object_index, void const *context, struct object_shadow *shadow); /* fake name */
static void object_build_shadow_recursive(long object_index, void const *context, struct object_shadow *shadow); /* fake name */

/* ---------- globals */

/* ---------- public code */

boolean object_build_shadow(
	long object_index,
	void const *context,
	struct object_shadow *shadow)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *definition = object_definition_get(object->definition_index);
	boolean result = FALSE;

	shadow->bounding_radius = definition->object.bounding_radius;
	shadow->bounds.x0 = REAL_MAX;
	shadow->bounds.x1 = -REAL_MAX;
	shadow->bounds.y0 = REAL_MAX;
	shadow->bounds.y1 = -REAL_MAX;
	shadow->bounds.z0 = REAL_MAX;
	shadow->bounds.z1 = -REAL_MAX;
	shadow->count = 0;
	shadow->field_1E = 0;
	object_shadow_add_object(object_index, context, shadow);
	object_build_shadow_recursive(object->object.first_child_object_index, context, shadow);

	if (shadow->count > 0)
	{
		result = TRUE;
	}

	return result;
}

/* ---------- private code */

static void object_shadow_add_object(
	long object_index,
	void const *context,
	struct object_shadow *shadow)
{
	object_get(object_index);

	return;
}

static void object_build_shadow_recursive(
	long object_index,
	void const *context,
	struct object_shadow *shadow)
{
	while (object_index != NONE)
	{
		struct object_datum *object = object_get(object_index);

		object_shadow_add_object(object_index, context, shadow);
		object_build_shadow_recursive(object->object.first_child_object_index, context, shadow);
		object_index = object->object.next_object_index;
	}

	return;
}
