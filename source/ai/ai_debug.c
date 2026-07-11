/*
AI_DEBUG.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_debug.h"

#include "actions.h"
#include "actors.h"
#include "actor_definitions.h"
#include "ai_communication.h"
#include "ai_scenario_definitions.h"
#include "encounters.h"
#include "props.h"

#include "camera/observer.h"
#include "game/game.h"
#include "game/players.h"
#include "memory/data.h"
#include "objects/damage.h"
#include "physics/collision_bsp_definitions.h"
#include "rasterizer/rasterizer.h"
#include "render/render_debug.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "structures/structure_bsp_definitions.h"
#include "text/draw_string.h"
#include "units/bipeds.h"
#include "units/biped_definitions.h"
#include "units/units.h"
#include "units/unit_definitions.h"

/* ---------- constants */

/* ---------- macros */

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

void ai_debug_drawstack_setup(union real_point3d const *drawstack_base);
static real_point3d *ai_debug_drawstack(void);

static void ai_debug_highlight_unit(long unit_index, boolean render_exclusive, union real_argb_color const *color);

static void ai_debug_render_path_nodes(
	struct path_state *path_state,
	boolean bsp_access_allowed,
	boolean render_all_nodes,
	boolean render_polygons,
	boolean render_costs,
	boolean render_closest);
static void ai_debug_render_surface(struct structure_bsp const *structure_bsp, long surface_index, real offset, union real_argb_color const *color);

static void ai_debug_render_actor(long actor_index, boolean render_exclusive, long *history_start_time);
static void ai_debug_render_path_storage(struct path_debug_storage *path);

/* ---------- globals */

struct ai_debug_state ai_debug;

struct actor_debug_info *actor_debug_array = NULL;
struct path_debug_storage *actor_path_debug_array = NULL;

real_point3d global_ai_debug_drawstack_next_position;
real_point3d global_ai_debug_drawstack_last_position;
real global_ai_debug_drawstack_height;
real_argb_color global_temporary_render_color;
long global_ai_debug_firing_position_color_count = NONE;

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

static long global_ai_debug_selected_squad_index = NONE;
static long global_ai_debug_selected_squad_time = NONE;
static unsigned long global_ai_debug_activation_cluster_bit_vector[16];

const real_argb_color global_ai_debug_firing_position_colors[] =
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
	ai_debug.selected_squad_index = NONE;
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

	if (scenario && ai_debug.selected_squad_index!=NONE)
	{
		struct encounter_definition* encounter = TAG_BLOCK_GET_ELEMENT(
			&scenario->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_debug.selected_squad_index),
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

	return found_path_index!=NONE ? &actor_path_debug_array[found_path_index] : NULL;
}

boolean ai_debug_highlight_cluster(
	short index,
	real_argb_color const **highlight_color)
{
	boolean result = FALSE;

	if (ai_debug.render_encounter_activeregion && ai_debug.selected_squad_index!=NONE)
	{
		if (global_ai_debug_selected_squad_time != game_time_get() ||
			global_ai_debug_selected_squad_index != ai_debug.selected_squad_index)
		{
			encounter_compute_activation_cluster_bit_vector(
				ai_debug.selected_squad_index,
				FALSE,
				SIZEOF_BITS(global_ai_debug_activation_cluster_bit_vector),
				0,
				global_ai_debug_activation_cluster_bit_vector);
			global_ai_debug_selected_squad_time = game_time_get();
			global_ai_debug_selected_squad_index = ai_debug.selected_squad_index;
		}

		match_assert("c:\\halo\\SOURCE\\ai\\ai_debug.c", 4133, highlight_color);
	
		if (BIT_VECTOR_TEST_FLAG(global_ai_debug_activation_cluster_bit_vector, index))
		{
			if (encounter_get(ai_debug.selected_squad_index)->active)
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

// TODO: remove this!!!!!
void test(void)
{
	ai_debug_render_actor(NULL, FALSE, NULL);
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
	union real_argb_color const *color)
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

// TODO: finish
static void ai_debug_render_path_nodes(
	struct path_state *path_state,
	boolean bsp_access_allowed,
	boolean render_all_nodes,
	boolean render_polygons,
	boolean render_costs,
	boolean render_closest)
{
	real_argb_color const *attractor_distance_color;
	real_argb_color const *closest_color;
	real_point3d temp_point2;
	real_argb_color const *polygon_color;
	real_argb_color const *cost_color;
	real_argb_color const *attractor_weight_color;
	real_point3d temp_point;
	static short current_traverse_index;

	return;
}

static void ai_debug_render_surface(
	struct structure_bsp const *structure_bsp,
	long surface_index,
	real offset,
	union real_argb_color const *color)
{
	struct collision_bsp const *collision_bsp = TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp);
	struct collision_surface const *collision_surface = TAG_BLOCK_GET_ELEMENT(&collision_bsp->surfaces, surface_index, struct collision_surface);
	long edge_index = collision_surface->first_edge_index;

	do
	{
		struct collision_edge const *edge = TAG_BLOCK_GET_ELEMENT(&collision_bsp->edges, edge_index, struct collision_edge);
		const boolean next_index_belongs_to_surface = edge->surface_indices[1] == surface_index;
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

		if (!actor->meta.swarm)
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
				char const *blocking_type;

				char const *blocking_mode = actor->emotions.crouch_friends_in_line_of_fire ? "friends-blocking" : "";

				if (actor->emotions.crouch_blocking_player_line_of_fire)
				{
					blocking_type = "blocking-player ";
				}
				else
				{
					blocking_type = actor->emotions.crouch_blocking_line_of_fire ? "blocking " : "";
				}

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "%s%s", blocking_mode, blocking_type),
					global_real_argb_orange);
			}

			/* Moving into fire */

			if (actor->emotions.moving_into_player_line_of_fire ||
				actor->emotions.moving_into_fire_timer > 0)
			{
				char const *string = actor->emotions.moving_into_player_line_of_fire ? "moving-into-fire" : "";

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "%sforce-stop %d", string, actor->emotions.moving_into_fire_timer),
					global_real_argb_pink);
			}
		}

		/* Player aiming blocked */

		if (ai_debug.render_player_aiming_blocked)
		{
			struct prop_datum *prop;
			struct prop_iterator iterator;
			real_vector3d aiming_vector;

			short blockage_type = 0;
			boolean player_prop_found = FALSE;

			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				if (prop->state >= _prop_state_becoming_unacknowledged &&
					prop->state <= _prop_state_acknowledged &&
					!prop->enemy)
				{
					if (prop->player)
					{
						short v0;

						player_prop_found = TRUE;
						unit_get_aiming_vector(prop->unit_index, &aiming_vector);
						v0 = actor_perception_aiming_vector_test_blockage(
							&prop->body_position,
							&aiming_vector,
							&actor->input.position.body_position,
							NULL);

						if (blockage_type <= v0)
						{
							blockage_type = v0;
						}
					}
				}
			}

			if (player_prop_found)
			{
				switch (blockage_type)
				{
				case 0:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "not-occluding-player", global_real_argb_green);
					break;
				case 1:
					render_debug_string_at_point(TRUE, ai_debug_drawstack(), "occluding-player", global_real_argb_blue);
					break;
				case 2:
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
							actor_debug_info->collision_t[ray_index],
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

						switch (actor_debug_info->field_62F8[i][j])
						{
						case 1:
							color = global_real_argb_aqua;
							break;
						case 2:
							color = global_real_argb_yellow;
							break;
						}

						if (actor_debug_info->field_62F8[i][j]>0)
						{
							real_point3d point0;
							real_point3d point1;

							point_from_line3d(
								&actor_debug_info->field_6358[i][j],
								&actor_debug_info->field_6418[i][j],
								actor_debug_info->avoid_t[i][j],
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
									global_real_argb_blue);
							}
						}
						else
						{
							render_debug_vector(
								TRUE,
								&actor_debug_info->field_6358[i][j],
								&actor_debug_info->field_6418[i][j],
								1.f,
								color);

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
							actor_debug_info->field_64D8[i]),
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
					set_real_vector3d(&height, 0.f, 0.f, actor_debug_info->avoidance_data.avoidance_objects[avoidance_object_index].height);
					render_debug_pill(
						TRUE,
						&actor_debug_info->avoidance_data.avoidance_objects[avoidance_object_index].base,
						&height,
						actor_debug_info->avoidance_data.avoidance_objects[avoidance_object_index].width,
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
				char encounterbuf[256];
				real_argb_color const *encounter_color;

				if (actor->meta.encounter_index!=NONE)
				{
					struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->ai_encounters,
						DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
						struct encounter_definition);
					struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);
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

					if (encounter->active)
					{
						encounter_color = global_real_argb_green;
					}
					else if (outside_current_bsp)
					{
						encounter_color = global_real_argb_red;
					}
					else
					{
						encounter_color = global_real_argb_purple;
					}
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
						real_vector3d offset_vector;
						real_point3d base_point;
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

						{
							struct object_datum const *ultimate_parent = object_get(object_get_ultimate_parent(unit_index));
							cluster_index = ultimate_parent->object.location.cluster_index;
						}

						if (cluster_index==NONE)
						{
							real_point3d p0;
							real_point3d p1;
							real_point3d p2;
							real_point3d p3;

							point_from_line3d(&base_point, &offset_vector, 0.1f, &p0);
							point_from_line3d(&base_point, &offset_vector, -0.1f, &p1);
							point_from_line3d(&p0, global_up3d, 0.2f, &p2);
							point_from_line3d(&p1, global_up3d, 0.2f, &p3);

							render_debug_line(TRUE, &p0, &p2, actor_color);
							render_debug_line(TRUE, &p2, &p3, actor_color);
							render_debug_line(TRUE, &p3, &p1, actor_color);
							render_debug_line(TRUE, &p1, &p0, actor_color);
						}
						else if (BIT_VECTOR_TEST_FLAG(pvs, cluster_index))
						{
							real_point3d p0;
							real_point3d p1;
							real_point3d p2;
							real_point3d p3;
							real_point3d mid_point = base_point;

							point_from_line3d(&base_point, global_up3d, 0.2f, &p0);
							point_from_line3d(&base_point, global_up3d, 0.1f, &p1);
							point_from_line3d(&p1, &offset_vector, 0.1f, &p3);
							point_from_line3d(&p1, &offset_vector, -0.1f, &p2);

							render_debug_line(TRUE, &mid_point, &p3, actor_color);
							render_debug_line(TRUE, &p3, &p0, actor_color);
							render_debug_line(TRUE, &p0, &p2, actor_color);
							render_debug_line(TRUE, &p2, &mid_point, actor_color);
						}
						else
						{
							real_point3d p0;
							real_point3d p1;
							real_point3d p2;
							real_point3d p3;

							point_from_line3d(&base_point, &offset_vector, 0.1f, &p0);
							point_from_line3d(&base_point, &offset_vector, -0.1f, &p1);
							point_from_line3d(&p0, global_up3d, 0.2f, &p2);
							point_from_line3d(&p1, global_up3d, 0.2f, &p3);

							render_debug_line(TRUE, &p0, &p3, actor_color);
							render_debug_line(TRUE, &p2, &p1, actor_color);
						}
					}

					if (!swarm || unit_count >= swarm->unit_count)
					{
						break;
					}

					unit_index = swarm->unit_indices[unit_count++];
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
			const char *strings[8] =
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

			render_debug_string_at_point(TRUE, ai_debug_drawstack(), actor->meta.team_index!=NONE ? teams[actor->meta.team_index] : "none", global_real_argb_green);
		}

		/* Player ratings */

		if (ai_debug.render_player_ratings && actor->meta.unit_index!=NONE)
		{
			real player_rating = ai_communication_get_player_rating(actor->meta.unit_index, TRUE, NULL, NULL);
			render_debug_string_at_point(TRUE, ai_debug_drawstack(), csprintf(temporary, "%.1f", player_rating), player_rating==0.f ? global_real_argb_blue : global_real_argb_white);
		}

		/* Audibility */

		if (ai_debug.render_audibility && actor_debug_info->field_A4)
		{
			real_argb_color const *color;
			char const *aud_type;
			char textstring[512];

			if (actor_debug_info->field_A6==0)
			{
				color = global_real_argb_red;
				aud_type = "none";
				
			}
			else if(actor_debug_info->field_A6==1)
			{
				color = global_real_argb_blue;
				aud_type = "part";
			}
			else
			{
				color = global_real_argb_green;
				aud_type = "full";
			}

			sprintf(textstring, "aud/%s %.1fp/%.1fd", aud_type, actor_debug_info->field_A8, actor_debug_info->field_AC);

			if (actor_debug_info->field_B0!=-1.f)
			{
				strcat(textstring, csprintf(temporary, "/%.1fs/%.1ff", actor_debug_info->field_B0, actor_debug_info->field_B4));
			}

			render_debug_string_at_point(TRUE, ai_debug_drawstack(), textstring, color);
		}

		/* Props */

		if (ai_debug.render_props || ai_debug.render_props_web)
		{
			struct prop_datum *prop;
			struct prop_iterator iterator;

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
					real_argb_color const *color;
					real_point3d origin;
					real_point3d string_point;

					point_from_line3d(&prop->head_position, global_up3d, 0.2, &string_point);

					switch (prop->state)
					{
					case 2:
					case 3:
						render_debug_line(TRUE, &prop_start_point, &prop->head_position, global_real_argb_yellow);
						break;
					case 1:
						set_real_point3d(
							&origin,
							((1.f-prop->awareness)*prop_start_point.x) + (prop->awareness*prop->head_position.x),
							((1.f-prop->awareness)*prop_start_point.y) + (prop->awareness*prop->head_position.y),
							((1.f-prop->awareness)*prop_start_point.z) + (prop->awareness*prop->head_position.z)
						);
						render_debug_line(TRUE, &prop_start_point, &origin, global_real_argb_yellow);
						render_debug_line(TRUE, &origin, &prop->head_position, global_real_argb_black);
						break;
					case 0:
						render_debug_line(TRUE, &prop_start_point, &prop->head_position, global_real_argb_black);
						break;
					case 4:
					case 5:
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

					if (ai_debug.render_props_unreachable)
					{
						real_argb_color const *color;

						if (prop->ignore)
						{
							color = global_real_argb_blue;
						}
						else
						{
							color = prop->preferred_target ? global_real_argb_pink : global_real_argb_red;
						}

						render_debug_string_at_point(TRUE, &string_point, csprintf(temporary, "%.2f", prop->target_weight), color);
						point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
						
						if (iterator.index==actor->target.target_prop_index)
						{
							render_debug_string_at_point(TRUE, &string_point, "target", global_real_argb_white);
							point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
						}
					}

					if (ai_debug.render_props_unopposable && prop->unreachable_ticks>0)
					{
						long time = prop->last_unreachable_time!=NONE ? game_time_get()-prop->last_unreachable_time : NONE;

						render_debug_string_at_point(
							TRUE,
							&string_point,
							csprintf(temporary, "unr %d %d", prop->unreachable_ticks, time),
							global_real_argb_darkgreen);
						point_from_line3d(&string_point, global_up3d, 0.05f, &string_point);
					}

					if (ai_debug.render_props_target_weight && prop->unopposable_enemy)
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

			if (ai_debug.render_props_target_weight && actor->emotions.unopposable_retreat_timer>0)
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
					if (location->type==1)
					{
						render_debug_line(TRUE, &actor->input.position.head_position, &location->position, actor_action_debug_color(actor_index));
					}
				}
			}
			
		}

		/* Aiming vectors */

		if (render_exclusive && ai_debug.render_aiming_vectors)
		{
			if (actor->meta.unit_index!=NONE)
			{
				real_point3d p0;
				real_point3d p1;
				real_vector3d forward;

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
					real_point3d end_point;

					struct biped_definition *biped_definition = biped_definition_get(unit->definition_index);
					
					unit_get_facing_vector(actor->meta.unit_index, &forward);

					if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit))
					{
						real_vector3d left;
						real_vector3d up;

						biped_build_flying_axes(&forward, &left, &up);
						scale_vector3d(&forward, actor->output.throttle.i, (real_vector3d *)&end_point);
						point_from_line3d(&end_point, &left, actor->output.throttle.j, &end_point);
						point_from_line3d(&end_point, &up, actor->output.throttle.k, &end_point);
					}
					else
					{
						real_vector3d v;

						set_real_vector3d(&v, -forward.j, forward.i, 0.f);
						scale_vector3d(&forward, actor->output.throttle.i, (real_vector3d *)&end_point);
						point_from_line3d(&end_point, &v, actor->output.throttle.j, &end_point);
					}

					point_from_line3d(&actor->input.position.body_position, global_up3d, 0.1f, &p0);
					render_debug_vector(TRUE, &p0, (real_vector3d *)&end_point, 1.6f, global_real_argb_pink);
				}
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
			actor->target.target_type!=0 &&
			actor->target.target_prop_index!=NONE)
		{
			real_point3d actor_target_position;
			real_point3d prop_target_position;

			struct prop_datum *prop = prop_get(actor->target.target_prop_index);
			real_argb_color const *target_color = global_real_argb_white;

			switch (actor->target.target_type-1)
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
			
			if (prop->unreachable_ticks>0 && (!ai_debug.render_props || !ai_debug.render_props_unopposable))
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
				struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
					struct encounter_definition);
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
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
				if (actor->state.action_data.flee.has_approach_point>0)
				{
					render_debug_sphere(TRUE, &actor->state.action_data.flee.approach_point, 0.25f, actor_action_debug_color(actor_index));
				}
				break;	
			case _actor_action_fight:
				if (actor->state.action_data.alert.move_position_order>0)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "change %d", actor->state.action_data.alert.move_position_order),
						actor_action_debug_color(actor_index));
				}
				break;
			case _actor_action_guard:
				if (actor->state.action_data.guard.wait_ticks>0)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "wait %d", actor->state.action_data.guard.wait_ticks),
						actor_action_debug_color(actor_index));
				}

				if (actor->state.action_data.guard.look_ticks>0)
				{
					render_debug_string_at_point(
						TRUE,
						ai_debug_drawstack(),
						csprintf(temporary, "look %d", actor->state.action_data.guard.look_ticks),
						actor_action_debug_color(actor_index));
				}

				if (actor->state.action_data.guard.cower)
				{
					if (actor->state.action_data.guard.cower_from_retreat)
					{
						render_debug_string_at_point(
							TRUE,
							ai_debug_drawstack(),
							csprintf(temporary, "retreat %d", actor->emotions.unopposable_retreat_timer),
							actor_action_debug_color(actor_index));
					}
					else
					{
						char const *string = actor->state.action_data.guard.cower_panicked ? "panic" : "hide";
						render_debug_string_at_point(
							TRUE,
							ai_debug_drawstack(),
							csprintf(temporary, "%s %d", string, actor->state.action_data.guard.cower_ticks),
							actor_action_debug_color(actor_index));
					}
				}

				if (actor->state.action_data.guard.has_guard_direction)
				{
					render_debug_vector(
						TRUE,
						&actor->input.position.head_position,
						&actor->state.action_data.guard.guard_direction,
						2.5f,
						actor_action_debug_color(actor_index));
				}
				break;
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
					if (120-actor->state.action_data.search.search_failure_timer<=actor->state.action_data.search.search_remaining_time)
					{
						delay = 120-actor->state.action_data.search.search_failure_timer;
					}
					else
					{
						delay = actor->state.action_data.search.search_remaining_time;
					}
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
					char buffer[256];

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
				struct ai_command_definition *command = NULL;

				if (actor->state.action_data.obey.command_list_index!=NONE)
				{
					struct ai_command_list_definition const* command_list = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->ai_command_lists,
						actor->state.action_data.obey.command_list_index,
						struct ai_command_list_definition);
					
					if (!actor->meta.swarm)
					{
						render_debug_string_at_point(
							TRUE,
							ai_debug_drawstack(),
							csprintf(
								temporary,
								"command-list %s: #%d of #%d",
								command_list->name,
								actor->state.action_data.obey.simple_control.current_command_index+1,
								command_list->commands.count),
							global_real_argb_purple);
						
						if (actor->state.action_data.obey.simple_control.current_command_index<command_list->commands.count)
						{
							command = TAG_BLOCK_GET_ELEMENT(
								&command_list->commands,
								actor->state.action_data.obey.simple_control.current_command_index,
								struct ai_command_definition);
						}

						if (TEST_FLAG(actor->state.action_data.obey.simple_control.metadata_flags, _obey_metadata_commands_finished_bit))
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
								if (actor->state.action_data.charge.melee_suicide)
								{
									render_debug_sphere(
										TRUE,
										&actor->state.action_data.obey.complex_control.destination_point,
										actor->state.action_data.obey.complex_control.destination_radius_valid ? 
										actor->state.action_data.obey.complex_control.destination_radius :
										0.5f,
										global_real_argb_purple);
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
							(unsigned long)(
								actor_variant_definition->ranged_combat.new_target_pattern_time*TICKS_PER_SECOND - 
								(real)actor->control.current_fire_target_timer
							)
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

			if (VALID_INDEX(actor_debug_info->firing_decision, NUMBER_OF_ACTOR_DEBUG_FIRING_DECISIONS))
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
				real damage = unit==NULL ? 0.f : unit->object.current_body_damage;
				char const *string = actor->meta.unit_index==NONE || !unit_is_busy(actor->meta.unit_index) ? "not-" : "";

				render_debug_string_at_point(
					TRUE,
					ai_debug_drawstack(),
					csprintf(temporary, "trying: %sbusy dmg %.1f", string, damage),
					global_real_argb_pink);
			}
		}

		/* Danger zones */

		if (ai_debug.render_danger_zones && actor->danger_zone.danger_type>0)
		{
			real_argb_color const *color;

			if (actor->danger_zone.currently_perceived)
			{
				if (actor->danger_zone.acknowledgement_timer>0)
				{
					color = global_real_argb_white;
				}
				else
				{
					if (actor->danger_zone.noticed_danger)
					{
						color = global_real_argb_yellow;
					}
					else
					{
						color = global_real_argb_blue;
					}
				}
			}
			else
			{
				color = global_real_argb_darkgreen;
			}

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
				char const *string = actor->danger_zone.projectile.time_until_explosion==NONE ?
					"NONE" :
					csprintf(temporary, "%d", actor->danger_zone.projectile.time_until_explosion);

				render_debug_string_at_point(
					TRUE,
					&actor->danger_zone.position,
					string,
					color);
			}

			if (actor->danger_zone.danger_type==_actor_unopposable_danger_visible)
			{
				char const *string = actor->danger_zone.projectile.time_until_explosion==NONE ?
					"NONE" :
					csprintf(temporary, "%d", actor->danger_zone.projectile.time_until_explosion);

				render_debug_string_at_point(
					TRUE,
					&actor->danger_zone.position,
					string,
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

			const char *control_flag_names[NUMBER_OF_UNIT_CONTROL_FLAGS] =
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

			short flag_count = NUMBER_OF_UNIT_CONTROL_FLAGS;
			short count = 0;

			strcpy(temporary, "");

			for (control_flag_bit = 0; control_flag_bit<NUMBER_OF_UNIT_CONTROL_FLAGS; ++control_flag_bit)
			{
				if (TEST_FLAG(actor->output.control_flags, control_flag_bit))
				{
					if (count > 0)
					{
						strcat(temporary, " ");
					}

					if (control_flag_bit<NUMBER_OF_UNIT_CONTROL_FLAGS)
					{
						strcat(temporary, control_flag_names[control_flag_bit]);
					}
					else
					{
						char string[88];

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

			for (control_flag_bit = 0; control_flag_bit<NUMBER_OF_UNIT_CONTROL_FLAGS; ++control_flag_bit)
			{
				if (TEST_FLAG(actor->output.persistent_control_flags, control_flag_bit))
				{
					if (count > 0)
					{
						csstrcat(temporary, " ");
					}

					if (control_flag_bit<NUMBER_OF_UNIT_CONTROL_FLAGS)
					{
						strcat(temporary, control_flag_names[control_flag_bit]);
					}
					else
					{
						char string[88];

						sprintf(string, "<unknown %d>", control_flag_bit);
						strcat(temporary, string);
					}

					++count;
				}
			}

			if (count>0)
			{
				char string[88];

				sprintf(string, ": persistent %d", actor->output.persistent_control_ticks);
				strcat(temporary, string);
	
				render_debug_string_at_point(TRUE, ai_debug_drawstack(), temporary, global_real_argb_orange);
			}

			{
				const char *movement_type_strings[NUMBER_OF_ACTOR_MOVEMENT_TYPES] =
				{
					"noncombat",
					"asleep",
					"combat",
					"flee",
					NULL
				};

				const char *aiming_speed_names[NUMBER_OF_UNIT_AIMING_SPEEDS] =
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
				
				alignment_vector_3d = actor->control.current_fire_target_aim_vector;

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
					real_vector3d facing_vector;
					real_vector3d right_facing_vector;

					struct biped_definition const *biped_definition = biped_definition_get(unit->definition_index);

					unit_get_facing_vector(actor->meta.unit_index, &facing_vector);

					if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit))
					{
						real_vector3d left_vector;
						real_vector3d up_vector;

						biped_build_flying_axes(&facing_vector, &left_vector, &up_vector);
						scale_vector3d(&facing_vector, actor->output.throttle.i, &throttle_vector);
						point_from_line3d((real_point3d *)&throttle_vector, &left_vector, actor->output.throttle.j, (real_point3d *)&throttle_vector);
						point_from_line3d((real_point3d *)&throttle_vector, &up_vector, actor->output.throttle.k, (real_point3d *)&throttle_vector);
					}
					else
					{
						set_real_vector3d(&right_facing_vector, -facing_vector.j, facing_vector.i, 0.f);
						scale_vector3d(&facing_vector, actor->output.throttle.i, (real_vector3d *)&throttle_vector);
						point_from_line3d((real_point3d *)&throttle_vector, &right_facing_vector, actor->output.throttle.j, (real_point3d *)&throttle_vector);
					}

					point_from_line3d(&actor->input.position.body_position, global_up3d, 0.1f, &p0);
					render_debug_vector(TRUE, &p0, &throttle_vector, 1.6f, global_real_argb_purple);
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
				sprintf(temporary, "melee-success (%sairborne)", actor_debug_info->field_198 ? "not-" : "not-" );
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
				real_vector3d v;
				real_point3d p0;
				real_point3d p1;
				real_point3d p2;

				render_debug_line(TRUE, &actor_debug_info->field_120, &actor_debug_info->field_13C, global_real_argb_purple);
				render_debug_sphere(TRUE, &actor_debug_info->field_13C, 0.2f, global_real_argb_purple);
				
				point_from_line3d(&actor_debug_info->field_108, &actor_debug_info->field_12C, actor_debug_info->field_14C, &p0);
				render_debug_line(TRUE, &actor_debug_info->field_108, &p0, global_real_argb_green);
				
				point_from_line3d(&actor_debug_info->field_108, &actor_debug_info->field_12C, actor_debug_info->field_148, &p1);
				perpendicular3d(&actor_debug_info->field_12C, &v);
				
				point_from_line3d(&p1, &v, 0.3f, &p0);
				point_from_line3d(&p1, &v, -0.3f, &p2);
				render_debug_line(TRUE, &p0, &p2, global_real_argb_green);
			}

			if (actor_debug_info->field_138)
			{
				render_debug_vector(TRUE, &actor_debug_info->field_108, &actor_debug_info->field_12C, 2.f, actor_debug_info->field_139 ? global_real_argb_yellow : global_real_argb_purple);
			}
			else
			{
				render_debug_vector(TRUE, &actor_debug_info->field_108, &actor_debug_info->field_12C, 2.f, global_real_argb_white);
			}
		}

		/* Vehicle avoidance */

		if (ai_debug.render_vehicle_avoidance &&
			actor_debug_info->last_vehicle_avoidance_time!=NONE &&
			actor_debug_info->last_vehicle_avoidance_time+TICKS_PER_SECOND >= game_time_get())
		{
			real_point3d p0;
			real_point3d p1;
			real_point3d p2;

			real_vector3d v0;

			render_debug_line(TRUE, &actor_debug_info->field_C8, &actor_debug_info->field_E4, global_real_argb_green);
			render_debug_sphere(TRUE, &actor_debug_info->field_D4, actor_debug_info->field_E0, global_real_argb_yellow);
		
			p1 = actor_debug_info->field_E4;
			p0 = p1;

			p0.x = p1.x - 0.2f;
			p1.x = p1.x + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_red);
			
			p0.x = p0.x + 0.2f;
			p1.x = p1.x - 0.2f;
			p0.y = p0.y - 0.2f;
			p1.y = p1.y + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_red);
			
			p0.y = p0.y + 0.2f;
			p1.y = p1.y - 0.2f;
			p0.z = p0.z - 0.2f;
			p1.z = p1.z + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_red);
			vector_from_points3d(&actor_debug_info->field_C8, &actor_debug_info->field_E4, &v0);
			point_from_line3d(&actor_debug_info->field_C8, &v0, actor_debug_info->field_F0, &p2);
			
			p1 = p2;
			p0 = p2;

			p0.x = p2.x - 0.2f;
			p1.x = p2.x + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);
			
			p0.x = p0.x + 0.2f;
			p1.x = p1.x - 0.2f;
			p0.y = p0.y - 0.2f;
			p1.y = p1.y + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);
			
			p0.y = p0.y + 0.2f;
			p1.y = p1.y - 0.2f;
			p0.z = p0.z - 0.2f;
			p1.z = p1.z + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);

			if (actor_debug_info->field_F4)
			{
				real_point3d p3;
				real_point3d p4;

				render_debug_line(TRUE, &actor_debug_info->field_D4, &actor_debug_info->field_F8, global_real_argb_yellow);
				render_debug_line(TRUE, &actor_debug_info->field_C8, &actor_debug_info->field_F8, global_real_argb_red);
				
				p4 = actor_debug_info->field_F8;
				p3 = p4;
				
				p3.x = p4.x - 0.2f;
				p4.x = p4.x + 0.2f;
				render_debug_line(TRUE, &p3, &p4, global_real_argb_blue);
				
				p3.x = p3.x + 0.2f;
				p4.x = p4.x - 0.2f;
				p3.y = p3.y - 0.2f;
				p4.y = p4.y + 0.2f;
				render_debug_line(TRUE, &p3, &p4, global_real_argb_blue);
				
				p3.y = p3.y + 0.2f;
				p4.y = p4.y - 0.2f;
				p3.z = p3.z - 0.2f;
				p4.z = p4.z + 0.2f;
				render_debug_line(TRUE, &p3, &p4, global_real_argb_blue);
			}
		}

		/* Projectile aiming */

		if (ai_debug.render_projectile_aiming &&
			actor_debug_info->last_projectile_aiming_time!=NONE &&
			actor_debug_info->last_projectile_aiming_time+15 >= game_time_get())
		{
			real_point3d p0;
			real_point3d p1;

			p1 = actor_debug_info->field_64;
			p0 = p1;
			
			p0.x = p1.x - 0.2f;
			p1.x = p1.x + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);
			
			p0.x = p0.x + 0.2f;
			p1.x = p1.x - 0.2f;
			p0.y = p0.y - 0.2f;
			p1.y = p1.y + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);
			
			p0.y = p0.y + 0.2f;
			p1.y = p1.y - 0.2f;
			p0.z = p0.z - 0.2f;
			p1.z = p1.z + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);
			
			render_debug_vector(
				TRUE,
				&actor_debug_info->field_64,
				&actor_debug_info->field_70,
				2.f,
				actor_debug_info->field_60 ? global_real_argb_blue : global_real_argb_red);
			
			if (!actor_debug_info->field_60)
			{
				render_debug_sphere(TRUE, &actor_debug_info->field_7C, 0.2f, global_real_argb_red);
			}

			if (actor_debug_info->field_88)
			{
				render_debug_vector(
					TRUE,
					&actor->input.position.head_position,
					&actor_debug_info->field_98,
					1.f,
					global_real_argb_white);
				render_debug_vector(
					TRUE,
					&actor_debug_info->field_64,
					&actor_debug_info->field_8C,
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

			p1.x = actor->control.desired_aiming_vector.i;
			p1.y = actor->control.desired_aiming_vector.j;
			p1.z = actor->control.desired_aiming_vector.k;
			p0 = p1;

			p0.x = p1.x - 0.2f;
			p1.x = p1.x + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);
			
			p0.x = p0.x + 0.2f;
			p1.x = p1.x - 0.2f;
			p0.y = p0.y - 0.2f;
			p1.y = p1.y + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);

			p0.y = p0.y + 0.2f;
			p1.y = p1.y - 0.2f;
			p0.z = p0.z - 0.2f;
			p1.z = p1.z + 0.2f;
			render_debug_line(TRUE, &p0, &p1, global_real_argb_blue);
			
			render_debug_sphere(TRUE, &actor->control.burst_target, 0.1f, global_real_argb_red);
			render_debug_vector(TRUE, &actor->control.burst_target, &actor->control.burst_adjustment, 5.f, global_real_argb_red);
		}


		/*  Vision cones */

		if (render_exclusive && ai_debug.render_vision_cones)
		{
			real max_distance;
			real perception_factor;
			real angle_itr;
			real_point3d last_points[2][2][2];

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
				real_point3d direction_vector[2][2];
				real_point3d current_points[2][2][2];
				real full_distance_reference[2];
				real partial_distance_reference;

				short side_index;
				short ring_index;
				short height_index;

				real cosine_vertical_angle[2];
				real sine_vertical_angle[2];

				real horizontal_angle = actor_definition->perception.peripheral_vision_angle>angle_itr ? actor_definition->perception.peripheral_vision_angle : angle_itr;
				real cosine_horizontal_angle = cosine(horizontal_angle);
				real sine_horizontal_angle = sine(horizontal_angle);
				
				cosine_vertical_angle[0] = cosine(DEGREES_TO_RADIANS(50));
				sine_vertical_angle[0] = sine(DEGREES_TO_RADIANS(50));
				cosine_vertical_angle[1] = cosine(DEGREES_TO_RADIANS(45));
				sine_vertical_angle[1] = -sine(DEGREES_TO_RADIANS(45));

				for (side_index = 0; side_index<2; ++side_index)
				{
					for (ring_index = 0; ring_index<2; ++ring_index)
					{
						real_vector3d headspace_vector;
						headspace_vector.i = cosine_horizontal_angle * sine_vertical_angle[ring_index];
						headspace_vector.j = (((real)(side_index==0 ? 1 : -1)) * sine_horizontal_angle) * sine_vertical_angle[ring_index];
						headspace_vector.k = cosine_vertical_angle[ring_index];

						direction_vector[side_index][ring_index].x = global_zero_vector3d->i;
						direction_vector[side_index][ring_index].y = global_zero_vector3d->j;
						direction_vector[side_index][ring_index].z = global_zero_vector3d->k;

						point_from_line3d(&direction_vector[side_index][ring_index], &actor->input.looking_vector, headspace_vector.i, &direction_vector[side_index][ring_index]);
						point_from_line3d(&direction_vector[side_index][ring_index], &actor->input.looking_left_vector, headspace_vector.j, &direction_vector[side_index][ring_index]);
						point_from_line3d(&direction_vector[side_index][ring_index], &actor->input.looking_up_vector, headspace_vector.k, &direction_vector[side_index][ring_index]);
					}
				}

				actor_get_vision_distances(actor_index, max_distance, perception_factor, horizontal_angle, full_distance_reference, &partial_distance_reference);
			
				for (side_index = 0; side_index < 2; ++side_index)
				{
					for (ring_index = 0; ring_index < 2; ++ring_index)
					{
						for (height_index = 0; height_index < 2; ++height_index)
						{
							point_from_line3d(
								&actor->input.position.head_position,
								(real_vector3d *)&direction_vector[ring_index][height_index],
								full_distance_reference[side_index],
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
			struct prop_datum *prop;
			struct prop_iterator iterator;
			char buffer[1024];

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
					"full",
					NULL
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
				short step_index;
				short first_index;
				real_argb_color const *color;

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
					&actor->control.path.path.steps[actor->control.path.path.step_index].point,
					color,
					0.1f);

				for (
					step_index = first_index;
					step_index<actor->control.path.path.step_count;
					++step_index)
				{
					real_point3d position;

					if (step_index>first_index)
					{
						render_debug_line_offset(
							TRUE,
							// FIXME: whatever is going on here
							(const real_point3d *)(&actor->control.path.path.endpoint.surface_index + 4 * step_index),
							&actor->control.path.path.steps[step_index].point,
							color,
							0.1f);
					}
						
					point_from_line3d(&actor->control.path.path.steps[step_index].point, global_up3d, 0.1f, &position);
					render_debug_tick(TRUE, &position, global_up3d, 0.02, color);
				}

				{
					real_point3d endpoint;

					point_from_line3d(&actor->control.path.path.endpoint.point, global_up3d, 0.1f, &endpoint);
					render_debug_sphere(TRUE, &endpoint, 0.15f, color);
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
			if (path->path_state.destination_valid != 0)
			{
				render_debug_line_offset(
					TRUE,
					&path->path_state.input.start_point,
					&path->path_state.destination.point,
					global_real_argb_pink,
					0.1f);
				render_debug_point(TRUE, &path->path_state.destination.point, 0.3f, global_real_argb_green);

				if (path->path_state.destination.target_radius > 0.0)
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
			// TODO: finish
		}
	}

	return;
}
