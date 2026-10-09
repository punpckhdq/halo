/*
SCENARIO_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __SCENARIO_DEFINITIONS_H
#define __SCENARIO_DEFINITIONS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_scenario_cortana_hack_bit = 0,
	_scenario_demo_ui_bit,
	NUMBER_OF_SCENARIO_FLAGS,
};

enum
{
	SCENARIO_GROUP_TAG = 'scnr'
};

enum
{
	_scenario_object_placement_not_automatic_bit = 0,
	_scenario_object_placement_not_on_easy_bit,
	_scenario_object_placement_not_on_normal_bit,
	_scenario_object_placement_not_on_hard_bit,
	NUMBER_OF_SCENARIO_OBJECT_LOCATION_PLACEMENT_FLAGS,
};

enum
{
	_equipment_created_at_rest_bit = 0,
	_equipment_obsolete_bit,
	_equipment_does_accelerate_bit,
	NUMBER_OF_SCENARIO_EQUIPMENT_FLAGS,
};

enum
{
	_trigger_volume_type_world_aligned_bounding_box = 0,
	_trigger_volume_type_bounding_box,
	NUMBER_OF_TRIGGER_VOLUME_TYPES
};

/* ---------- macros */

#define scenario_definition_get(index) ((struct scenario *)tag_get(SCENARIO_GROUP_TAG, index)) /* fake name */

/* ---------- structures */

struct scenario_object_palette_entry
{
	struct tag_reference reference;	// object_definition
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

struct scenario_object_permutation
{
	unsigned long change_colors[4];
	byte region_permutations[8];
	unsigned long unused2[2];
};

struct scenario_scenery_datum
{
	struct scenario_object_datum object;
	struct scenario_object_permutation permutation;
};

struct scenario_placeholder_datum
{
	struct scenario_object_datum object;
};

struct scenario_equipment_datum
{
	struct scenario_object_datum object;
};

struct scenario_sound_scenery_datum /* fake name */
{
	struct scenario_object_datum object;
};

struct scenario_unit_datum
{
	real body_vitality;
	unsigned long flags;
	unsigned long unused[2];
};

struct scenario_biped_datum
{
	struct scenario_object_datum object;
	struct scenario_object_permutation permutation;
	struct scenario_unit_datum unit;
	long unused[8];
};

struct scenario_vehicle_datum
{
	struct scenario_object_datum object;
	struct scenario_object_permutation permutation;
	struct scenario_unit_datum unit;
	byte multiplayer_team_index;
	byte unused_byte;
	word multiplayer_spawn_flags;
	long unused[7];
};

struct scenario_weapon_datum
{
	struct scenario_object_datum object;
	struct scenario_object_permutation permutation;
	short rounds_total;
	short rounds_loaded;
	word flags;
	word pad;
	unsigned long unused[3];
};

struct scenario_device_group
{
	char name[TAG_STRING_LENGTH+1];
	real initial_value;
	unsigned long flags;
	unsigned long unused[3];
};

struct scenario_device_datum
{
	short power_group_index;
	short position_group_index;
	unsigned long flags;
};

struct scenario_machine_datum
{
	struct scenario_object_datum object;
	struct scenario_device_datum device;
	unsigned long flags;
	unsigned long unused[3];
};

struct scenario_control_datum
{
	struct scenario_object_datum object;
	struct scenario_device_datum device;
	unsigned long flags;
	short hud_override_string_list_index;
	short pad;
	unsigned long unused[2];
};

struct scenario_light_fixture_datum
{
	struct scenario_object_datum object;
	struct scenario_device_datum device;
	real_rgb_color color;
	real intensity;
	real falloff_angle;
	real cutoff_angle;
	unsigned long unused[4];
};

struct scenario_object_name
{
	char name[TAG_STRING_LENGTH+1];
	short runtime_object_type;
	short runtime_scenario_datum_index;
};

struct scenario_player
{
	real_point3d position;
	real facing;
	short team_index;
	short bsp_index;
	short game_type[4];
	long unused[6];
};

struct scenario_structure_bsp_reference
{
	long offset;
	long size;
	void *address;
	unsigned long unused[1];
	struct tag_reference structure_bsp;	// structure_bsp
};

struct scenario_trigger_volume
{
	short type;
	word pad;
	char name[TAG_STRING_LENGTH+1];
	union
	{
		struct
		{
			long unused[3];
			real_vector3d forward;
			real_vector3d up;
			real_point3d position;
			real_vector3d extents;
		} bounding_box;
		struct
		{
			long unused[9];
			real_rectangle3d rectangle;
		} world_aligned_bounding_box;
	};
};

struct scenario_cutscene_flag
{
	long flags;
	char name[TAG_STRING_LENGTH+1];
	real_point3d position;
	real_euler_angles2d facing;
	long unused[9];
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

struct scenario_cutscene_title
{
	long flags;
	char name[TAG_STRING_LENGTH+1];
	long pad0;
	rectangle2d bounds;
	short text_index;
	short style;
	short justification;
	short pad1;
	unsigned long text_flags;
	pixel32 foreground_color;
	pixel32 shadow_color;
	real fade_in_time;
	real up_time;
	real fade_out_time;
	long unused[4];
};

struct scenario
{
	struct tag_reference ugly_structure_bsp;	// structure_bsp
	struct tag_reference unloved_globals;	// game_globals
	struct tag_reference bad_sky;			// sky
	struct tag_block sky_references;				// tag_reference
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
	struct tag_block players;						// scenario_player
	struct tag_block trigger_volumes;				// scenario_trigger_volume
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
	struct tag_block ai_encounters;						// encounter_definition
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
	struct tag_reference custom_object_names;	// unicode_string_list_group_header
	struct tag_reference ingame_help_text;	// unicode_string_list_group_header
	struct tag_reference hud_messages;		// hud_state_messages
	struct tag_block structure_bsp_references;		// scenario_structure_bsp_reference
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __SCENARIO_DEFINITIONS_H
