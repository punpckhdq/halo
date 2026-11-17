/*
HS.H

header included in hcex build.
*/

#ifndef __HS_H
#define __HS_H
#pragma once

/* ---------- constants */

enum
{
	HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES = 0x400
};

enum
{
	_hs_unparsed = 0,
	_hs_special_form,
	_hs_function_name,
	_hs_passthrough,

	FIRST_HS_TYPE,
	_hs_type_void = FIRST_HS_TYPE,
	_hs_type_boolean,
	_hs_type_real,
	_hs_type_short_integer,
	_hs_type_long_integer,
	_hs_type_string,
	_hs_type_script,
	_hs_type_trigger_volume,
	_hs_type_cutscene_flag,
	_hs_type_cutscene_camera_point,
	_hs_type_cutscene_title,
	_hs_type_cutscene_recording,
	_hs_type_device_group,
	_hs_type_ai,
	_hs_type_ai_command_list,
	_hs_type_starting_profile,
	_hs_type_conversation,
	_hs_type_navpoint,
	_hs_type_hud_message,
	_hs_type_object_list,

	FIRST_HS_TAG_REFERENCE_TYPE,
	_hs_type_sound = FIRST_HS_TAG_REFERENCE_TYPE,
	_hs_type_effect,
	_hs_type_damage,
	_hs_type_looping_sound,
	_hs_type_animation_graph,
	_hs_type_actor_variant,
	_hs_type_damage_effect,
	_hs_type_object_definition,
	NOT_LAST_HS_TAG_REFERENCE_TYPE,
	NUMBER_OF_HS_TAG_REFERENCE_TYPES = NOT_LAST_HS_TAG_REFERENCE_TYPE - FIRST_HS_TAG_REFERENCE_TYPE,
	LAST_HS_TAG_REFERENCE_TYPE = NOT_LAST_HS_TAG_REFERENCE_TYPE - 1,

	FIRST_HS_ENUM_TYPE = NOT_LAST_HS_TAG_REFERENCE_TYPE,
	_hs_type_enum_game_difficulty = FIRST_HS_ENUM_TYPE,
	_hs_type_enum_team,
	_hs_type_enum_ai_default_state,
	_hs_type_enum_actor_type,
	_hs_type_enum_hud_corner,
	NOT_LAST_HS_ENUM_TYPE,
	NUMBER_OF_HS_ENUM_TYPES = NOT_LAST_HS_ENUM_TYPE - FIRST_HS_ENUM_TYPE,
	LAST_HS_ENUM_TYPE = NOT_LAST_HS_ENUM_TYPE - 1,

	FIRST_HS_OBJECT_TYPE = NOT_LAST_HS_ENUM_TYPE,
	_hs_type_object = FIRST_HS_OBJECT_TYPE,
	_hs_type_unit,
	_hs_type_vehicle,
	_hs_type_weapon,
	_hs_type_device,
	_hs_type_scenery,
	FIRST_HS_OBJECT_NAME_TYPE,
	LAST_HS_OBJECT_TYPE = FIRST_HS_OBJECT_NAME_TYPE - 1,

	_hs_type_object_name = FIRST_HS_OBJECT_NAME_TYPE,
	NUMBER_OF_HS_OBJECT_TYPES = FIRST_HS_OBJECT_NAME_TYPE - FIRST_HS_OBJECT_TYPE,
	LAST_HS_OBJECT_NAME_TYPE = FIRST_HS_OBJECT_NAME_TYPE + NUMBER_OF_HS_OBJECT_TYPES - 1,

	NUMBER_OF_HS_NODE_TYPES
};

enum
{
	_hs_script_startup = 0,
	_hs_script_dormant,
	_hs_script_continuous,
	_hs_script_static,
	_hs_script_stub,
	NUMBER_OF_HS_SCRIPT_TYPES
};

enum
{
	_hs_syntax_node_primitive_bit = 0,
	_hs_syntax_node_script_bit,
	_hs_syntax_node_variable_bit,
	_hs_syntax_node_permanent_bit,
	NUMBER_OF_HS_SYNTAX_NODE_FLAGS
};

enum
{
	_hs_token_type_form = 0,
	_hs_token_type_script_type,
	_hs_token_type_type,
	_hs_token_type_function,
	_hs_token_type_script,
	_hs_token_type_variable,
	_hs_token_type_ai,
	_hs_token_type_ai_command_list,
	_hs_token_type_starting_profile,
	_hs_token_type_conversation,
	_hs_token_type_object,
	_hs_token_type_trigger_volume,
	_hs_token_type_cutscene_flag,
	_hs_token_type_cutscene_camera_point,
	_hs_token_type_cutscene_title,
	_hs_token_type_cutscene_recording,
	_hs_token_type_navpoint,
	_hs_token_type_hud_message,
	NUMBER_OF_HS_TOKEN_TYPES
};

enum
{
	HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT = 15 /* fake name */
};

/* ---------- macros */

#define hs_type_valid(type) ((type)>=FIRST_HS_TYPE && (type)<NUMBER_OF_HS_NODE_TYPES)
#define HS_TYPE_IS_TAG_REFERENCE(type) ((type)>=FIRST_HS_TAG_REFERENCE_TYPE && (type)<=LAST_HS_TAG_REFERENCE_TYPE)
#define HS_TYPE_IS_ENUM(type) ((type)>=FIRST_HS_ENUM_TYPE && (type)<=LAST_HS_ENUM_TYPE)
#define HS_TYPE_IS_OBJECT(type) ((type)>=FIRST_HS_OBJECT_TYPE && (type)<=LAST_HS_OBJECT_TYPE)
#define HS_TYPE_IS_OBJECT_NAME(type) ((type)>=FIRST_HS_OBJECT_NAME_TYPE && (type)<=LAST_HS_OBJECT_NAME_TYPE)

#define hs_syntax_get(index) ((struct hs_syntax_node *)datum_get(hs_syntax_data, (index)))
#define HS_GLOBAL_INDEX(designator) ((short)(designator)&MASK(HS_GLOBAL_DESIGNATOR_EXTERNAL_BIT))

#ifdef HS_COMPILER_ENABLED
#define HS_GLOBAL_EXTERNAL(name, type, pointer) static struct hs_global_external name##_definition = {#name, type, pointer} /* fake name */
#else
#define HS_GLOBAL_EXTERNAL(name, type, pointer) static struct hs_global_external name##_definition = {NULL, type, pointer} /* fake name */
#endif

#ifdef DEBUG
#define HS_GLOBAL_EXTERNAL_DEBUG(name, type, pointer) HS_GLOBAL_EXTERNAL(name, type, pointer) /* fake name */
#else
#define HS_GLOBAL_EXTERNAL_DEBUG(name, type, pointer) HS_GLOBAL_EXTERNAL(name, type, NULL) /* fake name */
#endif

/* ---------- structures */

struct hs_syntax_node
{
	short identifier;
	union
	{
		short script_index;
		short function_index;
		short constant_type;
	};
	short type;
	word flags;
	long next_node_index;
	long source_offset;
	unsigned long data;
};

struct hs_global_external
{
	char const *name;
	short type;
	void *pointer;
};

/* ---------- prototypes/HS.C */

void hs_initialize(void);
boolean hs_scenario_merge(struct scenario *master, struct scenario const *slave);
boolean hs_scenario_postprocess(boolean hack);
void hs_initialize_for_new_map(void);
void hs_dispose_from_old_map(void);
void hs_dispose(void);
void hs_recompile(void);
void hs_update(void);
void hs_node_gc(void);
struct hs_function_definition const *hs_function_get(short function_index);
short hs_find_script_by_name(char const *name);
short hs_find_tag_reference_by_index(long tag_index);
struct hs_global_external *hs_global_external_get(short global_index);
short hs_global_get_type(short designator);
char const *hs_global_get_name(short designator);
short hs_find_global_by_name(char const *name);
#ifdef HS_COMPILER_ENABLED
short hs_find_function_by_name(char const *name);
#endif
short hs_tokens_enumerate(char const *substring, long type_flags, char const **results, short maximum_count);
void hs_help(char const *function_name);
void hs_doc(void);
boolean hs_compile_and_evaluate(char const *expression);
boolean hs_evaluate_by_name(char const *script_name);
#ifdef HS_COMPILER_ENABLED
void hs_hack(void);
#endif

/* ---------- prototypes/HS_COMPILE.C */

#ifdef HS_COMPILER_ENABLED

void hs_compile_initialize(boolean permanent);
void hs_compile_dispose(void);
long hs_compile_expression(long source_size, char const *source_data, char const **error_message_pointer, char const **error_source_pointer);
void hs_compile(long source_file_size, char const *source_file_data, char const **error_message_pointer, char const **error_source_pointer);
boolean hs_verify_source_offset(long source_offset);
boolean hs_compile_postprocess(char const **error_message_pointer, char const **error_source_pointer);
boolean hs_parse(long expression_index, short expected_type);
boolean hs_macro_function_parse(short function_index, long expression_index);

#endif // HS_COMPILER_ENABLED

/* ---------- prototypes/HS_RUNTIME.C */

void hs_runtime_initialize(void);
void hs_runtime_initialize_for_new_map(void);
void hs_runtime_dispose_from_old_map(void);
void hs_runtime_dispose(void);
void hs_runtime_update(void);
long hs_runtime_evaluate(long expression_index);
char const *hs_runtime_get_executing_thread_name(void);
boolean hs_wake_by_name(char const *script_name);
void render_debug_scripting(void);
void render_debug_trigger_volumes(void);
void hs_return(long thread_index, long value);
boolean hs_can_cast(short actual_type, short desired_type);
long hs_cast(long thread_index, short actual_type, short desired_type, long value);
long *hs_macro_function_evaluate(short function_index, long thread_index, boolean initialize);

/* ---------- globals */

extern char *hs_type_names[];
extern char *hs_script_type_names[];
extern short const hs_type_sizes[];
extern short const hs_object_type_masks[];
extern long const hs_tag_reference_type_group_tags[];
extern boolean (*const hs_type_primitive_parsers[NUMBER_OF_HS_NODE_TYPES])(long expression_index);

extern boolean const _hs_type_boolean_default;
extern real const _hs_type_real_default;
extern short const _hs_type_short_integer_default;
extern long const _hs_type_long_integer_default;
extern char const *_hs_type_string_default;
extern short const _hs_type_script_default;
extern short const _hs_type_trigger_volume_default;
extern short const _hs_type_cutscene_flag_default;
extern short const _hs_type_cutscene_camera_point_default;
extern short const _hs_type_cutscene_title_default;
extern short const _hs_type_cutscene_recording_default;
extern short const _hs_type_device_group_default;
extern long const _hs_type_ai_default;
extern short const _hs_type_ai_command_list_default;
extern short const _hs_type_starting_profile_default;
extern short const _hs_type_conversation_default;
extern short const _hs_type_navpoint_default;
extern short const _hs_type_hud_message_default;
extern long const _hs_type_object_list_default;
extern long const _hs_type_sound_default;
extern long const _hs_type_looping_sound_default;
extern long const _hs_type_effect_default;
extern long const _hs_type_damage_default;
extern long const _hs_type_animation_graph_default;
extern long const _hs_type_actor_variant_default;
extern long const _hs_type_damage_effect_default;
extern long const _hs_type_object_definition_default;
extern short const _hs_type_enum_game_difficulty_default;
extern short const _hs_type_enum_team_default;
extern short const _hs_type_enum_ai_default_state_default;
extern short const _hs_type_enum_actor_type_default;
extern short const _hs_type_enum_hud_corner_default;
extern short const _hs_type_object_name_default;
extern long const _hs_type_object_default;
extern long const _hs_type_unit_default;
extern long const _hs_type_vehicle_default;
extern long const _hs_type_weapon_default;
extern long const _hs_type_device_default;
extern long const _hs_type_scenery_default;

extern struct hs_global_external *hs_external_globals[];
extern short const hs_external_global_count;

extern boolean debug_trigger_volumes;
extern boolean debug_scripting;
extern unsigned long hs_debug_data[];
extern struct data_array *hs_syntax_data;

#endif // __HS_H
