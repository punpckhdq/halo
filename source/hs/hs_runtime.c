/*
HS_RUNTIME.C
*/

/* ---------- headers */

#include "cseries.h"
#include "object_lists.h"
#include "hs_library_external.h"
#include "hs_library_internal.h"
#include "hs_scenario_definitions.h"
#include "objects.h"
#include "render.h"
#include "ai.h"
#include "ai_constants.h"
#include "console.h"
#include "game_state.h"
#include "collisions.h"
#include "ai_debug.h"
#include "encounters.h"
#include "render_debug.h"
#include "draw_string.h"
#include "editor_stubs.h"
#include "terminal.h"
#include "ai_script.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "game.h"
#include "tag_files.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_HS_THREADS = 0x100,
	MAXIMUM_NUMBER_OF_HS_GLOBALS = 0x400,
	HS_THREAD_STACK_SIZE = 0x200
};

enum
{
	_hs_thread_type_script = 0,
	_hs_thread_type_global_initialize,
	_hs_thread_type_runtime_evaluate,
	NUMBER_OF_HS_THREAD_TYPES
};

enum
{
	_hs_thread_in_function_call_bit = 0,
	_hs_thread_latent_sleep_bit,
	NUMBER_OF_HS_THREAD_FLAGS
};

enum
{
	HS_SLEEP_FINISHED = NONE,
	HS_SLEEP_INDEFINITE = -2
};

enum
{
	HS_GLOBAL_DATUM_SALT = 0xACED, /* fake name */
	HS_NODE_GC_PERIOD = 16 /* fake name */
};

/* ---------- macros */

#define hs_thread_get(index) ((struct hs_thread *)datum_get(hs_thread_data, (index)))
#define hs_global_designator_index(designator) ((short)(designator)&MASK(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT)) /* fake name */
#define hs_global_runtime_index(designator) (((designator)&FLAG(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT)) ? hs_global_designator_index(designator) : hs_global_designator_index(designator)+hs_external_global_count) /* fake name */
#define hs_global_runtime_get(designator) ((struct hs_global_runtime *)datum_get(hs_global_data, hs_global_runtime_index(designator))) /* fake name */

#define valid_thread(thread) ((byte *)(thread)>=(byte *)hs_thread_data->data && (byte *)(thread)<(byte *)hs_thread_data->data+hs_thread_data->count*hs_thread_data->size && (byte *)(thread)->stack>=(thread)->stack_data && (byte *)(thread)->stack<(thread)->stack_data+HS_THREAD_STACK_SIZE && (thread)->stack->data+(thread)->stack->size<=(thread)->stack_data+HS_THREAD_STACK_SIZE)

#define match_script_assert(file, line, thread_index, expr, message) if (!(expr)) { display_assert(csprintf(temporary, "a problem occurred while executing the script %s: %s (%s)", hs_thread_format(thread_index), (message) ? (message) : "no reason given.", #expr), MATCH_FILE(file), MATCH_LINE(line), TRUE); system_exit(-1); } /* fake name */
#define script_assert(thread_index, expr, message) match_script_assert(__FILE__, __LINE__, thread_index, expr, message) /* fake name */
#define script_demand(thread_index, expr, message) ((expr) || script_error((thread_index), (message), #expr)) /* fake name */

/* ---------- structures */

struct hs_stack_frame
{
	struct hs_stack_frame *parent;
	long expression_index;
	long *child_result;
	short size;
	byte data[];
};

struct hs_thread
{
	short identifier;
	byte type;
	byte flags;
	long script_index;
	long sleep_until;
	long latent_sleep_until;
	struct hs_stack_frame *stack;
	long result;
	byte stack_data[HS_THREAD_STACK_SIZE];
};

struct hs_global_runtime
{
	short identifier;
	word pad;
	long value;
};

/* ---------- prototypes */

static long hs_thread_new(short type, long script_index);
static void hs_thread_delete(long thread_index);
static char const *hs_thread_format(long thread_index);
static void hs_thread_main(long thread_index);
static void hs_evaluate(long thread_index, long expression_index, long *destination);
static long hs_global_evaluate(short global_designator);
static void hs_script_evaluate(short script_index, long thread_index, boolean initialize);
static void hs_stack_push(long thread_index);
static void hs_stack_pop(long thread_index);
static void *hs_stack_allocate(long thread_index, long size);
static void hs_wake(long thread_index);
static long *hs_arguments_evaluate(long thread_index, short formal_parameter_count, short const *formal_parameters, boolean initialize);
static long hs_find_thread_by_script(short script_index);
static long hs_find_thread_by_name(char const *name);
static void hs_global_reconcile_read(short global_designator);
static void hs_global_reconcile_write(short global_designator);

/* ---------- globals */

boolean debug_trigger_volumes;
boolean debug_scripting;
unsigned long hs_debug_data[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_TRIGGER_VOLUMES_PER_SCENARIO)];
struct data_array *hs_global_data;
struct data_array *hs_thread_data;

static struct
{
	boolean initialized;
	short executing_thread_index;
} hs_runtime_globals;

/* ---------- public code */

static boolean script_error(
	long thread_index,
	char const *message,
	char const *condition)
{
	struct hs_thread *thread = hs_thread_get(thread_index);

	error(
		_error_silent,
		"script %s needs to be recompiled. (%s: %s)",
		hs_thread_format(thread_index),
		message ? message : "no reason given.",
		condition);

	return FALSE;
}

#include "hs_library_internal_runtime.h"

void hs_runtime_initialize(
	void)
{
	hs_thread_data = game_state_data_new("hs thread", MAXIMUM_NUMBER_OF_HS_THREADS, sizeof(struct hs_thread));
	hs_global_data = game_state_data_new("hs globals", MAXIMUM_NUMBER_OF_HS_GLOBALS, sizeof(struct hs_global_runtime));

	if (hs_thread_data && hs_global_data)
	{
		short global_index;

		match_vassert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 169, 2*hs_external_global_count<MAXIMUM_NUMBER_OF_HS_GLOBALS, "raise MAXIMUM_NUMBER_OF_HS_GLOBALS.");
		data_make_valid(hs_global_data);

		for (global_index = 0; global_index<hs_external_global_count; global_index++)
		{
			long index = datum_new_at_index(hs_global_data, DATUM_INDEX_NEW(global_index, HS_GLOBAL_DATUM_SALT));

			match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 177, index!=NONE);
		}
	}
	else
	{
		error(_error_immediate, "couldn't allocate scripting globals.");
	}

	return;
}

void hs_runtime_initialize_for_new_map(
	void)
{
	long thread_index;

	data_make_valid(hs_thread_data);
	hs_runtime_globals.initialized = TRUE;
	hs_runtime_globals.executing_thread_index = NONE;
	thread_index = hs_thread_new(_hs_thread_type_global_initialize, NONE);

	if (global_scenario_index!=NONE)
	{
		short global_index;
		short script_index;
		struct scenario *scenario = global_scenario_get();
		struct hs_thread *internal_thread = hs_thread_get(thread_index);

		for (global_index = 0; global_index<scenario->hs_globals.count; global_index++)
		{
			struct hs_global_runtime *global_runtime;
			struct hs_global_internal *global = hs_global_internal_get(global_index);

			datum_new_at_index(
				hs_global_data,
				DATUM_INDEX_NEW(hs_global_runtime_index(global_index), HS_GLOBAL_DATUM_SALT));
			global_runtime = hs_global_runtime_get(global_index);
			internal_thread->script_index = NONE;
			internal_thread->stack->size = 0;
			hs_evaluate(thread_index, global->initialization_expression_index, &global_runtime->value);

			if (TEST_FLAG(internal_thread->flags, _hs_thread_in_function_call_bit))
			{
				hs_thread_main(thread_index);

				if (global->type==_hs_type_object_list)
				{
					object_list_add_reference(hs_global_evaluate(global_index));
				}

				match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 231, thread_index, internal_thread->sleep_until==0, "a global initialization attempted to sleep.");
			}

			hs_global_reconcile_write(global_index);
		}

		hs_thread_delete(thread_index);

		for (script_index = 0; script_index<scenario->hs_scripts.count; script_index++)
		{
			struct hs_script *script = TAG_BLOCK_GET_ELEMENT(&scenario->hs_scripts, script_index, struct hs_script);

			if (script->script_type!=_hs_script_static
				&& script->script_type!=_hs_script_stub
				&& hs_thread_new(_hs_thread_type_script, script_index)==NONE)
			{
				error(_error_immediate, "ran out of script threads.");
			}
		}
	}

	memset(hs_debug_data, 0, sizeof(hs_debug_data));

	return;
}

void hs_runtime_dispose_from_old_map(
	void)
{
	short global_index;

	data_make_invalid(hs_thread_data);

	for (global_index = hs_external_global_count; global_index<hs_global_data->count; global_index++)
	{
		if (datum_try_and_get(hs_global_data, global_index))
		{
			datum_delete(hs_global_data, global_index);
		}
	}

	hs_runtime_globals.initialized = FALSE;

	return;
}

void hs_runtime_dispose(
	void)
{
	data_make_invalid(hs_global_data);

	return;
}

void hs_runtime_update(
	void)
{
	if (hs_runtime_globals.initialized)
	{
		long thread_index;
		long time = game_time_get();
		boolean console_command_running = FALSE;

		for (thread_index = data_next_index(hs_thread_data, NONE);
			hs_runtime_globals.initialized && thread_index!=NONE;
			thread_index = data_next_index(hs_thread_data, thread_index))
		{
			struct hs_thread *thread = hs_thread_get(thread_index);

			if (thread->type==_hs_thread_type_runtime_evaluate)
			{
				console_command_running = TRUE;
			}

			if (thread->sleep_until>=0 && thread->sleep_until<=time)
			{
				hs_thread_main(thread_index);
			}
		}

		object_list_gc();

		if (!console_command_running && game_time_get() % HS_NODE_GC_PERIOD==0)
		{
			hs_node_gc();
		}
	}

	return;
}

long hs_runtime_evaluate(
	long expression_index)
{
	long result = NONE;

	if (hs_runtime_globals.initialized && expression_index!=NONE)
	{
		long thread_index = hs_thread_new(_hs_thread_type_runtime_evaluate, NONE);

		if (thread_index!=NONE)
		{
			struct hs_thread *thread = hs_thread_get(thread_index);

			hs_evaluate(thread_index, expression_index, &thread->result);

			if (TEST_FLAG(thread->flags, _hs_thread_in_function_call_bit))
			{
				hs_thread_main(thread_index);
			}
			else
			{
				result = thread->result;
			}
		}
		else
		{
			error(_error_silent, "there are not enough threads to execute that command.");
		}
	}

	return result;
}

char const *hs_runtime_get_executing_thread_name(
	void)
{
	char const *result = NULL;

	if (hs_runtime_globals.executing_thread_index!=NONE)
	{
		result = hs_thread_format(hs_runtime_globals.executing_thread_index);
	}

	if (!result)
	{
		result = "[unknown]";
	}

	return result;
}

boolean hs_wake_by_name(
	char const *script_name)
{
	long thread_index = hs_find_thread_by_name(script_name);
	boolean result = FALSE;

	if (thread_index!=NONE)
	{
		hs_wake(thread_index);
		result = TRUE;
	}

	return result;
}

static char const *expression_get_function_name(
	long thread_index,
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	struct hs_thread *thread = hs_thread_get(thread_index);

	if (TEST_FLAG(expression->flags, _hs_syntax_node_script_bit))
	{
		return hs_script_get(expression->script_index)->name;
	}
	else if (expression->function_index!=_hs_function_begin || expression_index!=thread->stack->expression_index)
	{
		return hs_function_get(expression->function_index)->name;
	}
	else
	{
		long next_expression_index = *(long *)thread->stack->data;

		if (next_expression_index==NONE)
		{
			return "(end of script)";
		}
		else
		{
			return expression_get_function_name(thread_index, next_expression_index);
		}
	}
}

void render_debug_scripting(
	void)
{
	if (debug_scripting)
	{
		char buffer[10240];
		long thread_index;
		short tab_stops[] = {200, 300};

		sprintf(buffer, "|n|n|nscript name|tsleep time|tfunction");

		for (thread_index = data_next_index(hs_thread_data, NONE);
			hs_runtime_globals.initialized && thread_index!=NONE;
			thread_index = data_next_index(hs_thread_data, thread_index))
		{
			struct hs_thread *thread = hs_thread_get(thread_index);

			if (thread->sleep_until>=0)
			{
				sprintf(&buffer[strlen(buffer)], "|n%s|t", hs_thread_format(thread_index));
				sprintf(&buffer[strlen(buffer)], "%d", thread->sleep_until ? thread->sleep_until - game_time_get() : 0);
				strcat(buffer, "|t");

				if (thread->stack!=(struct hs_stack_frame *)thread->stack_data
					&& thread->sleep_until!=HS_SLEEP_INDEFINITE)
				{
					strcat(buffer, expression_get_function_name(thread_index, thread->stack->expression_index));
				}
			}
		}

		buffer[1024] = '\0';
		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		render_debug_string(TRUE, buffer);
		draw_string_set_tab_stops(tab_stops, 0);
	}

	return;
}

void render_debug_trigger_volumes(
	void)
{
	if (debug_trigger_volumes)
	{
		short trigger_volume_index;
		struct scenario *scenario = global_scenario_get();

		for (trigger_volume_index = 0; trigger_volume_index<scenario->trigger_volumes.count; trigger_volume_index++)
		{
			real_matrix4x3 matrix;
			real_vector3d local_extent;
			real_vector3d world_extent;
			short side_index;
			struct scenario_trigger_volume *trigger_volume = TAG_BLOCK_GET_ELEMENT(
				&scenario->trigger_volumes,
				trigger_volume_index,
				struct scenario_trigger_volume);

			switch (trigger_volume->type)
			{
			case _trigger_volume_type_world_aligned_bounding_box:
				matrix = *global_identity4x3;
				set_real_point3d(
					&matrix.position,
					trigger_volume->world_aligned_bounding_box.rectangle.x0,
					trigger_volume->world_aligned_bounding_box.rectangle.y0,
					trigger_volume->world_aligned_bounding_box.rectangle.z0);
				set_real_vector3d(
					&local_extent,
					trigger_volume->world_aligned_bounding_box.rectangle.x1 - trigger_volume->world_aligned_bounding_box.rectangle.x0,
					trigger_volume->world_aligned_bounding_box.rectangle.y1 - trigger_volume->world_aligned_bounding_box.rectangle.y0,
					trigger_volume->world_aligned_bounding_box.rectangle.z1 - trigger_volume->world_aligned_bounding_box.rectangle.z0);
				world_extent = local_extent;
				break;
			case _trigger_volume_type_bounding_box:
				local_extent = trigger_volume->bounding_box.extents;
				world_extent = trigger_volume->bounding_box.extents;
				matrix4x3_from_point_and_vectors(
					&matrix,
					&trigger_volume->bounding_box.position,
					&trigger_volume->bounding_box.forward,
					&trigger_volume->bounding_box.up);
				matrix4x3_transform_vector(&matrix, &local_extent, &world_extent);
				break;
			default:
				match_unreachable("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 531);
			}

			for (side_index = 0; side_index<6; side_index++)
			{
				real_point3d points[4];
				real_vector3d sides[2] = {0.f};
				short odd = side_index % 2;
				short axis = side_index / 2;

				if (odd)
				{
					point_from_line3d(&matrix.position, &world_extent, 1.f, &points[0]);
					sides[0].n[(axis + 1) % 3] = -local_extent.n[(axis + 1) % 3];
					sides[1].n[(axis + 2) % 3] = -local_extent.n[(axis + 2) % 3];
					matrix4x3_transform_vector(&matrix, &sides[0], &sides[0]);
					matrix4x3_transform_vector(&matrix, &sides[1], &sides[1]);
					point_from_line3d(&points[0], &sides[0], 1.f, &points[1]);
					point_from_line3d(&points[1], &sides[1], 1.f, &points[2]);
					point_from_line3d(&points[2], &sides[0], -1.f, &points[3]);
				}
				else
				{
					points[0] = matrix.position;
					sides[0].n[(axis + 1) % 3] = local_extent.n[(axis + 1) % 3];
					sides[1].n[(axis + 2) % 3] = local_extent.n[(axis + 2) % 3];
					matrix4x3_transform_vector(&matrix, &sides[0], &sides[0]);
					matrix4x3_transform_vector(&matrix, &sides[1], &sides[1]);
					point_from_line3d(&points[0], &sides[0], 1.f, &points[1]);
					point_from_line3d(&points[1], &sides[1], 1.f, &points[2]);
					point_from_line3d(&points[2], &sides[0], -1.f, &points[3]);
				}

				if (BIT_VECTOR_TEST_FLAG(hs_debug_data, trigger_volume_index))
				{
					render_debug_polygon_edges(points, NUMBEROF(points), global_real_argb_blue);
				}
				else
				{
					real_argb_color color = *global_real_argb_blue;

					color.alpha = 0.15f;
					render_debug_polygon_edges(points, NUMBEROF(points), global_real_argb_red);
					render_debug_polygon(points, NUMBEROF(points), &color);
				}
			}

			{
				real_point3d center;
				real_vector3d ray;
				struct collision_result result;

				point_from_line3d(&matrix.position, &world_extent, 0.5f, &center);
				vector_from_points3d(&render.camera.position, &center, &ray);
				scale_vector3d(&ray, 0.95f, &ray);

				if (!collision_test_vector(
					_collision_test_for_line_of_sight_flags,
					&render.camera.position,
					&ray,
					NONE,
					&result))
				{
					if (BIT_VECTOR_TEST_FLAG(hs_debug_data, trigger_volume_index))
					{
						render_debug_string_at_point(TRUE, &center, trigger_volume->name, global_real_argb_yellow);
					}
					else
					{
						render_debug_string_at_point(TRUE, &center, trigger_volume->name, global_real_argb_white);
					}
				}
			}
		}
	}

	return;
}

static long hs_thread_new(
	short type,
	long script_index)
{
	long thread_index = datum_new(hs_thread_data);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 623, type>=0 && type<NUMBER_OF_HS_THREAD_TYPES);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 624, type!=_hs_thread_type_script || script_index!=NONE);

	if (thread_index!=NONE)
	{
		struct hs_thread *thread = hs_thread_get(thread_index);

		thread->stack = (struct hs_stack_frame *)thread->stack_data;
		thread->stack->parent = NULL;
		thread->stack->size = 0;
		thread->stack->expression_index = NONE;
		thread->type = (byte)type;
		thread->script_index = script_index;
		thread->flags = 0;

		if (script_index!=NONE && hs_script_get(script_index)->script_type==_hs_script_dormant)
		{
			thread->sleep_until = HS_SLEEP_INDEFINITE;
		}
		else
		{
			thread->sleep_until = 0;
		}
	}

	return thread_index;
}

static void hs_thread_delete(
	long thread_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 656, hs_thread_get(thread_index)->type!=_hs_thread_type_script);
	datum_delete(hs_thread_data, thread_index);

	return;
}

static char const *hs_thread_format(
	long thread_index)
{
	char const *result = NULL;

	switch (hs_thread_get(thread_index)->type)
	{
	case _hs_thread_type_script:
		result = hs_script_get(hs_thread_get(thread_index)->script_index)->name;
		break;
	case _hs_thread_type_global_initialize:
		result = "[global initialize]";
		break;
	case _hs_thread_type_runtime_evaluate:
		result = "[console command]";
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 681);
	}

	return result;
}

static void hs_thread_main(
	long thread_index)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	struct hs_script *script = NULL;

	hs_runtime_globals.executing_thread_index = (short)thread_index;

	if (thread->type==_hs_thread_type_script)
	{
		script = hs_script_get(thread->script_index);
		match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 698, thread_index, script->script_type!=_hs_script_static && script->script_type!=_hs_script_stub, "found a static script at toplevel.");
	}

	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 701, thread_index, valid_thread(thread), "corrupted stack.");
	thread->sleep_until = 0;

	if (thread->stack==(struct hs_stack_frame *)thread->stack_data)
	{
		match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 707, script);
		thread->stack->size = 0;
		hs_evaluate(thread_index, script->root_expression_index, hs_stack_allocate(thread_index, sizeof(long)));
	}

	while (thread->stack!=(struct hs_stack_frame *)thread->stack_data &&
		thread->sleep_until>=0 &&
		(!game_in_progress() || thread->sleep_until<=game_time_get()) &&
		hs_runtime_globals.initialized)
	{
		struct hs_syntax_node *expression = hs_syntax_get(thread->stack->expression_index);
		boolean initialize = TEST_FLAG(thread->flags, _hs_thread_in_function_call_bit);

		thread->stack->size = 0;
		SET_FLAG(thread->flags, _hs_thread_in_function_call_bit, FALSE);

		if (!TEST_FLAG(expression->flags, _hs_syntax_node_script_bit))
		{
			struct hs_function_definition const *function = hs_function_get(expression->function_index);

			match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 728, function->evaluate);
			function->evaluate(expression->function_index, thread_index, initialize);
		}
		else
		{
			hs_script_evaluate(expression->script_index, thread_index, initialize);
		}
	}

	if (thread->stack==(struct hs_stack_frame *)thread->stack_data)
	{
		if (thread->type==_hs_thread_type_script)
		{
			if (script->script_type==_hs_script_startup || script->script_type==_hs_script_dormant)
			{
				thread->sleep_until = HS_SLEEP_FINISHED;
			}
		}
		else if (thread->type==_hs_thread_type_runtime_evaluate)
		{
			hs_thread_delete(thread_index);
		}
	}

	hs_runtime_globals.executing_thread_index = NONE;

	return;
}

static void hs_evaluate(
	long thread_index,
	long expression_index,
	long *destination)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 767, thread_index, valid_thread(thread), "corrupted stack.");
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 768, destination);

	if (TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_primitive_bit))
	{
		if (TEST_FLAG(expression->flags, _hs_syntax_node_variable_bit))
		{
			*destination = hs_cast(
				thread_index,
				hs_global_get_type((short)expression->data),
				expression->type,
				hs_global_evaluate((short)expression->data));
		}
		else
		{
			*destination = hs_cast(thread_index, expression->constant_type, expression->type, expression->data);
		}
	}
	else
	{
		thread->stack->child_result = destination;
		hs_stack_push(thread_index);
		SET_FLAG(thread->flags, _hs_thread_in_function_call_bit, TRUE);
		thread->stack->expression_index = expression_index;
	}

	return;
}

void hs_return(
	long thread_index,
	long value)
{
	short actual_type;
	struct hs_thread *thread = hs_thread_get(thread_index);
	struct hs_syntax_node *expression = hs_syntax_get(thread->stack->expression_index);

	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 805, thread_index, valid_thread(thread), "corrupted stack.");

	if (!TEST_FLAG(expression->flags, _hs_syntax_node_script_bit))
	{
		actual_type = hs_function_get(expression->function_index)->return_type;
	}
	else
	{
		actual_type = hs_script_get(expression->script_index)->return_type;
	}

	*thread->stack->parent->child_result = hs_cast(thread_index, actual_type, expression->type, value);
	hs_stack_pop(thread_index);

	return;
}

static long hs_global_evaluate(
	short global_designator)
{
	hs_global_reconcile_read(global_designator);
	return hs_global_runtime_get(global_designator)->value;
}

static void hs_script_evaluate(
	short script_index,
	long thread_index,
	boolean initialize)
{
	struct hs_script *script = hs_script_get(script_index);
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	if (initialize)
	{
		hs_evaluate(thread_index, script->root_expression_index, result);
	}
	else
	{
		hs_return(thread_index, *result);
	}

	return;
}

static void hs_stack_push(
	long thread_index)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	struct hs_stack_frame *new_frame = (struct hs_stack_frame *)((byte *)(thread->stack + 1) + thread->stack->size);

	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 862, thread_index, (byte *) (new_frame+1)<thread->stack_data+HS_THREAD_STACK_SIZE, "stack overflow.");
	new_frame->parent = thread->stack;
	thread->stack = new_frame;
	new_frame->size = 0;

	return;
}

static void hs_stack_pop(
	long thread_index)
{
	struct hs_thread *thread = hs_thread_get(thread_index);

	thread->stack = thread->stack->parent;

	return;
}

static void *hs_stack_allocate(
	long thread_index,
	long size)
{
	void *result;
	struct hs_thread *thread = hs_thread_get(thread_index);
	struct hs_stack_frame *frame = thread->stack;

	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 893, thread_index, valid_thread(thread), "corrupted stack.");
	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 894, thread_index, size, "attempt to allocate zero space from the stack.");
	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 895, thread_index, frame->data+frame->size+size<=thread->stack_data+HS_THREAD_STACK_SIZE, "stack overflow.");
	result = frame->data + frame->size;
	frame->size += (short)size;

	return result;
}

static void hs_wake(
	long thread_index)
{
	struct hs_thread *thread = hs_thread_get(thread_index);

	if (thread->sleep_until!=HS_SLEEP_FINISHED)
	{
		thread->sleep_until = 0;

		if (TEST_FLAG(thread->flags, _hs_thread_latent_sleep_bit))
		{
			thread->sleep_until = thread->latent_sleep_until;
			SET_FLAG(thread->flags, _hs_thread_latent_sleep_bit, FALSE);
		}
		else if (thread->stack->expression_index!=NONE
			&& hs_syntax_get(thread->stack->expression_index)->function_index==_hs_function_sleep_until)
		{
			hs_stack_pop(thread_index);
		}
		else if (thread->stack->parent
			&& thread->stack->parent->expression_index!=NONE
			&& hs_syntax_get(thread->stack->parent->expression_index)->function_index==_hs_function_sleep_until)
		{
			hs_stack_pop(thread_index);
			hs_stack_pop(thread_index);
			SET_FLAG(thread->flags, _hs_thread_in_function_call_bit, FALSE);
		}
	}

	return;
}

static long *hs_arguments_evaluate(
	long thread_index,
	short formal_parameter_count,
	short const *formal_parameters,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *result = hs_stack_allocate(thread_index, formal_parameter_count * sizeof(long));
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));

	if (initialize)
	{
		*argument_index = 0;
		*expression_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;
	}

	if (*argument_index<formal_parameter_count)
	{
		match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 964, thread_index, *expression_index!=NONE, "corrupted syntax tree.");

		if (script_demand(thread_index, hs_syntax_get(*expression_index)->type==formal_parameters[*argument_index], "unexpected actual parameters."))
		{
			hs_evaluate(thread_index, *expression_index, &result[*argument_index]);
			*expression_index = hs_syntax_get(*expression_index)->next_node_index;
			(*argument_index)++;
			result = NULL;
		}
	}
	else
	{
		match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 977, thread_index, *expression_index==NONE, "corrupted syntax tree.");
	}

	return result;
}

static long hs_find_thread_by_script(
	short script_index)
{
	long thread_index;

	for (thread_index = data_next_index(hs_thread_data, NONE);
		thread_index!=NONE;
		thread_index = data_next_index(hs_thread_data, thread_index))
	{
		struct hs_thread *thread = hs_thread_get(thread_index);

		if (thread->script_index==script_index)
		{
			return thread_index;
		}
	}

	return NONE;
}

static long hs_find_thread_by_name(
	char const *name)
{
	long thread_index;

	for (thread_index = data_next_index(hs_thread_data, NONE);
		thread_index!=NONE;
		thread_index = data_next_index(hs_thread_data, thread_index))
	{
		struct hs_thread *thread = hs_thread_get(thread_index);

		if (thread->script_index!=NONE)
		{
			struct hs_script *script = hs_script_get(thread->script_index);

			if (!_stricmp(script->name, name))
			{
				return thread_index;
			}
		}
	}

	return NONE;
}

static long hs_long_to_boolean(
	long n)
{
	long result;

	*(boolean *)&result = !n;
	return result;
}

static long hs_short_to_boolean(
	long s)
{
	long result;

	*(boolean *)&result = !(short)s;
	return result;
}

static long hs_string_to_boolean(
	long n)
{
	long result;

	*(boolean *)&result = !strlen((char const *)n);
	return result;
}

static long hs_data_to_void(
	long n)
{
	return 0;
}

static long hs_short_to_real(
	long s)
{
	long result;

	*(real *)&result = (real)(short)s;
	return result;
}

static long hs_long_to_real(
	long l)
{
	long result;

	*(real *)&result = (real)l;
	return result;
}

static long hs_enum_to_real(
	long e)
{
	long result;

	*(real *)&result = (real)((short)e + 1);
	return result;
}

static long hs_real_to_short(
	long r)
{
	long result;

	*(short *)&result = (short)*(real *)&r;
	return result;
}

static long hs_real_to_long(
	long r)
{
	return (long)*(real *)&r;
}

static long hs_long_to_short(
	long l)
{
	long result;

	*(short *)&result = (short)l;
	return result;
}

static long hs_object_name_to_object_list(
	long object_name_index)
{
	long object_list_index = NONE;
	long object_index = object_index_from_name_index((short)object_name_index);

	if (object_index!=NONE)
	{
		object_list_index = object_list_new();
		object_list_add(object_list_index, object_index);
	}

	return object_list_index;
}

static long hs_object_to_object_list(
	long object_index)
{
	long object_list_index = NONE;

	if (object_index!=NONE)
	{
		object_list_index = object_list_new();
		object_list_add(object_list_index, object_index);
	}

	return object_list_index;
}

long (*typecasting_procedures[NUMBER_OF_HS_NODE_TYPES][NUMBER_OF_HS_NODE_TYPES])(long value) =
{
	{NULL},		// _hs_unparsed
	{NULL},		// _hs_special_form
	{NULL},		// _hs_function_name
	{NULL},		// _hs_passthrough
	{			// _hs_type_void
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void,
		hs_data_to_void
	},
	{			// _hs_type_boolean
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		hs_long_to_boolean,
		hs_short_to_boolean,
		hs_long_to_boolean,
		hs_string_to_boolean
	},
	{			// _hs_type_real
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		hs_short_to_real,
		hs_long_to_real,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		hs_enum_to_real,
		hs_enum_to_real,
		hs_enum_to_real,
		hs_enum_to_real
	},
	{			// _hs_type_short_integer
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		hs_real_to_short,
		NULL,
		hs_long_to_short
	},
	{			// _hs_type_long_integer
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		hs_real_to_long,
		NULL,
		hs_long_to_short
	},
	{NULL},		// _hs_type_string
	{NULL},		// _hs_type_script
	{NULL},		// _hs_type_trigger_volume
	{NULL},		// _hs_type_cutscene_flag
	{NULL},		// _hs_type_cutscene_camera_point
	{NULL},		// _hs_type_cutscene_title
	{NULL},		// _hs_type_cutscene_recording
	{NULL},		// _hs_type_device_group
	{NULL},		// _hs_type_ai
	{NULL},		// _hs_type_ai_command_list
	{NULL},		// _hs_type_starting_profile
	{NULL},		// _hs_type_conversation
	{NULL},		// _hs_type_navpoint
	{NULL},		// _hs_type_hud_message
	{			// _hs_type_object_list
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		object_list_from_ai_reference,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		hs_object_to_object_list,
		hs_object_to_object_list,
		hs_object_to_object_list,
		hs_object_to_object_list,
		hs_object_to_object_list,
		hs_object_name_to_object_list,
		hs_object_name_to_object_list,
		hs_object_name_to_object_list,
		hs_object_name_to_object_list,
		hs_object_name_to_object_list,
		hs_object_name_to_object_list
	}
};

static boolean hs_object_type_can_cast(
	short actual_type,
	short desired_type)
{
	short actual_type_mask = hs_object_type_masks[actual_type];
	short desired_type_mask = hs_object_type_masks[desired_type];

	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 1433, actual_type>=0 && actual_type<NUMBER_OF_HS_OBJECT_TYPES);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 1434, desired_type>=0 && desired_type<NUMBER_OF_HS_OBJECT_TYPES);
	return (actual_type_mask & desired_type_mask)==actual_type_mask;
}

boolean hs_can_cast(
	short actual_type,
	short desired_type)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 1444, actual_type==_hs_passthrough || hs_type_valid(actual_type));
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 1445, hs_type_valid(desired_type));

	if (actual_type==_hs_passthrough || actual_type==desired_type)
	{
		return TRUE;
	}

	if (HS_TYPE_IS_OBJECT(desired_type))
	{
		short actual_object_type;
		short desired_object_type = desired_type - FIRST_HS_OBJECT_TYPE;

		if (HS_TYPE_IS_OBJECT(actual_type))
		{
			actual_object_type = actual_type - FIRST_HS_OBJECT_TYPE;
		}
		else if (HS_TYPE_IS_OBJECT_NAME(actual_type))
		{
			actual_object_type = actual_type - FIRST_HS_OBJECT_NAME_TYPE;
		}
		else
		{
			return FALSE;
		}

		return hs_object_type_can_cast(actual_object_type, desired_object_type);
	}
	else if (HS_TYPE_IS_OBJECT_NAME(desired_type))
	{
		if (HS_TYPE_IS_OBJECT_NAME(actual_type))
		{
			return hs_object_type_can_cast(
				actual_type - FIRST_HS_OBJECT_NAME_TYPE,
				desired_type - FIRST_HS_OBJECT_NAME_TYPE);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return typecasting_procedures[desired_type][actual_type]!=NULL;
	}
}

long hs_cast(
	long thread_index,
	short actual_type,
	short desired_type,
	long value)
{
	long result;

	match_script_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 1496, thread_index, hs_can_cast(actual_type, desired_type), "bad typecast.");

	if (actual_type==desired_type || actual_type==_hs_passthrough)
	{
		result = value;
	}
	else if (HS_TYPE_IS_OBJECT_NAME(desired_type))
	{
		result = value;
	}
	else if (HS_TYPE_IS_OBJECT(desired_type))
	{
		if (HS_TYPE_IS_OBJECT_NAME(actual_type))
		{
			result = object_index_from_name_index((short)value);
		}
		else
		{
			result = value;
		}
	}
	else
	{
		result = typecasting_procedures[desired_type][actual_type](value);
	}

	return result;
}

long *hs_macro_function_evaluate(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_function_definition const *function = hs_function_get(function_index);

	return hs_arguments_evaluate(
		thread_index,
		function->formal_parameter_count,
		function->formal_parameters,
		initialize);
}

#define HS_GLOBAL_RECONCILE_READ(type, c_type) case type: *(c_type *)&global->value = external->pointer ? *(c_type *)external->pointer : type##_default; break; /* fake name */
#define HS_GLOBAL_RECONCILE_WRITE(type, c_type) case type: if (external->pointer) *(c_type *)external->pointer = (c_type)(*(c_type *)&global->value); break; /* fake name */

static void hs_global_reconcile_read(
	short global_designator)
{
	if (global_designator & FLAG(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT))
	{
		struct hs_global_runtime *global = hs_global_runtime_get(global_designator);
		struct hs_global_external *external = hs_global_external_get(hs_global_designator_index(global_designator));

		switch (hs_global_get_type(global_designator))
		{
		HS_GLOBAL_RECONCILE_READ(_hs_type_boolean, boolean);
		HS_GLOBAL_RECONCILE_READ(_hs_type_real, real);
		HS_GLOBAL_RECONCILE_READ(_hs_type_short_integer, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_long_integer, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_string, char const *);
		HS_GLOBAL_RECONCILE_READ(_hs_type_script, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_trigger_volume, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_cutscene_flag, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_cutscene_camera_point, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_cutscene_title, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_cutscene_recording, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_device_group, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_ai, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_ai_command_list, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_starting_profile, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_conversation, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_navpoint, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_hud_message, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_object_list, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_sound, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_effect, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_damage, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_looping_sound, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_animation_graph, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_actor_variant, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_damage_effect, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_object_definition, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_enum_game_difficulty, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_enum_team, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_enum_ai_default_state, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_enum_actor_type, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_enum_hud_corner, short);
		HS_GLOBAL_RECONCILE_READ(_hs_type_object, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_unit, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_vehicle, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_weapon, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_device, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_scenery, long);
		HS_GLOBAL_RECONCILE_READ(_hs_type_object_name, short);
		default:
			match_halt("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 1592);
		}
	}

	return;
}

static void hs_global_reconcile_write(
	short global_designator)
{
	if (global_designator & FLAG(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT))
	{
		struct hs_global_runtime *global = hs_global_runtime_get(global_designator);
		struct hs_global_external *external = hs_global_external_get(hs_global_designator_index(global_designator));

		switch (hs_global_get_type(global_designator))
		{
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_boolean, boolean);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_real, real);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_short_integer, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_long_integer, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_string, char const *);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_script, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_trigger_volume, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_cutscene_flag, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_cutscene_camera_point, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_cutscene_title, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_cutscene_recording, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_device_group, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_ai, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_ai_command_list, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_starting_profile, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_conversation, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_navpoint, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_hud_message, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_object_list, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_sound, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_effect, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_damage, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_looping_sound, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_animation_graph, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_actor_variant, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_damage_effect, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_object_definition, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_enum_game_difficulty, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_enum_team, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_enum_ai_default_state, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_enum_actor_type, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_enum_hud_corner, short);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_object, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_unit, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_vehicle, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_weapon, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_device, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_scenery, long);
		HS_GLOBAL_RECONCILE_WRITE(_hs_type_object_name, short);
		default:
			match_halt("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 1649);
		}
	}

	return;
}
