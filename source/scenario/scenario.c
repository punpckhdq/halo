/*
SCENARIO.C
*/

/* ---------- headers */

#include "cseries.h"
#include "scenario.h"
#include "render_cameras.h"
#include "structure_bsp_definitions.h"
#include "fog_definitions.h"
#include "sky_definitions.h"
#include "game_globals.h"
#include "game_state.h"
#include "bink_playback.h"
#include "objects.h"
#include "object_types.h"
#include "object_lights.h"
#include "ai.h"
#include "effects.h"
#include "particles.h"
#include "particle_systems.h"
#include "contrails.h"
#include "decals.h"
#include "structures.h"
#include "players.h"
#include "observer.h"
#include "sound_manager.h"
#include "sound_environment_definitions.h"
#include "render_debug.h"
#include "main.h"
#include "collision_bsp_definitions.h"
#include "bsp3d.h"
#include "collision_usage.h"
#include "sound_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct scenario_fog_interpolator
{
	boolean valid;
	real_point3d point;
	real atmospheric_fog_z_near;
	real atmospheric_fog_z_far;
	real atmospheric_fog_maximum_density;
	real_rgb_color atmospheric_fog_color;
	real screen_external_intensity;
};

struct scenario_global_data
{
	short structure_bsp_index;
	struct scenario_fog_interpolator local_players[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	boolean sound_environment_underwater;
	struct sound_environment sound_environment_interpolator;
};

struct memory_status /* fake name */
{
	unsigned long minimum_free;
	unsigned long maximum_free;
};

/* ---------- prototypes */

static void scenario_call_disconnect_from_structure_bsp_procs(void);
static void scenario_call_reconnect_to_structure_bsp_procs(void);
static void interpolate_real_rgb_color(real_rgb_color *current, real_rgb_color *desired, real maximum_speed);

/* ---------- globals */

long global_scenario_index = NONE;
short global_structure_bsp_index = NONE;
static void (*reconnect_to_structure_bsp_procs[])(void) =
{
	objects_reconnect_to_structure_bsp,
	lights_reconnect_to_structure_bsp,
	ai_reconnect_to_structure_bsp,
	effects_reconnect_to_structure_bsp,
	particles_reconnect_to_structure_bsp,
	particle_systems_reconnect_to_structure_bsp,
	contrails_reconnect_to_structure_bsp,
	decals_reconnect_to_structure_bsp,
	structure_decals_reconnect_to_structure_bsp,
	observer_reconnect_to_structure_bsp,
	players_reconnect_to_structure_bsp,
	sound_reconnect_to_structure_bsp,
	object_types_reconnect_to_structure_bsp
};
static void (*disconnect_from_structure_bsp_procs[])(void) =
{
	object_types_disconnect_from_structure_bsp,
	objects_disconnect_from_structure_bsp,
	lights_disconnect_from_structure_bsp,
	ai_disconnect_from_structure_bsp,
	effects_disconnect_from_structure_bsp,
	particles_disconnect_from_structure_bsp,
	particle_systems_disconnect_from_structure_bsp,
	contrails_disconnect_from_structure_bsp,
	structure_decals_disconnect_from_structure_bsp,
	decals_disconnect_from_structure_bsp
};
static struct memory_status scenario_load_memory_status = /* fake name */
{
	UNSIGNED_LONG_MAX,
	0
};
struct scenario_global_data *scenario_globals;
struct game_globals *global_game_globals;
struct bsp3d *global_bsp3d;
struct collision_bsp *global_collision_bsp;
struct structure_bsp *global_structure_bsp;
struct scenario *global_scenario;
boolean debug_sound_environment;

/* ---------- public code */

void scenario_initialize(
	void)
{
	scenario_globals = game_state_malloc("scenario globals", NULL, sizeof(struct scenario_global_data));

	return;
}

void scenario_initialize_for_new_map(
	void)
{
	wind_initialize_for_new_map();
	memset(scenario_globals->local_players, 0, sizeof(struct scenario_fog_interpolator) * MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	scenario_globals->sound_environment_interpolator = default_sound_environment;
	scenario_globals->sound_environment_underwater = FALSE;

	return;
}

void scenario_dispose_from_old_map(
	void)
{
	wind_dispose_from_old_map();

	return;
}

void scenario_frame_update(
	real dt)
{
	wind_update();

	return;
}

boolean scenario_load(
	char const *name)
{
	boolean result = FALSE;

	check_memory_status(&scenario_load_memory_status, "scenario_load");
	global_scenario_index = scenario_tags_load(name);
	if (global_scenario_index != NONE)
	{
		global_scenario = tag_get(SCENARIO_DEFINITION_TAG, global_scenario_index);
		if (global_scenario->structure_bsp_references.count > 0)
		{
			global_game_globals = tag_get(GAME_GLOBALS_DEFINITION_TAG, tag_loaded(GAME_GLOBALS_DEFINITION_TAG, "globals\\globals"));
			if (scenario_switch_structure_bsp(0))
			{
				result = TRUE;
			}
		}
		else
		{
			error(_error_delayed, "scenario doesn't have a structure bsp");
		}
	}
	else
	{
		char *missing_tags = "";

		error(_error_delayed, "need to get the following tags:");
		while (missing_tags)
		{
			char *newline = strchr(missing_tags, '\n');

			if (newline)
			{
				*newline = '\0';
			}
			error(_error_delayed, "%s", missing_tags);
			if (!newline)
			{
				break;
			}
			missing_tags = newline + 1;
			*newline = '\n';
		}
	}

	return result;
}

void scenario_unload(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 164, !bink_playback_active());
	scenario_tags_unload();
	global_scenario_index = NONE;
	global_structure_bsp_index = NONE;
	scenario_globals->structure_bsp_index = NONE;
	global_scenario = NULL;
	global_structure_bsp = NULL;
	global_collision_bsp = NULL;
	global_bsp3d = NULL;
	global_game_globals = NULL;

	return;
}

struct scenario *global_scenario_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 183, global_scenario);

	return global_scenario;
}

struct scenario *global_scenario_try_and_get(
	void)
{
	return global_scenario;
}

struct structure_bsp *global_structure_bsp_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 197, global_structure_bsp);

	return global_structure_bsp;
}

struct collision_bsp *global_collision_bsp_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 205, global_collision_bsp);

	return global_collision_bsp;
}

struct bsp3d *global_bsp3d_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 213, global_bsp3d);

	return global_bsp3d;
}

struct game_globals *scenario_get_game_globals(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 221, global_game_globals);

	return global_game_globals;
}

long global_structure_bsp_tag_index_get(
	void)
{
	struct scenario_structure_bsp_reference *reference = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->structure_bsp_references, global_structure_bsp_index, struct scenario_structure_bsp_reference);

	return reference->structure_bsp.index;
}

void scenario_location_from_point(
	struct location *location,
	real_point3d const *point)
{
	long cluster_index;

	location->leaf_index = scenario_leaf_index_from_point(point);
	if (location->leaf_index == NONE)
	{
		cluster_index = NONE;
	}
	else
	{
		struct structure_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, location->leaf_index & LONG_MAX, struct structure_leaf);

		cluster_index = leaf->cluster_index;
	}
	location->cluster_index = cluster_index;

	return;
}

void scenario_location_from_line(
	struct location *location,
	struct location const *start_location,
	real_point3d const *start_point,
	real_point3d const *end_point)
{
	scenario_location_from_point(location, end_point);

	return;
}

void scenario_location_award_bonus(
	struct location *location)
{
	location->bonus = NONE;

	return;
}

struct material_definition *default_material_definition_get(
	void)
{
	static struct material_definition default_material_definition;
	static boolean initialized;

	if (!initialized)
	{
		default_material_definition.melee_hit_sound.index = NONE;
		initialized = TRUE;
	}

	return &default_material_definition;
}

struct material_definition *scenario_material_definition_get(
	short material_type)
{
	struct game_globals *game_globals = scenario_get_game_globals();
	struct material_definition *result;

	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 286, material_type==NONE || (material_type>=0 && material_type<NUMBER_OF_MATERIAL_TYPES));
	if (material_type >= 0 && material_type < game_globals->materials.count)
	{
		result = TAG_BLOCK_GET_ELEMENT(&game_globals->materials, material_type, struct material_definition);
	}
	else
	{
		result = default_material_definition_get();
	}

	return result;
}

boolean scenario_location_deafening(
	struct location const *location)
{
	struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->clusters, location->cluster_index, struct structure_cluster);
	boolean result = FALSE;

	if (cluster->background_sound_palette_index != NONE && cluster->background_sound_palette_index < global_structure_bsp_get()->background_sound_palette.count)
	{
		struct structure_background_sound_palette_entry *sound = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp->background_sound_palette, cluster->background_sound_palette_index, struct structure_background_sound_palette_entry);

		if (sound->background_sound.index != NONE)
		{
			struct looping_sound_definition *definition = tag_get(LOOPING_SOUND_DEFINITION_TAG, sound->background_sound.index);

			result = TEST_FLAG(definition->flags, _looping_sound_deafening_bit);
		}
	}

	return result;
}

real scenario_fog_at_point(
	struct location const *viewer_location,
	real_point3d const *viewer_point,
	real_point3d const *point)
{
	return 0.f;
}

boolean scenario_illumination_at_point(
	real_point3d const *point,
	real_vector3d *surface_normal,
	real_vector3d *radiosity_vector,
	real_rgb_color *radiosity_color,
	real_rgb_color *diffuse_color)
{
	if (surface_normal)
	{
		*surface_normal = *global_up3d;
	}
	if (radiosity_vector)
	{
		*radiosity_vector = *global_left3d;
	}
	if (radiosity_color)
	{
		*radiosity_color = *global_real_rgb_white;
	}
	if (diffuse_color)
	{
		*diffuse_color = *global_real_rgb_white;
	}

	return TRUE;
}

boolean scenario_ensure_point_within_world(
	real_point3d *point)
{
	short count = 0;

	while (scenario_leaf_index_from_point(point) == NONE && count++ < 150)
	{
		point->z += 0.05f;
	}

	return count == 0;
}

long scenario_leaf_index_from_point(
	real_point3d const *point)
{
	return bsp3d_test_point(global_bsp3d_get(), 0, point);
}

long scenario_get_sky_definition_index(
	short sky_index)
{
	struct scenario *scenario = global_scenario_get();
	long result = NONE;

	if (sky_index >= 0 && sky_index < scenario->sky_references.count)
	{
		struct tag_reference *reference = TAG_BLOCK_GET_ELEMENT(&scenario->sky_references, sky_index, struct tag_reference);

		result = reference->index;
	}

	return result;
}

struct sky *scenario_get_sky(
	short sky_index)
{
	long sky_definition_index = scenario_get_sky_definition_index(sky_index);
	struct sky *result = NULL;

	if (sky_definition_index != NONE)
	{
		result = tag_get(SKY_DEFINITION_TAG, sky_definition_index);
	}

	return result;
}

void scenario_get_atmospheric_fog(
	short local_player_index,
	short sky_index,
	real_point3d *camera_point,
	struct render_fog *render_fog)
{
	struct scenario *scenario = global_scenario_get();
	struct sky *sky = sky_index == NONE ? scenario_get_sky(0) : scenario_get_sky(sky_index);
	struct scenario_fog_interpolator fake_interpolator;
	struct scenario_fog_interpolator *interpolator = local_player_index != NONE ? &scenario_globals->local_players[local_player_index] : &fake_interpolator;

	if (sky)
	{
		struct sky_atmospheric_fog *fog = sky_index == NONE ? &sky->indoor_fog : &sky->outdoor_fog;
		real screen_external_intensity = sky_index == NONE && scenario_get_sky(0)->indoor_fog_plane.index != NONE ? 1.f : 0.f;
		real distance;

		distance = distance3d(&interpolator->point, camera_point);
		if (local_player_index != NONE && distance < 15.f && interpolator->valid && fog->z_far != 0.f && interpolator->atmospheric_fog_z_far != 0.f)
		{
			real maximum_speed;

			interpolate_scalar(&interpolator->atmospheric_fog_z_near, fog->z_near, distance);
			interpolate_scalar(&interpolator->atmospheric_fog_z_far, fog->z_far, distance);
			maximum_speed = distance * 0.05f;
			interpolate_scalar(&interpolator->atmospheric_fog_maximum_density, fog->maximum_density, maximum_speed);
			interpolate_real_rgb_color(&interpolator->atmospheric_fog_color, &fog->color, maximum_speed);
			interpolate_scalar(&interpolator->screen_external_intensity, screen_external_intensity, maximum_speed);
		}
		else
		{
			interpolator->atmospheric_fog_z_near = fog->z_near;
			interpolator->atmospheric_fog_z_far = fog->z_far;
			interpolator->atmospheric_fog_maximum_density = fog->maximum_density;
			interpolator->atmospheric_fog_color = fog->color;
			interpolator->screen_external_intensity = screen_external_intensity;
			interpolator->valid = TRUE;
		}
		interpolator->point = *camera_point;
	}
	render_fog->atmospheric_color = interpolator->atmospheric_fog_color;
	render_fog->atmospheric_maximum_density = interpolator->atmospheric_fog_maximum_density;
	render_fog->atmospheric_minimum_distance = interpolator->atmospheric_fog_z_near;
	render_fog->atmospheric_maximum_distance = interpolator->atmospheric_fog_z_far != 0.f ? MAX(interpolator->atmospheric_fog_z_far, interpolator->atmospheric_fog_z_near + _real_epsilon) : 0.f;
	render_fog->screen_external_intensity = PIN(interpolator->screen_external_intensity, 0.f, 1.f);

	return;
}

boolean scenario_test_pvs(
	short cluster_index0,
	short cluster_index1)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	unsigned long const *pvs = structure_bsp_get_cluster_pvs(structure_bsp, cluster_index0);

	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 468, cluster_index1>=0 && cluster_index1<structure_bsp->clusters.count);

	return BIT_VECTOR_TEST_FLAG(pvs, cluster_index1);
}

boolean scenario_test_pas(
	short cluster_index0,
	short cluster_index1)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	unsigned long const *pvs0 = structure_bsp_get_cluster_pvs(structure_bsp, cluster_index0);
	unsigned long const *pvs1 = structure_bsp_get_cluster_pvs(structure_bsp, cluster_index1);

	return bit_vector_and(structure_bsp->clusters.count, pvs0, pvs1, NULL);
}

boolean scenario_location_potentially_visible_local(
	struct location const *location)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 487, location->cluster_index>=0 && location->cluster_index<global_structure_bsp_get()->clusters.count);

	return BIT_VECTOR_TEST_FLAG(players_get_combined_pvs_local(), location->cluster_index);
}

boolean scenario_location_potentially_visible(
	struct location const *location)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 495, location->cluster_index>=0 && location->cluster_index<global_structure_bsp_get()->clusters.count);

	return BIT_VECTOR_TEST_FLAG(players_get_combined_pvs(), location->cluster_index);
}

short scenario_object_name_index_from_string(
	struct scenario *scenario,
	char const *name)
{
	short i;

	for (i = 0; i < scenario->object_names.count; i++)
	{
		struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(&scenario->object_names, i, struct scenario_object_name);

		if (!strcmp(object_name->name, name))
		{
			return i;
		}
	}

	return NONE;
}

short scenario_get_fog_region_index(
	struct location const *location,
	real_point3d const *position)
{
	short result = NONE;

	if (location->cluster_index != NONE)
	{
		struct structure_bsp *structure_bsp = global_structure_bsp_get();
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, location->cluster_index, struct structure_cluster);
		short fog_designator = cluster->fog_designator;

		if (fog_designator != NONE)
		{
			if (fog_designator & FLAG(SHORT_BITS - 1))
			{
				struct structure_fog_plane *plane = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_planes, fog_designator & SHORT_MAX, struct structure_fog_plane);
				long fog_index = scenario_fog_region_get_fog_index(plane->region_index);
				real distance_to_water_plane = 0.f;

				if (fog_index != NONE)
				{
					struct fog_definition *fog = fog_definition_get(fog_index);

					if (TEST_FLAG(fog->flags, _fog_definition_is_water_bit))
					{
						distance_to_water_plane = fog->distance_to_water_plane;
					}
				}
				if (!position || plane3d_distance_to_point(&plane->plane, position) + distance_to_water_plane < 0.f)
				{
					result = plane->region_index;
				}
			}
			else
			{
				result = fog_designator & SHORT_MAX;
			}
		}
	}

	return result;
}

long scenario_fog_region_get_fog_index(
	short fog_region_index)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	if (fog_region_index != NONE)
	{
		struct structure_fog_region *region = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_regions, fog_region_index, struct structure_fog_region);

		if (region->fog_palette_index != NONE)
		{
			struct structure_fog_palette_entry *fog = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_palette, region->fog_palette_index, struct structure_fog_palette_entry);

			if (fog->fog.index != NONE)
			{
				return fog->fog.index;
			}
		}
	}

	return NONE;
}

boolean scenario_location_underwater(
	struct location const *location,
	real_point3d const *position,
	short *optional_weather_palette_index)
{
	boolean result = FALSE;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short weather_palette_index;
	short fog_region_index = scenario_get_fog_region_index(location, position);

	weather_palette_index = NONE;
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 600, location);
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 601, position);
	if (fog_region_index != NONE)
	{
		struct structure_fog_region *region = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_regions, fog_region_index, struct structure_fog_region);
		long fog_index = scenario_fog_region_get_fog_index(fog_region_index);

		if (fog_index != NONE)
		{
			struct fog_definition *fog = fog_definition_get(fog_index);

			result = TEST_FLAG(fog->flags, _fog_definition_is_water_bit);
		}
		weather_palette_index = region->weather_palette_index;
	}
	if (weather_palette_index == NONE && location->cluster_index != NONE)
	{
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, location->cluster_index, struct structure_cluster);

		weather_palette_index = cluster->weather_palette_index;
	}
	if (optional_weather_palette_index)
	{
		*optional_weather_palette_index = weather_palette_index;
	}

	return result;
}

real scenario_location_water_depth(
	struct location const *location,
	real_point3d const *position)
{
	real result = REAL_MIN;

	if (location->cluster_index != NONE)
	{
		struct structure_bsp *structure_bsp = global_structure_bsp_get();
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, location->cluster_index, struct structure_cluster);
		short fog_designator = cluster->fog_designator;

		if (fog_designator != NONE)
		{
			short fog_region_index;
			real_plane3d *plane;
			long fog_index;

			if (fog_designator & FLAG(SHORT_BITS - 1))
			{
				struct structure_fog_plane *fog_plane = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_planes, fog_designator & SHORT_MAX, struct structure_fog_plane);

				fog_region_index = fog_plane->region_index;
				plane = &fog_plane->plane;
			}
			else
			{
				fog_region_index = fog_designator & SHORT_MAX;
				plane = NULL;
			}
			fog_index = scenario_fog_region_get_fog_index(fog_region_index);
			if (fog_index != NONE)
			{
				struct fog_definition *fog = fog_definition_get(fog_index);

				if (TEST_FLAG(fog->flags, _fog_definition_is_water_bit))
				{
					if (plane)
					{
						result = -(plane3d_distance_to_point(plane, position) + fog->distance_to_water_plane);
					}
					else
					{
						result = REAL_MAX;
					}
				}
			}
		}
	}

	return result;
}

boolean scenario_switch_structure_bsp(
	short structure_bsp_index)
{
	boolean result = FALSE;

	if (structure_bsp_index != global_structure_bsp_index && structure_bsp_index >= 0 && structure_bsp_index < global_scenario->structure_bsp_references.count)
	{
		struct scenario_structure_bsp_reference *reference = TAG_BLOCK_GET_ELEMENT(&global_scenario->structure_bsp_references, structure_bsp_index, struct scenario_structure_bsp_reference);
		boolean reconnect = FALSE;

		match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 695, global_scenario);
		main_stop_time();
		collision_log_enable(FALSE);
		if (global_structure_bsp_index != NONE)
		{
			scenario_call_disconnect_from_structure_bsp_procs();
			reconnect = TRUE;
			scenario_structure_bsp_unload(TAG_BLOCK_GET_ELEMENT(&global_scenario->structure_bsp_references, global_structure_bsp_index, struct scenario_structure_bsp_reference));
			scenario_globals->structure_bsp_index = NONE;
			global_structure_bsp_index = NONE;
		}
		if (scenario_structure_bsp_load(reference))
		{
			global_structure_bsp = tag_get(STRUCTURE_BSP_DEFINITION_TAG, reference->structure_bsp.index);
			global_collision_bsp = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp->collision_bsp, 0, struct collision_bsp);
			global_bsp3d = (struct bsp3d *)TAG_BLOCK_GET_ELEMENT(&global_structure_bsp->collision_bsp, 0, struct collision_bsp);
			scenario_globals->structure_bsp_index = structure_bsp_index;
			global_structure_bsp_index = structure_bsp_index;
			if (reconnect)
			{
				scenario_call_reconnect_to_structure_bsp_procs();
			}
			result = TRUE;
		}
		else
		{
			error(_error_immediate, "failed to load structure bsp '%s'", reference->structure_bsp.name);
		}
		collision_log_enable(TRUE);
		main_start_time();
	}

	return result;
}

void scenario_reload_structure_bsp_if_necessary(
	void)
{
	if (scenario_globals->structure_bsp_index != global_structure_bsp_index)
	{
		scenario_structure_bsp_unload(TAG_BLOCK_GET_ELEMENT(&global_scenario->structure_bsp_references, global_structure_bsp_index, struct scenario_structure_bsp_reference));
		global_structure_bsp_index = NONE;
		scenario_switch_structure_bsp(scenario_globals->structure_bsp_index);
	}

	return;
}

short scenario_get_structure_reference_index_from_tag_index(
	struct scenario *scenario,
	long structure_bsp_index)
{
	char const *name = tag_get_name(structure_bsp_index);
	short result = NONE;
	short i;

	for (i = 0; i < scenario->structure_bsp_references.count; i++)
	{
		struct scenario_structure_bsp_reference *reference = TAG_BLOCK_GET_ELEMENT(&scenario->structure_bsp_references, i, struct scenario_structure_bsp_reference);

		if (!strcmp(name, reference->structure_bsp.name))
		{
			result = i;
			break;
		}
	}

	return result;
}

boolean scenario_trigger_volume_test_point(
	short trigger_volume_index,
	real_point3d const *position)
{
	struct scenario_trigger_volume *volume = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->trigger_volumes, trigger_volume_index, struct scenario_trigger_volume);

	switch (volume->type)
	{
	case _trigger_volume_type_world_aligned_bounding_box:
		return position->x > volume->world_aligned_bounding_box.rectangle.x0 &&
			position->y > volume->world_aligned_bounding_box.rectangle.y0 &&
			position->z > volume->world_aligned_bounding_box.rectangle.z0 &&
			position->x < volume->world_aligned_bounding_box.rectangle.x1 &&
			position->y < volume->world_aligned_bounding_box.rectangle.y1 &&
			position->z < volume->world_aligned_bounding_box.rectangle.z1;
	case _trigger_volume_type_bounding_box:
	{
		real_matrix4x3 matrix;
		real_point3d transformed_point;

		matrix4x3_from_point_and_vectors(&matrix, &volume->bounding_box.position, &volume->bounding_box.forward, &volume->bounding_box.up);
		matrix4x3_inverse_transform_point(&matrix, position, &transformed_point);
		return transformed_point.x > 0.f && transformed_point.y > 0.f && transformed_point.z > 0.f &&
			transformed_point.x < volume->bounding_box.extents.i &&
			transformed_point.y < volume->bounding_box.extents.j &&
			transformed_point.z < volume->bounding_box.extents.k;
	}
	default:
		match_vassert("c:\\halo\\SOURCE\\scenario\\scenario.c", 817, FALSE, NULL);
	}

	return FALSE;
}

boolean scenario_trigger_volume_test_object(
	short trigger_volume_index,
	long object_index)
{
	boolean result = FALSE;

	if (object_index != NONE)
	{
		struct object_datum *object = object_get_and_verify_type(object_index, UNSIGNED_LONG_MAX);

		result = scenario_trigger_volume_test_point(trigger_volume_index, &object->object.bounding_sphere_center);
	}

	return result;
}

void scenario_get_sound_environment(
	long *background_sound_index,
	struct sound_environment **sound_environment,
	boolean *crossed_water_boundary)
{
	long environment_index = NONE;
	long sound_index = NONE;
	short priority = SHORT_MIN;
	boolean sound_environment_underwater = FALSE;
	short local_player_index;
	struct sound_environment *desired;
	struct sound_environment *current;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		if (local_player_get_player_index(local_player_index) != NONE)
		{
			struct observer_result const *camera = observer_get_camera(local_player_index);

			if (camera->location.cluster_index != NONE)
			{
				struct structure_bsp const *structure_bsp = global_structure_bsp_get();
				struct structure_cluster const *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, camera->location.cluster_index, struct structure_cluster);
				long fog_index = scenario_fog_region_get_fog_index(scenario_get_fog_region_index(&camera->location, &camera->position));

				if (fog_index != NONE)
				{
					struct fog_definition const *fog = fog_definition_get(fog_index);
					long candidate = fog->sound_environment.index;

					if (candidate != NONE && sound_environment_get(candidate)->priority > priority)
					{
						priority = sound_environment_get(candidate)->priority;
						environment_index = candidate;
						sound_index = fog->background_sound.index;
						sound_environment_underwater = TEST_FLAG(fog_definition_get(fog_index)->flags, _fog_definition_is_water_bit);
					}
				}
				if (cluster->sound_environment_palette_index != NONE)
				{
					long candidate = TAG_BLOCK_GET_ELEMENT(&structure_bsp->sound_environment_palette, cluster->sound_environment_palette_index, struct structure_sound_environment_palette_entry)->sound_environment.index;

					if (candidate != NONE && sound_environment_get(candidate)->priority > priority)
					{
						environment_index = candidate;
						priority = sound_environment_get(candidate)->priority;
						sound_environment_underwater = FALSE;
						if (cluster->background_sound_palette_index == NONE || cluster->background_sound_palette_index >= structure_bsp->background_sound_palette.count)
						{
							sound_index = NONE;
						}
						else
						{
							sound_index = TAG_BLOCK_GET_ELEMENT(&structure_bsp->background_sound_palette, cluster->background_sound_palette_index, struct structure_background_sound_palette_entry)->background_sound.index;
						}
					}
				}
			}
		}
	}
	if (debug_sound_environment)
	{
		sprintf(temporary, "|n|n|n|n%s", environment_index == NONE ? "no sound environment" : tag_get_name(environment_index));
		render_debug_string(FALSE, temporary);
	}
	desired = sound_environment_get(environment_index);
	current = &scenario_globals->sound_environment_interpolator;
	if (sound_environment_underwater != scenario_globals->sound_environment_underwater)
	{
		*current = *desired;
		scenario_globals->sound_environment_underwater = sound_environment_underwater;
		*crossed_water_boundary = TRUE;
	}
	else
	{
		interpolate_scalar(&current->room_intensity, desired->room_intensity, 0.03f);
		interpolate_scalar(&current->room_intensity_hf, desired->room_intensity_hf, 0.03f);
		interpolate_scalar(&current->room_rolloff_factor, desired->room_rolloff_factor, 0.3f);
		interpolate_scalar(&current->decay_time, desired->decay_time, 0.1f);
		interpolate_scalar(&current->decay_hf_ratio, desired->decay_hf_ratio, 0.03f);
		interpolate_scalar(&current->reflections_intensity, desired->reflections_intensity, 0.03f);
		interpolate_scalar(&current->reflections_delay, desired->reflections_delay, 0.09f);
		interpolate_scalar(&current->reverb_intensity, desired->reverb_intensity, 0.03f);
		interpolate_scalar(&current->reverb_delay, desired->reverb_delay, 0.003f);
		interpolate_scalar(&current->diffusion, desired->diffusion, 0.03f);
		interpolate_scalar(&current->density, desired->density, 0.03f);
		interpolate_scalar(&current->hf_reference, desired->hf_reference, 600.f);
		*crossed_water_boundary = FALSE;
	}
	*background_sound_index = sound_index;
	*sound_environment = current;

	return;
}

void scenario_debug_to_file(
	FILE *stream)
{
	if (global_scenario_index != NONE)
	{
		struct data_iterator iterator;
		struct player_datum *player;

		fprintf(stream, "\"%s\" bsp \"%s\" (#%d)\n", tag_get_name(global_scenario_index), tag_get_name(TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->structure_bsp_references, global_structure_bsp_index, struct scenario_structure_bsp_reference)->structure_bsp.index), global_structure_bsp_index);
		data_iterator_new(&iterator, player_data);
		while (player = data_iterator_next(&iterator))
		{
			fprintf(stream, "player 0x%08x", iterator.index);
			if (player->unit_index != NONE)
			{
				struct object_datum *unit = object_get_and_verify_type(player->unit_index, _object_mask_unit);

				fprintf(stream, " at (%.2f,%.2f,%.2f) (leaf#%d,cluster#%d)\n", unit->object.bounding_sphere_center.x, unit->object.bounding_sphere_center.y, unit->object.bounding_sphere_center.z, unit->object.location.leaf_index, unit->object.location.cluster_index);
			}
			else
			{
				fprintf(stream, " dead\n");
			}
		}
	}
	else
	{
		fprintf(stream, "<no scenario loaded>\n");
	}

	return;
}

short global_structure_bsp_index_get(
	void)
{
	return global_structure_bsp_index;
}

/* ---------- private code */

static __inline void scenario_call_disconnect_from_structure_bsp_procs(
	void)
{
	short i;

	for (i = 0; i < NUMBEROF(disconnect_from_structure_bsp_procs); i++)
	{
		disconnect_from_structure_bsp_procs[i]();
	}

	return;
}

static __inline void scenario_call_reconnect_to_structure_bsp_procs(
	void)
{
	short i;

	for (i = 0; i < NUMBEROF(reconnect_to_structure_bsp_procs); i++)
	{
		reconnect_to_structure_bsp_procs[i]();
	}

	return;
}

static void interpolate_real_rgb_color(
	real_rgb_color *current,
	real_rgb_color *desired,
	real maximum_speed)
{
	interpolate_scalar(&current->red, desired->red, maximum_speed);
	interpolate_scalar(&current->green, desired->green, maximum_speed);
	interpolate_scalar(&current->blue, desired->blue, maximum_speed);

	return;
}
