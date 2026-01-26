/*
SCENARIO.H

header included in hcex build.
*/

#ifndef __SCENARIO_H
#define __SCENARIO_H
#pragma once


/* ---------- headers */

#include "real_math.h"

/* ---------- constants */

enum
{
	MAXIMUM_SCENARIO_PLAYERS_PER_BLOCK= 0x100,
	MAXIMUM_TRIGGER_VOLUMES_PER_SCENARIO= 0x100,
	MAXIMUM_FUNCTIONS_PER_SCENARIO= 0x20,
	MAXIMUM_OBJECT_NAMES_PER_SCENARIO= 0x200,
	MAXIMUM_DEVICE_GROUPS_PER_SCENARIO= 0x80,
	MAXIMUM_SCENARIO_OBJECT_PALETTE_ENTRIES_PER_BLOCK= 0x64,
	MAXIMUM_CUTSCENE_FLAGS_PER_SCENARIO= 0x200,
	MAXIMUM_CUTSCENE_CAMERA_POINTS_PER_SCENARIO= 0x200,
	MAXIMUM_CUTSCENE_TITLES_PER_SCENARIO= 0x40,
	MAXIMUM_SCENARIO_NETGAME_FLAGS_PER_SCENARIO= 0xC8,
	MAXIMUM_SCENARIO_NETGAME_EQUIPMENT_PER_SCENARIO= 0xC8,
	MAXIMUM_SCENARIO_STARTING_EQUIPMDNG_PER_SCENARIO= 0xC8,
	MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO= 0x10,
	MAXIMUM_CHILD_SCENARIOS_PER_SCENARIO= 0x10,
	MAXIMUM_SKIES_PER_SCENARIO= 0x8,
	MAXIMUM_DECALS_PER_SCENARIO= 0x10000,
	MAXIMUM_DECAL_PALETTES_PER_SCENARIO= 0x80,
	MAXIMUM_SCENERY_DATUMS_PER_SCENARIO= 0x7D0,
	MAXIMUM_SOUND_SCENERY_DATUMS_PER_SCENARIO= 0x100,
	MAXIMUM_WEAPON_DATUMS_PER_SCENARIO= 0x80,
	MAXIMUM_EQUIPMENT_DATUMS_PER_SCENARIO= 0x100,
	MAXIMUM_BIPED_DATUMS_PER_SCENARIO= 0x80,
	MAXIMUM_VEHICLE_DATUMS_PER_SCENARIO= 0x50,
	MAXIMUM_MACHINE_DATUMS_PER_SCENARIO= 0x190,
	MAXIMUM_CONTROL_DATUMS_PER_SCENARIO= 0x64,
	MAXIMUM_LIGHT_FIXTURE_DATUMS_PER_SCENARIO= 0x1F4,
	MAXIMUM_EDITOR_SCENARIO_DATA_SIZE= 0x10000,
	MAXIMUM_EDITOR_COMMENTS= 0x400,
	MAXIMUM_EDITOR_COMMENT_LENGTH= 0x4000,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/SCENARIO.C */

void scenario_initialize(void);
void scenario_initialize_for_new_map(void);
void scenario_dispose_from_old_map(void);
void scenario_frame_update(real dt);
void scenario_unload(void);
struct scenario *global_scenario_get(void);
struct scenario *global_scenario_try_and_get(void);
struct structure_bsp *global_structure_bsp_get(void);
struct collision_bsp *global_collision_bsp_get(void);
struct bsp3d *global_bsp3d_get(void);
struct game_globals *scenario_get_game_globals(void);
long global_structure_bsp_tag_index_get(void);
void scenario_location_award_bonus(struct location *location);
struct material_definition *default_material_definition_get(void);
struct material_definition *scenario_material_definition_get(short material_type);
boolean scenario_location_deafening(const struct location *location);
real scenario_fog_at_point(const struct location *viewer_location, const real_point3d *viewer_point, const real_point3d *point);
boolean scenario_illumination_at_point(const real_point3d *point, real_vector3d *surface_normal, real_vector3d *radiosity_vector, real_rgb_color *radiosity_color, real_rgb_color *diffuse_color);
long scenario_leaf_index_from_point(const real_point3d *point);
long scenario_get_sky_definition_index(short sky_index);
struct sky *scenario_get_sky(short sky_index);
boolean scenario_test_pvs(short cluster_index0, short cluster_index1);
boolean scenario_test_pas(short cluster_index0, short cluster_index1);
boolean scenario_location_potentially_visible_local(const struct location *location);
boolean scenario_location_potentially_visible(const struct location *location);
short scenario_object_name_index_from_string(struct scenario *scenario, const char *name);
long scenario_fog_region_get_fog_index(short fog_region_index);
boolean scenario_switch_structure_bsp(short structure_bsp_index);
void scenario_reload_structure_bsp_if_necessary(void);
short scenario_get_structure_reference_index_from_tag_index(struct scenario *scenario, long structure_bsp_index);
boolean scenario_trigger_volume_test_point(short trigger_volume_index, const real_point3d *position);
boolean scenario_trigger_volume_test_object(short trigger_volume_index, long object_index);
void scenario_debug_to_file(FILE *stream);
short global_structure_bsp_index_get(void);
boolean scenario_load(const char *name);
void scenario_location_from_point(struct location *location, const real_point3d *point);
void scenario_location_from_line(struct location *location, const struct location *start_location, const real_point3d *start_point, const real_point3d *end_point);
boolean scenario_ensure_point_within_world(real_point3d *point);
short scenario_get_fog_region_index(const struct location *location, const real_point3d *position);
boolean scenario_location_underwater(const struct location *location, const real_point3d *position, short *optional_weather_palette_index);
real scenario_location_water_depth(const struct location *location, const real_point3d *position);
void scenario_get_sound_environment(long *background_sound_index, long *sound_environment_tag, boolean *crossed_water_boundary);
void scenario_get_atmospheric_fog(short local_player_index, short sky_index, real_point3d *camera_point, struct render_fog *render_fog);

/* ---------- globals */

extern long global_scenario_index;
extern short global_structure_bsp_index;

/* comm */
struct scenario *global_scenario;

/* comm */
struct bsp3d *global_bsp3d;

/* ---------- public code */

#endif // __SCENARIO_H
