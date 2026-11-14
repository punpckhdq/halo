/*
HS_SCENARIO_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __HS_SCENARIO_DEFINITIONS_H
#define __HS_SCENARIO_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "hs.h"

/* ---------- constants */

enum
{
	MAXIMUM_HS_SCRIPTS_PER_SCENARIO = 0x200,
	MAXIMUM_HS_GLOBALS_PER_SCENARIO = 0x80,
	MAXIMUM_HS_REFERENCES_PER_SCENARIO = 0x100,
	MAXIMUM_HS_SYNTAX_NODES_PER_SCENARIO = 0x4A39,
	MAXIMUM_HS_STRING_DATA_PER_SCENARIO = 0x40000,
	MAXIMUM_HS_SOURCE_FILES_PER_SCENARIO = 0x8,
	MAXIMUM_HS_SOURCE_DATA_PER_FILE = 0x40000
};

/* ---------- macros */

#define SIZEOF_HS_SYNTAX_DATA (sizeof(struct data_array) + MAXIMUM_HS_SYNTAX_NODES_PER_SCENARIO * sizeof(struct hs_syntax_node)) /* fake name? */

#define hs_script_get(index) TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->hs_scripts, (index), struct hs_script) /* fake name? */
#define hs_global_internal_get(index) TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->hs_globals, (index), struct hs_global_internal) /* fake name? */

/* ---------- structures */

struct hs_script
{
	char name[TAG_STRING_LENGTH+1];
	short script_type;
	short return_type;
	long root_expression_index;
	long unused[13];
};

struct hs_global_internal
{
	char name[TAG_STRING_LENGTH+1];
	short type;
	word pad;
	long pad2;
	long initialization_expression_index;
	long unused[12];
};

struct hs_tag_reference
{
	long runtime_instance_index;
	long runtime_timestamp;
	long unused[4];
	struct tag_reference reference;
};

struct hs_source_file
{
	char name[TAG_STRING_LENGTH+1];
	struct tag_data source;
};

/* ---------- globals */

extern struct enum_definition hs_script_types_enum;
extern struct enum_definition hs_types_enum;
extern struct tag_block_definition hs_scripts_block;
extern struct tag_block_definition hs_globals_block;
extern struct tag_block_definition hs_references_block;
extern struct tag_data_definition hs_source_data_definition;
extern struct tag_block_definition hs_source_files_block;
extern struct tag_data_definition hs_syntax_data_definition;
extern struct tag_data_definition hs_string_data_definition;

#endif // __HS_SCENARIO_DEFINITIONS_H
