/*
SCENERY.C

*/

/* ---------- headers */

#include "cseries.h"
#include "scenery.h"
#include "console.h"
#include "editor_stubs.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void scenery_animation_start_private(
	long scenery_index,
	long animation_graph_index,
	char const *animation_name,
	short frame_index);

/* ---------- globals */

/* ---------- public code */

void scenery_initialize(
	void)
{
	return;
}

void scenery_initialize_for_new_map(
	void)
{
	return;
}

void scenery_dispose_from_old_map(
	void)
{
	return;
}

void scenery_dispose(
	void)
{
	return;
}

void scenery_place(
	long scenery_index,
	struct scenario_scenery_datum *scenario_scenery)
{
	object_add_scenario_permutation(scenery_index, &scenario_scenery->permutation);

	return;
}

boolean scenery_new(
	long object_index)
{
	struct scenery_datum *scenery = scenery_get(object_index);
	struct object_definition *definition = object_definition_get(scenery->definition_index);

	if (definition->object.animation_graph.index != NONE &&
		animation_graph_definition_get(definition->object.animation_graph.index)->animations.count > 0)
	{
		short animation_index = animation_choose_random_permutation(definition->object.animation_graph.index, 0);

		if (animation_index != NONE)
		{
			scenery->object.animation.state.index = animation_index;
			scenery->object.animation.animation_graph_index = definition->object.animation_graph.index;
			SET_FLAG(scenery->object.flags, _object_animates_automatically_bit, TRUE);
		}
	}
	SET_FLAG(scenery->object.flags, _object_shadowless_bit, TRUE);

	return TRUE;
}

void scenery_delete(
	long scenery_index)
{
	return;
}

boolean scenery_update(
	long scenery_index)
{
	struct scenery_datum *scenery = scenery_get(scenery_index);

	if (TEST_FLAG(scenery->scenery.flags, _scenery_self_animated_bit))
	{
		switch (animation_update(scenery->object.animation.animation_graph_index, &scenery->object.animation.state, NULL))
		{
		case _animation_will_restart_on_next_frame:
			scenery->object.animation.state.frame_index--;
			break;
		}
	}

	return TRUE;
}

short scenery_get_animation_time(
	long scenery_index)
{
	struct scenery_datum *scenery = scenery_get(scenery_index);

	if (TEST_FLAG(scenery->scenery.flags, _scenery_self_animated_bit))
	{
		struct animation_graph *graph = animation_graph_definition_get(scenery->object.animation.animation_graph_index);
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&graph->animations, scenery->object.animation.state.index, struct animation);

		return MAX(animation->frame_count - scenery->object.animation.state.frame_index - 2, 0);
	}

	return 0;
}

void scenery_animation_start(
	long scenery_index,
	long animation_graph_index,
	char const *animation_name)
{
	scenery_animation_start_private(scenery_index, animation_graph_index, animation_name, 0);

	return;
}

void scenery_animation_start_at_frame(
	long scenery_index,
	long animation_graph_index,
	char const *animation_name,
	short frame_index)
{
	scenery_animation_start_private(scenery_index, animation_graph_index, animation_name, frame_index);

	return;
}

/* ---------- private code */

static void scenery_animation_start_private(
	long scenery_index,
	long animation_graph_index,
	char const *animation_name,
	short frame_index)
{
	if (scenery_index != NONE && animation_graph_index != NONE)
	{
		struct scenery_datum *scenery = scenery_get(scenery_index);
		struct animation_graph *graph = animation_graph_definition_get(animation_graph_index);
		short animation_index = animation_graph_get_animation_by_name(animation_graph_index, animation_name);

		if (animation_index != NONE)
		{
			struct animation *animation = TAG_BLOCK_GET_ELEMENT(&graph->animations, animation_index, struct animation);

			SET_FLAG(scenery->scenery.flags, _scenery_self_animated_bit, TRUE);
			SET_FLAG(scenery->object.flags, _object_animates_automatically_bit, FALSE);
			scenery->object.animation.state.index = animation_index;
			scenery->object.animation.state.frame_index = PIN(frame_index, 0, animation->frame_count - 1);
			scenery->object.animation.animation_graph_index = animation_graph_index;
		}
		else
		{
			console_warning("the animation '%s' doesn't exist in the graph '%s'", animation_name, tag_get_name(animation_graph_index));
		}
	}

	return;
}
