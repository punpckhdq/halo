/*
ACTION_OBEY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "bipeds.h"
#include "hs.h"
#include "recorded_animations.h"
#include "recorded_animation_definitions.h"
#include "dialogue_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_COMMAND_LIST_LOOPS = 10, /* fake name */
	MAXIMUM_VEHICLE_POSSIBILITIES = 16 /* fake name */
};

enum
{
	_ai_atom_script_modifier_wait_for_finish = 0, /* fake name */
	_ai_atom_script_modifier_wake_and_continue, /* fake name */
	NUMBER_OF_AI_ATOM_SCRIPT_MODIFIERS /* fake name */
};

/* ---------- structures */

struct vehicle_possibility
{
	real distance_squared;
	long vehicle_index;
};

typedef void (*obey_individual_iterator)(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data); /* fake name */

/* ---------- prototypes */

static int vehicle_possibility_qsort(void const *p1, void const *p2);
static void action_obey_directmovement_update_facing(long actor_index, long unit_index, struct obey_individual_simple_control *simple_control);
static boolean action_obey_command_begin(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control);
static boolean action_obey_command_perform(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control);
static void action_obey_command_end(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, byte *next_command_index);
static void action_obey_individual_setup(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data);
static void action_obey_individual_perform(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data);
static void action_obey_individual_flush_command_indices(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data);
static void action_obey_individual_begin(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data);
static void action_obey_individual_advance(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data);
static void action_obey_individual_end(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data);
static void action_obey_individual_update(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control, void *user_data);
static void action_obey_individuals_iterate(long actor_index, boolean initialize_structures, struct obey_state_data *state_data, obey_individual_iterator iterator, void *user_data);

/* ---------- public code */

static int vehicle_possibility_qsort(
	void const *p1,
	void const *p2)
{
	int result;
	struct vehicle_possibility const *possibility1 = (struct vehicle_possibility const *)p1;
	struct vehicle_possibility const *possibility2 = (struct vehicle_possibility const *)p2;

	if (possibility1->distance_squared<possibility2->distance_squared)
	{
		result = -1;
	}
	else if (possibility1->distance_squared>possibility2->distance_squared)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}

	return result;
}

static void action_obey_directmovement_update_facing(
	long actor_index,
	long unit_index,
	struct obey_individual_simple_control *simple_control)
{
	real_vector3d forward;
	real_vector3d left;
	struct actor_datum *actor = actor_get(actor_index);

	if (unit_index==actor->meta.unit_index)
	{
		forward = actor->input.facing_vector;
	}
	else
	{
		unit_get_facing_vector(unit_index, &forward);
	}

	switch (simple_control->directmovement.facing)
	{
	case _actor_facing_forward:
		simple_control->directmovement.vector = forward;
		break;

	case _actor_facing_backward:
		negate_vector3d(&forward, &simple_control->directmovement.vector);
		break;

	case _actor_facing_left:
	case _actor_facing_right:
		cross_product3d(global_up3d, &forward, &left);

		if (normalize3d(&left)==0.f)
		{
			struct unit_datum *unit = unit_get(unit_index);

			cross_product3d(&unit->object.up, &forward, &left);

			if (normalize3d(&left)==0.f)
			{
				left = *global_forward3d;
			}
		}

		if (simple_control->directmovement.facing==_actor_facing_left)
		{
			simple_control->directmovement.vector = left;
		}
		else
		{
			negate_vector3d(&left, &simple_control->directmovement.vector);
		}

		break;
	}

	return;
}

static boolean action_obey_command_begin(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct actor_variant_definition *actor_variant = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_command_lists, command_list_index, struct ai_command_list_definition);
	boolean success = FALSE;

	if (simple_control->current_command_index<command_list->commands.count)
	{
		struct ai_command_definition *current_command = TAG_BLOCK_GET_ELEMENT(&command_list->commands, simple_control->current_command_index, struct ai_command_definition);
		short command_number = simple_control->current_command_index + 1;

		switch (current_command->atom_type)
		{
		case _ai_atom_pause:
			simple_control->pause_timer = (short)(current_command->parameter1 * TICKS_PER_SECOND);
			success = TRUE;
			break;

		case _ai_atom_go_to:
		case _ai_atom_go_to_and_face:
			if (complex_control)
			{
				if (current_command->point1_index>=0 && current_command->point1_index<command_list->points.count)
				{
					struct ai_command_point_definition *point = TAG_BLOCK_GET_ELEMENT(&command_list->points, current_command->point1_index, struct ai_command_point_definition);

					simple_control->pause_timer = 0;
					complex_control->destination_valid = TRUE;
					complex_control->destination_keep_moving = current_command->atom_modifier==_ai_atom_go_to_modifier_keep_moving;
					complex_control->destination_point = point->position;
					complex_control->destination_surface_index = point->surface_index;
					success = actor_move_to_point(
						actor_index,
						&complex_control->destination_point,
						complex_control->destination_surface_index,
						NONE);

					if (success)
					{
						if (complex_control->destination_keep_moving)
						{
							actor_move_keep_moving_past_destination(actor_index);
						}

						if (current_command->atom_type==_ai_atom_go_to_and_face &&
							current_command->point2_index>=0 &&
							current_command->point2_index<command_list->points.count)
						{
							struct ai_command_point_definition *facing_point = TAG_BLOCK_GET_ELEMENT(&command_list->points, current_command->point2_index, struct ai_command_point_definition);

							complex_control->destination_facing = TRUE;
							complex_control->destination_facing_point = facing_point->position;
						}
					}
				}
			}

			break;

		case _ai_atom_look:
		case _ai_atom_look_random:
		case _ai_atom_look_player:
		case _ai_atom_look_object:
			if (complex_control)
			{
				short point_index = NONE;
				long prop_index = NONE;
				long look_unit_index = NONE;
				real look_time = current_command->parameter1;

				if (current_command->atom_type==_ai_atom_look)
				{
					if (current_command->point1_index>=0 && current_command->point1_index<command_list->points.count)
					{
						point_index = current_command->point1_index;
						look_time = current_command->parameter1;
					}
				}
				else if (current_command->atom_type==_ai_atom_look_random)
				{
					if (current_command->point1_index>=0 &&
						current_command->point1_index<command_list->points.count &&
						current_command->point2_index>=0 &&
						current_command->point2_index<command_list->points.count)
					{
						point_index = random_range(current_command->point1_index, current_command->point2_index + 1);

						if (current_command->parameter1==0.f && current_command->parameter2==0.f)
						{
							look_time = real_random_range(
								actor_definition->looking.idle_look_noncombat.idle_look_time_lower_bound,
								actor_definition->looking.idle_look_noncombat.idle_look_time_upper_bound);
						}
						else
						{
							look_time = real_random_range(current_command->parameter1, current_command->parameter2);
						}
					}
				}
				else if (current_command->atom_type==_ai_atom_look_player)
				{
					{
						struct prop_iterator iterator;
						struct prop_datum *prop;
						real best_distance = REAL_MAX;

						prop_iterator_new(&iterator, actor_index);

						while (prop = prop_iterator_next(&iterator))
						{
							if (prop->state>=_prop_state_becoming_unacknowledged &&
								prop->state<=_prop_state_acknowledged &&
								prop->player &&
								prop->distance<best_distance)
							{
								prop_index = iterator.index;
								best_distance = prop->distance;
							}
						}
					}

					if (prop_index==NONE)
					{
						struct data_iterator iterator;
						struct player_datum *player;
						real best_distance_squared = REAL_MAX;

						data_iterator_new(&iterator, player_data);

						while (player = data_iterator_next(&iterator))
						{
							if (player->unit_index!=NONE)
							{
								real_point3d head_position;
								real distance_squared;

								unit_get_head_position(player->unit_index, &head_position);
								distance_squared = distance_squared3d(&head_position, &actor->input.position.head_position);

								if (distance_squared<best_distance_squared)
								{
									look_unit_index = player->unit_index;
									best_distance_squared = distance_squared;
								}
							}
						}
					}
				}
				else if (current_command->atom_type==_ai_atom_look_object)
				{
					struct scenario *scenario = global_scenario_get();

					if (current_command->object_name_index>=0 && current_command->object_name_index<scenario->object_names.count)
					{
						long object_index = object_index_from_name_index(current_command->object_name_index);

						if (unit_try_and_get(object_index))
						{
							prop_index = prop_get_active_by_unit_index(actor_index, object_index);
							look_unit_index = object_index;
						}
					}
				}

				if (look_time>0.f &&
					(prop_index!=NONE || look_unit_index!=NONE || (point_index>=0 && point_index<command_list->points.count)))
				{
					struct direction_specification direction;
					short priority = _secondary_look_priority_default;

					if (current_command->atom_modifier==_ai_atom_look_modifier_idle_turn_around)
					{
						priority = _secondary_look_priority_turn_and_aim;
					}
					else if (current_command->atom_modifier==_ai_atom_look_modifier_idle_look)
					{
						priority = _secondary_look_priority_idle_look;
					}
					else if (current_command->atom_modifier==_ai_atom_look_modifier_force_aim_weapon)
					{
						priority = _secondary_look_priority_override;
					}
					else if (current_command->atom_modifier==_ai_atom_look_modifier_force_facing)
					{
						priority = _secondary_look_priority_override_exact_facing;
					}

					if (prop_index!=NONE)
					{
						direction.type = _direction_specification_prop;
						direction.prop_index = prop_index;
					}
					else if (look_unit_index!=NONE)
					{
						direction.type = _direction_specification_point;
						unit_get_head_position(look_unit_index, &direction.point);
					}
					else
					{
						struct ai_command_point_definition *point = TAG_BLOCK_GET_ELEMENT(&command_list->points, point_index, struct ai_command_point_definition);

						direction.type = _direction_specification_point;
						direction.point = point->position;
					}

					actor_look_secondary(actor_index, _secondary_look_scripted, priority, &direction);
					simple_control->pause_timer = (short)(look_time * TICKS_PER_SECOND);
					success = TRUE;
				}
			}

			break;

		case _ai_atom_move_direction:
			if (unit_index==actor->meta.unit_index)
			{
				simple_control->directmovement.start_position = actor->input.position.body_position;
			}
			else
			{
				object_get_origin(unit_index, &simple_control->directmovement.start_position);
			}

			if (current_command->point1_index>=0 && current_command->point1_index<command_list->points.count)
			{
				struct ai_command_point_definition *point = TAG_BLOCK_GET_ELEMENT(&command_list->points, current_command->point1_index, struct ai_command_point_definition);

				vector_from_points3d(
					&simple_control->directmovement.start_position,
					&point->position,
					&simple_control->directmovement.vector);
				success = normalize3d(&simple_control->directmovement.vector)>0.f;
			}
			else if (current_command->parameter2>=0.f && current_command->parameter2<360.f)
			{
				vector3d_from_angle(&simple_control->directmovement.vector, current_command->parameter2 * (_pi / 180.f));
				success = TRUE;
			}

			if (success)
			{
				if (current_command->atom_modifier>=0 && current_command->atom_modifier<=_ai_atom_move_immediate_modifier_backward)
				{
					simple_control->directmovement.facing = current_command->atom_modifier;
				}
				else
				{
					simple_control->directmovement.facing = NONE;
				}

				if (unit_index==actor->meta.unit_index)
				{
					actor_move_halt(actor_index);
				}

				SET_FLAG(simple_control->simple_control_flags, _obey_simple_directmovement_bit, TRUE);
				SET_FLAG(simple_control->simple_control_flags, _obey_simple_directmovement_update_continuously_bit, FALSE);
			}

			break;

		case _ai_atom_move_immediate:
			if (current_command->atom_modifier>=0 && current_command->atom_modifier<=_ai_atom_move_immediate_modifier_backward)
			{
				simple_control->directmovement.facing = current_command->atom_modifier;
			}
			else
			{
				simple_control->directmovement.facing = _actor_facing_forward;
			}

			action_obey_directmovement_update_facing(actor_index, unit_index, simple_control);
			simple_control->pause_timer = (short)(current_command->parameter1 * TICKS_PER_SECOND);
			SET_FLAG(simple_control->simple_control_flags, _obey_simple_directmovement_bit, TRUE);
			SET_FLAG(simple_control->simple_control_flags, _obey_simple_directmovement_update_continuously_bit, TRUE);
			success = TRUE;
			break;

		case _ai_atom_running_jump:
			if (unit_index!=actor->meta.unit_index || actor->input.vehicle_index==NONE)
			{
				boolean moving_forward;

				SET_FLAG(simple_control->simple_control_flags, _obey_simple_jump_bit, TRUE);
				SET_FLAG(simple_control->simple_control_flags, _obey_simple_jump_jumped_bit, FALSE);
				SET_FLAG(simple_control->simple_control_flags, _obey_simple_jump_targeted_bit, FALSE);

				if (unit_index==actor->meta.unit_index)
				{
					moving_forward = actor->control.moving && actor->control.moving_facing_direction==_actor_facing_forward;
				}
				else
				{
					struct unit_datum *unit = unit_get(unit_index);

					moving_forward = unit->object.parent_object_index==NONE &&
						dot_product3d(&unit->object.translational_velocity, &unit->object.forward)>2.f / TICKS_PER_SECOND;
				}

				simple_control->jump.delay_ticks = moving_forward ? 0 : 10;
				simple_control->pause_timer = 60;
				success = TRUE;
			}

			break;

		case _ai_atom_targeted_jump:
			if (unit_index!=actor->meta.unit_index || actor->input.vehicle_index==NONE)
			{
				SET_FLAG(simple_control->simple_control_flags, _obey_simple_jump_bit, TRUE);
				SET_FLAG(simple_control->simple_control_flags, _obey_simple_jump_jumped_bit, FALSE);
				SET_FLAG(simple_control->simple_control_flags, _obey_simple_jump_targeted_bit, TRUE);
				simple_control->jump.delay_ticks = 0;
				simple_control->jump.target_horizontal_vel = current_command->parameter1;
				simple_control->jump.target_vertical_vel = current_command->parameter2;
				simple_control->pause_timer = 60;
				success = TRUE;
			}

			break;

		case _ai_atom_animation_mode:
			if (complex_control &&
				current_command->atom_modifier>=0 &&
				current_command->atom_modifier<NUMBER_OF_AI_ATOM_ANIMATION_MODE_MODIFIERS)
			{
				complex_control->override_movement_type = current_command->atom_modifier;
				success = TRUE;
			}

			break;

		case _ai_atom_crouch:
			if (complex_control)
			{
				complex_control->override_crouch = current_command->atom_modifier==_ai_atom_crouch_modifier_enable;
				success = TRUE;
			}

			break;

		case _ai_atom_targeting:
			SET_FLAG(
				simple_control->metadata_flags,
				_obey_metadata_targeting_bit,
				current_command->atom_modifier==_ai_atom_targeting_modifier_enable);
			success = TRUE;
			break;

		case _ai_atom_initiative:
			if (unit_index==actor->meta.unit_index)
			{
				struct obey_state_data *state_data = &actor->state.action_data.obey;

				state_data->initiative = current_command->atom_modifier==_ai_atom_initiative_modifier_enable;
				success = TRUE;
			}

			break;

		case _ai_atom_set_radius:
			if (complex_control && current_command->parameter1>0.f)
			{
				complex_control->destination_radius_valid = TRUE;
				complex_control->destination_radius = current_command->parameter1;
				success = TRUE;
			}

			break;

		case _ai_atom_action:
			if (complex_control)
			{
				complex_control->play_action = FALSE;

				switch (current_command->atom_modifier)
				{
				case _ai_atom_action_modifier_berserk:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_berserk;
					complex_control->action_communication_type = _ai_communication_berserk;
					break;

				case _ai_atom_action_modifier_surprise_front:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_surprise_front;
					complex_control->action_communication_type = _ai_communication_surprise;
					break;

				case _ai_atom_action_modifier_surprise_back:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_surprise_back;
					complex_control->action_communication_type = _ai_communication_surprise;
					break;

				case _ai_atom_action_modifier_evade_left:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_evade_left;
					complex_control->action_communication_type = NONE;
					break;

				case _ai_atom_action_modifier_evade_right:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_evade_right;
					complex_control->action_communication_type = NONE;
					break;

				case _ai_atom_action_modifier_dive_forward:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_dive_front;
					complex_control->action_communication_type = _ai_communication_dive;
					break;

				case _ai_atom_action_modifier_dive_back:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_dive_back;
					complex_control->action_communication_type = _ai_communication_dive;
					break;

				case _ai_atom_action_modifier_dive_left:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_dive_left;
					complex_control->action_communication_type = _ai_communication_dive;
					break;

				case _ai_atom_action_modifier_dive_right:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = _unit_animation_impulse_dive_right;
					complex_control->action_communication_type = _ai_communication_dive;
					break;

				case _ai_atom_action_modifier_vehicle_woohoo:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = NONE;
					complex_control->action_communication_type = _ai_communication_vehicle_woohoo;
					break;

				case _ai_atom_action_modifier_vehicle_scared:
					complex_control->play_action = TRUE;
					complex_control->action_animation_impulse = NONE;
					complex_control->action_communication_type = _ai_communication_vehicle_scared;
					break;
				}

				success = complex_control->play_action;
			}

			break;

		case _ai_atom_shoot:
			if (complex_control && current_command->point1_index>=0 && current_command->point1_index<command_list->points.count)
			{
				struct ai_command_point_definition *point = TAG_BLOCK_GET_ELEMENT(&command_list->points, current_command->point1_index, struct ai_command_point_definition);

				complex_control->shoot_at_target = TRUE;
				complex_control->shoot_target = point->position;
				complex_control->shoot_burst_length = current_command->parameter1;
				success = TRUE;
			}

			break;

		case _ai_atom_grenade:
			if (complex_control &&
				actor_variant->grenade_combat.grenade_type!=NONE &&
				current_command->point1_index>=0 &&
				current_command->point1_index<command_list->points.count)
			{
				struct ai_command_point_definition *point = TAG_BLOCK_GET_ELEMENT(&command_list->points, current_command->point1_index, struct ai_command_point_definition);

				unit_add_grenade_type_to_inventory(actor->meta.unit_index, actor_variant->grenade_combat.grenade_type, 1);
				complex_control->grenade_throw_started = FALSE;
				complex_control->grenade_throw_depress_trigger = FALSE;
				complex_control->grenade_target = point->position;
				complex_control->grenade_throw_trajectory_type = _actor_atom_grenade_modifier_toss;

				if (current_command->atom_modifier>=0 && current_command->atom_modifier<NUMBER_OF_AI_ATOM_GRENADE_MODIFIERS)
				{
					complex_control->grenade_throw_trajectory_type = current_command->atom_modifier;
				}

				simple_control->pause_timer = 60;
				success = TRUE;
			}

			break;

		case _ai_atom_vehicle:
			if (unit_index==actor->meta.unit_index)
			{
				short possibility_index;
				struct vehicle_possibility possibilities[MAXIMUM_VEHICLE_POSSIBILITIES];
				struct object_iterator iterator;
				short seat_desire_type = NONE;
				short possibility_count = 0;

				object_iterator_new(&iterator, _object_mask_vehicle, 0);

				while (object_iterator_next(&iterator))
				{
					real_point3d vehicle_origin;
					real distance_squared;

					object_get_origin(iterator.index, &vehicle_origin);
					distance_squared = distance_squared3d(&actor->input.position.body_position, &vehicle_origin);

					if (current_command->parameter1==0.f || distance_squared<current_command->parameter1 * current_command->parameter1)
					{
						possibilities[possibility_count].distance_squared = distance_squared;
						possibilities[possibility_count].vehicle_index = iterator.index;
						possibility_count++;

						if (possibility_count>=NUMBEROF(possibilities))
						{
							break;
						}
					}
				}

				if (possibility_count>1)
				{
					qsort(possibilities, possibility_count, sizeof(possibilities[0]), vehicle_possibility_qsort);
				}

				if (current_command->atom_modifier>=0 && current_command->atom_modifier<NUMBER_OF_AI_ATOM_VEHICLE_MODIFIERS)
				{
					seat_desire_type = current_command->atom_modifier;
				}

				for (possibility_index = 0; possibility_index<possibility_count; possibility_index++)
				{
					if (actor_action_try_to_enter_vehicle(
						actor_index,
						possibilities[possibility_index].vehicle_index,
						"",
						seat_desire_type,
						0,
						NULL))
					{
						SET_FLAG(simple_control->metadata_flags, _obey_metadata_action_changed_bit, TRUE);
						success = TRUE;
						break;
					}
				}
			}

			break;

		case _ai_atom_animate:
			if (current_command->animation_reference_index!=NONE)
			{
				struct ai_animation_reference_definition *animation_reference = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_animation_references, current_command->animation_reference_index, struct ai_animation_reference_definition);
				long animation_graph_index = animation_reference->animation_graph.index;
				boolean absolute_movement = FALSE;
				boolean no_collision = FALSE;
				boolean interpolate = TRUE;

				if (animation_graph_index==NONE)
				{
					struct unit_datum *unit = unit_get(unit_index);
					struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

					animation_graph_index = unit_definition->object.animation_graph.index;
				}

				switch (current_command->atom_modifier)
				{
				case _ai_atom_animate_modifier_absolute_movement:
					absolute_movement = TRUE;
					break;

				case _ai_atom_animate_modifier_absolute_movement_no_collision:
					absolute_movement = TRUE;
					no_collision = TRUE;
					break;

				case _ai_atom_animate_modifier_no_interpolation_relative_movement:
					interpolate = FALSE;
					break;

				case _ai_atom_animate_modifier_no_interpolation_absolute_movement:
					interpolate = FALSE;
					absolute_movement = TRUE;
					break;

				case _ai_atom_animate_modifier_no_interpolation_absolute_movement_no_collision:
					interpolate = FALSE;
					absolute_movement = TRUE;
					no_collision = TRUE;
					break;
				}

				if (unit_start_user_animation(unit_index, animation_graph_index, animation_reference->animation_name, interpolate))
				{
					struct biped_datum *biped = biped_try_and_get(unit_index);

					if (biped)
					{
						SET_FLAG(biped->biped.flags, _biped_absolute_movement_bit, absolute_movement);
						SET_FLAG(biped->biped.flags, _biped_no_collision_bit, no_collision);
					}

					success = TRUE;
				}
			}

			break;

		case _ai_atom_recording:
			if (current_command->recording_reference_index>=0 &&
				current_command->recording_reference_index<global_scenario_get()->ai_recording_references.count)
			{
				struct ai_recording_reference_definition *recording_reference = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_recording_references, current_command->recording_reference_index, struct ai_recording_reference_definition);
				short animation_index = scenario_get_animation_by_name(global_scenario_get(), recording_reference->recording_name);

				if (animation_index!=NONE)
				{
					success = recorded_animation_play(unit_index, animation_index);
				}
			}

			break;

		case _ai_atom_script:
			if (current_command->script_reference_index>=0 &&
				current_command->script_reference_index<global_scenario_get()->ai_script_references.count)
			{
				struct ai_script_reference_definition *script_reference = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_script_references, current_command->script_reference_index, struct ai_script_reference_definition);

				success = hs_wake_by_name(script_reference->script_name);
			}

			break;

		case _ai_atom_wait:
			success = TRUE;
			break;

		case _ai_atom_loop:
			if (current_command->command_index>=0 &&
				current_command->command_index<command_list->commands.count &&
				current_command->command_index!=simple_control->current_command_index)
			{
				success = TRUE;
			}

			break;

		case _ai_atom_die:
		{
			struct unit_datum *unit = unit_get(unit_index);

			if (current_command->atom_modifier==_ai_atom_die_modifier_silent)
			{
				SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_silent_bit, TRUE);
			}
			else
			{
				SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_bit, TRUE);
			}

			success = TRUE;
			break;
		}

		case _ai_atom_vocalize:
		{
			short vocalization_type = current_command->atom_modifier;
			long sound_definition_index = NONE;
			short play_type = unit_test_speech(
				unit_index,
				_unit_speech_scripted,
				TRUE,
				TRUE,
				NULL,
				&vocalization_type,
				&sound_definition_index);

			if (play_type>_unit_play_speech_none)
			{
				struct unit_speech_item speech;

				memset(&speech, 0, sizeof(speech));
				speech.vocalization_type = vocalization_type;
				speech.sound_definition_index = sound_definition_index;
				speech.priority = _unit_speech_scripted;
				ai_communication_packet_new(&speech.ai);
				unit_speak(unit_index, play_type, &speech);
				success = TRUE;
			}

			break;
		}

		case _ai_atom_teleport:
			if (current_command->point1_index>=0 && current_command->point1_index<command_list->points.count)
			{
				real_vector3d forward_vector;
				struct ai_command_point_definition *point = TAG_BLOCK_GET_ELEMENT(&command_list->points, current_command->point1_index, struct ai_command_point_definition);

				unit_get_facing_vector(unit_index, &forward_vector);

				if (current_command->point2_index>=0 && current_command->point2_index<command_list->points.count)
				{
					struct ai_command_point_definition *facing_point = TAG_BLOCK_GET_ELEMENT(&command_list->points, current_command->point2_index, struct ai_command_point_definition);
					struct biped_datum *biped = biped_try_and_get(unit_index);
					struct biped_definition *biped_definition = (biped==NULL) ? NULL : biped_definition_get(biped->definition_index);

					vector_from_points3d(&point->position, &facing_point->position, &forward_vector);

					if (biped_definition &&
						(TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit) ||
						TEST_FLAG(biped_definition->biped.flags, _biped_climbs_anything_bit)))
					{
						if (normalize3d(&forward_vector)==0.f)
						{
							unit_get_facing_vector(unit_index, &forward_vector);
						}
					}
					else
					{
						forward_vector.k = 0.f;

						if (normalize2d((real_vector2d *)&forward_vector)==0.f)
						{
							unit_get_facing_vector(unit_index, &forward_vector);
						}
					}
				}

				object_set_position(unit_index, &point->position, &forward_vector, NULL);
				object_reset(unit_index);
				object_compute_node_matrices_recursive(unit_index);

				if (unit_index==actor->meta.unit_index)
				{
					actor_input_sample_position(actor_index, actor->meta.unit_index, &actor->input.position);
					actor_move_halt(actor_index);
				}

				success = TRUE;
			}

			break;
		}

		if (ai_debug.print_command_lists)
		{
			char encounterbuf[256];

			if (actor->meta.encounter_index==NONE)
			{
				strcpy(encounterbuf, "<no encounter>");
			}
			else
			{
				struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index), struct encounter_definition);
				struct squad_definition *squad = TAG_BLOCK_GET_ELEMENT(&encounter->squads, actor->meta.squad_index, struct squad_definition);

				sprintf(encounterbuf, "%s/%s", encounter->name, squad->name);
			}

			action_obey_describe_command(global_scenario_get(), current_command, temporary, sizeof(temporary));
			error(
				_error_silent,
				"%s: %s #%d%s: %s",
				encounterbuf,
				command_list->name,
				command_number,
				success ? "" : " FAILED",
				temporary);
		}
	}

	return success;
}

static boolean action_obey_command_perform(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_command_lists, command_list_index, struct ai_command_list_definition);
	boolean finished = TRUE;

	if (simple_control->current_command_index<command_list->commands.count)
	{
		struct ai_command_definition *current_command = TAG_BLOCK_GET_ELEMENT(&command_list->commands, simple_control->current_command_index, struct ai_command_definition);

		switch (current_command->atom_type)
		{
		case _ai_atom_pause:
		case _ai_atom_look:
		case _ai_atom_look_random:
		case _ai_atom_look_player:
		case _ai_atom_look_object:
			finished = simple_control->pause_timer==0;
			break;

		case _ai_atom_go_to:
		case _ai_atom_go_to_and_face:
			if (unit_index==actor->meta.unit_index && complex_control)
			{
				finished = actor_path_at_destination(actor_index);

				if (!finished && complex_control->destination_keep_moving && complex_control->destination_valid)
				{
					real_vector3d direction_to_point;
					real distance_squared;
					real tolerance = actor_destination_tolerance(actor_index);

					vector_from_points3d(&actor->input.position.body_position, &complex_control->destination_point, &direction_to_point);
					distance_squared = magnitude_squared3d(&direction_to_point);

					if (distance_squared<tolerance * tolerance)
					{
						finished = TRUE;
					}
					else if (distance_squared<(tolerance + 0.5f) * (tolerance + 0.5f))
					{
						struct unit_datum *unit = unit_get(actor->meta.unit_index);

						if (dot_product3d(&unit->object.translational_velocity, &direction_to_point)<0.f)
						{
							finished = TRUE;
						}
					}
				}

				if (current_command->atom_type==_ai_atom_go_to_and_face ||
					current_command->atom_modifier==_ai_atom_go_to_modifier_stop_at_point)
				{
					if (actor->control.moving)
					{
						simple_control->pause_timer = 10;
					}

					finished = finished && simple_control->pause_timer==0;
				}

				if (finished && complex_control->destination_facing)
				{
					real facing_dot = 1.f;

					match_assert("c:\\halo\\SOURCE\\ai\\action_obey.c", 983, current_command->atom_type == _ai_atom_go_to_and_face);

					if (actor->state.flying)
					{
						real_vector3d direction_to_point;

						vector_from_points3d(
							&actor->input.position.body_position,
							&complex_control->destination_facing_point,
							&direction_to_point);

						if (normalize3d(&direction_to_point)>0.f)
						{
							facing_dot = dot_product3d(&direction_to_point, &actor->input.facing_vector);
						}
					}
					else
					{
						real_vector2d direction_to_point;

						vector_from_points2d(
							(real_point2d *)&actor->input.position.body_position,
							(real_point2d *)&complex_control->destination_facing_point,
							&direction_to_point);

						if (normalize2d(&direction_to_point)>0.f)
						{
							facing_dot = dot_product2d(&direction_to_point, (real_vector2d *)&actor->input.facing_vector);
						}
					}

					if (facing_dot<0.984f)
					{
						finished = FALSE;
					}
				}

				if (finished)
				{
					actor_move_halt(actor_index);
				}
			}
			else
			{
				finished = TRUE;
			}

			break;

		case _ai_atom_move_direction:
		{
			real_point3d current_position;
			real_vector3d direction_to_point;

			if (unit_index==actor->meta.unit_index)
			{
				current_position = actor->input.position.body_position;
			}
			else
			{
				object_get_origin(unit_index, &current_position);
			}

			vector_from_points3d(&simple_control->directmovement.start_position, &current_position, &direction_to_point);
			finished = dot_product3d(&simple_control->directmovement.vector, &direction_to_point)>current_command->parameter1;
			break;
		}

		case _ai_atom_move_immediate:
			finished = simple_control->pause_timer==0;
			break;

		case _ai_atom_shoot:
			if (unit_index==actor->meta.unit_index && complex_control)
			{
				if (!(actor->control.current_fire_target_type==_actor_fire_target_manual_point &&
					distance_squared3d(&complex_control->shoot_target, &actor->control.current_fire_target_manual_point)<0.25f))
				{
					struct actor_variant_definition *actor_variant = actor_variant_definition_get(actor->meta.variant_definition_index);
					short delay_ticks = (short)(actor_variant->ranged_combat.first_burst_delay_upper_bound * TICKS_PER_SECOND);

					simple_control->pause_timer = MAX(delay_ticks, 60);
				}

				finished = simple_control->pause_timer==0;
			}
			else
			{
				finished = TRUE;
			}

			break;

		case _ai_atom_running_jump:
		case _ai_atom_targeted_jump:
			if (TEST_FLAG(simple_control->simple_control_flags, _obey_simple_jump_bit))
			{
				boolean in_midair;

				if (unit_index==actor->meta.unit_index)
				{
					in_midair = actor->input.in_midair;
				}
				else
				{
					in_midair = unit_flying_through_air(unit_index);
				}

				if (TEST_FLAG(simple_control->simple_control_flags, _obey_simple_jump_jumped_bit) && in_midair)
				{
					simple_control->pause_timer = 0;
				}

				finished = simple_control->pause_timer==0;
			}
			else
			{
				finished = TRUE;
			}

			break;

		case _ai_atom_grenade:
			if (unit_index==actor->meta.unit_index && complex_control)
			{
				if (complex_control->grenade_throw_started)
				{
					struct unit_datum *unit = unit_get(unit_index);

					simple_control->pause_timer = (unit->unit.grenade_throw_state!=_unit_grenade_throw_idle) ? 30 : 0;
				}
				else if (!unit_is_busy(unit_index))
				{
					real_point3d grenade_target = complex_control->grenade_target;

					if (actor_combat_plan_grenade_trajectory(
						actor_index,
						complex_control->grenade_throw_trajectory_type,
						&grenade_target,
						NONE,
						NONE))
					{
						complex_control->grenade_throw_depress_trigger = TRUE;
					}
				}

				finished = simple_control->pause_timer==0;
			}
			else
			{
				finished = TRUE;
			}

			break;

		case _ai_atom_animate:
		{
			struct unit_datum *unit = unit_get(unit_index);

			finished = unit->unit.animation.state!=_unit_state_user_animation;
			break;
		}

		case _ai_atom_recording:
			finished = !recorded_animation_controlling_unit(unit_index);
			break;

		case _ai_atom_action:
			finished = !complex_control || !complex_control->play_action;
			break;

		case _ai_atom_wait:
			switch (current_command->atom_modifier)
			{
			case _ai_atom_wait_modifier_alerted:
				finished = actor->state.combat_status>_actor_combat_status_none;
				break;

			case _ai_atom_wait_modifier_visible_enemy:
				finished = actor->state.combat_status>=_actor_combat_status_visible;
				break;

			case _ai_atom_wait_modifier_told_to_advance:
				if (TEST_FLAG(simple_control->metadata_flags, _obey_metadata_told_to_advance_bit))
				{
					SET_FLAG(simple_control->metadata_flags, _obey_metadata_told_to_advance_bit, FALSE);
					SET_FLAG(simple_control->metadata_flags, _obey_metadata_waiting_for_advance_notification_bit, FALSE);
					finished = TRUE;
				}
				else
				{
					SET_FLAG(simple_control->metadata_flags, _obey_metadata_waiting_for_advance_notification_bit, TRUE);
					finished = FALSE;
				}

				break;

			default:
				finished = TRUE;
				break;
			}

			break;

		case _ai_atom_vocalize:
		{
			struct unit_datum *unit = unit_get(unit_index);

			finished = unit->unit.speech.current.priority!=_unit_speech_scripted;
			break;
		}

		case _ai_atom_animation_mode:
		case _ai_atom_crouch:
		case _ai_atom_vehicle:
		case _ai_atom_script:
		case _ai_atom_targeting:
		case _ai_atom_initiative:
		case _ai_atom_loop:
		case _ai_atom_die:
		case _ai_atom_set_radius:
		case _ai_atom_teleport:
			finished = TRUE;
			break;
		}
	}

	return finished;
}

static void action_obey_command_end(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	byte *next_command_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_command_lists, command_list_index, struct ai_command_list_definition);

	if (simple_control->current_command_index<command_list->commands.count)
	{
		struct ai_command_definition *current_command = TAG_BLOCK_GET_ELEMENT(&command_list->commands, simple_control->current_command_index, struct ai_command_definition);

		switch (current_command->atom_type)
		{
		case _ai_atom_go_to:
		case _ai_atom_go_to_and_face:
			if (unit_index==actor->meta.unit_index)
			{
				actor_move_halt(actor_index);
			}

			if (complex_control)
			{
				complex_control->destination_valid = FALSE;
				complex_control->destination_facing = FALSE;
			}

			break;

		case _ai_atom_move_direction:
		case _ai_atom_move_immediate:
			SET_FLAG(simple_control->simple_control_flags, _obey_simple_directmovement_bit, FALSE);
			simple_control->directmovement.facing = NONE;
			break;

		case _ai_atom_shoot:
			if (complex_control)
			{
				complex_control->shoot_at_target = FALSE;
			}

			break;

		case _ai_atom_running_jump:
		case _ai_atom_targeted_jump:
			SET_FLAG(simple_control->simple_control_flags, _obey_simple_jump_bit, FALSE);
			simple_control->jump.delay_ticks = 0;
			break;

		case _ai_atom_loop:
		{
			boolean loop = TRUE;

			if (current_command->atom_modifier==_ai_atom_loop_modifier_until_told_to_advance)
			{
				loop = !TEST_FLAG(simple_control->metadata_flags, _obey_metadata_told_to_advance_bit);
				SET_FLAG(simple_control->metadata_flags, _obey_metadata_told_to_advance_bit, FALSE);
				SET_FLAG(simple_control->metadata_flags, _obey_metadata_waiting_for_advance_notification_bit, loop);
			}

			if (loop)
			{
				if (current_command->command_index==simple_control->current_command_index)
				{
					char actor_description[512];

					ai_debug_describe_actor(actor_index, NONE, TRUE, actor_description, sizeof(actor_description));
					error(
						_error_silent,
						"%s: command list %s entry #%d tried to loop to itself",
						actor_description,
						command_list->name,
						simple_control->current_command_index);
				}
				else if (simple_control->loop_counter>=MAXIMUM_COMMAND_LIST_LOOPS)
				{
					char actor_description[512];

					ai_debug_describe_actor(actor_index, NONE, TRUE, actor_description, sizeof(actor_description));
					error(
						_error_silent,
						"%s: command list %s is stuck looping (aborting on loop #%d)",
						actor_description,
						command_list->name,
						simple_control->current_command_index);
				}
				else
				{
					*next_command_index = (byte)current_command->command_index;
					simple_control->loop_counter++;
				}
			}

			break;
		}

		case _ai_atom_animate:
		{
			struct biped_datum *biped = biped_try_and_get(unit_index);

			if (biped)
			{
				SET_FLAG(biped->biped.flags, _biped_absolute_movement_bit, FALSE);
				SET_FLAG(biped->biped.flags, _biped_no_collision_bit, FALSE);
			}

			break;
		}

		case _ai_atom_look:
		case _ai_atom_look_random:
		case _ai_atom_look_player:
		case _ai_atom_look_object:
			if (unit_index==actor->meta.unit_index)
			{
				actor_look_secondary_stop(actor_index);
			}

			break;
		}
	}

	return;
}

static void action_obey_individual_setup(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	void *user_data)
{
	boolean *targeting_reference = (boolean *)user_data;

	memset(simple_control, 0, sizeof(*simple_control));
	simple_control->current_command_index = NONE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_obey.c", 1360, targeting_reference);
	SET_FLAG(simple_control->metadata_flags, _obey_metadata_targeting_bit, *targeting_reference);

	if (complex_control)
	{
		memset(complex_control, 0, sizeof(*complex_control));
		complex_control->override_movement_type = NONE;
	}

	return;
}

static void action_obey_individual_perform(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	void *user_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_command_lists, command_list_index, struct ai_command_list_definition);
	boolean *finished_reference = (boolean *)user_data;

	if (!TEST_FLAG(simple_control->metadata_flags, _obey_metadata_commands_finished_bit))
	{
		boolean command_valid = FALSE;

		if (simple_control->current_command_index<command_list->commands.count)
		{
			command_valid = TRUE;
		}

		simple_control->loop_counter = 0;

		while (!command_valid || action_obey_command_perform(actor_index, unit_index, command_list_index, simple_control, complex_control))
		{
			byte next_command_index = (simple_control->current_command_index==NONE) ? 0 : simple_control->current_command_index + 1;

			if (command_valid)
			{
				action_obey_command_end(
					actor_index,
					unit_index,
					command_list_index,
					simple_control,
					complex_control,
					&next_command_index);
			}

			if (next_command_index>=command_list->commands.count)
			{
				SET_FLAG(simple_control->metadata_flags, _obey_metadata_commands_finished_bit, TRUE);
				break;
			}

			simple_control->current_command_index = next_command_index;
			command_valid = action_obey_command_begin(actor_index, unit_index, command_list_index, simple_control, complex_control);

			if (TEST_FLAG(simple_control->metadata_flags, _obey_metadata_action_changed_bit))
			{
				break;
			}
		}
	}

	if (!TEST_FLAG(simple_control->metadata_flags, _obey_metadata_commands_finished_bit))
	{
		match_assert("c:\\halo\\SOURCE\\ai\\action_obey.c", 1429, finished_reference);
		*finished_reference = FALSE;
	}

	return;
}

static void action_obey_individual_flush_command_indices(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	void *user_data)
{
	struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_command_lists, command_list_index, struct ai_command_list_definition);

	if (simple_control->current_command_index>=command_list->commands.count)
	{
		simple_control->current_command_index = NONE;
	}

	return;
}

static void action_obey_individual_begin(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	void *user_data)
{
	struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_command_lists, command_list_index, struct ai_command_list_definition);

	if (TEST_FLAG(command_list->flags, _ai_command_list_disable_falling_damage_bit))
	{
		struct unit_datum *unit = unit_get(unit_index);

		SET_FLAG(unit->unit.flags, _unit_no_falling_damage_bit, TRUE);
	}

	return;
}

static void action_obey_individual_advance(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	void *user_data)
{
	SET_FLAG(simple_control->metadata_flags, _obey_metadata_told_to_advance_bit, TRUE);
	SET_FLAG(simple_control->metadata_flags, _obey_metadata_waiting_for_advance_notification_bit, FALSE);

	return;
}

static void action_obey_individual_end(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	void *user_data)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (!TEST_FLAG(simple_control->metadata_flags, _obey_metadata_commands_finished_bit))
	{
		byte dummy_next_index;

		action_obey_command_end(actor_index, unit_index, command_list_index, simple_control, complex_control, &dummy_next_index);
	}

	SET_FLAG(unit->unit.flags, _unit_no_falling_damage_bit, FALSE);

	return;
}

static void action_obey_individual_update(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control,
	void *user_data)
{
	if (simple_control->pause_timer>0)
	{
		simple_control->pause_timer--;
	}

	if (TEST_FLAG(simple_control->simple_control_flags, _obey_simple_jump_bit) && simple_control->jump.delay_ticks>0)
	{
		simple_control->jump.delay_ticks--;
	}

	if (TEST_FLAG(simple_control->simple_control_flags, _obey_simple_directmovement_bit) &&
		TEST_FLAG(simple_control->simple_control_flags, _obey_simple_directmovement_update_continuously_bit))
	{
		action_obey_directmovement_update_facing(actor_index, unit_index, simple_control);
	}

	return;
}

static void action_obey_individuals_iterate(
	long actor_index,
	boolean initialize_structures,
	struct obey_state_data *state_data,
	obey_individual_iterator iterator,
	void *user_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_obey.c", 1553, !actor->meta.swarm || (actor->meta.swarm_cache_index != NONE));

	if (actor->meta.swarm)
	{
		short component_index;
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

		for (component_index = 0; component_index<swarm->unit_count; component_index++)
		{
			struct swarm_component_datum *component = swarm_component_get(swarm->component_indices[component_index]);

			if (initialize_structures)
			{
				memset(&component->obey, 0, sizeof(component->obey));
				SET_FLAG(component->flags, _swarm_component_wander_bit, FALSE);
				SET_FLAG(component->flags, _swarm_component_obey_bit, TRUE);
			}

			if (TEST_FLAG(component->flags, _swarm_component_obey_bit))
			{
				iterator(
					actor_index,
					swarm->unit_indices[component_index],
					state_data->command_list_index,
					&component->obey,
					NULL,
					user_data);
			}
		}
	}
	else
	{
		iterator(
			actor_index,
			actor->meta.unit_index,
			state_data->command_list_index,
			&state_data->simple_control,
			&state_data->complex_control,
			user_data);
	}

	return;
}

boolean action_obey_command_list_setup(
	long actor_index,
	short command_list_index,
	struct obey_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct scenario *scenario = global_scenario_get();
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_obey.c", 1597, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (command_list_index>=0 && command_list_index<scenario->ai_command_lists.count)
	{
		struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&scenario->ai_command_lists, command_list_index, struct ai_command_list_definition);

		if (actor->meta.swarm && actor->meta.swarm_cache_index==NONE)
		{
			if (actor->meta.active)
			{
				char actor_description[256];

				ai_debug_describe_actor(actor_index, NONE, TRUE, actor_description, sizeof(actor_description));
				error(_error_silent, "swarm actor %s cannot execute command list, ran out of swarm caches", actor_description);
			}
			else
			{
				actor->state.command_list_index = command_list_index;
			}
		}
		else if (command_list->runtime_structure_bsp_reference_index==NONE ||
			command_list->runtime_structure_bsp_reference_index==global_structure_bsp_index)
		{
			state_data->command_list_index = command_list_index;
			success = TRUE;
		}
		else
		{
			error(_error_silent, "wrong structure bsp, cannot execute command list %s", command_list->name);
		}

		if (success)
		{
			boolean initiative = TEST_FLAG(command_list->flags, _ai_command_list_allow_initiative_bit);
			boolean targeting = TEST_FLAG(command_list->flags, _ai_command_list_allow_targeting_bit);
			boolean allow_looking = !TEST_FLAG(command_list->flags, _ai_command_list_disable_looking_bit);
			boolean allow_communication = !TEST_FLAG(command_list->flags, _ai_command_list_disable_communication_bit);

			if (!allow_looking)
			{
				actor_look_secondary_stop(actor_index);
			}

			state_data->initiative = initiative;
			state_data->allow_looking = allow_looking;
			state_data->allow_communication = allow_communication;
			action_obey_individuals_iterate(actor_index, TRUE, state_data, action_obey_individual_setup, &targeting);
		}
	}

	return success;
}

void action_obey_flush_command_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct obey_state_data *state_data = &actor->state.action_data.obey;
	struct scenario *scenario = global_scenario_get();

	if (state_data->command_list_index>=0 && state_data->command_list_index<scenario->ai_command_lists.count)
	{
		action_obey_individuals_iterate(actor_index, FALSE, state_data, action_obey_individual_flush_command_indices, NULL);
	}
	else
	{
		state_data->command_list_index = NONE;
		state_data->finished = TRUE;
		actor_action_change(actor_index, _actor_action_none, NULL);
	}

	return;
}

void action_obey_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct obey_state_data *state_data = &actor->state.action_data.obey;

	if (state_data->complex_control.destination_valid)
	{
		state_data->complex_control.destination_surface_index = NONE;
	}

	return;
}

void action_obey_advance_command_list(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	action_obey_individuals_iterate(actor_index, FALSE, &actor->state.action_data.obey, action_obey_individual_advance, NULL);

	return;
}

void action_obey_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	action_obey_individuals_iterate(actor_index, FALSE, &actor->state.action_data.obey, action_obey_individual_begin, NULL);

	return;
}

void action_obey_end(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	action_obey_individuals_iterate(actor_index, FALSE, &actor->state.action_data.obey, action_obey_individual_end, NULL);

	return;
}

void action_obey_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	action_obey_individuals_iterate(actor_index, FALSE, &actor->state.action_data.obey, action_obey_individual_update, NULL);

	return;
}

boolean action_obey_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct obey_state_data *state_data = &actor->state.action_data.obey;
	boolean finished = TRUE;

	action_obey_individuals_iterate(actor_index, FALSE, state_data, action_obey_individual_perform, &finished);

	if (finished && !state_data->finished)
	{
		struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_command_lists, state_data->command_list_index, struct ai_command_list_definition);

		if (!TEST_FLAG(command_list->flags, _ai_command_list_disable_falling_damage_bit) ||
			!actor->input.in_midair ||
			TEST_FLAG(actor_definition_get(actor->meta.definition_index)->flags, _actor_definition_flying_bit))
		{
			actor->state.last_command_list_time = game_time_get();
			state_data->finished = TRUE;
		}
	}

	return actor->state.action==_actor_action_obey && state_data->finished;
}

void action_obey_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct obey_state_data *state_data = &actor->state.action_data.obey;

	match_assert("c:\\halo\\SOURCE\\ai\\action_obey.c", 1777, !actor->meta.swarm);

	if (state_data->complex_control.shoot_at_target)
	{
		actor->orders.look.primary_priority = _primary_priority_locked_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_target;
		actor->orders.look.idle_look_type = _idle_look_combat;
		actor->orders.combat.shoot_at_target = TRUE;
		actor->orders.combat.override_firing_restrictions = TRUE;
		actor->orders.combat.use_manual_target_point = TRUE;
		actor->orders.combat.target_point = state_data->complex_control.shoot_target;
		actor->orders.combat.override_burst_duration = state_data->complex_control.shoot_burst_length;
	}
	else if (state_data->complex_control.destination_facing && actor_path_at_destination(actor_index))
	{
		actor->orders.look.primary_priority = _primary_priority_exact_facing;
		actor->orders.look.primary_direction.type = _direction_specification_point;
		actor->orders.look.primary_direction.point = state_data->complex_control.destination_facing_point;

		if (!actor->state.flying)
		{
			actor->orders.look.primary_direction.point.z = actor->input.position.head_position.z;
		}

		actor->orders.look.idle_look_type = (actor->state.mode>=_actor_mode_combat) ? _idle_look_combat : _idle_look_noncombat;
	}
	else if (state_data->complex_control.override_movement_type==_actor_movement_type_panic ||
		state_data->complex_control.override_movement_type==_actor_movement_type_asleep)
	{
		actor->orders.look.primary_priority = _primary_priority_locked_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_movement;
		actor->orders.look.idle_look_type = _idle_look_none;
	}
	else if (actor->state.combat_status>=_actor_combat_status_clear_los &&
		TEST_FLAG(state_data->simple_control.metadata_flags, _obey_metadata_targeting_bit))
	{
		actor->orders.look.primary_priority = _primary_priority_locked_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_target;
		actor->orders.look.idle_look_type = _idle_look_combat;
		actor->orders.combat.shoot_at_target = TRUE;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_none;

		if (state_data->allow_looking)
		{
			actor->orders.look.idle_look_type = (actor->state.mode>=_actor_mode_combat) ? _idle_look_combat : _idle_look_noncombat;
		}
		else
		{
			actor->orders.look.idle_look_type = _idle_look_none;
		}
	}

	if (state_data->complex_control.grenade_throw_depress_trigger)
	{
		actor->orders.combat.throw_grenade = TRUE;
		state_data->complex_control.grenade_throw_depress_trigger = FALSE;
	}

	actor->orders.move.stationary_crouch = state_data->complex_control.override_crouch;
	actor->orders.move.moving_crouch = state_data->complex_control.override_crouch;
	actor->orders.move.override_movement_type = state_data->complex_control.override_movement_type;

	if (state_data->complex_control.play_action && !actor_move_animation_busy(actor_index))
	{
		if (state_data->complex_control.action_animation_impulse!=NONE)
		{
			real_vector2d alignment_vector = *(real_vector2d *)&actor->control.desired_facing_vector;

			normalize2d(&alignment_vector);
			actor_move_animation_impulse(actor_index, state_data->complex_control.action_animation_impulse, &alignment_vector);
		}

		if (state_data->complex_control.action_communication_type!=NONE)
		{
			ai_communication_event(
				state_data->complex_control.action_communication_type,
				actor->meta.unit_index,
				NONE,
				NONE,
				NONE,
				NONE,
				NULL);
		}

		state_data->complex_control.play_action = FALSE;
	}

	if (TEST_FLAG(state_data->simple_control.simple_control_flags, _obey_simple_directmovement_bit))
	{
		actor->orders.move.override_movement_direction = TRUE;
		actor->orders.move.override_movement_direction_vector = state_data->simple_control.directmovement.vector;
		actor->orders.move.override_movement_facing = state_data->simple_control.directmovement.facing;
	}

	if (TEST_FLAG(state_data->simple_control.simple_control_flags, _obey_simple_jump_bit))
	{
		boolean hold_facing = FALSE;

		if (TEST_FLAG(state_data->simple_control.simple_control_flags, _obey_simple_jump_jumped_bit))
		{
			hold_facing = state_data->simple_control.jump.delay_ticks>0;
		}
		else if (state_data->simple_control.jump.delay_ticks==0 &&
			!actor->input.in_midair &&
			!unit_is_busy(actor->meta.unit_index))
		{
			real_vector2d alignment_vector = *(real_vector2d *)&actor->input.facing_vector;

			if (normalize2d(&alignment_vector)==0.f)
			{
				alignment_vector = *global_forward2d;
			}

			actor->orders.move.jump = TRUE;
			actor->orders.move.jump_leap = state_data->simple_control.jump.target_horizontal_vel * 0.7f>state_data->simple_control.jump.target_vertical_vel;
			actor->orders.move.jump_targeted = TEST_FLAG(state_data->simple_control.simple_control_flags, _obey_simple_jump_targeted_bit);
			actor->orders.move.jump_alignment_vector = alignment_vector;
			actor->orders.move.jump_target_horizontal_vel = state_data->simple_control.jump.target_horizontal_vel;
			actor->orders.move.jump_target_vertical_vel = state_data->simple_control.jump.target_vertical_vel;
			SET_FLAG(state_data->simple_control.simple_control_flags, _obey_simple_jump_jumped_bit, TRUE);

			if (!TEST_FLAG(state_data->simple_control.simple_control_flags, _obey_simple_jump_targeted_bit))
			{
				state_data->simple_control.jump.delay_ticks = 15;
			}
		}
		else
		{
			hold_facing = TRUE;
		}

		if (hold_facing)
		{
			actor->orders.move.override_movement_direction = TRUE;
			actor->orders.move.override_movement_direction_vector = actor->input.facing_vector;
			actor->orders.move.override_movement_facing = _actor_facing_forward;
		}
	}

	return;
}

void action_obey_describe_command(
	struct scenario *scenario,
	struct ai_command_definition *command,
	char *string,
	long string_size)
{
	char const *look_modifier_names[NUMBER_OF_AI_ATOM_LOOK_MODIFIERS] =
	{
		"idle aim weapon",
		"idle turn around",
		"idle look with head",
		"forced exact facing",
		"forced aim weapon"
	};

	switch (command->atom_type)
	{
	case _ai_atom_pause:
		_snprintf(string, string_size, "pause %.1f", command->parameter1);
		break;

	case _ai_atom_go_to:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_GO_TO_MODIFIERS] =
		{
			"stop_at_point",
			"keep_moving"
		};

		_snprintf(string, string_size, "go to (p%d) %s", command->point1_index, modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_go_to_and_face:
		_snprintf(string, string_size, "go to (p%d) and face (p%d)", command->point1_index, command->point2_index);
		break;

	case _ai_atom_move_direction:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_MOVE_DIRECTION_MODIFIERS] =
		{
			"forwards",
			"left",
			"right",
			"backwards",
			"any-facing"
		};

		if (command->point1_index==NONE)
		{
			_snprintf(
				string,
				string_size,
				"move %s along angle %.1f, dist %.2f",
				modifier_names[command->atom_modifier],
				command->parameter2,
				command->parameter1);
		}
		else
		{
			_snprintf(
				string,
				string_size,
				"move %s towards (p%d), dist %.2f",
				modifier_names[command->atom_modifier],
				command->point1_index,
				command->parameter1);
		}

		break;
	}

	case _ai_atom_move_immediate:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_MOVE_IMMEDIATE_MODIFIERS] =
		{
			"forwards",
			"left",
			"right",
			"backwards"
		};

		_snprintf(string, string_size, "move %s for %.1f sec", modifier_names[command->atom_modifier], command->parameter1);
		break;
	}

	case _ai_atom_look:
		_snprintf(
			string,
			string_size,
			"look %s at (p%d) for %.1f",
			look_modifier_names[command->atom_modifier],
			command->point1_index,
			command->parameter1);
		break;

	case _ai_atom_look_random:
		_snprintf(
			string,
			string_size,
			"look %s at random one of (p%d-p%d) for %.1f-%.1f",
			look_modifier_names[command->atom_modifier],
			command->point1_index,
			command->point2_index,
			command->parameter1,
			command->parameter2);
		break;

	case _ai_atom_look_player:
		_snprintf(
			string,
			string_size,
			"look %s at player for %.1f",
			look_modifier_names[command->atom_modifier],
			command->parameter1);
		break;

	case _ai_atom_look_object:
	{
		char const *object_name = "<error>";

		if (command->object_name_index>=0 && command->object_name_index<scenario->object_names.count)
		{
			object_name = TAG_BLOCK_GET_ELEMENT(&scenario->object_names, command->object_name_index, struct scenario_object_name)->name;
		}

		_snprintf(
			string,
			string_size,
			"look %s at %s for %.1f",
			look_modifier_names[command->atom_modifier],
			object_name,
			command->parameter1);
		break;
	}

	case _ai_atom_animation_mode:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_ANIMATION_MODE_MODIFIERS] =
		{
			"noncombat",
			"asleep",
			"combat",
			"panic"
		};

		_snprintf(string, string_size, "animation mode %s", modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_crouch:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_CROUCH_MODIFIERS] =
		{
			"disable",
			"enable"
		};

		_snprintf(string, string_size, "crouch %s", modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_shoot:
		_snprintf(string, string_size, "shoot at (p%d) for %.1f", command->point1_index, command->parameter1);
		break;

	case _ai_atom_grenade:
		_snprintf(string, string_size, "throw grenade at (p%d)", command->point1_index);
		break;

	case _ai_atom_vehicle:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_VEHICLE_MODIFIERS] =
		{
			"any-non-driver",
			"gunner",
			"passenger",
			"driver",
			"any-seat"
		};

		_snprintf(
			string,
			string_size,
			"enter vehicle as %s if within %.1f",
			modifier_names[command->atom_modifier],
			command->parameter1);
		break;
	}

	case _ai_atom_running_jump:
		_snprintf(string, string_size, "running jump");
		break;

	case _ai_atom_targeted_jump:
		_snprintf(string, string_size, "targeted jump (%.2fh, %.2fv)", command->parameter1, command->parameter2);
		break;

	case _ai_atom_script:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_SCRIPT_MODIFIERS] =
		{
			"wait-for-finish",
			"wake-and-continue"
		};
		char const *script_name = "<error>";

		if (command->script_reference_index==NONE)
		{
			script_name = "NONE";
		}
		else if (command->script_reference_index>=0 && command->script_reference_index<scenario->ai_script_references.count)
		{
			script_name = TAG_BLOCK_GET_ELEMENT(&scenario->ai_script_references, command->script_reference_index, struct ai_script_reference_definition)->script_name;
		}

		_snprintf(string, string_size, "script %s %s", script_name, modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_animate:
	{
		char const *animation_name = "<error>";

		if (command->animation_reference_index==NONE)
		{
			animation_name = "NONE";
		}
		else if (command->animation_reference_index>=0 &&
			command->animation_reference_index<scenario->ai_animation_references.count)
		{
			animation_name = TAG_BLOCK_GET_ELEMENT(&scenario->ai_animation_references, command->animation_reference_index, struct ai_animation_reference_definition)->animation_name;
		}

		_snprintf(string, string_size, "animate %s", animation_name);
		break;
	}

	case _ai_atom_recording:
	{
		char const *recording_name = "<error>";

		if (command->recording_reference_index==NONE)
		{
			recording_name = "NONE";
		}
		else if (command->recording_reference_index>=0 &&
			command->recording_reference_index<scenario->ai_recording_references.count)
		{
			recording_name = TAG_BLOCK_GET_ELEMENT(&scenario->ai_recording_references, command->recording_reference_index, struct ai_recording_reference_definition)->recording_name;
		}

		_snprintf(string, string_size, "play recording %s", recording_name);
		break;
	}

	case _ai_atom_vocalize:
		_snprintf(string, string_size, "vocalize %s", dialogue_get_vocalization_name(command->atom_modifier, FALSE));
		break;

	case _ai_atom_targeting:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_TARGETING_MODIFIERS] =
		{
			"enable",
			"disable"
		};

		_snprintf(string, string_size, "targeting %s", modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_action:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_ACTION_MODIFIERS] =
		{
			"berserk",
			"surprise-front",
			"surprise-back",
			"evade-left",
			"evade-right",
			"dive-fwd",
			"dive-back",
			"dive-left",
			"dive-right",
			"vehicle-woohoo",
			"vehicle-scared"
		};

		_snprintf(string, string_size, "action %s", modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_initiative:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_INITIATIVE_MODIFIERS] =
		{
			"enable",
			"disable"
		};

		_snprintf(string, string_size, "initiative %s", modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_wait:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_WAIT_MODIFIERS] =
		{
			"until alerted",
			"until visible enemy",
			"until told to advance"
		};

		_snprintf(string, string_size, "wait %s", modifier_names[command->atom_modifier]);
		break;
	}

	case _ai_atom_loop:
	{
		char const *modifier_names[NUMBER_OF_AI_ATOM_LOOP_MODIFIERS] =
		{
			"always",
			"only until told to advance"
		};

		if (command->command_index==NONE)
		{
			_snprintf(string, string_size, "loop to <none> %s", modifier_names[command->atom_modifier]);
		}
		else
		{
			_snprintf(string, string_size, "loop to #%d %s", command->command_index, modifier_names[command->atom_modifier]);
		}

		break;
	}

	case _ai_atom_set_radius:
		_snprintf(string, string_size, "set radius %.2f", command->parameter1);
		break;

	case _ai_atom_teleport:
	{
		char facing_string[256];

		strcpy(facing_string, "");

		if (command->point2_index!=NONE)
		{
			sprintf(facing_string, " and face at (p%d)", command->point2_index);
		}

		_snprintf(string, string_size, "teleport to (p%d)%s", command->point1_index, facing_string);
		break;
	}

	case _ai_atom_die:
		_snprintf(string, string_size, "die");
		break;

	default:
		_snprintf(string, string_size, "<unknown %d>", command->atom_type);
		break;
	}

	return;
}
