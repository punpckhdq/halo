/*
AI_COMMUNICATION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_communication.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "ai_globals.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "console.h"
#include "game_state.h"
#include "collisions.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "damage_effect_definitions.h"
#include "dialogue_definitions.h"

/* ---------- constants */

enum
{
	_ai_communication_priority_none = 0,
	_ai_communication_priority_filler,
	_ai_communication_priority_chatter,
	_ai_communication_priority_talk,
	_ai_communication_priority_communicate,
	_ai_communication_priority_shout,
	_ai_communication_priority_yell,
	_ai_communication_priority_exclaim,
	NUMBER_OF_AI_COMMUNICATION_PRIORITIES,
};

enum
{
	_comm_team_human = 0,
	_comm_team_covenant,
	NUMBER_OF_AI_COMMUNICATION_TEAMS,
};

enum
{
	_comm_hostility_none = 0,
	_comm_hostility_self,
	_comm_hostility_friend,
	_comm_hostility_enemy,
	_comm_hostility_traitor,
	NUMBER_OF_AI_COMMUNICATION_HOSTILITIES,
};

enum
{
	_comm_enemy_never = 0,
	_comm_enemy_dead,
	_comm_enemy_lost,
	_comm_enemy_not_visible,
	_comm_enemy_not_dangerous,
	_comm_enemy_visible,
	NUMBER_OF_COMMUNICATION_ENEMY_STATUS_TYPES,
};

enum
{
	_comm_group_extended = 0,
	_comm_group_tactical,
	NUMBER_OF_COMMUNICATION_GROUP_TYPES,
};

enum
{
	_comm_protagonist_subject = 0,
	_comm_protagonist_cause,
	_comm_protagonist_friend,
	_comm_protagonist_target,
	_comm_protagonist_enemy,
	NUMBER_OF_COMMUNICATION_PROTAGONIST_TYPES,
};

enum
{
	_comm_look_direction_none = 0,
	_comm_look_direction_subject,
	_comm_look_direction_protagonist,
	_comm_look_direction_target,
	_comm_look_direction_danger,
	NUMBER_OF_AI_COMMUNICATION_LOOK_DIRECTIONS,
};

enum
{
	_dialogue_usage_lookup_bit = 0,
	_dialogue_usage_force_bit,
	_dialogue_usage_immediate_notify_bit,
	_dialogue_usage_player_bit,
	_dialogue_usage_same_vehicle_bit,
	_dialogue_usage_allow_subject_bit,
	_dialogue_usage_override_scripted_bit,
	NUMBER_OF_DIALOGUE_USAGE_FLAGS,
};

enum
{
	_reply_usage_override_scripted_bit = 0,
	NUMBER_OF_REPLY_USAGE_FLAGS,
};

enum
{
	_find_actor_allow_lookup_bit = 0,
	_find_actor_near_to_players_bit,
	_find_actor_same_vehicle_bit,
	_find_actor_allow_subject_bit,
	_find_actor_allow_cause_bit,
	NUMBER_OF_FIND_ACTOR_FLAGS,
};

enum
{
	MAXIMUM_COMMUNICATION_POSSIBILITIES = 16,
};

enum
{
	_communication_rating_normal = 0, /* fake name */
	_communication_rating_low, /* fake name */
	NUMBER_OF_COMMUNICATION_RATINGS, /* fake name */
};

enum
{
	_communication_timer_chatter = 0, /* fake name */
	_communication_timer_talk, /* fake name */
	_communication_timer_unit, /* fake name */
	_communication_timer_shout, /* fake name */
	_communication_timer_overlap, /* fake name */
	NUMBER_OF_COMMUNICATION_TIMERS, /* fake name */
};

enum
{
	_speech_disabled_by_chatter = 0, /* fake name */
	_speech_disabled_by_talk, /* fake name */
	_speech_disabled_by_shout, /* fake name */
	NUMBER_OF_SPEECH_DISABLED_REASONS, /* fake name */
};

enum
{
	_find_actor_mode_same_team = 0,
	_find_actor_mode_friend,
	_find_actor_mode_enemy,
	NUMBER_OF_FIND_ACTOR_MODES,
};

// These 4 are extremely fake names
#define FIND_ACTOR_WITNESS_DISTANCE 18.f /* fake name */
#define FIND_ACTOR_FRIEND_DISTANCE 10.f /* fake name */
#define FIND_ACTOR_ENEMY_DISTANCE 12.f /* fake name */
#define FIND_ACTOR_REPLY_DISTANCE 9.f /* fake name */

/* ---------- macros */

/* ---------- structures */

struct dialogue_usage
{
	short communication_type;
	short communication_priority;
	short vocalization_type;
	short animation_type;
	short protagonist_type;
	short protagonist_look_priority;
	short recipient_look_direction;
	short recipient_look_priority;
	real weight;
	real repeat_delay;
	short flags;
	short required_group;
	short required_hostility;
	short required_enemy_status;
	short required_subject_race;
	short required_cause_race;
	short required_damage;
};

struct reply_usage
{
	short original_vocalization_type;
	short original_damage_category;
	short protagonist_type;
	short vocalization_type;
	short animation_type;
	short communication_priority;
	word flags;
	real chance;
	real player_chance;
	real delay_time;
	real repeat_delay;
	boolean (*reply_filter)(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
};

struct dialogue_event_status
{
	long last_time_spoken;
	long disable_until_time;
};

struct communication_possibility
{
	real weight;
	boolean force;
	boolean spoken_by_player;
	short vocalization_type;
	short unit_speech_priority;
	short animation_type;
	short speech_play_type;
	short play_delay;
	short notification_delay;
	long protagonist_unit_index;
	long protagonist_actor_index;
	long target_unit_index;
	long preselected_reply_actor_index;
	short protagonist_look_priority;
	short recipient_look_priority;
	short recipient_look_type;
	struct ai_information_look_data recipient_look_data;
	long sound_definition_index;
	short dialogue_index;
};

/* ---------- prototypes */

static long ai_communication_find_actor_to_reply_to_player(long unit_index, long target_unit_index, short vocalization_type, short damage_category, real *reply_rating_reference);
static void actor_reset_idle_vocalization_timer(long actor_index);
static boolean reply_filter_close(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_not_close(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_searching(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_same_platoon(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_fighting(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_fighting_close(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_same_target(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_no_certain_target(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static boolean reply_filter_flee_leader(long original_unit_index, struct ai_information_packet *communication, long reply_actor_index);
static void ai_communication_handle_received_looking(long actor_index, long prop_index, struct ai_information_packet *information);
static short ai_communication_consider_speech(long unit_index, short communication_priority, short speech_priority, short delay_ticks, boolean allow_vocalization_lookup, boolean allow_recent_disabling, short *vocalization_type, real *weight, long *sound_definition_index_reference, char *debugstring);
static void ai_communication_update_speech_timers(long unit_index, short priority, short vocalization_type, short dialogue_type_index, short reply_table_index);
static real ai_communication_actor_talk_weight(long actor_index, long subject_unit_index, real_point3d *subject_point, long cause_unit_index, real_point3d *cause_point, real stimulus_range, short ai_communication_type, short ai_communication_priority, short unit_speech_priority, short vocalization_type, short animation_type, short flags);
static long ai_communication_find_specific_actor_to_talk(long ai_index, long subject_unit_index, long cause_unit_index, real max_distance, short ai_communication_type, short ai_communication_priority, short unit_speech_priority, short vocalization_type, short animation_type, short flags);
static long ai_communication_find_global_actor_to_talk(short team_index, short find_actor_mode, long subject_unit_index, long cause_unit_index, real max_distance, short ai_communication_type, short ai_communication_priority, short unit_speech_priority, short vocalization_type, short animation_type, short flags);
static void ai_communication_look_secondary_at_unit(long actor_index, short type, short priority, long look_unit_index, long prop_index);
static void ai_communication_look_secondary_at_object(long actor_index, short type, short priority, long object_index);
static boolean ai_conversation_begin(long conversation_index, boolean *continue_trying);
static long ai_conversation_new(short conversation_definition_index, boolean scripted);
static boolean ai_conversation_find_participant(long conversation_index, short participant_index, boolean *found_specific_unit_reference, boolean *try_alternate_reference, boolean *success_with_better_player_rating_reference, real *best_distance_reference);
static boolean ai_conversation_line_begin(long conversation_index);
static boolean ai_conversation_line_perform(long conversation_index);
static void ai_conversation_line_end(long conversation_index);

/* ---------- globals */

short const communication_speech_priorities[NUMBER_OF_AI_COMMUNICATION_PRIORITIES] =
{
	_unit_speech_none,
	_unit_speech_talk,
	_unit_speech_talk,
	_unit_speech_talk,
	_unit_speech_communicate,
	_unit_speech_shout,
	_unit_speech_shout,
	_unit_speech_exclamation,
};

real const communication_notification_delays[NUMBER_OF_AI_COMMUNICATION_PRIORITIES] =
{
	0.5f,
	0.5f,
	0.5f,
	0.5f,
	0.5f,
	0.5f,
	0.5f,
	0.3f,
};

short const communication_protagonist_default_look_priorities[NUMBER_OF_AI_COMMUNICATION_PRIORITIES] =
{
	_secondary_look_priority_none,
	_secondary_look_priority_aim,
	_secondary_look_priority_aim,
	_secondary_look_priority_turn_and_aim,
	_secondary_look_priority_stop_and_aim,
	_secondary_look_priority_stop_and_aim,
	_secondary_look_priority_stop_and_aim,
	_secondary_look_priority_aim,
};

short const communication_recipient_default_look_priorities[NUMBER_OF_AI_COMMUNICATION_PRIORITIES] =
{
	_secondary_look_priority_none,
	_secondary_look_priority_idle_aim,
	_secondary_look_priority_idle_aim,
	_secondary_look_priority_aim,
	_secondary_look_priority_turn_and_aim,
	_secondary_look_priority_turn_and_aim,
	_secondary_look_priority_stop_and_aim,
	_secondary_look_priority_aim,
};

short const communication_player_speaking_priorities[NUMBER_OF_AI_COMMUNICATION_PRIORITIES] =
{
	_ai_communication_priority_none,
	_ai_communication_priority_chatter,
	_ai_communication_priority_talk,
	_ai_communication_priority_communicate,
	_ai_communication_priority_communicate,
	_ai_communication_priority_yell,
	_ai_communication_priority_yell,
	_ai_communication_priority_exclaim,
};

short const communication_unit_prefer_silent_time = 2 * TICKS_PER_SECOND;

real const communication_timer_tolerances[NUMBER_OF_AI_COMMUNICATION_PRIORITIES][NUMBER_OF_COMMUNICATION_RATINGS][NUMBER_OF_COMMUNICATION_TIMERS] =
{
	{ { 0.f, 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f, 0.f } },
	{ { 4.f, 4.5f, 2.5f, 2.f, 0.f }, { 5.f, 5.f, 3.f, 2.5f, 0.f } },
	{ { 3.f, 3.5f, 2.f, 1.5f, 0.f }, { 4.5f, 4.5f, 2.5f, 2.f, 0.f } },
	{ { 0.3f, 1.f, 1.f, 1.f, 0.8f }, { 1.f, 2.f, 1.5f, 1.5f, 0.3f } },
	{ { 0.f, 0.5f, 0.5f, 0.5f, 1.3f }, { 0.2f, 1.f, 1.f, 1.f, 0.8f } },
	{ { 0.f, 0.f, 1.f, 1.f, 1.5f }, { 0.f, 0.f, 1.5f, 1.5f, 1.f } },
	{ { 0.f, 0.f, 0.5f, 0.f, 1.5f }, { 0.f, 0.f, 0.5f, 0.f, 1.5f } },
	{ { 0.f, 0.f, 0.5f, 0.f, 0.f }, { 0.f, 0.f, 0.5f, 0.f, 0.f } },
};

real const communication_play_delays[NUMBER_OF_COMMUNICATION_PROTAGONIST_TYPES] =
{
	0.f,
	0.5f,
	0.8f,
	0.5f,
	0.8f,
};

short const communication_player_additional_delay = TICKS_PER_SECOND;
short const communication_overlap_time_modifier = (3 * TICKS_PER_SECOND) / 2;
short const communication_timeout_low_priority_modifier = TICKS_PER_SECOND;
short const communication_repeat_selection_time = 30 * TICKS_PER_SECOND;
real const communication_player_absolute_range = 30.f;
real const communication_player_ideal_range_min = 3.f;
real const communication_player_ideal_range_max = 15.f;
real const communication_player_ideal_fov = _sqrt_2 / 2.f;
real const communication_player_rating_low_priority = 2.f;

struct dialogue_usage const global_dialogue_table[] =
{
	{ _ai_communication_death, _ai_communication_priority_talk, _vocalization_killed_friend, NONE, _comm_protagonist_cause, _secondary_look_priority_stop_and_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_friend, NONE, NONE, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_yell, _vocalization_killed_friend_player, NONE, _comm_protagonist_cause, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 20.f, 0.f, FLAG(_dialogue_usage_force_bit) | FLAG(_dialogue_usage_override_scripted_bit), NONE, _comm_hostility_friend, NONE, _race_player, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, FLAG(_dialogue_usage_player_bit), NONE, _comm_hostility_enemy, NONE, NONE, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_yell, _vocalization_killed_enemy_player, NONE, _comm_protagonist_cause, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 20.f, 0.f, FLAG(_dialogue_usage_force_bit) | FLAG(_dialogue_usage_player_bit) | FLAG(_dialogue_usage_override_scripted_bit), NONE, _comm_hostility_enemy, NONE, _race_player, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_covenant, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, FLAG(_dialogue_usage_player_bit), NONE, _comm_hostility_enemy, NONE, _race_covenant, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_floodcombat, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 40.f, 0.f, FLAG(_dialogue_usage_player_bit), NONE, _comm_hostility_enemy, NONE, _race_floodcombat, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_floodcarrier, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 40.f, 0.f, FLAG(_dialogue_usage_player_bit), NONE, _comm_hostility_enemy, NONE, _race_floodcarrier, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_sentinel, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 40.f, 0.f, FLAG(_dialogue_usage_player_bit), NONE, _comm_hostility_enemy, NONE, _race_sentinel, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_bullet, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_bullet },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_plasma, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_plasma },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_needler, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_needle },
	{ _ai_communication_death, _ai_communication_priority_talk, _vocalization_killed_enemy_sniper, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_sniper },
	{ _ai_communication_death, _ai_communication_priority_talk, _vocalization_killed_enemy_grenade, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_grenade },
	{ _ai_communication_death, _ai_communication_priority_talk, _vocalization_killed_enemy_explosion, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_highexplosive },
	{ _ai_communication_death, _ai_communication_priority_talk, _vocalization_killed_enemy_melee, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_melee },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_flame, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_flame },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_shotgun, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_shotgun },
	{ _ai_communication_death, _ai_communication_priority_talk, _vocalization_killed_enemy_vehicle, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 30.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_vehicle },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_killed_enemy_mountedweapon, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 30.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_mountedweapon },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_friend_died, NONE, _comm_protagonist_friend, _secondary_look_priority_default, _comm_look_direction_target, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_yell, _vocalization_friend_player_died, NONE, _comm_protagonist_friend, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 15.f, 0.f, FLAG(_dialogue_usage_force_bit) | FLAG(_dialogue_usage_override_scripted_bit), NONE, NONE, NONE, _race_player, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_talk, _vocalization_friend_killed_by_friend, NONE, _comm_protagonist_friend, _secondary_look_priority_turn_and_aim, _comm_look_direction_subject, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_friend, NONE, NONE, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_communicate, _vocalization_friend_killed_by_friend_player, NONE, _comm_protagonist_friend, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_turn_and_aim, 20.f, 0.f, FLAG(_dialogue_usage_force_bit) | FLAG(_dialogue_usage_override_scripted_bit), NONE, _comm_hostility_friend, NONE, NONE, _race_player, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_friend_killed_by_enemy, NONE, _comm_protagonist_friend, _secondary_look_priority_default, _comm_look_direction_target, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_friend_killed_by_enemy_player, NONE, _comm_protagonist_friend, _secondary_look_priority_default, _comm_look_direction_target, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, _race_player, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_friend_killed_by_covenant, NONE, _comm_protagonist_friend, _secondary_look_priority_default, _comm_look_direction_target, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, _race_covenant, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_friend_killed_by_flood, NONE, _comm_protagonist_friend, _secondary_look_priority_default, _comm_look_direction_target, _secondary_look_priority_default, 40.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, _race_flood, NONE },
	{ _ai_communication_death, _ai_communication_priority_chatter, _vocalization_friend_killed_by_sentinel, NONE, _comm_protagonist_friend, _secondary_look_priority_default, _comm_look_direction_target, _secondary_look_priority_default, 40.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, _race_sentinel, NONE },
	{ _ai_communication_death, _ai_communication_priority_shout, _vocalization_friend_betrayed, NONE, _comm_protagonist_friend, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 30.f, 0.f, FLAG(_dialogue_usage_force_bit), NONE, _comm_hostility_traitor, NONE, NONE, NONE, NONE },
	{ _ai_communication_killing_spree, _ai_communication_priority_communicate, _vocalization_killing_spree, NONE, _comm_protagonist_subject, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 30.f, 0.f, FLAG(_dialogue_usage_player_bit), NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_damage, _ai_communication_priority_talk, _vocalization_shot_friend, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_friend, NONE, NONE, NONE, NONE },
	{ _ai_communication_damage, _ai_communication_priority_talk, _vocalization_shot_friend_player, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, FLAG(_dialogue_usage_force_bit), NONE, _comm_hostility_friend, NONE, _race_player, NONE, NONE },
	{ _ai_communication_damage, _ai_communication_priority_talk, _vocalization_hurt_friend, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_target, _secondary_look_priority_default, 10.f, 0.f, FLAG(_dialogue_usage_player_bit), NONE, _comm_hostility_friend, NONE, NONE, NONE, NONE },
	{ _ai_communication_damage, _ai_communication_priority_talk, _vocalization_hurt_friend_player, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_default, 10.f, 5.f, 0, NONE, _comm_hostility_friend, NONE, NONE, _race_player, NONE },
	{ _ai_communication_hurt, _ai_communication_priority_filler, _vocalization_shot_enemy, NONE, _comm_protagonist_cause, _secondary_look_priority_default, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, NONE },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, NONE },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy_bullet, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_bullet },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy_needler, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_needle },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy_plasma, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_plasma },
	{ _ai_communication_hurt, _ai_communication_priority_talk, _vocalization_hurt_enemy_sniper, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_sniper },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy_explosion, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_highexplosive },
	{ _ai_communication_hurt, _ai_communication_priority_talk, _vocalization_hurt_enemy_melee, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_melee },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy_flame, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_flame },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy_shotgun, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 20.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_shotgun },
	{ _ai_communication_hurt, _ai_communication_priority_talk, _vocalization_hurt_enemy_vehicle, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 30.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_vehicle },
	{ _ai_communication_hurt, _ai_communication_priority_chatter, _vocalization_hurt_enemy_mountedweapon, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 30.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, _damage_category_mountedweapon },
	{ _ai_communication_sighted_enemy, _ai_communication_priority_shout, _vocalization_sighted_enemy_new, _unit_animation_impulse_signal_warn, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 10.f, 10.f, 0, NONE, NONE, _comm_enemy_never, NONE, NONE, NONE },
	{ _ai_communication_sighted_enemy, _ai_communication_priority_shout, _vocalization_sighted_enemy_recent, _unit_animation_impulse_signal_warn, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 10.f, 25.f, 0, NONE, NONE, _comm_enemy_lost, NONE, NONE, NONE },
	{ _ai_communication_found_enemy, _ai_communication_priority_shout, _vocalization_sighted_enemy_searching, _unit_animation_impulse_signal_warn, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 10.f, 25.f, 0, NONE, NONE, _comm_enemy_lost, NONE, NONE, NONE },
	{ _ai_communication_unexpected_enemy, _ai_communication_priority_talk, _vocalization_unexpected_enemy, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_aim, 10.f, 8.f, FLAG(_dialogue_usage_lookup_bit), NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_found_dead_friend, _ai_communication_priority_shout, _vocalization_dead_friend_found, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 10.f, 20.f, 0, NONE, NONE, _comm_enemy_not_dangerous, NONE, NONE, NONE },
	{ _ai_communication_allegiance_changed, _ai_communication_priority_yell, _vocalization_allegiance_broken, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_stop_and_aim, 10.f, 0.f, FLAG(_dialogue_usage_immediate_notify_bit) | FLAG(_dialogue_usage_override_scripted_bit), NONE, _comm_hostility_traitor, NONE, NONE, NONE, NONE },
	{ _ai_communication_allegiance_changed, _ai_communication_priority_communicate, _vocalization_allegiance_reformed, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_aim, 10.f, 0.f, FLAG(_dialogue_usage_override_scripted_bit), NONE, _comm_hostility_friend, NONE, NONE, NONE, NONE },
	{ _ai_communication_sighted_friend_player, _ai_communication_priority_communicate, _vocalization_sighted_friend_player, _unit_animation_impulse_signal_warn, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_target, _secondary_look_priority_turn_and_aim, 10.f, 15.f, 0, NONE, NONE, _comm_enemy_not_dangerous, NONE, NONE, NONE },
	{ _ai_communication_lost_contact, _ai_communication_priority_communicate, _vocalization_lost_contact, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 20.f, 0, NONE, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_lost_contact, _ai_communication_priority_communicate, _vocalization_alert_lost_contact, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 15.f, 20.f, 0, _comm_group_extended, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_alert_noncombat, _ai_communication_priority_communicate, _vocalization_alert_noncombat, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_turn_and_aim, 10.f, 20.f, 0, NONE, NONE, _comm_enemy_lost, NONE, NONE, NONE },
	{ _ai_communication_blocked, _ai_communication_priority_chatter, _vocalization_blocked, _unit_animation_impulse_signal_move, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, _comm_group_tactical, NONE, NONE, NONE, NONE, _damage_category_none },
	{ _ai_communication_search_start, _ai_communication_priority_talk, _vocalization_search_start, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 8.f, 0, _comm_group_extended, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_search_query, _ai_communication_priority_talk, _vocalization_search_query, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 25.f, 0, _comm_group_extended, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_search_report, _ai_communication_priority_talk, _vocalization_search_report, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 25.f, 0, _comm_group_extended, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_search_abandon, _ai_communication_priority_talk, _vocalization_search_abandon, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 8.f, 0, _comm_group_extended, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_search_group_abandon, _ai_communication_priority_communicate, _vocalization_search_group_abandon, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, _comm_group_extended, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_uncover_start, _ai_communication_priority_talk, _vocalization_uncover_start, _unit_animation_impulse_signal_attack, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 30.f, 0, _comm_group_tactical, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_cover, _ai_communication_priority_chatter, _vocalization_cover, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 30.f, 0, _comm_group_tactical, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_advance, _ai_communication_priority_communicate, _vocalization_advance, _unit_animation_impulse_signal_attack, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_retreat, _ai_communication_priority_communicate, _vocalization_retreat, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_shooting, _ai_communication_priority_filler, _vocalization_shooting, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_shooting_vehicle, _ai_communication_priority_filler, _vocalization_shooting_vehicle, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 20.f, 10.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_shooting_berserk, _ai_communication_priority_chatter, _vocalization_shooting_berserk, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 20.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_shooting_group, _ai_communication_priority_chatter, _vocalization_shooting_group, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_shooting_traitor, _ai_communication_priority_filler, _vocalization_shooting_traitor, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 30.f, 10.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_flee, _ai_communication_priority_talk, _vocalization_flee, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_flee_leader_died, _ai_communication_priority_communicate, _vocalization_flee_leader_died, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_flee_idle, _ai_communication_priority_talk, _vocalization_idle_flee, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_attempted_flee, _ai_communication_priority_chatter, _vocalization_attempted_flee, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_turn_and_aim, 10.f, 10.f, 0, _comm_group_tactical, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_hiding_finished, _ai_communication_priority_talk, _vocalization_hiding_finished, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 30.f, 0, _comm_group_tactical, NONE, _comm_enemy_not_visible, NONE, NONE, NONE },
	{ _ai_communication_vehicle_entry, _ai_communication_priority_talk, _vocalization_vehicle_entry, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_vehicle_exit, _ai_communication_priority_talk, _vocalization_vehicle_exit, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 10.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_vehicle_woohoo, _ai_communication_priority_chatter, _vocalization_vehicle_woohoo, _unit_animation_impulse_vehicle_celebrate, _comm_protagonist_friend, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 25.f, FLAG(_dialogue_usage_same_vehicle_bit) | FLAG(_dialogue_usage_allow_subject_bit), NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_vehicle_scared, _ai_communication_priority_chatter, _vocalization_vehicle_scared, _unit_animation_impulse_vehicle_panic, _comm_protagonist_friend, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 25.f, FLAG(_dialogue_usage_same_vehicle_bit) | FLAG(_dialogue_usage_allow_subject_bit), NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_vehicle_falling, _ai_communication_priority_exclaim, _vocalization_scream_fear, _unit_animation_impulse_vehicle_panic, _comm_protagonist_friend, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, FLAG(_dialogue_usage_same_vehicle_bit) | FLAG(_dialogue_usage_allow_subject_bit), NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_grenade_sighted, _ai_communication_priority_talk, _vocalization_grenade_sighted, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_danger, _secondary_look_priority_stop_and_aim, 10.f, 4.f, 0, _comm_group_extended, _comm_hostility_enemy, NONE, NONE, NONE, NONE },
	{ _ai_communication_grenade_startle, _ai_communication_priority_talk, _vocalization_grenade_startle, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_danger, _secondary_look_priority_stop_and_aim, 10.f, 4.f, 0, NONE, NONE, _comm_enemy_not_dangerous, NONE, NONE, NONE },
	{ _ai_communication_grenade_danger, _ai_communication_priority_shout, _vocalization_grenade_danger_enemy, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_danger, _secondary_look_priority_turn_and_aim, 10.f, 4.f, 0, _comm_group_extended, _comm_hostility_enemy, NONE, NONE, NONE, NONE },
	{ _ai_communication_grenade_danger, _ai_communication_priority_shout, _vocalization_grenade_danger_friend, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_danger, _secondary_look_priority_turn_and_aim, 10.f, 0.f, 0, _comm_group_extended, _comm_hostility_friend, NONE, NONE, NONE, NONE },
	{ _ai_communication_grenade_danger, _ai_communication_priority_yell, _vocalization_grenade_danger_self, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_danger, _secondary_look_priority_turn_and_aim, 10.f, 0.f, 0, NONE, _comm_hostility_self, NONE, NONE, NONE, NONE },
	{ _ai_communication_surprise, _ai_communication_priority_exclaim, _vocalization_surprise, NONE, _comm_protagonist_subject, _secondary_look_priority_stop_and_aim, _comm_look_direction_protagonist, _secondary_look_priority_aim, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_berserk, _ai_communication_priority_exclaim, _vocalization_berserk, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_melee, _ai_communication_priority_exclaim, _vocalization_melee, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_grenade_throwing, _ai_communication_priority_exclaim, _vocalization_grenade_throwing, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_dive, _ai_communication_priority_exclaim, _vocalization_dive, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_leap, _ai_communication_priority_exclaim, _vocalization_leap, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_falling_to_death, _ai_communication_priority_exclaim, _vocalization_scream_fear, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_alone, _ai_communication_priority_talk, _vocalization_postcombat_alone, NONE, _comm_protagonist_subject, _secondary_look_priority_turn_and_aim, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_unscathed, _ai_communication_priority_talk, _vocalization_postcombat_unscathed, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_wounded, _ai_communication_priority_talk, _vocalization_postcombat_wounded, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_massacre, _ai_communication_priority_talk, _vocalization_postcombat_massacre, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_triumph, _ai_communication_priority_talk, _vocalization_postcombat_triumph, NONE, _comm_protagonist_subject, _secondary_look_priority_none, _comm_look_direction_none, _secondary_look_priority_none, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_check_enemy, _ai_communication_priority_talk, _vocalization_check_body_enemy, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_enemy, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_check_friend, _ai_communication_priority_talk, _vocalization_check_body_friend, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, _comm_hostility_friend, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_shoot_corpse, _ai_communication_priority_talk, _vocalization_shoot_corpse, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ _ai_communication_postcombat_shoot_corpse, _ai_communication_priority_yell, _vocalization_shoot_corpse_player, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, FLAG(_dialogue_usage_force_bit), NONE, NONE, NONE, NONE, _race_player, NONE },
	{ _ai_communication_postcombat_celebrate, _ai_communication_priority_talk, _vocalization_celebrate, NONE, _comm_protagonist_subject, _secondary_look_priority_aim, _comm_look_direction_protagonist, _secondary_look_priority_default, 10.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
	{ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, 0.f, 0.f, 0, NONE, NONE, NONE, NONE, NONE, NONE },
};

struct reply_usage const global_reply_table[] =
{
	{ _vocalization_killing_spree, NONE, _comm_protagonist_friend, _vocalization_player_killing_spree_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 1.f, 0.7f, 30.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_bullet, _comm_protagonist_friend, _vocalization_player_kill_bullet_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.5f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_needle, _comm_protagonist_friend, _vocalization_player_kill_needler_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.6f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_plasma, _comm_protagonist_friend, _vocalization_player_kill_plasma_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.6f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_sniper, _comm_protagonist_friend, _vocalization_player_kill_sniper_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.8f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_grenade, _comm_protagonist_friend, _vocalization_player_kill_grenade_comment, NONE, _ai_communication_priority_chatter, 0, 0.5f, 0.9f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_highexplosive, _comm_protagonist_friend, _vocalization_player_kill_explosion_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 1.f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_melee, _comm_protagonist_friend, _vocalization_player_kill_melee_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 1.f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_flame, _comm_protagonist_friend, _vocalization_player_kill_flame_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.9f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_shotgun, _comm_protagonist_friend, _vocalization_player_kill_shotgun_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.6f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_vehicle, _comm_protagonist_friend, _vocalization_player_kill_vehicle_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.8f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy, _damage_category_mountedweapon, _comm_protagonist_friend, _vocalization_player_kill_mountedweapon_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 1.f, 0.7f, 60.f, reply_filter_close },
	{ _vocalization_killed_enemy_player, NONE, _comm_protagonist_friend, _vocalization_killed_enemy_player_comment, NONE, _ai_communication_priority_communicate, FLAG(_reply_usage_override_scripted_bit), 1.f, 1.f, 0.3f, 0.f, reply_filter_close },
	{ _vocalization_killed_enemy_covenant, NONE, _comm_protagonist_friend, _vocalization_killed_enemy_covenant_comment, NONE, _ai_communication_priority_chatter, 0, 0.8f, 0.6f, 0.5f, 30.f, reply_filter_close },
	{ _vocalization_killed_enemy_floodcombat, NONE, _comm_protagonist_friend, _vocalization_killed_enemy_floodcombat_comment, NONE, _ai_communication_priority_chatter, 0, 0.8f, 0.6f, 0.5f, 20.f, reply_filter_close },
	{ _vocalization_killed_enemy_floodcarrier, NONE, _comm_protagonist_friend, _vocalization_killed_enemy_floodcarrier_comment, NONE, _ai_communication_priority_chatter, 0, 0.8f, 0.6f, 0.5f, 20.f, reply_filter_close },
	{ _vocalization_killed_enemy_sentinel, NONE, _comm_protagonist_friend, _vocalization_killed_enemy_sentinel_comment, NONE, _ai_communication_priority_chatter, 0, 0.8f, 0.6f, 0.5f, 20.f, reply_filter_close },
	{ _vocalization_killed_enemy, NONE, _comm_protagonist_friend, _vocalization_killed_enemy_comment, NONE, _ai_communication_priority_chatter, 0, 0.6f, 0.4f, 0.5f, 30.f, reply_filter_close },
	{ _vocalization_killed_enemy, NONE, _comm_protagonist_friend, _vocalization_player_kill_comment, NONE, _ai_communication_priority_chatter, 0, 0.f, 0.4f, 0.7f, 40.f, reply_filter_close },
	{ _vocalization_shot_enemy, NONE, _comm_protagonist_friend, _vocalization_shot_enemy_comment, NONE, _ai_communication_priority_filler, 0, 0.8f, 0.f, 0.7f, 20.f, NULL },
	{ _vocalization_hurt_enemy, NONE, _comm_protagonist_enemy, _vocalization_hurt_enemy_reply, NONE, _ai_communication_priority_filler, 0, 0.8f, 0.f, 0.7f, 20.f, NULL },
	{ _vocalization_hurt_enemy, NONE, _comm_protagonist_friend, _vocalization_hurt_enemy_comment, NONE, _ai_communication_priority_filler, 0, 0.8f, 0.f, 0.7f, 20.f, NULL },
	{ _vocalization_killed_friend_player, NONE, _comm_protagonist_friend, _vocalization_killed_friend_player_comment, NONE, _ai_communication_priority_communicate, FLAG(_reply_usage_override_scripted_bit), 1.f, 0.f, 0.3f, 0.f, reply_filter_close },
	{ _vocalization_killed_friend, NONE, _comm_protagonist_friend, _vocalization_killed_friend_comment, NONE, _ai_communication_priority_talk, 0, 0.7f, 0.f, 0.3f, 20.f, reply_filter_close },
	{ _vocalization_hurt_friend, NONE, _comm_protagonist_target, _vocalization_hurt_friend_reply, NONE, _ai_communication_priority_chatter, 0, 0.7f, 0.4f, 0.5f, 20.f, NULL },
	{ _vocalization_sighted_enemy_new, NONE, _comm_protagonist_friend, _vocalization_sighted_enemy_near_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.7f, 20.f, reply_filter_close },
	{ _vocalization_sighted_enemy_new, NONE, _comm_protagonist_friend, _vocalization_sighted_enemy_far_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.7f, 20.f, reply_filter_not_close },
	{ _vocalization_sighted_enemy_recent, NONE, _comm_protagonist_friend, _vocalization_sighted_enemy_near_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.7f, 20.f, reply_filter_close },
	{ _vocalization_sighted_enemy_recent, NONE, _comm_protagonist_friend, _vocalization_sighted_enemy_far_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.7f, 20.f, reply_filter_not_close },
	{ _vocalization_sighted_enemy_searching, NONE, _comm_protagonist_friend, _vocalization_sighted_enemy_near_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.7f, 20.f, reply_filter_close },
	{ _vocalization_sighted_enemy_searching, NONE, _comm_protagonist_friend, _vocalization_sighted_enemy_far_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.7f, 20.f, reply_filter_not_close },
	{ _vocalization_alert_noncombat, NONE, _comm_protagonist_target, _vocalization_alert_noncombat_reply, NONE, _ai_communication_priority_talk, 0, 0.7f, 0.f, 0.5f, 15.f, NULL },
	{ _vocalization_alert_lost_contact, NONE, _comm_protagonist_friend, _vocalization_alert_lost_contact_reply, NONE, _ai_communication_priority_talk, 0, 0.7f, 0.f, 0.5f, 30.f, reply_filter_no_certain_target },
	{ _vocalization_blocked, NONE, _comm_protagonist_target, _vocalization_blocked_reply, NONE, _ai_communication_priority_talk, 0, 0.5f, 0.f, 0.5f, 20.f, NULL },
	{ _vocalization_search_query, NONE, _comm_protagonist_friend, _vocalization_search_query_reply, NONE, _ai_communication_priority_talk, 0, 1.f, 0.f, 0.3f, 20.f, reply_filter_searching },
	{ _vocalization_uncover_start, NONE, _comm_protagonist_friend, _vocalization_uncover_start_reply, NONE, _ai_communication_priority_talk, 0, 1.f, 0.f, 0.3f, 20.f, reply_filter_same_target },
	{ _vocalization_advance, NONE, _comm_protagonist_friend, _vocalization_advance_reply, NONE, _ai_communication_priority_communicate, 0, 0.7f, 0.f, 0.7f, 20.f, reply_filter_same_platoon },
	{ _vocalization_retreat, NONE, _comm_protagonist_friend, _vocalization_retreat_reply, NONE, _ai_communication_priority_communicate, 0, 0.7f, 0.f, 0.7f, 20.f, reply_filter_same_platoon },
	{ _vocalization_flee, NONE, _comm_protagonist_friend, _vocalization_flee_reply, NONE, _ai_communication_priority_talk, 0, 0.5f, 0.f, 0.7f, 30.f, reply_filter_fighting_close },
	{ _vocalization_flee, NONE, _comm_protagonist_enemy, _vocalization_taunt, NONE, _ai_communication_priority_talk, 0, 0.5f, 0.f, 0.7f, 30.f, reply_filter_fighting },
	{ _vocalization_flee_leader_died, NONE, _comm_protagonist_enemy, _vocalization_taunt, NONE, _ai_communication_priority_talk, 0, 0.5f, 0.f, 0.7f, 30.f, reply_filter_fighting },
	{ _vocalization_attempted_flee, NONE, _comm_protagonist_friend, _vocalization_attempted_flee_reply, NONE, _ai_communication_priority_talk, 0, 0.5f, 0.f, 0.7f, 20.f, reply_filter_flee_leader },
	{ _vocalization_postcombat_wounded, NONE, _comm_protagonist_friend, _vocalization_postcombat_wounded_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.5f, 0.f, NULL },
	{ _vocalization_postcombat_massacre, NONE, _comm_protagonist_friend, _vocalization_postcombat_massacre_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.5f, 0.f, NULL },
	{ _vocalization_postcombat_triumph, NONE, _comm_protagonist_friend, _vocalization_postcombat_triumph_reply, NONE, _ai_communication_priority_talk, 0, 0.8f, 0.f, 0.5f, 0.f, NULL },
	{ NONE, NONE, NONE, NONE, NONE, NONE, 0, 0.f, 0.f, 0.f, 0.f, NULL },
};

char const *global_communication_priority_names[NUMBER_OF_AI_COMMUNICATION_PRIORITIES] =
{
	"none",
	"filler",
	"chatter",
	"talk",
	"communicate",
	"shout",
	"yell",
	"exclaim",
};

char const *global_communication_team_names[NUMBER_OF_AI_COMMUNICATION_TEAMS][2] =
{
	{ "human", "HUM" },
	{ "covenant", "COV" },
};

char const *global_communication_type_names[NUMBER_OF_AI_COMMUNICATION_TYPES] =
{
	"death",
	"killing_spree",
	"hurt",
	"damage",
	"sighted_enemy",
	"found_enemy",
	"unexpected_enemy",
	"found_dead_friend",
	"allegiance_changed",
	"grenade_throwing",
	"grenade_startle",
	"grenade_sighted",
	"grenade_danger",
	"lost_contact",
	"blocked",
	"alert_noncombat",
	"search_start",
	"search_query",
	"search_report",
	"search_abandon",
	"search_group_abandon",
	"uncover_start",
	"advance",
	"retreat",
	"cover",
	"sighted_friend_player",
	"shooting",
	"shooting_vehicle",
	"shooting_berserk",
	"shooting_group",
	"shooting_traitor",
	"flee",
	"flee_leader_died",
	"flee_idle",
	"attempted_flee",
	"hiding_finished",
	"vehicle_entry",
	"vehicle_exit",
	"vehicle_woohoo",
	"vehicle_scared",
	"vehicle_falling",
	"surprise",
	"berserk",
	"melee",
	"dive",
	"uncover_exclamation",
	"falling",
	"leap",
	"postcombat_alone",
	"postcombat_unscathed",
	"postcombat_wounded",
	"postcombat_massacre",
	"postcombat_triumph",
	"postcombat_check_enemy",
	"postcombat_check_friend",
	"postcombat_shoot_corpse",
	"postcombat_celebrate",
};

short global_dialogue_event_count = 0;
struct dialogue_event_status *global_dialogue_events = NULL;
short global_reply_event_count = 0;
struct dialogue_event_status *global_reply_events = NULL;

short global_communication_table_indices[NUMBER_OF_AI_COMMUNICATION_TYPES];
struct data_array *conversation_data;

/* ---------- public code */

void ai_communication_initialize(
	void)
{
	struct dialogue_usage const *dialogue;
	struct reply_usage const *reply;
	short communication_type;

	global_dialogue_event_count = 0;

	for (dialogue = global_dialogue_table; dialogue->communication_type != NONE; dialogue++)
	{
		global_dialogue_event_count++;
	}

	if (!global_dialogue_events)
	{
		global_dialogue_events = game_state_malloc(
			"ai communication dialogue",
			NULL,
			NUMBER_OF_AI_COMMUNICATION_TEAMS * global_dialogue_event_count * sizeof(*global_dialogue_events));
		match_vassert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 646, global_dialogue_events, "ai_communication_initialize: unable to allocate comm dialogue status table");
	}

	global_reply_event_count = 0;

	for (reply = global_reply_table; reply->original_vocalization_type != NONE; reply++)
	{
		global_reply_event_count++;
	}

	if (!global_reply_events)
	{
		global_reply_events = game_state_malloc(
			"ai communication replies",
			NULL,
			NUMBER_OF_AI_COMMUNICATION_TEAMS * global_reply_event_count * sizeof(*global_reply_events));
		match_vassert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 659, global_reply_events, "ai_communication_initialize: unable to allocate comm reply status table");
	}

	for (communication_type = 0; communication_type < NUMBER_OF_AI_COMMUNICATION_TYPES; communication_type++)
	{
		short dialogue_index = 0;

		global_communication_table_indices[communication_type] = NONE;

		for (dialogue = global_dialogue_table; dialogue->communication_type != NONE; dialogue++)
		{
			if (dialogue->communication_type == communication_type)
			{
				global_communication_table_indices[communication_type] = dialogue_index;
				break;
			}

			dialogue_index++;
		}
	}

	conversation_data = game_state_data_new("ai conversation", 8, sizeof(struct conversation_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 680, conversation_data);

	return;
}

void ai_communication_dispose(
	void)
{
	return;
}

void ai_communication_initialize_for_new_map(
	void)
{
	short event_index;

	ai_globals->dialogue_triggers_enabled = TRUE;
	memset(ai_globals->last_chatter_time, 0, sizeof(ai_globals->last_chatter_time));
	memset(ai_globals->last_talk_time, 0, sizeof(ai_globals->last_talk_time));
	memset(ai_globals->last_shout_time, 0, sizeof(ai_globals->last_shout_time));

	for (event_index = 0; event_index < NUMBER_OF_AI_COMMUNICATION_TEAMS * global_dialogue_event_count; event_index++)
	{
		global_dialogue_events[event_index].disable_until_time = NONE;
		global_dialogue_events[event_index].last_time_spoken = NONE;
	}

	for (event_index = 0; event_index < NUMBER_OF_AI_COMMUNICATION_TEAMS * global_reply_event_count; event_index++)
	{
		global_reply_events[event_index].disable_until_time = NONE;
		global_reply_events[event_index].last_time_spoken = NONE;
	}

	ai_globals->recent_conversation_count = 0;
	ai_globals->recent_conversation_next_index = 0;
	memset(ai_globals->recent_conversations, 0, sizeof(ai_globals->recent_conversations));
	data_make_valid(conversation_data);

	return;
}

void ai_communication_dispose_from_old_map(
	void)
{
	data_make_invalid(conversation_data);

	return;
}

char const *ai_communication_get_type_name(
	short communication_type)
{
	char const *name = "<error>";

	if (communication_type >= 0 && communication_type < NUMBER_OF_AI_COMMUNICATION_TYPES)
	{
		name = global_communication_type_names[communication_type];
	}

	return name;
}

short ai_communication_get_type_by_name(
	char const *name)
{
	short communication_type;
	short result = NONE;

	for (communication_type = 0; communication_type < NUMBER_OF_AI_COMMUNICATION_TYPES; communication_type++)
	{
		if (!strcmp(global_communication_type_names[communication_type], name))
		{
			result = communication_type;
		}
	}

	return result;
}

void ai_communication_packet_new(
	struct ai_information_packet *information)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 768, information);
	memset(information, 0, sizeof(*information));
	information->target_unit_index = NONE;
	information->communication_type = NONE;
	information->dialogue_type_index = NONE;
	information->damage_category = NONE;

	return;
}

void ai_communication_event(
	short communication_type,
	long subject_unit_index,
	long cause_unit_index,
	short hostility,
	short damage_type,
	short information_type,
	struct ai_information_data *information_data)
{
	struct communication_possibility possibilities[MAXIMUM_COMMUNICATION_POSSIBILITIES];
	struct dialogue_usage const *usage;
	short dialogue_index;
	char printbuffer[1024];
	char silent_printbuffer[1024];
	short time_since_shout[NUMBER_OF_AI_COMMUNICATION_TEAMS];
	short time_since_talk[NUMBER_OF_AI_COMMUNICATION_TEAMS];
	short time_since_chatter[NUMBER_OF_AI_COMMUNICATION_TEAMS];
	real seconds_since_shout[NUMBER_OF_AI_COMMUNICATION_TEAMS];
	real seconds_since_talk[NUMBER_OF_AI_COMMUNICATION_TEAMS];
	real seconds_since_chatter[NUMBER_OF_AI_COMMUNICATION_TEAMS];
	short speech_disabled_reason[NUMBER_OF_AI_COMMUNICATION_TEAMS][NUMBER_OF_AI_COMMUNICATION_PRIORITIES][NUMBER_OF_COMMUNICATION_RATINGS];
	boolean enemy_status_enabled[NUMBER_OF_COMMUNICATION_ENEMY_STATUS_TYPES];
	boolean hostility_enabled[NUMBER_OF_AI_COMMUNICATION_HOSTILITIES];
	boolean subject_comm_groups_enabled[NUMBER_OF_COMMUNICATION_GROUP_TYPES];
	boolean cause_comm_groups_enabled[NUMBER_OF_COMMUNICATION_GROUP_TYPES];
	boolean speech_disabled[NUMBER_OF_AI_COMMUNICATION_TEAMS][NUMBER_OF_AI_COMMUNICATION_PRIORITIES][NUMBER_OF_COMMUNICATION_RATINGS];
	short speech_enable_after_ticks[NUMBER_OF_AI_COMMUNICATION_TEAMS][NUMBER_OF_AI_COMMUNICATION_PRIORITIES][NUMBER_OF_COMMUNICATION_RATINGS];
	short group_index;
	short team_index;
	long current_time = game_time_get();
	boolean speech_started = FALSE;
	short num_possibilities = 0;
	real total_possibility_weight = 0.f;
	boolean forced_possibilities = FALSE;
	long subject_encounter_index = NONE;
	struct encounter_datum *subject_encounter = NULL;
	long subject_actor_index = NONE;
	struct unit_datum *subject_unit = NULL;
	struct actor_datum *subject_actor = NULL;
	long cause_actor_index = NONE;
	struct unit_datum *cause_unit = NULL;
	struct actor_datum *cause_actor = NULL;
	long friend_actor_index = NONE;
	long enemy_actor_index = NONE;
	short subject_team_index = NONE;
	short cause_team_index = NONE;
	short subject_race = _race_none;
	short cause_race = _race_none;
	boolean must_find_friend = TRUE;
	boolean must_find_enemy = TRUE;
	boolean suppress_print = FALSE;
	boolean player_involved = FALSE;
	boolean any_silent_rejection = FALSE;
	boolean any_printed = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 846, (communication_type >= 0) && (communication_type < NUMBER_OF_AI_COMMUNICATION_TYPES));

	if (hostility == NONE)
	{
		hostility = _comm_hostility_none;
	}

	if (damage_type == NONE)
	{
		damage_type = _damage_category_none;
	}

	for (group_index = 0; group_index < NUMBER_OF_COMMUNICATION_GROUP_TYPES; group_index++)
	{
		subject_comm_groups_enabled[group_index] = FALSE;
		cause_comm_groups_enabled[group_index] = FALSE;
	}

	if (subject_unit_index != NONE)
	{
		subject_unit = unit_get(subject_unit_index);
		subject_actor_index = subject_unit->unit.actor_index;
		subject_team_index = subject_unit->object.owner_team_index;
		subject_race = ai_get_race_from_team_index(subject_team_index);

		if (subject_actor_index != NONE)
		{
			subject_actor = actor_get(subject_actor_index);
			subject_encounter_index = subject_actor->meta.encounter_index;
			subject_race = actor_type_get_race(subject_actor->meta.type);

			if (subject_actor->situation.close_friends > 0)
			{
				subject_comm_groups_enabled[_comm_group_extended] = TRUE;
				subject_comm_groups_enabled[_comm_group_tactical] = TRUE;
			}
			else if (subject_actor->situation.area_friends > 0)
			{
				subject_comm_groups_enabled[_comm_group_extended] = TRUE;
				subject_comm_groups_enabled[_comm_group_tactical] = FALSE;
			}

			if (subject_encounter_index != NONE)
			{
				subject_encounter = encounter_get(subject_encounter_index);
			}
		}
		else if (subject_unit->unit.player_index != NONE)
		{
			subject_race = _race_player;
		}
	}

	if (cause_unit_index != NONE)
	{
		cause_unit = unit_get(cause_unit_index);
		cause_actor_index = cause_unit->unit.actor_index;
		cause_team_index = cause_unit->object.owner_team_index;
		cause_race = ai_get_race_from_team_index(cause_team_index);

		if (cause_actor_index != NONE)
		{
			cause_actor = actor_get(cause_actor_index);
			cause_race = actor_type_get_race(cause_actor->meta.type);

			if (cause_actor->situation.close_friends > 0)
			{
				cause_comm_groups_enabled[_comm_group_extended] = TRUE;
				cause_comm_groups_enabled[_comm_group_tactical] = TRUE;
			}
			else if (cause_actor->situation.area_friends > 0)
			{
				cause_comm_groups_enabled[_comm_group_extended] = TRUE;
				cause_comm_groups_enabled[_comm_group_tactical] = FALSE;
			}
		}
		else if (cause_unit->unit.player_index != NONE)
		{
			cause_race = _race_player;
		}
	}

	if (subject_unit && cause_unit && subject_team_index != cause_team_index && game_team_is_ally(subject_team_index, cause_team_index))
	{
		boolean betrayal = FALSE;
		boolean observed = FALSE;

		if (communication_type == _ai_communication_death)
		{
			if (hostility == _comm_hostility_enemy)
			{
				betrayal = TRUE;
				observed = TRUE;
			}
			else
			{
				if (subject_encounter)
				{
					betrayal = subject_encounter->enemy_traitor || subject_encounter->enemy_visible_timer == NONE || subject_encounter->enemy_visible_timer >= 9 * TICKS_PER_SECOND;
				}

				friend_actor_index = ai_communication_find_global_actor_to_talk(
					subject_team_index,
					_find_actor_mode_same_team,
					subject_unit_index,
					cause_unit_index,
					FIND_ACTOR_WITNESS_DISTANCE,
					_ai_communication_death,
					_ai_communication_priority_yell,
					NONE,
					NONE,
					NONE,
					0);

				if (friend_actor_index != NONE)
				{
					must_find_friend = FALSE;
					observed = TRUE;
				}

				switch (damage_type)
				{
				case _damage_category_grenade:
				case _damage_category_highexplosive:
				case _damage_category_vehicle:
					if (!betrayal)
					{
						observed = FALSE;
					}
					break;
				}

				if (damage_type == _damage_category_grenade)
				{
					betrayal = FALSE;
				}
			}

			if (betrayal)
			{
				hostility = _comm_hostility_traitor;
			}

			if (ai_debug.print_allegiance)
			{
				console_printf(
					FALSE,
					"incident between teams %s and %s: %s, %s",
					global_game_team_names[subject_team_index],
					global_game_team_names[cause_team_index],
					betrayal ? "betrayal" : "accident",
					observed ? "observed" : "unobserved");
			}

			if (observed)
			{
				boolean notify_immediately = FALSE;
				boolean allegiance_broken = game_allegiance_incident(
					cause_team_index,
					subject_team_index,
					betrayal ? _allegiance_incident_betrayal : _allegiance_incident_accident,
					&notify_immediately);

				if (notify_immediately)
				{
					ai_handle_allegiance_broken_notification(cause_team_index, subject_team_index, allegiance_broken);
				}

				if (ai_debug.print_allegiance && !allegiance_broken)
				{
					short threshold;
					short incidents = game_allegiance_get_incidents(cause_team_index, subject_team_index, &threshold);

					console_printf(
						FALSE,
						"allegiance %s, %d incidents (threshold %d)",
						allegiance_broken ? "broken" : "still holds",
						incidents,
						(threshold == NONE) ? 999 : threshold);
				}
			}
		}

		if (game_team_is_enemy(subject_team_index, cause_team_index))
		{
			hostility = _comm_hostility_traitor;
		}
	}

	if (ai_debug.print_communication)
	{
		char hostility_chars[NUMBER_OF_AI_COMMUNICATION_HOSTILITIES] = { 'n', 's', 'f', 'e', 't' };

		sprintf(printbuffer, "%s-%c ", global_communication_type_names[communication_type], hostility_chars[hostility]);
		strupr(printbuffer);
		strcpy(silent_printbuffer, "");
	}

	if (!subject_actor)
	{
		short status_index;

		for (status_index = 0; status_index < NUMBER_OF_COMMUNICATION_ENEMY_STATUS_TYPES; status_index++)
		{
			enemy_status_enabled[status_index] = TRUE;
		}
	}
	else if (!subject_encounter)
	{
		enemy_status_enabled[_comm_enemy_never] = !subject_actor->target.any_target_ever;
		enemy_status_enabled[_comm_enemy_dead] = !subject_actor->target.target_really_alive && subject_actor->target.since_any_target_visible_timer != NONE;
		enemy_status_enabled[_comm_enemy_lost] = subject_actor->target.target_prop_index == NONE || subject_actor->target.since_any_target_visible_timer == NONE || subject_actor->target.since_any_target_visible_timer >= 6 * TICKS_PER_SECOND;
		enemy_status_enabled[_comm_enemy_not_visible] = subject_actor->state.combat_status < _actor_combat_status_definite &&
			(subject_actor->target.since_any_target_visible_timer == NONE || subject_actor->target.since_any_target_visible_timer >= (5 * TICKS_PER_SECOND) / 2) &&
			(subject_actor->target.target_really_alive || subject_actor->state.combat_status > _actor_combat_status_none);
		enemy_status_enabled[_comm_enemy_not_dangerous] = subject_actor->state.combat_status < _actor_combat_status_dangerous;
		enemy_status_enabled[_comm_enemy_visible] = subject_actor->target.target_type >= _actor_target_visible_enemy && subject_actor->target.target_really_alive;
	}
	else
	{
		enemy_status_enabled[_comm_enemy_never] = !subject_actor->target.any_target_ever;
		enemy_status_enabled[_comm_enemy_dead] = subject_encounter->enemy_visible_timer != NONE && !subject_encounter->enemy_alive;
		enemy_status_enabled[_comm_enemy_lost] = (subject_encounter->enemy_visible_timer == NONE || subject_encounter->enemy_visible_timer >= 6 * TICKS_PER_SECOND) && subject_encounter->enemy_alive;
		enemy_status_enabled[_comm_enemy_not_visible] = subject_actor->state.combat_status < _actor_combat_status_definite &&
			(subject_encounter->enemy_visible_timer == NONE || subject_encounter->enemy_visible_timer >= (5 * TICKS_PER_SECOND) / 2) &&
			(subject_encounter->enemy_alive || subject_actor->state.combat_status > _actor_combat_status_none);
		enemy_status_enabled[_comm_enemy_not_dangerous] = subject_actor->state.combat_status < _actor_combat_status_dangerous &&
			(subject_encounter->enemy_visible_timer == NONE || subject_encounter->enemy_visible_timer >= (5 * TICKS_PER_SECOND) / 2);
		enemy_status_enabled[_comm_enemy_visible] = subject_encounter->enemy_visible && subject_encounter->enemy_alive;
	}

	memset(hostility_enabled, 0, sizeof(hostility_enabled));

	if (hostility != NONE)
	{
		hostility_enabled[hostility] = TRUE;

		if (hostility == _comm_hostility_traitor)
		{
			hostility_enabled[_comm_hostility_enemy] = TRUE;
		}
	}

	memset(speech_disabled_reason, NONE, sizeof(speech_disabled_reason));
	memset(speech_disabled, 0, sizeof(speech_disabled));
	memset(speech_enable_after_ticks, 0, sizeof(speech_enable_after_ticks));

	for (team_index = 0; team_index < NUMBER_OF_AI_COMMUNICATION_TEAMS; team_index++)
	{
		short priority;

		time_since_shout[team_index] = (short)MAX(0, current_time - ai_globals->last_shout_time[team_index]);
		time_since_talk[team_index] = (short)MAX(0, current_time - ai_globals->last_talk_time[team_index]);
		time_since_chatter[team_index] = (short)MAX(0, current_time - ai_globals->last_chatter_time[team_index]);
		seconds_since_shout[team_index] = time_since_shout[team_index] * SECONDS_PER_TICK;
		seconds_since_talk[team_index] = time_since_talk[team_index] * SECONDS_PER_TICK;
		seconds_since_chatter[team_index] = time_since_chatter[team_index] * SECONDS_PER_TICK;
		speech_disabled[team_index][_ai_communication_priority_none][_communication_rating_normal] = TRUE;
		speech_disabled[team_index][_ai_communication_priority_none][_communication_rating_low] = TRUE;

		for (priority = _ai_communication_priority_filler; priority < _ai_communication_priority_yell; priority++)
		{
			short rating;

			for (rating = 0; rating < NUMBER_OF_COMMUNICATION_RATINGS; rating++)
			{
				boolean disabled = FALSE;
				short enable_after_ticks = 0;
				short disabled_reason = NONE;

				if (communication_timer_tolerances[priority][rating][_communication_timer_chatter] > 0.f)
				{
					short chatter_ticks = (short)(communication_timer_tolerances[priority][rating][_communication_timer_chatter] * TICKS_PER_SECOND - time_since_chatter[team_index]);

					if (chatter_ticks > 0)
					{
						disabled = TRUE;
						disabled_reason = _speech_disabled_by_chatter;
						enable_after_ticks = MAX(enable_after_ticks, chatter_ticks);
					}
				}

				if (communication_timer_tolerances[priority][rating][_communication_timer_talk] > 0.f)
				{
					short talk_ticks = (short)(communication_timer_tolerances[priority][rating][_communication_timer_talk] * TICKS_PER_SECOND - time_since_talk[team_index]);

					if (talk_ticks > 0)
					{
						disabled = TRUE;
						disabled_reason = _speech_disabled_by_talk;
						enable_after_ticks = MAX(enable_after_ticks, talk_ticks);
					}
				}

				if (communication_timer_tolerances[priority][rating][_communication_timer_shout] > 0.f)
				{
					short shout_ticks = (short)(communication_timer_tolerances[priority][rating][_communication_timer_shout] * TICKS_PER_SECOND - time_since_shout[team_index]);

					if (shout_ticks > 0)
					{
						disabled = TRUE;
						disabled_reason = _speech_disabled_by_shout;
						enable_after_ticks = MAX(enable_after_ticks, shout_ticks);
					}
				}

				if (disabled &&
					communication_timer_tolerances[priority][rating][_communication_timer_overlap] > 0.f &&
					speech_enable_after_ticks[team_index][priority][rating] < communication_timer_tolerances[priority][rating][_communication_timer_overlap] * TICKS_PER_SECOND)
				{
					disabled = FALSE;
				}

				speech_disabled[team_index][priority][rating] = disabled;
				speech_enable_after_ticks[team_index][priority][rating] = enable_after_ticks;
				speech_disabled_reason[team_index][priority][rating] = disabled_reason;
			}
		}
	}

	if (!ai_globals->dialogue_triggers_enabled)
	{
		if (ai_debug.print_communication)
		{
			strcat(printbuffer, "DISABLED");

			if (!suppress_print)
			{
				error(_error_silent, printbuffer);
			}
		}

		return;
	}

	dialogue_index = global_communication_table_indices[communication_type];

	if (game_connection() == _game_connection_local && BIT_VECTOR_TEST_FLAG(ai_debug.communication_suppress_flags, communication_type))
	{
		dialogue_index = NONE;
		suppress_print = TRUE;
	}
	else
	{
		suppress_print = BIT_VECTOR_TEST_FLAG(ai_debug.communication_ignore_flags, communication_type);
	}

	if (ai_debug.communication_focus_enable)
	{
		suppress_print = TRUE;
	}

	if (dialogue_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1219, (dialogue_index >= 0) && (dialogue_index < global_dialogue_event_count));

		for (usage = &global_dialogue_table[dialogue_index]; usage->communication_type == communication_type; usage++, dialogue_index++)
		{
			short unit_speech_priority;
			long protagonist_actor_index;
			long protagonist_unit_index;
			struct actor_datum *protagonist_actor;
			long target_unit_index;
			short recipient_look_type;
			struct ai_information_look_data recipient_look_data;
			short protagonist_look_priority;
			short recipient_look_priority;
			boolean *comm_groups_enabled;
			boolean low_priority;
			boolean spoken_by_player;
			long preselected_reply_actor_index;
			short play_delay;
			short notification_delay;
			short speech_enable_after;
			real player_rating;
			real recent_rating;
			real reply_rating;
			char classificationbuf[256];
			boolean no_unit;
			short communication_priority = usage->communication_priority;

			if (ai_debug.communication_focus_enable && BIT_VECTOR_TEST_FLAG(ai_debug.vocalization_focus_flags, usage->vocalization_type))
			{
				suppress_print = FALSE;
			}

			if (usage->required_hostility != NONE && !hostility_enabled[usage->required_hostility])
			{
				if (ai_debug.print_communication)
				{
					char const *hostility_names[NUMBER_OF_AI_COMMUNICATION_HOSTILITIES] = { "none", "self", "friend", "enemy", "traitor" };

					strcat(silent_printbuffer, csprintf(
						temporary,
						"[%s/%d host-%s] ",
						dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
						dialogue_index,
						hostility_names[usage->required_hostility]));
				}

				any_silent_rejection = TRUE;
				continue;
			}

			if (sound_scripted_dialog_is_playing() && usage->communication_priority < _ai_communication_priority_yell && !TEST_FLAG(usage->flags, _dialogue_usage_override_scripted_bit))
			{
				if (ai_debug.print_communication)
				{
					strcat(silent_printbuffer, "[scripted-override] ");
				}

				any_silent_rejection = TRUE;
				continue;
			}

			if (usage->required_enemy_status != NONE && !enemy_status_enabled[usage->required_enemy_status])
			{
				if (ai_debug.print_communication)
				{
					char const *status_names[NUMBER_OF_COMMUNICATION_ENEMY_STATUS_TYPES] = { "never", "dead", "lost", "notvis", "nodanger", "vis" };

					strcat(silent_printbuffer, csprintf(
						temporary,
						"[%s/%d status-%s] ",
						dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
						dialogue_index,
						status_names[usage->required_enemy_status]));
				}

				any_silent_rejection = TRUE;
				continue;
			}

			if (usage->required_subject_race != NONE && (subject_unit_index == NONE || !(subject_race & usage->required_subject_race)))
			{
				if (ai_debug.print_communication)
				{
					strcat(silent_printbuffer, csprintf(
						temporary,
						"[%s/%d nosubrace] ",
						dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
						dialogue_index));
				}

				any_silent_rejection = TRUE;
				continue;
			}

			if (usage->required_cause_race != NONE && (cause_unit_index == NONE || !(cause_race & usage->required_cause_race)))
			{
				if (ai_debug.print_communication)
				{
					strcat(silent_printbuffer, csprintf(
						temporary,
						"[%s/%d nocausrace] ",
						dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
						dialogue_index));
				}

				any_silent_rejection = TRUE;
				continue;
			}

			if (usage->required_damage != NONE && usage->required_damage != damage_type)
			{
				if (ai_debug.print_communication)
				{
					strcat(silent_printbuffer, csprintf(
						temporary,
						"[%s/%d nodmg] ",
						dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
						dialogue_index));
				}

				any_silent_rejection = TRUE;
				continue;
			}

			unit_speech_priority = communication_speech_priorities[communication_priority];
			protagonist_actor_index = NONE;
			protagonist_unit_index = NONE;
			protagonist_actor = NULL;
			target_unit_index = NONE;
			recipient_look_type = _ai_information_look_none;
			protagonist_look_priority = _secondary_look_priority_none;
			recipient_look_priority = _secondary_look_priority_none;
			comm_groups_enabled = NULL;
			spoken_by_player = FALSE;
			preselected_reply_actor_index = NONE;
			speech_enable_after = 0;
			recent_rating = 1.f;
			reply_rating = 1.f;
			strcpy(classificationbuf, "<err>");

			switch (usage->protagonist_type)
			{
			case _comm_protagonist_subject:
				comm_groups_enabled = subject_comm_groups_enabled;
				protagonist_unit_index = subject_unit_index;
				protagonist_actor_index = subject_actor_index;
				protagonist_actor = subject_actor;
				target_unit_index = cause_unit_index;
				break;
			case _comm_protagonist_cause:
				comm_groups_enabled = cause_comm_groups_enabled;
				protagonist_unit_index = cause_unit_index;
				protagonist_actor_index = cause_actor_index;
				protagonist_actor = cause_actor;
				target_unit_index = subject_unit_index;
				break;
			case _comm_protagonist_friend:
				target_unit_index = cause_unit_index;

				if (must_find_friend)
				{
					short flags = 0;

					SET_FLAG(flags, _find_actor_allow_lookup_bit, TEST_FLAG(usage->flags, _dialogue_usage_lookup_bit));
					SET_FLAG(flags, _find_actor_near_to_players_bit, TRUE);
					SET_FLAG(flags, _find_actor_same_vehicle_bit, TEST_FLAG(usage->flags, _dialogue_usage_same_vehicle_bit));
					SET_FLAG(flags, _find_actor_allow_subject_bit, TEST_FLAG(usage->flags, _dialogue_usage_allow_subject_bit));
					SET_FLAG(flags, _find_actor_allow_cause_bit, TRUE);

					if (subject_encounter_index != NONE)
					{
						friend_actor_index = ai_communication_find_specific_actor_to_talk(
							DATUM_INDEX_TO_ABSOLUTE_INDEX(subject_encounter_index),
							subject_unit_index,
							cause_unit_index,
							FIND_ACTOR_FRIEND_DISTANCE,
							communication_type,
							communication_priority,
							unit_speech_priority,
							usage->vocalization_type,
							usage->animation_type,
							flags);
					}
					else
					{
						friend_actor_index = ai_communication_find_global_actor_to_talk(
							subject_team_index,
							_find_actor_mode_friend,
							subject_unit_index,
							cause_unit_index,
							FIND_ACTOR_FRIEND_DISTANCE,
							communication_type,
							communication_priority,
							unit_speech_priority,
							usage->vocalization_type,
							usage->animation_type,
							flags);
					}

					must_find_friend = FALSE;
				}

				protagonist_actor_index = friend_actor_index;

				if (protagonist_actor_index != NONE)
				{
					protagonist_actor = actor_get(protagonist_actor_index);
					protagonist_unit_index = protagonist_actor->meta.unit_index;
				}
				break;
			case _comm_protagonist_enemy:
				target_unit_index = cause_unit_index;

				if (must_find_enemy)
				{
					short flags = 0;

					SET_FLAG(flags, _find_actor_allow_lookup_bit, TEST_FLAG(usage->flags, _dialogue_usage_lookup_bit));
					SET_FLAG(flags, _find_actor_near_to_players_bit, TRUE);
					SET_FLAG(flags, _find_actor_same_vehicle_bit, TEST_FLAG(usage->flags, _dialogue_usage_same_vehicle_bit));
					SET_FLAG(flags, _find_actor_allow_subject_bit, TEST_FLAG(usage->flags, _dialogue_usage_allow_subject_bit));
					enemy_actor_index = ai_communication_find_global_actor_to_talk(
						subject_team_index,
						_find_actor_mode_enemy,
						subject_unit_index,
						cause_unit_index,
						FIND_ACTOR_ENEMY_DISTANCE,
						communication_type,
						communication_priority,
						unit_speech_priority,
						usage->vocalization_type,
						usage->animation_type,
						flags);
					must_find_enemy = FALSE;
				}

				protagonist_actor_index = enemy_actor_index;

				if (protagonist_actor_index != NONE)
				{
					protagonist_actor = actor_get(protagonist_actor_index);
					protagonist_unit_index = protagonist_actor->meta.unit_index;
				}
				break;
			default:
				match_halt("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1428);
			}

			no_unit = FALSE;

			if (protagonist_unit_index == NONE)
			{
				no_unit = TRUE;
			}
			else
			{
				struct unit_datum *protagonist_unit = unit_get(protagonist_unit_index);

				if (TEST_FLAG(protagonist_unit->object.damage_flags, _object_dead_bit))
				{
					no_unit = TRUE;
				}
				else if (protagonist_unit->object.type == _object_type_vehicle)
				{
					no_unit = TRUE;
				}
				else if (protagonist_unit->unit.player_index != NONE && protagonist_unit->unit.actor_index == NONE)
				{
					if (TEST_FLAG(usage->flags, _dialogue_usage_player_bit))
					{
						spoken_by_player = TRUE;
						player_involved = TRUE;
					}
					else
					{
						no_unit = TRUE;
					}
				}
			}

			if (protagonist_actor && (protagonist_actor->state.mode == _actor_mode_braindead || (protagonist_actor->state.action == _actor_action_obey && !protagonist_actor->state.action_data.obey.allow_communication)))
			{
				no_unit = TRUE;
			}

			if (no_unit)
			{
				if (ai_debug.print_communication)
				{
					char protagonist_code = (usage->protagonist_type == _comm_protagonist_subject) ? 's' :
						(usage->protagonist_type == _comm_protagonist_cause) ? 'c' :
						(usage->protagonist_type == _comm_protagonist_friend) ? 'f' :
						(usage->protagonist_type == _comm_protagonist_enemy) ? 'e' : '?';

					strcat(printbuffer, csprintf(
						temporary,
						"[%s/%d nounit-%c] ",
						dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
						dialogue_index,
						protagonist_code));
				}

				any_printed = TRUE;
				continue;
			}

			if (spoken_by_player)
			{
				preselected_reply_actor_index = ai_communication_find_actor_to_reply_to_player(
					protagonist_unit_index,
					target_unit_index,
					usage->vocalization_type,
					damage_type,
					&reply_rating);

				if (protagonist_unit_index == subject_unit_index)
				{
					must_find_friend = FALSE;
					friend_actor_index = preselected_reply_actor_index;
				}

				if (preselected_reply_actor_index == NONE)
				{
					if (ai_debug.print_communication)
					{
						strcat(printbuffer, csprintf(
							temporary,
							"[%s/%d noplyreply] ",
							dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
							dialogue_index));
					}

					any_printed = TRUE;
					continue;
				}
			}

			if (usage->required_group != NONE && comm_groups_enabled && !comm_groups_enabled[usage->required_group])
			{
				if (ai_debug.print_communication)
				{
					char group_code = (usage->required_group == _comm_group_extended) ? 'e' :
						(usage->required_group == _comm_group_tactical) ? 't' : '?';

					strcat(printbuffer, csprintf(
						temporary,
						"[%s/%d nogrp-%c] ",
						dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
						dialogue_index,
						group_code));
				}

				any_printed = TRUE;
				continue;
			}

			if (spoken_by_player)
			{
				communication_priority = communication_player_speaking_priorities[communication_priority];
				low_priority = FALSE;
				player_rating = 2.f;
				strcpy(classificationbuf, "player");
			}
			else
			{
				short communication_team;

				player_rating = ai_communication_get_player_rating(protagonist_unit_index, TRUE, NULL, NULL);

				if (player_rating == 0.f)
				{
					if (ai_debug.print_communication)
					{
						strcat(printbuffer, csprintf(
							temporary,
							"[%s/%d 0-playrat] ",
							dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
							dialogue_index));
					}

					any_printed = TRUE;
					continue;
				}

				low_priority = player_rating < communication_player_rating_low_priority;
				communication_team = (protagonist_actor_index == NONE) ? NONE : actor_communication_team(protagonist_actor_index);

				if (communication_team == NONE)
				{
					strcpy(classificationbuf, "unteamed");
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1579, (communication_team >= 0) && (communication_team < NUMBER_OF_AI_COMMUNICATION_TEAMS));
					match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1582, (communication_priority > _ai_communication_priority_none) && (communication_priority < NUMBER_OF_AI_COMMUNICATION_PRIORITIES));
					sprintf(
						classificationbuf,
						"%s-%c%c%c%s",
						global_communication_team_names[communication_team][1],
						global_communication_priority_names[communication_priority][0],
						global_communication_priority_names[communication_priority][1],
						global_communication_priority_names[communication_priority][2],
						low_priority ? "-lo" : "-hi");

					if (speech_disabled[communication_team][communication_priority][low_priority])
					{
						match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1593, communication_priority < _ai_communication_priority_yell);

						if (ai_debug.print_communication)
						{
							char disabled_reasonbuf[512];

							if (speech_disabled_reason[communication_team][communication_priority][low_priority] == _speech_disabled_by_chatter)
							{
								sprintf(
									disabled_reasonbuf,
									"chat:%.1f<%.1f",
									seconds_since_chatter[communication_team],
									communication_timer_tolerances[communication_priority][low_priority][_communication_timer_chatter]);
							}
							else if (speech_disabled_reason[communication_team][communication_priority][low_priority] == _speech_disabled_by_talk)
							{
								sprintf(
									disabled_reasonbuf,
									"talk:%.1f<%.1f",
									seconds_since_talk[communication_team],
									communication_timer_tolerances[communication_priority][low_priority][_communication_timer_talk]);
							}
							else if (speech_disabled_reason[communication_team][communication_priority][low_priority] == _speech_disabled_by_shout)
							{
								sprintf(
									disabled_reasonbuf,
									"shout:%.1f<%.1f",
									seconds_since_shout[communication_team],
									communication_timer_tolerances[communication_priority][low_priority][_communication_timer_shout]);
							}
							else
							{
								sprintf(disabled_reasonbuf, "<err>");
							}

							strcat(printbuffer, csprintf(
								temporary,
								"[%s/%d %s %s] ",
								dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
								dialogue_index,
								classificationbuf,
								disabled_reasonbuf));
						}

						any_printed = TRUE;
						continue;
					}

					speech_enable_after = speech_enable_after_ticks[communication_team][communication_priority][low_priority];

					if (communication_priority < _ai_communication_priority_exclaim)
					{
						struct dialogue_event_status *status = &global_dialogue_events[communication_team + NUMBER_OF_AI_COMMUNICATION_TEAMS * dialogue_index];

						if (status->last_time_spoken != NONE)
						{
							long ticks_since_spoken = current_time - status->last_time_spoken;

							recent_rating = (real)ticks_since_spoken / communication_repeat_selection_time;
							recent_rating = PIN(recent_rating, 0.f, 1.f);
						}

						if ((game_connection() != _game_connection_local || !ai_debug.communication_timeout_disabled) && status->disable_until_time != NONE)
						{
							long disable_ticks = status->disable_until_time - current_time;

							if (low_priority)
							{
								disable_ticks += communication_timeout_low_priority_modifier;
							}

							if (disable_ticks > 0)
							{
								if (ai_debug.print_communication)
								{
									strcat(printbuffer, csprintf(
										temporary,
										"[%s/%d %s-d-dis/%d] ",
										dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
										dialogue_index,
										global_communication_team_names[communication_team][1],
										disable_ticks));
								}

								any_printed = TRUE;
								continue;
							}
						}
					}
				}
			}

			play_delay = (short)(communication_play_delays[usage->protagonist_type] * TICKS_PER_SECOND);

			if (subject_race == _race_player && !spoken_by_player)
			{
				play_delay += communication_player_additional_delay;
			}

			notification_delay = (short)(TEST_FLAG(usage->flags, _dialogue_usage_immediate_notify_bit) ? 0.f : communication_notification_delays[communication_priority] * TICKS_PER_SECOND);
			play_delay += speech_enable_after;
			notification_delay += speech_enable_after;

			switch (usage->recipient_look_direction)
			{
			case _comm_look_direction_subject:
				if (subject_unit_index != NONE)
				{
					recipient_look_type = _ai_information_look_unit;
					recipient_look_data.unit.unit_index = subject_unit_index;
				}
				break;
			case _comm_look_direction_protagonist:
				if (protagonist_unit_index != NONE)
				{
					recipient_look_type = _ai_information_look_unit;
					recipient_look_data.unit.unit_index = protagonist_unit_index;
				}
				break;
			case _comm_look_direction_target:
				if (target_unit_index != NONE)
				{
					recipient_look_type = _ai_information_look_unit;
					recipient_look_data.unit.unit_index = target_unit_index;
				}
				break;
			case _comm_look_direction_danger:
				if (subject_actor_index != NONE)
				{
					struct actor_datum *subject_actor = actor_get(subject_actor_index);

					if (subject_actor->danger_zone.danger_type > _actor_danger_zone_none)
					{
						match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1712, subject_actor->danger_zone.object_index != NONE);
						recipient_look_type = _ai_information_look_object;
						recipient_look_data.object.object_index = subject_actor->danger_zone.object_index;
					}
				}
				break;
			}

			if (recipient_look_type > _ai_information_look_none)
			{
				recipient_look_priority = usage->recipient_look_priority;

				if (recipient_look_priority == NONE || recipient_look_priority == _secondary_look_priority_default)
				{
					recipient_look_priority = communication_recipient_default_look_priorities[communication_priority];
				}
			}

			protagonist_look_priority = usage->protagonist_look_priority;

			if (protagonist_look_priority == NONE || protagonist_look_priority == _secondary_look_priority_default)
			{
				protagonist_look_priority = communication_protagonist_default_look_priorities[communication_priority];
			}

			{
				short play_type;
				real weight;
				short vocalization_type = usage->vocalization_type;
				short animation_type = usage->animation_type;
				long sound_definition_index = NONE;
				real speech_rating = 1.f;
				real animation_rating = 1.f;

				if (!spoken_by_player)
				{
					char debugbuf[512];
					char *debug_string = NULL;

					debug_string = debugbuf;
					play_type = ai_communication_consider_speech(
						protagonist_unit_index,
						communication_priority,
						unit_speech_priority,
						play_delay,
						TEST_FLAG(usage->flags, _dialogue_usage_lookup_bit),
						FALSE,
						&vocalization_type,
						&speech_rating,
						&sound_definition_index,
						debug_string);

					if (play_type == _unit_play_speech_none)
					{
						if (ai_debug.print_communication)
						{
							strcat(printbuffer, csprintf(
								temporary,
								"[%s/%d u-%s-%s] ",
								dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
								dialogue_index,
								(speech_rating > 0.f) ? "dis" : "n/a",
								debug_string));
						}

						any_printed = TRUE;
						continue;
					}

					if (animation_type != NONE)
					{
						boolean animate = unit_test_animation_impulse(protagonist_unit_index, animation_type);

						if (animate && protagonist_actor_index != NONE)
						{
							match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1778, protagonist_actor);

							if (actor_action_class(protagonist_actor_index) == _action_class_transitory)
							{
								animate = FALSE;
							}
							else if (protagonist_actor->state.mode == _actor_mode_asleep)
							{
								animate = FALSE;
							}
						}

						if (animate)
						{
							animation_rating = 2.f;
						}
						else
						{
							animation_type = NONE;
						}
					}
				}

				weight = (usage->weight * speech_rating * player_rating * animation_rating) * recent_rating * reply_rating;

				if (weight > 0.f)
				{
					if (num_possibilities >= MAXIMUM_COMMUNICATION_POSSIBILITIES)
					{
						error(
							_error_silent,
							"ai_communication_event: type %d (%s) overflowed MAXIMUM_COMMUNICATION_POSSIBILITIES (%d)",
							communication_type,
							global_communication_type_names[communication_type],
							MAXIMUM_COMMUNICATION_POSSIBILITIES);
						break;
					}

					possibilities[num_possibilities].dialogue_index = dialogue_index;
					possibilities[num_possibilities].weight = weight;
					possibilities[num_possibilities].spoken_by_player = spoken_by_player;
					possibilities[num_possibilities].protagonist_unit_index = protagonist_unit_index;
					possibilities[num_possibilities].protagonist_actor_index = protagonist_actor_index;
					possibilities[num_possibilities].animation_type = usage->animation_type;
					possibilities[num_possibilities].target_unit_index = target_unit_index;
					possibilities[num_possibilities].preselected_reply_actor_index = preselected_reply_actor_index;
					possibilities[num_possibilities].unit_speech_priority = unit_speech_priority;
					possibilities[num_possibilities].play_delay = play_delay;
					possibilities[num_possibilities].notification_delay = notification_delay;
					possibilities[num_possibilities].speech_play_type = play_type;
					possibilities[num_possibilities].vocalization_type = vocalization_type;
					possibilities[num_possibilities].sound_definition_index = sound_definition_index;
					possibilities[num_possibilities].protagonist_look_priority = protagonist_look_priority;
					possibilities[num_possibilities].recipient_look_priority = recipient_look_priority;
					possibilities[num_possibilities].recipient_look_type = recipient_look_type;
					possibilities[num_possibilities].recipient_look_data = recipient_look_data;
					possibilities[num_possibilities].force = TEST_FLAG(usage->flags, _dialogue_usage_force_bit);

					if (possibilities[num_possibilities].force)
					{
						forced_possibilities = TRUE;
					}

					num_possibilities++;

					if (ai_debug.print_communication)
					{
						strcat(printbuffer, csprintf(
							temporary,
							"[%s/%d %s del%d w:%.1f%s s%.1f p%.1f%s a%.1f rc%.1f rp%.1f t%.1f] ",
							dialogue_get_vocalization_name(usage->vocalization_type, TRUE),
							dialogue_index,
							classificationbuf,
							speech_enable_after,
							usage->weight,
							possibilities[num_possibilities - 1].force ? "F" : "",
							speech_rating,
							player_rating,
							spoken_by_player ? "PLAYER" : "",
							animation_rating,
							recent_rating,
							reply_rating,
							possibilities[num_possibilities - 1].weight));
					}

					any_printed = TRUE;
					total_possibility_weight += weight;
				}
			}
		}
	}

	if (ai_debug.print_communication_player && !player_involved)
	{
		suppress_print = TRUE;
	}

	if (num_possibilities > 0)
	{
		struct ai_information_packet ai_packet;
		struct communication_possibility *selected_possibility = possibilities;

		if (forced_possibilities)
		{
			short possibility_index;
			real original_weight = total_possibility_weight;
			short original_count = num_possibilities;
			short forced_count = 0;

			total_possibility_weight = 0.f;

			for (possibility_index = 0; possibility_index < num_possibilities; possibility_index++)
			{
				if (possibilities[possibility_index].force)
				{
					forced_count++;
				}
				else
				{
					possibilities[possibility_index].weight = 0.f;
				}

				total_possibility_weight += possibilities[possibility_index].weight;
			}

			match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1902, total_possibility_weight > 0.0f);

			if (ai_debug.print_communication)
			{
				strcat(printbuffer, csprintf(
					temporary,
					"[%d/%.1f force %d/%.1f] ",
					original_count,
					original_weight,
					forced_count,
					total_possibility_weight));
			}
		}

		if (num_possibilities > 1)
		{
			short possibility_index;
			real accumulated_weight = 0.f;
			real random_weight = real_random() * total_possibility_weight;

			for (possibility_index = 0; possibility_index < num_possibilities - 1; possibility_index++)
			{
				accumulated_weight += possibilities[possibility_index].weight;

				if (accumulated_weight >= random_weight)
				{
					break;
				}
			}

			selected_possibility = &possibilities[possibility_index];

			if (ai_debug.print_communication)
			{
				strcat(printbuffer, csprintf(
					temporary,
					"[rnd%.1f tot%.1f cum%.1f@%d] ",
					random_weight,
					total_possibility_weight,
					accumulated_weight,
					possibility_index));
			}
		}

		ai_packet.communication_type = communication_type;
		ai_packet.damage_category = damage_type;
		ai_packet.target_unit_index = selected_possibility->target_unit_index;
		ai_packet.dialogue_type_index = selected_possibility->dialogue_index;
		ai_packet.updated_dialogue_timers = TRUE;
		ai_packet.look_priority = selected_possibility->recipient_look_priority;
		ai_packet.look_type = selected_possibility->recipient_look_type;
		ai_packet.look_data = selected_possibility->recipient_look_data;
		ai_packet.information_type = (information_type == NONE) ? _ai_information_none : information_type;

		if (!information_data)
		{
			memset(&ai_packet.information_data, 0, sizeof(ai_packet.information_data));
		}
		else
		{
			ai_packet.information_data = *information_data;
		}

		if (selected_possibility->spoken_by_player)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 1959, selected_possibility->preselected_reply_actor_index != NONE);
			ai_communication_started(
				selected_possibility->protagonist_unit_index,
				selected_possibility->unit_speech_priority,
				selected_possibility->vocalization_type,
				&ai_packet);
			ai_communication_notify(
				selected_possibility->protagonist_unit_index,
				selected_possibility->unit_speech_priority,
				selected_possibility->vocalization_type,
				&ai_packet);
			ai_communication_finished(
				selected_possibility->protagonist_unit_index,
				selected_possibility->unit_speech_priority,
				selected_possibility->vocalization_type,
				TRUE,
				selected_possibility->preselected_reply_actor_index,
				&ai_packet);
		}
		else
		{
			struct unit_speech_item speech;

			speech.priority = selected_possibility->unit_speech_priority;
			speech.vocalization_type = selected_possibility->vocalization_type;
			speech.sound_definition_index = selected_possibility->sound_definition_index;
			speech.delay_time = selected_possibility->play_delay;
			speech.ai_notification_delay = selected_possibility->notification_delay;
			speech.pause_time = 24;
			speech.ai = ai_packet;
			unit_speak(selected_possibility->protagonist_unit_index, selected_possibility->speech_play_type, &speech);
			speech_started = TRUE;

			if (selected_possibility->animation_type != NONE)
			{
				real_vector2d alignment_vector;
				struct unit_datum *protagonist_unit = unit_get(selected_possibility->protagonist_unit_index);

				alignment_vector = *(real_vector2d *)&protagonist_unit->object.forward;

				if (selected_possibility->target_unit_index != NONE)
				{
					real_point3d protagonist_head_position;
					real_point3d target_head_position;

					unit_get_head_position(selected_possibility->protagonist_unit_index, &protagonist_head_position);
					unit_get_head_position(selected_possibility->target_unit_index, &target_head_position);
					vector_from_points2d((real_point2d *)&protagonist_head_position, (real_point2d *)&target_head_position, &alignment_vector);

					if (normalize2d(&alignment_vector) == 0.f)
					{
						alignment_vector = *(real_vector2d *)&protagonist_unit->object.forward;
					}
				}

				unit_start_animation_impulse(selected_possibility->protagonist_unit_index, selected_possibility->animation_type, &alignment_vector);
			}

			if (selected_possibility->protagonist_actor_index != NONE)
			{
				ai_communication_look_secondary_at_unit(
					selected_possibility->protagonist_actor_index,
					_secondary_look_communicated_direction,
					selected_possibility->protagonist_look_priority,
					selected_possibility->target_unit_index,
					NONE);
			}

			ai_communication_update_speech_timers(
				selected_possibility->protagonist_unit_index,
				selected_possibility->unit_speech_priority,
				selected_possibility->vocalization_type,
				selected_possibility->dialogue_index,
				NONE);
		}

		if (ai_debug.print_communication)
		{
			strcat(printbuffer, strupr(csprintf(
				temporary,
				">>%s<<",
				dialogue_get_vocalization_name(selected_possibility->vocalization_type, TRUE))));

			if (!suppress_print)
			{
				error(_error_silent, printbuffer);
			}
		}

		return;
	}

	if (ai_debug.print_communication)
	{
		if (any_silent_rejection && !any_printed)
		{
			strcat(printbuffer, silent_printbuffer);
		}

		if (ai_debug.communication_focus_enable || !any_silent_rejection || any_printed)
		{
			strcat(printbuffer, "NONE");

			if (!suppress_print)
			{
				error(_error_silent, printbuffer);
			}
		}
	}

	return;
}

void ai_communication_started(
	long unit_index,
	short priority,
	short vocalization_type,
	struct ai_information_packet *ai_information)
{
	switch (priority)
	{
	case _unit_speech_none:
	case _unit_speech_idle:
	case _unit_speech_pain:
	case _unit_speech_involuntary:
	case _unit_speech_death:
		break;
	default:
		if (ai_debug.print_vocalizations)
		{
			char buffer[1024];
			struct unit_datum *unit = unit_get(unit_index);

			if (unit->unit.actor_index != NONE)
			{
				char encounterbuf[256];
				struct actor_datum *actor = actor_get(unit->unit.actor_index);

				if (actor->meta.encounter_index == NONE)
				{
					strcpy(encounterbuf, "<no encounter>");
				}
				else
				{
					struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->ai_encounters,
						DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
						struct encounter_definition);

					sprintf(
						encounterbuf,
						"%s/%s",
						encounter_definition->name,
						TAG_BLOCK_GET_ELEMENT(&encounter_definition->squads, actor->meta.squad_index, struct squad_definition)->name);
				}

				sprintf(buffer, "%s/%s: ", encounterbuf, actor_type_get_name(actor->meta.type));
			}
			else if (unit->object.name_index != NONE)
			{
				sprintf(
					buffer,
					"%s: ",
					TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, unit->object.name_index, struct scenario_object_name)->name);
			}
			else if (unit->unit.player_index != NONE)
			{
				sprintf(buffer, "player %d: ", DATUM_INDEX_TO_ABSOLUTE_INDEX(unit->unit.player_index));
			}
			else
			{
				sprintf(buffer, "unit %04X: ", DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index));
			}

			sprintf(
				temporary,
				"%s %s",
				unit_get_speech_priority_name(priority),
				(vocalization_type == NONE) ? "non-voc" : dialogue_get_vocalization_name(vocalization_type, FALSE));
			strcat(buffer, temporary);

			if (ai_information && ai_information->dialogue_type_index != NONE)
			{
				sprintf(
					temporary,
					" [%d/%s]",
					ai_information->dialogue_type_index,
					ai_communication_get_type_name(global_dialogue_table[ai_information->dialogue_type_index].communication_type));
				strcat(buffer, temporary);
			}

			console_printf(FALSE, buffer);
		}

		if (ai_debug.print_speech)
		{
			struct unit_datum *unit = unit_get(unit_index);

			if (unit->unit.speech.current.priority > _unit_speech_none)
			{
				char speechbuf[512];
				char unitbuf[512];

				error(
					_error_silent,
					"%s: %s",
					ai_debug_describe_actor(unit->unit.actor_index, unit_index, FALSE, unitbuf, sizeof(unitbuf)),
					unit_describe_speech(unit_index, TRUE, sizeof(speechbuf), speechbuf));
			}
		}

		if (!ai_information->updated_dialogue_timers)
		{
			ai_communication_update_speech_timers(
				unit_index,
				priority,
				vocalization_type,
				ai_information->dialogue_type_index,
				NONE);
		}
		break;
	}

	return;
}

void ai_communication_notify(
	long unit_index,
	short priority,
	short vocalization_type,
	struct ai_information_packet *ai_information)
{
	switch (ai_information->information_type)
	{
	case _ai_information_allegiance:
		ai_handle_allegiance_broken_notification(
			ai_information->information_data.allegiance.team1_index,
			ai_information->information_data.allegiance.team2_index,
			ai_information->information_data.allegiance.broken);
		break;
	}

	if (ai_information->information_type != _ai_information_none || ai_information->look_priority > _secondary_look_priority_none)
	{
		struct actor_iterator iterator;
		struct actor_datum *actor;
		real_point3d speech_point;
		struct unit_datum *unit = unit_get(unit_index);
		struct location *location = &unit->object.location;
		short team_index = unit->object.owner_team_index;
		short sound_volume = _ai_sound_volume_medium;

		unit_get_head_position(unit_index, &speech_point);

		if (ai_information->dialogue_type_index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 2196, (ai_information->dialogue_type_index >= 0) && (ai_information->dialogue_type_index < global_dialogue_event_count));

			if (global_dialogue_table[ai_information->dialogue_type_index].communication_priority >= _ai_communication_priority_communicate)
			{
				sound_volume = _ai_sound_volume_shout;
			}
		}

		if (unit->object.parent_object_index != NONE)
		{
			long parent_object_index = object_get_ultimate_parent(unit_index);

			location = &object_get(parent_object_index)->object.location;
		}

		actor_iterator_new(&iterator, TRUE);

		while (actor = actor_iterator_next(&iterator))
		{
			if (actor->meta.unit_index != unit_index &&
				!game_team_is_enemy(actor->meta.team_index, team_index) &&
				!(distance_squared3d(&actor->input.position.head_position, &speech_point) > 30.f * 30.f))
			{
				long prop_index = prop_get_base_by_unit_index(iterator.index, unit_index, TRUE, TRUE);

				if (prop_index != NONE)
				{
					struct actor_position_data sense_position;
					struct prop_datum *prop = prop_get(prop_index);

					actor_perception_find_sense_position(iterator.index, &speech_point, prop_index, &sense_position);

					if (actor_audibility_at_point(iterator.index, &sense_position, &speech_point, location, sound_volume, 1.f, prop->line_of_sight) >= _actor_perception_full)
					{
						actor_handle_communication(iterator.index, prop_index, ai_information);
						ai_communication_handle_received_looking(iterator.index, prop_index, ai_information);
					}
				}
			}
		}
	}

	return;
}

static long ai_communication_find_actor_to_reply_to_player(
	long unit_index,
	long target_unit_index,
	short vocalization_type,
	short damage_category,
	real *reply_rating_reference)
{
	long reply_actor_index = NONE;
	real reply_rating = 1.f;

	if (ai_globals->dialogue_triggers_enabled && vocalization_type != NONE)
	{
		char printbuffer[1024];
		struct reply_usage const *reply;
		short reply_index;
		boolean suppress_print = ai_debug.communication_focus_enable;

		if (ai_debug.print_communication)
		{
			sprintf(printbuffer, "PLAYER-REPLY %s ", dialogue_get_vocalization_name(vocalization_type, TRUE));
		}

		if (ai_debug.communication_focus_enable && BIT_VECTOR_TEST_FLAG(ai_debug.vocalization_focus_flags, vocalization_type))
		{
			suppress_print = FALSE;
		}

		reply = global_reply_table;

		for (reply_index = 0; reply->original_vocalization_type != NONE; reply_index++)
		{
			if (reply->original_vocalization_type == vocalization_type &&
				(reply->original_damage_category == NONE || reply->original_damage_category == damage_category))
			{
				short unit_speech_priority = communication_speech_priorities[reply->communication_priority];

				if (ai_debug.communication_focus_enable && BIT_VECTOR_TEST_FLAG(ai_debug.vocalization_focus_flags, reply->vocalization_type))
				{
					suppress_print = FALSE;
				}

				if (sound_scripted_dialog_is_playing() && !TEST_FLAG(reply->flags, _reply_usage_override_scripted_bit))
				{
					if (ai_debug.print_communication)
					{
						strcat(printbuffer, "[scripted-override] ");
					}
				}
				else if (reply->player_chance > 0.f)
				{
					real random = real_random();

					if ((game_connection() == _game_connection_local && ai_debug.communication_random_disabled) || random < reply->player_chance)
					{
						switch (reply->protagonist_type)
						{
						case _comm_protagonist_friend:
						{
							struct unit_datum *unit = unit_get(unit_index);

							reply_actor_index = ai_communication_find_global_actor_to_talk(
								unit->object.owner_team_index,
								_find_actor_mode_friend,
								unit_index,
								NONE,
								FIND_ACTOR_REPLY_DISTANCE,
								NONE,
								reply->communication_priority,
								unit_speech_priority,
								reply->vocalization_type,
								reply->animation_type,
								0);
							break;
						}
						case _comm_protagonist_target:
						{
							struct unit_datum *target_unit = unit_try_and_get(target_unit_index);

							if (target_unit)
							{
								reply_actor_index = target_unit->unit.actor_index;
							}
							break;
						}
						case _comm_protagonist_enemy:
						{
							struct unit_datum *unit = unit_get(unit_index);

							reply_actor_index = ai_communication_find_global_actor_to_talk(
								unit->object.owner_team_index,
								_find_actor_mode_enemy,
								unit_index,
								NONE,
								FIND_ACTOR_REPLY_DISTANCE,
								NONE,
								reply->communication_priority,
								unit_speech_priority,
								reply->vocalization_type,
								reply->animation_type,
								0);
							break;
						}
						}

						if (reply_actor_index == NONE)
						{
							if (ai_debug.print_communication)
							{
								strcat(printbuffer, csprintf(
									temporary,
									"[%s nobody] ",
									dialogue_get_vocalization_name(reply->vocalization_type, TRUE)));
							}
						}
						else
						{
							short communication_team = actor_communication_team(reply_actor_index);

							if (communication_team != NONE)
							{
								struct dialogue_event_status *status = &global_reply_events[communication_team + NUMBER_OF_AI_COMMUNICATION_TEAMS * reply_index];
								long current_time = game_time_get();

								if (status->last_time_spoken != NONE)
								{
									reply_rating = (real)(current_time - status->last_time_spoken) / communication_repeat_selection_time;
									reply_rating = PIN(reply_rating, 0.f, 1.f);
								}

								if ((game_connection() != _game_connection_local || !ai_debug.communication_timeout_disabled) && status->disable_until_time != NONE)
								{
									long disable_ticks = status->disable_until_time - current_time;

									if (disable_ticks > 0)
									{
										if (ai_debug.print_communication)
										{
											strcat(
												printbuffer,
												csprintf(
													temporary,
													"[%s %s-d-dis/%d] ",
													dialogue_get_vocalization_name(reply->vocalization_type, TRUE),
													global_communication_team_names[communication_team][1],
													disable_ticks));
										}

										reply_actor_index = NONE;
									}
								}
							}

							if (reply_actor_index != NONE && ai_debug.print_communication)
							{
								strcat(printbuffer, csprintf(
									temporary,
									"[%s found-actor] ",
									dialogue_get_vocalization_name(reply->vocalization_type, TRUE)));
							}
						}
					}
					else if (ai_debug.print_communication)
					{
						strcat(printbuffer, csprintf(
							temporary,
							"[%s rand%.2f>%.2f] ",
							dialogue_get_vocalization_name(reply->vocalization_type, TRUE),
							random,
							reply->player_chance));
					}
				}
				else if (ai_debug.print_communication)
				{
					strcat(printbuffer, csprintf(
						temporary,
						"[%s 0-player-chance] ",
						dialogue_get_vocalization_name(reply->vocalization_type, TRUE)));
				}

				if (reply_actor_index != NONE)
				{
					break;
				}
			}

			reply++;
		}

		if (ai_debug.print_communication && !suppress_print)
		{
			error(_error_silent, printbuffer);
		}
	}

	if (reply_rating_reference)
	{
		*reply_rating_reference = reply_rating;
	}

	return reply_actor_index;
}

void ai_communication_finished(
	long unit_index,
	short priority,
	short vocalization_type,
	boolean reply_to_player,
	long preselected_reply_actor_index,
	struct ai_information_packet *ai_information)
{
	if (ai_globals->dialogue_triggers_enabled && vocalization_type != NONE)
	{
		char printbuffer[1024];
		struct reply_usage const *reply;
		short reply_index;
		boolean suppress_print = FALSE;
		boolean any_reply = FALSE;

		if (ai_debug.print_communication)
		{
			sprintf(printbuffer, "REPLY %s: ", dialogue_get_vocalization_name(vocalization_type, FALSE));
		}

		if (ai_debug.communication_focus_enable)
		{
			suppress_print = !BIT_VECTOR_TEST_FLAG(ai_debug.vocalization_focus_flags, vocalization_type);
		}

		reply = global_reply_table;
		reply_index = 0;

		while (reply->original_vocalization_type != NONE)
		{
			if (reply->original_vocalization_type == vocalization_type)
			{
				struct unit_datum *unit = unit_get(unit_index);
				struct actor_datum *actor = (unit->unit.actor_index == NONE) ? NULL : actor_get(unit->unit.actor_index);
				long reply_unit_index = NONE;
				short unit_speech_priority = communication_speech_priorities[reply->communication_priority];

				any_reply = TRUE;

				if (ai_debug.print_communication)
				{
					strcat(printbuffer, csprintf(
						temporary,
						"%s:",
						dialogue_get_vocalization_name(reply->vocalization_type, FALSE)));
				}

				if (ai_debug.communication_focus_enable && BIT_VECTOR_TEST_FLAG(ai_debug.vocalization_focus_flags, reply->vocalization_type))
				{
					suppress_print = FALSE;
				}

				if (reply->original_damage_category == NONE || reply->original_damage_category == ai_information->damage_category)
				{
					if (sound_scripted_dialog_is_playing() && !TEST_FLAG(reply->flags, _reply_usage_override_scripted_bit))
					{
						if (ai_debug.print_communication)
						{
							strcat(printbuffer, "override-scripted ");
						}
					}
					else
					{
						if (preselected_reply_actor_index != NONE)
						{
							reply_unit_index = actor_get(preselected_reply_actor_index)->meta.unit_index;
						}
						else
						{
							match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 2533, !reply_to_player);

							switch (reply->protagonist_type)
							{
							case _comm_protagonist_friend:
							{
								long reply_actor_index;

								if (actor && actor->meta.encounter_index != NONE)
								{
									reply_actor_index = ai_communication_find_specific_actor_to_talk(
										DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
										unit_index,
										NONE,
										FIND_ACTOR_REPLY_DISTANCE,
										NONE,
										reply->communication_priority,
										unit_speech_priority,
										reply->vocalization_type,
										reply->animation_type,
										0);
								}
								else
								{
									reply_actor_index = ai_communication_find_global_actor_to_talk(
										unit->object.owner_team_index,
										_find_actor_mode_friend,
										unit_index,
										NONE,
										FIND_ACTOR_REPLY_DISTANCE,
										NONE,
										reply->communication_priority,
										unit_speech_priority,
										reply->vocalization_type,
										reply->animation_type,
										0);
								}

								if (reply_actor_index != NONE)
								{
									reply_unit_index = actor_get(reply_actor_index)->meta.unit_index;
								}
								break;
							}
							case _comm_protagonist_target:
								if (unit_try_and_get(ai_information->target_unit_index))
								{
									reply_unit_index = ai_information->target_unit_index;
								}
								break;
							case _comm_protagonist_enemy:
							{
								long reply_actor_index = ai_communication_find_global_actor_to_talk(
									unit->object.owner_team_index,
									_find_actor_mode_enemy,
									unit_index,
									NONE,
									FIND_ACTOR_REPLY_DISTANCE,
									NONE,
									reply->communication_priority,
									unit_speech_priority,
									reply->vocalization_type,
									reply->animation_type,
									0);

								if (reply_actor_index != NONE)
								{
									reply_unit_index = actor_get(reply_actor_index)->meta.unit_index;
								}
								break;
							}
							}
						}

						if (reply_unit_index != NONE)
						{
							struct unit_datum *reply_unit = unit_get(reply_unit_index);

							if (reply_unit->unit.player_index == NONE)
							{
								boolean chance_succeeded = reply_to_player;

								if (!reply_to_player)
								{
									if (reply->chance > 0.f)
									{
										real random = real_random();

										if ((game_connection() == _game_connection_local && ai_debug.communication_random_disabled) || random < reply->chance)
										{
											chance_succeeded = TRUE;
										}
										else if (ai_debug.print_communication)
										{
											strcat(printbuffer, csprintf(
												temporary,
												"rand %.2f>%.2f ",
												random,
												reply->chance));
										}
									}
									else if (ai_debug.print_communication)
									{
										strcat(printbuffer, "0-chance ");
									}
								}

								if (chance_succeeded)
								{
									if (reply->reply_filter && !reply->reply_filter(unit_index, ai_information, reply_unit->unit.actor_index))
									{
										if (ai_debug.print_communication)
										{
											strcat(printbuffer, "filter ");
										}
									}
									else
									{
										char debugbuf[512];
										short reply_vocalization_type = reply->vocalization_type;
										long sound_definition_index = NONE;
										real weight = 1.f;
										short delay_ticks = (short)(reply->delay_time * TICKS_PER_SECOND);
										short play_type = ai_communication_consider_speech(
											reply_unit_index,
											reply->communication_priority,
											unit_speech_priority,
											delay_ticks,
											FALSE,
											FALSE,
											&reply_vocalization_type,
											&weight,
											&sound_definition_index,
											debugbuf);

										if (play_type > _unit_play_speech_none)
										{
											struct unit_speech_item speech;

											speech.priority = unit_speech_priority;
											speech.vocalization_type = reply_vocalization_type;
											speech.sound_definition_index = sound_definition_index;
											speech.delay_time = delay_ticks;
											speech.ai_notification_delay = (short)(communication_notification_delays[reply->communication_priority] * TICKS_PER_SECOND);
											speech.pause_time = 24;
											speech.ai.target_unit_index = unit_index;
											speech.ai.communication_type = NONE;
											speech.ai.dialogue_type_index = NONE;
											speech.ai.damage_category = NONE;
											speech.ai.updated_dialogue_timers = TRUE;
											speech.ai.look_priority = _secondary_look_priority_none;
											speech.ai.look_type = _ai_information_look_none;
											speech.ai.information_type = _ai_information_none;
											memset(&speech.ai.information_data, 0, sizeof(speech.ai.information_data));
											unit_speak(reply_unit_index, play_type, &speech);
											ai_communication_update_speech_timers(
												reply_unit_index,
												unit_speech_priority,
												reply_vocalization_type,
												NONE,
												reply_index);
											ai_communication_look_secondary_at_unit(
												reply_unit->unit.actor_index,
												_secondary_look_communicating_prop,
												communication_protagonist_default_look_priorities[reply->communication_priority],
												unit_index,
												NONE);

											if (ai_debug.print_communication)
											{
												strcat(printbuffer, strupr(csprintf(
													temporary,
													">>%s<<",
													dialogue_get_vocalization_name(speech.vocalization_type, TRUE))));
											}
											break;
										}

										if (ai_debug.print_communication)
										{
											strcat(printbuffer, csprintf(
												temporary,
												"u-%s-%s ",
												(weight > 0.f) ? "dis" : "n/a",
												debugbuf));
										}
									}
								}
								else if (ai_debug.print_communication)
								{
									strcat(printbuffer, "rand-failed ");
								}
							}
							else if (ai_debug.print_communication)
							{
								strcat(printbuffer, "playercant ");
							}
						}
						else if (ai_debug.print_communication)
						{
							strcat(printbuffer, "nobody ");
						}
					}
				}
				else if (ai_debug.print_communication)
				{
					strcat(printbuffer, "wrong-dmg ");
				}
			}

			reply++;
			reply_index++;
		}

		if (any_reply && ai_debug.print_communication && !suppress_print)
		{
			error(_error_silent, printbuffer);
		}
	}

	return;
}

static void actor_reset_idle_vocalization_timer(
	long actor_index)
{
	real idle_time;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *definition = actor_definition_get(actor->meta.definition_index);
	boolean in_combat = actor_in_combat(actor_index);
	short delay_ticks = 0;

	if (actor->meta.unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		if (unit->unit.speech.current.priority > _unit_speech_none)
		{
			delay_ticks = unit->unit.speech.sound_timer;
		}
	}

	if (in_combat)
	{
		idle_time = real_random_range(definition->communication.idle_combat_time_lower_bound, definition->communication.idle_combat_time_upper_bound);
	}
	else
	{
		idle_time = real_random_range(definition->communication.idle_noncombat_time_lower_bound, definition->communication.idle_noncombat_time_upper_bound);
	}

	actor->control.idle_vocalization_combat = in_combat;
	actor->control.idle_vocalization_timer = (short)(idle_time * TICKS_PER_SECOND + delay_ticks);

	return;
}

void actor_communication_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->state.mode >= _actor_mode_alert && ai_globals->dialogue_triggers_enabled)
	{
		boolean in_combat = actor_in_combat(actor_index);

		if (!actor->control.idle_vocalization_timer || actor->control.idle_vocalization_combat != in_combat)
		{
			actor_reset_idle_vocalization_timer(actor_index);
		}

		if (actor->control.idle_vocalization_timer > 0)
		{
			actor->control.idle_vocalization_timer--;

			if (!actor->control.idle_vocalization_timer)
			{
				short vocalization_type = in_combat ? _vocalization_idle_combat : _vocalization_idle_noncombat;
				long sound_definition_index = NONE;
				short play_type = unit_test_speech(
					actor->meta.unit_index,
					_unit_speech_idle,
					TRUE,
					FALSE,
					NULL,
					&vocalization_type,
					&sound_definition_index);

				if (play_type > _unit_play_speech_none)
				{
					struct unit_speech_item speech;

					memset(&speech, 0, sizeof(speech));
					speech.vocalization_type = vocalization_type;
					speech.sound_definition_index = sound_definition_index;
					speech.priority = _unit_speech_idle;
					ai_communication_packet_new(&speech.ai);
					unit_speak(actor->meta.unit_index, play_type, &speech);
				}
			}
		}
	}

	return;
}

static boolean reply_filter_close(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (reply_actor_index != NONE)
	{
		long prop_index = prop_get_base_by_unit_index(reply_actor_index, original_unit_index, TRUE, TRUE);

		if (prop_index != NONE)
		{
			struct prop_datum *prop = prop_get(prop_index);

			if (prop->distance < 5.f && (prop->line_of_sight == _ai_line_of_sight_clear || prop->line_of_sight == _ai_line_of_sight_occluded))
			{
				result = TRUE;
			}
		}
	}

	return result;
}

static boolean reply_filter_not_close(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (reply_actor_index != NONE)
	{
		long prop_index = prop_get_base_by_unit_index(reply_actor_index, original_unit_index, TRUE, TRUE);

		if (prop_index != NONE)
		{
			struct prop_datum *prop = prop_get(prop_index);

			if (prop->distance > 5.f || (prop->line_of_sight != _ai_line_of_sight_clear && prop->line_of_sight != _ai_line_of_sight_occluded))
			{
				result = TRUE;
			}
		}
	}

	return result;
}

static boolean reply_filter_searching(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (reply_actor_index != NONE)
	{
		struct actor_datum *actor = actor_get(reply_actor_index);

		switch (actor->state.action)
		{
		case _actor_action_uncover:
			result = actor->state.action_data.uncover.pursuit_location.type == _pursuit_location_position;
			break;
		case _actor_action_search:
			result = TRUE;
			break;
		}
	}

	return result;
}

static boolean reply_filter_same_platoon(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (reply_filter_close(original_unit_index, communication, reply_actor_index))
	{
		long original_actor_index = unit_get(original_unit_index)->unit.actor_index;

		if (original_actor_index != NONE && reply_actor_index != NONE)
		{
			struct actor_datum *original_actor = actor_get(original_actor_index);
			struct actor_datum *reply_actor = actor_get(reply_actor_index);

			result = original_actor->meta.encounter_index != NONE &&
				original_actor->meta.encounter_index == reply_actor->meta.encounter_index &&
				original_actor->meta.platoon_index == reply_actor->meta.platoon_index;
		}
	}

	return result;
}

static boolean reply_filter_fighting(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	return actor_is_fighting(reply_actor_index);
}

static boolean reply_filter_fighting_close(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (reply_filter_close(original_unit_index, communication, reply_actor_index) && actor_is_fighting(reply_actor_index))
	{
		result = TRUE;
	}

	return result;
}

static boolean reply_filter_same_target(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (reply_filter_close(original_unit_index, communication, reply_actor_index))
	{
		long original_actor_index = unit_get(original_unit_index)->unit.actor_index;

		if (original_actor_index != NONE && reply_actor_index != NONE)
		{
			struct actor_datum *original_actor = actor_get(original_actor_index);
			struct actor_datum *reply_actor = actor_get(reply_actor_index);

			if (original_actor->target.target_prop_index != NONE && reply_actor->target.target_prop_index != NONE)
			{
				struct prop_datum *original_prop = prop_get(original_actor->target.target_prop_index);
				struct prop_datum *reply_prop = prop_get(reply_actor->target.target_prop_index);

				result = original_prop->unit_index == reply_prop->unit_index;
			}
		}
	}

	return result;
}

static boolean reply_filter_no_certain_target(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (reply_actor_index != NONE)
	{
		struct actor_datum *actor = actor_get(reply_actor_index);

		if (actor->state.mode == _actor_mode_combat && actor->state.combat_status < _actor_combat_status_certain)
		{
			result = TRUE;
		}
	}

	return result;
}

static boolean reply_filter_flee_leader(
	long original_unit_index,
	struct ai_information_packet *communication,
	long reply_actor_index)
{
	boolean result = FALSE;

	if (actor_is_fighting(reply_actor_index) && actor_get(reply_actor_index)->meta.type == _actor_elite)
	{
		result = TRUE;
	}

	return result;
}

static void ai_communication_handle_received_looking(
	long actor_index,
	long prop_index,
	struct ai_information_packet *information)
{
	if (information->look_type > _ai_information_look_none)
	{
		struct prop_datum *prop = prop_get(prop_index);
		short type = _secondary_look_communicated_direction;

		if (information->look_type == _ai_information_look_unit && information->look_data.unit.unit_index == prop->unit_index)
		{
			type = _secondary_look_communicating_prop;
		}

		switch (information->look_type)
		{
		case _ai_information_look_unit:
			ai_communication_look_secondary_at_unit(
				actor_index,
				type,
				information->look_priority,
				information->look_data.unit.unit_index,
				NONE);
			break;
		case _ai_information_look_object:
			ai_communication_look_secondary_at_object(actor_index, type, information->look_priority, information->look_data.object.object_index);
			break;
		}
	}

	return;
}

static short ai_communication_consider_speech(
	long unit_index,
	short communication_priority,
	short speech_priority,
	short delay_ticks,
	boolean allow_vocalization_lookup,
	boolean allow_recent_disabling,
	short *vocalization_type,
	real *weight,
	long *sound_definition_index_reference,
	char *debugstring)
{
	long last_speech_time;
	short play_type;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 3098, vocalization_type && sound_definition_index_reference && weight);
	play_type = unit_test_speech(
		unit_index,
		speech_priority,
		allow_vocalization_lookup,
		TRUE,
		&last_speech_time,
		vocalization_type,
		sound_definition_index_reference);

	if (play_type == _unit_play_speech_none)
	{
		if (debugstring)
		{
			sprintf(debugstring, "nospch-%s", unit_get_speech_priority_name(speech_priority));
		}
	}
	else if (play_type == _unit_play_speech_queue)
	{
		*weight *= 0.3f;
	}

	if ((game_connection() != _game_connection_local || !ai_debug.communication_unit_repeat_disabled) &&
		allow_recent_disabling &&
		communication_priority < _ai_communication_priority_shout &&
		last_speech_time != NONE)
	{
		long elapsed_ticks = game_time_get() - last_speech_time;
		short time_since_speech = (elapsed_ticks < 0) ? 0 : elapsed_ticks;
		short tolerance_ticks = (short)(communication_timer_tolerances[communication_priority][_communication_rating_normal][_communication_timer_unit] * TICKS_PER_SECOND + delay_ticks);

		if (time_since_speech <= tolerance_ticks)
		{
			play_type = _unit_play_speech_none;
			*weight = 0.f;

			if (debugstring)
			{
				sprintf(debugstring, "spk%d<tol%d+%d", time_since_speech, delay_ticks, tolerance_ticks - delay_ticks);
			}
		}
		else if (time_since_speech < tolerance_ticks + communication_unit_prefer_silent_time)
		{
			*weight = (time_since_speech - tolerance_ticks) * *weight / communication_unit_prefer_silent_time;
		}
	}

	match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 3145, (play_type == _unit_play_speech_none) || (*weight > 0.0f));

	return play_type;
}

short actor_communication_team(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	short race = actor_type_get_race(actor->meta.type);
	short communication_team = NONE;

	if (race & _race_human)
	{
		communication_team = _comm_team_human;
	}
	else if (race & _race_covenant)
	{
		communication_team = _comm_team_covenant;
	}

	return communication_team;
}

static void ai_communication_update_speech_timers(
	long unit_index,
	short priority,
	short vocalization_type,
	short dialogue_type_index,
	short reply_table_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct actor_datum *actor = (unit->unit.actor_index == NONE) ? NULL : actor_get(unit->unit.actor_index);
	long current_time = game_time_get();
	long finish_time = MAX(0, unit->unit.speech.sound_timer - communication_overlap_time_modifier) + current_time;

	unit->unit.speech.last_speech_finished_time = finish_time;

	if (actor)
	{
		short communication_team;

		actor_reset_idle_vocalization_timer(unit->unit.actor_index);
		communication_team = actor_communication_team(unit->unit.actor_index);

		if (communication_team != NONE)
		{
			if (priority <= _unit_speech_shout)
			{
				ai_globals->last_chatter_time[communication_team] = MAX(ai_globals->last_chatter_time[communication_team], finish_time);

				if (priority >= _unit_speech_talk)
				{
					ai_globals->last_talk_time[communication_team] = MAX(ai_globals->last_talk_time[communication_team], finish_time);
				}

				if (priority >= _unit_speech_shout)
				{
					ai_globals->last_shout_time[communication_team] = MAX(ai_globals->last_shout_time[communication_team], finish_time);
				}

				if (ai_debug.print_speech_timers)
				{
					error(
						_error_silent,
						"%s %s %d/%s: %s %d",
						(communication_team == NONE) ? "unteamed" : global_communication_team_names[communication_team][0],
						unit_get_speech_priority_name(priority),
						dialogue_type_index,
						dialogue_get_vocalization_name(vocalization_type, TRUE),
						(priority >= _unit_speech_talk) ? "talk" : "chatter",
						finish_time - current_time);
				}
			}

			if (dialogue_type_index != NONE)
			{
				struct dialogue_usage const *dialogue;
				struct dialogue_event_status *status;

				match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 3228, (dialogue_type_index >= 0) && (dialogue_type_index < global_dialogue_event_count));
				dialogue = &global_dialogue_table[dialogue_type_index];
				status = &global_dialogue_events[communication_team + NUMBER_OF_AI_COMMUNICATION_TEAMS * dialogue_type_index];
				status->last_time_spoken = current_time;

				if (game_connection() != _game_connection_local || !ai_debug.communication_timeout_disabled)
				{
					if (dialogue->repeat_delay > 0.f)
					{
						status->disable_until_time = (long)(finish_time + dialogue->repeat_delay * TICKS_PER_SECOND);
					}
				}
			}

			if (reply_table_index != NONE)
			{
				struct reply_usage const *reply;
				struct dialogue_event_status *status;

				match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 3247, (reply_table_index >= 0) && (reply_table_index < global_reply_event_count));
				reply = &global_reply_table[reply_table_index];
				status = &global_reply_events[communication_team + NUMBER_OF_AI_COMMUNICATION_TEAMS * reply_table_index];
				status->last_time_spoken = current_time;

				if (game_connection() != _game_connection_local || !ai_debug.communication_timeout_disabled)
				{
					if (reply->repeat_delay > 0.f)
					{
						status->disable_until_time = (long)(finish_time + reply->repeat_delay * TICKS_PER_SECOND);
					}
				}
			}
		}
	}

	return;
}

static real ai_communication_actor_talk_weight(
	long actor_index,
	long subject_unit_index,
	real_point3d *subject_point,
	long cause_unit_index,
	real_point3d *cause_point,
	real stimulus_range,
	short ai_communication_type,
	short ai_communication_priority,
	short unit_speech_priority,
	short vocalization_type,
	short animation_type,
	short flags)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean has_stimulus = subject_unit_index != NONE || cause_unit_index != NONE;
	boolean valid = TRUE;
	real weight = 10.f;

	if (actor->meta.unit_index == NONE)
	{
		valid = FALSE;
	}

	if (actor->state.mode <= _actor_mode_asleep)
	{
		valid = FALSE;
	}

	if (valid && has_stimulus)
	{
		valid = (subject_unit_index != NONE && distance_squared3d(&actor->input.position.head_position, subject_point) < stimulus_range * stimulus_range) ||
			(cause_unit_index != NONE && distance_squared3d(&actor->input.position.head_position, cause_point) < stimulus_range * stimulus_range);
	}

	if (valid && TEST_FLAG(flags, _find_actor_near_to_players_bit))
	{
		real player_rating = ai_communication_get_player_rating(actor->meta.unit_index, FALSE, NULL, NULL);

		if (player_rating == 0.f)
		{
			valid = FALSE;
		}
		else
		{
			weight += 5.f * player_rating;
		}
	}

	if (valid && TEST_FLAG(flags, _find_actor_same_vehicle_bit) && subject_unit_index != NONE)
	{
		struct unit_datum *subject_unit = unit_get(subject_unit_index);

		if (subject_unit->object.parent_object_index != actor->input.vehicle_index)
		{
			valid = FALSE;
		}
	}

	if (valid && animation_type != NONE && unit_test_animation_impulse(actor->meta.unit_index, animation_type))
	{
		weight += 5.f;
	}

	if (valid && vocalization_type != NONE)
	{
		short vocalization_lookup_type = vocalization_type;
		long sound_definition_index = NONE;

		if (!ai_communication_consider_speech(
			actor->meta.unit_index,
			ai_communication_priority,
			unit_speech_priority,
			0,
			TEST_FLAG(flags, _find_actor_allow_lookup_bit),
			TRUE,
			&vocalization_lookup_type,
			&weight,
			&sound_definition_index,
			NULL))
		{
			valid = FALSE;
		}
	}

	if (valid && has_stimulus)
	{
		boolean subject_visible = FALSE;
		boolean cause_visible = FALSE;

		if (subject_unit_index != NONE)
		{
			if (actor->meta.unit_index == subject_unit_index)
			{
				if (TEST_FLAG(flags, _find_actor_allow_subject_bit))
				{
					subject_visible = TRUE;
				}
				else
				{
					valid = FALSE;
				}
			}
			else
			{
				long prop_index = prop_get_base_by_unit_index(actor_index, subject_unit_index, TRUE, FALSE);

				if (prop_index != NONE)
				{
					struct prop_datum *prop = prop_get(prop_index);

					if (prop->distance > stimulus_range)
					{
						subject_visible = FALSE;
					}
					else if (prop->state < _prop_state_becoming_unacknowledged || prop->state > _prop_state_acknowledged)
					{
						if (!prop->enemy)
						{
							if (ai_communication_type != _ai_communication_death)
							{
								subject_visible = TRUE;
							}
							else if (prop->audibility >= _actor_perception_full ||
								prop->ineffability >= _actor_perception_full ||
								actor_visibility_at_point(
									actor_index,
									&actor->input.position,
									&prop->head_position,
									prop->flashlight ? _prop_lighting_bright : prop->lighting,
									prop->line_of_sight,
									TRUE,
									FALSE,
									actor_get_perception_knowledge(actor_index, prop_index)) >= _actor_perception_full)
							{
								subject_visible = TRUE;
							}
						}
					}
					else
					{
						subject_visible = TRUE;
					}

					if (subject_visible)
					{
						weight += (1.f - prop->distance / stimulus_range) * 10.f;
					}
				}
			}
		}

		if (cause_unit_index != NONE)
		{
			if (actor->meta.unit_index == cause_unit_index)
			{
				if (TEST_FLAG(flags, _find_actor_allow_cause_bit))
				{
					cause_visible = TRUE;
				}
				else
				{
					valid = FALSE;
				}
			}
			else
			{
				long prop_index = prop_get_active_by_unit_index(actor_index, cause_unit_index);

				if (prop_index != NONE)
				{
					struct prop_datum *prop = prop_get(prop_index);

					if (prop->distance > stimulus_range)
					{
						cause_visible = FALSE;
					}
					else if (prop->state < _prop_state_becoming_unacknowledged || prop->state > _prop_state_acknowledged)
					{
						if (!prop->enemy)
						{
							cause_visible = TRUE;
						}
					}
					else
					{
						cause_visible = TRUE;
					}

					if (cause_visible)
					{
						weight += (1.f - prop->distance / stimulus_range) * 10.f;
					}
				}
			}
		}

		if (valid)
		{
			valid = cause_visible | subject_visible;
		}
	}

	return valid ? weight : 0.f;
}

static long ai_communication_find_specific_actor_to_talk(
	long ai_index,
	long subject_unit_index,
	long cause_unit_index,
	real max_distance,
	short ai_communication_type,
	short ai_communication_priority,
	short unit_speech_priority,
	short vocalization_type,
	short animation_type,
	short flags)
{
	long best_actor_index = NONE;
	real best_weight = 0.f;

	if (ai_index != NONE)
	{
		struct ai_index_actor_iterator iterator;
		real_point3d subject_point;
		real_point3d cause_point;

		if (subject_unit_index != NONE)
		{
			unit_get_head_position(subject_unit_index, &subject_point);
		}

		if (cause_unit_index != NONE)
		{
			unit_get_head_position(cause_unit_index, &cause_point);
		}

		ai_index_actor_iterator_new(ai_index, &iterator);

		while (ai_index_actor_iterator_next(&iterator))
		{
			real weight = ai_communication_actor_talk_weight(
				iterator.iterator.index,
				subject_unit_index,
				&subject_point,
				cause_unit_index,
				&cause_point,
				max_distance,
				ai_communication_type,
				ai_communication_priority,
				unit_speech_priority,
				vocalization_type,
				animation_type,
				flags);

			if (weight > best_weight)
			{
				best_weight = weight;
				best_actor_index = iterator.iterator.index;
			}
		}
	}

	return best_actor_index;
}

static long ai_communication_find_global_actor_to_talk(
	short team_index,
	short find_actor_mode,
	long subject_unit_index,
	long cause_unit_index,
	real max_distance,
	short ai_communication_type,
	short ai_communication_priority,
	short unit_speech_priority,
	short vocalization_type,
	short animation_type,
	short flags)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;
	real_point3d subject_point;
	real_point3d cause_point;
	long best_actor_index = NONE;
	real best_weight = 0.f;

	if (subject_unit_index != NONE)
	{
		unit_get_head_position(subject_unit_index, &subject_point);
	}

	if (cause_unit_index != NONE)
	{
		unit_get_head_position(subject_unit_index, &subject_point);
	}

	actor_iterator_new(&iterator, TRUE);

	while (actor = actor_iterator_next(&iterator))
	{
		boolean valid = FALSE;

		if (team_index == NONE)
		{
			valid = TRUE;
		}
		else
		{
			boolean enemy = game_team_is_enemy(team_index, actor->meta.team_index);

			switch (find_actor_mode)
			{
			case _find_actor_mode_same_team:
				valid = actor->meta.team_index == team_index;
				break;
			case _find_actor_mode_friend:
				valid = !enemy;
				break;
			case _find_actor_mode_enemy:
				valid = enemy;
				break;
			default:
				match_unreachable("c:\\halo\\SOURCE\\ai\\ai_communication.c", 3581);
				break;
			}
		}

		if (valid)
		{
			real weight = ai_communication_actor_talk_weight(
				iterator.index,
				subject_unit_index,
				&subject_point,
				cause_unit_index,
				&cause_point,
				max_distance,
				ai_communication_type,
				ai_communication_priority,
				unit_speech_priority,
				vocalization_type,
				animation_type,
				flags);

			if (weight > best_weight)
			{
				best_weight = weight;
				best_actor_index = iterator.index;
			}
		}
	}

	return best_actor_index;
}

static void ai_communication_look_secondary_at_unit(
	long actor_index,
	short type,
	short priority,
	long look_unit_index,
	long prop_index)
{
	if (actor_index != NONE && priority > _secondary_look_priority_none && look_unit_index != NONE && unit_try_and_get(look_unit_index))
	{
		struct direction_specification direction;

		if (prop_index == NONE)
		{
			prop_index = prop_get_active_by_unit_index(actor_index, look_unit_index);
		}

		if (prop_index != NONE)
		{
			short state = prop_get(prop_index)->state;

			if (state < _prop_state_becoming_unacknowledged || state > _prop_state_acknowledged)
			{
				prop_index = NONE;
			}
		}

		if (prop_index != NONE)
		{
			direction.type = _direction_specification_prop;
			direction.prop_index = prop_index;
		}
		else
		{
			direction.type = _direction_specification_point;
			unit_get_head_position(look_unit_index, &direction.point);
		}

		actor_look_secondary(actor_index, type, priority, &direction);
	}

	return;
}

static void ai_communication_look_secondary_at_object(
	long actor_index,
	short type,
	short priority,
	long object_index)
{
	if (actor_index != NONE && priority > _secondary_look_priority_none && object_index != NONE && object_try_and_get(object_index))
	{
		struct direction_specification direction;

		direction.type = _direction_specification_object;
		direction.object_index = object_index;
		actor_look_secondary(actor_index, type, priority, &direction);
	}

	return;
}

real ai_communication_get_player_rating(
	long unit_index,
	boolean test_line_of_sight,
	long *unit_index_reference,
	real *distance_reference)
{
	struct data_iterator iterator;
	struct player_datum *player;
	real_point3d unit_head_position;
	long best_unit_index = NONE;
	real best_rating = 0.f;
	real best_distance = REAL_MAX;
	boolean found_player = FALSE;

	unit_get_head_position(unit_index, &unit_head_position);
	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->unit_index != NONE)
		{
			real_point3d player_head_position;
			real_vector3d vector_from_player;
			real distance_squared;

			found_player = TRUE;
			unit_get_head_position(player->unit_index, &player_head_position);
			vector_from_points3d(&player_head_position, &unit_head_position, &vector_from_player);
			distance_squared = magnitude_squared3d(&vector_from_player);

			if (distance_squared < communication_player_absolute_range * communication_player_absolute_range)
			{
				boolean not_potentially_visible = FALSE;
				boolean obstructed = FALSE;
				boolean visible = FALSE;

				if (test_line_of_sight)
				{
					short unit_cluster_index = object_get(object_get_ultimate_parent(unit_index))->object.location.cluster_index;
					short player_cluster_index = object_get(object_get_ultimate_parent(player->unit_index))->object.location.cluster_index;

					if (unit_cluster_index == NONE || player_cluster_index == NONE || scenario_test_pvs(unit_cluster_index, player_cluster_index))
					{
						struct collision_result collision;
						boolean blocked;

						ai_profile.meters[_ai_meter_collision_vector].current_count++;
						match_collision_log_begin_user("c:\\halo\\SOURCE\\ai\\ai_communication.c", 3729, _collision_user_ai_comms);
						blocked = collision_test_line(
							FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit) | FLAG(_collision_test_ignore_two_sided_surfaces_bit) | FLAG(_collision_test_structure_bit),
							&player_head_position,
							&unit_head_position,
							NONE,
							&collision);
						match_collision_log_end_user("c:\\halo\\SOURCE\\ai\\ai_communication.c", 3735);
						visible = distance_squared < communication_player_ideal_range_min * communication_player_ideal_range_min || !blocked;
					}
					else
					{
						not_potentially_visible = TRUE;
					}
				}

				if (!not_potentially_visible)
				{
					real rating = 1.f;
					real distance = square_root(distance_squared);

					if (distance < communication_player_ideal_range_max)
					{
						if (distance < communication_player_ideal_range_min)
						{
							rating += 1.f;
						}
						else
						{
							rating += (communication_player_ideal_range_max - distance) / (communication_player_ideal_range_max - communication_player_ideal_range_min);
						}

						if (visible)
						{
							rating += 0.5f;
						}

						if (distance > _real_epsilon)
						{
							real_vector3d player_aiming_vector;
							real cosine;

							unit_get_aiming_vector(player->unit_index, &player_aiming_vector);
							cosine = dot_product3d(&vector_from_player, &player_aiming_vector) / distance;

							if (cosine > communication_player_ideal_fov)
							{
								rating += 0.7f - (1.f - cosine) / (1.f - communication_player_ideal_fov) * 0.35f;
							}
						}
					}
					else if (obstructed)
					{
						rating -= 0.5f;
					}

					if (rating > best_rating)
					{
						best_rating = rating;
						best_unit_index = player->unit_index;
						best_distance = distance;
					}
				}
			}
		}
	}

	if (!found_player)
	{
		best_rating = 1.f;
	}

	if (distance_reference)
	{
		*distance_reference = best_distance;
	}

	if (unit_index_reference)
	{
		*unit_index_reference = best_unit_index;
	}

	return best_rating;
}

boolean ai_conversation(
	short conversation_definition_index,
	boolean scripted)
{
	struct scenario *scenario = global_scenario_get();
	boolean success = FALSE;

	if (conversation_definition_index >= 0 && conversation_definition_index < scenario->ai_conversations.count)
	{
		long conversation_index = ai_conversation_new(conversation_definition_index, scripted);

		if (ai_debug.print_conversations)
		{
			console_printf(FALSE, "%s: script tried to start conversation", ai_conversation_definition_get(conversation_definition_index)->name);
		}

		if (conversation_index == NONE)
		{
			error(_error_silent, "WARNING: too many executing conversations (ran out of MAXIMUM_CONVERSATIONS_PER_MAP %d)", MAXIMUM_CONVERSATIONS_PER_MAP);
		}
		else
		{
			boolean keep_trying = FALSE;

			if (ai_conversation_begin(conversation_index, &keep_trying))
			{
				if (ai_debug.print_conversations)
				{
					console_printf(FALSE, "%s: begun successfully", ai_conversation_definition_get(conversation_definition_index)->name);
				}

				success = TRUE;
			}
			else if (keep_trying)
			{
				if (ai_debug.print_conversations)
				{
					console_printf(
						FALSE,
						"%s: can't begin yet but will remember and keep trying it",
						ai_conversation_definition_get(conversation_definition_index)->name);
				}

				success = TRUE;
			}
			else
			{
				if (ai_debug.print_conversations)
				{
					console_printf(
						FALSE,
						"%s: could not start, and not set to keep trying... aborting (status 5)",
						ai_conversation_definition_get(conversation_definition_index)->name);
				}

				ai_conversation_finish(conversation_index, TRUE, FALSE);
			}
		}
	}

	return success;
}

short ai_conversation_status(
	short conversation_definition_index)
{
	struct data_iterator iterator;
	struct conversation_datum *conversation;
	short status = _ai_conversation_status_none;

	data_iterator_new(&iterator, conversation_data);

	while (conversation = data_iterator_next(&iterator))
	{
		if (conversation->conversation_definition_index == conversation_definition_index)
		{
			short conversation_status;

			if (!conversation->begun)
			{
				conversation_status = _ai_conversation_status_trying_to_begin;
			}
			else if (!conversation->any_line_spoken)
			{
				conversation_status = _ai_conversation_status_waiting_for_participants;
			}
			else
			{
				conversation_status = conversation->waiting_to_advance ? _ai_conversation_status_waiting_to_advance : _ai_conversation_status_playing;
			}

			status = MAX(status, conversation_status);
		}
	}

	if (status == _ai_conversation_status_none)
	{
		short recent_index;
		long latest_time = NONE;
		short latest_index = NONE;

		for (recent_index = 0; recent_index < ai_globals->recent_conversation_count; recent_index++)
		{
			if (ai_globals->recent_conversations[recent_index].definition_index == conversation_definition_index &&
				ai_globals->recent_conversations[recent_index].finish_time > latest_time)
			{
				latest_index = recent_index;
				latest_time = ai_globals->recent_conversations[recent_index].finish_time;
			}
		}

		if (latest_index != NONE)
		{
			struct recent_conversation *recent = &ai_globals->recent_conversations[latest_index];

			if (recent->unable_to_begin)
			{
				status = _ai_conversation_status_unable_to_begin;
			}
			else
			{
				status = recent->finished_successfully ? _ai_conversation_status_finished_successfully : _ai_conversation_status_finished_prematurely;
			}
		}
	}

	return status;
}

short ai_conversation_line(
	short conversation_definition_index)
{
	struct data_iterator iterator;
	struct conversation_datum *conversation;
	short line_index = 999;

	data_iterator_new(&iterator, conversation_data);

	while (conversation = data_iterator_next(&iterator))
	{
		if (conversation->conversation_definition_index == conversation_definition_index)
		{
			line_index = conversation->line_index;
			break;
		}
	}

	return line_index;
}

void ai_conversation_stop(
	short conversation_definition_index)
{
	struct data_iterator iterator;
	struct conversation_datum *conversation;

	data_iterator_new(&iterator, conversation_data);

	while (conversation = data_iterator_next(&iterator))
	{
		if (conversation->conversation_definition_index == conversation_definition_index)
		{
			if (ai_debug.print_conversations)
			{
				console_printf(FALSE, "%s: told to stop by scripting", ai_conversation_definition_get(conversation_definition_index)->name);
			}

			ai_conversation_finish(iterator.index, FALSE, FALSE);
		}
	}

	return;
}

void ai_conversation_advance(
	short conversation_definition_index)
{
	struct data_iterator iterator;
	struct conversation_datum *conversation;

	data_iterator_new(&iterator, conversation_data);

	while (conversation = data_iterator_next(&iterator))
	{
		if (conversation->conversation_definition_index == conversation_definition_index)
		{
			if (ai_debug.print_conversations)
			{
				struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation_definition_index);

				console_printf(FALSE, "%s: told to advance by scripting", conversation_definition->name);
			}

			conversation->told_to_advance = TRUE;
		}
	}

	return;
}

void ai_conversation_update(
	void)
{
	struct data_iterator iterator;
	struct conversation_datum *conversation;
	long current_time = game_time_get();

	data_iterator_new(&iterator, conversation_data);

	while (conversation = data_iterator_next(&iterator))
	{
		struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);

		if (!conversation->begun)
		{
			boolean keep_trying = TRUE;

			if (!((current_time - conversation->creation_time) % TICKS_PER_SECOND))
			{
				if (ai_debug.print_conversations)
				{
					console_printf(FALSE, "%s: trying to begin", conversation_definition->name);
				}

				ai_conversation_begin(iterator.index, &keep_trying);
			}

			if (!conversation->begun && !keep_trying)
			{
				if (ai_debug.print_conversations)
				{
					console_printf(FALSE, "%s: unable to begin, and no point in continuing", conversation_definition->name);
				}

				ai_conversation_finish(iterator.index, TRUE, FALSE);
			}
		}

		if (conversation->begun && !conversation->finished)
		{
			boolean line_valid = conversation->line_index >= 0 && conversation->line_index < conversation_definition->lines.count;

			while (!line_valid || ai_conversation_line_perform(iterator.index))
			{
				if (line_valid)
				{
					ai_conversation_line_end(iterator.index);
				}

				conversation->line_index++;

				if (conversation->line_index >= conversation_definition->lines.count)
				{
					if (ai_debug.print_conversations)
					{
						console_printf(FALSE, "%s: no more lines to play", conversation_definition->name);
					}

					conversation->finished = TRUE;
					break;
				}

				line_valid = ai_conversation_line_begin(iterator.index);
			}
		}

		if (conversation->finished)
		{
			ai_conversation_finish(iterator.index, FALSE, TRUE);
		}
		else if (conversation->begun)
		{
			short participant_index;

			for (participant_index = 0; participant_index < conversation_definition->participants.count; participant_index++)
			{
				if (TEST_FLAG(conversation->participant_bitmask, participant_index) && conversation->actor_indices[participant_index] != NONE)
				{
					struct actor_datum *actor = actor_get(conversation->actor_indices[participant_index]);

					actor->external_orders.conversation_index = iterator.index;
					actor->external_orders.conversation_attention_unit_index = NONE;

					if (actor->meta.unit_index == conversation->line_unit_index)
					{
						actor->external_orders.conversation_attention_unit_index = conversation->line_address_unit_index;
					}
					else if (actor->meta.unit_index == conversation->line_address_unit_index && TEST_FLAG(conversation->line_flags, _ai_conversation_line_addressee_look_back_bit))
					{
						actor->external_orders.conversation_attention_unit_index = conversation->line_unit_index;
					}
					else if (TEST_FLAG(conversation->line_flags, _ai_conversation_line_everyone_look_at_speaker_bit))
					{
						actor->external_orders.conversation_attention_unit_index = conversation->line_unit_index;
					}
					else if (TEST_FLAG(conversation->line_flags, _ai_conversation_line_everyone_look_at_addressee_bit))
					{
						actor->external_orders.conversation_attention_unit_index = conversation->line_address_unit_index;
					}
				}
			}
		}
	}

	return;
}

void ai_conversation_actor_deleted(
	long actor_index)
{
	struct data_iterator iterator;
	struct conversation_datum *conversation;

	data_iterator_new(&iterator, conversation_data);

	while (conversation = data_iterator_next(&iterator))
	{
		short participant_index;
		struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);

		for (participant_index = 0; participant_index < conversation_definition->participants.count; participant_index++)
		{
			if (conversation->actor_indices[participant_index] == actor_index)
			{
				if (TEST_FLAG(conversation_definition->flags, _ai_conversation_stop_if_anyone_dies_bit))
				{
					ai_conversation_finish(iterator.index, FALSE, FALSE);
					break;
				}

				SET_FLAG(conversation->participant_bitmask, participant_index, FALSE);
				conversation->actor_indices[participant_index] = NONE;

				if (conversation->line_participant_index == participant_index)
				{
					conversation->line_advance = TRUE;
				}
			}
		}
	}

	return;
}

void ai_conversation_unit_died(
	long unit_index,
	boolean deleted)
{
	struct data_iterator iterator;
	struct conversation_datum *conversation;

	data_iterator_new(&iterator, conversation_data);

	while (conversation = data_iterator_next(&iterator))
	{
		struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);
		boolean abort = FALSE;

		if (conversation->line_unit_index == unit_index)
		{
			abort = TRUE;
			conversation->line_advance = TRUE;
			conversation->line_unit_index = NONE;
		}

		if (conversation->line_address_unit_index == unit_index)
		{
			abort = TRUE;
			conversation->line_address_unit_index = NONE;
		}

		if (conversation->triggering_player_unit_index == unit_index)
		{
			abort = TRUE;
			conversation->triggering_player_unit_index = NONE;
		}

		if (deleted || TEST_FLAG(conversation_definition->flags, _ai_conversation_stop_if_anyone_dies_bit))
		{
			short participant_index;

			for (participant_index = 0; participant_index < conversation_definition->participants.count; participant_index++)
			{
				if (TEST_FLAG(conversation->participant_bitmask, participant_index) && conversation->actor_indices[participant_index] != NONE)
				{
					struct actor_datum *actor = actor_get(conversation->actor_indices[participant_index]);

					if (actor->meta.unit_index == unit_index)
					{
						abort = TRUE;
					}

					if (deleted)
					{
						if (actor->state.action == _actor_action_converse && actor->state.action_data.converse.run_to_unit_index == unit_index)
						{
							actor->state.action_data.converse.run_to_unit_index = NONE;
						}

						if (actor->external_orders.conversation_attention_unit_index == unit_index)
						{
							actor->external_orders.conversation_attention_unit_index = NONE;
						}
					}
				}
			}

			if (abort)
			{
				if (ai_debug.print_conversations)
				{
					console_printf(FALSE, "%s: unit died, aborting", conversation_definition->name);
				}

				ai_conversation_finish(iterator.index, FALSE, FALSE);
				break;
			}
		}
	}

	return;
}

void ai_conversation_finish(
	long conversation_index,
	boolean unable_to_begin,
	boolean success)
{
	if (conversation_index != NONE)
	{
		struct conversation_datum *finished_conversation;
		short recent_index;
		short participant_index;
		struct conversation_datum *conversation = conversation_get(conversation_index);
		struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);

		if (ai_debug.print_conversations)
		{
			console_printf(
				FALSE,
				"%s: finished %s%s",
				conversation_definition->name,
				success ? "successfully" : "prematurely",
				unable_to_begin ? " (unable to begin)" : "");
		}

		finished_conversation = conversation_get(conversation_index);
		recent_index = ai_globals->recent_conversation_next_index;
		ai_globals->recent_conversation_next_index = recent_index + 1;
		ai_globals->recent_conversation_next_index %= (short)NUMBEROF(ai_globals->recent_conversations);
		ai_globals->recent_conversation_count = MAX(ai_globals->recent_conversation_count, recent_index + 1);
		ai_globals->recent_conversations[recent_index].definition_index = finished_conversation->conversation_definition_index;
		ai_globals->recent_conversations[recent_index].unable_to_begin = unable_to_begin;
		ai_globals->recent_conversations[recent_index].finished_successfully = success;
		ai_globals->recent_conversations[recent_index].finish_time = game_time_get();

		for (participant_index = 0; participant_index < conversation_definition->participants.count; participant_index++)
		{
			if (TEST_FLAG(conversation->participant_bitmask, participant_index))
			{
				long actor_index = conversation->actor_indices[participant_index];

				if (actor_index != NONE)
				{
					struct actor_datum *actor = actor_get(actor_index);

					actor->external_orders.conversation_index = NONE;
					actor->external_orders.conversation_attention_unit_index = NONE;

					if (actor->state.action == _actor_action_converse)
					{
						actor->state.action_data.converse.conversation_index = NONE;
					}
				}
			}
		}

		datum_delete(conversation_data, conversation_index);
	}

	return;
}

static boolean ai_conversation_begin(
	long conversation_index,
	boolean *continue_trying)
{
	short index;
	boolean participant_not_ready;
	boolean participant_missing;
	struct conversation_datum *conversation = conversation_get(conversation_index);
	struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);
	unsigned long possible_participant_mask = 0;
	boolean try_alternates = FALSE;
	boolean alternate_found = FALSE;
	boolean found_specific_unit = FALSE;
	boolean success = TRUE;
	boolean wait = FALSE;
	real best_participant_distance = REAL_MAX;

	conversation->participant_bitmask = 0;
	memset(conversation->actor_indices, NONE, sizeof(conversation->actor_indices));
	memset(conversation->dialogue_indices, NONE, sizeof(conversation->dialogue_indices));

	for (index = 0; index < conversation_definition->participants.count; index++)
	{
		struct ai_conversation_participant *participant = TAG_BLOCK_GET_ELEMENT(&conversation_definition->participants, index, struct ai_conversation_participant);

		if (!TEST_FLAG(participant->flags, _ai_conversation_participant_is_alternate_bit))
		{
			boolean possible_in_future = FALSE;

			ai_conversation_find_participant(
				conversation_index,
				index,
				&found_specific_unit,
				&try_alternates,
				&possible_in_future,
				&best_participant_distance);
			SET_FLAG(possible_participant_mask, index, possible_in_future);
		}
	}

	if (success && try_alternates)
	{
		for (index = 0; index < conversation_definition->participants.count; index++)
		{
			struct ai_conversation_participant *participant = TAG_BLOCK_GET_ELEMENT(&conversation_definition->participants, index, struct ai_conversation_participant);

			if (!TEST_FLAG(conversation->participant_bitmask, index) && TEST_FLAG(participant->flags, _ai_conversation_participant_is_alternate_bit))
			{
				boolean possible_in_future = FALSE;

				if (ai_conversation_find_participant(conversation_index, index, &found_specific_unit, NULL, &possible_in_future, &best_participant_distance))
				{
					alternate_found = TRUE;
				}
				else
				{
					SET_FLAG(possible_participant_mask, index, possible_in_future);
				}
			}
		}
	}

	participant_not_ready = FALSE;
	participant_missing = FALSE;

	for (index = 0; index < conversation_definition->participants.count; index++)
	{
		struct ai_conversation_participant *participant = TAG_BLOCK_GET_ELEMENT(&conversation_definition->participants, index, struct ai_conversation_participant);

		if (!TEST_FLAG(participant->flags, _ai_conversation_participant_optional_bit) &&
			!TEST_FLAG(conversation->participant_bitmask, index) &&
			(!TEST_FLAG(participant->flags, _ai_conversation_participant_has_alternate_bit) || !alternate_found) &&
			(!TEST_FLAG(participant->flags, _ai_conversation_participant_is_alternate_bit) || try_alternates))
		{
			if (TEST_FLAG(possible_participant_mask, index))
			{
				participant_not_ready = TRUE;
			}
			else
			{
				participant_missing = TRUE;
			}

			if (ai_debug.print_conversations)
			{
				char const *name = "none";
				short name_index = (participant->preexisting_object_name_index != NONE) ? participant->preexisting_object_name_index : participant->new_attach_object_name_index;

				if (name_index >= 0 && name_index < global_scenario_get()->object_names.count)
				{
					name = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, name_index, struct scenario_object_name)->name;
				}

				if (TEST_FLAG(possible_participant_mask, index))
				{
					console_printf(
						FALSE,
						"%s: found participant %d/%s but not ready to talk yet",
						conversation_definition->name,
						index,
						name);
				}
				else
				{
					console_printf(
						FALSE,
						"%s: could not find participant %d/%s",
						conversation_definition->name,
						index,
						name);
				}
			}
			break;
		}
	}

	if (participant_missing)
	{
		success = FALSE;
	}
	else if (participant_not_ready)
	{
		success = FALSE;
		wait = TRUE;
	}

	if (success &&
		TEST_FLAG(conversation_definition->flags, _ai_conversation_keep_trying_to_play_bit) &&
		conversation_definition->trigger_dist > 0.f &&
		found_specific_unit &&
		best_participant_distance > conversation_definition->trigger_dist)
	{
		if (ai_debug.print_conversations)
		{
			console_printf(
				FALSE,
				"%s: participants currently outside trigger-dist %.1f > %.1f, must wait",
				conversation_definition->name,
				best_participant_distance,
				conversation_definition->trigger_dist);
		}

		success = FALSE;
		wait = TRUE;
	}

	conversation->triggering_player_unit_index = NONE;

	if (success && TEST_FLAG(conversation_definition->flags, _ai_conversation_player_must_be_visible_bit))
	{
		if (!found_specific_unit)
		{
			success = FALSE;
		}
		else
		{
			struct data_iterator iterator;
			struct player_datum *player;
			real closest_player_distance = REAL_MAX;

			data_iterator_new(&iterator, player_data);

			while (player = data_iterator_next(&iterator))
			{
				if (player->unit_index != NONE)
				{
					short index;
					real closest_participant_distance = REAL_MAX;

					for (index = 0; index < conversation_definition->participants.count; index++)
					{
						if (conversation->actor_indices[index] != NONE)
						{
							long prop_index = prop_get_active_by_unit_index(conversation->actor_indices[index], player->unit_index);

							if (prop_index != NONE)
							{
								struct prop_datum *prop = prop_get(prop_index);

								if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged)
								{
									closest_participant_distance = MIN(closest_participant_distance, prop->distance);
								}
							}
						}
					}

					if (closest_participant_distance < closest_player_distance)
					{
						conversation->triggering_player_unit_index = player->unit_index;
						closest_player_distance = closest_participant_distance;
					}
				}
			}

			if (conversation->triggering_player_unit_index == NONE)
			{
				if (ai_debug.print_conversations)
				{
					console_printf(FALSE, "%s: cannot start, nobody can see player", conversation_definition->name);
				}

				success = FALSE;

				if (TEST_FLAG(conversation_definition->flags, _ai_conversation_keep_trying_to_play_bit))
				{
					wait = TRUE;
				}
			}
		}
	}

	if (success && TEST_FLAG(conversation_definition->flags, _ai_conversation_player_must_be_looking_at_bit) && found_specific_unit)
	{
		struct data_iterator iterator;
		struct player_datum *player;
		boolean looking = FALSE;

		data_iterator_new(&iterator, player_data);

		while ((player = data_iterator_next(&iterator)) && !looking)
		{
			if (player->unit_index != NONE)
			{
				short index;

				for (index = 0; index < conversation_definition->participants.count; index++)
				{
					if (conversation->actor_indices[index] != NONE)
					{
						struct actor_datum *actor = actor_get(conversation->actor_indices[index]);

						if (unit_can_see_point(player->unit_index, &actor->input.position.head_position, DEGREES_TO_RADIANS(30)))
						{
							looking = TRUE;
							break;
						}
					}
				}
			}
		}

		if (!looking)
		{
			if (ai_debug.print_conversations)
			{
				console_printf(FALSE, "%s: cannot start, players are not looking at us", conversation_definition->name);
			}

			success = FALSE;

			if (TEST_FLAG(conversation_definition->flags, _ai_conversation_keep_trying_to_play_bit))
			{
				wait = TRUE;
			}
		}
	}

	if (success)
	{
		for (index = 0; index < conversation_definition->participants.count; index++)
		{
			if (TEST_FLAG(conversation->participant_bitmask, index) && conversation->actor_indices[index] != NONE)
			{
				struct action_state_data new_state_data;
				short dialogue_variant;
				struct ai_conversation_participant *participant = TAG_BLOCK_GET_ELEMENT(&conversation_definition->participants, index, struct ai_conversation_participant);
				struct actor_datum *actor = actor_get(conversation->actor_indices[index]);
				struct unit_datum *unit = unit_get(actor->meta.unit_index);

				if (participant->new_attach_object_name_index != NONE)
				{
					object_set_object_index_for_name_index(participant->new_attach_object_name_index, actor->meta.unit_index);
				}

				if (TEST_FLAG(conversation_definition->flags, _ai_conversation_stop_other_actions_bit) &&
					action_converse_setup(conversation->actor_indices[index], conversation_index, &new_state_data.converse))
				{
					actor_action_change(conversation->actor_indices[index], _actor_action_converse, &new_state_data);
				}

				match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 4630, (conversation->dialogue_indices[index] >= 0) && (conversation->dialogue_indices[index] < MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT));
				dialogue_variant = participant->dialogue_variants[conversation->dialogue_indices[index]];

				if (unit->object.variant_number != dialogue_variant)
				{
					unit->object.variant_number = dialogue_variant;
					SET_FLAG(unit->unit.flags, _unit_must_set_up_dialogue_bit, FALSE);
				}
			}
		}

		conversation->begun = TRUE;
	}
	else
	{
		*continue_trying = wait && TEST_FLAG(conversation_definition->flags, _ai_conversation_keep_trying_to_play_bit);
	}

	return success;
}

static long ai_conversation_new(
	short conversation_definition_index,
	boolean scripted)
{
	long conversation_index = datum_new(conversation_data);

	if (conversation_index == NONE && scripted)
	{
		struct data_iterator iterator;
		struct conversation_datum *conversation;
		boolean oldest_scripted = TRUE;
		long oldest_time = LONG_MAX;
		long oldest_index = NONE;

		data_iterator_new(&iterator, conversation_data);

		while (conversation = data_iterator_next(&iterator))
		{
			if (conversation->scripted < oldest_scripted || conversation->creation_time < oldest_time)
			{
				oldest_time = conversation->creation_time;
				oldest_index = iterator.index;
				oldest_scripted = conversation->scripted;
			}
		}

		if (oldest_index != NONE)
		{
			if (ai_debug.print_conversations)
			{
				console_printf(
					FALSE,
					"%s: this conversation is already running or trying to run, overwrite it",
					ai_conversation_definition_get(conversation_definition_index)->name);
			}

			ai_conversation_finish(oldest_index, FALSE, FALSE);
			conversation_index = datum_new_at_index(conversation_data, oldest_index);
		}
	}

	if (conversation_index != NONE)
	{
		struct conversation_datum *conversation = conversation_get(conversation_index);

		conversation->conversation_definition_index = conversation_definition_index;
		conversation->line_index = NONE;
		conversation->scripted = scripted;
		conversation->creation_time = game_time_get();
	}

	return conversation_index;
}

static boolean ai_conversation_find_participant(
	long conversation_index,
	short participant_index,
	boolean *found_specific_unit_reference,
	boolean *try_alternate_reference,
	boolean *success_with_better_player_rating_reference,
	real *best_distance_reference)
{
	struct conversation_datum *conversation = conversation_get(conversation_index);
	struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);
	struct ai_conversation_participant *participant = TAG_BLOCK_GET_ELEMENT(&conversation_definition->participants, participant_index, struct ai_conversation_participant);
	short best_variant_index = NONE;
	long best_actor_index = NONE;
	real best_distance = REAL_MAX;
	boolean success_with_better_player_rating = FALSE;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 4730, conversation_definition && conversation);

	if (participant->selection_type == _ai_conversation_selection_disembodied)
	{
		best_variant_index = 0;
		success = TRUE;
	}
	else
	{
		struct ai_index_actor_iterator ai_index_actor_iterator;
		struct actor_iterator actor_iterator;
		struct actor_datum *actor;
		long actor_index;
		boolean test_line_of_sight;
		short nearby_unit_count;
		real_point3d nearby_unit_positions[MAXIMUM_PARTICIPANTS_PER_CONVERSATION];
		short rejected_counts[7];
		short nearby_participant_index;
		boolean ignore_position = FALSE;
		boolean select_specific_unit = FALSE;
		boolean select_by_ai_index = FALSE;
		long specific_unit_index = NONE;
		real best_desirability = 0.f;
		short possible_actor_count = 0;

		memset(rejected_counts, 0, sizeof(rejected_counts));

		if (participant->selection_type == _ai_conversation_selection_radio || participant->selection_type == _ai_conversation_selection_radio_sargeant)
		{
			ignore_position = TRUE;
		}

		nearby_unit_count = 0;

		for (nearby_participant_index = 0; nearby_participant_index < conversation_definition->participants.count; nearby_participant_index++)
		{
			if (conversation->actor_indices[nearby_participant_index] != NONE)
			{
				struct actor_datum *nearby_actor = actor_get(conversation->actor_indices[nearby_participant_index]);

				match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 4781, nearby_unit_count < MAXIMUM_PARTICIPANTS_PER_CONVERSATION);
				nearby_unit_positions[nearby_unit_count++] = nearby_actor->input.position.body_position;
			}
		}

		test_line_of_sight = nearby_unit_count == 0;

		if (participant->preexisting_object_name_index != NONE)
		{
			specific_unit_index = object_index_from_name_index(participant->preexisting_object_name_index);
			select_specific_unit = TRUE;
		}
		else if (participant->runtime_ai_index != NONE)
		{
			ai_index_actor_iterator_new(participant->runtime_ai_index, &ai_index_actor_iterator);
			select_by_ai_index = TRUE;
		}
		else
		{
			actor_iterator_new(&actor_iterator, TRUE);
		}

		while (TRUE)
		{
			real desirability = 0.f;
			real actor_distance = REAL_MAX;
			short actor_variant_index = NONE;
			real player_rating = 0.f;
			struct unit_datum *player_unit = NULL;

			if (select_specific_unit)
			{
				struct unit_datum *unit = unit_try_and_get(specific_unit_index);

				actor_index = NONE;
				actor = NULL;

				if (unit && unit->unit.actor_index != NONE)
				{
					actor_index = unit->unit.actor_index;
					actor = actor_get(actor_index);
				}

				specific_unit_index = NONE;
			}
			else if (select_by_ai_index)
			{
				actor = ai_index_actor_iterator_next(&ai_index_actor_iterator);
				actor_index = ai_index_actor_iterator.iterator.index;
			}
			else
			{
				actor = actor_iterator_next(&actor_iterator);
				actor_index = actor_iterator.index;
			}

			if (!actor)
			{
				break;
			}

			possible_actor_count++;

			if (actor->meta.unit_index == NONE)
			{
				rejected_counts[0]++;
			}
			else if (actor->meta.type != participant->actor_type)
			{
				rejected_counts[1]++;
			}
			else
			{
				short conversation_participant_index;

				for (conversation_participant_index = 0; conversation_participant_index < conversation_definition->participants.count; conversation_participant_index++)
				{
					if (actor_index == conversation->actor_indices[conversation_participant_index])
					{
						break;
					}
				}

				if (conversation_participant_index < conversation_definition->participants.count)
				{
					rejected_counts[2]++;
				}
				else
				{
					long player_unit_index;

					player_rating = ai_communication_get_player_rating(actor->meta.unit_index, test_line_of_sight, &player_unit_index, &actor_distance);

					if (player_unit_index == NONE && !ignore_position)
					{
						rejected_counts[3]++;
						success_with_better_player_rating = TRUE;
					}
					else
					{
						boolean valid = TRUE;

						if (player_unit_index != NONE)
						{
							desirability = player_rating;
							player_unit = unit_get(player_unit_index);
						}

						switch (participant->selection_type)
						{
						case _ai_conversation_selection_friendly_actor:
						case _ai_conversation_selection_radio:
							if (player_unit && game_team_is_enemy(actor->meta.team_index, player_unit->object.owner_team_index))
							{
								valid = FALSE;
							}
							break;
						case _ai_conversation_selection_in_player_vehicle:
							if (player_unit && player_unit->object.parent_object_index != NONE && actor->input.vehicle_index == player_unit->object.parent_object_index)
							{
								if (actor->input.vehicle_gunner)
								{
									desirability += 1.f;
								}
							}
							else
							{
								valid = FALSE;
							}
							break;
						case _ai_conversation_selection_not_in_vehicle:
							if (actor->input.vehicle_index != NONE)
							{
								valid = FALSE;
							}
							break;
						case _ai_conversation_selection_sargeant:
						case _ai_conversation_selection_radio_sargeant:
							if (actor->meta.unique_leader)
							{
								desirability += 1.5f;
							}
							break;
						}

						if (!valid)
						{
							rejected_counts[4]++;
						}
						else if (test_line_of_sight && !ignore_position && player_rating < 2.f && conversation_definition->run_to_player_dist == 0.f)
						{
							rejected_counts[5]++;
							success_with_better_player_rating = TRUE;
						}
						else
						{
							struct unit_datum *unit;
							short unit_variant;
							short variant_index;
							short change_variant_indices_count;
							short change_variant_indices[MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT];
							short found_variant_index;
							boolean found_variant;

							if (nearby_unit_count > 0)
							{
								short nearby_unit_index;
								real closest_distance_squared = REAL_MAX;

								for (nearby_unit_index = 0; nearby_unit_index < nearby_unit_count; nearby_unit_index++)
								{
									real distance_squared = distance_squared3d(&actor->input.position.body_position, &nearby_unit_positions[nearby_unit_index]);

									closest_distance_squared = MIN(closest_distance_squared, distance_squared);
								}

								if (closest_distance_squared < 4.5f * 4.5f)
								{
									real closest_distance = square_root(closest_distance_squared);

									desirability += 1.f - (closest_distance - 1.5f) / 3.f;
								}
							}

							unit = unit_get(actor->meta.unit_index);
							unit_variant = unit->object.variant_number;
							change_variant_indices_count = 0;
							found_variant_index = NONE;
							found_variant = FALSE;

							for (variant_index = 0; variant_index < MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT; variant_index++)
							{
								if (participant->dialogue_variants[variant_index] != NONE)
								{
									if (participant->dialogue_variants[variant_index] == unit_variant)
									{
										found_variant_index = variant_index;
										found_variant = TRUE;
										break;
									}

									if (participant->dialogue_variants[variant_index] == 0)
									{
										found_variant_index = variant_index;
										found_variant = TRUE;
									}
									else if (unit_variant < 100 && participant->dialogue_variants[variant_index] < 100)
									{
										match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 5039, change_variant_indices_count < MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT);
										change_variant_indices[change_variant_indices_count++] = variant_index;
									}
								}
							}

							if (found_variant)
							{
								match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 5048, found_variant_index != NONE);
								actor_variant_index = found_variant_index;
								desirability += 0.7f;
							}
							else if (change_variant_indices_count > 0)
							{
								if (change_variant_indices_count == 1)
								{
									actor_variant_index = change_variant_indices[0];
								}
								else
								{
									actor_variant_index = change_variant_indices[random_range(0, change_variant_indices_count)];
								}

								match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 5063, (actor_variant_index >= 0) && (actor_variant_index < MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT));
							}
							else
							{
								rejected_counts[6]++;
								continue;
							}

							if (desirability > best_desirability)
							{
								best_actor_index = actor_index;
								best_desirability = desirability;
								best_distance = actor_distance;
								best_variant_index = actor_variant_index;
								success = TRUE;
							}
						}
					}
				}
			}
		}

		if (!success && ai_debug.print_conversations)
		{
			char potential_desc[256];
			char const *name = "none";
			short name_index = (participant->preexisting_object_name_index != NONE) ? participant->preexisting_object_name_index : participant->new_attach_object_name_index;

			if (name_index >= 0 && name_index < global_scenario_get()->object_names.count)
			{
				name = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, name_index, struct scenario_object_name)->name;
			}

			if (select_specific_unit)
			{
				strcpy(potential_desc, "<specific unit>");
			}
			else if (select_by_ai_index)
			{
				ai_index_to_string(participant->runtime_ai_index, global_scenario_get(), potential_desc, sizeof(potential_desc));
			}
			else
			{
				strcpy(potential_desc, "<everyone>");
			}

			console_printf(
				FALSE,
				"%s: didn't find %d/%s in %s (%d possible actors)",
				conversation_definition->name,
				participant_index,
				name,
				potential_desc,
				possible_actor_count);

			if (possible_actor_count > 0)
			{
				char tempstring[512];
				short reason_index;
				char const *rejection_reasons[7] =
				{
					"swarm",
					"wrong-type",
					"already-conversing",
					"nowhere-near-player",
					"selection",
					"not-near-player",
					"no-dialogue-match",
				};

				strcpy(tempstring, "  reasons: ");

				for (reason_index = 0; reason_index < NUMBEROF(rejected_counts); reason_index++)
				{
					if (rejected_counts[reason_index] > 0)
					{
						sprintf(&tempstring[strlen(tempstring)], "%s(%d) ", rejection_reasons[reason_index], rejected_counts[reason_index]);
					}
				}

				console_printf(FALSE, tempstring);
			}
		}
	}

	if (success)
	{
		SET_FLAG(conversation->participant_bitmask, participant_index, TRUE);
		conversation->actor_indices[participant_index] = best_actor_index;
		conversation->dialogue_indices[participant_index] = best_variant_index;

		if (found_specific_unit_reference && best_actor_index != NONE)
		{
			*found_specific_unit_reference = TRUE;
		}
	}
	else if (TEST_FLAG(participant->flags, _ai_conversation_participant_has_alternate_bit) && try_alternate_reference)
	{
		*try_alternate_reference = TRUE;
	}

	if (success_with_better_player_rating && success_with_better_player_rating_reference)
	{
		*success_with_better_player_rating_reference = TRUE;
	}

	if (best_distance_reference && *best_distance_reference > best_distance)
	{
		*best_distance_reference = best_distance;
	}

	return success;
}

static boolean ai_conversation_line_begin(
	long conversation_index)
{
	struct conversation_datum *conversation = conversation_get(conversation_index);
	struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);
	struct ai_conversation_line *line = TAG_BLOCK_GET_ELEMENT(&conversation_definition->lines, conversation->line_index, struct ai_conversation_line);
	boolean success = FALSE;

	if (line->participant_index >= 0 &&
		line->participant_index < conversation_definition->participants.count &&
		TEST_FLAG(conversation->participant_bitmask, line->participant_index))
	{
		struct ai_conversation_participant *participant = TAG_BLOCK_GET_ELEMENT(&conversation_definition->participants, line->participant_index, struct ai_conversation_participant);
		long actor_index = conversation->actor_indices[line->participant_index];

		conversation->line_participant_index = line->participant_index;

		if (actor_index == NONE)
		{
			conversation->line_actor_index = NONE;
			conversation->line_unit_index = NONE;
			conversation->line_address_unit_index = NONE;
			conversation->line_unspatialized = TRUE;
		}
		else
		{
			struct actor_datum *actor = actor_get(actor_index);

			conversation->line_actor_index = actor_index;
			conversation->line_unit_index = actor->meta.unit_index;
			conversation->line_address_unit_index = NONE;

			switch (line->address_type)
			{
			case _ai_conversation_address_player:
				conversation->line_address_unit_index = conversation->triggering_player_unit_index;
				break;
			case _ai_conversation_address_participant:
				if (line->address_participant_index >= 0 && line->address_participant_index < conversation_definition->participants.count)
				{
					long address_actor_index = conversation->actor_indices[line->address_participant_index];

					if (address_actor_index != NONE)
					{
						conversation->line_address_unit_index = actor_get(address_actor_index)->meta.unit_index;
					}
				}
				break;
			}

			conversation->line_unspatialized = participant->selection_type == _ai_conversation_selection_radio || participant->selection_type == _ai_conversation_selection_radio_sargeant;
		}

		match_assert("c:\\halo\\SOURCE\\ai\\ai_communication.c", 5227, (conversation->dialogue_indices[line->participant_index] >= 0) && (conversation->dialogue_indices[line->participant_index] < MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT));
		conversation->line_sound_index = line->dialogue[conversation->dialogue_indices[line->participant_index]].index;
		conversation->line_delay_timer = (short)(line->delay_time * TICKS_PER_SECOND);
		conversation->line_flags = line->flags;
		conversation->line_advance = FALSE;
		conversation->line_finished = FALSE;
		conversation->line_spoken = FALSE;
		success = TRUE;
	}

	return success;
}

static boolean ai_conversation_line_perform(
	long conversation_index)
{
	struct conversation_datum *conversation = conversation_get(conversation_index);
	struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);

	if (!conversation->line_advance)
	{
		if (!conversation->line_spoken)
		{
			boolean blocked = FALSE;

			if (conversation->line_sound_index != NONE)
			{
				if (TEST_FLAG(conversation->line_flags, _ai_conversation_line_wait_until_speaker_nearby_bit) || TEST_FLAG(conversation->line_flags, _ai_conversation_line_wait_until_everyone_nearby_bit))
				{
					short participant_index;

					for (participant_index = 0; participant_index < conversation_definition->participants.count; participant_index++)
					{
						long actor_index = conversation->actor_indices[participant_index];

						if (actor_index != NONE)
						{
							struct actor_datum *actor = actor_get(actor_index);

							if ((TEST_FLAG(conversation->line_flags, _ai_conversation_line_wait_until_everyone_nearby_bit) ||
								(TEST_FLAG(conversation->line_flags, _ai_conversation_line_wait_until_speaker_nearby_bit) && actor_index == conversation->line_actor_index)) &&
								actor->state.action == _actor_action_converse &&
								actor->state.action_data.converse.run_to_unit_index != NONE &&
								!actor->state.action_data.converse.in_range &&
								!actor->state.action_data.converse.failed)
							{
								blocked = TRUE;
							}
						}
					}
				}

				if (sound_scripted_dialog_is_playing())
				{
					blocked = TRUE;
				}

				if (!blocked)
				{
					if (conversation->line_unit_index == NONE || conversation->line_unspatialized)
					{
						scripted_sound_new(conversation->line_sound_index, NONE, 1.f);
					}
					else
					{
						long sound_definition_index = conversation->line_sound_index;
						short vocalization_type = NONE;
						short play_type = unit_test_speech(
							conversation->line_unit_index,
							_unit_speech_scripted,
							FALSE,
							TRUE,
							NULL,
							&vocalization_type,
							&sound_definition_index);

						if (play_type == _unit_play_speech_queue)
						{
							blocked = TRUE;
						}
						else if (play_type > _unit_play_speech_none)
						{
							struct unit_speech_item speech;

							memset(&speech, 0, sizeof(speech));
							speech.priority = _unit_speech_scripted;
							speech.sound_definition_index = conversation->line_sound_index;
							speech.vocalization_type = NONE;
							speech.ai.communication_type = NONE;
							speech.ai.damage_category = NONE;
							speech.ai.dialogue_type_index = NONE;
							speech.ai.target_unit_index = conversation->line_address_unit_index;
							speech.ai.look_priority = _secondary_look_priority_default;
							speech.ai.look_type = _ai_information_look_unit;
							speech.ai.look_data.unit.unit_index = conversation->line_unit_index;
							speech.ai.information_type = _ai_information_none;

							if (ai_debug.print_conversations)
							{
								console_printf(FALSE, "%s: speak %s", conversation_definition->name, tag_get_name(speech.sound_definition_index));
							}

							unit_speak(conversation->line_unit_index, play_type, &speech);
						}
					}
				}
			}

			if (!blocked)
			{
				conversation->line_spoken = TRUE;
				conversation->any_line_spoken = TRUE;
			}
		}

		if (conversation->line_spoken)
		{
			if (!conversation->line_finished)
			{
				if (conversation->line_unit_index == NONE)
				{
					conversation->line_finished = conversation->line_sound_index == NONE || !scripted_sound_time(conversation->line_sound_index);
				}
				else
				{
					conversation->line_finished = unit_get(conversation->line_unit_index)->unit.speech.current.priority != _unit_speech_scripted;
				}
			}

			if (conversation->line_finished)
			{
				if (conversation->line_delay_timer > 0)
				{
					conversation->line_delay_timer--;
				}
				else
				{
					conversation->line_advance = TRUE;

					if (TEST_FLAG(conversation->line_flags, _ai_conversation_line_wait_after_until_told_to_advance_bit))
					{
						if (!conversation->waiting_to_advance)
						{
							conversation->waiting_to_advance = TRUE;
							conversation->told_to_advance = FALSE;
						}

						if (conversation->told_to_advance)
						{
							conversation->waiting_to_advance = FALSE;
						}
						else
						{
							conversation->line_advance = FALSE;
						}
					}
				}
			}
		}
	}

	return conversation->line_advance;
}

static void ai_conversation_line_end(
	long conversation_index)
{
	struct conversation_datum *conversation = conversation_get(conversation_index);
	struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);

	return;
}

/* ---------- private code */
