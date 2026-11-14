/*
HS_COMPILE.C
*/

/* ---------- headers */

#include "cseries.h"
#include <ctype.h>
#include "hs_library_internal.h"
#include "hs_scenario_definitions.h"
#include "ai.h"
#include "ai_constants.h"
#include "network_game_globals.h"
#include "players.h"
#include "input.h"
#include "encounters.h"
#include "hud_definitions.h"
#include "devices.h"
#include "ai_script.h"
#include "recorded_animation_definitions.h"
#include "hud_messaging_definitions.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "interface.h"

/* ---------- structures */

struct hs_tokenizer
{
	char *cursor;
};

/* ---------- prototypes */

static char *hs_compile_add_source(long source_size, char const *source_data);
static void hs_compile_finish(void);
static void hs_concatenate_expression(long expression_index);
static long hs_concatenate_string_constant(char const *string);
static long hs_find_string_constant(char const *string);
static long hs_tokenize(struct hs_tokenizer *state);
static void hs_tokenize_primitive(struct hs_tokenizer *state, long expression_index);
static void hs_tokenize_nonprimitive(struct hs_tokenizer *state, long expression_index);
static void skip_whitespace(char **c);
static boolean character_in_list(char c, short list_count, char const *list);
static boolean hs_parse_primitive(long expression_index);
static boolean hs_parse_variable(long expression_index);
static boolean hs_parse_nonprimitive(long expression_index);
static short string_list_find(char const *string, short list_count, char const **string_list);

/* ---------- globals */

static struct
{
	boolean initialized;
	long compiled_source_size;
	char *compiled_source;
	char *string_constant_buffer;
	long string_constant_buffer_offset;
	long string_constant_buffer_size;
	boolean error_since_initialize;
	char const *error;
	long error_offset;
	char error_buffer[256];
	boolean malloced;
	boolean permanent;
	boolean disallow_blocks;
	boolean disallow_sets;
	boolean variables_predetermined;
} hs_compile_globals;

/* ---------- public code */

#include "hs_library_internal_compile.h"

void hs_compile_initialize(
	boolean permanent)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 91, !hs_compile_globals.initialized);

	hs_compile_globals.initialized = TRUE;
	hs_compile_globals.compiled_source = NULL;
	hs_compile_globals.compiled_source_size = 0;
	hs_compile_globals.permanent = permanent;
	hs_compile_globals.error_since_initialize = FALSE;
	hs_compile_globals.error = NULL;

	if (permanent)
	{
		struct scenario *scenario = global_scenario_get();

		tag_block_resize(&scenario->hs_scripts, 0);
		tag_block_resize(&scenario->hs_globals, 0);
		tag_block_resize(&scenario->hs_references, 0);
		tag_data_resize(&scenario->hs_string_constants, 0);
		data_delete_all(hs_syntax_data);
	}

	return;
}

void hs_compile_dispose(
	void)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 117, hs_compile_globals.initialized);

	if (hs_compile_globals.permanent)
	{
		if (!hs_compile_globals.error_since_initialize)
		{
			hs_compile_finish();
		}
		else
		{
			struct scenario *scenario = global_scenario_get();

			tag_block_resize(&scenario->hs_scripts, 0);
			tag_block_resize(&scenario->hs_globals, 0);
			tag_data_resize(&scenario->hs_string_constants, 0);
			data_delete_all(hs_syntax_data);
		}

		if (hs_compile_globals.compiled_source)
		{
			match_free("c:\\halo\\SOURCE\\hs\\hs_compile.c", 135, hs_compile_globals.compiled_source);
		}
	}

	if (hs_compile_globals.malloced)
	{
		match_free("c:\\halo\\SOURCE\\hs\\hs_compile.c", 140, hs_compile_globals.compiled_source);
		hs_compile_globals.compiled_source = NULL;
		hs_compile_globals.malloced = FALSE;
	}

	hs_compile_globals.initialized = FALSE;

	return;
}

long hs_compile_expression(
	long source_size,
	char const *source_data,
	char const **error_message_pointer,
	char const **error_source_pointer)
{
	long result = NONE;

	if (source_size<HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES)
	{
		struct hs_tokenizer tokenizer;
		long source_offset;

		if (global_scenario_index!=NONE)
		{
			match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 166, global_scenario_get()->hs_string_constants.size>=HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES);
			source_offset = global_scenario_get()->hs_string_constants.size - HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES;
			hs_compile_globals.compiled_source = global_scenario_get()->hs_string_constants.address;
		}
		else
		{
			source_offset = 0;
			hs_compile_globals.compiled_source = match_malloc("c:\\halo\\SOURCE\\hs\\hs_compile.c", 175, source_size + 1);
			hs_compile_globals.malloced = TRUE;
			match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 178, hs_compile_globals.compiled_source);
		}

		memcpy(hs_compile_globals.compiled_source + source_offset, source_data, source_size);
		hs_compile_globals.compiled_source_size = source_offset + source_size;
		hs_compile_globals.compiled_source[hs_compile_globals.compiled_source_size] = '\0';
		tokenizer.cursor = hs_compile_globals.compiled_source + source_offset;
		hs_compile_globals.error = NULL;
		*error_message_pointer = NULL;
		*error_source_pointer = NULL;
		hs_compile_globals.error_offset = NONE;
		skip_whitespace(&tokenizer.cursor);

		if (*tokenizer.cursor)
		{
			long expression_index = hs_tokenize(&tokenizer);

			if (!hs_compile_globals.error)
			{
				long root_expression_index = datum_new(hs_syntax_data);
				long function_name_index = datum_new(hs_syntax_data);

				if (root_expression_index!=NONE && function_name_index!=NONE)
				{
					struct hs_syntax_node *root_expression = hs_syntax_get(root_expression_index);
					struct hs_syntax_node *function_name = hs_syntax_get(function_name_index);

					root_expression->data = function_name_index;
					root_expression->next_node_index = NONE;
					root_expression->source_offset = hs_syntax_get(expression_index)->source_offset;
					root_expression->flags = 0;
					function_name->next_node_index = expression_index;
					function_name->source_offset = NONE;
					function_name->function_index = _hs_function_inspect;
					function_name->flags = FLAG(_hs_syntax_node_primitive_bit);
					function_name->type = _hs_function_name;

					if (hs_parse(root_expression_index, _hs_type_void))
					{
						result = root_expression_index;
					}
				}
			}

			if (result==NONE)
			{
				*error_message_pointer = hs_compile_globals.error;

				if (hs_compile_globals.error_offset!=NONE)
				{
					hs_compile_globals.error_offset -= source_offset;
					match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 235, hs_compile_globals.error_offset>=0 && hs_compile_globals.error_offset<source_size);
					*error_source_pointer = source_data + hs_compile_globals.error_offset;
				}
			}
		}
	}

	return result;
}

static char *hs_compile_add_source(
	long source_size,
	char const *source_data)
{
	char *result = NULL;
	char *compiled_source = match_realloc("c:\\halo\\SOURCE\\hs\\hs_compile.c", 253, hs_compile_globals.compiled_source, hs_compile_globals.compiled_source_size + source_size + 1);

	if (compiled_source)
	{
		hs_compile_globals.compiled_source = compiled_source;
		result = compiled_source + hs_compile_globals.compiled_source_size;
		memcpy(result, source_data, source_size);
		hs_compile_globals.compiled_source_size += source_size;
		hs_compile_globals.compiled_source[hs_compile_globals.compiled_source_size] = '\0';
	}

	return result;
}

void hs_compile(
	long source_file_size,
	char const *source_file_data,
	char const **error_message_pointer,
	char const **error_source_pointer)
{
	struct hs_tokenizer tokenizer;

	tokenizer.cursor = hs_compile_add_source(source_file_size, source_file_data);

	if (tokenizer.cursor)
	{
		boolean success = TRUE;

		hs_compile_globals.error = NULL;
		*error_message_pointer = NULL;
		*error_source_pointer = NULL;
		hs_compile_globals.error_offset = NONE;
		skip_whitespace(&tokenizer.cursor);

		while (success && *tokenizer.cursor)
		{
			long expression_index = hs_tokenize(&tokenizer);

			skip_whitespace(&tokenizer.cursor);
			success = !hs_compile_globals.error;

			if (success)
			{
				success = hs_parse(expression_index, _hs_special_form);
			}
		}

		if (!success)
		{
			match_vassert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 305, hs_compile_globals.error, "tell matt that somebody failed to correctly report a parsing error.");
			*error_message_pointer = hs_compile_globals.error;
			hs_compile_globals.error_since_initialize = TRUE;

			if (hs_compile_globals.error_offset!=NONE)
			{
				hs_compile_globals.error_offset -= hs_compile_globals.compiled_source_size - source_file_size;
				match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 315, hs_compile_globals.error_offset>=0 && hs_compile_globals.error_offset<source_file_size);
				*error_source_pointer = source_file_data + hs_compile_globals.error_offset;
			}
		}
	}
	else
	{
		*error_message_pointer = "couldn't allocate memory for compiled source.";
	}

	return;
}

static void hs_compile_finish(
	void)
{
	struct scenario *scenario = global_scenario_get();
	boolean success = tag_data_resize(&scenario->hs_string_constants, hs_compile_globals.compiled_source_size);

	if (success)
	{
		short global_index;
		short script_index;

		hs_compile_globals.string_constant_buffer = scenario->hs_string_constants.address;
		hs_compile_globals.string_constant_buffer_offset = 0;
		hs_compile_globals.string_constant_buffer_size = hs_compile_globals.compiled_source_size;

		for (global_index = 0; global_index<scenario->hs_globals.count; global_index++)
		{
			hs_concatenate_expression(TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->hs_globals,
				global_index,
				struct hs_global_internal)->initialization_expression_index);
		}

		for (script_index = 0; script_index<scenario->hs_scripts.count; script_index++)
		{
			hs_concatenate_expression(TAG_BLOCK_GET_ELEMENT(
				&scenario->hs_scripts,
				script_index,
				struct hs_script)->root_expression_index);
		}

		hs_node_gc();
		success = tag_data_resize(
			&scenario->hs_string_constants,
			hs_compile_globals.string_constant_buffer_offset + HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES);
	}

	match_vassert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 365, success, "increase MAXIMUM_HS_STRING_DATA_PER_SCENARIO");

	return;
}

boolean hs_verify_source_offset(
	long source_offset)
{
	boolean result = TRUE;

	if (source_offset<0 || source_offset>=hs_compile_globals.compiled_source_size)
	{
		hs_compile_globals.error = "bad source offset (you need to recompile.)";
		result = FALSE;
	}

	return result;
}

boolean hs_compile_postprocess(
	char const **error_message_pointer,
	char const **error_source_pointer)
{
	struct hs_script *script;
	long expression_index;
	short return_type;
	boolean success = TRUE;

	hs_compile_globals.compiled_source = global_scenario_get()->hs_string_constants.address;
	hs_compile_globals.compiled_source_size = global_scenario_get()->hs_string_constants.size - HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES;
	hs_compile_globals.error = NULL;
	hs_compile_globals.variables_predetermined = TRUE;
	*error_message_pointer = NULL;
	*error_source_pointer = NULL;

	for (expression_index = data_next_index(hs_syntax_data, NONE);
		success && expression_index!=NONE;
		expression_index = data_next_index(hs_syntax_data, expression_index))
	{
		struct hs_syntax_node *expression = hs_syntax_get(expression_index);

		if (hs_type_valid(expression->type))
		{
			if (TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_primitive_bit))
			{
				if (expression->type>=_hs_type_string || TEST_FLAG(expression->flags, _hs_syntax_node_variable_bit))
				{
					success = hs_verify_source_offset(expression->source_offset);

					if (success)
					{
						success = hs_parse_primitive(expression_index);
					}
				}

				if (success && TEST_FLAG(expression->flags, _hs_syntax_node_variable_bit))
				{
					return_type = hs_global_get_type((short)expression->data);
				}
				else
				{
					return_type = expression->constant_type;
				}
			}
			else if (TEST_FLAG(expression->flags, _hs_syntax_node_script_bit))
			{
				if (expression->script_index>=0
					&& expression->script_index<global_scenario_get()->hs_scripts.count
					&& (script = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->hs_scripts, expression->script_index, struct hs_script))->script_type==_hs_script_static
					|| script->script_type==_hs_script_stub)
				{
					return_type = script->return_type;
				}
				else
				{
					hs_compile_globals.error = "bad script index (you need to recompile.)";
					success = FALSE;
				}
			}
			else
			{
				long function_name_index = hs_syntax_get(expression_index)->data;

				if (function_name_index!=NONE)
				{
					struct hs_syntax_node *function_name = hs_syntax_get(function_name_index);

					if (function_name->type==_hs_function_name)
					{
						if (hs_verify_source_offset(function_name->source_offset))
						{
							short function_index = hs_find_function_by_name(
								hs_compile_globals.compiled_source + function_name->source_offset);

							if (function_index!=NONE)
							{
								expression->function_index = function_index;
								hs_function_get(expression->function_index);
								return_type = hs_function_get(expression->function_index)->return_type;
							}
							else
							{
								hs_compile_globals.error = "missing function (you need to recompile scripts.)";
								success = FALSE;
							}
						}
						else
						{
							success = FALSE;
						}
					}
					else
					{
						hs_compile_globals.error = "corrupt syntax tree (you need to recompile scripts.)";
						success = FALSE;
					}
				}
				else
				{
					hs_compile_globals.error = "corrupt syntax tree (you need to recompile scripts.)";
					success = FALSE;
				}
			}

			if (success)
			{
				if ((hs_type_valid(return_type) || return_type==_hs_passthrough)
					&& hs_can_cast(return_type, expression->type))
				{
					success = TRUE;
				}
				else
				{
					hs_compile_globals.error = "type is inconsistent with usage (you need to recompile scripts.)";
					success = FALSE;
				}
			}
		}
		else if (expression->type!=_hs_function_name)
		{
			hs_compile_globals.error = "missing type (you need to recompile scripts.)";
			success = FALSE;
		}
	}

	if (!success)
	{
		*error_message_pointer = hs_compile_globals.error;

		if (hs_compile_globals.error_offset!=NONE)
		{
			*error_source_pointer = hs_compile_globals.compiled_source + hs_compile_globals.error_offset;
		}
	}

	hs_compile_globals.compiled_source = NULL;
	hs_compile_globals.error = NULL;
	hs_compile_globals.variables_predetermined = FALSE;

	return success;
}

static void hs_concatenate_expression(
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	SET_FLAG(expression->flags, _hs_syntax_node_permanent_bit, TRUE);

	if (TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_primitive_bit))
	{
		if (expression->type==_hs_function_name)
		{
			if (expression->source_offset!=NONE)
			{
				expression->source_offset = hs_concatenate_string_constant(
					hs_compile_globals.compiled_source + expression->source_offset);
			}
			else
			{
				expression->source_offset = hs_concatenate_string_constant(
					hs_function_get(expression->function_index)->name);
			}
		}
		else if (TEST_FLAG(expression->flags, _hs_syntax_node_variable_bit) || expression->type>=_hs_type_string)
		{
			expression->source_offset = hs_concatenate_string_constant(
				hs_compile_globals.compiled_source + expression->source_offset);
		}
	}
	else
	{
		long argument_index;

		for (argument_index = expression->data;
			argument_index!=NONE;
			argument_index = hs_syntax_get(argument_index)->next_node_index)
		{
			hs_concatenate_expression(argument_index);
		}
	}

	return;
}

static long hs_concatenate_string_constant(
	char const *string)
{
	long offset = hs_find_string_constant(string);

	if (offset==NONE)
	{
		short length = strlen(string) + 1;

		if (length<hs_compile_globals.string_constant_buffer_size)
		{
			memcpy(
				hs_compile_globals.string_constant_buffer + hs_compile_globals.string_constant_buffer_offset,
				string,
				length);
			offset = hs_compile_globals.string_constant_buffer_offset;
			hs_compile_globals.string_constant_buffer_offset += length;
			hs_compile_globals.string_constant_buffer_size -= length;
		}
		else
		{
			match_vassert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 612, FALSE, "ran out of script string constant memory");
		}
	}

	return offset;
}

static long hs_find_string_constant(
	char const *string)
{
	char const *string_constant;
	long offset;

	for (offset = 0; offset<hs_compile_globals.string_constant_buffer_offset; offset += strlen(string_constant) + 1)
	{
		string_constant = hs_compile_globals.string_constant_buffer + offset;

		if (!strcmp(string, string_constant))
		{
			return offset;
		}
	}

	return NONE;
}

static boolean hs_add_global(
	long expression_index)
{
	long name_index;
	long initialization_index;
	boolean success = FALSE;
	long type_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	if (type_index==NONE ||
		(name_index = hs_syntax_get(type_index)->next_node_index)==NONE ||
		(initialization_index = hs_syntax_get(name_index)->next_node_index)==NONE ||
		hs_syntax_get(initialization_index)->next_node_index!=NONE)
	{
		hs_compile_globals.error = "i expected (global<type> <name> <initial value>)";
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
	}
	else
	{
		short type = string_list_find(
			hs_compile_globals.compiled_source + hs_syntax_get(type_index)->source_offset,
			NUMBER_OF_HS_NODE_TYPES,
			hs_type_names);

		if (hs_type_valid(type))
		{
			char const *name = hs_compile_globals.compiled_source + hs_syntax_get(name_index)->source_offset;

			if (strlen(name)>0 && strlen(name)<=TAG_STRING_LENGTH)
			{
				if (hs_find_global_by_name(name)==NONE)
				{
					hs_compile_globals.disallow_blocks = TRUE;
					hs_compile_globals.disallow_sets = TRUE;

					if (hs_parse(initialization_index, type))
					{
						short global_index = tag_block_add_element(&global_scenario_get()->hs_globals);

						if (global_index!=NONE)
						{
							struct hs_global_internal *global = TAG_BLOCK_GET_ELEMENT(
								&global_scenario_get()->hs_globals,
								global_index,
								struct hs_global_internal);

							strcpy(global->name, name);
							global->type = type;
							global->initialization_expression_index = initialization_index;
							success = TRUE;
						}
						else
						{
							hs_compile_globals.error = "i couldn't allocate space for this global.";
							hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
						}
					}

					hs_compile_globals.disallow_blocks = FALSE;
					hs_compile_globals.disallow_sets = FALSE;
				}
				else
				{
					hs_compile_globals.error = "there is already a variable by this name.";
					hs_compile_globals.error_offset = hs_syntax_get(name_index)->source_offset;
				}
			}
			else
			{
				hs_compile_globals.error = "i expected a global variable name less than 32 characters.";
				hs_compile_globals.error_offset = hs_syntax_get(name_index)->source_offset;
			}
		}
		else
		{
			hs_compile_globals.error = "this is not a valid type.";
			hs_compile_globals.error_offset = hs_syntax_get(type_index)->source_offset;
		}
	}

	return success;
}

static boolean hs_add_script(
	long expression_index)
{
	boolean success = FALSE;
	long type_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	if (type_index!=NONE)
	{
		struct hs_syntax_node *type_expression = hs_syntax_get(type_index);
		short script_type = string_list_find(
			hs_compile_globals.compiled_source + type_expression->source_offset,
			NUMBER_OF_HS_SCRIPT_TYPES,
			hs_script_type_names);

		if (script_type!=NONE)
		{
			long body_index;
			short return_type;
			long name_index = NONE;

			if (script_type==_hs_script_static || script_type==_hs_script_stub)
			{
				long return_type_index = hs_syntax_get(type_index)->next_node_index;

				if (return_type_index!=NONE)
				{
					struct hs_syntax_node *return_type_expression = hs_syntax_get(return_type_index);

					return_type = string_list_find(
						hs_compile_globals.compiled_source + return_type_expression->source_offset,
						NUMBER_OF_HS_NODE_TYPES,
						hs_type_names);
					name_index = hs_syntax_get(return_type_index)->next_node_index;

					if (hs_type_valid(return_type))
					{
						success = TRUE;
					}
					else
					{
						hs_compile_globals.error = "this is not a valid return type.";
						hs_compile_globals.error_offset = hs_syntax_get(return_type_index)->source_offset;
					}
				}
				else
				{
					hs_compile_globals.error = "i expected (script local <type> <name> <expression(s)>).";
					hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
				}
			}
			else
			{
				return_type = _hs_type_void;
				name_index = hs_syntax_get(type_index)->next_node_index;
				success = TRUE;
			}

			if (success)
			{
				success = FALSE;

				if (name_index==NONE || (body_index = hs_syntax_get(name_index)->next_node_index)==NONE)
				{
					if (script_type==_hs_script_static)
					{
						hs_compile_globals.error = "i expected (script static <type> <name> <expression(s)>)";
					}
					else if (script_type==_hs_script_stub)
					{
						hs_compile_globals.error = "i expected (script stub <type> <name> <expression(s)>)";
					}
					else
					{
						hs_compile_globals.error = "i expected (script <type> <name> <expression(s)>)";
					}

					hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
				}
				else
				{
					char const *name = hs_compile_globals.compiled_source + hs_syntax_get(name_index)->source_offset;

					if (strlen(name)>0 && strlen(name)<=TAG_STRING_LENGTH)
					{
						struct scenario *scenario = global_scenario_get();
						long script_index = hs_find_script_by_name(name);

						if (script_index==NONE)
						{
							script_index = tag_block_add_element(&scenario->hs_scripts);

							if (script_index==NONE)
							{
								hs_compile_globals.error = "i couldn't allocate a script.";
								hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
							}
						}
						else
						{
							struct hs_script *script = TAG_BLOCK_GET_ELEMENT(
								&scenario->hs_scripts,
								script_index,
								struct hs_script);

							if (script->script_type!=_hs_script_stub
								|| script->return_type!=return_type
								|| script_type!=_hs_script_static)
							{
								if (script->script_type==_hs_script_static
									&& script->return_type==return_type
									&& script_type==_hs_script_stub)
								{
									script_index = NONE;
									success = TRUE;
								}
								else
								{
									script_index = NONE;
									hs_compile_globals.error = "only static scripts of the same type can override stub scripts.";
									hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
								}
							}
						}

						if (script_index!=NONE)
						{
							struct hs_script *script = TAG_BLOCK_GET_ELEMENT(
								&scenario->hs_scripts,
								script_index,
								struct hs_script);
							long root_expression_index = datum_new(hs_syntax_data);
							long function_name_index = datum_new(hs_syntax_data);

							if (root_expression_index==NONE || function_name_index==NONE)
							{
								hs_compile_globals.error = "i couldn't allocate a syntax node.";
							}
							else
							{
								struct hs_syntax_node *root_expression = hs_syntax_get(root_expression_index);
								struct hs_syntax_node *function_name = hs_syntax_get(function_name_index);

								root_expression->data = function_name_index;
								root_expression->next_node_index = NONE;
								root_expression->source_offset = hs_syntax_get(expression_index)->source_offset;
								root_expression->flags = 0;
								function_name->next_node_index = body_index;
								function_name->source_offset = NONE;
								function_name->function_index = _hs_function_begin;
								function_name->flags = FLAG(_hs_syntax_node_primitive_bit);
								function_name->type = _hs_function_name;

								if (hs_parse(root_expression_index, return_type))
								{
									strcpy(script->name, name);
									script->script_type = script_type;
									script->return_type = return_type;
									script->root_expression_index = root_expression_index;
									success = TRUE;
								}
							}
						}
					}
					else
					{
						hs_compile_globals.error = "i expected a script name less than 32 characters.";
						hs_compile_globals.error_offset = hs_syntax_get(name_index)->source_offset;
					}
				}
			}
		}
		else
		{
			hs_compile_globals.error = "script type must be \"startup\", \"dormant\", \"continuous\", or \"static\".";
			hs_compile_globals.error_offset = hs_syntax_get(type_index)->source_offset;
		}
	}
	else
	{
		hs_compile_globals.error = "i expected (script <type> <name> <expression(s)>)";
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
	}

	return success;
}

enum
{
	_begin_delimiter = '(',
	_end_delimiter = ')',
	_string_delimiter = '"',
	_comment_delimiter = ';',
	_block_comment_delimiter = '*'
};

char const whitespace_characters[] = {' ', '\t'};
char const eol_characters[] = {'\n', '\r'};

static long hs_tokenize(
	struct hs_tokenizer *state)
{
	long expression_index = datum_new(hs_syntax_data);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 926, !hs_compile_globals.error);

	if (expression_index!=NONE)
	{
		struct hs_syntax_node *expression = hs_syntax_get(expression_index);

		expression->flags = 0;
		expression->type = _hs_unparsed;
		expression->constant_type = NONE;
		expression->next_node_index = NONE;
		SET_FLAG(expression->flags, _hs_syntax_node_primitive_bit, *state->cursor!=_begin_delimiter);

		if (TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_primitive_bit))
		{
			hs_tokenize_primitive(state, expression_index);
		}
		else
		{
			hs_tokenize_nonprimitive(state, expression_index);
		}
	}
	else
	{
		hs_compile_globals.error = "i couldn't allocate a syntax node.";
	}

	return expression_index;
}

static void hs_tokenize_primitive(
	struct hs_tokenizer *state,
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	if (*state->cursor==_string_delimiter)
	{
		state->cursor++;
		expression->source_offset = state->cursor - hs_compile_globals.compiled_source;

		while (*state->cursor && *state->cursor!=_string_delimiter)
		{
			state->cursor++;
		}

		if (!*state->cursor)
		{
			hs_compile_globals.error = "this quoted constant is unterminated.";
			hs_compile_globals.error_offset = expression->source_offset - 1;
		}

		*state->cursor++ = '\0';
	}
	else
	{
		expression->source_offset = state->cursor - hs_compile_globals.compiled_source;

		while (*state->cursor &&
			*state->cursor!=_end_delimiter &&
			*state->cursor!=_comment_delimiter &&
			!character_in_list(*state->cursor, NUMBEROF(whitespace_characters), whitespace_characters) &&
			!character_in_list(*state->cursor, NUMBEROF(eol_characters), eol_characters))
		{
			state->cursor++;
		}
	}

	strlwr(hs_compile_globals.compiled_source + expression->source_offset);

	return;
}

static void hs_tokenize_nonprimitive(
	struct hs_tokenizer *state,
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	long *next_node_index_reference = (long *)&expression->data;

	expression->source_offset = state->cursor - hs_compile_globals.compiled_source;
	state->cursor++;

	while (!hs_compile_globals.error)
	{
		char *cursor = state->cursor;

		skip_whitespace(&state->cursor);

		if (state->cursor!=cursor)
		{
			*cursor = '\0';
		}

		if (!*state->cursor)
		{
			hs_compile_globals.error = "this left parenthesis is unmatched.";
			hs_compile_globals.error_offset = expression->source_offset;
		}
		else if (*state->cursor==_end_delimiter)
		{
			*state->cursor++ = '\0';
			break;
		}
		else
		{
			long argument_index = hs_tokenize(state);

			*next_node_index_reference = argument_index;

			if (argument_index!=NONE)
			{
				next_node_index_reference = &hs_syntax_get(argument_index)->next_node_index;
			}
		}
	}

	if (next_node_index_reference==(long *)&expression->data && !hs_compile_globals.error)
	{
		hs_compile_globals.error = "this expression is empty.";
		hs_compile_globals.error_offset = expression->source_offset;
	}

	return;
}

enum
{
	_skip_whitespace_state_no_comment = 0,
	_skip_whitespace_state_comment,
	_skip_whitespace_state_block_comment,
	_skip_whitespace_state_done,
	NUMBER_OF_SKIP_WHITESPACE_STATES
};

static void skip_whitespace(
	char **c)
{
	short state = _skip_whitespace_state_no_comment;

	while (state!=_skip_whitespace_state_done)
	{
		switch (state)
		{
		case _skip_whitespace_state_no_comment:
			if (**c==_comment_delimiter)
			{
				state = _skip_whitespace_state_comment;

				if (*++*c==_block_comment_delimiter)
				{
					state = _skip_whitespace_state_block_comment;
					++*c;
				}
			}
			else if (character_in_list(**c, NUMBEROF(whitespace_characters), whitespace_characters)
				|| character_in_list(**c, NUMBEROF(eol_characters), eol_characters))
			{
				++*c;
			}
			else
			{
				state = _skip_whitespace_state_done;
			}
			break;
		case _skip_whitespace_state_comment:
			if (**c)
			{
				if (character_in_list(**c, NUMBEROF(eol_characters), eol_characters))
				{
					state = _skip_whitespace_state_no_comment;
				}

				++*c;
			}
			else
			{
				state = _skip_whitespace_state_done;
			}
			break;
		case _skip_whitespace_state_block_comment:
			if (**c)
			{
				if (**c==_block_comment_delimiter && (*c)[1]==_comment_delimiter)
				{
					state = _skip_whitespace_state_no_comment;
					++*c;
				}

				++*c;
			}
			else
			{
				state = _skip_whitespace_state_done;
				hs_compile_globals.error = "unterminated comment.";
			}
			break;
		default:
			match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1132, !"unreachable");
		}
	}

	return;
}

static boolean character_in_list(
	char c,
	short list_count,
	char const *list)
{
	short index;

	for (index = 0; index<list_count; index++)
	{
		if (c==list[index])
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean hs_parse(
	long expression_index,
	short expected_type)
{
	boolean success = TRUE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1166, !hs_compile_globals.error);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1169, hs_type_valid(expected_type) || expected_type==_hs_special_form || expected_type==_hs_unparsed);

	if (expression->type==_hs_unparsed)
	{
		expression->type = expected_type;

		if (TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_primitive_bit))
		{
			expression->constant_type = expected_type;
			success = hs_parse_primitive(expression_index);
		}
		else
		{
			success = hs_parse_nonprimitive(expression_index);
		}
	}

	return success;
}

static boolean hs_parse_primitive(
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1199, hs_type_valid(expression->type) || expression->type==_hs_special_form || expression->type==_hs_unparsed);

	if (expression->type==_hs_special_form)
	{
		hs_compile_globals.error = "i expected a script or variable definition.";
		hs_compile_globals.error_offset = expression->source_offset;
	}
	else if (expression->type==_hs_type_void)
	{
		hs_compile_globals.error = "the value of this expression (in a <void> slot) can never be used.";
		hs_compile_globals.error_offset = expression->source_offset;
	}
	else
	{
		if (!hs_compile_globals.variables_predetermined || TEST_FLAG(expression->flags, _hs_syntax_node_variable_bit))
		{
			success = hs_parse_variable(expression_index);
		}

		if (!success
			&& expression->type
			&& !hs_compile_globals.error
			&& (!hs_compile_globals.variables_predetermined || !TEST_FLAG(expression->flags, _hs_syntax_node_variable_bit)))
		{
			if (hs_type_primitive_parsers[expression->type])
			{
				success = hs_type_primitive_parsers[expression->type](expression_index);
			}
			else
			{
				sprintf(
					hs_compile_globals.error_buffer,
					"expressions of type %s are currently unsupported.",
					hs_type_names[expression->type]);
				hs_compile_globals.error = hs_compile_globals.error_buffer;
				hs_compile_globals.error_offset = expression->source_offset;
				success = FALSE;
			}
		}
	}

	return success;
}

static boolean hs_parse_variable(
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1252, hs_type_valid(expression->type) || expression->type==_hs_unparsed);

	expression->data = hs_find_global_by_name(hs_compile_globals.compiled_source + expression->source_offset);

	if (expression->data!=NONE)
	{
		short type = hs_global_get_type((short)expression->data);

		if (expression->type && !hs_can_cast(type, expression->type))
		{
			sprintf(
				hs_compile_globals.error_buffer,
				"i expected a value of type %s, but the variable %s has type %s",
				hs_type_names[expression->type],
				hs_global_get_name((short)expression->data),
				hs_type_names[type]);
			hs_compile_globals.error = hs_compile_globals.error_buffer;
			hs_compile_globals.error_offset = expression->source_offset;
		}
		else
		{
			if (!expression->type)
			{
				expression->type = type;
			}

			SET_FLAG(expression->flags, _hs_syntax_node_variable_bit, TRUE);
			success = TRUE;
		}
	}
	else if (hs_compile_globals.variables_predetermined)
	{
		hs_compile_globals.error = "this is not a valid variable name.";
		hs_compile_globals.error_offset = expression->source_offset;
		success = FALSE;
	}

	return success;
}

static void hs_parse_call_predicate(
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	long predicate_index = hs_syntax_get(expression_index)->data;
	struct hs_syntax_node *predicate = hs_syntax_get(predicate_index);

	if (predicate->type!=_hs_function_name)
	{
		expression->function_index = hs_find_function_by_name(
			hs_compile_globals.compiled_source + predicate->source_offset);
		predicate->type = _hs_function_name;

		if (expression->function_index==NONE)
		{
			expression->script_index = hs_find_script_by_name(
				hs_compile_globals.compiled_source + predicate->source_offset);

			if (expression->script_index!=NONE)
			{
				SET_FLAG(expression->flags, _hs_syntax_node_script_bit, TRUE);
			}
		}

		predicate->function_index = expression->function_index;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1312, predicate->function_index!=NONE);
		expression->function_index = predicate->function_index;
	}

	return;
}

static boolean hs_parse_nonprimitive(
	long expression_index)
{
	boolean success = FALSE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	long predicate_index = hs_syntax_get(expression_index)->data;
	struct hs_syntax_node *predicate = hs_syntax_get(predicate_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1332, hs_type_valid(expression->type) || expression->type==_hs_special_form || expression->type==_hs_unparsed);

	if (TEST_FLAG(hs_syntax_get(predicate_index)->flags, _hs_syntax_node_primitive_bit))
	{
		if (expression->type==_hs_special_form)
		{
			if (!strcmp(hs_compile_globals.compiled_source + predicate->source_offset, "global"))
			{
				success = hs_add_global(expression_index);
			}
			else if (!strcmp(hs_compile_globals.compiled_source + predicate->source_offset, "script"))
			{
				success = hs_add_script(expression_index);
			}
			else
			{
				hs_compile_globals.error = "i expected \"script\" or \"global\".";
				hs_compile_globals.error_offset = predicate->source_offset;
			}
		}
		else
		{
			hs_parse_call_predicate(expression_index);

			if (expression->function_index!=NONE)
			{
				if (TEST_FLAG(expression->flags, _hs_syntax_node_script_bit))
				{
					struct hs_script *script = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->hs_scripts,
						expression->script_index,
						struct hs_script);

					if (script->script_type==_hs_script_static || script->script_type==_hs_script_stub)
					{
						if (expression->type && !hs_can_cast(script->return_type, expression->type))
						{
							sprintf(
								hs_compile_globals.error_buffer,
								"i expected a %s, but this script returns a %s.",
								hs_type_names[expression->type],
								hs_type_names[script->return_type]);
							hs_compile_globals.error = hs_compile_globals.error_buffer;
							hs_compile_globals.error_offset = expression->source_offset;
						}
						else
						{
							if (!expression->type)
							{
								expression->type = script->return_type;
							}

							success = TRUE;
						}
					}
					else
					{
						hs_compile_globals.error = "this is not a static script.";
						hs_compile_globals.error_offset = expression->source_offset;
					}
				}
				else
				{
					struct hs_function_definition const *function = hs_function_get(expression->function_index);

					if (expression->type && !hs_can_cast(function->return_type, expression->type))
					{
						sprintf(
							hs_compile_globals.error_buffer,
							"i expected a %s, but this function returns a %s.",
							hs_type_names[expression->type],
							hs_type_names[function->return_type]);
						hs_compile_globals.error = hs_compile_globals.error_buffer;
						hs_compile_globals.error_offset = expression->source_offset;
					}
					else if (hs_compile_globals.disallow_blocks
						&& (expression->function_index==_hs_function_sleep || expression->function_index==_hs_function_sleep_until))
					{
						hs_compile_globals.error = "it is illegal to block in this context.";
						hs_compile_globals.error_offset = expression->source_offset;
					}
					else if (hs_compile_globals.disallow_sets && expression->function_index==_hs_function_set)
					{
						hs_compile_globals.error = "it is illegal to set the value of variables in this context.";
						hs_compile_globals.error_offset = expression->source_offset;
					}
					else
					{
						if (!expression->type && function->return_type!=_hs_passthrough)
						{
							expression->type = function->return_type;
						}

						match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1420, function->parse);
						success = function->parse(expression->function_index, expression_index);
					}
				}
			}
			else
			{
				hs_compile_globals.error = "this is not a valid function or script name.";
				hs_compile_globals.error_offset = predicate->source_offset;
			}
		}
	}
	else
	{
		sprintf(
			hs_compile_globals.error_buffer,
			"i expected %s, but i got an expression.",
			expression->type==_hs_special_form ? "\"script\" or \"global\"" : "a function name");
		hs_compile_globals.error = hs_compile_globals.error_buffer;
		hs_compile_globals.error_offset = predicate->source_offset;
	}

	return success;
}

static boolean hs_parse_boolean(
	long expression_index)
{
	boolean result;
	boolean success = TRUE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	char const *source = hs_compile_globals.compiled_source + expression->source_offset;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1459, expression->type==_hs_type_boolean);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1460, expression->constant_type==expression->type);

	if (!strcmp(source, "false") || !strcmp(source, "off") || !strcmp(source, "0"))
	{
		result = FALSE;
	}
	else if (!strcmp(source, "true") || !strcmp(source, "on") || !strcmp(source, "1"))
	{
		result = TRUE;
	}
	else
	{
		hs_compile_globals.error = "i expected \"true\" or \"false\".";
		hs_compile_globals.error_offset = expression->source_offset;
		success = FALSE;
	}

	*(boolean *)&expression->data = result;

	return success;
}

static boolean hs_parse_real(
	long expression_index)
{
	boolean success = TRUE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	char const *c = hs_compile_globals.compiled_source + expression->source_offset;
	boolean decimal = FALSE;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1494, expression->type==_hs_type_real);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1495, expression->constant_type==expression->type);

	if (*c=='-')
	{
		c++;
	}

	for (; *c; c++)
	{
		if (!isdigit(*c))
		{
			if (decimal || *c!='.')
			{
				hs_compile_globals.error = "this is not a valid real number.";
				hs_compile_globals.error_offset = expression->source_offset;
				success = FALSE;
				break;
			}

			decimal = TRUE;
		}
	}

	*(real *)&expression->data = (real)atof(hs_compile_globals.compiled_source + expression->source_offset);

	return success;
}

static boolean hs_parse_integer(
	long expression_index)
{
	long value;
	boolean success = TRUE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	char const *c = hs_compile_globals.compiled_source + expression->source_offset;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1527, expression->type==_hs_type_short_integer || expression->type==_hs_type_long_integer);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1528, expression->constant_type==expression->type);

	if (*c=='-')
	{
		c++;
	}

	for (; *c; c++)
	{
		if (!isdigit(*c))
		{
			hs_compile_globals.error = "this is not a valid integer.";
			hs_compile_globals.error_offset = expression->source_offset;
			success = FALSE;
			break;
		}
	}

	value = atoi(hs_compile_globals.compiled_source + expression->source_offset);

	if (success && expression->type!=_hs_type_long_integer && (value>SHORT_MAX || value<SHORT_MIN))
	{
		hs_compile_globals.error = "shorts must be in the range [-32767, 32768].";
		hs_compile_globals.error_offset = expression->source_offset;
		success = FALSE;
	}

	if (expression->type==_hs_type_long_integer)
	{
		expression->data = value;
	}
	else
	{
		*(short *)&expression->data = (short)value;
	}

	return success;
}

static boolean hs_parse_string(
	long expression_index)
{
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1566, expression->type==_hs_type_string);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1567, expression->constant_type==expression->type);

	expression->data = (unsigned long)(hs_compile_globals.compiled_source + expression->source_offset);

	return TRUE;
}

static boolean hs_parse_script(
	long expression_index)
{
	short script_index;
	boolean success;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1581, expression->type==_hs_type_script);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1582, expression->constant_type==expression->type);

	script_index = hs_find_script_by_name(hs_compile_globals.compiled_source + expression->source_offset);

	if (script_index!=NONE)
	{
		*(short *)&expression->data = script_index;
		success = TRUE;
	}
	else
	{
		hs_compile_globals.error = "this is not a valid script name.";
		hs_compile_globals.error_offset = expression->source_offset;
		success = FALSE;
	}

	return success;
}

static boolean hs_parse_tag_reference(
	long expression_index)
{
	long group_tag;
	short reference_index;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	struct scenario *scenario = global_scenario_get();

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1694, HS_TYPE_IS_TAG_REFERENCE(expression->type));

	group_tag = hs_tag_reference_type_group_tags[expression->type - FIRST_HS_TAG_REFERENCE_TYPE];

	for (reference_index = 0; reference_index<scenario->hs_references.count; reference_index++)
	{
		struct hs_tag_reference *reference = TAG_BLOCK_GET_ELEMENT(
			&scenario->hs_references,
			reference_index,
			struct hs_tag_reference);

		if (!strcmp(reference->reference.name, hs_compile_globals.compiled_source + expression->source_offset)
			&& reference->reference.group_tag==group_tag)
		{
			expression->data = reference->reference.index;
			break;
		}
	}

	return TRUE;
}

static boolean hs_parse_enum(
	long expression_index)
{
	short enum_index;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	struct hs_enum_definition const *enum_definition = &hs_enum_table[expression->type - FIRST_HS_ENUM_TYPE];
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1724, HS_TYPE_IS_ENUM(expression->type));
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1725, expression->constant_type==expression->type);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1726, enum_definition->count);

	for (enum_index = 0; enum_index<enum_definition->count; enum_index++)
	{
		if (!_stricmp(
			hs_compile_globals.compiled_source + expression->source_offset,
			enum_definition->identifiers[enum_index]))
		{
			break;
		}
	}

	if (enum_index==enum_definition->count)
	{
		sprintf(hs_compile_globals.error_buffer, "%s must be ", hs_type_names[expression->type]);

		for (enum_index = 0; enum_index<enum_definition->count - 1; enum_index++)
		{
			strcat(hs_compile_globals.error_buffer, "\"");
			strcat(hs_compile_globals.error_buffer, enum_definition->identifiers[enum_index]);
			strcat(hs_compile_globals.error_buffer, "\", ");
		}

		if (enum_definition->count>1)
		{
			strcat(hs_compile_globals.error_buffer, "or ");
		}

		strcat(hs_compile_globals.error_buffer, "\"");
		strcat(hs_compile_globals.error_buffer, enum_definition->identifiers[enum_index]);
		strcat(hs_compile_globals.error_buffer, "\".");
		hs_compile_globals.error = hs_compile_globals.error_buffer;
		hs_compile_globals.error_offset = expression->source_offset;
		success = FALSE;
	}

	*(short *)&expression->data = enum_index;

	return success;
}

static boolean hs_parse_scenario_datum(
	long expression_index,
	short offset,
	struct tag_block *block,
	long element_size)
{
	short element_index;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1777, element_size<=SHORT_MAX);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1778, offset+TAG_STRING_LENGTH<element_size);

	for (element_index = 0; element_index<block->count; element_index++)
	{
		char *element = tag_block_get_element_with_size(block, element_index, element_size);

		if (!_stricmp(element + offset, hs_compile_globals.compiled_source + expression->source_offset))
		{
			expression->data = element_index;
			success = TRUE;
			break;
		}
	}

	if (!success)
	{
		sprintf(
			hs_compile_globals.error_buffer,
			"this is not a valid %s name",
			hs_type_names[expression->type]);
		hs_compile_globals.error = hs_compile_globals.error_buffer;
		hs_compile_globals.error_offset = expression->source_offset;
	}

	return success;
}

static boolean hs_parse_trigger_volume(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1809, hs_syntax_get(expression_index)->type==_hs_type_trigger_volume);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct scenario_trigger_volume, name),
		&global_scenario_get()->trigger_volumes,
		sizeof(struct scenario_trigger_volume));
}

static boolean hs_parse_cutscene_flag(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1817, hs_syntax_get(expression_index)->type==_hs_type_cutscene_flag);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct scenario_cutscene_flag, name),
		&global_scenario_get()->cutscene_flags,
		sizeof(struct scenario_cutscene_flag));
}

static boolean hs_parse_cutscene_camera_point(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1825, hs_syntax_get(expression_index)->type==_hs_type_cutscene_camera_point);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct scenario_cutscene_camera_point, name),
		&global_scenario_get()->cutscene_camera_points,
		sizeof(struct scenario_cutscene_camera_point));
}

static boolean hs_parse_cutscene_title(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1833, hs_syntax_get(expression_index)->type==_hs_type_cutscene_title);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct scenario_cutscene_title, name),
		&global_scenario_get()->cutscene_chapter_titles,
		sizeof(struct scenario_cutscene_title));
}

static boolean hs_parse_cutscene_recording(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1841, hs_syntax_get(expression_index)->type==_hs_type_cutscene_recording);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct recorded_animation_definition, name),
		&global_scenario_get()->recorded_animations,
		sizeof(struct recorded_animation_definition));
}

static boolean hs_parse_device_group(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1849, hs_syntax_get(expression_index)->type==_hs_type_device_group);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct scenario_device_group, name),
		&global_scenario_get()->device_groups,
		sizeof(struct scenario_device_group));
}

static boolean hs_parse_ai(
	long expression_index)
{
	boolean success;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1860, hs_syntax_get(expression_index)->type==_hs_type_ai);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1861, expression->constant_type==expression->type);

	success = ai_index_from_string(
		global_scenario_get(),
		hs_compile_globals.compiled_source + expression->source_offset,
		(long *)&expression->data);

	if (!success)
	{
		hs_compile_globals.error = "this is not a valid ai encounter or squad.";
		hs_compile_globals.error_offset = expression->source_offset;
	}

	return success;
}

static boolean hs_parse_ai_command_list(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1877, hs_syntax_get(expression_index)->type==_hs_type_ai_command_list);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct ai_command_list_definition, name),
		&global_scenario_get()->ai_command_lists,
		sizeof(struct ai_command_list_definition));
}

static boolean hs_parse_starting_profile(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1885, hs_syntax_get(expression_index)->type==_hs_type_starting_profile);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct scenario_starting_profile, name),
		&global_scenario_get()->starting_profiles,
		sizeof(struct scenario_starting_profile));
}

static boolean hs_parse_conversation(
	long expression_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1893, hs_syntax_get(expression_index)->type==_hs_type_conversation);

	return hs_parse_scenario_datum(
		expression_index,
		offsetof(struct ai_conversation, name),
		&global_scenario_get()->ai_conversations,
		sizeof(struct ai_conversation));
}

static boolean hs_parse_object_name(
	long expression_index)
{
	short object_name_index;
	boolean success = FALSE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1905, HS_TYPE_IS_OBJECT_NAME(expression->type));

	object_name_index = scenario_object_name_index_from_string(
		global_scenario_get(),
		hs_compile_globals.compiled_source + expression->source_offset);

	if (object_name_index!=NONE)
	{
		struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->object_names,
			object_name_index,
			struct scenario_object_name);

		match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1914, object_name->runtime_object_type!=NONE);

		if (TEST_FLAG(hs_object_type_masks[expression->type - FIRST_HS_OBJECT_NAME_TYPE], object_name->runtime_object_type))
		{
			*(short *)&expression->data = object_name_index;
			success = TRUE;
		}
		else
		{
			sprintf(
				hs_compile_globals.error_buffer,
				"this is not an object of type %s.",
				hs_type_names[expression->type]);
			hs_compile_globals.error = hs_compile_globals.error_buffer;
			hs_compile_globals.error_offset = expression->source_offset;
		}
	}
	else
	{
		hs_compile_globals.error = "this is not a valid object name.";
		hs_compile_globals.error_offset = expression->source_offset;
	}

	return success;
}

static boolean hs_parse_object(
	long expression_index)
{
	boolean success = TRUE;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1946, HS_TYPE_IS_OBJECT(expression->type));

	if (!strcmp(hs_compile_globals.compiled_source + expression->source_offset, "none"))
	{
		expression->data = NONE;
	}
	else
	{
		expression->type += NUMBER_OF_HS_OBJECT_TYPES;
		expression->constant_type = expression->type;
		success = hs_parse_object_name(expression_index);
		expression->type -= NUMBER_OF_HS_OBJECT_TYPES;
	}

	return success;
}

static boolean hs_parse_navpoint(
	long expression_index)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1969, hs_syntax_get(expression_index)->type==_hs_type_navpoint);

	if (interface_get_tag_index(_interface_hud_globals)!=NONE)
	{
		struct hud_globals_definition *hud_globals = hud_globals_definition_get(
			interface_get_tag_index(_interface_hud_globals));

		success = hs_parse_scenario_datum(
			expression_index,
			offsetof(struct hud_waypoint_arrow, name),
			&hud_globals->waypoint.arrows,
			sizeof(struct hud_waypoint_arrow));
	}

	return success;
}

static boolean hs_parse_hud_message(
	long expression_index)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1983, hs_syntax_get(expression_index)->type==_hs_type_hud_message);

	if (global_scenario_get()->hud_messages.index!=NONE)
	{
		struct hud_state_messages *hud_messages = hud_state_messages_get(global_scenario_get()->hud_messages.index);

		success = hs_parse_scenario_datum(
			expression_index,
			offsetof(struct hud_state_message_definition, name),
			&hud_messages->messages,
			sizeof(struct hud_state_message_definition));
	}

	return success;
}

static boolean hs_parse_object_list(
	long expression_index)
{
	boolean success;
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 1999, expression->type==_hs_type_object_list);

	expression->constant_type = _hs_type_object_name;
	expression->type = _hs_type_object_name;
	success = hs_parse_object_name(expression_index);
	expression->type = _hs_type_object_list;

	return success;
}

boolean (*const hs_type_primitive_parsers[NUMBER_OF_HS_NODE_TYPES])(long expression_index) =
{
	NULL,							// _hs_unparsed
	NULL,							// _hs_special_form
	NULL,							// _hs_function_name
	NULL,							// _hs_passthrough
	NULL,							// _hs_type_void
	hs_parse_boolean,				// _hs_type_boolean
	hs_parse_real,					// _hs_type_real
	hs_parse_integer,				// _hs_type_short_integer
	hs_parse_integer,				// _hs_type_long_integer
	hs_parse_string,				// _hs_type_string
	hs_parse_script,				// _hs_type_script
	hs_parse_trigger_volume,		// _hs_type_trigger_volume
	hs_parse_cutscene_flag,			// _hs_type_cutscene_flag
	hs_parse_cutscene_camera_point,	// _hs_type_cutscene_camera_point
	hs_parse_cutscene_title,		// _hs_type_cutscene_title
	hs_parse_cutscene_recording,	// _hs_type_cutscene_recording
	hs_parse_device_group,			// _hs_type_device_group
	hs_parse_ai,					// _hs_type_ai
	hs_parse_ai_command_list,		// _hs_type_ai_command_list
	hs_parse_starting_profile,		// _hs_type_starting_profile
	hs_parse_conversation,			// _hs_type_conversation
	hs_parse_navpoint,				// _hs_type_navpoint
	hs_parse_hud_message,			// _hs_type_hud_message
	hs_parse_object_list,			// _hs_type_object_list
	hs_parse_tag_reference,			// _hs_type_sound
	hs_parse_tag_reference,			// _hs_type_effect
	hs_parse_tag_reference,			// _hs_type_damage
	hs_parse_tag_reference,			// _hs_type_looping_sound
	hs_parse_tag_reference,			// _hs_type_animation_graph
	hs_parse_tag_reference,			// _hs_type_actor_variant
	hs_parse_tag_reference,			// _hs_type_damage_effect
	hs_parse_tag_reference,			// _hs_type_object_definition
	hs_parse_enum,					// _hs_type_enum_game_difficulty
	hs_parse_enum,					// _hs_type_enum_team
	hs_parse_enum,					// _hs_type_enum_ai_default_state
	hs_parse_enum,					// _hs_type_enum_actor_type
	hs_parse_enum,					// _hs_type_enum_hud_corner
	hs_parse_object,				// _hs_type_object
	hs_parse_object,				// _hs_type_unit
	hs_parse_object,				// _hs_type_vehicle
	hs_parse_object,				// _hs_type_weapon
	hs_parse_object,				// _hs_type_device
	hs_parse_object,				// _hs_type_scenery
	hs_parse_object_name,			// _hs_type_object_name
	hs_parse_object_name,			// _hs_type_unit_name
	hs_parse_object_name,			// _hs_type_vehicle_name
	hs_parse_object_name,			// _hs_type_weapon_name
	hs_parse_object_name,			// _hs_type_device_name
	hs_parse_object_name			// _hs_type_scenery_name
};

boolean hs_macro_function_parse(
	short function_index,
	long expression_index)
{
	short parameter_index;
	boolean success = TRUE;
	struct hs_function_definition const *definition = hs_function_get(function_index);
	long argument_index = hs_syntax_get(hs_syntax_get(expression_index)->data)->next_node_index;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_compile.c", 2073, hs_type_valid(definition->return_type));

	for (parameter_index = 0;
		success && parameter_index<definition->formal_parameter_count && argument_index!=NONE;
		parameter_index++)
	{
		if (hs_parse(argument_index, definition->formal_parameters[parameter_index]))
		{
			struct hs_syntax_node *argument = hs_syntax_get(argument_index);

			argument_index = argument->next_node_index;
		}
		else
		{
			success = FALSE;
		}
	}

	if (success && (parameter_index!=definition->formal_parameter_count || argument_index!=NONE))
	{
		sprintf(
			hs_compile_globals.error_buffer,
			"the \"%s\" call requires exactly %d arguments.",
			definition->name,
			definition->formal_parameter_count);
		hs_compile_globals.error = hs_compile_globals.error_buffer;
		hs_compile_globals.error_offset = hs_syntax_get(expression_index)->source_offset;
		success = FALSE;
	}

	return success;
}

static short string_list_find(
	char const *string,
	short list_count,
	char const **string_list)
{
	short index;

	for (index = 0; index<list_count; index++)
	{
		if (!strcmp(string, string_list[index]))
		{
			return index;
		}
	}

	return NONE;
}
