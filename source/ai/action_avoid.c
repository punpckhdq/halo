/*
ACTION_AVOID.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean action_avoid_setup(
	long actor_index,
	struct avoid_state_data *state_data)
{
	match_assert("c:\\halo\\SOURCE\\ai\\action_avoid.c", 30, state_data);

	memset(state_data, 0, sizeof(struct avoid_state_data));

	return TRUE;
}

void action_avoid_begin(
	long actor_index)
{
	return;
}

void action_avoid_end(
	long actor_index)
{
	return;
}

boolean action_avoid_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct avoid_state_data *state_data = &actor->state.action_data.avoid;

	match_assert("c:\\halo\\SOURCE\\ai\\action_avoid.c", 55, !actor->meta.swarm);

	if (actor->meta.timeslice)
	{
		struct firing_position_evaluation_context evaluation_context;
		struct firing_position new_firing_position;
		struct path_state path_state;
		boolean area_path_valid;
		long previous_owner;
		short firing_position_index;

		memset(&evaluation_context, 0, sizeof(evaluation_context));
		evaluation_context.evaluation_mode = _firing_point_evaluation_mode_avoid;

		firing_position_index = actor_active_select_firing_position(actor_index, &evaluation_context, &new_firing_position, &previous_owner, &path_state, &area_path_valid);
		actor_change_firing_position(actor_index, firing_position_index, &new_firing_position, previous_owner, &path_state, area_path_valid);
	}

	return actor->danger_zone.danger_type == _actor_danger_zone_none;
}

void action_avoid_update(
	long actor_index)
{
	return;
}

void action_avoid_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->target.target_type >= _actor_target_uninspected_orphan)
	{
		actor->orders.combat.shoot_at_target = TRUE;
		actor->orders.look.primary_priority = _primary_priority_locked_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_target;
	}
	else if (actor->danger_zone.danger_type > _actor_danger_zone_none)
	{
		actor->orders.look.primary_priority = _primary_priority_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_danger;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_target;
	}

	actor->orders.look.idle_look_type = _idle_look_combat;
	actor->orders.move.stationary_crouch = actor->emotions.defensive_crouch;
	actor->orders.move.moving_crouch = FALSE;
	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;

	return;
}

/* ---------- private code */
