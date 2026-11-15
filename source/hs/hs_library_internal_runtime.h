/*
HS_LIBRARY_INTERNAL_RUNTIME.H

file has inline function assertions.
*/

#ifndef __HS_LIBRARY_INTERNAL_RUNTIME_H
#define __HS_LIBRARY_INTERNAL_RUNTIME_H
#pragma once

/* ---------- public code */

void hs_evaluate_begin(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 21, function_index==_hs_function_begin);

	if (initialize)
	{
		*expression_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;
		*result = 0;
	}

	if (*expression_index!=NONE)
	{
		hs_evaluate(thread_index, *expression_index, result);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
	}
	else
	{
		hs_return(thread_index, *result);
	}

	return;
}

static long hs_syntax_nth(
	long expression_index,
	short n)
{
	short i;

	for (i = 0; i<n; i++)
	{
		expression_index = hs_syntax_get(expression_index)->next_node_index;
	}

	return expression_index;
}

void hs_evaluate_begin_random(
	short function_index,
	long thread_index,
	boolean initialize)
{
	short random_offset;
	short i;
	struct hs_thread *thread = hs_thread_get(thread_index);
	short *argument_count = hs_stack_allocate(thread_index, sizeof(short));
	unsigned long *evaluated_arguments = hs_stack_allocate(thread_index, BIT_VECTOR_SIZE_IN_BYTES(LONG_BITS));
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 69, function_index==_hs_function_begin_random);

	if (initialize)
	{
		long expression_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;

		for (*argument_count = 0; expression_index!=NONE; (*argument_count)++)
		{
			expression_index = hs_syntax_get(expression_index)->next_node_index;
		}

		match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 80, *argument_count<LONG_BITS);
		memset(evaluated_arguments, 0, BIT_VECTOR_SIZE_IN_BYTES(*argument_count));
	}

	random_offset = random_range(0, *argument_count);

	for (i = 0; i<*argument_count; i++)
	{
		short argument_index = (random_offset + i) % *argument_count;

		if (!BIT_VECTOR_TEST_FLAG(evaluated_arguments, argument_index))
		{
			hs_evaluate(
				thread_index,
				hs_syntax_nth(hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index, argument_index),
				result);
			BIT_VECTOR_SET_FLAG(evaluated_arguments, argument_index, TRUE);
			break;
		}
	}

	if (i==*argument_count)
	{
		hs_return(thread_index, *result);
	}

	return;
}

void hs_evaluate_if(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *condition = hs_stack_allocate(thread_index, sizeof(long));
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 119, function_index==_hs_function_if);

	if (initialize)
	{
		*condition = FALSE;
		*expression_index = NONE;
		hs_evaluate(
			thread_index,
			hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index,
			condition);
	}
	else if (*expression_index==NONE)
	{
		if (*(boolean *)condition)
		{
			*expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index)->next_node_index;
		}
		else
		{
			*expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index)->next_node_index)->next_node_index;

			if (*expression_index==NONE)
			{
				hs_return(thread_index, 0);
				return;
			}
		}

		hs_evaluate(thread_index, *expression_index, result);
	}
	else
	{
		hs_return(thread_index, *result);
	}

	return;
}

void hs_evaluate_set(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long variable_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;
	struct hs_syntax_node *variable = hs_syntax_get(variable_index);
	long *result = hs_stack_allocate(thread_index, sizeof(long));
	short type = hs_global_get_type((short)variable->data);

	if (initialize)
	{
		if (type==_hs_type_object_list)
		{
			object_list_remove_reference(hs_global_evaluate((short)variable->data));
		}

		hs_evaluate(
			thread_index,
			hs_syntax_get(variable_index)->next_node_index,
			&hs_global_runtime_get(variable->data)->value);
	}
	else
	{
		hs_global_reconcile_write((short)variable->data);

		if (type==_hs_type_object_list)
		{
			object_list_add_reference(hs_global_evaluate((short)variable->data));
		}

		hs_return(thread_index, hs_global_evaluate((short)variable->data));
	}

	return;
}

void hs_evaluate_logical(
	short function_index,
	long thread_index,
	boolean initialize)
{
	long result_long;
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	long *argument = hs_stack_allocate(thread_index, sizeof(long));
	boolean *result = hs_stack_allocate(thread_index, sizeof(boolean));
	boolean and = function_index==_hs_function_and;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 207, function_index==_hs_function_and || function_index==_hs_function_or);

	if (initialize)
	{
		*expression_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;
		*result = and;
	}
	else
	{
		boolean value = *(boolean *)argument;

		if (and)
		{
			*result = *result && value;
		}
		else
		{
			*result = *result || value;
		}
	}

	if (*expression_index!=NONE && *result==and)
	{
		hs_evaluate(thread_index, *expression_index, argument);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
	}
	else
	{
		*(boolean *)&result_long = *result;
		hs_return(thread_index, result_long);
	}

	return;
}

void hs_evaluate_arithmetic(
	short function_index,
	long thread_index,
	boolean initialize)
{
	long result_long;
	struct hs_thread *thread = hs_thread_get(thread_index);
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	real *argument = hs_stack_allocate(thread_index, sizeof(real));
	real *result = hs_stack_allocate(thread_index, sizeof(real));

	if (initialize)
	{
		*argument_index = 0;
		*expression_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;
	}
	else
	{
		real value = *argument;

		if (*argument_index==0)
		{
			*result = value;
		}
		else
		{
			switch (function_index)
			{
			case _hs_function_plus:
				*result += value;
				break;
			case _hs_function_minus:
				*result -= value;
				break;
			case _hs_function_times:
				*result *= value;
				break;
			case _hs_function_divide:
				*result /= value;
				break;
			case _hs_function_min:
				*result = MIN(*result, value);
				break;
			case _hs_function_max:
				*result = MAX(*result, value);
				break;
			default:
				match_halt("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 273);
			}
		}

		(*argument_index)++;
	}

	if (*expression_index!=NONE)
	{
		hs_evaluate(thread_index, *expression_index, (long *)argument);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
	}
	else
	{
		*(real *)&result_long = *result;
		hs_return(thread_index, result_long);
	}

	return;
}

void hs_evaluate_equality(
	short function_index,
	long thread_index,
	boolean initialize)
{
	short parameter_types[2];
	long *arguments;
	long result_long;
	boolean result;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 305, function_index==_hs_function_equal || function_index==_hs_function_not_equal);
	parameter_types[1] = hs_syntax_get(hs_syntax_get(hs_syntax_get(hs_thread_get(thread_index)->stack->expression_index)->data)->next_node_index)->type;
	parameter_types[0] = parameter_types[1];
	arguments = hs_arguments_evaluate(thread_index, NUMBEROF(parameter_types), parameter_types, initialize);

	if (arguments)
	{
		result = memcmp(&arguments[0], &arguments[1], hs_type_sizes[parameter_types[0]])==0;

		if (function_index==_hs_function_not_equal)
		{
			result = !result;
		}

		*(boolean *)&result_long = result;
		hs_return(thread_index, result_long);
	}

	return;
}

#define match_hs_inequality(file, line, operand0, operand1) /* fake name */ \
{ \
	real arg0 = (operand0); \
	real arg1 = (operand1); \
	\
	switch (function_index) \
	{ \
	case _hs_function_gt: result = arg0>arg1; break; \
	case _hs_function_lt: result = arg0<arg1; break; \
	case _hs_function_gte: result = arg0>=arg1; break; \
	case _hs_function_lte: result = arg0<=arg1; break; \
	default: result = FALSE; match_halt(file, line); \
	} \
}

void hs_evaluate_inequality(
	short function_index,
	long thread_index,
	boolean initialize)
{
	static short parameter_types[2];
	long *arguments;
	boolean result;
	long result_long;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 349, function_index>=_hs_function_gt && function_index<=_hs_function_lte);
	parameter_types[1] = hs_syntax_get(hs_syntax_get(hs_syntax_get(hs_thread_get(thread_index)->stack->expression_index)->data)->next_node_index)->type;
	parameter_types[0] = parameter_types[1];
	arguments = hs_arguments_evaluate(thread_index, NUMBEROF(parameter_types), parameter_types, initialize);

	if (arguments)
	{
		switch (parameter_types[0])
		{
		case _hs_type_real:
			match_hs_inequality("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 363, *(real *)&arguments[0], *(real *)&arguments[1]);
			break;
		case _hs_type_long_integer:
			match_hs_inequality("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 366, (real)arguments[0], (real)arguments[1]);
			break;
		default:
			match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 369, parameter_types[0]==_hs_type_short_integer || HS_TYPE_IS_ENUM(parameter_types[0]));
			match_hs_inequality("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 370, (real)*(short *)&arguments[0], (real)*(short *)&arguments[1]);
			break;
		}

		*(boolean *)&result_long = result;
		hs_return(thread_index, result_long);
	}

	return;
}

void hs_evaluate_sleep(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *sleep_ticks = hs_stack_allocate(thread_index, sizeof(long));
	long *script_index = hs_stack_allocate(thread_index, sizeof(long));
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long sleep_thread_index = thread_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 393, function_index==_hs_function_sleep);

	if (initialize)
	{
		hs_evaluate(
			thread_index,
			hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index,
			sleep_ticks);
		*argument_index = 0;
	}
	else
	{
		if (*argument_index==0)
		{
			long expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index)->next_node_index;

			(*argument_index)++;

			if (expression_index!=NONE)
			{
				hs_evaluate(thread_index, expression_index, script_index);
				return;
			}

			*script_index = NONE;
		}

		if (*argument_index)
		{
			short ticks = *(short *)sleep_ticks;

			if (ticks)
			{
				short sleep_script_index = *(short *)script_index;

				if (sleep_script_index!=NONE)
				{
					sleep_thread_index = hs_find_thread_by_script(sleep_script_index);
				}

				if (sleep_thread_index!=NONE)
				{
					struct hs_thread *sleep_thread = hs_thread_get(sleep_thread_index);
					long sleep_until = ticks<0 ? HS_SLEEP_INDEFINITE : game_time_get() + ticks;

					if (sleep_thread->sleep_until!=HS_SLEEP_FINISHED)
					{
						if (sleep_thread_index!=thread_index
							&& !TEST_FLAG(sleep_thread->flags, _hs_thread_latent_sleep_bit))
						{
							SET_FLAG(sleep_thread->flags, _hs_thread_latent_sleep_bit, TRUE);
							sleep_thread->latent_sleep_until = sleep_thread->sleep_until;
						}

						hs_thread_get(sleep_thread_index)->sleep_until = sleep_until;
					}
				}
			}

			hs_return(thread_index, 0);
		}
	}

	return;
}

void hs_evaluate_sleep_until(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *condition = hs_stack_allocate(thread_index, sizeof(long));
	long *period = hs_stack_allocate(thread_index, sizeof(long));
	long *timeout = hs_stack_allocate(thread_index, sizeof(long));
	long *start_time = hs_stack_allocate(thread_index, sizeof(long));
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 485, function_index==_hs_function_sleep_until);

	if (initialize)
	{
		*(boolean *)condition = FALSE;
		*start_time = game_time_get();
		*argument_index = 0;
		*(short *)period = TICKS_PER_SECOND;
		*timeout = NONE;

		if (expression_index!=NONE)
		{
			hs_evaluate(thread_index, expression_index, period);
			return;
		}
	}

	if (*argument_index==0)
	{
		(*argument_index)++;

		if (expression_index!=NONE)
		{
			expression_index = hs_syntax_get(expression_index)->next_node_index;

			if (expression_index!=NONE)
			{
				hs_evaluate(thread_index, expression_index, timeout);
				return;
			}
		}
	}

	if (*argument_index==1)
	{
		if (*(boolean *)condition || *timeout!=NONE && game_time_get()>=*start_time + *timeout)
		{
			hs_return(thread_index, 0);
		}
		else
		{
			hs_evaluate(
				thread_index,
				hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index,
				condition);
			thread->sleep_until = MAX(1, *(short *)period) + game_time_get();

			if (*timeout!=NONE)
			{
				thread->sleep_until = MIN(*start_time + *timeout, thread->sleep_until);
			}
		}
	}

	return;
}

void hs_evaluate_wake(
	short function_index,
	long thread_index,
	boolean initialize)
{
	long script_thread_index;
	struct hs_thread *thread = hs_thread_get(thread_index);
	long script_name_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;
	struct hs_syntax_node *script_name_node = hs_syntax_get(script_name_index);

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 556, function_index==_hs_function_wake);
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 557, TEST_FLAG(script_name_node->flags, _hs_syntax_node_primitive_bit));
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 558, script_name_node->type==_hs_type_script);
	script_thread_index = hs_find_thread_by_script((short)script_name_node->data);

	if (script_thread_index!=NONE)
	{
		hs_wake(script_thread_index);
	}

	hs_return(thread_index, 0);

	return;
}

static void hs_inspect_boolean(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 577, type==_hs_type_boolean);
	sprintf(buffer, "%s", (boolean)value ? "true" : "false");

	return;
}

static void hs_inspect_real(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 588, type==_hs_type_real);
	sprintf(buffer, "%f", *(real *)&value);

	return;
}

static void hs_inspect_short_integer(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 599, type==_hs_type_short_integer);
	sprintf(buffer, "%d", (short)value);

	return;
}

static void hs_inspect_long_integer(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 610, type==_hs_type_long_integer);
	sprintf(buffer, "%ld", value);

	return;
}

static void hs_inspect_string(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 621, type==_hs_type_string);
	sprintf(buffer, "%s", (char const *)value);

	return;
}

static void hs_inspect_enum(
	short type,
	long value,
	char *buffer)
{
	struct hs_enum_definition const *enum_definition = &hs_enum_table[type - FIRST_HS_ENUM_TYPE];
	short enum_value = (short)value;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 635, HS_TYPE_IS_ENUM(type));
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 636, enum_value>=0 && enum_value<enum_definition->count);
	sprintf(buffer, "%s", enum_definition->identifiers[enum_value]);

	return;
}

static void (*hs_type_inspectors[NUMBER_OF_HS_NODE_TYPES])(short type, long value, char *buffer) =
{
	NULL,							// _hs_unparsed
	NULL,							// _hs_special_form
	NULL,							// _hs_function_name
	NULL,							// _hs_passthrough
	NULL,							// _hs_type_void
	hs_inspect_boolean,				// _hs_type_boolean
	hs_inspect_real,				// _hs_type_real
	hs_inspect_short_integer,		// _hs_type_short_integer
	hs_inspect_long_integer,		// _hs_type_long_integer
	hs_inspect_string,				// _hs_type_string
	NULL,							// _hs_type_script
	NULL,							// _hs_type_trigger_volume
	NULL,							// _hs_type_cutscene_flag
	NULL,							// _hs_type_cutscene_camera_point
	NULL,							// _hs_type_cutscene_title
	NULL,							// _hs_type_cutscene_recording
	NULL,							// _hs_type_device_group
	NULL,							// _hs_type_ai
	NULL,							// _hs_type_ai_command_list
	NULL,							// _hs_type_starting_profile
	NULL,							// _hs_type_conversation
	NULL,							// _hs_type_navpoint
	NULL,							// _hs_type_hud_message
	NULL,							// _hs_type_object_list
	NULL,							// _hs_type_sound
	NULL,							// _hs_type_effect
	NULL,							// _hs_type_damage
	NULL,							// _hs_type_looping_sound
	NULL,							// _hs_type_animation_graph
	NULL,							// _hs_type_actor_variant
	NULL,							// _hs_type_damage_effect
	NULL,							// _hs_type_object_definition
	hs_inspect_enum,				// _hs_type_enum_game_difficulty
	hs_inspect_enum,				// _hs_type_enum_team
	hs_inspect_enum,				// _hs_type_enum_ai_default_state
	hs_inspect_enum,				// _hs_type_enum_actor_type
	hs_inspect_enum,				// _hs_type_enum_hud_corner
	NULL,							// _hs_type_object
	NULL,							// _hs_type_unit
	NULL,							// _hs_type_vehicle
	NULL,							// _hs_type_weapon
	NULL,							// _hs_type_device
	NULL,							// _hs_type_scenery
	NULL							// _hs_type_object_name
};

void hs_evaluate_inspect(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 700, function_index==_hs_function_inspect);

	if (initialize)
	{
		hs_evaluate(
			thread_index,
			hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index,
			result);
	}
	else
	{
		short type = hs_syntax_get(hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index)->type;

		if (hs_type_inspectors[type])
		{
			char printbuffer[1024];

			hs_type_inspectors[type](type, *result, printbuffer);
			console_printf(FALSE, printbuffer);
		}

		hs_return(thread_index, 0);
	}

	return;
}

void hs_evaluate_object_cast_up(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *object_index = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 732, function_index>=_hs_function_object_to_unit && function_index<=_hs_function_object_to_unit);

	if (initialize)
	{
		hs_evaluate(
			thread_index,
			hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index,
			object_index);
	}
	else if (*object_index!=NONE)
	{
		struct object_datum *object = object_get(*object_index);
		short object_type = function_index - _hs_function_object_to_unit + _hs_type_unit - FIRST_HS_OBJECT_TYPE;

		if (TEST_FLAG(hs_object_type_masks[object_type], object->object.type))
		{
			hs_return(thread_index, *object_index);
		}
		else
		{
			error(
				_error_silent,
				"attempt to convert object %s to type %s",
				tag_get_name(object->definition_index),
				hs_type_names[FIRST_HS_OBJECT_TYPE + object_type]);
			hs_return(thread_index, NONE);
		}
	}
	else
	{
		hs_return(thread_index, NONE);
	}

	return;
}

enum
{
	MAXIMUM_DEBUG_STRING_COUNT = 32 /* fake name */
};

void hs_evaluate_debug_string(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread *thread = hs_thread_get(thread_index);
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	long *string_count = hs_stack_allocate(thread_index, sizeof(long));
	char const **strings = hs_stack_allocate(thread_index, MAXIMUM_DEBUG_STRING_COUNT * sizeof(char const *));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 772, (function_index>=_hs_function_debug_string__first) && (function_index<=_hs_function_debug_string__last));

	if (initialize)
	{
		*expression_index = hs_syntax_get(hs_syntax_get(thread->stack->expression_index)->data)->next_node_index;
		*string_count = 0;
		memset((void *)strings, 0, MAXIMUM_DEBUG_STRING_COUNT * sizeof(char const *));
	}

	if (*expression_index!=NONE && *string_count<MAXIMUM_DEBUG_STRING_COUNT)
	{
		long parameter_result;

		hs_evaluate(thread_index, *expression_index, &parameter_result);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
		strings[*string_count] = (char const *)parameter_result;
		(*string_count)++;
	}
	else
	{
		void (*debug_function)(long name_count, char const **names) = NULL;

		switch (function_index)
		{
		case _hs_function_debug_string__ai_debug_communication_suppress:
			debug_function = ai_debug_communication_suppress;
			break;
		case _hs_function_debug_string__ai_debug_communication_ignore:
			debug_function = ai_debug_communication_ignore;
			break;
		case _hs_function_debug_string__ai_debug_communication_focus:
			debug_function = ai_debug_communication_focus;
			break;
		default:
			match_halt("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 816);
		}

		if (debug_function)
		{
			debug_function(*string_count, strings);
		}

		hs_return(thread_index, NONE);
	}

	return;
}

#endif // __HS_LIBRARY_INTERNAL_RUNTIME_H
