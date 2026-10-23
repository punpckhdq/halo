/*
FIRST_PERSON_WEAPONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "first_person_weapons.h"
#include "meter_definitions.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "bitmaps_inlines.h"
#include "console.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "light_definitions.h"
#include "shaders.h"
#include "network_messages.h"
#include "rasterizer/common/rasterizer_common.h"
#include "sound_manager.h"
#include "weapon_interface_definitions.h"
#include "director.h"
#include "weapons.h"
#include "game_sound.h"
#include "bipeds.h"
#include "observer.h"
#include "effects.h"
#include "player_effects.h"
#include "predicted_resources.h"
#include "particles.h"

/* ---------- constants */

enum
{
	_first_person_weapon_state_idle = 0,
	_first_person_weapon_state_overheating,
	_first_person_weapon_state_overheating_again,
	_first_person_weapon_state_overheated,
	_first_person_weapon_state_charged,
	_first_person_weapon_state_posing,
	_first_person_weapon_state_primary_fire,
	_first_person_weapon_state_secondary_fire,
	_first_person_weapon_state_primary_misfire,
	_first_person_weapon_state_secondary_misfire,
	_first_person_weapon_state_melee,
	_first_person_weapon_state_light_on,
	_first_person_weapon_state_light_off,
	_first_person_weapon_state_reload_while_empty,
	_first_person_weapon_state_reload_while_full,
	_first_person_weapon_state_shotgun_enter_reload,
	_first_person_weapon_state_shotgun_exit_reload_empty,
	_first_person_weapon_state_shotgun_exit_reload_full,
	_first_person_weapon_state_put_away,
	_first_person_weapon_state_ready,
	_first_person_weapon_state_throw_grenade,
	_first_person_weapon_state_throw_grenade_overheated,
	_first_person_weapon_state_overheated_exit,
	_first_person_weapon_state_overheating_super_recoil,
	NUMBER_OF_FIRST_PERSON_WEAPON_STATES
};

enum
{
	_first_person_weapon_overlay_frame_forward = 0,		/* fake name */
	_first_person_weapon_overlay_frame_backward,		/* fake name */
	_first_person_weapon_overlay_frame_right,			/* fake name */
	_first_person_weapon_overlay_frame_left,			/* fake name */
	_first_person_weapon_overlay_frame_turn_left,		/* fake name */
	_first_person_weapon_overlay_frame_turn_right,		/* fake name */
	_first_person_weapon_overlay_frame_pitch_up,		/* fake name */
	_first_person_weapon_overlay_frame_pitch_down,		/* fake name */
	_first_person_weapon_overlay_frame_firing,			/* fake name */
	NUMBER_OF_FIRST_PERSON_WEAPON_OVERLAY_FRAMES
};

enum
{
	FIRST_PERSON_WEAPON_FIRING_INTERPOLATION_FRAMES = 3,	/* fake name */
	FIRST_PERSON_WEAPON_INTERPOLATION_FRAMES = 6,			/* fake name */
	FIRST_PERSON_WEAPON_RELOAD_SETTLE_TICKS = 44			/* fake name */
};

/* ---------- macros */

#define FIRST_PERSON_WEAPON_FIRING_PUSH_BACK_VELOCITY 0.05f	/* fake name */

/* ---------- structures */

struct first_person_weapon
{
	boolean visible;
	long unit_index;
	long weapon_index;
	short state;
	short ticks_until_pose;
	short ticks_idle;
	short ticks_until_predict;
	short overlay_animation_index;
	struct animation_state state_animation;
	struct animation_state moving_animation;
	struct animation_state_continuous overcharged_jitter_animation;
	real firing_push_back;
	real firing_push_back_velocity;
	real_vector2d position;
	real_vector2d position_velocity;
	real_vector2d turning;
	real_vector2d turning_velocity;
	boolean rendered;
	real_vector3d render_forward;
	real_euler_angles2d render_facing;
	real_euler_angles2d last_render_facing;
	real_point3d render_position;
	real_point3d last_render_position;
	short interpolation_frame_index;
	short interpolation_frame_count;
	real_orientation node_orientations[MAXIMUM_NODES_PER_ANIMATION];
	real_orientation original_node_orientations[MAXIMUM_NODES_PER_ANIMATION];
	real_matrix4x3 node_matrices[MAXIMUM_NODES_PER_ANIMATION];
	boolean weapon_node_remapping_table_valid;
	short weapon_node_remapping_table[MAXIMUM_NODES_PER_ANIMATION];
	boolean hands_node_remapping_table_valid;
	short hands_node_remapping_table[MAXIMUM_NODES_PER_ANIMATION];
	boolean shotgun_empty;
	short shotgun_shells_to_reload;
	short shotgun_reload_type;
	long current_sound_index;
	short current_sound_state;
};

/* ---------- prototypes */

static short first_person_weapon_state_from_weapon_message(short message_type);
static short first_person_animation_type_from_weapon_state(short state);
static void weapon_play_first_person_weapon_sound(long weapon_index, short message_type);
static void first_person_weapon_next_state(short local_player_index);
static void first_person_weapon_set_state(short local_player_index, short new_state, boolean reset_sounds);
static void first_person_weapon_start_interpolation(short local_player_index, short frame_count);
static void first_person_weapon_update(short local_player_index);
static void first_person_weapon_build_node_matrices(short local_player_index);
static struct first_person_weapon *first_person_weapon_get(short local_player_index);
static void first_person_weapon_switch_weapons(short local_player_index);
static void first_person_weapon_new_unit(short local_player_index, long unit_index);
static void first_person_weapon_set_visibility(short local_player_index, boolean visible);
static void model_remap_node_matrices_to_match_animation_graph(long model_index, real_matrix4x3 *model_node_matrices, long animation_graph_index, real_matrix4x3 const *animation_graph_node_matrices, short const *model_node_indices_to_animation_node_indices);
static boolean model_build_remapping_table_for_animation_graph(long model_index, long animation_graph_index, short *model_node_indices_to_animation_node_indices);
static short first_person_weapon_index_from_weapon_index(long weapon_index);
static short first_person_weapon_index_from_unit_index(long unit_index);
static void first_person_weapon_message(short local_player_index, short message_type);
static void first_person_weapon_predict(short local_player_index);

/* ---------- globals */

static struct first_person_weapon *first_person_weapons;

/* ---------- public code */

void first_person_weapons_initialize(
	void)
{
	first_person_weapons = game_state_malloc("first person weapons", NULL, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS * sizeof(*first_person_weapons));
	match_assert("c:\\halo\\SOURCE\\interface\\first_person_weapons.c", 240, first_person_weapons);

	return;
}

void first_person_weapons_dispose(
	void)
{
	return;
}

void first_person_weapons_initialize_for_new_map(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		memset(&first_person_weapons[local_player_index], 0, sizeof(first_person_weapons[local_player_index]));
		first_person_weapons[local_player_index].unit_index = NONE;
		first_person_weapons[local_player_index].current_sound_index = NONE;
		first_person_weapons[local_player_index].current_sound_state = NONE;
	}

	return;
}

void first_person_weapons_dispose_from_old_map(
	void)
{
	return;
}

void first_person_weapons_update(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE)
		{
			struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);
			long unit_index = player_get(player_index)->unit_index;

			if (first_person_weapon->unit_index != unit_index)
			{
				first_person_weapon_new_unit(local_player_index, unit_index);
			}

			if (first_person_weapon->weapon_index == NONE)
			{
				first_person_weapon_switch_weapons(local_player_index);
			}

			first_person_weapon_update(local_player_index);
		}
	}

	return;
}

void first_person_weapon_render_update(
	void)
{
	if (render.local_player_index != NONE)
	{
		struct first_person_weapon *first_person_weapon = first_person_weapon_get(render.local_player_index);

		if (first_person_weapon->unit_index != NONE && first_person_weapon->weapon_index != NONE)
		{
			boolean visible = director_get_perspective(render.local_player_index) == _director_perspective_first_person && player_control_get_zoom_level(render.local_player_index) == NONE;

			first_person_weapon_set_visibility(render.local_player_index, visible);

			if (first_person_weapon->visible)
			{
				first_person_weapon_build_node_matrices(render.local_player_index);
			}
		}
	}

	return;
}

void first_person_weapon_draw(
	void)
{
	if (render.local_player_index != NONE)
	{
		struct first_person_weapon *first_person_weapon = first_person_weapon_get(render.local_player_index);

		if (local_player_get_player_index(render.local_player_index) != NONE)
		{
			long unit_index = player_get(local_player_get_player_index(render.local_player_index))->unit_index;

			if (unit_index != NONE &&
				first_person_weapon->visible &&
				first_person_weapon->unit_index != NONE &&
				first_person_weapon->weapon_index != NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);
				struct weapon_datum *weapon = weapon_get(first_person_weapon->weapon_index);
				struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

				if (weapon_definition->weapon.interface_definition.first_person_animations.index != NONE)
				{
					real_matrix4x3 temporary_node_matrices[MAXIMUM_NODES_PER_ANIMATION];
					struct render_model_effect effect;
					struct game_globals_first_person_interface *first_person_interface = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->first_person_interface, 0, struct game_globals_first_person_interface);
					struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);
					struct render_lighting const *lighting = object_get_cached_render_lighting(unit_index, FLT_MAX);

					effect.modifier_shader = NULL;

					if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit) || unit->unit.active_camouflage > 0.f)
					{
						effect.type = _render_model_effect_type_active_camouflage;
						effect.intensity = unit->unit.active_camouflage;
						effect.parameter = unit->unit.active_camouflage_super_amount;
						effect.source_object_index = unit_index;
						effect.source_object_centroid = render.camera.position;
					}
					else
					{
						effect.type = _render_model_effect_type_none;
					}

					if (first_person_weapon->weapon_node_remapping_table_valid && weapon_definition->weapon.interface_definition.first_person_model.index != NONE)
					{
						model_remap_node_matrices_to_match_animation_graph(
							weapon_definition->weapon.interface_definition.first_person_model.index,
							temporary_node_matrices,
							weapon_definition->weapon.interface_definition.first_person_animations.index,
							first_person_weapon->node_matrices,
							first_person_weapon->weapon_node_remapping_table);
						render_model(
							weapon_definition->weapon.interface_definition.first_person_model.index,
							0.f,
							temporary_node_matrices,
							NULL,
							weapon->object.outgoing_change_colors,
							weapon->object.outgoing_function_values,
							lighting,
							&render.camera.position,
							0.f,
							&effect,
							first_person_weapon->weapon_index,
							0,
							FLAG(_render_model_first_person_bit));
					}

					if (first_person_weapon->hands_node_remapping_table_valid && first_person_interface->hands.index != NONE)
					{
						model_remap_node_matrices_to_match_animation_graph(
							first_person_interface->hands.index,
							temporary_node_matrices,
							weapon_definition->weapon.interface_definition.first_person_animations.index,
							first_person_weapon->node_matrices,
							first_person_weapon->hands_node_remapping_table);
						render_model(
							first_person_interface->hands.index,
							0.f,
							temporary_node_matrices,
							NULL,
							unit->object.outgoing_change_colors,
							unit->object.outgoing_function_values,
							lighting,
							&render.camera.position,
							0.f,
							&effect,
							first_person_weapon->weapon_index,
							0,
							FLAG(_render_model_first_person_bit));
					}
				}
			}
		}
	}

	return;
}

long first_person_weapon_get_local_index(
	long weapon_index)
{
	short local_player_index;
	long result;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

		if (first_person_weapon->weapon_index == weapon_index && first_person_weapon->visible)
		{
			break;
		}
	}

	if (local_player_index == MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
	{
		result = NONE;
	}
	else
	{
		result = local_player_index;
	}

	return result;
}

void first_person_weapon_message_from_unit(
	long unit_index,
	short message_type)
{
	short local_player_index = first_person_weapon_index_from_unit_index(unit_index);

	first_person_weapon_message(local_player_index, message_type);

	if (local_player_index == NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->unit.current_weapon_index != NONE)
		{
			weapon_play_first_person_weapon_sound(unit->unit.current_weapon_index, message_type);
		}
	}

	return;
}

void first_person_weapon_message_from_weapon(
	long weapon_index,
	short message_type)
{
	short local_player_index = first_person_weapon_index_from_weapon_index(weapon_index);

	first_person_weapon_message(local_player_index, message_type);

	if (local_player_index == NONE)
	{
		weapon_play_first_person_weapon_sound(weapon_index, message_type);
	}

	return;
}

static short first_person_weapon_state_from_weapon_message(
	short message_type)
{
	short state;

	switch (message_type)
	{
	case _first_person_weapon_message_primary_fire:
		state = _first_person_weapon_state_primary_fire;
		break;
	case _first_person_weapon_message_secondary_fire:
		state = _first_person_weapon_state_secondary_fire;
		break;
	case _first_person_weapon_message_primary_misfire:
		state = _first_person_weapon_state_primary_misfire;
		break;
	case _first_person_weapon_message_secondary_misfire:
		state = _first_person_weapon_state_secondary_misfire;
		break;
	case _first_person_weapon_message_melee:
		state = _first_person_weapon_state_melee;
		break;
	case _first_person_weapon_message_light_on:
		state = _first_person_weapon_state_light_on;
		break;
	case _first_person_weapon_message_light_off:
		state = _first_person_weapon_state_light_off;
		break;
	case _first_person_weapon_message_reload_while_empty:
		state = _first_person_weapon_state_reload_while_empty;
		break;
	case _first_person_weapon_message_reload_while_full:
		state = _first_person_weapon_state_reload_while_full;
		break;
	case _first_person_weapon_message_put_away:
		state = _first_person_weapon_state_put_away;
		break;
	case _first_person_weapon_message_ready:
		state = _first_person_weapon_state_ready;
		break;
	case _first_person_weapon_message_charged:
		state = _first_person_weapon_state_charged;
		break;
	case _first_person_weapon_message_overheating:
		state = _first_person_weapon_state_overheating;
		break;
	case _first_person_weapon_message_throw_grenade:
		state = _first_person_weapon_state_throw_grenade;
		break;
	case _first_person_weapon_message_overheating_super_recoil:
		state = _first_person_weapon_state_overheating_super_recoil;
		break;
	default:
		state = NONE;
		break;
	}

	return state;
}

static short first_person_animation_type_from_weapon_state(
	short state)
{
	short animation_type;

	switch (state)
	{
	case _first_person_weapon_state_idle:
		animation_type = _first_person_weapon_animation_idle;
		break;
	case _first_person_weapon_state_overheated:
		animation_type = _first_person_weapon_animation_overheated;
		break;
	case _first_person_weapon_state_charged:
		animation_type = _first_person_weapon_animation_overcharged;
		break;
	case _first_person_weapon_state_posing:
		animation_type = _first_person_weapon_animation_posing;
		break;
	case _first_person_weapon_state_primary_fire:
		animation_type = _first_person_weapon_animation_primary_fire;
		break;
	case _first_person_weapon_state_secondary_fire:
		animation_type = _first_person_weapon_animation_secondary_fire;
		break;
	case _first_person_weapon_state_primary_misfire:
		animation_type = _first_person_weapon_animation_primary_misfire;
		break;
	case _first_person_weapon_state_secondary_misfire:
		animation_type = _first_person_weapon_animation_secondary_misfire;
		break;
	case _first_person_weapon_state_melee:
		animation_type = _first_person_weapon_animation_melee;
		break;
	case _first_person_weapon_state_light_on:
		animation_type = _first_person_weapon_animation_light_on;
		break;
	case _first_person_weapon_state_light_off:
		animation_type = _first_person_weapon_animation_light_off;
		break;
	case _first_person_weapon_state_reload_while_empty:
		animation_type = _first_person_weapon_animation_reload_while_empty;
		break;
	case _first_person_weapon_state_reload_while_full:
		animation_type = _first_person_weapon_animation_reload_while_full;
		break;
	case _first_person_weapon_state_put_away:
		animation_type = _first_person_weapon_animation_put_away;
		break;
	case _first_person_weapon_state_ready:
		animation_type = _first_person_weapon_animation_ready;
		break;
	case _first_person_weapon_state_throw_grenade:
		animation_type = _first_person_weapon_animation_throw_grenade;
		break;
	case _first_person_weapon_state_throw_grenade_overheated:
		animation_type = _first_person_weapon_animation_throw_grenade_overheated;
		break;
	case _first_person_weapon_state_overheating:
		animation_type = _first_person_weapon_animation_overheating;
		break;
	case _first_person_weapon_state_overheating_again:
		animation_type = _first_person_weapon_animation_overheating_again;
		break;
	case _first_person_weapon_state_shotgun_enter_reload:
		animation_type = _first_person_weapon_animation_shotgun_enter;
		break;
	case _first_person_weapon_state_shotgun_exit_reload_empty:
		animation_type = _first_person_weapon_animation_shotgun_exit_empty;
		break;
	case _first_person_weapon_state_shotgun_exit_reload_full:
		animation_type = _first_person_weapon_animation_shotgun_exit_full;
		break;
	case _first_person_weapon_state_overheated_exit:
		animation_type = _first_person_weapon_animation_overheated_exit;
		break;
	case _first_person_weapon_state_overheating_super_recoil:
		animation_type = _first_person_weapon_animation_overheated_supercharge_enter;
		break;
	default:
		animation_type = NONE;
		break;
	}

	return animation_type;
}

static void weapon_play_first_person_weapon_sound(
	long weapon_index,
	short message_type)
{
	if (weapon_index != NONE && message_type != NONE && weapon_try_and_get(weapon_index))
	{
		struct weapon_definition *weapon_definition = weapon_definition_get(weapon_get(weapon_index)->definition_index);

		if (weapon_definition->weapon.interface_definition.first_person_animations.index != NONE)
		{
			short state = first_person_weapon_state_from_weapon_message(message_type);

			if (state != NONE)
			{
				short animation_type = first_person_animation_type_from_weapon_state(state);

				if (animation_type != NONE)
				{
					struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);
					struct animation_graph_first_person_weapon_animations *weapon_animations = !animation_graph->first_person_weapon_animations.count ? NULL : TAG_BLOCK_GET_ELEMENT(&animation_graph->first_person_weapon_animations, 0, struct animation_graph_first_person_weapon_animations);
					short animation_index = animation_type >= 0 && animation_type < weapon_animations->animations.count ? animation_graph_animation_index_get(&weapon_animations->animations)[animation_type].animation_index : NONE;

					if (animation_index != NONE)
					{
						short sound_index = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation)->sound_index;

						if (sound_index != NONE)
						{
							long sound_definition_index = TAG_BLOCK_GET_ELEMENT(&animation_graph->sound_references, sound_index, struct animation_graph_sound_reference)->sound.index;

							if (sound_definition_index != NONE)
							{
								object_impulse_sound_new(
									weapon_index,
									sound_definition_index,
									NONE,
									global_origin3d,
									global_forward3d,
									1.f);
							}
						}
					}
				}
			}
		}
	}

	return;
}

short first_person_weapon_get_marker_by_name_render(
	long weapon_index,
	char const *name,
	struct object_marker *markers,
	short maximum_marker_count)
{
	if (render.local_player_index == first_person_weapon_index_from_weapon_index(weapon_index))
	{
		return first_person_weapon_get_marker_by_name(weapon_index, name, markers, maximum_marker_count);
	}
	else
	{
		return 0;
	}
}

short first_person_weapon_get_marker_by_name(
	long weapon_index,
	char const *name,
	struct object_marker *markers,
	short maximum_marker_count)
{
	short marker_count = 0;
	struct weapon_datum *weapon = weapon_try_and_get(weapon_index);

	if (weapon)
	{
		short local_player_index = first_person_weapon_index_from_weapon_index(weapon_index);

		if (local_player_index != NONE && director_get_perspective(local_player_index) == _director_perspective_first_person)
		{
			struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

			if (first_person_weapon->weapon_node_remapping_table_valid &&
				weapon_definition->weapon.interface_definition.first_person_model.index != NONE &&
				weapon_definition->weapon.interface_definition.first_person_animations.index != NONE)
			{
				marker_count = model_get_marker_by_name(
					weapon_definition->weapon.interface_definition.first_person_model.index,
					name,
					NULL,
					first_person_weapon->weapon_node_remapping_table,
					(short)animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index)->nodes.count,
					first_person_weapon->node_matrices,
					FALSE,
					markers,
					maximum_marker_count);
			}
		}
	}

	return marker_count;
}

void first_person_weapon_center_flashlight(
	long unit_index,
	real_point3d *position,
	real_vector3d *forward,
	real_vector3d *up)
{
	short local_player_index = first_person_weapon_index_from_unit_index(unit_index);

	if (local_player_index != NONE)
	{
		struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

		if (first_person_weapon->visible)
		{
			struct object_marker marker;

			if (first_person_weapon_get_marker_by_name(first_person_weapon->weapon_index, "flashlight", &marker, 1) > 0)
			{
				point_from_line3d(&marker.matrix.position, &marker.matrix.forward, -0.5f, position);
				*forward = marker.matrix.forward;
				*up = marker.matrix.up;
			}
		}
	}

	return;
}

boolean first_person_weapon_adjust_light(
	long weapon_index,
	char const *marker_name,
	real_point3d *position,
	real_vector3d *forward,
	real_vector3d *up)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct unit_datum *unit = unit_get(weapon->object.parent_object_index);
	boolean result = FALSE;

	if (unit->unit.player_index != NONE)
	{
		struct player_datum *player = player_get(unit->unit.player_index);

		if (player->local_player_index != NONE && player->local_player_index == render.local_player_index)
		{
			struct first_person_weapon *first_person_weapon = first_person_weapon_get(player->local_player_index);

			if (first_person_weapon->visible)
			{
				struct object_marker marker;

				if (first_person_weapon_get_marker_by_name(weapon_index, marker_name, &marker, 1) > 0)
				{
					*position = marker.matrix.position;
					*forward = marker.matrix.forward;
					*up = marker.matrix.up;
					result = TRUE;
				}
			}
		}
	}

	return result;
}

real_matrix4x3 *first_person_weapon_get_node_matrix(
	short local_player_index,
	short node_index)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);
	struct weapon_datum *weapon = weapon_get(first_person_weapon->weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);

	match_assert("c:\\halo\\SOURCE\\interface\\first_person_weapons.c", 718, node_index>=0 && node_index<animation_graph->nodes.count);

	return &first_person_weapon->node_matrices[node_index];
}

static void first_person_weapon_next_state(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);
	short new_state = NONE;

	switch (first_person_weapon->state)
	{
	case _first_person_weapon_state_idle:
	case _first_person_weapon_state_posing:
	case _first_person_weapon_state_primary_fire:
	case _first_person_weapon_state_secondary_fire:
	case _first_person_weapon_state_primary_misfire:
	case _first_person_weapon_state_secondary_misfire:
	case _first_person_weapon_state_melee:
	case _first_person_weapon_state_light_on:
	case _first_person_weapon_state_light_off:
	case _first_person_weapon_state_shotgun_exit_reload_empty:
	case _first_person_weapon_state_shotgun_exit_reload_full:
	case _first_person_weapon_state_ready:
	case _first_person_weapon_state_throw_grenade:
	case _first_person_weapon_state_overheated_exit:
		new_state = _first_person_weapon_state_idle;
		break;
	case _first_person_weapon_state_overheating:
	case _first_person_weapon_state_overheating_again:
	case _first_person_weapon_state_throw_grenade_overheated:
	case _first_person_weapon_state_overheating_super_recoil:
		new_state = _first_person_weapon_state_overheated;
		break;
	case _first_person_weapon_state_put_away:
		first_person_weapon->state_animation.frame_index--;
		break;
	case _first_person_weapon_state_shotgun_enter_reload:
		if (weapon_definition_get(weapon_get(first_person_weapon->weapon_index)->definition_index)->weapon.weapon_type == _weapon_type_shotgun &&
			first_person_weapon->shotgun_reload_type == _shotgun_reload_type_first_and_last_round)
		{
			new_state = first_person_weapon->shotgun_empty ? _first_person_weapon_state_shotgun_exit_reload_empty : _first_person_weapon_state_shotgun_exit_reload_full;
		}
		else
		{
			new_state = _first_person_weapon_state_idle;
		}
		break;
	case _first_person_weapon_state_reload_while_empty:
	case _first_person_weapon_state_reload_while_full:
		if (weapon_definition_get(weapon_get(first_person_weapon->weapon_index)->definition_index)->weapon.weapon_type == _weapon_type_shotgun)
		{
			if (!first_person_weapon->shotgun_reload_type || first_person_weapon->shotgun_reload_type == NONE)
			{
				new_state = _first_person_weapon_state_idle;
			}
			else
			{
				match_assert("c:\\halo\\SOURCE\\interface\\first_person_weapons.c", 799, first_person_weapon->shotgun_reload_type == _shotgun_reload_type_first_and_last_round || first_person_weapon->shotgun_reload_type == _shotgun_reload_type_last_round);
				new_state = first_person_weapon->shotgun_empty ? _first_person_weapon_state_shotgun_exit_reload_empty : _first_person_weapon_state_shotgun_exit_reload_full;
			}
		}
		else
		{
			new_state = _first_person_weapon_state_idle;
		}
		break;
	}

	if (new_state != NONE)
	{
		first_person_weapon_set_state(local_player_index, new_state, FALSE);
	}

	return;
}

static void first_person_weapon_set_state(
	short local_player_index,
	short new_state,
	boolean reset_sounds)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

	if (first_person_weapon->weapon_index != NONE && TEST_FLAG(weapon_get(first_person_weapon->weapon_index)->weapon.flags, _weapon_overheated_bit))
	{
		switch (new_state)
		{
		case _first_person_weapon_state_ready:
			new_state = _first_person_weapon_state_overheating_again;
			break;
		case _first_person_weapon_state_throw_grenade:
			new_state = _first_person_weapon_state_throw_grenade_overheated;
			break;
		}
	}

	switch (new_state)
	{
	case _first_person_weapon_state_ready:
		if (first_person_weapon->state == _first_person_weapon_state_ready)
		{
			new_state = NONE;
		}
		break;
	case _first_person_weapon_state_light_on:
	case _first_person_weapon_state_light_off:
		if (first_person_weapon->state != _first_person_weapon_state_idle && first_person_weapon->state != _first_person_weapon_state_posing)
		{
			new_state = NONE;
		}
		break;
	case _first_person_weapon_state_primary_fire:
	case _first_person_weapon_state_secondary_fire:
	case _first_person_weapon_state_primary_misfire:
	case _first_person_weapon_state_secondary_misfire:
		if (first_person_weapon->state != _first_person_weapon_state_idle &&
			first_person_weapon->state != _first_person_weapon_state_posing &&
			first_person_weapon->state != _first_person_weapon_state_primary_fire &&
			first_person_weapon->state != _first_person_weapon_state_charged &&
			first_person_weapon->state != _first_person_weapon_state_shotgun_enter_reload &&
			first_person_weapon->state != _first_person_weapon_state_overheated_exit &&
			first_person_weapon->state != _first_person_weapon_state_shotgun_exit_reload_empty &&
			first_person_weapon->state != _first_person_weapon_state_shotgun_exit_reload_full &&
			first_person_weapon->state != _first_person_weapon_state_reload_while_empty &&
			first_person_weapon->state != _first_person_weapon_state_reload_while_full)
		{
			new_state = NONE;
		}
		break;
	}

	if (new_state != NONE && first_person_weapon->weapon_index != NONE)
	{
		short animation_type;
		short interpolation_frame_count;
		struct weapon_datum *weapon = weapon_get(first_person_weapon->weapon_index);
		struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

		if (weapon_definition->weapon.weapon_type == _weapon_type_plasma_pistol &&
			new_state == _first_person_weapon_state_overheated &&
			!TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit))
		{
			new_state = _first_person_weapon_state_idle;
		}

		animation_type = first_person_animation_type_from_weapon_state(new_state);

		if (weapon_definition->weapon.weapon_type == _weapon_type_shotgun && first_person_weapon->state == _first_person_weapon_state_shotgun_exit_reload_empty)
		{
			interpolation_frame_count = 0;
		}
		else
		{
			switch (new_state)
			{
			case _first_person_weapon_state_overheated:
			case _first_person_weapon_state_melee:
			case _first_person_weapon_state_ready:
				interpolation_frame_count = 0;
				break;
			case _first_person_weapon_state_primary_fire:
			case _first_person_weapon_state_secondary_fire:
			case _first_person_weapon_state_primary_misfire:
			case _first_person_weapon_state_secondary_misfire:
				interpolation_frame_count = FIRST_PERSON_WEAPON_FIRING_INTERPOLATION_FRAMES;
				break;
			default:
				interpolation_frame_count = FIRST_PERSON_WEAPON_INTERPOLATION_FRAMES;
				break;
			}
		}

		if (first_person_weapon->unit_index != NONE && first_person_weapon->weapon_index != NONE)
		{
			struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition_get(weapon_get(first_person_weapon->weapon_index)->definition_index)->weapon.interface_definition.first_person_animations.index);
			struct animation_graph_first_person_weapon_animations *weapon_animations = animation_graph->first_person_weapon_animations.count ? TAG_BLOCK_GET_ELEMENT(&animation_graph->first_person_weapon_animations, 0, struct animation_graph_first_person_weapon_animations) : NULL;

			if (weapon_animations)
			{
				short animation_index = animation_type >= 0 && animation_type < weapon_animations->animations.count ? animation_graph_animation_index_get(&weapon_animations->animations)[animation_type].animation_index : NONE;

				if (animation_index != NONE)
				{
					if (reset_sounds &&
						first_person_weapon->current_sound_index != NONE &&
						first_person_weapon->current_sound_state != _first_person_weapon_state_overheating)
					{
						sound_stop_impulse(first_person_weapon->current_sound_index);
						first_person_weapon->current_sound_index = NONE;
						first_person_weapon->current_sound_state = NONE;
					}

					if (interpolation_frame_count > 0)
					{
						first_person_weapon_start_interpolation(local_player_index, interpolation_frame_count);
					}

					first_person_weapon->state = new_state;
					first_person_weapon->state_animation.index = animation_index;
					first_person_weapon->state_animation.frame_index = 0;
				}
			}
		}
	}

	return;
}

static void first_person_weapon_start_interpolation(
	short local_player_index,
	short frame_count)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);
	struct weapon_datum *weapon = weapon_get(first_person_weapon->weapon_index);
	struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);

	memcpy(
		first_person_weapon->original_node_orientations,
		first_person_weapon->node_orientations,
		animation_graph->nodes.count * sizeof(real_orientation));

	if (frame_count >= first_person_weapon->interpolation_frame_count - first_person_weapon->interpolation_frame_index)
	{
		first_person_weapon->interpolation_frame_index = 0;
		first_person_weapon->interpolation_frame_count = frame_count;
	}

	return;
}

static void first_person_weapon_update(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

	if (first_person_weapon->weapon_index != NONE && !weapon_try_and_get(first_person_weapon->weapon_index))
	{
		error(
			_error_log,
			"local player %d, weapon (0x%x), deleted unexpectedly",
			local_player_index,
			first_person_weapon->weapon_index);
		first_person_weapon->weapon_index = NONE;
	}

	if (first_person_weapon->unit_index != NONE && first_person_weapon->weapon_index != NONE)
	{
		long sound_definition_index;
		short animation_update_result;
		boolean moving;
		struct unit_datum *unit = unit_get(first_person_weapon->unit_index);
		struct weapon_datum *weapon = weapon_get(first_person_weapon->weapon_index);
		struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
		struct model *model = model_definition_get(weapon_definition->weapon.interface_definition.first_person_model.index);
		struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);

		if (first_person_weapon->state == _first_person_weapon_state_overheated || first_person_weapon->state == _first_person_weapon_state_overheating)
		{
			if (TEST_FLAG(weapon->weapon.flags, _weapon_overheated_exit_bit))
			{
				first_person_weapon_set_state(local_player_index, _first_person_weapon_state_overheated_exit, TRUE);
			}

			if (!TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit))
			{
				first_person_weapon_set_state(local_player_index, _first_person_weapon_state_idle, TRUE);
			}
		}

		animation_update_result = animation_update_render_only(
			weapon_definition->weapon.interface_definition.first_person_animations.index,
			&first_person_weapon->state_animation,
			&sound_definition_index);

		if (animation_update_result != _animation_key_frame && animation_update_result == _animation_will_restart_on_next_frame)
		{
			first_person_weapon_next_state(local_player_index);
		}

		if (sound_definition_index != NONE && director_get_perspective(local_player_index) == _director_perspective_first_person)
		{
			first_person_weapon->current_sound_index = object_impulse_sound_new(
				first_person_weapon->weapon_index,
				sound_definition_index,
				NONE,
				global_origin3d,
				global_forward3d,
				1.f);
			first_person_weapon->current_sound_state = first_person_weapon->state;
		}

		moving = magnitude3d(&unit->unit.throttle) > 0.1f;

		if (unit_flying_through_air(first_person_weapon->unit_index))
		{
			moving = FALSE;
		}

		if (first_person_weapon->moving_animation.index != NONE)
		{
			animation_update_render_only(
				weapon_definition->weapon.interface_definition.first_person_animations.index,
				&first_person_weapon->moving_animation,
				NULL);

			if (!moving)
			{
				if (first_person_weapon->state == _first_person_weapon_state_idle)
				{
					first_person_weapon_start_interpolation(local_player_index, FIRST_PERSON_WEAPON_INTERPOLATION_FRAMES);
				}

				first_person_weapon->moving_animation.index = NONE;
			}
		}
		else if (moving)
		{
			struct animation_graph_first_person_weapon_animations *weapon_animations = !animation_graph->first_person_weapon_animations.count ? NULL : TAG_BLOCK_GET_ELEMENT(&animation_graph->first_person_weapon_animations, 0, struct animation_graph_first_person_weapon_animations);

			first_person_weapon->moving_animation.frame_index = 0;
			first_person_weapon->moving_animation.index = weapon_animations->animations.count > _first_person_weapon_animation_moving ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_moving].animation_index : NONE;
		}

		if (first_person_weapon->overcharged_jitter_animation.index == NONE)
		{
			if (first_person_weapon->state == _first_person_weapon_state_charged)
			{
				struct animation_graph_first_person_weapon_animations *weapon_animations = !animation_graph->first_person_weapon_animations.count ? NULL : TAG_BLOCK_GET_ELEMENT(&animation_graph->first_person_weapon_animations, 0, struct animation_graph_first_person_weapon_animations);

				first_person_weapon->overcharged_jitter_animation.frame_index = 0.f;
				first_person_weapon->overcharged_jitter_animation.index = weapon_animations->animations.count > _first_person_weapon_animation_overcharged_jitter ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_overcharged_jitter].animation_index : NONE;
			}
		}
		else if (first_person_weapon->state == _first_person_weapon_state_charged)
		{
			struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, first_person_weapon->overcharged_jitter_animation.index, struct animation);

			first_person_weapon->overcharged_jitter_animation.frame_index = (real)fmod(
				(weapon->weapon.overcharged + 1.f) * 2.f + first_person_weapon->overcharged_jitter_animation.frame_index,
				animation->frame_count);
		}
		else
		{
			first_person_weapon->overcharged_jitter_animation.index = NONE;
		}

		if (first_person_weapon->rendered)
		{
			real_vector2d turning;

			accelerate_to_position(
				&first_person_weapon->position.i,
				&first_person_weapon->position_velocity.i,
				unit->unit.throttle.i,
				0.08f,
				0.5f,
				-1.f,
				1.f,
				FALSE);
			accelerate_to_position(
				&first_person_weapon->position.j,
				&first_person_weapon->position_velocity.j,
				unit->unit.throttle.j,
				0.08f,
				0.5f,
				-1.f,
				1.f,
				FALSE);
			turning.i = signed_angular_difference(first_person_weapon->last_render_facing.yaw, first_person_weapon->render_facing.yaw) * TICKS_PER_SECOND;
			turning.j = signed_angular_difference(first_person_weapon->last_render_facing.pitch, first_person_weapon->render_facing.pitch) * -TICKS_PER_SECOND;
			turning.i = PIN(turning.i, -1.f, 1.f);
			turning.j = PIN(turning.j, -1.f, 1.f);
			accelerate_to_position(
				&first_person_weapon->turning.i,
				&first_person_weapon->turning_velocity.i,
				turning.i,
				0.03f,
				0.2f,
				-1.f,
				1.f,
				FALSE);
			accelerate_to_position(
				&first_person_weapon->turning.j,
				&first_person_weapon->turning_velocity.j,
				turning.j,
				0.03f,
				0.2f,
				-1.f,
				1.f,
				FALSE);
		}

		accelerate_to_position(
			&first_person_weapon->firing_push_back,
			&first_person_weapon->firing_push_back_velocity,
			0.f,
			0.01f,
			0.2f,
			0.f,
			1.f,
			FALSE);

		if (first_person_weapon->firing_push_back == 1.f)
		{
			first_person_weapon->firing_push_back_velocity = 0.f;
		}

		if (first_person_weapon->interpolation_frame_count > 0 && ++first_person_weapon->interpolation_frame_index >= first_person_weapon->interpolation_frame_count)
		{
			first_person_weapon->interpolation_frame_count = 0;
		}

		if (player_control_get_autoaim_level(local_player_index) == 0.f &&
			player_control_get_zoom_level(local_player_index) == NONE &&
			first_person_weapon->firing_push_back == 0.f &&
			first_person_weapon->position.i == 0.f &&
			first_person_weapon->position.j == 0.f &&
			first_person_weapon->turning.i == 0.f &&
			first_person_weapon->turning.j == 0.f)
		{
			if (first_person_weapon->state == _first_person_weapon_state_idle)
			{
				struct game_globals_player_information *player_information = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information);

				if (!first_person_weapon->ticks_until_pose)
				{
					first_person_weapon->ticks_until_pose = (short)(real_local_random_range(
						player_information->first_person_idle_time_lower_bound,
						player_information->first_person_idle_time_upper_bound) * TICKS_PER_SECOND);
				}

				if (++first_person_weapon->ticks_idle > first_person_weapon->ticks_until_pose)
				{
					first_person_weapon->ticks_until_pose = 0;

					if (real_local_random() >= player_information->first_person_idle_skip_fraction)
					{
						first_person_weapon_set_state(local_player_index, _first_person_weapon_state_posing, TRUE);
					}
				}
			}
			else
			{
				first_person_weapon->ticks_idle = 0;
			}
		}
		else
		{
			first_person_weapon->ticks_idle = 0;

			if (first_person_weapon->state == _first_person_weapon_state_posing)
			{
				first_person_weapon_set_state(local_player_index, _first_person_weapon_state_idle, TRUE);
			}
		}
	}

	if (--first_person_weapon->ticks_until_predict <= 0)
	{
		first_person_weapon_predict(local_player_index);
	}

	return;
}

static void first_person_weapon_build_node_matrices(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

	if (!first_person_weapon->rendered)
	{
		euler_angles2d_from_vector3d(&first_person_weapon->render_facing, &render.camera.forward);
		first_person_weapon->render_position = render.camera.position;
	}

	first_person_weapon->last_render_facing = first_person_weapon->render_facing;
	first_person_weapon->last_render_position = first_person_weapon->render_position;
	euler_angles2d_from_vector3d(&first_person_weapon->render_facing, &render.camera.forward);
	first_person_weapon->render_position = render.camera.position;
	first_person_weapon->render_forward = render.camera.forward;
	first_person_weapon->rendered = TRUE;

	if (first_person_weapon->weapon_index != NONE && !weapon_try_and_get(first_person_weapon->weapon_index))
	{
		error(
			_error_log,
			"local player %d, weapon (0x%x), deleted unexpectedly",
			local_player_index,
			first_person_weapon->weapon_index);
		first_person_weapon->weapon_index = NONE;
	}

	if (first_person_weapon->weapon_index != NONE)
	{
		struct weapon_datum *weapon = weapon_get(first_person_weapon->weapon_index);
		struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
		struct model *model = model_definition_get(weapon_definition->weapon.interface_definition.first_person_model.index);
		struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);
		struct animation_graph_first_person_weapon_animations *weapon_animations = animation_graph->first_person_weapon_animations.count ? TAG_BLOCK_GET_ELEMENT(&animation_graph->first_person_weapon_animations, 0, struct animation_graph_first_person_weapon_animations) : NULL;

		if (weapon_animations)
		{
			short ammunition_animation_index;
			short overlay_animation_index;

			if (first_person_weapon->state_animation.index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, first_person_weapon->state_animation.index, struct animation);

				animation_get_node_orientations(
					NULL,
					animation,
					first_person_weapon->state_animation.frame_index,
					first_person_weapon->node_orientations);
			}
			else
			{
				match_halt("c:\\halo\\SOURCE\\interface\\first_person_weapons.c", 1294);
				model_get_node_orientations(model, first_person_weapon->node_orientations);
			}

			ammunition_animation_index = weapon_animations->animations.count > _first_person_weapon_animation_ammunition ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_ammunition].animation_index : NONE;

			if (ammunition_animation_index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, ammunition_animation_index, struct animation);

				if (weapon_definition->weapon.weapon_type == _weapon_type_needler &&
					(first_person_weapons->state == _first_person_weapon_state_reload_while_empty || first_person_weapons->state == _first_person_weapon_state_reload_while_full))
				{
					short frame_index = weapon->weapon.magazines[0].rounds_loaded;
					short ticks = weapon->weapon.magazines[0].original_time - weapon->weapon.magazines[0].state_timer;

					if (ticks >= FIRST_PERSON_WEAPON_RELOAD_SETTLE_TICKS)
					{
						struct weapon_magazine_definition *magazine = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, 0, struct weapon_magazine_definition);
						real fraction = MIN((ticks - FIRST_PERSON_WEAPON_RELOAD_SETTLE_TICKS) * 0.2f, 1.f);
						short final_rounds_loaded = MIN(weapon->weapon.magazines[0].rounds_total, magazine->rounds_loaded_maximum);

						frame_index += (short)((final_rounds_loaded - frame_index) * fraction);
					}

					overlay_animation_apply(animation, frame_index, first_person_weapon->node_orientations);
				}
				else if (weapon->weapon.magazines[0].rounds_loaded < animation->frame_count)
				{
					overlay_animation_apply(
						animation,
						weapon->weapon.magazines[0].rounds_loaded,
						first_person_weapon->node_orientations);
				}
			}

			if (first_person_weapon->moving_animation.index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, first_person_weapon->moving_animation.index, struct animation);

				overlay_animation_apply(
					animation,
					first_person_weapon->moving_animation.frame_index,
					first_person_weapon->node_orientations);
			}

			if (first_person_weapon->overcharged_jitter_animation.index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, first_person_weapon->overcharged_jitter_animation.index, struct animation);

				overlay_animation_apply_continuous_scaled(
					animation,
					first_person_weapon->overcharged_jitter_animation.frame_index,
					weapon->weapon.overcharged + 0.5f,
					first_person_weapon->node_orientations);
			}

			overlay_animation_index = weapon_animations->animations.count > _first_person_weapon_animation_overlays ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_overlays].animation_index : NONE;

			if (overlay_animation_index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, overlay_animation_index, struct animation);

				if (animation->frame_count >= NUMBER_OF_FIRST_PERSON_WEAPON_OVERLAY_FRAMES)
				{
					if (first_person_weapon->position.i > 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_forward,
							first_person_weapon->position.i,
							first_person_weapon->node_orientations);
					}
					else if (first_person_weapon->position.i < 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_backward,
							-first_person_weapon->position.i,
							first_person_weapon->node_orientations);
					}

					if (first_person_weapon->position.j > 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_left,
							first_person_weapon->position.j,
							first_person_weapon->node_orientations);
					}
					else if (first_person_weapon->position.j < 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_right,
							-first_person_weapon->position.j,
							first_person_weapon->node_orientations);
					}

					if (first_person_weapon->turning.i > 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_turn_left,
							first_person_weapon->turning.i,
							first_person_weapon->node_orientations);
					}
					else if (first_person_weapon->turning.i < 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_turn_right,
							-first_person_weapon->turning.i,
							first_person_weapon->node_orientations);
					}

					if (first_person_weapon->turning.j > 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_pitch_down,
							first_person_weapon->turning.j,
							first_person_weapon->node_orientations);
					}
					else if (first_person_weapon->turning.j < 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_pitch_up,
							-first_person_weapon->turning.j,
							first_person_weapon->node_orientations);
					}

					if (first_person_weapon->firing_push_back > 0.f)
					{
						overlay_animation_apply_scaled(
							animation,
							_first_person_weapon_overlay_frame_firing,
							first_person_weapon->firing_push_back,
							first_person_weapon->node_orientations);
					}
				}
			}

			if (first_person_weapon->interpolation_frame_count > 0)
			{
				interpolate_node_orientations(
					(short)animation_graph->nodes.count,
					first_person_weapon->original_node_orientations,
					first_person_weapon->node_orientations,
					first_person_weapon->interpolation_frame_index,
					first_person_weapon->interpolation_frame_count);
			}
		}

		animation_graph_node_matrices_from_orientations(
			weapon_definition->weapon.interface_definition.first_person_animations.index,
			first_person_weapon->node_matrices,
			first_person_weapon->node_orientations,
			&render.camera.position,
			&render.camera.forward,
			&render.camera.up);
	}

	return;
}

static struct first_person_weapon *first_person_weapon_get(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\first_person_weapons.c", 1433, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &first_person_weapons[local_player_index];
}

static void first_person_weapon_switch_weapons(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);
	boolean visible = first_person_weapon->visible;

	first_person_weapon->weapon_index = NONE;

	if (visible)
	{
		first_person_weapon_set_visibility(local_player_index, FALSE);
	}

	if (first_person_weapon->unit_index != NONE)
	{
		long weapon_index = unit_inventory_get_weapon(first_person_weapon->unit_index, unit_get(first_person_weapon->unit_index)->unit.current_weapon_index);

		if (weapon_index != NONE)
		{
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon_get(weapon_index)->definition_index);

			if (weapon_definition->weapon.interface_definition.first_person_model.index != NONE && weapon_definition->weapon.interface_definition.first_person_animations.index != NONE)
			{
				struct animation_graph *animation_graph = animation_graph_definition_get(weapon_definition->weapon.interface_definition.first_person_animations.index);
				struct animation_graph_first_person_weapon_animations *weapon_animations = animation_graph->first_person_weapon_animations.count ? TAG_BLOCK_GET_ELEMENT(&animation_graph->first_person_weapon_animations, 0, struct animation_graph_first_person_weapon_animations) : NULL;

				if (weapon_animations)
				{
					short overlay_animation_index;
					struct game_globals_first_person_interface *first_person_interface;

					first_person_weapon->overlay_animation_index = NONE;
					overlay_animation_index = weapon_animations->animations.count > _first_person_weapon_animation_overlays ? animation_graph_animation_index_get(&weapon_animations->animations)[_first_person_weapon_animation_overlays].animation_index : NONE;

					if (overlay_animation_index != NONE && TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, overlay_animation_index, struct animation)->frame_count >= NUMBER_OF_FIRST_PERSON_WEAPON_OVERLAY_FRAMES)
					{
						first_person_weapon->overlay_animation_index = overlay_animation_index;
					}

					first_person_interface = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->first_person_interface, 0, struct game_globals_first_person_interface);

					if (first_person_interface->hands.index != NONE)
					{
						first_person_weapon->hands_node_remapping_table_valid = model_build_remapping_table_for_animation_graph(
							first_person_interface->hands.index,
							weapon_definition->weapon.interface_definition.first_person_animations.index,
							first_person_weapon->hands_node_remapping_table);
					}

					first_person_weapon->weapon_node_remapping_table_valid = model_build_remapping_table_for_animation_graph(
						weapon_definition->weapon.interface_definition.first_person_model.index,
						weapon_definition->weapon.interface_definition.first_person_animations.index,
						first_person_weapon->weapon_node_remapping_table);

					if (first_person_weapon->weapon_node_remapping_table_valid && first_person_weapon->hands_node_remapping_table_valid)
					{
						first_person_weapon->weapon_index = weapon_index;
						first_person_weapon->state = NONE;
						first_person_weapon->state_animation.index = NONE;
						first_person_weapon->moving_animation.index = NONE;
						first_person_weapon->overcharged_jitter_animation.index = NONE;
						first_person_weapon->firing_push_back = 0.f;
						first_person_weapon->firing_push_back_velocity = 0.f;
						first_person_weapon->ticks_idle = 0;
						first_person_weapon->current_sound_index = NONE;
						first_person_weapon->current_sound_state = NONE;
						first_person_weapon_set_state(local_player_index, _first_person_weapon_state_idle, TRUE);
						first_person_weapon->interpolation_frame_count = 0;

						if (visible)
						{
							first_person_weapon_set_visibility(local_player_index, TRUE);
						}
					}
				}
			}
		}
	}

	first_person_weapon_predict(local_player_index);

	return;
}

static void first_person_weapon_new_unit(
	short local_player_index,
	long unit_index)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

	first_person_weapon->rendered = FALSE;
	first_person_weapon->unit_index = unit_index;
	first_person_weapon_switch_weapons(local_player_index);

	return;
}

static void first_person_weapon_set_visibility(
	short local_player_index,
	boolean visible)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

	if (visible != first_person_weapon->visible)
	{
		if (visible)
		{
			match_assert("c:\\halo\\SOURCE\\interface\\first_person_weapons.c", 1555, first_person_weapon->weapon_index!=NONE);
			effects_start_on_first_person_weapon(local_player_index, first_person_weapon->weapon_index);
		}
		else
		{
			effects_stop_on_first_person_weapon(local_player_index);
			particles_stop_on_first_person_weapon(local_player_index);
		}

		first_person_weapon->visible = visible;
	}

	return;
}

static void model_remap_node_matrices_to_match_animation_graph(
	long model_index,
	real_matrix4x3 *model_node_matrices,
	long animation_graph_index,
	real_matrix4x3 const *animation_graph_node_matrices,
	short const *model_node_indices_to_animation_node_indices)
{
	short node_index;
	struct model *model = model_definition_get(model_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);

	for (node_index = 0; node_index < model->nodes.count; node_index++)
	{
		short animation_graph_node_index = model_node_indices_to_animation_node_indices[node_index];

		match_assert("c:\\halo\\SOURCE\\interface\\first_person_weapons.c", 1587, animation_graph_node_index>=0 && animation_graph_node_index<animation_graph->nodes.count);
		model_node_matrices[node_index] = animation_graph_node_matrices[animation_graph_node_index];
	}

	return;
}

static boolean model_build_remapping_table_for_animation_graph(
	long model_index,
	long animation_graph_index,
	short *model_node_indices_to_animation_node_indices)
{
	short model_node_index;
	struct model *model = model_definition_get(model_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);
	boolean result = TRUE;

	for (model_node_index = 0; model_node_index < model->nodes.count; model_node_index++)
	{
		short node_index;
		struct model_node *model_node = TAG_BLOCK_GET_ELEMENT(&model->nodes, model_node_index, struct model_node);
		short animation_graph_node_index = NONE;

		for (node_index = 0; node_index < animation_graph->nodes.count; node_index++)
		{
			struct animation_graph_node *animation_graph_node = TAG_BLOCK_GET_ELEMENT(&animation_graph->nodes, node_index, struct animation_graph_node);

			if (!strcmp(model_node->name, animation_graph_node->name))
			{
				animation_graph_node_index = node_index;
				break;
			}
		}

		if (animation_graph_node_index != NONE)
		{
			model_node_indices_to_animation_node_indices[model_node_index] = animation_graph_node_index;
		}
		else
		{
			result = FALSE;
		}
	}

	return result;
}

static short first_person_weapon_index_from_weapon_index(
	long weapon_index)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE)
		{
			long unit_index = player_get(player_index)->unit_index;

			if (unit_index != NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);

				if (unit->unit.current_weapon_index != NONE && weapon_index == unit->unit.weapon_object_indices[unit->unit.current_weapon_index])
				{
					return local_player_index;
				}
			}
		}
	}

	return NONE;
}

static short first_person_weapon_index_from_unit_index(
	long unit_index)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE && player_get(player_index)->unit_index == unit_index)
		{
			return local_player_index;
		}
	}

	return NONE;
}

static void first_person_weapon_message(
	short local_player_index,
	short message_type)
{
	if (local_player_index != NONE)
	{
		struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);
		short state = NONE;

		switch (message_type)
		{
		case _first_person_weapon_message_drop:
			first_person_weapon->weapon_index = NONE;
			break;
		case _first_person_weapon_message_ready:
			first_person_weapon_switch_weapons(local_player_index);
			break;
		case _first_person_weapon_message_primary_fire:
			first_person_weapon->firing_push_back_velocity += FIRST_PERSON_WEAPON_FIRING_PUSH_BACK_VELOCITY;
			break;
		case _first_person_weapon_message_reload_while_empty:
		case _first_person_weapon_message_reload_while_full:
			player_control_unzoom(first_person_weapon->unit_index);
			break;
		}

		if (first_person_weapon->weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(first_person_weapon->weapon_index);

			if (weapon->definition_index != NONE)
			{
				struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

				switch (weapon_definition->weapon.weapon_type)
				{
				case _weapon_type_shotgun:
					if (message_type == _first_person_weapon_message_reload_while_empty || message_type == _first_person_weapon_message_reload_while_full)
					{
						struct weapon_magazine_definition *magazine = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, 0, struct weapon_magazine_definition);
						short rounds_loaded = weapon->weapon.magazines[0].rounds_loaded;
						short rounds_total = weapon->weapon.magazines[0].rounds_total;

						if (first_person_weapon->state == _first_person_weapon_state_shotgun_enter_reload ||
							first_person_weapon->state == _first_person_weapon_state_overheated_exit ||
							first_person_weapon->state == _first_person_weapon_state_shotgun_exit_reload_empty ||
							first_person_weapon->state == _first_person_weapon_state_shotgun_exit_reload_full ||
							first_person_weapon->state == _first_person_weapon_state_reload_while_empty ||
							first_person_weapon->state == _first_person_weapon_state_reload_while_full ||
							weapon->weapon.magazines[0].state)
						{
							if (MIN(magazine->rounds_loaded_maximum - rounds_loaded, rounds_total) == 1)
							{
								first_person_weapon->shotgun_reload_type = _shotgun_reload_type_last_round;
							}
							else
							{
								first_person_weapon->shotgun_reload_type = NONE;
							}
						}
						else
						{
							first_person_weapon->shotgun_shells_to_reload = MIN(magazine->rounds_loaded_maximum - rounds_loaded, rounds_total);
							first_person_weapon->shotgun_empty = rounds_loaded == 0;
							first_person_weapon->shotgun_reload_type = first_person_weapon->shotgun_shells_to_reload == 1 ? _shotgun_reload_type_first_and_last_round : _shotgun_reload_type_first_round;
						}

						switch (first_person_weapon->shotgun_reload_type)
						{
						case _shotgun_reload_type_first_round:
						case _shotgun_reload_type_first_and_last_round:
							state = _first_person_weapon_state_shotgun_enter_reload;
							break;
						case NONE:
							state = _first_person_weapon_state_reload_while_empty;
							break;
						}
					}
					break;
				}
			}
		}

		if (state == NONE)
		{
			state = first_person_weapon_state_from_weapon_message(message_type);
		}

		if (state != NONE)
		{
			first_person_weapon_set_state(local_player_index, state, TRUE);
		}

		switch (message_type)
		{
		case _first_person_weapon_message_ready:
			first_person_weapon->interpolation_frame_count = 0;
			break;
		}
	}

	return;
}

static void first_person_weapon_predict(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon = first_person_weapon_get(local_player_index);

	if (first_person_weapon->weapon_index != NONE)
	{
		predicted_resources_precache(&weapon_definition_get(weapon_get(first_person_weapon->weapon_index)->definition_index)->weapon.predicted_resources);
	}

	first_person_weapon->ticks_until_predict = TICKS_PER_SECOND;

	return;
}
