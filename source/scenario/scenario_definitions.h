/*
SCENARIO_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __SCENARIO_DEFINITIONS_H
#define __SCENARIO_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "tag_files/tag_files.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	_scenario_object_placement_not_automatic_bit= 0,
	_scenario_object_placement_not_on_easy_bit,
	_scenario_object_placement_not_on_normal_bit,
	_scenario_object_placement_not_on_hard_bit,
	NUMBER_OF_SCENARIO_OBJECT_LOCATION_PLACEMENT_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct scenario_object_palette_entry
{
	struct tag_reference reference;
	unsigned long unused[8];
};

struct scenario_object_datum
{
	short palette_entry_index;
	short name_index;
	word placement_flags;
	short variant_number;
	real_point3d position;
	real_euler_angles3d rotation;
	word on_bsp_flags;
	word misc_flags;
	unsigned long unused;
};

struct scenario_placeholder_datum
{
	struct scenario_object_datum object;
};

struct scenario_object_name
{
	char name[TAG_STRING_LENGTH+1];
	short runtime_object_type;
	short runtime_scenario_datum_index;
};

struct scenario_cutscene_camera_point
{
	long flags;
	char name[TAG_STRING_LENGTH+1];
	long pad;
	real_point3d position;
	real_euler_angles3d orientation;
	real field_of_view;
	long unused[9];
};

struct scenario
{
	struct tag_reference ugly_structure_bsp;
	struct tag_reference unloved_globals;
	struct tag_reference bad_sky;
	struct tag_block sky_references;
	short type;
	word flags;
	struct tag_block scenario_references;
	real local_north;
	unsigned long header_unused[5];
	long reference_unused[34];
	struct tag_block predicted_ui_resources;
	struct tag_block functions;
	struct tag_data editor_scenario_data;
	struct tag_block comments;
	long user_edit_unused[56];
	struct tag_block object_names;					// scenario_object_name
	struct tag_block scenery;
	struct tag_block scenery_palette;
	struct tag_block bipeds;
	struct tag_block biped_palette;
	struct tag_block vehicles;
	struct tag_block vehicle_palette;
	struct tag_block equipment;
	struct tag_block equipment_palette;
	struct tag_block weapons;
	struct tag_block weapon_palette;
	struct tag_block device_groups;
	struct tag_block machines;
	struct tag_block machine_palette;
	struct tag_block controls;
	struct tag_block control_palette;
	struct tag_block light_fixtures;
	struct tag_block light_fixtures_palette;
	struct tag_block sound_scenery;
	struct tag_block sound_scenery_palette;
	struct tag_block unused_blocks[7];
	struct tag_block starting_profiles;
	struct tag_block players;
	struct tag_block trigger_volumes;
	struct tag_block recorded_animations;
	struct tag_block netgame_flags;
	struct tag_block netgame_equipment;
	struct tag_block scenario_starting_equipment;
	struct tag_block bsp_switch_trigger_volumes;
	struct tag_block decals;
	struct tag_block decal_palette;
	struct tag_block detail_object_collection_palette;
	long render_unused[21];
	struct tag_block ai_actor_palette;
	struct tag_block ai_encounters;
	struct tag_block ai_command_lists;
	struct tag_block ai_animation_references;
	struct tag_block ai_script_references;
	struct tag_block ai_recording_references;
	struct tag_block ai_conversations;
	struct tag_data hs_syntax_data;
	struct tag_data hs_string_constants;
	struct tag_block hs_scripts;
	struct tag_block hs_globals;
	struct tag_block hs_references;
	struct tag_block hs_source_files;
	long scripting_unused[6];
	struct tag_block cutscene_flags;
	struct tag_block cutscene_camera_points;			// scenario_cutscene_camera_point
	struct tag_block cutscene_chapter_titles;
	long rapidly_dwindling_unused_space[27];
	struct tag_reference custom_object_names;
	struct tag_reference ingame_help_text;
	struct tag_reference hud_messages;
	struct tag_block structure_bsp_references;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __SCENARIO_DEFINITIONS_H
