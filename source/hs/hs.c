/*
HS.C
*/

/* ---------- headers */

#include "cseries.h"
#include <ctype.h>
#include "hs.h"
#include "object_lists.h"
#include "hs_library_internal.h"
#include "hs_scenario_definitions.h"
#include "objects.h"
#include "console.h"
#include "sound_definitions.h"
#include "effect_definitions.h"
#include "damage_effect_definitions.h"
#include "recorded_animation_definitions.h"
#include "hud_messaging_definitions.h"
#include "interface.h"
#include "hud_definitions.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "actor_definitions.h"
#include "model_animation_definitions.h"
#include "hs_library_external.h"
#include "hs_glue.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "game_state.h"
#include "bungie_net/network/transport.h"
#include "shaders.h"
#include "actors.h"
#include "sound_manager.h"
#include "text_group.h"
#include "vehicles.h"
#include "ai_debug.h"
#include "encounters.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "actor_types.h"
#include "director.h"
#include "ui_widget.h"
#include "game_sound.h"
#include "texture_cache.h"
#include "hud.h"
#include "object_lights.h"
#include "terminal.h"
#include "player_ui.h"
#include "cinematics.h"
#include "devices.h"
#include "player_effects.h"
#include "hud_messaging.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "sound_cache.h"
#include "scenery.h"
#include "recorded_animations.h"
#include "ai_globals.h"
#include "rasterizer_cinematics.h"
#include "sound_classes.h"
#include "camera_scripting.h"
#include "attract_mode.h"
#include "input_device_defaults.h"
#include "structures.h"
#include "units.h"
#include "cheats.h"
#include "game.h"
#include "structures/radiosity/radiosity.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

/* ---------- constants */

enum
{
	_hs_fakery_none = 0,
	_hs_fakery_parenthesize,
	_hs_fakery_set,
	NUMBER_OF_HS_FAKERY_TYPES
};

#define HS_GLOBAL_SCRIPTS_FILE_PATH "data\\global_scripts.hsc" /* fake name */

/* ---------- prototypes */

#ifdef HS_COMPILER_ENABLED
static boolean hs_compile_source(void);
static void hs_get_function_parameters_string(short function_index, char *buffer);
static void hs_get_function_documentation_string(short function_index, char *buffer);
#endif

/* ---------- globals */

char *hs_type_names[NUMBER_OF_HS_NODE_TYPES] =
{
	"unparsed",
	"special form",
	"function name",
	"passthrough",
	"void",
	"boolean",
	"real",
	"short",
	"long",
	"string",
	"script",
	"trigger_volume",
	"cutscene_flag",
	"cutscene_camera_point",
	"cutscene_title",
	"cutscene_recording",
	"device_group",
	"ai",
	"ai_command_list",
	"starting_profile",
	"conversation",
	"navpoint",
	"hud_message",
	"object_list",
	"sound",
	"effect",
	"damage",
	"looping_sound",
	"animation_graph",
	"actor_variant",
	"damage_effect",
	"object_definition",
	"game_difficulty",
	"team",
	"ai_default_state",
	"actor_type",
	"hud_corner",
	"object",
	"unit",
	"vehicle",
	"weapon",
	"device",
	"scenery",
	"object_name",
	"unit_name",
	"vehicle_name",
	"weapon_name",
	"device_name",
	"scenery_name"
};

char *hs_script_type_names[NUMBER_OF_HS_SCRIPT_TYPES] =
{
	"startup",
	"dormant",
	"continuous",
	"static",
	"stub"
};

short const hs_object_type_masks[NUMBER_OF_HS_OBJECT_TYPES] =
{
	_object_mask_all,			// _hs_type_object
	_object_mask_unit,			// _hs_type_unit
	_object_mask_vehicle,		// _hs_type_vehicle
	_object_mask_weapon,		// _hs_type_weapon
	_object_mask_device,		// _hs_type_device
	_object_mask_scenery		// _hs_type_scenery
};

long const hs_tag_reference_type_group_tags[NUMBER_OF_HS_TAG_REFERENCE_TYPES] =
{
	SOUND_DEFINITION_TAG,			// _hs_type_sound
	EFFECT_DEFINITION_TAG,			// _hs_type_effect
	DAMAGE_EFFECT_DEFINITION_TAG,	// _hs_type_damage
	LOOPING_SOUND_DEFINITION_TAG,	// _hs_type_looping_sound
	ANIMATION_GRAPH_TAG,			// _hs_type_animation_graph
	ACTOR_VARIANT_DEFINITION_TAG,	// _hs_type_actor_variant
	DAMAGE_EFFECT_DEFINITION_TAG,	// _hs_type_damage_effect
	OBJECT_DEFINITION_TAG			// _hs_type_object_definition
};

short const hs_type_sizes[NUMBER_OF_HS_NODE_TYPES] =
{
	0,					// _hs_unparsed
	0,					// _hs_special_form
	0,					// _hs_function_name
	0,					// _hs_passthrough
	0,					// _hs_type_void
	sizeof(boolean),	// _hs_type_boolean
	sizeof(real),		// _hs_type_real
	sizeof(short),		// _hs_type_short_integer
	sizeof(long),		// _hs_type_long_integer
	sizeof(char *),		// _hs_type_string
	sizeof(long),		// _hs_type_script
	sizeof(short),		// _hs_type_trigger_volume
	sizeof(short),		// _hs_type_cutscene_flag
	sizeof(short),		// _hs_type_cutscene_camera_point
	sizeof(short),		// _hs_type_cutscene_title
	sizeof(short),		// _hs_type_cutscene_recording
	sizeof(short),		// _hs_type_device_group
	sizeof(long),		// _hs_type_ai
	sizeof(short),		// _hs_type_ai_command_list
	sizeof(short),		// _hs_type_starting_profile
	sizeof(short),		// _hs_type_conversation
	sizeof(short),		// _hs_type_navpoint
	sizeof(short),		// _hs_type_hud_message
	sizeof(long),		// _hs_type_object_list
	sizeof(long),		// _hs_type_sound
	sizeof(long),		// _hs_type_effect
	sizeof(long),		// _hs_type_damage
	sizeof(long),		// _hs_type_looping_sound
	sizeof(long),		// _hs_type_animation_graph
	sizeof(long),		// _hs_type_actor_variant
	sizeof(long),		// _hs_type_damage_effect
	sizeof(long),		// _hs_type_object_definition
	sizeof(short),		// _hs_type_enum_game_difficulty
	sizeof(short),		// _hs_type_enum_team
	sizeof(short),		// _hs_type_enum_ai_default_state
	sizeof(short),		// _hs_type_enum_actor_type
	sizeof(short),		// _hs_type_enum_hud_corner
	sizeof(long),		// _hs_type_object
	sizeof(long),		// _hs_type_unit
	sizeof(long),		// _hs_type_vehicle
	sizeof(long),		// _hs_type_weapon
	sizeof(long),		// _hs_type_device
	sizeof(long),		// _hs_type_scenery
	sizeof(short),		// _hs_type_object_name
	sizeof(short),		// _hs_type_unit_name
	sizeof(short),		// _hs_type_vehicle_name
	sizeof(short),		// _hs_type_weapon_name
	sizeof(short),		// _hs_type_device_name
	sizeof(short)		// _hs_type_scenery_name
};

boolean const _hs_type_boolean_default = FALSE;
real const _hs_type_real_default = 0.f;
short const _hs_type_short_integer_default = 0;
long const _hs_type_long_integer_default = 0;
char const *_hs_type_string_default = "";
short const _hs_type_script_default = NONE;
short const _hs_type_trigger_volume_default = NONE;
short const _hs_type_cutscene_flag_default = NONE;
short const _hs_type_cutscene_camera_point_default = NONE;
short const _hs_type_cutscene_title_default = NONE;
short const _hs_type_cutscene_recording_default = NONE;
short const _hs_type_device_group_default = NONE;
long const _hs_type_ai_default = NONE;
short const _hs_type_ai_command_list_default = NONE;
short const _hs_type_starting_profile_default = NONE;
short const _hs_type_conversation_default = NONE;
short const _hs_type_navpoint_default = NONE;
short const _hs_type_hud_message_default = NONE;
long const _hs_type_object_list_default = NONE;
long const _hs_type_sound_default = NONE;
long const _hs_type_looping_sound_default = NONE;
long const _hs_type_effect_default = NONE;
long const _hs_type_damage_default = NONE;
long const _hs_type_animation_graph_default = NONE;
long const _hs_type_actor_variant_default = NONE;
long const _hs_type_damage_effect_default = NONE;
long const _hs_type_object_definition_default = NONE;
short const _hs_type_enum_game_difficulty_default = NONE;
short const _hs_type_enum_team_default = NONE;
short const _hs_type_enum_ai_default_state_default = NONE;
short const _hs_type_enum_actor_type_default = NONE;
short const _hs_type_enum_hud_corner_default = NONE;
short const _hs_type_object_name_default = NONE;
long const _hs_type_object_default = NONE;
long const _hs_type_unit_default = NONE;
long const _hs_type_vehicle_default = NONE;
long const _hs_type_weapon_default = NONE;
long const _hs_type_device_default = NONE;
long const _hs_type_scenery_default = NONE;

#include "hs_function.h"
#include "hs_enum.h"

struct data_array *hs_syntax_data;

static boolean hs_syntax_data_allocated;
static boolean recompile;

static struct profile_section hs_update_section = {"hs_update", NONE, TRUE}; /* fake name */

static char const *enumeration_substring;
static char const **enumeration_results;
static short enumeration_maximum_count;
static short enumeration_count;

/* ---------- public code */

void hs_initialize(
	void)
{
	match_vassert("c:\\halo\\SOURCE\\hs\\hs.c", 244, hs_object_type_masks[NUMBER_OF_HS_OBJECT_TYPES - 1], "you can't add an hs object type without defining its mask.");
	match_vassert("c:\\halo\\SOURCE\\hs\\hs.c", 245, hs_type_names[NUMBER_OF_HS_NODE_TYPES - 1], "you can't add an hs type without defining its name.");

	object_lists_initialize();
	hs_runtime_initialize();
	hs_initialize_for_new_map();

	return;
}

boolean hs_scenario_merge(
	struct scenario *master,
	struct scenario const *slave)
{
	short slave_source_file_index;
	boolean success = TRUE;

	for (slave_source_file_index = 0; slave_source_file_index<slave->hs_source_files.count; slave_source_file_index++)
	{
		short master_source_file_index;
		struct hs_source_file *slave_source_file = TAG_BLOCK_GET_ELEMENT(
			&slave->hs_source_files,
			slave_source_file_index,
			struct hs_source_file);

		for (master_source_file_index = 0; master_source_file_index<master->hs_source_files.count; master_source_file_index++)
		{
			if (!_stricmp(
				slave_source_file->name,
				TAG_BLOCK_GET_ELEMENT(&master->hs_source_files, master_source_file_index, struct hs_source_file)->name))
			{
				break;
			}
		}

		if (master_source_file_index==master->hs_source_files.count)
		{
			short new_source_file_index = (short)tag_block_add_element(&master->hs_source_files);

			if (new_source_file_index!=NONE)
			{
				struct hs_source_file *master_source_file = TAG_BLOCK_GET_ELEMENT(
					&master->hs_source_files,
					new_source_file_index,
					struct hs_source_file);

				strcpy(master_source_file->name, slave_source_file->name);

				if (tag_data_resize(&master_source_file->source, slave_source_file->source.size))
				{
					memcpy(
						master_source_file->source.address,
						slave_source_file->source.address,
						slave_source_file->source.size);
				}
				else
				{
					success = FALSE;
				}
			}
			else
			{
				success = FALSE;
			}
		}
	}

	tag_block_resize(&master->hs_scripts, 0);

	return success;
}

static void hs_allocate(
	void)
{
	struct scenario *scenario = global_scenario_index!=NONE ? global_scenario_get() : NULL;

	if (!scenario || scenario->hs_syntax_data.size!=SIZEOF_HS_SYNTAX_DATA)
	{
		hs_syntax_data = data_new("script node", MAXIMUM_HS_SYNTAX_NODES_PER_SCENARIO, sizeof(struct hs_syntax_node));

		if (hs_syntax_data)
		{
			data_make_valid(hs_syntax_data);

			if (scenario)
			{
				match_free("c:\\halo\\SOURCE\\hs\\hs.c", 336, scenario->hs_syntax_data.address);
				scenario->hs_syntax_data.address = hs_syntax_data;
				scenario->hs_syntax_data.size = SIZEOF_HS_SYNTAX_DATA;
				tag_data_resize(&scenario->hs_string_constants, HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES);
				tag_block_resize(&scenario->hs_scripts, 0);
			}
			else
			{
				hs_syntax_data_allocated = TRUE;
			}
		}
		else
		{
			error(_error_immediate, "couldn't allocate script syntax data");
		}
	}

	return;
}

boolean hs_scenario_postprocess(
	boolean hack)
{
	char const *error_message;
	char const *error_source;
	boolean recompile_scripts;
	boolean success = TRUE;
	struct scenario *scenario = global_scenario_get();
	struct data_array *old_syntax_data = hs_syntax_data;

	hs_allocate();
	recompile_scripts = !scenario->hs_scripts.count && scenario->hs_source_files.count>0;
	hs_syntax_data = (struct data_array *)scenario->hs_syntax_data.address;
	hs_syntax_data->data = hs_syntax_data + 1;

	if (!recompile_scripts
#ifdef HS_COMPILER_ENABLED
		&& hs_compile_postprocess(&error_message, &error_source)
#endif
	)
	{
		if (scenario->hs_string_constants.size<HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES)
		{
			success = tag_data_resize(
				&scenario->hs_string_constants,
				scenario->hs_string_constants.size + HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES);
		}
	}
	else
	{
#ifdef HS_COMPILER_ENABLED
		if (recompile_scripts)
		{
			error(_error_immediate, "recompiling scripts after scenarios were merged.");
		}
		else if (!error_message)
		{
			error(_error_immediate, "an unspecified error occurred loading scripts");
		}
		else if (!error_source)
		{
			error(_error_immediate, "%s", error_message);
		}
		else
		{
			error(_error_immediate, "%s: %s", error_source, error_message);
		}

		if (hs_compile_source() && hs_compile_postprocess(&error_message, &error_source))
		{
			success = TRUE;
		}
		else
#endif
		{
#ifdef HS_COMPILER_ENABLED
			data_delete_all(hs_syntax_data);

			if (!tag_block_resize(&scenario->hs_globals, 0) ||
				!tag_block_resize(&scenario->hs_scripts, 0) ||
				!tag_data_resize(&global_scenario_get()->hs_string_constants, HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES))
			{
				error(_error_immediate, "couldn't reset scripts.");
			}
#endif

			success = FALSE;
		}
	}

	if (hack)
	{
		hs_syntax_data = old_syntax_data;
	}

	return success;
}

void hs_initialize_for_new_map(
	void)
{
	struct scenario *scenario = global_scenario_index!=NONE ? global_scenario_get() : NULL;

	hs_allocate();

	if (scenario && scenario->hs_syntax_data.size)
	{
		hs_scenario_postprocess(FALSE);
	}

	object_lists_initialize_for_new_map();
	hs_runtime_initialize_for_new_map();

	return;
}

void hs_dispose_from_old_map(
	void)
{
	if (hs_syntax_data)
	{
		hs_node_gc();

		if (hs_syntax_data_allocated)
		{
			data_make_invalid(hs_syntax_data);
			data_dispose(hs_syntax_data);
			hs_syntax_data_allocated = FALSE;
		}

		hs_syntax_data = NULL;
	}

	hs_runtime_dispose_from_old_map();
	object_lists_dispose_from_old_map();

	return;
}

void hs_dispose(
	void)
{
	hs_runtime_dispose_from_old_map();
	object_lists_dispose();

	return;
}

void hs_recompile(
	void)
{
	recompile = TRUE;

	return;
}

void hs_update(
	void)
{
	profile_enter(hs_update_section);
	hs_runtime_update();
	profile_exit(hs_update_section);

	return;
}

void hs_node_gc(
	void)
{
	long expression_index;

	for (expression_index = data_next_index(hs_syntax_data, NONE); expression_index!=NONE; expression_index = data_next_index(hs_syntax_data, expression_index))
	{
		if (!TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_permanent_bit))
		{
			datum_delete(hs_syntax_data, expression_index);
		}
	}

	return;
}

struct hs_function_definition const *hs_function_get(
	short function_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs.c", 522, function_index>=0 && function_index<hs_function_table_count);

	return hs_function_table[function_index];
}

short hs_find_script_by_name(
	char const *name)
{
	if (global_scenario_index!=NONE)
	{
		short script_index;
		struct scenario *scenario = global_scenario_get();

		for (script_index = 0; script_index<scenario->hs_scripts.count; script_index++)
		{
			struct hs_script *script = TAG_BLOCK_GET_ELEMENT(&scenario->hs_scripts, script_index, struct hs_script);

			if (!strcmp(name, script->name))
			{
				return script_index;
			}
		}
	}

	return NONE;
}

short hs_find_tag_reference_by_index(
	long tag_index)
{
	if (global_scenario_index!=NONE)
	{
		short reference_index;
		struct scenario *scenario = global_scenario_get();

		for (reference_index = 0; reference_index<scenario->hs_references.count; reference_index++)
		{
			struct hs_tag_reference *reference = TAG_BLOCK_GET_ELEMENT(
				&scenario->hs_references,
				reference_index,
				struct hs_tag_reference);

			if (reference->reference.index==tag_index)
			{
				return reference_index;
			}
		}
	}

	return NONE;
}

struct hs_global_external *hs_global_external_get(
	short global_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs.c", 576, global_index>=0 && global_index<hs_external_global_count);

	return hs_external_globals[global_index];
}

short hs_global_get_type(
	short designator)
{
	if (designator & FLAG(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT))
	{
		return hs_global_external_get(HS_GLOBAL_INDEX(designator))->type;
	}
	else
	{
		return HS_GLOBAL_INTERNAL_GET(HS_GLOBAL_INDEX(designator))->type;
	}
}

char const *hs_global_get_name(
	short designator)
{
	if (designator & FLAG(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT))
	{
		return hs_global_external_get(HS_GLOBAL_INDEX(designator))->name;
	}
	else
	{
		return HS_GLOBAL_INTERNAL_GET(HS_GLOBAL_INDEX(designator))->name;
	}
}

short hs_find_global_by_name(
	char const *name)
{
	short global_index;

	for (global_index = 0; global_index<hs_external_global_count; global_index++)
	{
		if (!_stricmp(name, hs_global_external_get(global_index)->name))
		{
			return global_index | FLAG(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT);
		}
	}

	if (global_scenario_index!=NONE)
	{
		struct scenario *scenario = global_scenario_get();

		for (global_index = 0; global_index<scenario->hs_globals.count; global_index++)
		{
			if (!_stricmp(name, HS_GLOBAL_INTERNAL_GET(global_index)->name))
			{
				return global_index & ~FLAG(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT);
			}
		}
	}

	return NONE;
}

#ifdef HS_COMPILER_ENABLED
short hs_find_function_by_name(
	char const *name)
{
	short function_index;

	for (function_index = 0; function_index<hs_function_table_count; function_index++)
	{
		if (!_stricmp(hs_function_table[function_index]->name, name))
		{
			return function_index;
		}
	}

	return NONE;
}
#endif

static int __cdecl alphabetize(
	void const *string1,
	void const *string2)
{
	return _stricmp(*(char const **)string1, *(char const **)string2);
}

static void hs_tokens_enumerate_add_string(
	char const *string)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs.c", 666, enumeration_results);

	if (enumeration_count<enumeration_maximum_count && !_strnicmp(string, enumeration_substring, strlen(enumeration_substring)))
	{
		enumeration_results[enumeration_count++] = string;
	}

	return;
}

static void hs_enumerate_from_string_list(
	char const **strings,
	short start_index,
	short count)
{
	short string_index;

	for (string_index = start_index; string_index<count; string_index++)
	{
		hs_tokens_enumerate_add_string(strings[string_index]);
	}

	return;
}

static void hs_enumerate_block_data(
	struct tag_block *block,
	short element_offset,
	long size)
{
	short element_index;

	for (element_index = 0; element_index<block->count; element_index++)
	{
		char *element = (char *)tag_block_get_element_with_size(block, element_index, size);

		hs_tokens_enumerate_add_string(element + element_offset);
	}

	return;
}

static void hs_enumerate_scenario_data(
	short block_offset,
	short element_offset,
	long size)
{
	if (global_scenario_index!=NONE)
	{
		struct tag_block *block = (struct tag_block *)((char *)global_scenario_get() + block_offset);

		hs_enumerate_block_data(block, element_offset, size);
	}

	return;
}

static void hs_enumerate_special_form_names(
	void)
{
	hs_tokens_enumerate_add_string("script");
	hs_tokens_enumerate_add_string("global");

	return;
}

static void hs_enumerate_script_type_names(
	void)
{
	hs_enumerate_from_string_list(hs_script_type_names, 0, NUMBER_OF_HS_SCRIPT_TYPES);

	return;
}

static void hs_enumerate_type_names(
	void)
{
	hs_enumerate_from_string_list(hs_type_names, FIRST_HS_TYPE, NUMBER_OF_HS_NODE_TYPES);

	return;
}

static void hs_enumerate_function_names(
	void)
{
	#ifdef HS_COMPILER_ENABLED
	
	short function_index;

	for (function_index = 0; function_index<hs_function_table_count; function_index++)
	{
		hs_tokens_enumerate_add_string(hs_function_get(function_index)->name);
	}

	#endif

	return;
}

static void hs_enumerate_script_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, hs_scripts),
		offsetof(struct hs_script, name),
		sizeof(struct hs_script));

	return;
}

static void hs_enumerate_variable_names(
	void)
{
	short global_index;

	for (global_index = 0; global_index<hs_external_global_count; global_index++)
	{
		hs_tokens_enumerate_add_string(hs_global_external_get(global_index)->name);
	}

	hs_enumerate_scenario_data(
		offsetof(struct scenario, hs_globals),
		offsetof(struct hs_global_internal, name),
		sizeof(struct hs_global_internal));

	return;
}

static void hs_enumerate_ai_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, ai_encounters),
		offsetof(struct encounter_definition, name),
		sizeof(struct encounter_definition));

	return;
}

static void hs_enumerate_ai_command_list_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, ai_command_lists),
		offsetof(struct ai_command_list_definition, name),
		sizeof(struct ai_command_list_definition));

	return;
}

static void hs_enumerate_starting_profile_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, starting_profiles),
		offsetof(struct scenario_starting_profile, name),
		sizeof(struct scenario_starting_profile));

	return;
}

static void hs_enumerate_conversation_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, ai_conversations),
		offsetof(struct ai_conversation, name),
		sizeof(struct ai_conversation));

	return;
}

static void hs_enumerate_object_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, object_names),
		offsetof(struct scenario_object_name, name),
		sizeof(struct scenario_object_name));

	return;
}

static void hs_enumerate_trigger_volume_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, trigger_volumes),
		offsetof(struct scenario_trigger_volume, name),
		sizeof(struct scenario_trigger_volume));

	return;
}

static void hs_enumerate_cutscene_flag_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, cutscene_flags),
		offsetof(struct scenario_cutscene_flag, name),
		sizeof(struct scenario_cutscene_flag));

	return;
}

static void hs_enumerate_cutscene_camera_point_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, cutscene_camera_points),
		offsetof(struct scenario_cutscene_camera_point, name),
		sizeof(struct scenario_cutscene_camera_point));

	return;
}

static void hs_enumerate_cutscene_title_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, cutscene_chapter_titles),
		offsetof(struct scenario_cutscene_title, name),
		sizeof(struct scenario_cutscene_title));

	return;
}

static void hs_enumerate_cutscene_recording_names(
	void)
{
	hs_enumerate_scenario_data(
		offsetof(struct scenario, recorded_animations),
		offsetof(struct recorded_animation_definition, name),
		sizeof(struct recorded_animation_definition));

	return;
}

static void hs_enumerate_navpoints(
	void)
{
	if (interface_get_tag_index(_interface_hud_globals)!=NONE)
	{
		struct hud_globals_definition *hud_globals = hud_globals_definition_get(
			interface_get_tag_index(_interface_hud_globals));

		hs_enumerate_block_data(
			&hud_globals->waypoint.arrows,
			offsetof(struct hud_waypoint_arrow, name),
			sizeof(struct hud_waypoint_arrow));
	}

	return;
}

static void hs_enumerate_hud_messages(
	void)
{
	if (global_scenario_get()->hud_messages.index!=NONE)
	{
		struct hud_state_messages *hud_messages = hud_state_messages_get(global_scenario_get()->hud_messages.index);

		hs_enumerate_block_data(
			&hud_messages->messages,
			offsetof(struct hud_state_message_definition, name),
			sizeof(struct hud_state_message_definition));
	}

	return;
}

static void (*hs_token_enumerators[NUMBER_OF_HS_TOKEN_TYPES])(void) =
{
	hs_enumerate_special_form_names,			// _hs_token_type_form
	hs_enumerate_script_type_names,				// _hs_token_type_script_type
	hs_enumerate_type_names,					// _hs_token_type_type
	hs_enumerate_function_names,				// _hs_token_type_function
	hs_enumerate_script_names,					// _hs_token_type_script
	hs_enumerate_variable_names,				// _hs_token_type_variable
	hs_enumerate_ai_names,						// _hs_token_type_ai
	hs_enumerate_ai_command_list_names,			// _hs_token_type_ai_command_list
	hs_enumerate_starting_profile_names,		// _hs_token_type_starting_profile
	hs_enumerate_conversation_names,			// _hs_token_type_conversation
	hs_enumerate_object_names,					// _hs_token_type_object
	hs_enumerate_trigger_volume_names,			// _hs_token_type_trigger_volume
	hs_enumerate_cutscene_flag_names,			// _hs_token_type_cutscene_flag
	hs_enumerate_cutscene_camera_point_names,	// _hs_token_type_cutscene_camera_point
	hs_enumerate_cutscene_title_names,			// _hs_token_type_cutscene_title
	hs_enumerate_cutscene_recording_names,		// _hs_token_type_cutscene_recording
	hs_enumerate_navpoints,						// _hs_token_type_navpoint
	hs_enumerate_hud_messages					// _hs_token_type_hud_message
};

short hs_tokens_enumerate(
	char const *substring,
	long type_flags,
	char const **results,
	short maximum_count)
{
	short type_index;

	match_assert("c:\\halo\\SOURCE\\hs\\hs.c", 920, !enumeration_results);

	enumeration_count = 0;
	enumeration_maximum_count = maximum_count;
	enumeration_results = results;
	enumeration_substring = substring ? substring : "";

	for (type_index = 0; type_index<NUMBEROF(hs_token_enumerators); type_index++)
	{
		match_assert("c:\\halo\\SOURCE\\hs\\hs.c", 929, hs_token_enumerators[type_index]);

		if (TEST_FLAG(type_flags, type_index))
		{
			hs_token_enumerators[type_index]();
		}
	}

	qsort((void *)results, enumeration_count, sizeof(*results), alphabetize);
	enumeration_results = NULL;

	return enumeration_count;
}

static boolean hs_rebuild_source_file(
	struct file_reference *script_file)
{
	struct scenario *scenario = global_scenario_get();
	boolean success = FALSE;

	if (file_exists(script_file))
	{
		short source_file_index = (short)tag_block_add_element(&scenario->hs_source_files);

		if (source_file_index!=NONE)
		{
			unsigned long source_bytes;
			struct hs_source_file *source_file = TAG_BLOCK_GET_ELEMENT(
				&scenario->hs_source_files,
				source_file_index,
				struct hs_source_file);
			void *source = file_read_into_memory(script_file, &source_bytes);

			if (source)
			{
				if (tag_data_resize(&source_file->source, source_bytes))
				{
					char name[256];

					file_reference_get_name(script_file, FLAG(_name_filename_bit), name);
					strncpy(source_file->name, name, TAG_STRING_LENGTH);
					source_file->name[TAG_STRING_LENGTH] = '\0';
					memcpy(tag_data_get_pointer(&source_file->source, 0, source_bytes), source, source_bytes);
					success = TRUE;
				}
				else
				{
					error(_error_silent, "maximum source file size exceeded.");
				}
			}
			else
			{
				error(_error_silent, "couldn't read source file into memory.");
			}
		}
		else
		{
			error(_error_silent, "maximum source files per scenario exceeded.");
		}
	}

	return success;
}

static int __cdecl alphabetize_file_references(
	void const *ref1,
	void const *ref2)
{
	char n1[256];
	char n2[256];

	file_reference_get_name((struct file_reference const *)ref1, FLAG(_name_filename_bit), n1);
	file_reference_get_name((struct file_reference const *)ref2, FLAG(_name_filename_bit), n2);

	return _stricmp(n1, n2);
}

#ifdef HS_COMPILER_ENABLED
static boolean hs_rebuild_source(
	void)
{
	char script_directory_name[256];
	struct file_reference script_directory;
	struct file_reference global_scripts_file;
	struct file_reference script_files[MAXIMUM_HS_SOURCE_FILES_PER_SCENARIO];
	char extension[256];
	short script_file_count;
	short script_file_index;
	boolean success = TRUE;

	tag_block_resize(&global_scenario_get()->hs_source_files, 0);
	sprintf(script_directory_name, "data\\%s", tag_get_name(global_scenario_index));
	sprintf(strrchr(script_directory_name, '\\') + 1, "scripts");
	file_reference_create_from_path(&global_scripts_file, HS_GLOBAL_SCRIPTS_FILE_PATH, FALSE);

	if (file_exists(&global_scripts_file))
	{
		success = hs_rebuild_source_file(&global_scripts_file);
	}

	file_reference_create_from_path(&script_directory, script_directory_name, TRUE);
	script_file_count = (short)find_files(0, &script_directory, MAXIMUM_HS_SOURCE_FILES_PER_SCENARIO, script_files);
	qsort(script_files, script_file_count, sizeof(*script_files), alphabetize_file_references);

	for (script_file_index = 0; script_file_index<script_file_count; script_file_index++)
	{
		file_reference_get_name(&script_files[script_file_index], FLAG(_name_extension_bit), extension);

		if (!strcmp(extension, "hsc") && !hs_rebuild_source_file(&script_files[script_file_index]))
		{
			success = FALSE;
		}
	}

	return success;
}

static void hs_compile_source_error(
	char const *file_name,
	char const *error_message,
	char *error_source,
	char const *source)
{
	char *end_of_line = NULL;

	if (error_source)
	{
		end_of_line = strchr(error_source, '\n');

		if (end_of_line)
		{
			*end_of_line = '\0';
		}
	}

	if (file_name && end_of_line)
	{
		short line_number;

		for (line_number = 1; end_of_line>source; end_of_line--)
		{
			if (*end_of_line=='\n')
			{
				line_number++;
			}
		}

		error(_error_silent, "[%s line %d] %s: %s", file_name, line_number, error_message, error_source);
	}
	else
	{
		error(_error_silent, "%s: %s", error_message, error_source);
	}

	return;
}

static boolean hs_compile_source(
	void)
{
	short source_file_index;
	struct scenario *scenario = global_scenario_get();
	boolean success = TRUE;

	hs_compile_initialize(TRUE);

	for (source_file_index = 0; source_file_index<scenario->hs_source_files.count; source_file_index++)
	{
		char const *error_message;
		char const *error_source;
		struct hs_source_file *source_file = TAG_BLOCK_GET_ELEMENT(
			&scenario->hs_source_files,
			source_file_index,
			struct hs_source_file);

		hs_compile(
			source_file->source.size,
			tag_data_get_pointer(&source_file->source, 0, source_file->source.size),
			&error_message,
			&error_source);

		if (error_message)
		{
			hs_compile_source_error(
				source_file->name,
				error_message,
				(char *)error_source,
				tag_data_get_pointer(&source_file->source, 0, source_file->source.size));
			success = FALSE;
		}
	}

	if (success)
	{
		console_printf(FALSE, "scripts successfully compiled.");
	}

	hs_compile_dispose();

	return success;
}

void hs_help(
	char const *function_name)
{
	char printbuffer[2048];
	short function_index = hs_find_function_by_name(function_name);

	if (function_index!=NONE)
	{
		hs_get_function_parameters_string(function_index, printbuffer);
		console_printf(FALSE, printbuffer);
		hs_get_function_documentation_string(function_index, printbuffer);
		console_printf(FALSE, printbuffer);
	}

	return;
}

void hs_doc(
	void)
{
	short function_index;
	char printbuffer[2048];
	FILE *file = fopen("hs_doc.txt", "w");

	for (function_index = 0; function_index<hs_function_table_count; function_index++)
	{
		hs_function_get(function_index);
		hs_get_function_parameters_string(function_index, printbuffer);
		fprintf(file, "%s\r\n", printbuffer);
		hs_get_function_documentation_string(function_index, printbuffer);
		fprintf(file, "%s\r\n\r\n", printbuffer);
	}

	fclose(file);

	return;
}

static void hs_get_function_parameters_string(
	short function_index,
	char *buffer)
{
	struct hs_function_definition const *definition = hs_function_get(function_index);

	sprintf(buffer, "(%s", definition->name);

	if (definition->parameters)
	{
		sprintf(buffer + strlen(buffer), " %s", definition->parameters);
	}
	else
	{
		short parameter_index;

		for (parameter_index = 0; parameter_index<definition->formal_parameter_count; parameter_index++)
		{
			strcat(buffer, " <");
			strcat(buffer, hs_type_names[definition->formal_parameters[parameter_index]]);
			strcat(buffer, ">");
		}
	}

	strcat(buffer, ")");

	return;
}

static void hs_get_function_documentation_string(
	short function_index,
	char *buffer)
{
	struct hs_function_definition const *function = hs_function_get(function_index);

	strcpy(buffer, function->documentation);

	return;
}

boolean hs_compile_and_evaluate(
	char const *expression)
{
	char expressionbuf[HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES];
	char fakery_buffer[HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES];
	char *c;
	boolean empty;
	boolean success = FALSE;

	strncpy(expressionbuf, expression, HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES);
	expressionbuf[HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES - 1] = '\0';
	c = strchr(expressionbuf, ';');

	if (c)
	{
		expressionbuf[0] = '\0';
	}

	empty = TRUE;

	for (c = expressionbuf; *c; c++)
	{
		if (!isspace(*c))
		{
			empty = FALSE;
			break;
		}
	}

	if (!empty)
	{
		char const *error_message;
		char const *error_source;
		long expression_index;
		short fakery_type = _hs_fakery_none;

		hs_compile_initialize(FALSE);

		if (expressionbuf[0]!='(')
		{
			char *space = strchr(expressionbuf, ' ');

			if (space)
			{
				*space = '\0';
			}

			if (hs_find_global_by_name(expressionbuf)!=NONE)
			{
				if (space)
				{
					fakery_type = _hs_fakery_set;
				}
			}
			else
			{
				fakery_type = _hs_fakery_parenthesize;
			}

			if (space)
			{
				*space = ' ';
			}
		}

		switch (fakery_type)
		{
		case _hs_fakery_none:
			break;
		case _hs_fakery_parenthesize:
			sprintf(fakery_buffer, "(%s)", expressionbuf);
			expression = fakery_buffer;
			break;
		case _hs_fakery_set:
			sprintf(fakery_buffer, "(set %s)", expressionbuf);
			expression = fakery_buffer;
			break;
		default:
			match_halt("c:\\halo\\SOURCE\\hs\\hs.c", 1287);
		}

		expression_index = hs_compile_expression(strlen(expression), expression, &error_message, &error_source);

		if (expression_index!=NONE)
		{
			success = TRUE;
			hs_runtime_evaluate(expression_index);
		}
		else if (error_message)
		{
			hs_compile_source_error(NULL, error_message, (char *)error_source, expression);
		}

		hs_compile_dispose();
	}

	if (recompile)
	{
		hs_hack();
		recompile = FALSE;
	}

	return success;
}
#endif

boolean hs_evaluate_by_name(
	char const *script_name)
{
	boolean success;
	short script_index = hs_find_script_by_name(script_name);

	if (script_index!=NONE)
	{
		struct hs_script *script = hs_script_get(script_index);

		hs_runtime_evaluate(script->root_expression_index);
		success = TRUE;
	}
	else
	{
		success = FALSE;
	}

	return success;
}

#ifdef HS_COMPILER_ENABLED
void hs_hack(
	void)
{
	if (hs_rebuild_source())
	{
		hs_compile_source();
		hs_dispose_from_old_map();
		hs_initialize_for_new_map();
	}

	return;
}
#endif
