/*
AI_DEBUG.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_debug.h"
#include "ai_globals.h"
#include "actors.h"
#include "actor_types.h"
#include "encounters.h"
#include "props.h"
#include "collision_bsp_definitions.h"
#include "rasterizer.h"
#include "players.h"
#include "render_debug.h"
#include "draw_string.h"
#include "bipeds.h"
#include "observer.h"
#include "dialogue_definitions.h"
#include "director.h"
#include "ai_profile.h"
#include "collisions.h"
#include "console.h"
#include "editor_stubs.h"
#include "ai_script.h"

/* ---------- constants */

/* ---------- macros */

#define ai_debug_render_cross(point, color)									\
{																			\
	p0 = p1 = *(point);														\
	p0.x -= 0.2f;															\
	p1.x += 0.2f;															\
	render_debug_line(TRUE, &p0, &p1, color);								\
	p0.x += 0.2f;															\
	p1.x -= 0.2f;															\
	p0.y -= 0.2f;															\
	p1.y += 0.2f;															\
	render_debug_line(TRUE, &p0, &p1, color);								\
	p0.y += 0.2f;															\
	p1.y -= 0.2f;															\
	p0.z -= 0.2f;															\
	p1.z += 0.2f;															\
	render_debug_line(TRUE, &p0, &p1, color);								\
}

#define actor_debug_print_threat(actor, threat_type, string, color)		\
if (actor->situation.specific_threats[threat_type])						\
{																		\
	render_debug_string_at_point(										\
		TRUE,															\
		ai_debug_drawstack(),											\
		csprintf(														\
			temporary,													\
			string,														\
			(actor)->situation.specific_threats[threat_type],			\
			(actor)->situation.cumulative_threats[threat_type]),		\
		color);															\
}

/* ---------- structures */

/* ---------- prototypes */

static void ai_debug_drawstack_setup(real_point3d const *drawstack_base);
static real_point3d *ai_debug_drawstack(void);

static void ai_debug_highlight_unit(long unit_index, boolean render_exclusive, real_argb_color const *color);

static void ai_debug_render_path_nodes(
	struct path_state *path_state,
	boolean bsp_access_allowed,
	boolean render_all_nodes,
	boolean render_polygons,
	boolean render_costs,
	boolean render_closest);
static void ai_debug_render_surface(struct structure_bsp const *structure_bsp, long surface_index, real offset, real_argb_color const *color);

static void ai_debug_render_actor(long actor_index, boolean render_exclusive, long *history_start_time);
static void ai_debug_render_path_storage(struct path_debug_storage *path);

static void ai_debug_path_storage_update(void);
static void ai_debug_speech_update(void);
static void ai_debug_render_all_actors(boolean render_inactive);
static void ai_debug_render_encounter(long encounter_index);
static void ai_debug_render_path_line(real_point3d const *start_point, short step_count, struct path_step const *steps, real_argb_color const *color);
static void ai_debug_render_path_node(
	struct structure_bsp const *structure_bsp,
	boolean bsp_access_allowed,
	struct path_state const *state,
	struct path_node const *node,
	struct path_node const *child_node,
	real_point3d const *child_point,
	real_argb_color const *path_color,
	real_argb_color const *polygon_color,
	real_argb_color const *cost_color,
	real_argb_color const *attractor_distance_color,
	real_argb_color const *attractor_weight_color,
	real_argb_color const *closest_color);
static void ai_debug_render_paths_failed(void);
static void ai_debug_render_lineoffire(void);
static void ai_debug_render_ballistic_lineoffire(void);
static short ai_debug_lineofsight_findpoint(real_point3d const *point, short cluster_index);
static long ai_debug_lineofsight_storeray(short p0_index, short p1_index);
static void ai_debug_render_lineofsight(void);
static void ai_debug_render_path(void);
static long ai_debug_get_this_actor(void);
static void ai_debug_select_this_actor(void);
static void ai_debug_render_aiming_validity(void);
static void ai_debug_render_speech(void);
static void ai_debug_render_idle_look(void);
static void ai_debug_render_spatial_effects(void);
static void ai_debug_render_vehicles_enterable(void);
static void ai_debug_communication_toggle_bits(
	long name_count,
	char const **names,
	unsigned long *flags,
	unsigned long vector_size,
	short (*lookup)(char const *name));

/* ---------- globals */

struct ai_debug_state ai_debug;

struct actor_debug_info *actor_debug_array = NULL;
struct path_debug_storage *actor_path_debug_array = NULL;

real_point3d global_ai_debug_drawstack_next_position;
real_point3d global_ai_debug_drawstack_last_position;
real global_ai_debug_drawstack_height;
real_argb_color global_temporary_render_color;
long global_ai_debug_firing_position_color_count = NONE;
short global_ai_debug_string_position;

static char const *postcombat_type_strings[NUMBER_OF_ACTOR_POSTCOMBAT_ACTIONS] =
{
	"none",
	"alone",
	"unscathed",
	"wounded",
	"massacre",
	"triumph",
	"run-to",
	"check-enemy",
	"check-friend",
	"shoot-corpse",
	"celebrate"
};

static long global_ai_debug_selected_encounter_index = NONE;
static long global_ai_debug_selected_encounter_time = NONE;
static unsigned long global_ai_debug_activation_cluster_bit_vector[16];

real_argb_color const global_ai_debug_firing_position_colors[] =
{
	{ { 1.f, 1.f, 0.f, 1.f } },
	{ { 1.f, 0.f, 1.f, 1.f } },
	{ { 1.f, 1.f, 0.5f, 0.f } },
	{ { 1.f, 0.f, 1.f, 0.5f } },
	{ { 1.f, 0.5f, 0.f, 1.f } },
	{ { 1.f, 1.f, 0.f, 0.5f } },
	{ { 1.f, 0.5f, 1.f, 0.f } },
	{ { 1.f, 0.f, 0.5f, 1.f } },
	{ { 1.f, 0.5f, 0.f, 0.f } },
	{ { 1.f, 0.f, 0.5f, 0.f } },
	{ { 1.f, 0.f, 0.f, 0.5f } },
	{ { 1.f, 1.f, 1.f, 0.5f } },
	{ { 1.f, 1.f, 0.5f, 1.f } },
	{ { 1.f, 0.5f, 1.f, 1.f } },
	{ { 1.f, 0.5f, 0.5f, 0.f } },
	{ { 1.f, 0.f, 0.5f, 0.5f } },
	{ { 1.f, 0.5f, 0.f, 0.5f } },
	{ { REAL_MAX, REAL_MAX, REAL_MAX, REAL_MAX } }
};

/* ---------- public code */

void ai_debug_initialize(
	void)
{
	memset(&ai_debug, 0, sizeof(ai_debug));
	
	ai_debug.selected_actor_index = NONE;
	ai_debug.selected_encounter_index = NONE;
	ai_debug.last_render_id = 1;
	ai_debug.render = TRUE;

	actor_debug_array = actor_debug_array==NULL ? (struct actor_debug_info *)debug_malloc(sizeof(*actor_debug_array) * MAXIMUM_NUMBER_OF_ACTORS, FALSE, "c:\\halo\\SOURCE\\ai\\ai_debug.c", 147) : actor_debug_array;
	actor_path_debug_array = actor_path_debug_array==NULL ? (struct path_debug_storage *)debug_malloc(sizeof(*actor_path_debug_array) * MAXIMUM_NUMBER_OF_ACTOR_PATHS, FALSE, "c:\\halo\\SOURCE\\ai\\ai_debug.c", 148) : actor_path_debug_array;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 150, actor_debug_array && actor_path_debug_array);

	return;
}

void ai_debug_dispose(
	void)
{
	if (actor_debug_array)
	{
		debug_free(actor_debug_array, "c:\\halo\\SOURCE\\ai\\ai_debug.c", 160);
		actor_debug_array = NULL;
	}

	if (actor_path_debug_array)
	{
		debug_free(actor_path_debug_array, "c:\\halo\\SOURCE\\ai\\ai_debug.c", 166);
		actor_path_debug_array = NULL;
	}

	return;
}

void ai_debug_dispose_from_old_map(
	void)
{
	struct scenario *scenario = global_scenario_try_and_get();

	if (scenario && ai_debug.selected_encounter_index!=NONE)
	{
		struct encounter_definition* encounter = TAG_BLOCK_GET_ELEMENT(
			&scenario->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_debug.selected_encounter_index),
			struct encounter_definition);
		
		strncpy(ai_debug.selected_squad_name, encounter->name, NUMBEROF(ai_debug.selected_squad_name));
		ai_debug.selected_squad_name[NUMBEROF(ai_debug.selected_squad_name)-1] = '\0';
	}
	else
	{
		strcpy(ai_debug.selected_squad_name, "");
	}

	return;
}

void ai_debug_clear_storage(
	void)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 208, actor_debug_array);

	memset(actor_debug_array, 0, sizeof(*actor_debug_array) * MAXIMUM_NUMBER_OF_ACTORS);
	match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 211, actor_path_debug_array)

	memset(actor_path_debug_array, 0, sizeof(*actor_path_debug_array) * MAXIMUM_NUMBER_OF_ACTOR_PATHS);

	return;
}

void ai_debug_actor_deleted(
	long actor_index)
{
	short path_index;

	for (path_index = 0; path_index<MAXIMUM_NUMBER_OF_ACTOR_PATHS; ++path_index)
	{
		struct path_debug_storage *path = &actor_path_debug_array[path_index];

		if (path->valid)
		{
			if (path->actor_index==actor_index)
			{
				path->valid = FALSE;
			}
		}
	}

	return;
}

struct path_debug_storage *ai_debug_get_last_path(
	long actor_index)
{
	short path_index;

	short found_path_index = NONE;
	long found_path_time = NONE;

	for (path_index = 0; path_index<MAXIMUM_NUMBER_OF_ACTOR_PATHS; ++path_index)
	{
		struct path_debug_storage const *path = &actor_path_debug_array[path_index];
	
		if (path->valid && path->actor_index==actor_index && path->path_time>found_path_time)
		{
			found_path_index = path_index;
			found_path_time = path->path_time;
		}
	}

	return found_path_index==NONE ? NULL : &actor_path_debug_array[found_path_index];
}

struct path_debug_storage *ai_debug_get_path_storage(
	long actor_index)
{
	short path_index;

	struct path_debug_storage *found_path = NULL;
	short found_path_index = NONE;

	for (path_index = 0; path_index<MAXIMUM_NUMBER_OF_ACTOR_PATHS; ++path_index)
	{
		struct path_debug_storage const *path = &actor_path_debug_array[path_index];
		
		if (path->actor_index==actor_index && !path->failure)
		{
			found_path_index = path_index;
			break;
		}

		if (found_path_index==NONE && !path->valid)
		{
			found_path_index = path_index;
		}
	}

	if (found_path_index==NONE)
	{
		short best_path_index = NONE;
		long best_path_time = LONG_MAX;

		for (path_index = 0; path_index<MAXIMUM_NUMBER_OF_ACTOR_PATHS; ++path_index)
		{
			struct path_debug_storage const *path = &actor_path_debug_array[path_index];
		
			match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 291, path->valid);

			if (path->path_time < best_path_time)
			{
				best_path_time = path->path_time;
				best_path_index = path_index;
			}
		}

		found_path_index = best_path_index;
	}

	if (found_path_index!=NONE)
	{
		found_path = &actor_path_debug_array[found_path_index];

		memset(found_path, 0, sizeof(*found_path));
		found_path->valid = TRUE;
		found_path->actor_index = actor_index;
		found_path->path_time = game_time_get();
	}

	return found_path;
}

void ai_debug_select_encounter(
	long encounter_index)
{
	if (ai_debug.selected_encounter_index != encounter_index)
	{
		ai_debug.selected_encounter_index = encounter_index;
		ai_debug.firing_position_context_valid = FALSE;
		memset(&ai_debug.firing_position_context, 0, sizeof(ai_debug.firing_position_context));
		memset(&ai_debug.firing_positions, 0, sizeof(ai_debug.firing_positions));

		ai_debug_select_actor(encounter_index, NONE);
	}

	return;
}

void ai_debug_select_actor(
	long encounter_index,
	long actor_index)
{
	if (ai_debug.selected_encounter_index != encounter_index
	|| ai_debug.selected_actor_index != actor_index)
	{
		short firing_position_index;

		ai_debug_select_encounter(encounter_index);
		ai_debug.selected_actor_index = actor_index;
		ai_debug.firing_position_context_valid = FALSE;
	
		for (firing_position_index = 0; firing_position_index<NUMBEROF(ai_debug.firing_positions); ++firing_position_index)
		{
			ai_debug.firing_positions[firing_position_index].evaluated = FALSE;
		}

		ai_debug.idle_look_valid = actor_index != NONE;
		ai_debug.prop_idle_actor_index = actor_index;
		ai_debug.prop_idle_look_count = 0;
	}

	return;
}

void ai_debug_sound_point_set(
	void)
{
	return;
}

void ai_debug_lineoffire_new(
	real_point3d const *origin,
	real_vector3d const *vector)
{
	ai_debug.lineoffire_valid = TRUE;
	ai_debug.lineoffire_success = FALSE;
	ai_debug.lineoffire_origin = *origin;
	ai_debug.lineoffire_vector = *vector;
	ai_debug.lineoffire_numpills = 0;
	return;
}

void ai_debug_lineoffire_addpill(
	real_point3d const *base,
	real_vector3d const *directedheight,
	real width,
	boolean hit)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4036, ai_debug.lineoffire_valid);

	if (ai_debug.lineoffire_numpills<16)
	{
		ai_debug.lineoffire_pillhit[ai_debug.lineoffire_numpills] = hit;
		ai_debug.lineoffire_pillbase[ai_debug.lineoffire_numpills] = *base;
		ai_debug.lineoffire_pilldirectedheight[ai_debug.lineoffire_numpills] = *directedheight;
		ai_debug.lineoffire_pillwidth[ai_debug.lineoffire_numpills++] = width;
	}
	return;
}

void ai_debug_lineoffire_success(
	boolean success)
{
	ai_debug.lineoffire_success = success;
	return;
}

boolean ai_debug_highlight_cluster(
	short index,
	real_argb_color const **highlight_color)
{
	boolean result = FALSE;

	if (ai_debug.render_encounter_activeregion && ai_debug.selected_encounter_index!=NONE)
	{
		if (global_ai_debug_selected_encounter_time != game_time_get() ||
			global_ai_debug_selected_encounter_index != ai_debug.selected_encounter_index)
		{
			encounter_compute_activation_cluster_bit_vector(
				ai_debug.selected_encounter_index,
				FALSE,
				SIZEOF_BITS(global_ai_debug_activation_cluster_bit_vector),
				0,
				global_ai_debug_activation_cluster_bit_vector);
			global_ai_debug_selected_encounter_time = game_time_get();
			global_ai_debug_selected_encounter_index = ai_debug.selected_encounter_index;
		}

		match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4133, highlight_color);
	
		if (BIT_VECTOR_TEST_FLAG(global_ai_debug_activation_cluster_bit_vector, index))
		{
			if (encounter_get(ai_debug.selected_encounter_index)->active)
			{
				*highlight_color = global_real_argb_yellow;
			}
			else
			{
				*highlight_color = global_real_argb_blue;
			}
		}
		else
		{
			*highlight_color = global_real_argb_grey;
		}

		result = TRUE;
	}

	return result;
}

void ai_debug_lineofsight_reset(
	void)
{
	ai_debug.lineofsight_numpoints = 0;
	ai_debug.lineofsight_numrays = 0;
	return;
}

char *ai_debug_describe_actor(
	long actor_index,
	long unit_index,
	boolean include_squad,
	char *buffer,
	long bufsize)
{
	char const *tag_name;

	char actor_string[256];
	char object_name[256];

	strcpy(actor_string, "");
	
	if (include_squad && actor_index!=NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);
		unit_index = actor->meta.unit_index;

		if (actor->meta.encounter_index==NONE)
		{
			csstrcpy(actor_string, "encounterless ");
		}
		else
		{
			struct encounter_definition const *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
				struct encounter_definition);
			struct squad_definition const *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					actor->meta.squad_index,
					struct squad_definition);
			struct platoon_definition const *platoon_definition = actor->meta.platoon_index!=NONE ?
				TAG_BLOCK_GET_ELEMENT(&encounter_definition->platoons, actor->meta.platoon_index, struct platoon_definition) :
				NULL;


			if (platoon_definition==NULL)
			{
				sprintf(actor_string, "%s/%s ", encounter_definition->name, squad_definition->name);
			}
			else
			{
				sprintf(
					actor_string,
					"%s/(%s) %s ",
					encounter_definition->name,
					platoon_definition->name,
					squad_definition->name);
			}
		}
	}

	tag_name = "";
	strcpy(object_name, "");

	if (unit_index!=NONE)
	{
		struct unit_datum const *unit = unit_get(unit_index);
		struct unit_definition const *unit_definition = unit_definition_get(unit->definition_index);

		tag_name = tag_name_strip_path(unit_definition->object.model.name);
	
		if (unit->object.name_index!=NONE)
		{
			struct scenario_object_name const *scenario_object_name = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, unit->object.name_index, struct scenario_object_name);
			sprintf(object_name, " (%s)", scenario_object_name->name);
		}
	}

	_snprintf(buffer, bufsize, "%s%s%s", actor_string, tag_name, object_name);

	return buffer;
}

void ai_debug_vocalize(
	char const *speech_priority_name,
	char const *vocalization_type_name)
{
	if (ai_debug.selected_actor_index!=NONE)
	{
		struct actor_datum const *actor = actor_get(ai_debug.selected_actor_index);
		
		ai_debug.render_speech = TRUE;

		if (actor->meta.unit_index!=NONE)
		{
			short speech_priority = unit_get_speech_priority_by_name(speech_priority_name);
			short vocalization_type = dialogue_get_vocalization_type_by_name(vocalization_type_name);

			if (speech_priority > 0 && vocalization_type != NONE)
			{
				long sound_definition_index_reference = NONE;
				short play_type = unit_test_speech(
						actor->meta.unit_index,
						speech_priority,
						TRUE,
						TRUE,
						NULL,
						&vocalization_type,
						&sound_definition_index_reference);

				if (play_type)
				{
					struct unit_speech_item speech_item;

					memset(&speech_item, 0, sizeof(speech_item));
					
					speech_item.priority = speech_priority;
					speech_item.vocalization_type = vocalization_type;
					speech_item.sound_definition_index = sound_definition_index_reference;

					ai_communication_packet_new(&speech_item.ai);
					unit_speak(actor->meta.unit_index, play_type, &speech_item);
				}
			}
		}
	}

	return;
}

void ai_debug_speak(
	char const *vocalization_type_name)
{
	if (ai_debug.selected_actor_index!=NONE)
	{
		struct actor_datum const *actor = actor_get(ai_debug.selected_actor_index);
		short vocalization_type = dialogue_get_vocalization_type_by_name(vocalization_type_name);

		if (actor->meta.unit_index!=NONE && vocalization_type!=NONE)
		{
			ai_debug.render_speech = TRUE;
			ai_debug.speak_active = TRUE;
			ai_debug.speak_delay_timer = 0;
			ai_debug.speak_list = FALSE;
			ai_debug.speaking_unit_index = actor->meta.unit_index;
			ai_debug.vocalization_type = vocalization_type;
		}
	}

	return;
}

/* ---------- private code */

static void ai_debug_drawstack_setup(
	real_point3d const *drawstack_base)
{
	real_vector3d vector_to_stack;

	struct observer_result const *camera = observer_get_camera(0);

	global_ai_debug_drawstack_last_position = *drawstack_base;
	global_ai_debug_drawstack_next_position = global_ai_debug_drawstack_last_position;

	if (camera)
	{
		vector_from_points3d(&global_ai_debug_drawstack_last_position, &camera->position, &vector_to_stack);
		global_ai_debug_drawstack_height = magnitude3d(&vector_to_stack) / 40.f;
	}
	else
	{
		global_ai_debug_drawstack_height = 0.05f;
	}
	return;
}

static real_point3d *ai_debug_drawstack(
	void)
{
	global_ai_debug_drawstack_last_position = global_ai_debug_drawstack_next_position;
	point_from_line3d(&global_ai_debug_drawstack_last_position, global_up3d, global_ai_debug_drawstack_height, &global_ai_debug_drawstack_next_position);
	return &global_ai_debug_drawstack_last_position;
}

static void ai_debug_highlight_unit(
	long unit_index,
	boolean render_exclusive,
	real_argb_color const *color)
{
	struct biped_datum *biped = biped_try_and_get(unit_index);

	if (biped)
	{
		real_point3d base;
		real width;
		real pill_height;

		struct unit_datum *unit = unit_try_and_get(biped->object.parent_object_index);

		if (unit && unit->unit.driver_object_index==unit_index)
		{
			object_get_bounding_sphere(biped->object.parent_object_index, &base, &width);
			pill_height = 0.f;
		}
		else
		{
			biped_get_physics_pill(unit_index, &base, &pill_height, &width);
		}

		if (render_exclusive && pill_height>0.f)
		{
			real_vector3d height;
			set_real_vector3d(&height, 0.f, 0.f, pill_height);
			render_debug_pill(TRUE, &base, &height, width, color);
		}
		else
		{
			render_debug_sphere(TRUE, &base, 0.75f * width, color);
		}

		if (render_exclusive)
		{
			render_debug_point(TRUE, &base, 1.8f * width, color);
		}
	}

	return;
}

static void ai_debug_render_path_line(
	real_point3d const *start_point,
	short step_count,
	struct path_step const *steps,
	real_argb_color const *color)
{
	short index;

	if (step_count>0)
	{
		render_debug_line_offset(TRUE, start_point, &steps[0].point, color, 0.1f);
	}

	for (index = 0; index<step_count; ++index)
	{
		if (index>0)
		{
			render_debug_line_offset(TRUE, &steps[index - 1].point, &steps[index].point, color, 0.1f);
		}

		render_debug_tick(TRUE, &steps[index].point, global_up3d, 0.02f, color);
	}

	return;
}

static void ai_debug_render_path_node(
	struct structure_bsp const *structure_bsp,
	boolean bsp_access_allowed,
	struct path_state const *state,
	struct path_node const *node,
	struct path_node const *child_node,
	real_point3d const *child_point,
	real_argb_color const *path_color,
	real_argb_color const *polygon_color,
	real_argb_color const *cost_color,
	real_argb_color const *attractor_distance_color,
	real_argb_color const *attractor_weight_color,
	real_argb_color const *closest_color)
{
	real_point3d text_point;
	real text_increment_height;
	struct observer_result const *camera = observer_get_camera(0);
	real_point3d const *point = &node->entry_point;

	midpoint3d(child_point, point, &text_point);
	point_from_line3d(&text_point, global_up3d, 0.1f, &text_point);

	if (camera)
	{
		real_vector3d camera_vector;

		vector_from_points3d(&text_point, &camera->position, &camera_vector);
		text_increment_height = square_root(camera_vector.i*camera_vector.i + camera_vector.j*camera_vector.j + camera_vector.k*camera_vector.k) * 0.025f;
	}
	else
	{
		text_increment_height = 0.05f;
	}

	if (path_color)
	{
		render_debug_line_offset(TRUE, point, child_point, path_color, 0.03f);
		render_debug_tick(TRUE, point, global_up3d, 0.02f, path_color);

		if (!child_node)
		{
			render_debug_tick(TRUE, child_point, global_up3d, 0.02f, path_color);
		}
	}

	if (polygon_color && bsp_access_allowed)
	{
		ai_debug_render_surface(structure_bsp, node->surface_index, 0.f, polygon_color);
	}

	if (cost_color)
	{
		render_debug_string_at_point(TRUE, &text_point, csprintf(temporary, "%.1f", node->linear_distance_to_entry_point), cost_color);
		point_from_line3d(&text_point, global_up3d, text_increment_height, &text_point);
	}

	if (closest_color && node->closest_distance<REAL_MAX)
	{
		real closest_text_increment_height;
		real_point3d closest_text_point = node->closest_point;

		if (camera)
		{
			real_vector3d camera_vector;

			vector_from_points3d(&closest_text_point, &camera->position, &camera_vector);
			closest_text_increment_height = magnitude3d(&camera_vector) * 0.025f;
		}
		else
		{
			closest_text_increment_height = 0.05f;
		}

		render_debug_point(TRUE, &node->closest_point, 0.15f, closest_color);
		point_from_line3d(&closest_text_point, global_up3d, (closest_text_increment_height + 0.15f), &closest_text_point);
		render_debug_string_at_point(TRUE, &closest_text_point, csprintf(temporary, "%.1f", node->closest_distance), closest_color);
	}

	if (state && state->input.attractor_valid)
	{
		real attractor_distance = REAL_MAX;
		real attractor_weight = path_attractor_weight(state, point, child_point, &attractor_distance);

		if (attractor_distance_color)
		{
			render_debug_string_at_point(TRUE, &text_point, csprintf(temporary, "%.1f", attractor_distance), global_real_argb_yellow);
			point_from_line3d(&text_point, global_up3d, text_increment_height, &text_point);
		}

		if (attractor_weight_color && attractor_weight>0.f)
		{
			render_debug_string_at_point(TRUE, &text_point, csprintf(temporary, "%.1f", state->input.attractor_weight), global_real_argb_red);
		}
	}

	return;
}

static void ai_debug_render_path_nodes(
	struct path_state *path_state,
	boolean bsp_access_allowed,
	boolean render_all_nodes,
	boolean render_polygons,
	boolean render_costs,
	boolean render_closest)
{
	real_argb_color const *polygon_color = render_polygons ? global_real_argb_purple : NULL;
	real_argb_color const *cost_color = render_costs ? global_real_argb_white : NULL;
	real_argb_color const *attractor_distance_color = render_costs ? global_real_argb_yellow : NULL;
	real_argb_color const *attractor_weight_color = render_costs ? global_real_argb_red : NULL;
	real_argb_color const *closest_color = render_closest ? global_real_argb_yellow : NULL;
	static short current_traverse_index = 0;

	current_traverse_index++;

	if (path_state->destination_valid)
	{
		struct path_node *node;
		struct path_node const *previous_node = NULL;
		short node_index = path_node_from_hash_table(path_state, path_state->destination.surface_index);

		while (node_index!=NONE)
		{
			node = path_get_node(path_state, node_index);

			ai_debug_render_path_node(
				path_state->structure,
				bsp_access_allowed,
				path_state,
				node,
				previous_node,
				!previous_node ? &path_state->destination.point : &previous_node->entry_point,
				global_real_argb_red,
				polygon_color,
				cost_color,
				attractor_distance_color,
				attractor_weight_color,
				closest_color);

			node->debug_render_traverse_index = current_traverse_index;
			previous_node = node;
			node_index = node->parent_node_index;
		}
	}

	if (render_all_nodes)
	{
		short node_iterator_index;
		struct collision_bsp const *collision_bsp = TAG_BLOCK_GET_ELEMENT(&path_state->structure->collision_bsp, 0, struct collision_bsp);

		for (node_iterator_index = path_state->node_count - 1; node_iterator_index>=0; --node_iterator_index)
		{
			struct path_node const *previous_node = NULL;
			short node_index = node_iterator_index;

			while (node_index!=NONE)
			{
				struct path_node *node = path_get_node(path_state, node_index);

				if (node->debug_render_traverse_index!=current_traverse_index)
				{
					real_point3d const *child_point;
					real_point3d midpoint = *global_origin3d;

					if (!previous_node)
					{
						if (bsp_access_allowed)
						{
							struct collision_surface const *surface = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, node->surface_index, struct collision_surface);
							long edge_index = surface->first_edge_index;
							long vertex_count = 0;
							real scale = 1.f;

							do
							{
								struct collision_edge const *edge = TAG_BLOCK_GET_ELEMENT(&collision_bsp->edges, edge_index, struct collision_edge);
								boolean const rev = edge->surface_indices[1]==node->surface_index;
								struct collision_vertex const *vertex = TAG_BLOCK_GET_ELEMENT(&collision_bsp->vertices, edge->vertex_indices[rev], struct collision_vertex);

								add_vectors3d((real_vector3d const *)&midpoint, (real_vector3d const *)&vertex->point, (real_vector3d *)&midpoint);
								vertex_count++;
								edge_index = edge->edge_indices[rev];
							}
							while (edge_index!=surface->first_edge_index);

							scale /= vertex_count;
							scale_point3d(&midpoint, scale, &midpoint);
							child_point = &midpoint;
						}
						else
						{
							midpoint = node->closest_point;
							child_point = &midpoint;
						}
					}
					else
					{
						child_point = &node->entry_point;
					}

					ai_debug_render_path_node(
						path_state->structure,
						bsp_access_allowed,
						path_state,
						node,
						previous_node,
						child_point,
						global_real_argb_blue,
						polygon_color,
						cost_color,
						attractor_distance_color,
						attractor_weight_color,
						closest_color);

					node->debug_render_traverse_index = current_traverse_index;
					previous_node = node;
					node_index = node->parent_node_index;
				}
				else
				{
					if (previous_node)
					{
						render_debug_line_offset(TRUE, &previous_node->entry_point, &node->entry_point, global_real_argb_blue, 0.1f);
					}

					break;
				}
			}
		}
	}

	return;
}

static void ai_debug_render_surface(
	struct structure_bsp const *structure_bsp,
	long surface_index,
	real offset,
	real_argb_color const *color)
{
	struct collision_bsp const *collision_bsp = TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp);
	struct collision_surface const *collision_surface = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, surface_index, struct collision_surface);
	long edge_index = collision_surface->first_edge_index;

	do
	{
		struct collision_edge const *edge = TAG_BLOCK_GET_ELEMENT(&collision_bsp->edges, edge_index, struct collision_edge);
		boolean const next_index_belongs_to_surface = edge->surface_indices[1]==surface_index;
		struct collision_vertex const *point0 = TAG_BLOCK_GET_ELEMENT(&collision_bsp->vertices, edge->vertex_indices[0], struct collision_vertex);
		struct collision_vertex const *point1 = TAG_BLOCK_GET_ELEMENT(&collision_bsp->vertices, edge->vertex_indices[1], struct collision_vertex);

		render_debug_line_offset(TRUE, &point0->point, &point1->point, color, offset + 0.015f);
		edge_index = edge->edge_indices[next_index_belongs_to_surface];
	}
	while (edge_index!=collision_surface->first_edge_index);

	return;
}

static void ai_debug_render_actor(
	long actor_index,
	boolean render_exclusive,
	long *history_start_time)
{
	struct actor_datum* actor = actor_get(actor_index);
	struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];

	if (actor_debug_info->last_render_id!=ai_debug.last_render_id)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
		struct path_debug_storage *path = ai_debug_get_last_path(actor_index);
		struct unit_datum *unit = NULL;
		struct unit_definition *unit_definition = NULL;

		/* Get unit info if the actor has an associated unit */

		if (actor->meta.unit_index!=NONE)
		{
			unit = unit_get(actor->meta.unit_index);
			unit_definition = unit_definition_get(unit->definition_index);
		}

		actor_debug_info->last_render_id = ai_debug.last_render_id;

		/* Set history start time */

		if (history_start_time)
		{
			if (actor->state.combat_status>=2 && actor->target.target_prop_index!=NONE)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				if (*history_start_time==NONE || prop->last_perceived_time<*history_start_time)
				{
					*history_start_time = prop->last_perceived_time;
				}
			}
		}

		/* Stack setup */

		{
			real_point3d stack_base;

			point_from_line3d(&actor->input.position.head_position, global_up3d, 0.1f, &stack_base);
			ai_debug_drawstack_setup(&stack_base);
		}

		/* Unit highlighting */

		if (actor->meta.swarm)
		{
			if (actor->meta.swarm_cache_index!=NONE)
			{
				short unit_num;
				struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

				for (unit_num = 0; unit_num<swarm->unit_count; ++unit_num)
				{
					ai_debug_highlight_unit(swarm->unit_indices[unit_num], render_exclusive, actor_action_debug_color(actor_index));
				}
			}
		}
		else
		{
			ai_debug_highlight_unit(actor->meta.unit_index, render_exclusive, actor_action_debug_color(actor_index));
		}

		/* Line of fire crouching status */

		if (ai_debug.render_lineoffire_crouching &&
			(TEST_FLAG(actor_definition->flags, _actor_definition_crouch_in_line_of_fire_bit) ||
			TEST_FLAG(actor_definition->flags, _actor_definition_avoid_friend_line_of_fire_bit)))
		{
			/* Blocking messages */

			if (actor->emotions.crouch_blocking_line_of_fire ||
				actor->emotions.crouch_friends_in_line_of_fire ||
				actor->emotions.crouch_blocking_player_line_of_fire)
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(
						temporary,
						"%s%s",
						actor->emotions.crouch_blocking_player_line_of_fire ? "blocking-player " : actor->emotions.crouch_blocking_line_of_fire ? "blocking " : "",
						actor->emotions.crouch_friends_in_line_of_fire ? "friends-blocking" : ""),
					global_real_argb_orange);
			}

			/* Moving into fire */

			if (actor->emotions.moving_into_player_line_of_fire ||
				actor->emotions.moving_into_fire_timer > 0)
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(
						temporary,
						"%sforce-stop %d", 
						actor->emotions.moving_into_player_line_of_fire ? "moving-into-fire" : "",
						actor->emotions.moving_into_fire_timer),
					global_real_argb_pink);
			}
		}

		/* Player aiming blocked */

		if (ai_debug.render_player_aiming_blocked)
		{
			struct prop_iterator iterator;
			struct prop_datum *prop;
			real_vector3d aiming_vector;
			short player_obstruction = _actor_aiming_clear;
			boolean found_player = FALSE;

			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				if (prop->state >= _prop_state_becoming_unacknowledged &&
					prop->state <= _prop_state_acknowledged &&
					!prop->enemy)
				{
					if (prop->player)
					{
						short obstruction;

						found_player = TRUE;
						unit_get_aiming_vector(prop->unit_index, &aiming_vector);
						obstruction = actor_perception_aiming_vector_test_blockage(
							&prop->body_position,
							&aiming_vector,
							&actor->input.position.body_position,
							NULL);

						player_obstruction = MAX(player_obstruction, obstruction);
					}
				}
			}

			if (found_player)
			{
				switch (player_obstruction)
				{
				case _actor_aiming_clear:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "not-occluding-player", global_real_argb_green);
					break;
				case _actor_aiming_occluded:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "occluding-player", global_real_argb_blue);
					break;
				case _actor_aiming_blocked:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "blocking-player", global_real_argb_red);
					break;
				default:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "aiming occlusion error", global_real_argb_pink);
					break;
				}

			}
		}

		/* Vector avoidance */

		if (ai_debug.render_vector_avoidance &&
			actor_debug_info->field_19C != NONE &&
			actor_debug_info->field_19C + TICKS_PER_SECOND>game_time_get())
		{
			short i;
			short j;

			/* Avoidance rays */

			if (ai_debug.render_vector_avoidance_rays)
			{
				short ray_index;
				for (ray_index = 0; ray_index<ACTOR_MAXIMUM_AVOIDANCE_RAYS; ++ray_index)
				{
					real_argb_color const *color = global_real_argb_white;

					switch (actor_debug_info->avoidance_type[ray_index])
					{
					case _actor_vector_avoidance_obstructed_object:
						color = global_real_argb_magenta;
						break;
					case _actor_vector_avoidance_obstructed_structure:
						color = global_real_argb_red;
						break;
					}

					if (actor_debug_info->avoidance_type[ray_index]>0)
					{
						real_point3d point0;
						real_point3d point1;

						point_from_line3d(
							&actor_debug_info->ray_origin[ray_index],
							&actor_debug_info->ray_direction[ray_index],
							(actor_debug_info->collision_t[ray_index]),
							&point0);
						point_from_line3d(
							&actor_debug_info->ray_origin[ray_index],
							&actor_debug_info->ray_direction[ray_index],
							1.f,
							&point1);

						render_debug_line(TRUE, &actor_debug_info->ray_origin[ray_index], &point0, global_real_argb_white);
						render_debug_point(TRUE, &point0, 0.2f, color);
						render_debug_line(TRUE, &point0, &point1, color);

						if (ai_debug.render_vector_avoidance_sense_t)
						{
							point_from_line3d(&point0, global_up3d, 0.25f, &point0);
							render_debug_string_at_point(
								TRUE,
								&point0,
								csprintf(temporary, "%.3f", actor_debug_info->collision_t[ray_index]),
								color);
						}
					}
					else
					{
						render_debug_vector(
							TRUE,
							&actor_debug_info->ray_origin[ray_index],
							&actor_debug_info->ray_direction[ray_index],
							1.f,
							global_real_argb_white);
					}
				}
			}

			/* Avoidance rays */

			for (i = 0; i < 8; ++i)
			{
				for (j = 0; j < 2; ++j)
				{
					if (ai_debug.render_vector_avoidance_rays)
					{
						real_argb_color const *color = global_real_argb_white;

						switch (actor_debug_info->avoid_result[i][j])
						{
						case _actor_vector_avoidance_obstructed_object:
							color = global_real_argb_aqua;
							break;
						case _actor_vector_avoidance_obstructed_structure:
							color = global_real_argb_yellow;
							break;
						}

						if (actor_debug_info->avoid_result[i][j]>_actor_vector_avoidance_clear)
						{
							real_point3d point0;
							real_point3d point1;

							point_from_line3d(
								&actor_debug_info->field_6358[i][j],
								&actor_debug_info->field_6418[i][j],
								(actor_debug_info->avoid_t[i][j]),
								&point0);
							point_from_line3d(
								&actor_debug_info->field_6358[i][j],
								&actor_debug_info->field_6418[i][j],
								1.f,
								&point1);

							render_debug_line(TRUE, &actor_debug_info->field_6358[i][j], &point0, global_real_argb_blue);
							render_debug_point(TRUE, &point0, 0.2f, color);
							render_debug_line(TRUE, &point0, &point1, color);

							if (ai_debug.render_vector_avoidance_avoid_t)
							{
								point_from_line3d(&point0, global_up3d, 0.25f, &point0);
								render_debug_string_at_point(
									TRUE,
									&point0,
									csprintf(
										temporary,
										"%.3f",
										actor_debug_info->avoid_t[i][j]),
									color);
							}
						}
						else
						{
							render_debug_vector(
								TRUE,
								&actor_debug_info->field_6358[i][j],
								&actor_debug_info->field_6418[i][j],
								1.f,
								global_real_argb_blue);

							if (ai_debug.render_vector_avoidance_clear_time)
							{
								real_point3d point;

								point_from_line3d(&actor_debug_info->field_6358[i][j], &actor_debug_info->field_6418[i][j], 0.1f, &point);
								point_from_line3d(&point, global_up3d, 0.15f, &point);
								render_debug_string_at_point(
									TRUE,
									&point,
									csprintf(
										temporary,
										"c%d",
										actor->control.vector_avoidance_clear_times[i][j]),
									global_real_argb_white);
							}
						}
					}

					/* Avoidance weights */

					if (ai_debug.render_vector_avoidance_weights && j==1)
					{
						real_point3d point;

						point_from_line3d(
							&actor_debug_info->field_6358[i][j],
							&actor_debug_info->field_6418[i][j],
							0.25f,
							&point);
						point_from_line3d(&point, global_up3d, 0.15f, &point);
						render_debug_string_at_point(
							TRUE,
							&point,
							csprintf(
							temporary,
							"%d: w%.2f",
							i,
							actor_debug_info->avoidance_weights[i]),
							global_real_argb_green);
					}
				}
			}

			/* Avoidance intermediate */

			if (ai_debug.render_vector_avoidance_intermediate)
			{
				real_vector3d direction;
				real_argb_color const *color = actor_debug_info->field_6550 ? global_real_argb_magenta : global_real_argb_cyan;

				render_debug_vector(TRUE, &actor_debug_info->avoidance_data.origin, &actor_debug_info->field_6524, 5.f, global_real_argb_yellow);
				render_debug_vector(TRUE, &actor_debug_info->avoidance_data.origin, &actor_debug_info->field_6530, 1.f, global_real_argb_purple);
				actor_move_get_avoidance_direction(&actor_debug_info->avoidance_data, actor_debug_info->field_6500, &direction);

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "best %.2f at %d", actor_debug_info->field_64FC, actor_debug_info->field_6500),
					color);
				render_debug_vector(TRUE, &actor_debug_info->avoidance_data.origin, &direction, 2.f, color);

				actor_move_get_avoidance_direction(&actor_debug_info->avoidance_data, actor_debug_info->field_6504, &direction);
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "curr ~%.2f at %.2f", actor_debug_info->field_6508, actor_debug_info->field_6504),
					global_real_argb_green);

				render_debug_vector(TRUE, &actor_debug_info->avoidance_data.origin, &direction, 2.0, global_real_argb_green);
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "danger %.2f turncos %.2f", actor_debug_info->sign_no_danger, actor_debug_info->field_6510),
					global_real_argb_yellow);

				if (actor_debug_info->field_6551)
				{
					actor_move_transform_avoidance_vector(&actor_debug_info->avoidance_data, &actor_debug_info->avoidance_vector, &direction);
					render_debug_vector(TRUE, &actor_debug_info->avoidance_data.origin, &direction, 2.0, global_real_argb_darkgreen);
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(
						temporary,
						"turn angvel %.2f danger %.2f bonus %.2f",
						actor_debug_info->field_6558,
						actor_debug_info->field_6568,
						actor_debug_info->field_6554),
						global_real_argb_darkgreen);
				}
			}

			/* Avoidance objects */

			if (ai_debug.render_vector_avoidance_objects)
			{
				short avoidance_object_index;

				for (avoidance_object_index = 0; avoidance_object_index<actor_debug_info->avoidance_data.avoidance_object_count; ++avoidance_object_index)
				{
					real_vector3d height;
					struct vehicle_avoidance_cylinder const *avoidance_object = &actor_debug_info->avoidance_data.avoidance_objects[avoidance_object_index];

					set_real_vector3d(&height, 0.f, 0.f, avoidance_object->height);
					render_debug_pill(
						TRUE,
						&avoidance_object->base,
						&height,
						avoidance_object->width,
						global_real_argb_aqua);
				}
			}

			{
				boolean found_player = FALSE;

				switch (actor_debug_info->field_653C)
				{
				case 0:
					sprintf(temporary, "clear");
					break;
				case 1:
					sprintf(temporary, "sensed at %f, turn %f", actor_debug_info->field_651C, actor_debug_info->field_6520);
					found_player = TRUE;
					break;
				case 2:
					sprintf(temporary, "sign no-danger %f < %f", actor_debug_info->sign_no_danger, 1.3f);
					break;
				case 3:
					sprintf(temporary, "sign too-far cosangle %f < %f", actor_debug_info->sign_too_far_cosangle, 0.5f);
					break;
				case 4:
					sprintf(temporary, "sign rotated %f", actor_debug_info->sign_rotated);
					found_player = TRUE;
					break;
				case 5:
					sprintf(temporary, "sharp new-turn");
					found_player = TRUE;
					break;
				case 6:
					sprintf(temporary, "sharp change-dir");
					found_player = TRUE;
					break;
				case 7:
					sprintf(temporary, "sharp continue");
					found_player = TRUE;
					break;
				default:
					sprintf(temporary, "<error>");
					break;
				}

				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_orange);

				if (found_player)
				{
					render_debug_vector(TRUE, &actor_debug_info->avoidance_data.origin, &actor_debug_info->field_6540, 1.f, global_real_argb_pink);
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "move emergency %.2f", actor_debug_info->field_654C), global_real_argb_pink);
				}
			}
		}

		/* Activation */

		if (ai_debug.render_activation)
		{
			{
				real_argb_color const *encounter_color;
				char encounterbuf[256];

				if (actor->meta.encounter_index!=NONE)
				{
					struct encounter_definition const *encounter_definition = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->ai_encounters,
						DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
						struct encounter_definition);
					struct encounter_datum const *encounter = encounter_get(actor->meta.encounter_index);
					boolean outside_current_bsp = FALSE;


					if (encounter_definition->runtime_structure_bsp_reference_index==NONE)
					{
						encounter_color = global_real_argb_green;
						sprintf(encounterbuf, "%s (no-bsp)", encounter_definition->name);
					}
					else
					{
						outside_current_bsp = encounter_definition->runtime_structure_bsp_reference_index!=global_structure_bsp_index;
						sprintf(encounterbuf, "%s (bsp %d)", encounter_definition->name, encounter_definition->runtime_structure_bsp_reference_index);
					}

					encounter_color = encounter->active ? global_real_argb_green : outside_current_bsp ? global_real_argb_red : global_real_argb_purple;
				}
				else
				{
					encounter_color = global_real_argb_blue;
					sprintf(encounterbuf, "encounterless");
				}

				render_debug_string_at_point(TRUE, ai_debug_drawstack(), encounterbuf, encounter_color);
			}

			{
				real_argb_color const *actor_color = actor_activation_debug_color(actor_index);
				long unit_index = actor->meta.unit_index;
				short unit_count = 0;
				struct swarm_datum *swarm = NULL;
				struct observer_result const *camera = observer_get_camera(0);
				unsigned long const *pvs = players_get_combined_pvs();

				if (actor->meta.swarm && actor->meta.swarm_cache_index!=NONE)
				{
					swarm = swarm_get(actor->meta.swarm_cache_index);
				}

				while (TRUE)
				{
					if (unit_index!=NONE)
					{
						real_point3d base_point;
						real_vector3d offset_vector;
						short cluster_index;

						unit_get_head_position(unit_index, &base_point);
						point_from_line3d(&base_point, global_up3d, 0.2f, &base_point);

						if (camera==NULL)
						{
							offset_vector = *global_forward3d;
						}
						else
						{
							cross_product3d(&camera->forward, global_up3d, &offset_vector);
							if (normalize3d(&offset_vector)==0.f)
							{
								offset_vector = *global_forward3d;
							}
						}

						cluster_index = object_get(object_get_ultimate_parent(unit_index))->object.location.cluster_index;

						if (cluster_index==NONE)
						{
							real_point3d p0;
							real_point3d p1;
							real_point3d p2;
							real_point3d p3;

							point_from_line3d(&base_point, &offset_vector, 0.1f, &p0);
							point_from_line3d(&base_point, &offset_vector, -0.1f, &p3);
							point_from_line3d(&p0, global_up3d, 0.2f, &p1);
							point_from_line3d(&p3, global_up3d, 0.2f, &p2);

							render_debug_line(TRUE, &p0, &p1, actor_color);
							render_debug_line(TRUE, &p1, &p2, actor_color);
							render_debug_line(TRUE, &p2, &p3, actor_color);
							render_debug_line(TRUE, &p3, &p0, actor_color);
						}
						else if (BIT_VECTOR_TEST_FLAG(pvs, cluster_index))
						{
							real_point3d mid_point;
							real_point3d p1;
							real_point3d p2;
							real_point3d p3;
							real_point3d p0 = base_point;

							point_from_line3d(&base_point, global_up3d, 0.2f, &p2);
							point_from_line3d(&base_point, global_up3d, 0.1f, &mid_point);
							point_from_line3d(&mid_point, &offset_vector, 0.1f, &p1);
							point_from_line3d(&mid_point, &offset_vector, -0.1f, &p3);

							render_debug_line(TRUE, &p0, &p1, actor_color);
							render_debug_line(TRUE, &p1, &p2, actor_color);
							render_debug_line(TRUE, &p2, &p3, actor_color);
							render_debug_line(TRUE, &p3, &p0, actor_color);
						}
						else
						{
							real_point3d p0;
							real_point3d p1;
							real_point3d p2;
							real_point3d p3;

							point_from_line3d(&base_point, &offset_vector, 0.1f, &p0);
							point_from_line3d(&base_point, &offset_vector, -0.1f, &p3);
							point_from_line3d(&p0, global_up3d, 0.2f, &p1);
							point_from_line3d(&p3, global_up3d, 0.2f, &p2);

							render_debug_line(TRUE, &p0, &p2, actor_color);
							render_debug_line(TRUE, &p1, &p3, actor_color);
						}
					}

					if (swarm && unit_count<swarm->unit_count)
					{
						unit_index = swarm->unit_indices[unit_count++];
					}
					else
					{
						break;
					}
				}
			}
		}

		/* Support surfaces */

		if (ai_debug.render_support_surfaces)
		{
			struct swarm_datum *swarm = NULL;
			long unit_index = NONE;
			short unit_num = 0;

			if (!actor->meta.swarm || actor->meta.swarm_cache_index==NONE)
			{
				unit_index = actor->meta.unit_index;
			}
			else
			{
				swarm = swarm_get(actor->meta.swarm_cache_index);

				if (swarm->unit_count>0)
				{
					unit_index = swarm->unit_indices[0];
				}
			}

			while (unit_index!=NONE)
			{
				struct biped_datum *biped = biped_try_and_get(unit_index);

				if (!biped || biped->biped.support_surface_index==NONE)
				{
					real_point3d origin;

					object_get_origin(unit_index, &origin);
					render_debug_sphere(TRUE, &origin, 0.3f, global_real_argb_pink);

					if (!actor->meta.swarm)
					{
						if (actor->input.pathfinding_surface_index==NONE)
						{
							render_debug_sphere(TRUE, &actor->input.position.body_position, 0.4f, global_real_argb_red);
						}
						else
						{
							render_debug_sphere(TRUE, &actor->input.pathfinding_point, 0.4f, global_real_argb_orange);
							ai_debug_render_surface(global_structure_bsp_get(), actor->input.pathfinding_surface_index, 0.f, global_real_argb_orange);
						}
					}
				}
				else
				{
					ai_debug_render_surface(global_structure_bsp_get(), biped->biped.support_surface_index, 0.f, global_real_argb_pink);
				}

				++unit_num;
				unit_index = NONE;

				if (swarm && unit_num<swarm->unit_count)
				{
					unit_index = swarm->unit_indices[unit_num];
				}
			}
		}

		/* Vitality */

		if (ai_debug.render_vitality)
		{
			if (actor->input.body_vitality>0.f)
			{
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "body %3.2f", actor->input.body_vitality), global_real_argb_red);
			}

			if (actor->input.shield_vitality>0.f)
			{
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "shld %3.2f", actor->input.shield_vitality), global_real_argb_blue);
			}
		}

		/* Damage */

		if (ai_debug.render_recent_damage)
		{
			if (actor->input.recent_body_damage>0.f)
			{
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "b/dmg %3.2f", actor->input.recent_body_damage), global_real_argb_yellow);
			}

			if (actor->input.recent_shield_damage>0.f)
			{
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "s/dmg %3.2f", actor->input.recent_shield_damage), global_real_argb_green);
			}
		}

		/* Cover seeking */

		if (ai_debug.render_active_cover_seeking && actor_debug_info->field_B8)
		{
			char const *strings[8] =
			{
				"wrongaction",
				"visibletarget",
				"repeattimer",
				"visibletimer",
				"shielded",
				"unavailable",
				"success",
				"panic"
			};

			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(temporary, "%s %d %.2f", strings[actor_debug_info->field_BA], actor_debug_info->field_BC, actor_debug_info->field_C0),
				global_real_argb_yellow);
		}

		/* Threats */

		if (ai_debug.render_threats && actor->situation.known_enemies)
		{
			render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "enemy %d", actor->situation.known_enemies), global_real_argb_red);

			actor_debug_print_threat(actor, _actor_threat_visible, "vis %d/%d", global_real_argb_white);
			actor_debug_print_threat(actor, _actor_threat_visible_facing_me, "facing %d/%d", global_real_argb_lightblue);
			actor_debug_print_threat(actor, _actor_threat_visible_aiming_at_me, "aim at %d/%d", global_real_argb_blue);
			actor_debug_print_threat(actor, _actor_threat_shooting, "shoot %d/%d", global_real_argb_yellow);
			actor_debug_print_threat(actor, _actor_threat_shooting_near_me, "s/near %d/%d", global_real_argb_orange);
			actor_debug_print_threat(actor, _actor_threat_shooting_at_me, "s/at me %d/%d", global_real_argb_red);
			actor_debug_print_threat(actor, _actor_threat_extremely_close_to_me, "ex.close %d/%d", global_real_argb_magenta);
			actor_debug_print_threat(actor, _actor_threat_damaging_me, "dmging %d/%d", global_real_argb_magenta);
		}

		/* Emotions */

		if (ai_debug.render_emotions)
		{
			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(temporary, "dngr %3.2f/%3.2f", actor->emotions.instantaneous_danger, actor->emotions.perceived_danger),
				global_real_argb_yellow);

			if (actor->emotions.unopposable_retreat_timer>0)
			{
				real_point3d position;
				struct prop_datum *prop = prop_get(actor->emotions.unopposable_retreat_prop_index);

				point_from_line3d(&actor->input.position.head_position, global_up3d, 0.05f, &position);
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "retreat t%d", actor->emotions.unopposable_retreat_timer), global_real_argb_red);
				render_debug_line(TRUE, &position, &prop->head_position, global_real_argb_red);
			}

			if (TEST_FLAG(actor_definition->flags, _actor_definition_fixed_crouch_facing_bit))
			{
				char const *string;

				if (actor->control.desire_stationary_facing)
				{
					if (actor->control.fixed_stationary_facing)
					{
						render_debug_string_at_point(TRUE, ai_debug_drawstack(), "fixed-facing", global_real_argb_orange);
						render_debug_vector(TRUE, &actor->input.position.head_position, &actor->control.fixed_stationary_facing_vector, 1.5f, global_real_argb_orange);
					}
					else
					{
						render_debug_string_at_point(TRUE, ai_debug_drawstack(), "desire-fixed-facing", global_real_argb_orange);
					}
				}
				else
				{
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "nostationary%s", actor->control.moving ? " (moving)" : ""), global_real_argb_orange);
				}
			}
		}

		/* Teams */

		if (ai_debug.render_teams)
		{
			char const *teams[NUMBER_OF_SOLO_CAMPAIGN_TEAMS] =
			{
				"default",
				"player",
				"human",
				"covenant",
				"flood",
				"sentinel",
				"unused6",
				"unused7",
				"unused8",
				"unused9"
			};

			render_debug_string_at_point(TRUE, ai_debug_drawstack(), actor->meta.team_index==NONE ? "none" : teams[actor->meta.team_index], global_real_argb_green);
		}

		/* Player ratings */

		if (ai_debug.render_player_ratings && actor->meta.unit_index!=NONE)
		{
			real player_rating = ai_communication_get_player_rating(actor->meta.unit_index, TRUE, NULL, NULL);
			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(temporary, "%.1f", player_rating),
				player_rating==0.f ? global_real_argb_blue : global_real_argb_white);
		}

		/* Audibility */

		if (ai_debug.render_audibility && actor_debug_info->audibility_valid)
		{
			char const *aud_type;
			char textstring[512];
			real_argb_color const *color;

			if (actor_debug_info->audibility_result==_actor_perception_none)
			{
				color = global_real_argb_red;
				aud_type = "none";
				
			}
			else if (actor_debug_info->audibility_result==_actor_perception_partial)
			{
				color = global_real_argb_blue;
				aud_type = "part";
			}
			else
			{
				color = global_real_argb_green;
				aud_type = "full";
			}

			sprintf(textstring, "aud/%s %.1fp/%.1fd", aud_type, actor_debug_info->audibility_perception_distance, actor_debug_info->audibility_straight_distance);

			if (actor_debug_info->audibility_propagation_distance!=-1.f)
			{
				strcat(textstring, csprintf(temporary, "/%.1fs/%.1ff", actor_debug_info->audibility_propagation_distance, actor_debug_info->audibility_final_distance));
			}

			render_debug_string_at_point(TRUE, ai_debug_drawstack(), textstring, color);
		}

		/* Props */

		if (ai_debug.render_props || ai_debug.render_props_web)
		{
			struct prop_iterator iterator;
			struct prop_datum *prop;
			real_point3d prop_start_point;

			short dead_count = 0;
			short friend_count = 0;
			short enemy_count = 0;
			short orphan_count = 0;
			short total_count = 0;

			point_from_line3d(&actor->input.position.head_position, global_up3d, 0.2f, &prop_start_point);

			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				++total_count;

				if (prop->dead)
				{
					++dead_count;
				}
				else if (
					prop->state>=_prop_state_uninspected_orphan &&
					prop->state<=_prop_state_inspected_orphan)
				{
					match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 2230, prop->enemy);
					++orphan_count;
				}
				else
				{
					if (prop->enemy)
					{
						++enemy_count;
					}
					else
					{
						++friend_count;
					}
				}

				if ((render_exclusive || ai_debug.render_props_web) &&
					(prop->enemy || !ai_debug.render_props_no_friends))
				{
					real_point3d string_point;
					real_point3d origin;
					real_argb_color const *color;

					point_from_line3d(&prop->head_position, global_up3d, 0.2, &string_point);

					switch (prop->state)
					{
					case _prop_state_becoming_unacknowledged:
					case _prop_state_acknowledged:
						render_debug_line(TRUE, &prop_start_point, &prop->head_position, global_real_argb_yellow);
						break;
					case _prop_state_becoming_acknowledged:
						set_real_point3d(
							&origin,
							(1.f-prop->awareness)*prop_start_point.x + prop->awareness*prop->head_position.x,
							(1.f-prop->awareness)*prop_start_point.y + prop->awareness*prop->head_position.y,
							(1.f-prop->awareness)*prop_start_point.z + prop->awareness*prop->head_position.z
						);
						render_debug_line(TRUE, &prop_start_point, &origin, global_real_argb_yellow);
						render_debug_line(TRUE, &origin, &prop->head_position, global_real_argb_black);
						break;
					case _prop_state_unacknowledged:
						render_debug_line(TRUE, &prop_start_point, &prop->head_position, global_real_argb_black);
						break;
					case _prop_state_uninspected_orphan:
					case _prop_state_inspected_orphan:
						color = iterator.index==actor->meta.interesting_orphan_index ? global_real_argb_purple : global_real_argb_blue;
						render_debug_line(TRUE, &prop_start_point, &prop->head_position, color);
						
						if (!prop->definitely_located)
						{
							real_vector3d hint_vector;
							real_argb_color const *alt_color = prop->state==_prop_state_uninspected_orphan ? global_real_argb_yellow : global_real_argb_blue;

							set_real_vector3d(&hint_vector, prop->orphan_hint_vector.i, prop->orphan_hint_vector.j, 0.f);
							render_debug_sphere(TRUE, &prop->head_position, 0.2f, alt_color);
							render_debug_vector(TRUE, &prop->head_position, &hint_vector, 1.f, alt_color);
						}
						break;
					default:
						break;
					}

					point_from_line3d(&prop_start_point, global_up3d, 0.03f, &prop_start_point);

					if (prop->required_ticks>0 ||
						prop->state>=_prop_state_uninspected_orphan && prop->state<=_prop_state_inspected_orphan)
					{
						if (prop->required_ticks>0)
						{
							sprintf(temporary, "r%d ", prop->required_ticks);
						}
						else
						{
							strcpy(temporary, "");
						}

						if (prop->state>=_prop_state_uninspected_orphan && prop->state<=_prop_state_inspected_orphan)
						{
							char temp[256];
							strcpy(temp, temporary);
							sprintf(temporary, "%so%d ", temp, prop->orphan_lifespan_ticks);
						}
						
						if (prop->state==_prop_state_uninspected_orphan)
						{
							char temp[256];
							strcpy(temp, temporary);
							sprintf(temporary, "%si%d ", temp, prop->orphan_inspection_ticks);
						}

						render_debug_string_at_point(TRUE, &string_point, temporary, global_real_argb_pink);
						point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
					}

					if (ai_debug.render_props_target_weight)
					{
						render_debug_string_at_point(
							TRUE,
							&string_point,
							csprintf(temporary, "%.2f", prop->target_weight),
							prop->ignore ? global_real_argb_blue : prop->preferred_target ? global_real_argb_pink : global_real_argb_red);
						point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
						
						if (iterator.index==actor->target.target_prop_index)
						{
							render_debug_string_at_point(TRUE, &string_point, "target", global_real_argb_white);
							point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
						}
					}

					if (ai_debug.render_props_unreachable && prop->unreachable_ticks>0)
					{
						long time = prop->last_unreachable_time!=NONE ? game_time_get()-prop->last_unreachable_time : NONE;

						render_debug_string_at_point(
							TRUE,
							&string_point,
							csprintf(temporary, "unr %d %d", prop->unreachable_ticks, time),
							global_real_argb_darkgreen);
						point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
					}

					if (ai_debug.render_props_unopposable && prop->unopposable_enemy)
					{
						sprintf(
							temporary,
							"unopp c%d(%d) t%d",
							prop->unopposable_casualties_inflicted,
							prop->unopposable_casualty_decay_timer,
							prop->unopposable_trigger_timer);
						if (prop->unopposable_trigger_timer>0)
						{
							char string[256];

							sprintf(string, "/%d h%d", prop->unopposable_trigger_threshold, prop->unopposable_trigger_hysteresis);
							strcat(temporary, string);
						}

						render_debug_string_at_point(TRUE, &string_point, temporary, global_real_argb_pink);
						point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
						render_debug_line(TRUE, &prop_start_point, &prop->head_position, global_real_argb_pink);
						point_from_line3d(&prop_start_point, global_up3d, 0.03f, &prop_start_point);
					}
				}
			}

			if (ai_debug.render_props_unopposable && actor->emotions.unopposable_retreat_timer>0)
			{
				real_point3d p0;
				struct prop_datum const *retreating_prop = prop_get(actor->emotions.unopposable_retreat_prop_index);
				
				point_from_line3d(&actor->input.position.head_position, global_up3d, 0.03f, &p0);
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "retreat t%d", actor->emotions.unopposable_retreat_timer),
					global_real_argb_red);
				render_debug_line(TRUE, &p0, &retreating_prop->head_position, global_real_argb_red);
			}

			if (render_exclusive || ai_debug.render_props_web)
			{
				sprintf(temporary, "d%d o%d e%d f%d", dead_count, orphan_count, enemy_count, friend_count);
			}
			else
			{
				sprintf(temporary, "%d", total_count);
			}
			
			render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_green);
		}

		if (ai_debug.render_secondary_looking && actor->control.secondary_look_type>0)
		{
			char const *secondary_look_type_strings[NUMBER_OF_SECONDARY_LOOK_TYPES] =
			{
				"none",
				"noise",
				"moving",
				"impact",
				"ack",
				"bumped",
				"deton",
				"shoot",
				"comm",
				"comm/d",
				"combat",
				"damage",
				"danger",
				"script"
			};

			char const *secondary_look_priotity_strings[NUMBER_OF_SECONDARY_LOOK_PRIORITIES] =
			{
				"none",
				"def",
				"i/look",
				"i/aim",
				"aim",
				"turn/a",
				"stop/a",
				"over",
				"over/f"
			};

			char const *direction_specification_type_strings[NUMBER_OF_DIRECTION_SPECIFICATION_TYPES] =
			{
				"move",
				"prop",
				"targ",
				"point",
				"vector",
				"danger",
				NULL
			};

			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(
					temporary,
					"%s %s %s %d",
					direction_specification_type_strings[actor->control.secondary_look_direction.type],
					secondary_look_type_strings[actor->control.secondary_look_type],
					secondary_look_priotity_strings[actor->control.secondary_look_priority],
					actor->control.secondary_look_timer),
				global_real_argb_magenta);

			switch (actor->control.secondary_look_direction.type)
			{
			case _direction_specification_prop:
			{
				struct prop_datum *prop = prop_get(actor->control.secondary_look_direction.prop_index);

				render_debug_line(
					TRUE,
					&actor->input.position.head_position,
					&prop->head_position,
					global_real_argb_magenta);
				break;
			}
			case _direction_specification_target:
				if (actor->target.target_prop_index!=NONE)
				{
					struct prop_datum *prop = prop_get(actor->target.target_prop_index);

					render_debug_line(
						TRUE,
						&actor->input.position.head_position,
						&prop->head_position,
						global_real_argb_magenta);
				}
				break;
			case _direction_specification_point:
				render_debug_line(
					TRUE, 
					&actor->input.position.head_position,
					&actor->control.secondary_look_direction.point,
					global_real_argb_magenta);
				break;
			case _direction_specification_vector:
			{
				real_point3d aim_pos;
				
				point_from_line3d(
					&actor->input.position.head_position,
					(real_vector3d *)&actor->control.secondary_look_direction.point,
					1.f,
					&aim_pos);
				render_debug_line(
					TRUE,
					&actor->input.position.head_position,
					&aim_pos,
					global_real_argb_magenta);
				break;
			}
			default:
				break;
			}
		}

		/* Pursuit */

		if (ai_debug.render_pursuit)
		{
			struct pursuit_location const *location = actor_get_pursuit_location(actor_index);

			if (location)
			{
				if (location->type==0 && actor->target.target_prop_index!=NONE)
				{
					struct prop_datum *prop = prop_get(actor->target.target_prop_index);
					render_debug_line(TRUE, &actor->input.position.head_position, &prop->body_position, actor_action_debug_color(actor_index));
				}
				else
				{
					if (location->type==_pursuit_location_position)
					{
						render_debug_line(TRUE, &actor->input.position.head_position, &location->position, actor_action_debug_color(actor_index));
					}
				}
			}
			
		}

		/* Aiming vectors */

		if (render_exclusive && ai_debug.render_aiming_vectors && actor->meta.unit_index!=NONE)
		{
			real_point3d p0;
			real_point3d p1;
			real_vector3d forward;
			real_vector3d v;

			point_from_line3d(&actor->input.position.head_position, global_up3d, 0.01f, &p0);
			point_from_line3d(&actor->input.position.head_position, &actor->control.desired_facing_vector, 1.5f, &p1);
			point_from_line3d(&p1, global_up3d, 0.01f, &p1);
			render_debug_line(TRUE, &p0, &p1, global_real_argb_orange);
			
			point_from_line3d(&actor->input.position.head_position, &actor->control.desired_aiming_vector, 1.4f, &p1);
			point_from_line3d(&p1, global_up3d, 0.02f, &p1);
			render_debug_line(TRUE, &p0, &p1, global_real_argb_green);
			
			point_from_line3d(&actor->input.position.head_position, &actor->control.desired_looking_vector, 1.3f, &p1);
			point_from_line3d(&p1, global_up3d, 0.03f, &p1);
			render_debug_line(TRUE, &p0, &p1, global_real_argb_cyan);
			
			point_from_line3d(&actor->input.position.head_position, global_up3d, -0.04f, &p0);
			unit_get_facing_vector(actor->meta.unit_index, &forward);
			
			render_debug_vector(TRUE, &p0, &forward, 1.f, global_real_argb_red);
			render_debug_vector(TRUE, &p0, &unit->unit.aiming_vector, 1.f, global_real_argb_darkgreen);
			render_debug_vector(TRUE, &p0, &unit->unit.looking_vector, 1.f, global_real_argb_blue);


			if (unit->object.type==_object_type_biped && unit->object.parent_object_index==NONE)
			{
				real_vector3d throttle_vector;
				struct biped_definition *biped_definition = biped_definition_get(unit->definition_index);
				
				unit_get_facing_vector(actor->meta.unit_index, &forward);

				if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit))
				{
					real_vector3d left_vector;
					real_vector3d up_vector;

					biped_build_flying_axes(&forward, &left_vector, &up_vector);
					scale_vector3d(&forward, actor->output.throttle.i, &throttle_vector);
					vector_from_line3d(&throttle_vector, &left_vector, actor->output.throttle.j, &throttle_vector);
					vector_from_line3d(&throttle_vector, &up_vector, actor->output.throttle.k, &throttle_vector);
				}
				else
				{
					set_real_vector3d(&v, -forward.j, forward.i, 0.f);
					scale_vector3d(&forward, actor->output.throttle.i, &throttle_vector);
					vector_from_line3d(&throttle_vector, &v, actor->output.throttle.j, &throttle_vector);
				}

				point_from_line3d(&actor->input.position.body_position, global_up3d, 0.1f, &p0);
				render_debug_vector(TRUE, &p0, &throttle_vector, 1.6f, global_real_argb_pink);
			}
		}

		/* Gun positions */
		
		if (ai_debug.render_gun_positions && actor->meta.unit_index!=NONE)
		{
			real_point3d estimated_position;
			real_vector3d *gun_offset = NULL;
			real_argb_color const *color = global_real_argb_red;
			real_vector3d desired_facing = actor->input.aiming_vector;

			if (normalize2d((real_vector2d *)&desired_facing)>0.f)
			{
				desired_facing.k = 0.f;
			}
			else
			{
				desired_facing = actor->input.facing_vector;
			}

			if (actor->control.crouching)
			{
				if (magnitude_squared3d(&actor_variant_definition->ranged_combat.gun_offset_crouch)>_real_epsilon)
				{
					gun_offset = &actor_variant_definition->ranged_combat.gun_offset_crouch;
					color = global_real_argb_magenta;
				}
				else
				{
					if (magnitude_squared3d(&actor_definition->perception.gun_offset_crouch)>_real_epsilon)
					{
						gun_offset = &actor_definition->perception.gun_offset_crouch;
						color = global_real_argb_pink;
					}
				}
			}
			else
			{
				if (magnitude_squared3d(&actor_variant_definition->ranged_combat.gun_offset_stand)>_real_epsilon)
				{
					gun_offset = &actor_variant_definition->ranged_combat.gun_offset_stand;
					color = global_real_argb_magenta;
				}
				else
				{
					if (magnitude_squared3d(&actor_definition->perception.gun_offset_stand)>_real_epsilon)
					{
						gun_offset = &actor_definition->perception.gun_offset_stand;
						color = global_real_argb_pink;
					}
				}
			}

			if (gun_offset==NULL)
			{
				estimated_position = actor->input.position.head_position;
			}
			else
			{
				unit_estimate_position(
					actor->meta.unit_index,
					_unit_estimate_gun_position,
					&actor->input.position.body_position,
					&desired_facing,
					gun_offset,
					&estimated_position);
			}

			render_debug_vector(TRUE, &estimated_position, &actor->input.aiming_vector, 1.f, color);
		}

		/* Targets */

		if ((ai_debug.render_targets || ai_debug.render_targets_last_visible) &&
			actor->target.target_type!=_actor_target_none &&
			actor->target.target_prop_index!=NONE)
		{
			real_point3d actor_target_position;
			real_point3d prop_target_position;
			struct prop_datum *prop = prop_get(actor->target.target_prop_index);
			real_argb_color const *target_color = global_real_argb_white;

			switch (actor->target.target_type)
			{
			case _actor_target_partial_enemy:
				target_color = global_real_argb_grey;
				break;
			case _actor_target_dead_enemy:
				target_color = global_real_argb_green;
				break;
			case _actor_target_disregarded_orphan:
				target_color = global_real_argb_salmon;
				break;
			case _actor_target_inspected_orphan:
				target_color = global_real_argb_blue;
				break;
			case _actor_target_uninspected_orphan:
				target_color = global_real_argb_lightblue;
				break;
			case _actor_target_definite_orphan:
				target_color = global_real_argb_cyan;
				break;
			case _actor_target_acknowledged_enemy:
				target_color = global_real_argb_purple;
				break;
			case _actor_target_clear_line_of_sight_enemy:
				target_color = global_real_argb_yellow;
				break;
			case _actor_target_potentially_dangerous_enemy:
				target_color = global_real_argb_orange;
				break;
			case _actor_target_visible_enemy:
				target_color = global_real_argb_red;
				break;
			case _actor_target_damaging_enemy:
				target_color = global_real_argb_magenta;
				break;
			default:
				match_vassert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 2653, FALSE, NULL);
			}

			point_from_line3d(&actor->input.position.head_position, global_down3d, 0.01f, &actor_target_position);
			point_from_line3d(&prop->head_position, global_down3d, 0.01f, &prop_target_position);
			render_debug_line(TRUE, &actor_target_position, &prop_target_position, target_color);

			if (ai_debug.render_targets_last_visible && prop->last_visible_time!=NONE)
			{
				render_debug_sphere(TRUE, &prop->last_visible_head_position, 0.2f, target_color);
			}
			
			if (prop->unreachable_ticks>0 && (!ai_debug.render_props || !ai_debug.render_props_unreachable))
			{
				long time = prop->last_unreachable_time!=NONE ? game_time_get()-prop->last_unreachable_time : NONE;

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "unr %d %d", prop->unreachable_ticks, time),
					global_real_argb_darkgreen);
			}
		}

		/* States */

		if (ai_debug.render_states)
		{
			struct observer_result const *camera = observer_get_camera(0);

			if (camera)
			{
				real_point3d position;

				point_from_line3d(&camera->position, &camera->forward, 0.05f, &position);
				render_debug_line(TRUE, &position, &actor->input.position.head_position, actor_action_debug_color(actor_index));
			}
		}

		/* Current state */

		if (ai_debug.render_current_state)
		{
			if (actor->meta.encounter_index!=NONE)
			{
				struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);
				struct encounter_definition const *encounter_definition = TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
					struct encounter_definition);
				struct squad_definition const *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					actor->meta.squad_index,
					struct squad_definition);
				struct squad_datum *squad = encounter_get_squad(encounter, actor->meta.squad_index);

				if (squad->delay_timer>0)
				{
					if (squad->delay_timer_started)
					{
						render_debug_string_at_point(
							TRUE,
							ai_debug_drawstack(),
							csprintf(temporary, "delaying %d", squad->delay_timer),
							global_real_argb_green);
					}
					else
					{
						if (TEST_FLAG(squad_definition->flags, _squad_delay_forever_bit))
						{
							render_debug_string_at_point(TRUE, ai_debug_drawstack(), "delay forever", global_real_argb_green);
						}
						else
						{
							render_debug_string_at_point(TRUE, ai_debug_drawstack(), "delay not triggered", global_real_argb_green);
						}
					}
				}
			}

			switch (actor->state.action)
			{
			case _actor_action_flee:
			{
				struct flee_state_data *flee = &actor->state.action_data.flee;

				if (flee->has_approach_point)
				{
					render_debug_sphere(TRUE, &flee->approach_point, 0.25f, actor_action_debug_color(actor_index));
				}
				break;
			}
			case _actor_action_fight:
			{
				struct alert_state_data *alert = &actor->state.action_data.alert;

				if (alert->move_position_order>0)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "change %d", alert->move_position_order),
						actor_action_debug_color(actor_index));
				}
				break;
			}
			case _actor_action_guard:
			{
				struct guard_state_data *guard = &actor->state.action_data.guard;

				if (guard->wait_ticks>0)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "wait %d", guard->wait_ticks),
						actor_action_debug_color(actor_index));
				}

				if (guard->look_ticks>0)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "look %d", guard->look_ticks),
						actor_action_debug_color(actor_index));
				}

				if (guard->cower)
				{
					if (guard->cower_from_retreat)
					{
						render_debug_string_at_point(
							TRUE,
							ai_debug_drawstack(),
							csprintf(temporary, "retreat %d", actor->emotions.unopposable_retreat_timer),
							actor_action_debug_color(actor_index));
					}
					else
					{
						render_debug_string_at_point(
							TRUE,
							ai_debug_drawstack(),
							csprintf(temporary, "%s %d", guard->cower_panicked ? "panic" : "hide", guard->cower_ticks),
							actor_action_debug_color(actor_index));
					}
				}

				if (guard->has_guard_direction)
				{
					render_debug_vector(
						TRUE,
						&actor->input.position.head_position,
						&guard->guard_direction,
						2.5f,
						actor_action_debug_color(actor_index));
				}
				break;
			}
			case _actor_action_uncover:
			case _actor_action_search:
			{
				struct pursuit_location *pursuit_location = actor_get_pursuit_location(actor_index);
				long delay = 0;

				match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 2773, pursuit_location != NULL);

				if (pursuit_location->type==_pursuit_location_target)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "targ(%d)", actor->firing_positions.pursuit_positions_count),
						actor_action_debug_color(actor_index));
				}
				else if (pursuit_location->type==_pursuit_location_position)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "pt(%d)", actor->firing_positions.pursuit_positions_count),
						actor_action_debug_color(actor_index));
				}
				else
				{
					render_debug_string_at_point(1, ai_debug_drawstack(), "undirected", actor_action_debug_color(actor_index));
				}

				if (actor->state.action==_actor_action_uncover)
				{
					delay = actor->state.action_data.uncover.uncover_remaining_time;
				}
				else if (actor->state.action==_actor_action_search)
				{
					struct search_state_data *search = &actor->state.action_data.search;

					delay = MIN(120-search->search_failure_timer, search->search_remaining_time);
				}

				render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "delay %d", delay), actor_action_debug_color(actor_index));
				break;
			}
			case _actor_action_vehicle:
				if (actor->state.action_data.vehicle.started_entry)
				{
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "entering", global_real_argb_darkgreen);
				}
				else
				{
					char buffer[1024];

					strcpy(buffer, "");

					if (actor->state.action_data.vehicle.currently_correct_facing)
					{
						strcat(buffer, "facing-ok ");
					}
					
					if (actor->state.action_data.vehicle.currently_within_range)
					{
						strcat(buffer, "range-ok ");
					}

					if (actor->state.action_data.vehicle.fake_entry_potential_timer>0)
					{
						strcat(buffer, csprintf(temporary, "fake-entry %d ", actor->state.action_data.vehicle.fake_entry_potential_timer));
					}

					if (actor_path_has_path(actor_index))
					{
						if (actor_path_at_destination(actor_index))
						{
							strcat(buffer, "destination-");
						}

						strcat(buffer, "moving ");
					}

					if (actor->state.action_data.vehicle.lock_facing)
					{
						strcat(buffer, "locked ");
					}

					render_debug_string_at_point(TRUE, ai_debug_drawstack(), buffer, global_real_argb_darkgreen);
				}

				render_debug_line(
					TRUE,
					&actor->input.position.body_position,
					&actor->state.action_data.vehicle.destination_point,
					global_real_argb_darkgreen);
				render_debug_vector(
					TRUE,
					&actor->state.action_data.vehicle.destination_point,
					&actor->state.action_data.vehicle.destination_facing,
					1.f,
					global_real_argb_yellow);
				break;
			case _actor_action_charge:
				switch (actor->state.action_data.charge.goal)
				{
				case _charge_goal_close_range:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "charge", global_real_argb_red);
					break;
				case _charge_goal_stalking:
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(
							temporary,
							"stalk%s %s disc%d",
							actor->state.action_data.charge.stalking_catch_target ? " catchtarget" : "",
							actor->state.action_data.charge.stalking_currently_exposed ? " exposed" : "",
							actor->state.action_data.charge.stalking_discovery_timer),
						global_real_argb_blue);
					break;
				case _charge_goal_melee:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "melee", global_real_argb_red);
					break;
				case _charge_goal_melee_leaping:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "melee-leap", global_real_argb_red);
					break;
				case _charge_goal_vehicle_strafing:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "strafe", global_real_argb_red);
					break;
				case _charge_goal_vehicle_ramming:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "ramming", global_real_argb_red);
					break;
				default:
					break;
				}

				if (actor->emotions.berserk)
				{
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "berserk", global_real_argb_red);
				}
				break;
			case _actor_action_obey:
			{
				struct obey_state_data *obey = &actor->state.action_data.obey;
				struct ai_command_definition *command = NULL;

				if (obey->command_list_index!=NONE)
				{
					struct obey_individual_simple_control *simple_control;
					struct obey_individual_complex_control *complex_control;
					struct ai_command_list_definition const* command_list = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->ai_command_lists,
						obey->command_list_index,
						struct ai_command_list_definition);
					
					if (!actor->meta.swarm)
					{
						simple_control = &obey->simple_control;
						complex_control = &obey->complex_control;

						render_debug_string_at_point(
							TRUE,
							ai_debug_drawstack(),
							csprintf(
								temporary,
								"command-list %s: #%d of #%d",
								command_list->name,
								simple_control->current_command_index+1,
								command_list->commands.count),
							global_real_argb_purple);
						
						if (simple_control->current_command_index<command_list->commands.count)
						{
							command = TAG_BLOCK_GET_ELEMENT(
								&command_list->commands,
								simple_control->current_command_index,
								struct ai_command_definition);
						}

						if (TEST_FLAG(simple_control->metadata_flags, _obey_metadata_commands_finished_bit))
						{
							render_debug_string_at_point(TRUE, ai_debug_drawstack(), "finished", global_real_argb_purple);
						}

						if (command)
						{
							boolean v427 = FALSE;
							boolean v426 = FALSE;
							boolean v425 = FALSE;
							boolean v424 = FALSE;

							switch (command->atom_type)
							{
							case _ai_atom_go_to:
								v427 = TRUE;
								v426 = TRUE;
								break;
							case _ai_atom_go_to_and_face:
								v427 = TRUE;
								v426 = TRUE;
								v425 = TRUE;
								break;
							case _ai_atom_move_direction:
								v426 = TRUE;
								v424 = TRUE;
								break;
							case _ai_atom_look:
							case _ai_atom_shoot:
							case _ai_atom_grenade:
								v426 = TRUE;
								break;
							default:
								break;
							}

							if (v427)
							{
								if (complex_control->destination_valid)
								{
									real radius = complex_control->destination_radius_valid ? complex_control->destination_radius : 0.5f;

									render_debug_sphere(TRUE, &complex_control->destination_point, radius, global_real_argb_purple);
								}
							}

							if (v426)
							{
								if (command->point1_index>=0 &&
									command->point1_index<command_list->points.count)
								{
									real_point3d position;
									struct ai_command_point_definition const *point = TAG_BLOCK_GET_ELEMENT(
										&command_list->points,
										command->point1_index,
										struct ai_command_point_definition);

									render_debug_line(
										TRUE,
										&actor->input.position.head_position,
										&point->position,
										global_real_argb_purple);
									point_from_line3d(&point->position, global_up3d, 0.1f, &position);
									render_debug_string_at_point(
										TRUE,
										&position,
										csprintf(temporary, "%d", command->point1_index),
										global_real_argb_purple);
								}
								else
								{
									if (v424 && command->parameter2>=0.f && command->parameter2<360.f)
									{
										real_vector3d v;

										vector3d_from_angle(&v, command->parameter2);
										render_debug_vector(TRUE, &actor->input.position.head_position, &v, 1.5f, global_real_argb_purple);
									}
									else
									{
										render_debug_string_at_point(
											TRUE,
											ai_debug_drawstack(),
											csprintf(temporary, "error: invalid point 1 specified (%d)", command->point1_index),
											global_real_argb_purple);
									}
								}
							}

							if (v425)
							{
								if (command->point2_index>=0 &&
									command->point2_index<command_list->points.count)
								{
									real_point3d position;
									struct ai_command_point_definition const *point = TAG_BLOCK_GET_ELEMENT(
										&command_list->points,
										command->point2_index,
										struct ai_command_point_definition);

									render_debug_line(
										TRUE,
										&actor->input.position.head_position,
										&point->position,
										global_real_argb_purple);
									point_from_line3d(&point->position, global_up3d, 0.1f, &position);
									render_debug_string_at_point(1, &position, csprintf(temporary, "%d", command->point2_index), global_real_argb_purple);

								}
								else
								{
									render_debug_string_at_point(
										TRUE,
										ai_debug_drawstack(),
										csprintf(temporary, "error: invalid point 2 specified (%d)", command->point2_index),
										global_real_argb_purple);
								}
							}
						}
					}

					if (command)
					{
						action_obey_describe_command(global_scenario_get(), command, temporary, NUMBEROF(temporary));
						render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_purple);
					}
				}
				break;
			}
			default:
				break;
			}
		}

		/* Shooting */

		if (ai_debug.render_shooting)
		{
			if (actor_debug_info->firing_decision!=_firing_no_target)
			{
				char const *string = actor_move_animation_busy(actor_index) ? "busy " : "";

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(), 
					csprintf(
						temporary,
						"%srof %.1f err %.1f dmg %.1f blk %d",
						string,
						actor_debug_info->shooting_rof,
						actor->control.burst_error,
						actor->control.burst_damage_modifier,
						actor->control.blocked_communication_timer),
					global_real_argb_white);


				if (actor->emotions.berserk)
				{
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "berserk", global_real_argb_yellow);
				}
				else if (actor->control.firing_at_new_target)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(
							temporary,
							"newtarget %d", 
							((actor_variant_definition->ranged_combat.new_target_pattern_time)*TICKS_PER_SECOND) - actor->control.current_fire_target_timer
						),
						global_real_argb_blue);
				}
				else if (actor->control.firing_while_moving)
				{
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "moving", global_real_argb_green);
				}

				switch (actor->control.fire_state)
				{
				case _actor_fire_state_none:
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						"none",
						global_real_argb_red);
					break;
				case _actor_fire_state_holding:
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "delay %d", actor->control.fire_state_timer),
						global_real_argb_red);
					break;
				case _actor_fire_state_bursting:
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "burst %d", actor->control.fire_state_timer),
						global_real_argb_red);
					break;
				case _actor_fire_state_pausing:
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "pause %d", actor->control.fire_state_timer),
						global_real_argb_red);
					break;
				case _actor_fire_state_wild:
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "wildfire %d", actor->control.fire_state_timer), 
						global_real_argb_red);
					break;
				default:
					match_vassert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 3081, FALSE, NULL);
					break;
				}
			}

			if (actor_debug_info->firing_decision >= 0 && actor_debug_info->firing_decision < NUMBER_OF_ACTOR_DEBUG_FIRING_DECISIONS)
			{
				char const *firing_decision_names[NUMBER_OF_ACTOR_DEBUG_FIRING_DECISIONS] =
				{
					"firing disabled",
					"animation busy",
					"wrong target",
					"no target",
					"outside active region",
					"not visible",
					"outside range",
					"blocked",
					"first burst align",
					"first burst delay",
					"burst pause align",
					"burst pause",
					"firing wildly",
					"bursting",
					"in midair",
					"not crouching",
					"not standing",
					"not stationary",
					"underwater",
					"minimum range"
				};

				render_debug_string_at_point(TRUE, ai_debug_drawstack(), firing_decision_names[actor_debug_info->firing_decision], global_real_argb_blue);
			}
		}

		/* Grenade decisions */

		if (ai_debug.render_grenade_decisions)
		{
			if (actor_debug_info->grenade_eval_time!=NONE &&
				actor_debug_info->grenade_eval_time+7 >= game_time_get())
			{
				switch (actor_debug_info->grenade_decision)
				{
				case _grenade_vehicle:
					csstrcpy(temporary, "in vehicle");
					break;
				case _grenade_unit_busy:
					csstrcpy(temporary, "unit busy");
					break;
				case _grenade_being_hurt:
					sprintf(temporary, "dmg %.2f", actor_debug_info->grenade_current_damage);
					break;
				case _grenade_no_grenades:
					csstrcpy(temporary, "no grenades");
					break;
				case _grenade_random_failed:
					sprintf(
						temporary,
						"random %.2f > %.2f",
						actor_debug_info->grenade_random_value,
						actor_debug_info->grenade_random_chance);
					break;
				case _grenade_encounter_timeout:
					sprintf(temporary, "encounter time %d", actor_debug_info->grenade_encounter_timeout_ticks);
					break;
				case _grenade_target_failed:
					csstrcpy(temporary, "no target");
					break;
				case _grenade_not_enough_enemies:
					sprintf(
						temporary,
						"not enough enemy %d < %d",
						actor_debug_info->grenade_enemy_count,
						actor_debug_info->grenade_required_enemy_count);
					break;
				case _grenade_collateral_damage:
					csstrcpy(temporary, "collateral dmg");
					break;
				case _grenade_trajectory_failed:
					csstrcpy(temporary, "no trajectory");
					break;
				case _grenade_success:
					csstrcpy(temporary, "success");
					break;
				default:
					csstrcpy(temporary, "<unknown>");
					break;
				}

				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_pink);
			}

			if (actor->control.grenade_trying_to_throw)
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(
						temporary,
						"trying: %sbusy dmg %.1f",
						actor->meta.unit_index!=NONE && unit_is_busy(actor->meta.unit_index) ? "" : "not-",
						unit==NULL ? 0.f : unit->object.current_body_damage),
					global_real_argb_pink);
			}
		}

		/* Danger zones */

		if (ai_debug.render_danger_zones && actor->danger_zone.danger_type>0)
		{
			real_argb_color const *color = actor->danger_zone.currently_perceived ?
				(actor->danger_zone.acknowledgement_timer>0 ?
					global_real_argb_white :
					(actor->danger_zone.noticed_danger ? global_real_argb_yellow : global_real_argb_blue)) :
				global_real_argb_darkgreen;

			render_debug_line(TRUE, &actor->input.position.head_position, &actor->danger_zone.position, color);
			render_debug_sphere(TRUE, &actor->danger_zone.position, actor->danger_zone.danger_radius, global_real_argb_red);
			render_debug_vector(TRUE, &actor->danger_zone.position, &actor->danger_zone.velocity, 45.f, global_real_argb_red);
			render_debug_sphere(
				TRUE,
				&actor->danger_zone.bounding_sphere_center,
				actor->danger_zone.bounding_sphere_radius,
				global_real_argb_orange);

			if (actor->danger_zone.danger_type==_actor_unopposable_danger_shooting)
			{
				render_debug_string_at_point(
					TRUE,
					&actor->danger_zone.position,
					actor->danger_zone.projectile.time_until_explosion==NONE ?
						"NONE" :
						csprintf(temporary, "%d", actor->danger_zone.projectile.time_until_explosion),
					color);
			}

			if (actor->danger_zone.danger_type==_actor_unopposable_danger_visible)
			{
				render_debug_string_at_point(
					TRUE,
					&actor->danger_zone.position,
					actor->danger_zone.projectile.time_until_explosion==NONE ?
						"NONE" :
						csprintf(temporary, "%d", actor->danger_zone.projectile.time_until_explosion),
					color);
			}

			if (actor_debug_info->danger_avoidance_time!=NONE)
			{
				if (actor_debug_info->danger_avoidance_time+15 >= game_time_get())
				{
					boolean avoiding;

					if (actor_debug_info->danger_abandoned_path)
					{
						render_debug_string_at_point(TRUE, ai_debug_drawstack(), "discarded fp", global_real_argb_magenta);
					}

					avoiding = TRUE;

					switch (actor_debug_info->danger_decision)
					{
					case _danger_avoidance_none:
						avoiding = FALSE;
						break;
					case _danger_avoidance_unnoticed:
						strcpy(temporary, "unnoticed");
						break;
					case _danger_avoidance_animation_busy:
						strcpy(temporary, "animation busy");
						break;
					case _danger_avoidance_vehicle:
						strcpy(temporary, "in vehicle");
						break;
					case _danger_avoidance_far_away:
						sprintf(
							temporary,
							"far away (%.1f > %.1f)",
							actor_debug_info->danger_far_dist,
							actor_debug_info->danger_far_radius);
						break;
					case _danger_avoidance_outside_zone:
						sprintf(
							temporary,
							"outside (%.1f > %.1f)",
							actor_debug_info->danger_zone_dist,
							actor_debug_info->danger_zone_radius);
						break;
					case _danger_avoidance_evasion_disallowed:
						strcpy(temporary, "evasion not allowed");
						break;
					case _danger_avoidance_no_safe_direction:
						strcpy(temporary, "no safe direction");
						break;
					case _danger_avoidance_no_desire:
						if (actor_debug_info->danger_intersect_time==REAL_MAX)
						{
							strcpy(temporary, "no desire (no int'n)");
						}
						else
						{
							sprintf(temporary, "no desire (int'n %.1f)", actor_debug_info->danger_intersect_time);
						}
						break;
					case _danger_avoidance_can_avoid:
						strcpy(temporary, "can avoid");
						break;
					case _danger_avoidance_imminent_explosion:
						strcpy(temporary, "imminent explosion");
						break;
					case _danger_avoidance_imminent_impact:
						strcpy(temporary, "imminent impact");
						break;
					case _danger_avoidance_no_animation:
						strcpy(temporary, "no animation");
						break;
					case _danger_avoidance_attached_to_us:
						strcpy(temporary, "attached to us");
						break;
					default:
						strcpy(temporary, "<error>");
						break;
					}

					if (avoiding)
					{
						render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_magenta);
					}
				}
			}

			if (actor_debug_info->dive_decision_time!=NONE &&
				actor_debug_info->dive_decision_time+15 >= game_time_get())
			{
				switch (actor_debug_info->dive_decision)
				{
				case _dive_not_attempted:
					csstrcpy(temporary, "not attempted");
					break;
				case _dive_cannot_move:
					csstrcpy(temporary, "cannot move");
					break;
				case _dive_no_animation:
					csstrcpy(temporary, "animation unavailable");
					break;
				case _dive_animation_failure:
					csstrcpy(temporary, "animation failed");
					break;
				case _dive_success:
					csstrcpy(temporary, "success");
					break;
				default:
					csstrcpy(temporary, "<error>");
					break;
				}

				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_purple);
			}
		}

		/* Trigger */

		if (ai_debug.render_trigger && TEST_FLAG(actor->output.control_flags, _unit_control_weapon_primary_trigger_bit))
		{
			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(temporary, "trigger %.1f", actor->output.analog_primary_trigger),
				global_real_argb_yellow);
		}

		/* Control */

		if (ai_debug.render_control && actor->meta.unit_index!=NONE)
		{
			short control_flag_bit;
			short flag_count = NUMBER_OF_UNIT_CONTROL_FLAGS;

			char const *control_flag_names[] =
			{
				"crouch",
				"jump",
				"user1",
				"user2",
				"light",
				"exactfacing",
				"action",
				"equipment",
				"lookdontturn",
				"forcealert",
				"reload",
				"trigger",
				"trigger2",
				"grenade"
			};

			short count = 0;

			strcpy(temporary, "");

			for (control_flag_bit = 0; control_flag_bit<flag_count; ++control_flag_bit)
			{
				if (TEST_FLAG(actor->output.control_flags, control_flag_bit))
				{
					if (count > 0)
					{
						strcat(temporary, " ");
					}

					if (control_flag_bit<_unit_control_swap_weapons_bit)
					{
						strcat(temporary, control_flag_names[control_flag_bit]);
					}
					else
					{
						char string[80];

						sprintf(string, "<unknown %d>", control_flag_bit);
						strcat(temporary, string);
					}

					++count;
				}
			}
			
			if (count>0)
			{
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_yellow);
			}

			count = 0;

			strcpy(temporary, "");

			for (control_flag_bit = 0; control_flag_bit<flag_count; ++control_flag_bit)
			{
				if (TEST_FLAG(actor->output.persistent_control_flags, control_flag_bit))
				{
					if (count > 0)
					{
						csstrcat(temporary, " ");
					}

					if (control_flag_bit<_unit_control_swap_weapons_bit)
					{
						strcat(temporary, control_flag_names[control_flag_bit]);
					}
					else
					{
						char string[80];

						sprintf(string, "<unknown %d>", control_flag_bit);
						strcat(temporary, string);
					}

					++count;
				}
			}

			if (count>0)
			{
				char string[80];

				sprintf(string, ": persistent %d", actor->output.persistent_control_ticks);
				strcat(temporary, string);
	
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_orange);
			}

			{
				char const *movement_type_strings[NUMBER_OF_ACTOR_MOVEMENT_TYPES] =
				{
					"noncombat",
					"asleep",
					"combat",
					"flee",
					NULL
				};

				char const *aiming_speed_names[NUMBER_OF_UNIT_AIMING_SPEEDS] =
				{
					"alert",
					"casual"
				};
				
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(), csprintf(
						temporary,
						"m-%s a-%s",
						movement_type_strings[actor->output.movement_type],
						aiming_speed_names[actor->output.aiming_speed]),
					global_real_argb_magenta);
			}

			if (TEST_FLAG(actor->output.control_flags, _unit_control_weapon_primary_trigger_bit))
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "analog %.2f", actor->output.analog_primary_trigger), 
					global_real_argb_cyan);
			}

			if (actor->output.animation.impulse!=NONE)
			{
				real_point3d base_point;
				real_vector3d alignment_vector_3d;

				point_from_line3d(&actor->input.position.body_position, global_up3d, 0.2f, &base_point);
				
				alignment_vector_3d.i = actor->output.animation.alignment_vector.i;
				alignment_vector_3d.j = actor->output.animation.alignment_vector.j;
				alignment_vector_3d.k = 0.f;

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(), 
					csprintf(temporary, "animation %d", actor->output.animation.impulse),
					global_real_argb_pink);
				render_debug_vector(
					TRUE,
					&base_point,
					&alignment_vector_3d,
					1.f,
					global_real_argb_pink);
			}

			{
				real_point3d p0;
				real_point3d p1;
				real_vector3d facing_vector;
				real_vector3d right_facing_vector;

				point_from_line3d(&actor->input.position.head_position, global_up3d, 0.01f, &p0);
				point_from_line3d(&actor->input.position.head_position, &actor->output.facing_vector, 1.3f, &p1);
				point_from_line3d(&p1, global_up3d, 0.01f, &p1);
				
				render_debug_line(TRUE, &p0, &p1, global_real_argb_red);
				
				point_from_line3d(&actor->input.position.head_position, &actor->output.aiming_vector, 1.2f, &p1);
				point_from_line3d(&p1, global_up3d, 0.02f, &p1);
				
				render_debug_line(TRUE, &p0, &p1, global_real_argb_green);
				
				point_from_line3d(&actor->input.position.head_position, &actor->output.looking_vector, 1.1f, &p1);
				point_from_line3d(&p1, global_up3d, 0.03f, &p1);

				render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);

				if (unit->object.type==_object_type_biped && unit->object.parent_object_index==NONE)
				{
					real_vector3d throttle_vector;
					struct biped_definition const *biped_definition = biped_definition_get(unit->definition_index);

					unit_get_facing_vector(actor->meta.unit_index, &facing_vector);

					if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit))
					{
						real_vector3d left_vector;
						real_vector3d up_vector;

						biped_build_flying_axes(&facing_vector, &left_vector, &up_vector);
						scale_vector3d(&facing_vector, actor->output.throttle.i, &throttle_vector);
						vector_from_line3d(&throttle_vector, &left_vector, actor->output.throttle.j, &throttle_vector);
						vector_from_line3d(&throttle_vector, &up_vector, actor->output.throttle.k, &throttle_vector);
					}
					else
					{
						set_real_vector3d(&right_facing_vector, -facing_vector.j, facing_vector.i, 0.f);
						scale_vector3d(&facing_vector, actor->output.throttle.i, &throttle_vector);
						vector_from_line3d(&throttle_vector, &right_facing_vector, actor->output.throttle.j, &throttle_vector);
					}

					point_from_line3d(&actor->input.position.body_position, global_up3d, 0.1f, &p0);
					render_debug_vector(TRUE, &p0, &throttle_vector, 1.f, global_real_argb_purple);
				}
			}
		}
		
		/* Charge decisions */

		if (ai_debug.render_charge_decisions &&
			actor_debug_info->charge_last_time!=NONE &&
			actor_debug_info->charge_last_time+TICKS_PER_SECOND >= game_time_get())
		{
			switch (actor_debug_info->charge_decision)
			{
			case _charge_vehicle_success:
				csstrcpy(temporary, "vehicle-success");
				break;
			case _charge_vehicle_not_driver:
				csstrcpy(temporary, "vehicle-notdriver");
				break;
			case _charge_melee_swarm_cant:
				csstrcpy(temporary, "melee-swarmcan't");
				break;
			case _charge_melee_inhibited:
				csstrcpy(temporary, "melee-inhibited");
				break;
			case _charge_melee_notarget:
				csstrcpy(temporary, "melee-notarget");
				break;
			case _charge_melee_no_animation:
				sprintf(temporary, "melee-noanimation (%sairborne)", actor_debug_info->field_198 ? "" : "not-");
				break;
			case _charge_melee_cannot_move:
				sprintf(temporary, "melee-cannotmove (%f)", actor_debug_info->field_194);
				break;
			case _charge_melee_success:
				sprintf(temporary, "melee-success (%sairborne)", actor_debug_info->field_198 ? "" : "not-" );
				break;
			case _charge_stalking_success:
				csstrcpy(temporary, "stalking-success");
				break;
			case _charge_close_success:
				csstrcpy(temporary, "close-success");
				break;
			default:
				sprintf(temporary, "<unknown charge-setup decision %d>", actor_debug_info->charge_decision);
				break;
			}

			render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_red);
		}
		
		/* Melee check */

		if (ai_debug.render_melee_check &&
			actor_debug_info->last_melee_time!=NONE &&
			actor_debug_info->last_melee_time+TICKS_PER_SECOND >= game_time_get())
		{
			render_debug_vector(TRUE, &actor_debug_info->field_108, &actor_debug_info->field_114, 1.5f, global_real_argb_red);
			render_debug_point(TRUE, &actor_debug_info->field_120, 0.2f, global_real_argb_red);
		
			if (!actor_debug_info->field_139)
			{
				real_point3d p0;
				real_point3d p1;
				real_point3d p2;
				real_vector3d v;

				render_debug_line(TRUE, &actor_debug_info->field_120, &actor_debug_info->field_13C, global_real_argb_purple);
				render_debug_sphere(TRUE, &actor_debug_info->field_13C, 0.2f, global_real_argb_purple);
				
				point_from_line3d(&actor_debug_info->field_108, &actor_debug_info->field_12C, (actor_debug_info->field_14C), &p1);
				render_debug_line(TRUE, &actor_debug_info->field_108, &p1, global_real_argb_green);
				
				point_from_line3d(&actor_debug_info->field_108, &actor_debug_info->field_12C, (actor_debug_info->field_148), &p0);
				perpendicular3d(&actor_debug_info->field_12C, &v);
				
				point_from_line3d(&p0, &v, 0.3f, &p1);
				point_from_line3d(&p0, &v, -0.3f, &p2);
				render_debug_line(TRUE, &p1, &p2, global_real_argb_green);
			}

			render_debug_vector(
				TRUE,
				&actor_debug_info->field_108,
				&actor_debug_info->field_12C,
				2.f,
				actor_debug_info->field_138 ? (actor_debug_info->field_139 ? global_real_argb_yellow : global_real_argb_purple) : global_real_argb_white);
		}

		/* Vehicle avoidance */

		if (ai_debug.render_vehicle_avoidance &&
			actor_debug_info->last_vehicle_avoidance_time!=NONE &&
			actor_debug_info->last_vehicle_avoidance_time+TICKS_PER_SECOND >= game_time_get())
		{
			real_vector3d v0;
			real_point3d p2;

			render_debug_line(TRUE, &actor_debug_info->vehicle_avoidance_point, &actor_debug_info->vehicle_intended_entry_point, global_real_argb_green);
			render_debug_sphere(TRUE, &actor_debug_info->vehicle_center, actor_debug_info->vehicle_radius, global_real_argb_yellow);
		
			{
				real_point3d p0;
				real_point3d p1;

				ai_debug_render_cross(&actor_debug_info->vehicle_intended_entry_point, global_real_argb_red);
				vector_from_points3d(&actor_debug_info->vehicle_avoidance_point, &actor_debug_info->vehicle_intended_entry_point, &v0);
				point_from_line3d(&actor_debug_info->vehicle_avoidance_point, &v0, (actor_debug_info->vehicle_intersect_t), &p2);
			
				ai_debug_render_cross(&p2, global_real_argb_blue);
			}

			if (actor_debug_info->vehicle_modified)
			{
				real_point3d p0;
				real_point3d p1;

				render_debug_line(TRUE, &actor_debug_info->vehicle_center, &actor_debug_info->vehicle_modified_point, global_real_argb_yellow);
				render_debug_line(TRUE, &actor_debug_info->vehicle_avoidance_point, &actor_debug_info->vehicle_modified_point, global_real_argb_red);
				
				ai_debug_render_cross(&actor_debug_info->vehicle_modified_point, global_real_argb_blue);
			}
		}

		/* Projectile aiming */

		if (ai_debug.render_projectile_aiming &&
			actor_debug_info->last_projectile_aiming_time!=NONE &&
			actor_debug_info->last_projectile_aiming_time+15 >= game_time_get())
		{
			real_point3d p0;
			real_point3d p1;

			ai_debug_render_cross(&actor_debug_info->aim_last_origin, global_real_argb_blue);
			
			render_debug_vector(
				TRUE,
				&actor_debug_info->aim_last_origin,
				&actor_debug_info->aim_last_vector,
				2.f,
				actor_debug_info->aim_last_by_vector ? global_real_argb_blue : global_real_argb_red);
			
			if (!actor_debug_info->aim_last_by_vector)
			{
				render_debug_sphere(TRUE, &actor_debug_info->aim_last_target, 0.2f, global_real_argb_red);
			}

			if (actor_debug_info->aim_last_rotated)
			{
				render_debug_vector(
					TRUE,
					&actor->input.position.head_position,
					&actor_debug_info->aim_last_rotated_original_vector,
					1.f,
					global_real_argb_white);
				render_debug_vector(
					TRUE,
					&actor_debug_info->aim_last_origin,
					&actor_debug_info->aim_last_rotated_vector,
					2.f,
					global_real_argb_purple);
			}
		}

		/* Burst Geometry */

		if (ai_debug.render_burst_geometry && actor->control.fire_state==_actor_fire_state_bursting)
		{
			real_point3d p0;
			real_point3d p1;

			render_debug_line(TRUE, &actor->control.burst_initial_position, &actor_debug_info->burst_last_known_position, global_real_argb_green);
			
			if (magnitude_squared3d(&actor_debug_info->burst_lead_vector)>_real_epsilon)
			{
				render_debug_vector(
					TRUE,
					&actor_debug_info->burst_tracked_position,
					&actor_debug_info->burst_lead_vector,
					1.f,
					global_real_argb_purple);
			}

			ai_debug_render_cross(&actor->control.burst_origin, global_real_argb_blue);

			render_debug_sphere(TRUE, &actor->control.burst_target, 0.1f, global_real_argb_red);
			render_debug_vector(TRUE, &actor->control.burst_target, &actor->control.burst_adjustment, 5.f, global_real_argb_red);
		}


		/*  Vision cones */

		if (render_exclusive && ai_debug.render_vision_cones)
		{
			real max_distance;
			real perception_factor;
			real_point3d last_points[2][2][2];
			real angle_itr;
			real const angle_step = 0.08726646f;

			real_argb_color const *const *colors[2] =
			{
				&global_real_argb_red,
				&global_real_argb_blue
			};

			if (actor_debug_info->vision_last_time!=NONE &&
				actor_debug_info->vision_last_time+15 >= game_time_get())
			{
				max_distance = actor_debug_info->vision_last_maximum_distance;
				perception_factor = actor_debug_info->vision_last_perception_factor;
			}
			else
			{
				max_distance = actor_definition->perception.maximum_vision_distance;
				perception_factor = 1.f;
			}

			render_debug_vector(
				TRUE,
				&actor->input.position.head_position,
				&actor->input.looking_vector,
				max_distance*perception_factor,
				global_real_argb_yellow);

			for (angle_itr = 0.f; angle_itr<actor_definition->perception.peripheral_vision_angle+angle_step; angle_itr+=angle_step)
			{
				real_point3d current_points[2][2][2];
				real distances[2];
				real_vector3d direction_vector[2][2];
				short side_index;
				short ring_index;
				short height_index;
				real actual_angle = MIN(angle_itr, actor_definition->perception.peripheral_vision_angle);

				{
					real cosine_vertical_angle[2];
					real sine_vertical_angle[2];
					real cosine_horizontal_angle = cosine(actual_angle);
					real sine_horizontal_angle = sine(actual_angle);

					cosine_vertical_angle[0] = cosine(DEGREES_TO_RADIANS(30));
					sine_vertical_angle[0] = sine(DEGREES_TO_RADIANS(30));
					cosine_vertical_angle[1] = cosine(DEGREES_TO_RADIANS(45));
					sine_vertical_angle[1] = -sine(DEGREES_TO_RADIANS(45));

					for (ring_index = 0; ring_index<2; ++ring_index)
					{
						for (height_index = 0; height_index<2; ++height_index)
						{
							real_vector3d headspace_vector;
							headspace_vector.i = cosine_horizontal_angle * cosine_vertical_angle[height_index];
							headspace_vector.j = ((real)(ring_index==0 ? 1 : -1)) * cosine_vertical_angle[height_index] * sine_horizontal_angle;
							headspace_vector.k = sine_vertical_angle[height_index];

							direction_vector[ring_index][height_index] = *global_zero_vector3d;

							vector_from_line3d(&direction_vector[ring_index][height_index], &actor->input.looking_vector, headspace_vector.i, &direction_vector[ring_index][height_index]);
							vector_from_line3d(&direction_vector[ring_index][height_index], &actor->input.looking_left_vector, headspace_vector.j, &direction_vector[ring_index][height_index]);
							vector_from_line3d(&direction_vector[ring_index][height_index], &actor->input.looking_up_vector, headspace_vector.k, &direction_vector[ring_index][height_index]);
						}
					}
				}

				actor_get_vision_distances(actor_index, max_distance, perception_factor, actual_angle, &distances[0], &distances[1]);
			
				for (side_index = 0; side_index < 2; ++side_index)
				{
					for (ring_index = 0; ring_index < 2; ++ring_index)
					{
						for (height_index = 0; height_index < 2; ++height_index)
						{
							point_from_line3d(
								&actor->input.position.head_position,
								&direction_vector[ring_index][height_index],
								distances[side_index],
								&current_points[side_index][ring_index][height_index]);

							if (angle_itr>0.f || ring_index==0)
							{
								render_debug_line(
									TRUE,
									&current_points[side_index][ring_index][height_index],
									&actor->input.position.head_position,
									*colors[side_index]);
							}

							if (angle_itr>0.f)
							{
								render_debug_line(
									TRUE,
									&last_points[side_index][ring_index][height_index],
									&current_points[side_index][ring_index][height_index],
									*colors[side_index]);
							}
						}

						if (angle_itr>0.f || ring_index==0)
						{
							render_debug_line(TRUE, &current_points[side_index][ring_index][0], &current_points[side_index][ring_index][1], *colors[side_index]);
						}
					}
				}

				memcpy(last_points, current_points, sizeof(last_points));
			}
		}

		/* Detailed state */

		if (render_exclusive &&ai_debug.render_detailed_state)
		{
			char buffer[1024];
			struct prop_iterator iterator;
			struct prop_datum *prop;

			short tabs[7] =
			{
				100,
				175,
				250,
				325,
				400,
				475,
				550
			};

			sprintf(buffer, "|n|n|n|n");

			if (actor->meta.unit_index!=NONE)
			{
				sprintf(
					&buffer[strlen(buffer)],
					"body|t%3.2f|t[0.0,%3.2f]|nshield|t%3.2f|t[0.0,%3.2f]|n|n",
					object_get_actual_body_vitality(actor->meta.unit_index, FALSE),
					object_get_maximum_body_vitality(actor->meta.unit_index, FALSE),
					object_get_actual_shield_vitality(actor->meta.unit_index, FALSE),
					object_get_maximum_shield_vitality(actor->meta.unit_index, FALSE));
			}

			sprintf(&buffer[strlen(buffer)], "|ntype|tstate|tvis|taud|tlos|ttarget|tlook|n");

			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				char const *states[NUMBER_OF_PROP_STATES] =
				{
					"-----",
					"->ack",
					"ack->",
					" ack ",
					"u/orph",
					"i/orph"
				};

				char const *los[NUMBER_OF_AI_LINE_OF_SIGHTS] =
				{
					"clear",
					"occl",
					"f/cvr",
					"to/cvr",
					"obstr"
				};

				char const *lighting_state_strings[NUMBER_OF_PROP_LIGHTING_STATES] =
				{
					" (dark)",
					" (dim)",
					""
				};

				char const *perceptions[NUMBER_OF_ACTOR_PERCEPTION_TYPES] =
				{
					"",
					"part",
					"full"
				};

				sprintf(
					&buffer[strlen(buffer)],
					"%s|t%s|t%s%s|t%s|t%s|t%3.2f|t%3.2f|n",
					tag_get_name(object_get(prop->unit_index)->definition_index),
					states[prop->state],
					perceptions[prop->visibility],
					lighting_state_strings[prop->lighting],
					perceptions[prop->audibility],
					los[prop->line_of_sight],
					prop->target_weight,
					prop->look_interest);
			}

			draw_string_set_tab_stops(tabs, NUMBEROF(tabs));
			draw_string_set_color(global_real_argb_white);
			rasterizer_draw_string(NULL, NULL, NULL, 0, buffer);
			draw_string_set_tab_stops(NULL, 0);
		}

		/* Paths */

		if (ai_debug.render_paths && (!ai_debug.render_paths_selected_only || render_exclusive))
		{
			if (ai_debug.render_paths_current && actor_path_has_path(actor_index))
			{
				real_argb_color const *color;
				real_point3d position;
				short step_index;
				short first_index;

				if (actor->control.path.at_destination)
				{
					color = global_real_argb_yellow;
				}
				else
				{
					color = actor->control.path.path.steps_finish_path ? global_real_argb_pink : global_real_argb_purple;
				}

				first_index = actor->control.path.path.step_index;

				render_debug_line_offset(
					TRUE,
					&actor->input.position.body_position,
					&actor->control.path.path.steps[first_index].point,
					color,
					0.1f);

				for (
					step_index = first_index;
					step_index<actor->control.path.path.step_count;
					++step_index)
				{
					if (step_index>first_index)
					{
						render_debug_line_offset(
							TRUE,
							&actor->control.path.path.steps[step_index-1].point,
							&actor->control.path.path.steps[step_index].point,
							color,
							0.1f);
					}
						
					point_from_line3d(&actor->control.path.path.steps[step_index].point, global_up3d, 0.1f, &position);
					render_debug_tick(TRUE, &position, global_up3d, 0.02, color);
				}

				point_from_line3d(&actor->control.path.path.endpoint.point, global_up3d, 0.1f, &position);
				render_debug_sphere(TRUE, &position, 0.15f, color);
			}

			if (actor_path_has_path(actor_index))
			{
				sprintf(
					temporary,
					"following path (%d/%d%s)",
					actor->control.path.path.step_index,
					actor->control.path.path.step_count,
					actor_path_at_destination(actor_index) ? " (at destination)" : "");
			}
			else
			{
				strcpy(temporary, "no current path");
			}

			if (actor->emotions.ignorant_of_broken_surfaces)
			{
				strcat(temporary, " [ignorant]");
			}

			render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_orange);
			
			if (actor_debug_info->last_path_refresh==NONE || actor_debug_info->last_path_refresh+150 < game_time_get())
			{
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), "not refreshing path", global_real_argb_blue);
			}
			else
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "path refreshed (%d)", game_time_get()-actor_debug_info->last_path_refresh),
					global_real_argb_blue);
				
				if (path && path->valid)
				{
					ai_debug_render_path_storage(path);
				}
				else
				{
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "path debugging not available", global_real_argb_red);
				}
			}

		}

		/* Postcombat */

		if (ai_debug.render_postcombat && actor->external_orders.postcombat_type>0)
		{
			render_debug_string_at_point(TRUE, ai_debug_drawstack(), postcombat_type_strings[actor->external_orders.postcombat_type], global_real_argb_green);
			
			if (actor->external_orders.postcombat_prop_index!=NONE)
			{
				struct prop_datum *prop = prop_get(actor->external_orders.postcombat_prop_index);
				
				render_debug_line(TRUE, &actor->input.position.head_position, &prop->head_position, global_real_argb_green);
			}
		}
	}

	return;
}

static void ai_debug_render_path_storage(
	struct path_debug_storage *path)
{
	if (path && path->valid && path->last_render_id != ai_debug.last_render_id)
	{
		char const *path_traverse_result_strings[NUMBER_OF_PATH_TRAVERSE_RESULTS] =
		{
			"none",
			"invalid start",
			"not close enough",
			"exhausted search",
			"overflowed nodes",
			"success"
		};

		char const *path_build_result_strings[NUMBER_OF_PATH_BUILD_RESULTS] =
		{
			"none",
			"no destination",
			"cached node missing",
			"not close enough",
			"obstacles blocked",
			"success"
		};

		boolean const matching_bsp = path->structure_bsp_index==global_structure_bsp_index_get();

		match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 3944, (path->path_traverse_result >= 0) && (path->path_traverse_result < NUMBER_OF_PATH_TRAVERSE_RESULTS));
		match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 3945, (path->path_build_result >= 0) && (path->path_build_result < NUMBER_OF_PATH_BUILD_RESULTS));

		render_debug_string_at_point(
			TRUE,
			ai_debug_drawstack(),
			csprintf(
				temporary,
				"%s / %s (%d)",
				path_traverse_result_strings[path->path_traverse_result],
				path_build_result_strings[path->path_build_result],
				game_time_get()-path->path_time),
			path->path_traverse_result!=_path_traverse_result_success || path->path_build_result!=_path_build_result_success ?
			global_real_argb_red :
			global_real_argb_green
		);

		if (ai_debug.render_paths_destination)
		{
			if (path->path_state.destination_valid)
			{
				render_debug_line_offset(
					TRUE,
					&path->path_state.input.start_point,
					&path->path_state.destination.point,
					global_real_argb_pink,
					0.1f);
				render_debug_point(TRUE, &path->path_state.destination.point, 0.3f, global_real_argb_green);

				if (path->path_state.destination.target_radius > 0.f)
				{
					render_debug_sphere(
						TRUE,
						&path->path_state.destination.point,
						path->path_state.destination.target_radius,
						global_real_argb_green);
				}

				if (path->path_state.destination.surface_index!=NONE && matching_bsp)
				{
					ai_debug_render_surface(
						path->path_state.structure,
						path->path_state.destination.surface_index,
						0.05f,
						global_real_argb_green);
				}
			}
			else
			{
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), "undirected", global_real_argb_green);
			}
		}

		if (ai_debug.render_paths_raw)
		{
			ai_debug_render_path_line(
				&path->path_state.input.start_point,
				path->raw_step_count,
				path->raw_steps,
				global_real_argb_red);
		}

		if (ai_debug.render_paths_smoothed)
		{
			ai_debug_render_path_line(
				&path->path_state.input.start_point,
				path->smoothed_step_count,
				path->smoothed_steps,
				global_real_argb_green);
		}

		if (ai_debug.render_paths_avoided)
		{
			ai_debug_render_path_line(
				&path->path_state.input.start_point,
				path->avoided_step_count,
				path->avoided_steps,
				global_real_argb_blue);
		}

		if (ai_debug.render_paths_avoidance_segment>=0 && ai_debug.render_paths_avoidance_segment<path->stored_obstacle_step_count)
		{
			if (ai_debug.render_paths_avoidance_obstacles)
			{
				render_debug_obstacles(
					&path->path_obstacles[ai_debug.render_paths_avoidance_segment],
					path->path_obstacle_paths[ai_debug.render_paths_avoidance_segment].radius);
			}

			if (ai_debug.render_paths_avoidance_search && matching_bsp)
			{
				render_debug_path(&path->path_obstacle_paths[ai_debug.render_paths_avoidance_segment]);
			}
		}

		if (ai_debug.render_paths_nodes)
		{
			ai_debug_render_path_nodes(
				&path->path_state,
				matching_bsp,
				ai_debug.render_paths_nodes_all,
				ai_debug.render_paths_nodes_polygons,
				ai_debug.render_paths_nodes_costs,
				ai_debug.render_paths_nodes_closest);
		}

		path->last_render_id = ai_debug.last_render_id;
	}

	return;
}

void ai_debug_speak_list(
	char const *name)
{
	if (ai_debug.selected_actor_index!=NONE)
	{
		struct
		{
			char const *name;
			short vocalization_type;
			boolean skip_unused;
		} lists[] =
		{
			{ "all", _vocalization_idle_noncombat, TRUE },
			{ "idle", _vocalization_idle_noncombat, FALSE },
			{ "involuntary", _vocalization_pain_body, FALSE },
			{ "hurting people", _vocalization_shot_friend, FALSE },
			{ "being hurt", _vocalization_hurt_friend, FALSE },
			{ "killing people", _vocalization_killed_friend, FALSE },
			{ "player kill comments", _vocalization_player_kill_comment, FALSE },
			{ "friends dying", _vocalization_friend_died, FALSE },
			{ "shouting", _vocalization_sighted_enemy_new, FALSE },
			{ "group communication", _vocalization_sighted_enemy_near_reply, FALSE },
			{ "actions", _vocalization_sighted_friend_player, FALSE },
			{ "exclamations", _vocalization_surprise, FALSE },
			{ "post-combat actions", _vocalization_celebrate, FALSE },
			{ "post-combat chatter", _vocalization_postcombat_alone, FALSE },
			{ NULL, NONE, FALSE }
		}, *list;
		struct actor_datum const *actor = actor_get(ai_debug.selected_actor_index);

		for (list = lists; list->name; list++)
		{
			if (!_stricmp(list->name, name))
			{
				break;
			}
		}

		if (!list->name)
		{
			console_printf(FALSE, "ai_speak_list: couldn't find the list '%s'... here are the known lists:", name);
			for (list = lists; list->name; list++)
			{
				console_printf(FALSE, "    %s", list->name);
			}
		}
		else if (actor->meta.unit_index!=NONE && list->vocalization_type!=NONE)
		{
			ai_debug.render_speech = TRUE;
			ai_debug.speak_active = TRUE;
			ai_debug.speak_delay_timer = 0;
			ai_debug.speak_list = TRUE;
			ai_debug.speak_list_skip_unused = list->skip_unused;
			ai_debug.speaking_unit_index = actor->meta.unit_index;
			ai_debug.vocalization_type = list->vocalization_type;
		}
	}

	return;
}

static void ai_debug_speech_update(
	void)
{
	if (ai_debug.speak_active && ai_debug.speaking_unit_index!=NONE)
	{
		struct unit_datum *unit = unit_try_and_get(ai_debug.speaking_unit_index);

		if (unit && !TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			if (unit->unit.speech.current.priority==_unit_speech_none)
			{
				if (ai_debug.speak_delay_timer>0)
				{
					ai_debug.speak_delay_timer--;
				}

				if (ai_debug.speak_delay_timer==0)
				{
					if (ai_debug.vocalization_type>=0 && ai_debug.vocalization_type<NUMBER_OF_VOCALIZATION_TYPES)
					{
						char const *sound_name;
						short vocalization_type = ai_debug.vocalization_type;
						long sound_definition_index = NONE;
						short play_type = unit_test_speech(
							ai_debug.speaking_unit_index,
							_unit_speech_talk,
							FALSE,
							FALSE,
							NULL,
							&vocalization_type,
							&sound_definition_index);

						if (play_type>=_unit_play_speech_immediate)
						{
							struct unit_speech_item speech_item;

							memset(&speech_item, 0, sizeof(speech_item));

							speech_item.vocalization_type = vocalization_type;
							speech_item.sound_definition_index = sound_definition_index;
							speech_item.priority = _unit_speech_communicate;
							speech_item.pause_time = 15;

							ai_communication_packet_new(&speech_item.ai);
							unit_speak(ai_debug.speaking_unit_index, play_type, &speech_item);

						}

						if (play_type>=_unit_play_speech_immediate && sound_definition_index!=NONE)
						{
							char const *conditional;

							sound_name = tag_get_name(sound_definition_index);
							conditional = strstr(sound_name, "conditional");
							if (conditional)
							{
								conditional = strchr(conditional, '\\');
							}
							if (conditional)
							{
								conditional++;
							}
							if (conditional)
							{
								sound_name = conditional;
							}
						}
						else
						{
							sound_name = "<none>";
						}

						console_printf(FALSE, "%s: %s", dialogue_get_vocalization_name(ai_debug.vocalization_type, FALSE), sound_name);

						if (ai_debug.speak_list)
						{
							ai_debug.speak_delay_timer = 15;
							do
							{
								ai_debug.vocalization_type++;
								if (strcmp(dialogue_get_vocalization_name(ai_debug.vocalization_type, FALSE), "unused"))
								{
									break;
								}
								if (!ai_debug.speak_list_skip_unused)
								{
									ai_debug.vocalization_type = NONE;
									break;
								}
							} while (ai_debug.vocalization_type<NUMBER_OF_VOCALIZATION_TYPES);
						}
						else
						{
							ai_debug.vocalization_type = NONE;
						}
					}

					if (ai_debug.vocalization_type<0 || ai_debug.vocalization_type>=NUMBER_OF_VOCALIZATION_TYPES)
					{
						console_printf(FALSE, "speech done");
						ai_debug.speak_active = FALSE;
					}
				}
			}
		}
		else
		{
			ai_debug.speak_active = FALSE;
		}
	}

	return;
}

static void ai_debug_communication_toggle_bits(
	long name_count,
	char const **names,
	unsigned long *flags,
	unsigned long vector_size,
	short (*lookup)(char const *name))
{
	unsigned long vector[BIT_VECTOR_SIZE_IN_LONGS(2048)];
	long index;
	short clear_count = 0;
	short set_count = 0;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4968, lookup);
	match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4969, vector_size <= 2048);

	memset(vector, 0, BIT_VECTOR_SIZE_IN_BYTES(vector_size));

	for (index = 0; index<name_count; index++)
	{
		short comm_type = lookup(names[index]);

		if (comm_type!=NONE)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4978, (comm_type >= 0) && (comm_type < vector_size));
			BIT_VECTOR_SET_FLAG(vector, comm_type, TRUE);
		}
		else if (!strcmp(names[index], "all"))
		{
			memset(vector, -1, BIT_VECTOR_SIZE_IN_BYTES(vector_size));
		}
	}

	for (index = 0; index<vector_size; index++)
	{
		if (BIT_VECTOR_TEST_FLAG(vector, index))
		{
			if (BIT_VECTOR_TEST_FLAG(flags, index))
			{
				clear_count++;
			}
			else
			{
				set_count++;
			}
		}
	}

	if (set_count)
	{
		bit_vector_or(vector_size, vector, flags, flags);
		console_printf(FALSE, "set %d flags", set_count);
	}
	else if (clear_count)
	{
		bit_vector_not(vector_size, vector, vector);
		bit_vector_and(vector_size, vector, flags, flags);
		console_printf(FALSE, "cleared %d flags", clear_count);
	}

	return;
}

void ai_debug_communication_suppress(
	long name_count,
	char const **names)
{
	ai_debug_communication_toggle_bits(
		name_count,
		names,
		ai_debug.communication_suppress_flags,
		NUMBER_OF_AI_COMMUNICATION_TYPES,
		ai_communication_get_type_by_name);

	return;
}

void ai_debug_communication_ignore(
	long name_count,
	char const **names)
{
	ai_debug_communication_toggle_bits(
		name_count,
		names,
		ai_debug.communication_ignore_flags,
		NUMBER_OF_AI_COMMUNICATION_TYPES,
		ai_communication_get_type_by_name);

	return;
}

void ai_debug_communication_focus(
	long name_count,
	char const **names)
{
	ai_debug_communication_toggle_bits(
		name_count,
		names,
		ai_debug.vocalization_focus_flags,
		NUMBER_OF_VOCALIZATION_TYPES,
		dialogue_get_vocalization_type_by_name);

	return;
}

void ai_debug_idle_look_clear(
	long actor_index)
{
	ai_debug.idle_look_valid = actor_index!=NONE;
	ai_debug.prop_idle_actor_index = actor_index;
	ai_debug.prop_idle_look_count = 0;

	return;
}

void ai_debug_idle_look_addprop(
	long prop_index,
	real distance)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 5062, ai_debug.idle_look_valid);

	if (ai_debug.prop_idle_look_count<MAXIMUM_IDLE_LOOK_PROPS)
	{
		ai_debug.prop_idle_look_indicies[ai_debug.prop_idle_look_count] = prop_index;
		ai_debug.prop_idle_look_distances[ai_debug.prop_idle_look_count] = distance;
		ai_debug.prop_idle_look_count++;
	}

	return;
}

static void ai_debug_render_idle_look(
	void)
{
	if (ai_debug.idle_look_valid)
	{
		struct actor_datum *actor = actor_try_and_get(ai_debug.prop_idle_actor_index);

		if (actor)
		{
			short index;

			for (index = 0; index<ai_debug.prop_idle_look_count; index++)
			{
				struct prop_datum *prop = prop_try_and_get(ai_debug.prop_idle_look_indicies[index]);

				if (prop)
				{
					real_point3d point;
					real_argb_color const *color;

					point_from_line3d(&actor->input.position.head_position, global_up3d, 0.05f, &point);
					point_from_line3d(&point, &prop->actor_to_prop, 0.9f, &point);

					if (actor->control.idle_major_active &&
						actor->control.idle_major_direction.type==_direction_specification_prop &&
						actor->control.idle_major_direction.prop_index==ai_debug.prop_idle_look_indicies[index])
					{
						color = global_real_argb_yellow;
					}
					else
					{
						color = global_real_argb_white;
					}

					render_debug_string_at_point(
						TRUE,
						&point,
						csprintf(temporary, "%.2f", ai_debug.prop_idle_look_distances[index]),
						color);
				}
			}
		}
	}

	return;
}

static void ai_debug_render_spatial_effects(
	void)
{
	short effect_index;
	long current_time = game_time_get();

	for (effect_index = ai_globals->spatial_effects_first_index;
		effect_index!=ai_globals->spatial_effects_last_index;
		effect_index = (effect_index + 1) % NUMBEROF(ai_globals->spatial_effects))
	{
		struct ai_spatial_effect *spatial_effect = &ai_globals->spatial_effects[effect_index];

		if (spatial_effect->type!=NONE)
		{
			real_argb_color const *const *colors[NUMBER_OF_AI_SPATIAL_EFFECTS] = { &global_real_argb_blue, &global_real_argb_yellow, &global_real_argb_red };
			real_argb_color const *color = global_real_argb_white;

			if (spatial_effect->type>=0 && spatial_effect->type<NUMBER_OF_AI_SPATIAL_EFFECTS)
			{
				color = *colors[spatial_effect->type];
			}

			render_debug_sphere(TRUE, &spatial_effect->position, 0.2f, color);

			{
				real_point3d string_point;

				point_from_line3d(&spatial_effect->position, global_up3d, 0.3f, &string_point);
				render_debug_string_at_point(
					TRUE,
					&string_point,
					csprintf(temporary, "c%d t%d", spatial_effect->count, current_time - spatial_effect->last_tick),
					color);
			}
		}
	}

	return;
}

static void ai_debug_render_speech(
	void)
{
	struct object_iterator iterator;
	struct unit_datum *unit;

	object_iterator_new(&iterator, _object_mask_unit, 0);
	while (unit = (struct unit_datum *)object_iterator_next(&iterator))
	{
		struct unit_speech *unit_speech = &unit->unit.speech;

		{
			real_point3d stack_base;
			real_point3d head_position;

			unit_get_head_position(iterator.index, &head_position);
			point_from_line3d(&head_position, global_up3d, 0.1f, &stack_base);
			ai_debug_drawstack_setup(&stack_base);
		}

		if (ai_debug.render_dialogue_variants)
		{
			struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

			if (unit_definition->unit.dialogue_variants.count>0)
			{
				short variant_index;
				char const *dialogue_name = "<none>";
				short variant_number = NONE;

				for (variant_index = 0; variant_index<unit_definition->unit.dialogue_variants.count; variant_index++)
				{
					struct dialogue_variant_definition *variant = TAG_BLOCK_GET_ELEMENT(
						&unit_definition->unit.dialogue_variants,
						variant_index,
						struct dialogue_variant_definition);

					if (variant->dialogue_variant.index==unit->unit.dialogue_index)
					{
						variant_number = variant->variant_number;
						break;
					}
				}

				if (unit->unit.dialogue_index!=NONE)
				{
					dialogue_name = tag_name_strip_path(tag_get_name(unit->unit.dialogue_index));
				}

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(
						temporary,
						"variant %d dialogue %d %s",
						unit->object.variant_number,
						variant_number,
						dialogue_name),
					global_real_argb_pink);
			}
		}

		if (ai_debug.render_speech)
		{
			if (unit_speech->current.priority>_unit_speech_none)
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(
						temporary,
						"%s %s %s (%d %d)",
						unit_get_speech_priority_name(unit_speech->current.priority),
						unit_speech->current.vocalization_type==NONE ? "NONE" : dialogue_get_vocalization_name(unit_speech->current.vocalization_type, FALSE),
						unit_speech->current.sound_definition_index==NONE ? "NONE" : tag_name_strip_path(tag_get_name(unit_speech->current.sound_definition_index)),
						unit_speech->sound_timer,
						unit_speech->post_delay_timer),
					global_real_argb_white);
			}

			if (unit_speech->queued.priority>_unit_speech_none)
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(
						temporary,
						"%s %s %s",
						unit_get_speech_priority_name(unit_speech->queued.priority),
						unit_speech->queued.vocalization_type==NONE ? "NONE" : dialogue_get_vocalization_name(unit_speech->queued.vocalization_type, FALSE),
						unit_speech->queued.sound_definition_index==NONE ? "NONE" : tag_name_strip_path(tag_get_name(unit_speech->queued.sound_definition_index))),
					global_real_argb_yellow);
			}
		}

		if (ai_debug.print_speech && !ai_debug.render_speech && unit_speech->current.priority>_unit_speech_none)
		{
			char speechbuf[512];
			real_argb_color const *color;

			switch (unit_speech->current.priority)
			{
			case _unit_speech_pain:
			case _unit_speech_involuntary:
			case _unit_speech_death:
				color = global_real_argb_red;
				break;
			case _unit_speech_scripted:
				color = global_real_argb_blue;
				break;
			default:
				color = global_real_argb_white;
				break;
			}

			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				unit_describe_speech(iterator.index, FALSE, sizeof(speechbuf), speechbuf),
				color);
		}
	}

	return;
}

static void ai_debug_path_storage_update(
	void)
{
	short path_index;

	for (path_index = 0; path_index<MAXIMUM_NUMBER_OF_ACTOR_PATHS; ++path_index)
	{
		struct path_debug_storage *path = &actor_path_debug_array[path_index];

		if (path->valid && path->failure)
		{
			short other_path_index;

			for (other_path_index = path_index + 1; other_path_index<MAXIMUM_NUMBER_OF_ACTOR_PATHS; ++other_path_index)
			{
				struct path_debug_storage *other_path = &actor_path_debug_array[other_path_index];

				if (other_path->valid &&
					other_path->failure &&
					other_path->actor_index==path->actor_index &&
					distance_squared3d(&path->path_state.input.start_point, &other_path->path_state.input.start_point)<0.25f &&
					path->path_state.destination_valid==other_path->path_state.destination_valid &&
					(!path->path_state.destination_valid ||
						distance_squared3d(&path->path_state.destination.point, &other_path->path_state.destination.point)<0.25f))
				{
					if (path->path_time<other_path->path_time)
					{
						path->valid = FALSE;
						break;
					}
					else
					{
						other_path->valid = FALSE;
					}
				}
			}
		}
	}

	return;
}

static void ai_debug_render_paths_failed(
	void)
{
	short path_index;

	for (path_index = 0; path_index<MAXIMUM_NUMBER_OF_ACTOR_PATHS; ++path_index)
	{
		struct path_debug_storage *path = &actor_path_debug_array[path_index];

		if (path->valid && path->failure)
		{
			real_point3d drawstack_base;
			char aibuf[256];

			point_from_line3d(&path->path_state.input.start_point, global_up3d, 1.f, &drawstack_base);
			ai_debug_drawstack_setup(&drawstack_base);
			ai_debug_describe_actor(path->actor_index, NONE, TRUE, aibuf, sizeof(aibuf));
			render_debug_string_at_point(TRUE, ai_debug_drawstack(), aibuf, global_real_argb_red);
			ai_debug_render_path_storage(path);
		}
	}

	return;
}

static void ai_debug_render_path(
	void)
{
	if (ai_debug.path_start_valid && ai_debug.path_destination_valid && !ai_debug.path.valid)
	{
		real_argb_color const *color;

		if (!ai_debug.path_flood && !ai_debug.path_state.destination_valid)
		{
			color = global_real_argb_blue;
		}
		else if (ai_debug.path_state.node_count==0)
		{
			color = global_real_argb_green;
		}
		else if (ai_debug.path_state.node_count>=PATH_NODE_LIST_SIZE)
		{
			color = global_real_argb_yellow;
		}
		else
		{
			color = global_real_argb_pink;
		}

		render_debug_line(TRUE, &ai_debug.path_start_point, &ai_debug.path_destination.point, color);
	}

	if (ai_debug.path_debug.valid)
	{
		ai_debug_drawstack_setup(&ai_debug.path_debug.path_state.input.start_point);
		ai_debug_render_path_storage(&ai_debug.path_debug);
	}

	return;
}

static void ai_debug_render_lineoffire(
	void)
{
	if (ai_debug.lineoffire_valid)
	{
		real_point3d endpt;
		long itr;

		point_from_line3d(&ai_debug.lineoffire_origin, &ai_debug.lineoffire_vector, 1.0f, &endpt);
		render_debug_line(TRUE, &ai_debug.lineoffire_origin, &endpt, ai_debug.lineoffire_success ? global_real_argb_green : global_real_argb_red);

		for (itr = 0; itr<ai_debug.lineoffire_numpills; itr++)
		{
			render_debug_pill(
				TRUE,
				&ai_debug.lineoffire_pillbase[itr],
				&ai_debug.lineoffire_pilldirectedheight[itr],
				ai_debug.lineoffire_pillwidth[itr],
				ai_debug.lineoffire_pillhit[itr] ? global_real_argb_red : global_real_argb_blue);
		}
	}

	return;
}

static void ai_debug_render_ballistic_lineoffire(
	void)
{
	if (ai_debug.ballistic_valid)
	{
		short itr;

		render_debug_point(TRUE, &ai_debug.ballistic_origin, 0.1f, global_real_argb_yellow);
		render_debug_vector(TRUE, &ai_debug.ballistic_origin, &ai_debug.ballistic_initial_velocity, 1.0f, global_real_argb_yellow);

		for (itr = 0; itr<ai_debug.ballistic_numpills; itr++)
		{
			render_debug_pill(
				TRUE,
				&ai_debug.ballistic_pillbase[itr],
				&ai_debug.ballistic_pilldirectedheight[itr],
				ai_debug.ballistic_pillwidth[itr],
				global_real_argb_blue);
		}

		for (itr = 0; itr<ai_debug.ballistic_numpoints-1; itr++)
		{
			real_argb_color const *color;

			if (ai_debug.ballistic_success)
			{
				color = global_real_argb_green;
			}
			else if (itr==ai_debug.ballistic_numpoints-2)
			{
				color = global_real_argb_orange;
			}
			else
			{
				color = global_real_argb_red;
			}

			render_debug_line(TRUE, &ai_debug.ballistic_points[itr], &ai_debug.ballistic_points[itr+1], color);
		}
	}

	return;
}

static short ai_debug_lineofsight_findpoint(
	real_point3d const *point,
	short cluster_index)
{
	long index;

	for (index = 0; index<ai_debug.lineofsight_numpoints; index++)
	{
		if (ai_debug.lineofsight_pointclusters[index]==cluster_index &&
			distance_squared3d(point, &ai_debug.lineofsight_points[index])<0.001f*0.001f)
		{
			break;
		}
	}

	if (index>=ai_debug.lineofsight_numpoints)
	{
		if (ai_debug.lineofsight_numpoints<MAXIMUM_LINEOFSIGHT_POINTS)
		{
			index = ai_debug.lineofsight_numpoints++;
			ai_debug.lineofsight_points[index] = *point;
			ai_debug.lineofsight_pointcounts[index] = 0;
			ai_debug.lineofsight_pointclusters[index] = cluster_index;
		}
		else
		{
			index = NONE;
			if (!ai_debug.lineofsight_overflow)
			{
				error(_error_silent, "ai_debug_lineofsight: overflowed point buffer (%d) with %d rays and counting", MAXIMUM_LINEOFSIGHT_POINTS, ai_debug.lineofsight_numrays);
				ai_debug.lineofsight_overflow = TRUE;
			}
		}
	}

	if (index!=NONE)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4223, index <= SHORT_MAX);
		ai_debug.lineofsight_pointcounts[index]++;
	}

	return (short)index;
}

static long ai_debug_lineofsight_storeray(
	short p0_index,
	short p1_index)
{
	long index = NONE;

	if (p0_index!=NONE && p1_index!=NONE)
	{
		for (index = 0; index<ai_debug.lineofsight_numrays; index++)
		{
			if (ai_debug.lineofsight_rays[index][0]==p0_index &&
				ai_debug.lineofsight_rays[index][1]==p1_index)
			{
				break;
			}
		}

		if (index>=ai_debug.lineofsight_numrays)
		{
			if (ai_debug.lineofsight_numrays<MAXIMUM_LINEOFSIGHT_RAYS)
			{
				index = ai_debug.lineofsight_numrays++;
				ai_debug.lineofsight_rays[index][0] = p0_index;
				ai_debug.lineofsight_rays[index][1] = p1_index;
				ai_debug.lineofsight_rays[index][2] = 0;
			}
			else
			{
				index = NONE;
				if (!ai_debug.lineofsight_overflow)
				{
					error(_error_silent, "ai_debug_lineofsight: overflowed ray buffer (%d) with %d points and counting", MAXIMUM_LINEOFSIGHT_RAYS, ai_debug.lineofsight_numpoints);
					ai_debug.lineofsight_overflow = TRUE;
				}
			}
		}

		if (index!=NONE)
		{
			ai_debug.lineofsight_rays[index][2]++;
		}
	}

	return index;
}

void ai_debug_lineofsight(
	real_point3d const *p0,
	short p0_cluster_index,
	real_point3d const *p1,
	short p1_cluster_index)
{
	ai_debug_lineofsight_storeray(
		ai_debug_lineofsight_findpoint(p0, p0_cluster_index),
		ai_debug_lineofsight_findpoint(p1, p1_cluster_index));
	return;
}

static void ai_debug_render_lineofsight(
	void)
{
	long index;

	real_argb_color const *const *lineofsight_colors[13] =
	{
		&global_real_argb_black,
		&global_real_argb_blue,
		&global_real_argb_lightblue,
		&global_real_argb_cyan,
		&global_real_argb_green,
		&global_real_argb_purple,
		&global_real_argb_salmon,
		&global_real_argb_pink,
		&global_real_argb_magenta,
		&global_real_argb_red,
		&global_real_argb_orange,
		&global_real_argb_yellow,
		&global_real_argb_white
	};
	long const max_lineofsight_colors = NUMBEROF(lineofsight_colors);

	for (index = 0; index<ai_debug.lineofsight_numpoints; index++)
	{
		sprintf(temporary, "%d", ai_debug.lineofsight_pointcounts[index]);
		render_debug_string_at_point(
			TRUE,
			&ai_debug.lineofsight_points[index],
			temporary,
			*lineofsight_colors[MIN(ai_debug.lineofsight_pointcounts[index], max_lineofsight_colors-1)]);
	}

	for (index = 0; index<ai_debug.lineofsight_numrays; index++)
	{
		render_debug_line(
			TRUE,
			&ai_debug.lineofsight_points[ai_debug.lineofsight_rays[index][0]],
			&ai_debug.lineofsight_points[ai_debug.lineofsight_rays[index][1]],
			*lineofsight_colors[MIN(ai_debug.lineofsight_rays[index][2], max_lineofsight_colors-1)]);
	}

	return;
}

void ai_debug_initialize_for_new_map(
	void)
{
	long encounter_index = encounter_get_by_name(ai_debug.selected_squad_name);

	ai_debug_clear_storage();
	ai_debug_select_actor(encounter_index, NONE);

	return;
}

void ai_debug_update(
	void)
{
	if (ai_debug.render_lineofsight)
	{
		ai_debug_lineofsight_reset();
	}

	if (ai_debug.path_enable)
	{
		if (!ai_debug.path_start_freeze)
		{
			long unit_index = player_control_get_unit_index(0);

			if (unit_index!=NONE && biped_try_and_get(unit_index))
			{
				real_point3d start_point;
				long start_surface_index = biped_find_pathfinding_surface_index(unit_index, &start_point);

				if (start_surface_index!=NONE)
				{
					ai_debug.path_start_surface_index = start_surface_index;
					ai_debug.path_start_point = start_point;
					ai_debug.path_start_unit_index = unit_index;
					ai_debug.path_start_valid = TRUE;
				}
			}
		}

		if (!ai_debug.path_end_freeze)
		{
			struct observer_result const *camera = observer_get_camera(0);

			if (camera)
			{
				real_vector3d collision_vector;
				struct collision_result collision;

				ai_profile.meters[_ai_meter_collision_vector].current_count++;
				scale_vector3d(global_down3d, 1000.f, &collision_vector);
				if (collision_test_vector(
					FLAG(_collision_test_front_facing_surfaces_bit)|FLAG(_collision_test_structure_bit),
					&camera->position,
					&collision_vector,
					NONE,
					&collision))
				{
					ai_debug.path_destination_valid = TRUE;
					ai_debug.path_destination.point = collision.point;
					ai_debug.path_destination.surface_index = collision.surface_index;
					ai_debug.path_destination.target_radius = 0.f;
				}
			}
		}

		if (ai_debug.path_start_valid)
		{
			struct path_input input;

			path_input_new(&input, 0.2f, FALSE, ai_debug.path_start_unit_index);
			path_input_set_start(&input, &ai_debug.path_start_point, ai_debug.path_start_surface_index);

			if (ai_debug.path_maximum_radius>0.f)
			{
				path_input_set_search_bounds(&input, ai_debug.path_maximum_radius);
			}

			if (ai_debug.path_attractor)
			{
				long unit_index = player_control_get_unit_index(0);

				if (unit_index!=NONE)
				{
					real_point3d origin;

					unit_get(unit_index);
					object_get_origin(unit_index, &origin);
					path_input_set_attractor(
						&input,
						&origin,
						ai_debug.path_attractor_radius==0.f ? 8.f : ai_debug.path_attractor_radius,
						NONE,
						ai_debug.path_attractor_weight==0.f ? 20.f : ai_debug.path_attractor_weight);
				}
			}

			path_state_new(&input, &ai_debug.path_state, &ai_debug.path_debug);

			if (ai_debug.path_destination_valid && !ai_debug.path_flood)
			{
				path_state_destination(
					&ai_debug.path_state,
					&ai_debug.path_destination.point,
					ai_debug.path_destination.surface_index,
					ai_debug.path_accept_radius);
			}

			path_state_find(&ai_debug.path_state);

			if (ai_debug.path_destination_valid && ai_debug.path_flood)
			{
				path_state_destination(
					&ai_debug.path_state,
					&ai_debug.path_destination.point,
					ai_debug.path_destination.surface_index,
					ai_debug.path_accept_radius);
			}

			path_state_build_path(&ai_debug.path_state, &ai_debug.path);

			ai_debug.path_state_valid = TRUE;
			ai_debug.path_debug.valid = TRUE;
			ai_debug.path_debug.path_time = game_time_get();
			ai_debug.path_debug.actor_index = NONE;
		}
	}

	if (ai_debug.fix_defending_guard_firing_positions && game_in_editor())
	{
		short encounter_index;

		struct scenario *scenario = global_scenario_get();
		long squad_count = 0;

		for (encounter_index = 0; encounter_index<scenario->ai_encounters.count; ++encounter_index)
		{
			short squad_index;

			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&scenario->ai_encounters,
				encounter_index,
				struct encounter_definition);

			for (squad_index = 0; squad_index<encounter_definition->squads.count; ++squad_index)
			{
				short group_index;

				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					squad_index,
					struct squad_definition);

				for (group_index = _firing_position_group_pursuing; group_index>_firing_position_group_attacking_guard; --group_index)
				{
					squad_definition->firing_position_groups[group_index] = squad_definition->firing_position_groups[group_index-1];
				}

				squad_definition->firing_position_groups[_firing_position_group_attacking_guard] =
					squad_definition->firing_position_groups[_firing_position_group_attacking];
				++squad_count;
			}
		}

		console_printf(FALSE, "updated all %d squads' guard positions. glory!", squad_count);
		ai_debug.fix_defending_guard_firing_positions = FALSE;
	}

	if (ai_debug.fix_actor_variants && game_in_editor())
	{
		short encounter_index;

		struct scenario *scenario = global_scenario_get();
		long starting_location_count = 0;

		for (encounter_index = 0; encounter_index<scenario->ai_encounters.count; ++encounter_index)
		{
			short squad_index;

			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&scenario->ai_encounters,
				encounter_index,
				struct encounter_definition);

			for (squad_index = 0; squad_index<encounter_definition->squads.count; ++squad_index)
			{
				short starting_location_index;

				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					squad_index,
					struct squad_definition);

				for (starting_location_index = 0; starting_location_index<squad_definition->starting_locations.count; ++starting_location_index)
				{
					struct actor_starting_location_definition *starting_location = TAG_BLOCK_GET_ELEMENT(
						&squad_definition->starting_locations,
						starting_location_index,
						struct actor_starting_location_definition);

					starting_location->actor_palette_index = NONE;
					++starting_location_count;
				}
			}
		}

		console_printf(FALSE, "reset the actor variant in all %d starting locations. glory!", starting_location_count);
		ai_debug.fix_actor_variants = FALSE;
	}

	ai_debug_speech_update();
	ai_debug_path_storage_update();

	return;
}

void ai_debug_change_selected_encounter(
	boolean search_forwards)
{
	long encounter_index = search_forwards ?
		data_next_index(encounter_data, ai_debug.selected_encounter_index) :
		data_prev_index(encounter_data, ai_debug.selected_encounter_index);
	struct encounter_datum const *encounter = encounter_try_and_get(encounter_index);

	if (encounter==NULL)
	{
		console_printf(FALSE, "no more encounters");
		ai_debug_select_encounter(NONE);
	}
	else
	{
		char bsp_string[256];
		char bsp_index_string[256];

		struct encounter_definition const *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
			struct encounter_definition);

		if (TEST_FLAG(encounter_definition->flags, _encounter_3d_firing_positions_bit))
		{
			csstrcpy(bsp_string, "3d-positions");
		}
		else
		{
			if (encounter_definition->runtime_structure_bsp_reference_index==NONE)
			{
				csstrcpy(bsp_index_string, "NONE");
			}
			else
			{
				sprintf(bsp_index_string, "%d", encounter_definition->runtime_structure_bsp_reference_index);
			}

			sprintf(
				bsp_string,
				"%s-bsp %s",
				TEST_FLAG(encounter_definition->flags, _encounter_manual_structure_bsp_bit) ? "manual" : "auto",
				bsp_index_string);
		}

		console_printf(
			FALSE,
			"encounter %s [%s %s] (%d actors)",
			encounter_definition->name,
			encounter->active ? "active" : "inactive",
			bsp_string,
			encounter->current_count);
		ai_debug_select_encounter(encounter_index);
	}

	return;
}

void ai_debug_change_selected_actor(
	boolean search_forwards)
{
	struct encounter_datum const *encounter = encounter_try_and_get(ai_debug.selected_encounter_index);

	if (encounter==NULL)
	{
		console_printf(FALSE, "no encounter selected (use F2/F3)");
		ai_debug_select_actor(NONE, NONE);
	}
	else
	{
		struct encounter_actor_iterator iterator;
		struct actor_datum const *actor;

		short index_in_encounter = 0;

		encounter_actor_iterator_new(&iterator, ai_debug.selected_encounter_index);

		if (ai_debug.selected_actor_index!=NONE)
		{
			while (encounter_actor_iterator_next(&iterator) && iterator.index!=ai_debug.selected_actor_index)
			{
				++index_in_encounter;
			}
		}

		if (search_forwards)
		{
			actor = encounter_actor_iterator_next(&iterator);
			++index_in_encounter;
		}
		else
		{
			actor = encounter_actor_iterator_prev(&iterator);
			--index_in_encounter;
		}

		if (actor)
		{
			ai_debug_describe_actor(iterator.index, NONE, TRUE, temporary, NUMBEROF(temporary));
			console_printf(FALSE, "actor %d/%d: %s", index_in_encounter + 1, encounter->current_count, temporary);
			ai_debug_select_actor(ai_debug.selected_encounter_index, iterator.index);
		}
		else
		{
			console_printf(FALSE, "no more actors");
			ai_debug_select_actor(ai_debug.selected_encounter_index, NONE);
		}
	}

	return;
}

void ai_debug_teleport_to(
	long encounter_index)
{
	if (encounter_index!=NONE)
	{
		struct encounter_definition const *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
			struct encounter_definition);

		if (encounter_definition->player_starting_locations.count>0)
		{
			struct data_iterator iterator;
			struct player_datum const *player;

			short player_count = 0;

			data_iterator_new(&iterator, player_data);
			while (player = (struct player_datum const *)data_iterator_next(&iterator))
			{
				if (player->unit_index!=NONE)
				{
					real_vector3d forward;

					struct scenario_player const *starting_location = TAG_BLOCK_GET_ELEMENT(
						&encounter_definition->player_starting_locations,
						player_count % encounter_definition->player_starting_locations.count,
						struct scenario_player);

					forward.i = cos(starting_location->facing);
					forward.j = sin(starting_location->facing);
					forward.k = 0.f;

					object_set_position(player->unit_index, &starting_location->position, &forward, NULL);
					++player_count;
				}
			}
		}
	}

	return;
}

static long ai_debug_get_this_actor(
	void)
{
	short local_player_index;

	long actor_index = NONE;
	short user_index = NONE;

	for (local_player_index = 0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; ++local_player_index)
	{
		if (local_player_exists(local_player_index))
		{
			user_index = local_player_index;
			break;
		}
	}

	if (user_index!=NONE)
	{
		struct collision_result collision;
		long object_index;
		real_vector3d v;
		long ignore_object_index = NONE;
		struct observer_result const *camera = observer_get_camera(user_index);

		match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4489, camera != NULL);

		if (director_get_perspective(user_index)==_director_perspective_first_person)
		{
			long player_index = local_player_get_player_index(user_index);

			if (player_index!=NONE)
			{
				ignore_object_index = player_get(player_index)->unit_index;
			}
		}

		scale_vector3d(&camera->forward, 50.f, &v);
		if (collision_test_vector(
			FLAG(_collision_test_front_facing_surfaces_bit)|FLAG(_collision_test_objects_bit),
			&camera->position,
			&v,
			ignore_object_index,
			&collision) &&
			collision.type==_collision_result_object)
		{
			object_index = collision.object_index;

			if (object_index!=NONE)
			{
				struct unit_datum const *unit = unit_try_and_get(object_index);

				if (unit)
				{
					actor_index = unit->unit.swarm_actor_index!=NONE ? unit->unit.swarm_actor_index : unit->unit.actor_index;

					if (actor_index==NONE && unit->unit.driver_object_index!=NONE)
					{
						struct unit_datum const *driver_unit = unit_get(unit->unit.driver_object_index);

						actor_index = driver_unit->unit.swarm_actor_index!=NONE ? driver_unit->unit.swarm_actor_index : driver_unit->unit.actor_index;
					}
				}
			}
		}
	}

	return actor_index;
}

static void ai_debug_select_this_actor(
	void)
{
	long actor_index = ai_debug_get_this_actor();

	if (actor_index!=NONE)
	{
		struct actor_datum const *actor = actor_get(actor_index);

		ai_debug_describe_actor(actor_index, NONE, TRUE, temporary, NUMBEROF(temporary));
		console_printf(FALSE, "selected %s", temporary);
		ai_debug_select_actor(actor->meta.encounter_index, actor_index);
	}
	else
	{
		ai_debug_select_actor(NONE, NONE);
	}

	ai_debug.select_this_actor = FALSE;

	return;
}

static void ai_debug_render_aiming_validity(
	void)
{
	if (ai_debug.aiming_validity_actor_index!=ai_debug.selected_actor_index)
	{
		ai_debug.aiming_validity_actor_index = ai_debug.selected_actor_index;
		ai_debug.aiming_validity_aiming_stored = FALSE;
		ai_debug.aiming_validity_looking_stored = FALSE;
	}

	if (ai_debug.aiming_validity_actor_index!=NONE)
	{
		struct actor_datum const *actor = actor_get(ai_debug.aiming_validity_actor_index);
		struct observer_result const *camera = observer_get_camera(0);

		if (camera)
		{
			real_vector3d test_vector;
			boolean valid_aiming;
			boolean valid_looking;

			vector_from_points3d(&actor->input.position.head_position, &camera->position, &test_vector);
			if (normalize3d(&test_vector)>0.f)
			{
				actor_looking_test_validity(ai_debug.aiming_validity_actor_index, &test_vector, &valid_aiming, &valid_looking);

				if (valid_aiming)
				{
					ai_debug.aiming_validity_aiming_stored = TRUE;
					ai_debug.aiming_validity_aiming_stored_vector = test_vector;
				}

				if (valid_looking)
				{
					ai_debug.aiming_validity_looking_stored = TRUE;
					ai_debug.aiming_validity_looking_stored_vector = test_vector;
				}

				render_debug_vector(TRUE, &actor->input.position.head_position, &test_vector, 1.f, global_real_argb_white);
				render_debug_vector(TRUE, &actor->input.position.head_position, &actor->input.facing_vector, 1.f, global_real_argb_red);
			}
		}

		if (ai_debug.aiming_validity_aiming_stored)
		{
			real_point3d point;

			point_from_line3d(&actor->input.position.head_position, global_up3d, 0.05f, &point);
			render_debug_vector(TRUE, &point, &ai_debug.aiming_validity_aiming_stored_vector, 1.f, global_real_argb_green);
		}

		if (ai_debug.aiming_validity_looking_stored)
		{
			real_point3d point;

			point_from_line3d(&actor->input.position.head_position, global_up3d, 0.05f, &point);
			render_debug_vector(TRUE, &point, &ai_debug.aiming_validity_looking_stored_vector, 1.f, global_real_argb_blue);
		}
	}

	return;
}

static void ai_debug_render_all_actors(
	boolean render_inactive)
{
	struct actor_iterator iterator;

	actor_iterator_new(&iterator, !render_inactive);
	while (actor_iterator_next(&iterator))
	{
		ai_debug_render_actor(iterator.index, iterator.index==ai_debug.selected_actor_index, NULL);
	}

	return;
}

static void ai_debug_render_encounter(
	long encounter_index)
{
	long firing_position_owner_actor_indices[MAXIMUM_FIRING_POSITIONS_PER_ENCOUNTER];
	short firing_position_index;
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition const *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	long history_start_time = NONE;

	{
		struct encounter_actor_iterator iterator;

		encounter_actor_iterator_new(&iterator, encounter_index);
		while (encounter_actor_iterator_next(&iterator))
		{
			boolean exclusive = ai_debug.selected_actor_index==iterator.index;

			if (exclusive || ai_debug.selected_actor_index==NONE || ai_debug.render_all_actors)
			{
				ai_debug_render_actor(iterator.index, exclusive, &history_start_time);
			}
		}
	}

	if (global_ai_debug_firing_position_color_count==NONE)
	{
		global_ai_debug_firing_position_color_count = 0;
		while (global_ai_debug_firing_position_colors[global_ai_debug_firing_position_color_count].alpha<=1.f &&
			global_ai_debug_firing_position_colors[global_ai_debug_firing_position_color_count].red<=1.f &&
			global_ai_debug_firing_position_colors[global_ai_debug_firing_position_color_count].green<=1.f &&
			global_ai_debug_firing_position_colors[global_ai_debug_firing_position_color_count].blue<=1.f)
		{
			global_ai_debug_firing_position_color_count++;
		}
	}

	encounter_build_firing_position_owner_actor_indices(encounter_index, firing_position_owner_actor_indices);

	for (firing_position_index = 0; firing_position_index<encounter_definition->firing_positions.count; firing_position_index++)
	{
		real_point3d points[4];
		boolean firing_position_crosses[MAXIMUM_NUMBER_OF_FIRING_POSITION_GROUPS];
		real_point3d position;
		real_argb_color const *firing_position_colors[MAXIMUM_NUMBER_OF_FIRING_POSITION_GROUPS];
		long color_index;
		boolean selected_actor_in_encounter;
		struct firing_position_definition const *firing_position = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->firing_positions,
			firing_position_index,
			struct firing_position_definition);
		long num_firing_position_colors = 0;

		points[3].z = firing_position->position.z + 0.05f;
		points[2].z = points[3].z;
		points[1].z = points[2].z;
		points[0].z = points[1].z;
		points[3].x = firing_position->position.x - 0.25f;
		points[0].x = points[3].x;
		points[2].x = firing_position->position.x + 0.25f;
		points[1].x = points[2].x;
		points[1].y = firing_position->position.y - 0.25f;
		points[0].y = points[1].y;
		points[3].y = firing_position->position.y + 0.25f;
		points[2].y = points[3].y;

		memset(firing_position_crosses, 0, sizeof(firing_position_crosses));

		selected_actor_in_encounter =
			ai_debug.selected_actor_index!=NONE &&
			actor_get(ai_debug.selected_actor_index)->meta.encounter_index==ai_debug.selected_encounter_index;

		if (!selected_actor_in_encounter)
		{
			if (!game_in_editor())
			{
				num_firing_position_colors = 1;
				firing_position_colors[0] = &global_ai_debug_firing_position_colors[firing_position->group_index % global_ai_debug_firing_position_color_count];
			}
		}
		else
		{
			short primary_group;
			short secondary_group;
			short attacking_group;
			short search_group;

			real_argb_color const *const *group_colors[NUMBER_OF_FIRING_POSITION_GROUPS] =
			{
				&global_real_argb_red,
				&global_real_argb_orange,
				&global_real_argb_green,
				&global_real_argb_blue,
				&global_real_argb_lightblue,
				&global_real_argb_green,
				&global_real_argb_aqua
			};
			struct actor_datum *actor = actor_get(ai_debug.selected_actor_index);
			struct encounter_definition const *actor_encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
				struct encounter_definition);
			struct squad_definition const *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&actor_encounter_definition->squads,
				actor->meta.squad_index,
				struct squad_definition);
			short guard_group = actor->emotions.currently_defending ? _firing_position_group_defending_guard : _firing_position_group_attacking_guard;

			if (squad_definition->firing_position_groups[guard_group] & FLAG(firing_position->group_index))
			{
				firing_position_colors[num_firing_position_colors++] = global_real_argb_green;
			}

			attacking_group = actor->emotions.currently_defending ? _firing_position_group_defending : _firing_position_group_attacking;
			search_group = actor->emotions.currently_defending ? _firing_position_group_defending_search : _firing_position_group_attacking_search;
			if (actor->state.searching)
			{
				primary_group = search_group;
				secondary_group = attacking_group;
			}
			else
			{
				primary_group = attacking_group;
				secondary_group = search_group;
			}

			if (squad_definition->firing_position_groups[primary_group] & FLAG(firing_position->group_index))
			{
				firing_position_colors[num_firing_position_colors++] = *group_colors[primary_group];
			}
			else if (squad_definition->firing_position_groups[secondary_group] & FLAG(firing_position->group_index))
			{
				firing_position_crosses[num_firing_position_colors] = TRUE;
				firing_position_colors[num_firing_position_colors++] = *group_colors[secondary_group];
			}

			if (squad_definition->firing_position_groups[_firing_position_group_pursuing] & FLAG(firing_position->group_index))
			{
				firing_position_colors[num_firing_position_colors++] = global_real_argb_aqua;
			}

			if (num_firing_position_colors==0)
			{
				firing_position_colors[num_firing_position_colors++] = global_real_argb_white;
			}
			else
			{
				match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 964, num_firing_position_colors < MAXIMUM_NUMBER_OF_FIRING_POSITION_GROUPS);
			}
		}

		if (firing_position_owner_actor_indices[firing_position_index]!=NONE)
		{
			real_point3d owner_points[4];
			struct actor_datum *owner_actor = actor_get(firing_position_owner_actor_indices[firing_position_index]);

			owner_points[3].z = firing_position->position.z + 0.05f;
			owner_points[2].z = owner_points[3].z;
			owner_points[1].z = owner_points[2].z;
			owner_points[0].z = owner_points[1].z;
			owner_points[3].x = firing_position->position.x - 0.375f;
			owner_points[0].x = owner_points[3].x;
			owner_points[2].x = firing_position->position.x + 0.375f;
			owner_points[1].x = owner_points[2].x;
			owner_points[1].y = firing_position->position.y - 0.375f;
			owner_points[0].y = owner_points[1].y;
			owner_points[3].y = firing_position->position.y + 0.375f;
			owner_points[2].y = owner_points[3].y;

			render_debug_polygon(owner_points, 4, actor_action_debug_color(firing_position_owner_actor_indices[firing_position_index]));
		}

		if (ai_debug.render_firing_positions)
		{
			real_argb_color const *color = firing_position->surface_index==NONE ? global_real_argb_red : global_real_argb_white;
			real_point3d point1 = firing_position->position;
			real_point3d point0 = firing_position->position;

			point0.z += 0.5f;
			point1.z -= 0.5f;
			render_debug_line(TRUE, &point0, &point1, color);

			point0.z -= 0.5f;
			point1.z += 0.5f;
			point0.x -= 0.1f;
			point1.x += 0.1f;
			render_debug_line(TRUE, &point0, &point1, color);

			point0.x += 0.1f;
			point1.x -= 0.1f;
			point0.y -= 0.1f;
			point1.y += 0.1f;
			render_debug_line(TRUE, &point0, &point1, color);
		}

		for (color_index = 0; color_index<num_firing_position_colors; color_index++)
		{
			points[3].z = points[0].z + 0.05f;
			points[2].z = points[3].z;
			points[1].z = points[2].z;
			points[0].z = points[1].z;

			if (firing_position_crosses[color_index])
			{
				render_debug_line(TRUE, &points[0], &points[2], firing_position_colors[color_index]);
				render_debug_line(TRUE, &points[1], &points[3], firing_position_colors[color_index]);
			}
			else
			{
				render_debug_polygon_edges(points, 4, firing_position_colors[color_index]);
			}
		}

		point_from_line3d(&firing_position->position, global_up3d, 0.2f, &position);
		ai_debug_drawstack_setup(&position);

		if (ai_debug.render_pursuit && ai_debug.firing_positions[firing_position_index].pursuit_position)
		{
			boolean already_examined;
			short actor_count;
			boolean current_pursuit_position = FALSE;

			if (ai_debug.selected_actor_index!=NONE)
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "%3.2f", ai_debug.firing_positions[firing_position_index].last_evaluation.evaluation),
					ai_debug.firing_positions[firing_position_index].last_evaluation.valid ? global_real_argb_white : global_real_argb_red);

				if (ai_debug.selected_actor_index!=NONE)
				{
					struct pursuit_location *pursuit_location = actor_get_pursuit_location(ai_debug.selected_actor_index);

					current_pursuit_position = pursuit_location && pursuit_location->type==_pursuit_location_position && pursuit_location->firing_position_index==firing_position_index;
				}
			}

			already_examined = encounter_pursuit_position_already_examined(
				encounter_index,
				ai_debug.selected_actor_index,
				firing_position_index,
				history_start_time==NONE ? 0 : history_start_time,
				&actor_count,
				NULL);
			if (ai_debug.selected_actor_index==NONE)
			{
				already_examined = FALSE;
			}

			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(temporary, "%d", actor_count),
				current_pursuit_position ? global_real_argb_yellow : (already_examined ? global_real_argb_blue : global_real_argb_white));

			if (ai_debug.firing_position_context_valid &&
				ai_debug.firing_position_context.has_target &&
				ai_debug.firing_position_context.find_path_direction_from_target &&
				ai_debug.firing_positions[firing_position_index].evaluated &&
				ai_debug.firing_positions[firing_position_index].last_evaluation.valid)
			{
				real_point3d point;

				add_vectors3d(
					(real_vector3d *)&ai_debug.firing_position_context.target_point,
					&ai_debug.firing_positions[firing_position_index].last_evaluation.path_direction_from_target,
					(real_vector3d *)&point);
				render_debug_line(TRUE, &ai_debug.firing_position_context.target_point, &point, global_real_argb_yellow);
				render_debug_line(TRUE, &point, &firing_position->position, global_real_argb_green);
			}
		}
		else if (ai_debug.render_evaluations &&
			ai_debug.firing_positions[firing_position_index].evaluated &&
			!ai_debug.firing_positions[firing_position_index].pursuit_position &&
			ai_debug.selected_actor_index!=NONE)
		{
			real_argb_color const *color;
			real_argb_color const *pre_evaluation_color = NULL;

			if (!ai_debug.firing_positions[firing_position_index].last_evaluation.valid)
			{
				color = global_real_argb_red;
			}
			else if (ai_debug.firing_positions[firing_position_index].last_evaluation.pre_evaluation>0.f)
			{
				pre_evaluation_color = global_real_argb_white;
				color = firing_position_owner_actor_indices[firing_position_index]==ai_debug.selected_actor_index ? global_real_argb_yellow : global_real_argb_blue;
			}
			else
			{
				color = global_real_argb_white;
			}

			if (pre_evaluation_color)
			{
				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "%3.2f", ai_debug.firing_positions[firing_position_index].last_evaluation.pre_evaluation),
					pre_evaluation_color);
			}

			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(temporary, "%3.2f", ai_debug.firing_positions[firing_position_index].last_evaluation.evaluation),
				color);
		}
	}

	return;
}

static void ai_debug_render_vehicles_enterable(
	void)
{
	short index;

	for (index = 0; index<ai_globals->enterable_vehicle_count; index++)
	{
		struct ai_vehicle_enterable *enterable = &ai_globals->enterable_vehicles[index];

		if (unit_try_and_get(enterable->vehicle_index))
		{
			real_point3d position;

			object_get_origin(enterable->vehicle_index, &position);
			point_from_line3d(&position, global_up3d, 0.5f, &position);
			ai_debug_drawstack_setup(&position);

			render_debug_string_at_point(
				TRUE,
				ai_debug_drawstack(),
				csprintf(temporary, "enterable: dist %.1f", enterable->radius),
				global_real_argb_pink);

			if (enterable->team_bitmask)
			{
				char const *teams[NUMBER_OF_SOLO_CAMPAIGN_TEAMS] =
				{
					"default",
					"player",
					"human",
					"covenant",
					"flood",
					"sentinel",
					"unused6",
					"unused7",
					"unused8",
					"unused9"
				};

				sprintf(temporary, "teams:");
				for (index = 0; index<NUMBER_OF_SOLO_CAMPAIGN_TEAMS; index++)
				{
					if (enterable->team_bitmask & FLAG(index))
					{
						strcat(temporary, " ");
						strcat(temporary, teams[index]);
					}
				}
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_pink);
			}

			if (enterable->actor_type_bitmask)
			{
				char const *actor_types[NUMBER_OF_ACTOR_TYPES] =
				{
					"elite",
					"jackal",
					"grunt",
					"hunter",
					"engineer",
					"assassin",
					"player",
					"marine",
					"crew",
					"combat form",
					"infection form",
					"carrier form",
					"monitor",
					"sentinel",
					"none",
					"mounted weapon"
				};

				sprintf(temporary, "actor types:");
				for (index = 0; index<NUMBER_OF_ACTOR_TYPES; index++)
				{
					if (enterable->actor_type_bitmask & FLAG(index))
					{
						strcat(temporary, " ");
						strcat(temporary, actor_types[index]);
					}
				}
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_pink);
			}

			if (enterable->ai_indices_count>0)
			{
				sprintf(temporary, "actors:");
				for (index = 0; index<enterable->ai_indices_count; index++)
				{
					char buffer[256];

					ai_index_to_string(enterable->ai_indices[index], global_scenario_get(), buffer, sizeof(buffer));
					strcat(temporary, " ");
					strcat(temporary, buffer);
				}
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_pink);
			}
		}
	}

	return;
}

void ai_debug_render(
	void)
{
	if (ai_globals->ai_initialized_for_map)
	{
		global_ai_debug_string_position = rasterizer_globals.frame_bounds.y1 - 20;
		ai_debug.last_render_id = (ai_debug.last_render_id + 1) % 1000;

		if (ai_debug.selected_actor_index!=NONE)
		{
			ai_debug.selected_encounter_index = actor_get(ai_debug.selected_actor_index)->meta.encounter_index;
		}

		if (ai_debug.select_this_actor)
		{
			ai_debug_select_this_actor();
		}

		if (ai_debug.render)
		{
			if (ai_debug.render_lineoffire)
			{
				ai_debug_render_lineoffire();
			}

			if (ai_debug.render_lineofsight)
			{
				ai_debug_render_lineofsight();
			}

			if (ai_debug.render_ballistic_lineoffire)
			{
				ai_debug_render_ballistic_lineoffire();
			}

			if (ai_debug.selected_encounter_index!=NONE)
			{
				ai_debug_render_encounter(ai_debug.selected_encounter_index);
			}

			if (ai_debug.selected_actor_index!=NONE)
			{
				ai_debug_render_actor(ai_debug.selected_actor_index, TRUE, NULL);
			}

			if (ai_debug.path_enable)
			{
				ai_debug_render_path();
			}

			if (ai_debug.render_paths_failed)
			{
				ai_debug_render_paths_failed();
			}

			if (ai_debug.render_aiming_validity)
			{
				ai_debug_render_aiming_validity();
			}

			if (ai_debug.render_all_actors)
			{
				ai_debug_render_all_actors(ai_debug.render_inactive_actors);
			}

			if (ai_debug.render_speech || ai_debug.print_speech || ai_debug.render_dialogue_variants)
			{
				ai_debug_render_speech();
			}

			if (ai_debug.render_idle_look)
			{
				ai_debug_render_idle_look();
			}

			if (ai_debug.render_spatial_effects)
			{
				ai_debug_render_spatial_effects();
			}

			if (ai_debug.render_vehicles_enterable)
			{
				ai_debug_render_vehicles_enterable();
			}
		}
	}

	return;
}
