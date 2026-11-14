/*
HS_LIBRARY_INTERNAL_COMPILE.H

file has inline function assertions.
*/

#ifndef __HS_LIBRARY_INTERNAL_COMPILE_H
#define __HS_LIBRARY_INTERNAL_COMPILE_H
#pragma once

/* ---------- public code */

boolean hs_parse_begin(
	short function_index,
	long expression_index)
{
	short argument_count;
	boolean success = TRUE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	long argument_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 21, function_index==_hs_function_begin || function_index==_hs_function_begin_random);

	for (argument_count = 0; success && argument_index!=NONE; argument_count++)
	{
		long next_argument_index = hs_syntax_get(argument_index)->next_node_index;

		if (function_index==_hs_function_begin)
		{
			short expected_type = next_argument_index==NONE ? expression->type : _hs_type_void;

			success = hs_parse(argument_index, expected_type);

			if (next_argument_index==NONE && !expression->type && success)
			{
				expression->type = hs_syntax_get(argument_index)->type;
			}
		}
		else
		{
			success = hs_parse(argument_index, expression->type);

			if (!expression->type && success)
			{
				expression->type = hs_syntax_get(argument_index)->type;
			}
		}

		argument_index = next_argument_index;
	}

	if (success)
	{
		if (argument_count<1)
		{
			sprintf(
				hs_compile_globals.error_buffer,
				"a statement block must contain at least one argument.",
				hs_function_get(function_index)->name);
			hs_compile_globals.error = hs_compile_globals.error_buffer;
			hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
			success = FALSE;
		}
		else if (argument_count>LONG_BITS && function_index==_hs_function_begin_random)
		{
			hs_compile_globals.error = "begin_random can take a maximum of 32 arguments (matt can increase this.)";
			hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
			success = FALSE;
		}
	}

	return success;
}

boolean hs_parse_if(
	short function_index,
	long expression_index)
{
	long then_index;
	long else_index;
	boolean success = FALSE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	long condition_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 91, function_index==_hs_function_if);

	if (condition_index!=NONE &&
		(then_index = hs_syntax_get(condition_index)->next_node_index)!=NONE &&
		((else_index = hs_syntax_get(then_index)->next_node_index)==NONE || hs_syntax_get(else_index)->next_node_index==NONE))
	{
		if (hs_parse(condition_index, _hs_type_boolean))
		{
			if (hs_parse(then_index, expression->type))
			{
				if (!expression->type)
				{
					expression->type = hs_syntax_get(then_index)->type;
				}

				success = else_index==NONE || hs_parse(else_index, expression->type);
			}
			else if (!hs_compile_globals.error
				&& !expression->type
				&& else_index!=NONE
				&& hs_parse(else_index, expression->type))
			{
				expression->type = hs_syntax_get(else_index)->type;
				success = hs_parse(then_index, expression->type);
			}
		}
	}
	else
	{
		hs_compile_globals.error = "i expected (if <condition> <then> [<else>]).";
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
	}

	return success;
}

static long hs_parse_cond_recursive(
	long root_expression_index,
	long expression_index)
{
	long if_index = datum_new(hs_syntax_data);

	if (if_index!=NONE)
	{
		struct hs_syntax_node *if_expression = hs_syntax_get(if_index);

		if_expression->source_offset = hs_syntax_get(root_expression_index)->source_offset;
		if_expression->flags = 0;
		if_expression->next_node_index = NONE;

		if (expression_index==NONE)
		{
			SET_FLAG(if_expression->flags, _hs_syntax_node_primitive_bit, TRUE);
			if_expression->constant_type = hs_syntax_get(root_expression_index)->type;
			if_expression->type = if_expression->constant_type;
			if_expression->data = 0;
		}
		else if (!TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_primitive_bit))
		{
			long condition_index = hs_syntax_get(expression_index)->data;
			struct hs_syntax_node *condition = hs_syntax_get(condition_index);

			if (!condition->next_node_index!=NONE)
			{
				long begin_index = datum_new(hs_syntax_data);
				long begin_name_index = datum_new(hs_syntax_data);

				if (begin_index==NONE || begin_name_index==NONE)
				{
					hs_compile_globals.error = "i couldn't allocate a syntax node.";
					hs_compile_globals.error_offset = hs_syntax_get(root_expression_index)->source_offset;
					if_index = NONE;
				}
				else
				{
					struct hs_syntax_node *begin = hs_syntax_get(begin_index);
					struct hs_syntax_node *begin_name = hs_syntax_get(begin_name_index);
					struct hs_syntax_node *if_name = hs_syntax_get(expression_index);

					begin->next_node_index = hs_parse_cond_recursive(
						root_expression_index,
						hs_syntax_get(expression_index)->next_node_index);

					if (begin->next_node_index==NONE)
					{
						if_index = NONE;
					}
					else
					{
						if_expression->function_index = _hs_function_if;
						if_expression->data = expression_index;
						if_name->data = 0;
						if_name->function_index = _hs_function_if;
						if_name->flags = FLAG(_hs_syntax_node_primitive_bit);
						if_name->next_node_index = condition_index;
						if_name->source_offset = NONE;
						if_name->type = _hs_function_name;
						begin->data = begin_name_index;
						begin->flags = 0;
						begin->source_offset = if_expression->source_offset;
						begin_name->data = 0;
						begin_name->function_index = _hs_function_begin;
						begin_name->flags = FLAG(_hs_syntax_node_primitive_bit);
						begin_name->next_node_index = hs_syntax_get(condition_index)->next_node_index;
						begin_name->source_offset = NONE;
						begin_name->type = _hs_function_name;
						condition->next_node_index = begin_index;
					}
				}
			}
			else
			{
				hs_compile_globals.error = "this argument to cond needs a result.";
				hs_compile_globals.error_offset = hs_syntax_get(condition_index)->source_offset;
				if_index = NONE;
			}
		}
		else
		{
			hs_compile_globals.error = "this argument to cond should be a condition/result pair";
			hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
			if_index = NONE;
		}
	}
	else
	{
		hs_compile_globals.error = "i couldn't allocate a syntax node.";
		hs_compile_globals.error_offset = hs_syntax_get(root_expression_index)->source_offset;
	}

	return if_index;
}

boolean hs_parse_cond(
	short function_index,
	long expression_index)
{
	boolean success = FALSE;
	long if_index = hs_parse_cond_recursive(
		expression_index,
		hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index);

	if (if_index!=NONE)
	{
		struct hs_syntax_node *expression = hs_syntax_get(expression_index);
		struct hs_syntax_node *if_expression = hs_syntax_get(if_index);
		short identifier = expression->identifier;
		short type = expression->type;

		if_expression->next_node_index = expression->next_node_index;
		*expression = *if_expression;
		expression->identifier = identifier;
		success = hs_parse(expression_index, type);
	}

	return success;
}

boolean hs_parse_set(
	short function_index,
	long expression_index)
{
	boolean success = FALSE;
	long variable_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	if (variable_index!=NONE)
	{
		long value_index = hs_syntax_get(variable_index)->next_node_index;

		if (value_index!=NONE)
		{
			if (hs_syntax_get(value_index)->next_node_index==NONE)
			{
				struct hs_syntax_node *variable = hs_syntax_get(variable_index);
				short global_index = hs_find_global_by_name(
					hs_compile_globals.compiled_source + variable->source_offset);

				if (global_index!=NONE)
				{
					variable->type = hs_global_get_type(global_index);

					if (!expression->type || hs_can_cast(variable->type, expression->type))
					{
						match_vassert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 294, hs_parse_variable(variable_index), "asserted");

						if (!expression->type)
						{
							expression->type = variable->type;
						}

						if (hs_parse(value_index, variable->type))
						{
							success = TRUE;
						}
					}
					else
					{
						sprintf(
							hs_compile_globals.error_buffer,
							"you cannot pass the result of this set (type %s) to a function that expects type %s.",
							variable->type,
							hs_syntax_get(expression_index)->type);
						hs_compile_globals.error = hs_compile_globals.error_buffer;
						hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
					}
				}
				else
				{
					hs_compile_globals.error = "this is not a valid global variable.";
					hs_compile_globals.error_offset = variable->source_offset;
				}
			}
			else
			{
				hs_compile_globals.error = "i didn't expect this argument.";
				hs_compile_globals.error_offset = hs_syntax_get(hs_syntax_get(value_index)->next_node_index)->source_offset;
			}
		}
		else
		{
			hs_compile_globals.error = "i expected an assignment value.";
			hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
		}
	}
	else
	{
		hs_compile_globals.error = "i expected a variable to set and a value.";
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
	}

	return success;
}

boolean hs_parse_logical(
	short function_index,
	long expression_index)
{
	short argument_count;
	boolean success = TRUE;
	long argument_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 349, function_index==_hs_function_and || function_index==_hs_function_or);

	for (argument_count = 0; success && argument_index!=NONE; argument_count++)
	{
		success = hs_parse(argument_index, _hs_type_boolean);
		argument_index = hs_syntax_get(argument_index)->next_node_index;
	}

	if (success && argument_count<2)
	{
		sprintf(
			hs_compile_globals.error_buffer,
			"the %s call requires at least 2 arguments.",
			hs_function_get(function_index)->name);
		hs_compile_globals.error = hs_compile_globals.error_buffer;
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
		success = FALSE;
	}

	return success;
}

boolean hs_parse_arithmetic(
	short function_index,
	long expression_index)
{
	short argument_count;
	boolean success = TRUE;
	long argument_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 381, function_index>=_hs_function_plus && function_index<=_hs_function_max);

	for (argument_count = 0; success && argument_index!=NONE; argument_count++)
	{
		success = hs_parse(argument_index, _hs_type_real);
		argument_index = hs_syntax_get(argument_index)->next_node_index;
	}

	if (success && argument_count<2 || function_index==_hs_function_divide && argument_count>2)
	{
		sprintf(
			hs_compile_globals.error_buffer,
			"the %s call requires %s2 arguments.",
			hs_function_get(function_index)->name,
			function_index==_hs_function_divide ? "" : "at least ");
		hs_compile_globals.error = hs_compile_globals.error_buffer;
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
		success = FALSE;
	}

	return success;
}

static boolean hs_get_parameter_indices(
	char const *function_name,
	short count,
	long *result_indices,
	long expression_index)
{
	short index;
	boolean success = TRUE;
	long argument_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	for (index = 0; argument_index!=NONE && index<count; index++)
	{
		result_indices[index] = argument_index;
		argument_index = hs_syntax_get(argument_index)->next_node_index;
	}

	if (index!=count || argument_index!=NONE)
	{
		sprintf(hs_compile_globals.error_buffer, "the %s call requires %d arguments.", function_name, count);
		hs_compile_globals.error = hs_compile_globals.error_buffer;
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
		success = FALSE;
	}

	return success;
}

boolean hs_parse_equality(
	short function_index,
	long expression_index)
{
	long parameter_indices[2];
	boolean success = FALSE;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 444, function_index==_hs_function_equal || function_index==_hs_function_not_equal);

	if (hs_get_parameter_indices(
		hs_function_get(function_index)->name,
		NUMBEROF(parameter_indices),
		parameter_indices,
		expression_index))
	{
		if (hs_parse(parameter_indices[0], _hs_unparsed))
		{
			if (hs_parse(parameter_indices[1], hs_syntax_get(parameter_indices[0])->type))
			{
				success = TRUE;
			}
		}
		else if (!hs_compile_globals.error && hs_parse(parameter_indices[1], _hs_unparsed))
		{
			if (hs_parse(parameter_indices[0], hs_syntax_get(parameter_indices[1])->type))
			{
				success = TRUE;
			}
		}
		else if (!hs_compile_globals.error
			&& hs_parse(parameter_indices[0], _hs_type_real)
			&& hs_parse(parameter_indices[1], _hs_type_real))
		{
			success = TRUE;
		}
	}

	return success;
}

#define HS_TYPE_IS_NUMBER(type) ((type)>=_hs_type_real && (type)<=_hs_type_long_integer) /* fake name */

boolean hs_parse_inequality(
	short function_index,
	long expression_index)
{
	long parameter_indices[2];
	boolean success = FALSE;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 483, function_index>=_hs_function_gt && function_index<=_hs_function_lte);

	if (hs_get_parameter_indices(
		hs_function_get(function_index)->name,
		NUMBEROF(parameter_indices),
		parameter_indices,
		expression_index))
	{
		if (hs_parse(parameter_indices[0], _hs_unparsed)
			&& (HS_TYPE_IS_ENUM(hs_syntax_get(parameter_indices[0])->type) || HS_TYPE_IS_NUMBER(hs_syntax_get(parameter_indices[0])->type)))
		{
			if (hs_parse(parameter_indices[1], hs_syntax_get(parameter_indices[0])->type))
			{
				success = TRUE;
			}
		}
		else if (!hs_compile_globals.error
			&& hs_parse(parameter_indices[1], _hs_unparsed)
			&& (HS_TYPE_IS_ENUM(hs_syntax_get(parameter_indices[1])->type) || HS_TYPE_IS_NUMBER(hs_syntax_get(parameter_indices[1])->type)))
		{
			if (hs_parse(parameter_indices[0], hs_syntax_get(parameter_indices[1])->type))
			{
				success = TRUE;
			}
		}
		else if (!hs_compile_globals.error
			&& hs_parse(parameter_indices[0], _hs_type_real)
			&& hs_parse(parameter_indices[1], _hs_type_real))
		{
			success = TRUE;
		}
	}

	return success;
}

boolean hs_parse_sleep(
	short function_index,
	long expression_index)
{
	boolean success = FALSE;
	long time_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 526, function_index==_hs_function_sleep);

	if (time_index!=NONE)
	{
		if (hs_parse(time_index, _hs_type_short_integer))
		{
			long script_index = hs_syntax_get(time_index)->next_node_index;

			if (script_index!=NONE)
			{
				if (hs_parse(script_index, _hs_type_script))
				{
					success = TRUE;
				}
			}
			else
			{
				success = TRUE;
			}
		}
	}
	else
	{
		hs_compile_globals.error = "the sleep call requires a time and, optionally, a script name.";
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
	}

	return success;
}

boolean hs_parse_sleep_until(
	short function_index,
	long expression_index)
{
	long condition_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 565, function_index==_hs_function_sleep_until);

	condition_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	if (condition_index!=NONE)
	{
		long period_index = hs_syntax_get(condition_index)->next_node_index;

		success = hs_parse(condition_index, _hs_type_boolean);

		if (success && period_index!=NONE)
		{
			long timeout_index = hs_syntax_get(period_index)->next_node_index;

			success = hs_parse(period_index, _hs_type_short_integer);

			if (success && timeout_index!=NONE)
			{
				success = hs_parse(timeout_index, _hs_type_long_integer);
			}
		}
	}
	else
	{
		hs_compile_globals.error = "the sleep_until call requires a condition and, optionally, a period.";
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
	}

	return success;
}

boolean hs_parse_wake(
	short function_index,
	long expression_index)
{
	long script_name_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 605, function_index==_hs_function_wake);

	if (hs_get_parameter_indices(hs_function_get(function_index)->name, 1, &script_name_index, expression_index))
	{
		struct hs_syntax_node *script_name = hs_syntax_get(script_name_index);

		if (hs_parse(script_name_index, _hs_type_script))
		{
			struct hs_script *script = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->hs_scripts,
				*(short *)&script_name->data,
				struct hs_script);

			if (script->script_type!=_hs_script_static && script->script_type!=_hs_script_stub)
			{
				success = TRUE;
			}
			else
			{
				hs_compile_globals.error = "this static script cannot be awakened.";
				hs_compile_globals.error_offset = script_name->source_offset;
			}
		}
	}

	return success;
}

boolean hs_parse_inspect(
	short function_index,
	long expression_index)
{
	long argument_index;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	boolean success = FALSE;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 639, function_index==_hs_function_inspect);

	if (hs_get_parameter_indices(hs_function_get(function_index)->name, 1, &argument_index, expression_index))
	{
		struct hs_syntax_node *argument = hs_syntax_get(argument_index);

		if (hs_parse(argument_index, _hs_unparsed))
		{
			success = TRUE;
		}
		else if (!hs_compile_globals.error)
		{
			hs_compile_globals.error = "this is not a global variable reference, function call, or script call.";
			hs_compile_globals.error_offset = argument->source_offset;
		}
	}

	return success;
}

boolean hs_parse_object_cast_up(
	short function_index,
	long expression_index)
{
	long object_reference_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 666, function_index>=_hs_function_object_to_unit && function_index<=_hs_function_object_to_unit);

	if (hs_get_parameter_indices(hs_function_get(function_index)->name, 1, &object_reference_index, expression_index))
	{
		success = hs_parse(object_reference_index, _hs_type_object);
	}

	return success;
}

boolean hs_parse_debug_string(
	short function_index,
	long expression_index)
{
	boolean success = TRUE;
	long argument_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_compile.h", 686, (function_index>=_hs_function_debug_string__first) && (function_index<=_hs_function_debug_string__last));

	while (success && argument_index!=NONE)
	{
		success = hs_parse(argument_index, _hs_type_string);
		argument_index = hs_syntax_get(argument_index)->next_node_index;
	}

	return success;
}

#endif // __HS_LIBRARY_INTERNAL_COMPILE_H
