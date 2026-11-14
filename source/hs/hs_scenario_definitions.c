/*
HS_SCENARIO_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hs_scenario_definitions.h"

/* ---------- prototypes */

static void byte_swap_script_syntax_data(void *scenario, void *data, long size);

/* ---------- globals */

struct enum_definition hs_script_types_enum =
{
	NUMBER_OF_HS_SCRIPT_TYPES,
	(char **)hs_script_type_names
};

struct enum_definition hs_types_enum =
{
	NUMBER_OF_HS_NODE_TYPES,
	(char **)hs_type_names
};

TAG_BLOCK(hs_scripts_block, MAXIMUM_HS_SCRIPTS_PER_SCENARIO, sizeof(struct hs_script), NULL, NULL, NULL, NULL)
{
	{_field_string, "name*"},
	{_field_enum, "script type*", &hs_script_types_enum},
	{_field_enum, "return type*", &hs_types_enum},
	{_field_long_integer, "root expression index*"},
	FIELD_PAD(13*sizeof(long)),
	{_field_terminator}
};

TAG_BLOCK(hs_globals_block, MAXIMUM_HS_GLOBALS_PER_SCENARIO, sizeof(struct hs_global_internal), NULL, NULL, NULL, NULL)
{
	{_field_string, "name*"},
	{_field_enum, "type*", &hs_types_enum},
	FIELD_PAD(sizeof(word)),
	FIELD_PAD(sizeof(long)),
	{_field_long_integer, "initialization expression index*"},
	FIELD_PAD(12*sizeof(long)),
	{_field_terminator}
};

static struct tag_reference_definition hs_reference_definition = {0, NONE}; /* fake name */

TAG_BLOCK(hs_references_block, MAXIMUM_HS_REFERENCES_PER_SCENARIO, sizeof(struct hs_tag_reference), NULL, NULL, NULL, NULL)
{
	FIELD_PAD(6*sizeof(long)),
	{_field_tag_reference, "reference*^", &hs_reference_definition},
	{_field_terminator}
};

struct tag_data_definition hs_source_data_definition =
{
	"hs_source_data_definition",
	0,
	MAXIMUM_HS_SOURCE_DATA_PER_FILE,
	NULL
};

TAG_BLOCK(hs_source_files_block, MAXIMUM_HS_SOURCE_FILES_PER_SCENARIO, sizeof(struct hs_source_file), NULL, NULL, NULL, NULL)
{
	{_field_string, "name*"},
	{_field_data, "source", &hs_source_data_definition},
	{_field_terminator}
};

static byte_swap_code data_array_header_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		TAG_STRING_LENGTH+1,
		_2byte,
		_2byte,
		_1byte,
		3,
		_4byte,
		_2byte,
		_2byte,
		_2byte,
		_2byte,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition data_array_header_bs_definition = /* fake name */
{
	"data_array_header",
	sizeof(struct data_array),
	data_array_header_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code syntax_node_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_2byte,
		_2byte,
		_2byte,
		_2byte,
		_4byte,
		_4byte,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition syntax_node_bs_definition = /* fake name */
{
	"syntax_node",
	sizeof(struct hs_syntax_node),
	syntax_node_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

struct tag_data_definition hs_syntax_data_definition =
{
	"hs_syntax_data_definition",
	0,
	SIZEOF_HS_SYNTAX_DATA,
	byte_swap_script_syntax_data
};

struct tag_data_definition hs_string_data_definition =
{
	"hs_string_data_definition",
	0,
	MAXIMUM_HS_STRING_DATA_PER_SCENARIO,
	NULL
};

/* ---------- private code */

static void byte_swap_script_syntax_data(
	void *scenario,
	void *data,
	long size)
{
	if (size)
	{
		match_assert("c:\\halo\\SOURCE\\hs\\hs_scenario_definitions.c", 109, size>=sizeof(struct data_array));

		if (!memcmp(data, "csirtpn \0edo", 12))
		{
			byte_swap_data(&data_array_header_bs_definition, data, 1);
			byte_swap_data(&syntax_node_bs_definition, data, 0x4000);
		}
		else
		{
			long data_size = size - sizeof(struct data_array);

			match_assert("c:\\halo\\SOURCE\\hs\\hs_scenario_definitions.c", 121, data_size>=0 && (data_size%sizeof(struct hs_syntax_node))==0);

			if (data_size>=0 && (data_size % sizeof(struct hs_syntax_node))==0)
			{
				byte_swap_data(&data_array_header_bs_definition, data, 1);
				byte_swap_data(
					&syntax_node_bs_definition,
					(byte *)data + sizeof(struct data_array),
					data_size / sizeof(struct hs_syntax_node));
			}
		}
	}

	return;
}
