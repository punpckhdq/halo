/*
WIDGETS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "widgets.h"
#include "antenna.h"
#include "flags.h"
#include "glow.h"
#include "light_volumes.h"
#include "lightning.h"
#include "aleph.h"
#include "cellular_automata.h"
#include "cellular_automata2d.h"
#include "widget_types.h"
#include "objects.h"
#include "collision_bsp.h"
#include "game_state.h"

/* ---------- constants */

enum
{
	MAXIMUM_WIDGETS_PER_MAP = 64,
};

/* ---------- macros */

#define widget_get(index) DATUM_GET(widget_data, index, struct widget_datum) /* fake name */
#define widget_try_and_get(index) ((struct widget_datum *)datum_try_and_get(widget_data, (index))) /* fake name */

/* ---------- structures */

struct widget_datum
{
	short identifier;
	short type;
	long type_datum_index;
	long next_widget_index;
};

/* ---------- prototypes */

/* ---------- globals */

struct data_array *widget_data;

/* ---------- public code */

void widgets_initialize(
	void)
{
	short type;
	struct widget_type_definition *type_definition;

	widget_data = game_state_data_new("widget", MAXIMUM_WIDGETS_PER_MAP, sizeof(struct widget_datum));
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\widgets.c", 46, widget_data);

	for (type = 0; type < NUMBER_OF_WIDGET_TYPES; type++)
	{
		type_definition = widget_type_definition_get(type);
		match_assert("c:\\halo\\SOURCE\\objects\\widgets\\widgets.c", 55, type_definition->group_tag);

		if (type_definition->initialize)
		{
			type_definition->initialize();
		}
	}

	return;
}

void widgets_initialize_for_new_map(
	void)
{
	short type;
	struct widget_type_definition *type_definition;

	data_make_valid(widget_data);

	for (type = 0; type < NUMBER_OF_WIDGET_TYPES; type++)
	{
		type_definition = widget_type_definition_get(type);

		if (type_definition->initialize_for_new_map)
		{
			type_definition->initialize_for_new_map();
		}
	}

	return;
}

void widgets_dispose_from_old_map(
	void)
{
	short type;
	struct widget_type_definition *type_definition;

	for (type = 0; type < NUMBER_OF_WIDGET_TYPES; type++)
	{
		type_definition = widget_type_definition_get(type);

		if (type_definition->dispose_from_old_map)
		{
			type_definition->dispose_from_old_map();
		}
	}

	data_make_invalid(widget_data);

	return;
}

void widgets_dispose(
	void)
{
	short type;
	struct widget_type_definition *type_definition;

	for (type = 0; type < NUMBER_OF_WIDGET_TYPES; type++)
	{
		type_definition = widget_type_definition_get(type);

		if (type_definition->dispose)
		{
			type_definition->dispose();
		}
	}

	return;
}

void widgets_new(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *definition = object_definition_get(object->definition_index);
	short widget_reference_index;
	struct object_definition_widget *widget_reference;
	short type;
	struct widget_type_definition *type_definition;
	long widget_index;
	struct widget_datum *widget;

	object->object.first_widget_index = NONE;

	for (widget_reference_index = 0;
		widget_reference_index < definition->object.widgets.count;
		widget_reference_index++)
	{
		widget_reference = TAG_BLOCK_GET_ELEMENT(
			&definition->object.widgets,
			widget_reference_index,
			struct object_definition_widget);
		type = tag_group_to_widget_type(widget_reference->type.group_tag);

		if (type != NONE && widget_reference->type.index != NONE)
		{
			type_definition = widget_type_definition_get(type);
			widget_index = datum_new(widget_data);

			if (widget_index != NONE)
			{
				widget = widget_get(widget_index);
				widget->type = type;

				if (type_definition->new_proc)
				{
					widget->type_datum_index = type_definition->new_proc(widget_reference->type.index);

					if (widget->type_datum_index != NONE)
					{
						widget->next_widget_index = object->object.first_widget_index;
						object->object.first_widget_index = widget_index;
					}
					else
					{
						datum_delete(widget_data, widget_index);
					}
				}
				else
				{
					widget->next_widget_index = object->object.first_widget_index;
					object->object.first_widget_index = widget_index;
					widget->type_datum_index = NONE;
				}
			}
		}
	}

	return;
}

void widgets_delete(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	long widget_index;
	struct widget_datum *widget;
	struct widget_type_definition *type_definition;
	long next_widget_index;

	for (widget_index = object->object.first_widget_index;
		widget_index != NONE;
		widget_index = next_widget_index)
	{
		widget = widget_get(widget_index);
		type_definition = widget_type_definition_get(widget->type);
		next_widget_index = widget->next_widget_index;

		if (widget->type_datum_index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\objects\\widgets\\widgets.c", 190, type_definition->delete_proc);
			type_definition->delete_proc(widget->type_datum_index);
		}

		datum_delete(widget_data, widget_index);
	}

	object->object.first_widget_index = NONE;

	return;
}

boolean widgets_need_lighting(
	long widget_index)
{
	boolean result = FALSE;

	while (widget_index != NONE)
	{
		struct widget_datum *widget = widget_get(widget_index);
		struct widget_type_definition *type_definition = widget_type_definition_get(widget->type);

		if (type_definition->needs_lighting)
		{
			result = TRUE;
			break;
		}

		widget_index = widget->next_widget_index;
	}

	return result;
}

void widgets_render(
	long object_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	struct object_datum *object = object_get(object_index);
	long widget_index;
	struct widget_datum *widget;
	struct widget_type_definition *type_definition;

	for (widget_index = object->object.first_widget_index;
		widget_index != NONE;
		widget_index = widget->next_widget_index)
	{
		widget = widget_get(widget_index);
		type_definition = widget_type_definition_get(widget->type);

		if (type_definition->render_proc)
		{
			match_assert("c:\\halo\\SOURCE\\objects\\widgets\\widgets.c", 241, !type_definition->needs_lighting || lighting);
			type_definition->render_proc(object_index, widget->type_datum_index, lighting, animation);
		}
	}

	return;
}

void widgets_update(
	real dt)
{
	short type;
	struct widget_type_definition *type_definition;

	for (type = 0; type < NUMBER_OF_WIDGET_TYPES; type++)
	{
		type_definition = widget_type_definition_get(type);

		if (type_definition->update)
		{
			type_definition->update(dt);
		}
	}

	return;
}

/* ---------- private code */
