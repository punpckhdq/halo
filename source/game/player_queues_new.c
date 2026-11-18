/*
PLAYER_QUEUES_NEW.C
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "network_game_globals.h"
#include "input.h"
#include "console.h"
#include "vehicles.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "saved_films.h"
#include "units.h"
#include "game.h"
#include "network_game_manager.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_UPDATE_QUEUES = 16, /* fake name */
	MAXIMUM_NUMBER_OF_SERVER_UPDATES = 32, /* fake name */
	MAXIMUM_NUMBER_OF_CLIENT_UPDATES = 128 /* fake name */
};

/* ---------- macros */

#define LATCHED_CONTROL_FLAGS (FLAG(_unit_control_integrated_light_bit) | FLAG(_unit_control_action_bit) | FLAG(_unit_control_use_equipment_bit) | FLAG(_unit_control_weapon_reload_bit)) /* fake name */

/* ---------- structures */

struct update
{
	long update_number;
	struct game_update data;
};

struct update_server_queue
{
	short identifier;
	word pad;
	long next_update_number;
	struct player_action current_action;
};

struct update_client_queue
{
	short identifier;
	word pad0;
	unsigned long control_flags;
	unsigned long latched_control_flags;
	real_euler_angles2d desired_facing;
	real_vector2d throttle;
	real primary_trigger;
	short desired_weapon_index;
	short desired_grenade_index;
	short desired_zoom_level;
	word pad1;
};

struct _update_server_globals
{
	boolean initialized;
	long next_update_number_to_build;
	struct data_array *queues;
	struct update updates[MAXIMUM_NUMBER_OF_SERVER_UPDATES];
};

struct _update_client_globals
{
	boolean initialized;
	long next_update_number_to_dequeue;
	long latest_update_number_received;
	struct player_action_collection saved_action_collection;
	long current_local_player;
	struct data_array *queues;
	struct update updates[MAXIMUM_NUMBER_OF_CLIENT_UPDATES];
};

/* ---------- prototypes */

static struct update *update_server_get_update(long update_number);
static struct update *update_client_get_update(long update_number);

/* ---------- globals */

static struct _update_client_globals update_client_globals;
static struct _update_server_globals update_server_globals;

/* ---------- public code */

void update_queues_reset_and_fill_with_lies(
	void)
{
	if (update_server_globals.initialized)
	{
		update_server_globals.next_update_number_to_build = 0;
		memset(update_server_globals.updates, 0, sizeof(update_server_globals.updates));
	}

	if (update_client_globals.initialized)
	{
		long game_time;
		long update_number;
		long update_index;

		memset(update_client_globals.updates, NONE, sizeof(update_client_globals.updates));
		memset(&update_client_globals.saved_action_collection, 0, sizeof(update_client_globals.saved_action_collection));
		update_client_globals.latest_update_number_received = NONE;
		update_client_globals.next_update_number_to_dequeue = 0;
		game_time = game_time_get();

		for (update_number = FLOOR(game_time - MAXIMUM_NUMBER_OF_CLIENT_UPDATES, 0), update_index = 0; update_number < game_time; update_index++, update_number++)
		{
			update_client_globals.updates[update_index].update_number = update_number;
			update_client_globals.updates[update_index].data.number_of_actions = 1;
			memset(update_client_globals.updates[update_index].data.actions, 0, sizeof(update_client_globals.updates[update_index].data.actions));
		}

		update_client_globals.latest_update_number_received = game_time - 1;
		update_client_globals.next_update_number_to_dequeue = game_time;
		update_server_globals.next_update_number_to_build = game_time;
	}

	if (update_server_globals.initialized)
	{
		struct update_server_queue *queue;

		update_server_start();
		queue = datum_get(update_server_globals.queues, 0);
		queue->next_update_number = update_server_globals.next_update_number_to_build;
	}
	else if (update_client_globals.initialized)
	{
		update_client_start();
	}

	return;
}

boolean update_server_new(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 172, !update_server_globals.initialized);

	memset(&update_server_globals, 0, sizeof(update_server_globals));
	update_server_globals.queues = data_new("update server queues", MAXIMUM_NUMBER_OF_UPDATE_QUEUES, sizeof(struct update_server_queue));

	if (update_server_globals.queues)
	{
		memset(update_server_globals.updates, 0, sizeof(update_server_globals.updates));

		if (update_client_new())
		{
			update_server_globals.initialized = TRUE;
		}
	}

	return update_server_globals.initialized;
}

void update_server_delete(
	void)
{
	if (update_server_globals.queues)
	{
		data_dispose(update_server_globals.queues);
		update_server_globals.queues = NULL;
	}

	update_server_globals.initialized = FALSE;
	update_server_globals.next_update_number_to_build = 0;
	update_client_delete();

	return;
}

void update_server_start(
	void)
{
	struct data_iterator iterator;

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 207, update_server_globals.initialized);

	data_make_valid(update_server_globals.queues);
	data_delete_all(update_server_globals.queues);
	data_iterator_new(&iterator, player_data);

	while (data_iterator_next(&iterator))
	{
		long queue_index = datum_new_at_index(update_server_globals.queues, iterator.index);

		match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 221, queue_index!=NONE);
	}

	update_client_start();

	return;
}

void update_server_add_player(
	long player_index)
{
	long queue_index = datum_new_at_index(update_server_globals.queues, player_index);

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 235, queue_index!=NONE);

	return;
}

void update_server_next_update(
	void)
{
	long update_number = update_server_globals.next_update_number_to_build;
	struct update *update;
	struct update_server_queue *queue;
	short queue_index;

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 250, update_server_globals.initialized);

	update_server_globals.next_update_number_to_build++;
	update = update_server_get_update(update_number);

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 256, update);

	update->update_number = update_number;
	update->data.number_of_actions = 0;

	for (queue_index = 0, queue = update_server_globals.queues->data; queue_index < update_server_globals.queues->count; queue_index++, queue++)
	{
		memcpy(&update->data.actions[queue_index], &queue->current_action, sizeof(struct player_action));
		update->data.number_of_actions++;
	}

	update_client_handle_server_update(&update->data, update_number);

	return;
}

void update_server_build_server_update(
	long machine_index,
	struct game_update *update,
	long *update_number)
{
	unsigned long time = system_milliseconds();
	struct update_server_queue *queue = NULL;

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 282, update && update_number && update_server_globals.initialized);

	if (machine_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 286, machine_index<MAXIMUM_NETWORK_MACHINE_COUNT);

		queue = datum_get(update_server_globals.queues, machine_index);

		if (queue->next_update_number < update_server_globals.next_update_number_to_build)
		{
			*update_number = queue->next_update_number;
		}
		else
		{
			*update_number = NONE;
		}
	}

	if (*update_number != NONE)
	{
		struct update *server_update = update_server_get_update(*update_number);

		if (server_update)
		{
			memcpy(update, &server_update->data, sizeof(*update));
		}

		if (queue)
		{
			queue->next_update_number++;
		}
	}

	return;
}

boolean update_client_new(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 326, !update_client_globals.initialized);

	memset(&update_client_globals, 0, sizeof(update_client_globals));
	update_client_globals.queues = data_new("update client queues", MAXIMUM_NUMBER_OF_UPDATE_QUEUES, sizeof(struct update_client_queue));

	if (update_client_globals.queues)
	{
		memset(update_client_globals.updates, NONE, sizeof(update_client_globals.updates));
		update_client_globals.latest_update_number_received = NONE;
		update_client_globals.next_update_number_to_dequeue = 0;
		update_client_globals.initialized = TRUE;
	}

	return update_client_globals.initialized;
}

void update_client_delete(
	void)
{
	if (update_client_globals.queues)
	{
		data_dispose(update_client_globals.queues);
		update_client_globals.queues = NULL;
	}

	update_client_globals.latest_update_number_received = NONE;
	update_client_globals.next_update_number_to_dequeue = 0;
	update_client_globals.initialized = FALSE;

	return;
}

void update_client_start(
	void)
{
	struct data_iterator iterator;

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 360, update_client_globals.initialized);

	data_make_valid(update_client_globals.queues);
	data_delete_all(update_client_globals.queues);
	data_iterator_new(&iterator, player_data);

	while (data_iterator_next(&iterator))
	{
		long queue_index = datum_new_at_index(update_client_globals.queues, iterator.index);

		match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 374, queue_index!=NONE);
	}

	return;
}

void update_client_add_player(
	long player_index)
{
	long queue_index = datum_new_at_index(update_client_globals.queues, player_index);

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 386, queue_index!=NONE);

	return;
}

void update_client_queue(
	struct player_action *action)
{
	update_client_globals.saved_action_collection.actions[update_client_globals.current_local_player] = *action;
	update_client_globals.current_local_player++;

	return;
}

void update_client_queue_push(
	void)
{
	update_client_globals.current_local_player = 0;
	memset(&update_client_globals.saved_action_collection, 0, sizeof(update_client_globals.saved_action_collection));

	return;
}

long update_client_get_maximum_actions(
	void)
{
	return update_client_globals.latest_update_number_received - update_client_globals.next_update_number_to_dequeue + 1;
}

boolean update_client_dequeue(
	struct player_action *actions)
{
	struct update *update;

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 431, update_client_globals.initialized);

	update = update_client_get_update(update_client_globals.next_update_number_to_dequeue);

	if (update &&
		update_client_globals.next_update_number_to_dequeue <= update_client_globals.latest_update_number_received &&
		update->data.number_of_actions > 0 &&
		update->data.number_of_actions <= NETWORK_GAME_MAXIMUM_PLAYER_COUNT)
	{
		struct update_client_queue *queue;
		short queue_index;

		for (queue_index = 0, queue = update_client_globals.queues->data; queue_index < update_client_globals.queues->count; queue_index++, queue++)
		{
			if (queue_index < update->data.number_of_actions)
			{
				struct player_action *action = &update->data.actions[queue_index];

				queue->control_flags = action->control_flags;
				queue->desired_facing = action->desired_facing;
				queue->throttle = action->throttle;
				queue->primary_trigger = action->primary_trigger;
				queue->desired_weapon_index = action->desired_weapon_index;
				queue->desired_grenade_index = action->desired_grenade_index;
				queue->desired_zoom_level = action->desired_zoom_level;

				match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 457, (NONE == queue->desired_weapon_index) || (queue->desired_weapon_index>=0 && queue->desired_weapon_index<MAXIMUM_WEAPONS_PER_UNIT));
				match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 458, (NONE == queue->desired_grenade_index) || (queue->desired_grenade_index>=0 && queue->desired_grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES));
				match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 459, (NONE == queue->desired_zoom_level) || (queue->desired_zoom_level>=0));
			}
		}

		for (queue_index = 0, queue = update_client_globals.queues->data; queue_index < update_client_globals.queues->count; queue_index++, queue++)
		{
			actions[queue_index].control_flags = queue->control_flags & ~queue->latched_control_flags;
			queue->latched_control_flags = queue->control_flags & LATCHED_CONTROL_FLAGS;
			actions[queue_index].desired_facing = queue->desired_facing;
			actions[queue_index].throttle = queue->throttle;
			actions[queue_index].primary_trigger = queue->primary_trigger;
			actions[queue_index].desired_weapon_index = queue->desired_weapon_index;
			actions[queue_index].desired_grenade_index = queue->desired_grenade_index;
			actions[queue_index].desired_zoom_level = queue->desired_zoom_level;

			match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 486, (NONE == actions[queue_index].desired_weapon_index) || (actions[queue_index].desired_weapon_index>=0 && actions[queue_index].desired_weapon_index<MAXIMUM_WEAPONS_PER_UNIT));
			match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 487, (NONE == actions[queue_index].desired_grenade_index) || (actions[queue_index].desired_grenade_index>=0 && actions[queue_index].desired_grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES));
			match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 488, (NONE == actions[queue_index].desired_zoom_level) || (actions[queue_index].desired_zoom_level>=0));
		}

		update_client_globals.next_update_number_to_dequeue++;

		return TRUE;
	}

	return FALSE;
}

long update_client_get_maximum_possible_server_time(
	void)
{
	long update_number;

	for (update_number = update_client_globals.next_update_number_to_dequeue; update_number <= update_client_globals.latest_update_number_received; update_number++)
	{
		struct update *update = update_client_get_update(update_number);

		if (!update || update->data.number_of_actions <= 0 || update->data.number_of_actions > NETWORK_GAME_MAXIMUM_PLAYER_COUNT)
		{
			break;
		}
	}

	return update_number;
}

void update_client_local_ticks(
	short ticks_elapsed)
{
	struct player_action_collection client_update;
	struct game_update update;
	long update_number;
	long machine_index = 0;

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 523, game_connection()==_game_connection_local);

	update_client_build_client_update(&client_update);
	update_server_handle_client_update(machine_index, client_update.actions);

	while (ticks_elapsed-- > 0)
	{
		update_server_next_update();
		update_server_build_server_update(machine_index, &update, &update_number);
	}

	return;
}

void update_server_handle_client_update(
	long machine_index,
	struct player_action *actions)
{
	long *player_list = machine_get_player_list(machine_index);
	long action_index = 0;
	long player_index;

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 554, update_server_globals.initialized);

	for (player_index = 0; player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; player_index++)
	{
		if (player_list[player_index] != NONE)
		{
			struct update_server_queue *queue = datum_get(update_server_globals.queues, player_list[player_index]);

			queue->current_action = actions[action_index++];
			match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_queues_new.c", 568, queue->current_action.desired_facing.pitch);
			match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_queues_new.c", 569, queue->current_action.desired_facing.yaw);
		}
	}

	return;
}

void update_client_build_client_update(
	struct player_action_collection *action_collection)
{
	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 580, action_collection && update_client_globals.initialized);

	memcpy(action_collection, &update_client_globals.saved_action_collection, sizeof(*action_collection));

	return;
}

void update_client_handle_server_update(
	struct game_update *new_update,
	long update_number)
{
	struct update *update = update_client_get_update(update_number);

	if (update)
	{
		update->update_number = update_number;
		memcpy(&update->data, new_update, sizeof(update->data));

		if (update_number > update_client_globals.latest_update_number_received)
		{
			long missed_update_number;

			for (missed_update_number = update_client_globals.latest_update_number_received + 1; missed_update_number < update_number; missed_update_number++)
			{
				update->data.number_of_actions = NONE;
			}

			update_client_globals.latest_update_number_received = update_number;
		}
	}
	else if ((!global_network_game_client_get() || !global_network_game_server_get()) && !main_menu_is_active())
	{
		error(_error_silent, "failed to get an update (#%d); sp scenario= '%s'", update_number, main_get_map_name());
		error(_error_silent, "if you're in a multiplayer game, you might be out of sync now");
		error(_error_silent, "if you're playing single player/coop, you can probably ignore this");
	}

	switch (game_connection())
	{
	case _game_connection_local:
	case _game_connection_network_client:
	case _game_connection_network_server:
		main_menu_is_active();
		break;
	}

	return;
}

long player_new_queue(
	long player_index)
{
	long queue_index = datum_new_at_index(update_server_globals.queues, player_index);

	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 658, queue_index!=NONE);

	return queue_index;
}

static struct update *update_server_get_update(
	long update_number)
{
	match_assert("c:\\halo\\SOURCE\\game\\player_queues_new.c", 670, update_server_globals.initialized);

	return (update_number < update_server_globals.next_update_number_to_build && update_number >= update_server_globals.next_update_number_to_build - MAXIMUM_NUMBER_OF_SERVER_UPDATES) ? &update_server_globals.updates[update_number & (MAXIMUM_NUMBER_OF_SERVER_UPDATES - 1)] : NULL;
}

static struct update *update_client_get_update(
	long update_number)
{
	return (update_number >= update_client_globals.next_update_number_to_dequeue && update_number < update_client_globals.next_update_number_to_dequeue + MAXIMUM_NUMBER_OF_CLIENT_UPDATES) ? &update_client_globals.updates[update_number & (MAXIMUM_NUMBER_OF_CLIENT_UPDATES - 1)] : NULL;
}
