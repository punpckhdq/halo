/*
HS_LIBRARY_INTERNAL.H

header included in hcex build.
*/

#ifndef __HS_LIBRARY_INTERNAL_H
#define __HS_LIBRARY_INTERNAL_H
#pragma once

/* ---------- headers */

#include "hs.h"

/* ---------- constants */

enum
{
	_hs_function_begin = 0,
	_hs_function_begin_random,
	_hs_function_if,
	_hs_function_cond,
	_hs_function_set,
	_hs_function_and,
	_hs_function_or,
	_hs_function_plus,
	_hs_function_minus,
	_hs_function_times,
	_hs_function_divide,
	_hs_function_min,
	_hs_function_max,
	_hs_function_equal,
	_hs_function_not_equal,
	_hs_function_gt,
	_hs_function_lt,
	_hs_function_gte,
	_hs_function_lte,
	_hs_function_sleep,
	_hs_function_sleep_until,
	_hs_function_wake,
	_hs_function_inspect,
	_hs_function_object_to_unit,
	_hs_function_debug_string__ai_debug_communication_suppress,
	_hs_function_debug_string__ai_debug_communication_ignore,
	_hs_function_debug_string__ai_debug_communication_focus,
	_hs_function_debug_string__first = _hs_function_debug_string__ai_debug_communication_suppress,
	_hs_function_debug_string__last = _hs_function_debug_string__ai_debug_communication_focus
};

/* ---------- macros */

// the parse member of hs_function_definition and its initializer (with the trailing comma)
#ifdef HS_COMPILER_ENABLED
#define HS_FUNCTION_PARSE(parse) parse, /* fake name */
#else
#define HS_FUNCTION_PARSE(parse)
#endif

/* ---------- structures */

struct hs_function_definition
{
	short return_type;
	char *name;
#ifdef HS_COMPILER_ENABLED
	boolean (*parse)(short function_index, long expression_index);
#endif
	void (*evaluate)(short function_index, long thread_index, boolean initialize);
	char const *documentation;
	char const *parameters;
	short formal_parameter_count;
	short formal_parameters[];
};

struct hs_enum_definition
{
	short count;
	char const **identifiers;
};

/* ---------- prototypes/HS_LIBRARY_INTERNAL_COMPILE.H */

#ifdef HS_COMPILER_ENABLED

boolean hs_parse_begin(short function_index, long expression_index);
boolean hs_parse_if(short function_index, long expression_index);
boolean hs_parse_cond(short function_index, long expression_index);
boolean hs_parse_set(short function_index, long expression_index);
boolean hs_parse_logical(short function_index, long expression_index);
boolean hs_parse_arithmetic(short function_index, long expression_index);
boolean hs_parse_equality(short function_index, long expression_index);
boolean hs_parse_inequality(short function_index, long expression_index);
boolean hs_parse_sleep(short function_index, long expression_index);
boolean hs_parse_sleep_until(short function_index, long expression_index);
boolean hs_parse_wake(short function_index, long expression_index);
boolean hs_parse_inspect(short function_index, long expression_index);
boolean hs_parse_object_cast_up(short function_index, long expression_index);
boolean hs_parse_debug_string(short function_index, long expression_index);

#endif // HS_COMPILER_ENABLED

/* ---------- prototypes/HS_LIBRARY_INTERNAL_RUNTIME.H */

void hs_evaluate_begin(short function_index, long thread_index, boolean initialize);
void hs_evaluate_begin_random(short function_index, long thread_index, boolean initialize);
void hs_evaluate_if(short function_index, long thread_index, boolean initialize);
void hs_evaluate_set(short function_index, long thread_index, boolean initialize);
void hs_evaluate_logical(short function_index, long thread_index, boolean initialize);
void hs_evaluate_arithmetic(short function_index, long thread_index, boolean initialize);
void hs_evaluate_equality(short function_index, long thread_index, boolean initialize);
void hs_evaluate_inequality(short function_index, long thread_index, boolean initialize);
void hs_evaluate_sleep(short function_index, long thread_index, boolean initialize);
void hs_evaluate_sleep_until(short function_index, long thread_index, boolean initialize);
void hs_evaluate_wake(short function_index, long thread_index, boolean initialize);
void hs_evaluate_inspect(short function_index, long thread_index, boolean initialize);
void hs_evaluate_object_cast_up(short function_index, long thread_index, boolean initialize);
void hs_evaluate_debug_string(short function_index, long thread_index, boolean initialize);

/* ---------- globals */

extern struct hs_enum_definition const hs_enum_table[NUMBER_OF_HS_ENUM_TYPES];

#endif // __HS_LIBRARY_INTERNAL_H
