/*
AI_COMMUNICATION.H

header included in hcex build.
*/

#ifndef __AI_COMMUNICATION_H
#define __AI_COMMUNICATION_H
#pragma once

/* ---------- headers */

#include "ai.h"
#include "ai_constants.h"
#include "ai_scenario_definitions.h"

/* ---------- constants */

enum
{
	_ai_communication_death = 0,
	_ai_communication_killing_spree,
	_ai_communication_hurt,
	_ai_communication_damage,
	_ai_communication_sighted_enemy,
	_ai_communication_found_enemy,
	_ai_communication_unexpected_enemy,
	_ai_communication_found_dead_friend,
	_ai_communication_allegiance_changed,
	_ai_communication_grenade_throwing,
	_ai_communication_grenade_startle,
	_ai_communication_grenade_sighted,
	_ai_communication_grenade_danger,
	_ai_communication_lost_contact,
	_ai_communication_blocked,
	_ai_communication_alert_noncombat,
	_ai_communication_search_start,
	_ai_communication_search_query,
	_ai_communication_search_report,
	_ai_communication_search_abandon,
	_ai_communication_search_group_abandon,
	_ai_communication_uncover_start,
	_ai_communication_advance,
	_ai_communication_retreat,
	_ai_communication_cover,
	_ai_communication_sighted_friend_player,
	_ai_communication_shooting,
	_ai_communication_shooting_vehicle,
	_ai_communication_shooting_berserk,
	_ai_communication_shooting_group,
	_ai_communication_shooting_traitor,
	_ai_communication_flee,
	_ai_communication_flee_leader_died,
	_ai_communication_flee_idle,
	_ai_communication_attempted_flee,
	_ai_communication_hiding_finished,
	_ai_communication_vehicle_entry,
	_ai_communication_vehicle_exit,
	_ai_communication_vehicle_woohoo,
	_ai_communication_vehicle_scared,
	_ai_communication_vehicle_falling,
	_ai_communication_surprise,
	_ai_communication_berserk,
	_ai_communication_melee,
	_ai_communication_dive,
	_ai_communication_uncover_exclamation,
	_ai_communication_falling_to_death,
	_ai_communication_leap,
	_ai_communication_postcombat_alone,
	_ai_communication_postcombat_unscathed,
	_ai_communication_postcombat_wounded,
	_ai_communication_postcombat_massacre,
	_ai_communication_postcombat_triumph,
	_ai_communication_postcombat_check_enemy,
	_ai_communication_postcombat_check_friend,
	_ai_communication_postcombat_shoot_corpse,
	_ai_communication_postcombat_celebrate,
	NUMBER_OF_AI_COMMUNICATION_TYPES,
};

enum
{
	_ai_conversation_status_none = 0, /* fake name */
	_ai_conversation_status_trying_to_begin, /* fake name */
	_ai_conversation_status_waiting_for_participants, /* fake name */
	_ai_conversation_status_playing, /* fake name */
	_ai_conversation_status_waiting_to_advance, /* fake name */
	_ai_conversation_status_unable_to_begin, /* fake name */
	_ai_conversation_status_finished_successfully, /* fake name */
	_ai_conversation_status_finished_prematurely, /* fake name */
	NUMBER_OF_AI_CONVERSATION_STATUSES, /* fake name */
};

/* ---------- macros */

#define conversation_get(index) ((struct conversation_datum *)datum_get(conversation_data, (index))) /* fake name */

/* ---------- structures */

struct conversation_datum
{
	short identifier;
	short conversation_definition_index;
	boolean scripted;
	boolean any_line_spoken;
	boolean begun;
	boolean finished;
	boolean waiting_to_advance;
	boolean told_to_advance;
	long creation_time;
	long triggering_player_unit_index;
	unsigned long participant_bitmask;
	short dialogue_indices[MAXIMUM_PARTICIPANTS_PER_CONVERSATION];
	long actor_indices[MAXIMUM_PARTICIPANTS_PER_CONVERSATION];
	short line_index;
	short line_participant_index;
	short line_delay_timer;
	word line_flags;
	long line_actor_index;
	long line_unit_index;
	long line_address_unit_index;
	long line_sound_index;
	boolean line_unspatialized;
	boolean line_spoken;
	boolean line_finished;
	boolean line_advance;
};

/* ---------- prototypes/AI_COMMUNICATION.C */

void ai_communication_initialize(void);
void ai_communication_dispose(void);
void ai_communication_initialize_for_new_map(void);
void ai_communication_dispose_from_old_map(void);
char const *ai_communication_get_type_name(short communication_type);
short ai_communication_get_type_by_name(char const *name);
void ai_communication_packet_new(struct ai_information_packet *information);
void ai_communication_event(short communication_type, long subject_unit_index, long cause_unit_index, short hostility, short damage_type, short information_type, struct ai_information_data *information_data);
void ai_communication_started(long unit_index, short priority, short vocalization_type, struct ai_information_packet *ai_information);
void ai_communication_notify(long unit_index, short priority, short vocalization_type, struct ai_information_packet *ai_information);
void ai_communication_finished(long unit_index, short priority, short vocalization_type, boolean reply_to_player, long preselected_reply_actor_index, struct ai_information_packet *ai_information);
void actor_communication_update(long actor_index);
short actor_communication_team(long actor_index);
real ai_communication_get_player_rating(long unit_index, boolean test_line_of_sight, long *unit_index_reference, real *distance_reference);
boolean ai_conversation(short conversation_definition_index, boolean scripted);
short ai_conversation_status(short conversation_definition_index);
short ai_conversation_line(short conversation_definition_index);
void ai_conversation_stop(short conversation_definition_index);
void ai_conversation_advance(short conversation_definition_index);
void ai_conversation_update(void);
void ai_conversation_actor_deleted(long actor_index);
void ai_conversation_unit_died(long unit_index, boolean deleted);
void ai_conversation_finish(long conversation_index, boolean unable_to_begin, boolean success);

/* ---------- globals */

extern struct data_array *conversation_data;

/* ---------- public code */

#endif // __AI_COMMUNICATION_H
