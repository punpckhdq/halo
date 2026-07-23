/*
UNITS.H
*/

#ifndef __UNITS_H
#define __UNITS_H
#pragma once

/* ---------- headers */

#include "ai/ai.h"
#include "objects/objects.h"

/* ---------- constants */

enum
{
	MAXIMUM_WEAPONS_PER_UNIT = 4,
	NUMBER_OF_UNIT_USER_ANIMATIONS = 2,
};

enum
{
	MAXIMUM_ATTACKERS_PER_UNIT = 4,
};

enum
{
	_unit_actively_controlled_bit = 0,
	_unit_changed_aiming_angles_bit,
	_unit_desired_weapon_invalid_bit,
	_unit_exact_facing_bit,
	_unit_active_camouflaged_bit,
	_unit_super_camouflaged_bit,
	_unit_controllable_bit,
	_unit_ignore_hard_pings_bit,
	_unit_must_set_up_dialogue_bit,
	_unit_placed_here_dead_bit,
	_unit_ignored_by_actors_bit,
	_unit_preferred_target_bit,
	_unit_no_falling_damage_bit,
	_unit_feign_death_allowed_bit,
	_unit_aim_without_turning_bit,
	_unit_attached_melee_attack_bit,
	_unit_not_enterable_by_player_bit,
	_unit_spawned_actors_bit,
	_unit_unloading_bit,
	_unit_integrated_light_on_bit,
	_unit_doesnt_drop_items_bit,
	_unit_has_already_been_hacky_hacky_hacked_bit,
	_unit_cannot_blink_bit,
	_unit_impervious_bit,
	_unit_suspended_bit,
	_unit_running_blindly_bit,
	_unit_integrated_night_vision_on_bit,
	_unit_possessed_by_recording_bit,
	_unit_desired_integrated_light_on_bit,
	_unit_desired_integrated_light_off_bit,
	NUMBER_OF_UNIT_DATUM_FLAGS
};

enum
{
	_unit_control_crouch_modifier_bit = 0,
	_unit_control_jump_bit,
	_unit_control_user_animation1_bit,
	_unit_control_user_animation2_bit,
	_unit_control_integrated_light_bit,
	_unit_control_exact_facing_bit,
	_unit_control_action_bit,
	_unit_control_use_equipment_bit,
	_unit_control_look_dont_turn_bit,
	_unit_control_force_alert_bit,
	_unit_control_weapon_reload_bit,
	_unit_control_weapon_primary_trigger_bit,
	_unit_control_weapon_secondary_trigger_bit,
	_unit_control_throw_grenade_bit,
	_unit_control_swap_weapons_bit,
	NUMBER_OF_UNIT_CONTROL_FLAGS,

	UNIT_CONTROL_DRIVER_MASK =
		FLAG(_unit_control_crouch_modifier_bit) |
		FLAG(_unit_control_jump_bit) |
		FLAG(_unit_control_user_animation1_bit) |
		FLAG(_unit_control_user_animation2_bit) |
		FLAG(_unit_control_integrated_light_bit) |
		FLAG(_unit_control_exact_facing_bit),

	UNIT_CONTROL_GUNNER_MASK =
		FLAG(_unit_control_weapon_reload_bit) |
		FLAG(_unit_control_weapon_primary_trigger_bit) |
		FLAG(_unit_control_weapon_secondary_trigger_bit) |
		FLAG(_unit_control_throw_grenade_bit) |
		FLAG(_unit_control_swap_weapons_bit),
};

enum
{
	_unit_state_idle = 0,
	_unit_state_gesture,
	_unit_state_turn_left,
	_unit_state_turn_right,
	_unit_state_move_front,
	_unit_state_move_back,
	_unit_state_move_left,
	_unit_state_move_right,
	_unit_state_stunned_move_front,
	_unit_state_stunned_move_back,
	_unit_state_stunned_move_left,
	_unit_state_stunned_move_right,
	_unit_state_slide_front,
	_unit_state_slide_back,
	_unit_state_slide_left,
	_unit_state_slide_right,
	_unit_state_flying_front,
	_unit_state_flying_back,
	_unit_state_flying_left,
	_unit_state_flying_right,
	_unit_state_airborne,
	_unit_state_land_soft,
	_unit_state_land_hard,
	_unit_state_hard_ping,
	_unit_state_dying_airborne,
	_unit_state_dying,
	_unit_state_entering_seat,
	_unit_state_exiting_seat,
	_unit_state_user_animation,
	_unit_state_ai_impulse,
	_unit_state_melee_attack,
	_unit_state_melee_airborne,
	_unit_state_melee_continuous,
	_unit_state_throw_grenade,
	_unit_state_resurrect_front,
	_unit_state_resurrect_back,
	_unit_state_feeding,
	_unit_state_opening,
	_unit_state_closing,
	_unit_state_leap_start,
	_unit_state_leap_airborne,
	_unit_state_leap_melee,
	_unit_state_shield_sapping,
	_unit_state_hovering,
	NUMBER_OF_UNIT_STATES,
};

enum
{
	_unit_animation_postpone_weapon_ik_until_interpolation_ends_bit = 0,
	_unit_animation_showing_acceleration_bit,
	_unit_animation_ignore_translation_bit,
	_unit_animation_fallen_on_front_bit,
	NUMBER_OF_UNIT_ANIMATION_FLAGS,
};


enum
{
	_unit_animation_state_asleep = 0,
	_unit_animation_state_alert,
	_unit_animation_state_suspicious,
	_unit_animation_state_in_combat,
	_unit_animation_state_wary,
	_unit_animation_state_flee,
	_unit_animation_state_flaming,
	NUMBER_OF_UNIT_ANIMATION_STATES,
};

enum
{
	_unit_seat_animation_airborne_dead = 0,
	_unit_seat_animation_landing_dead,
	_unit_seat_animation_acceleration_front_back,
	_unit_seat_animation_acceleration_left_right,
	_unit_seat_animation_acceleration_up_down,
	_unit_seat_animation_push_impact,
	_unit_seat_animation_twist_impact,
	_unit_seat_animation_seat_enter,
	_unit_seat_animation_seat_exit,
	_unit_seat_animation_looking,
	_unit_seat_animation_mouth_aperture,
	_unit_seat_animation_emotions,
	_unit_seat_animation_unused3,
	_unit_seat_animation_user0,
	_unit_seat_animation_user1,
	_unit_seat_animation_user2,
	_unit_seat_animation_user3,
	_unit_seat_animation_user4,
	_unit_seat_animation_user5,
	_unit_seat_animation_user6,
	_unit_seat_animation_user7,
	_unit_seat_animation_user8,
	_unit_seat_animation_user9,
	_unit_seat_animation_flying_front,
	_unit_seat_animation_flying_back,
	_unit_seat_animation_flying_left,
	_unit_seat_animation_flying_right,
	_unit_seat_animation_opening,
	_unit_seat_animation_closing,
	_unit_seat_animation_hovering,
	NUMBER_OF_UNIT_SEAT_ANIMATIONS,
};

enum
{
	_unit_weapon_class_animation_idle = 0,
	_unit_weapon_class_animation_gesture,
	_unit_weapon_class_animation_turning_left,
	_unit_weapon_class_animation_turning_right,
	_unit_weapon_class_animation_diving_front,
	_unit_weapon_class_animation_diving_back,
	_unit_weapon_class_animation_diving_left,
	_unit_weapon_class_animation_diving_right,
	_unit_weapon_class_animation_moving_front,
	_unit_weapon_class_animation_moving_back,
	_unit_weapon_class_animation_moving_left,
	_unit_weapon_class_animation_moving_right,
	_unit_weapon_class_animation_sliding_front,
	_unit_weapon_class_animation_sliding_back,
	_unit_weapon_class_animation_sliding_left,
	_unit_weapon_class_animation_sliding_right,
	_unit_weapon_class_animation_airborne,
	_unit_weapon_class_animation_land_soft,
	_unit_weapon_class_animation_land_hard,
	_unit_weapon_class_animation_land_mine,
	_unit_weapon_class_animation_throw_grenade,
	_unit_weapon_class_animation_disarm,
	_unit_weapon_class_animation_drop,
	_unit_weapon_class_animation_ready,
	_unit_weapon_class_animation_put_away,
	_unit_weapon_class_animation_aiming_still,
	_unit_weapon_class_animation_aiming_moving,
	_unit_weapon_class_animation_surprise_front,
	_unit_weapon_class_animation_surprise_back,
	_unit_weapon_class_animation_berserk,
	_unit_weapon_class_animation_evade_left,
	_unit_weapon_class_animation_evade_right,
	_unit_weapon_class_animation_signal_move,
	_unit_weapon_class_animation_signal_attack,
	_unit_weapon_class_animation_signal_warn,
	_unit_weapon_class_animation_moving_wounded_front,
	_unit_weapon_class_animation_moving_wounded_back,
	_unit_weapon_class_animation_moving_wounded_left,
	_unit_weapon_class_animation_moving_wounded_right,
	_unit_weapon_class_animation_melee_attack,
	_unit_weapon_class_animation_celebrate,
	_unit_weapon_class_animation_panic,
	_unit_weapon_class_animation_melee_airborne,
	_unit_weapon_class_animation_flaming,
	_unit_weapon_class_animation_resurrect_front,
	_unit_weapon_class_animation_resurrect_back,
	_unit_weapon_class_animation_melee_continuous,
	_unit_weapon_class_animation_feeding,
	_unit_weapon_class_animation_leap_start,
	_unit_weapon_class_animation_leap_airborne,
	_unit_weapon_class_animation_leap_melee,
	_unit_weapon_class_animation_zapping,
	_unit_weapon_class_animation_unused11,
	_unit_weapon_class_animation_unused12,
	_unit_weapon_class_animation_unused13,
	NUMBER_OF_UNIT_WEAPON_CLASS_ANIMATIONS,
};


enum
{
	_unit_grenade_human_fragmentation = 0,
	_unit_grenade_covenant_plasma,
	NUMBER_OF_UNIT_GRENADE_TYPES
};

enum
{
	cause_for_camo_regrowth_default = 0,
	cause_for_camo_regrowth_shot_fired,
};

enum
{
	_unit_aiming_speed_alert = 0,
	_unit_aiming_speed_casual,
	NUMBER_OF_UNIT_AIMING_SPEEDS,
};

enum
{
	_unit_grenade_throw_idle = 0,
	_unit_grenade_throw_wind_up,
	_unit_grenade_throw_in_hand,
	_unit_grenade_throw_ending,
	NUMBER_OF_UNIT_GRENADE_ATTACK_STATES,
};

enum
{
	_unit_base_seat_asleep = 0,
	_unit_base_seat_alert,
	_unit_base_seat_stand,
	_unit_base_seat_crouch,
	_unit_base_seat_flee,
	_unit_base_seat_flaming,
	NUMBER_OF_UNIT_BASE_SEATS,
};

enum
{
	_unit_base_weapon_none = 0,
	NUMBER_OF_UNIT_BASE_WEAPONS,
};

enum
{
	_unit_scream_falling = 0,
	_unit_scream_grenade_attached_to_us,
	_unit_scream_burning_to_death,
	_unit_scream_destroyed_limb,
	_unit_scream_destroyed_head,
	_unit_scream_resurrection,
	NUMBER_OF_UNIT_SCREAM_TYPES
};

enum
{
	_unit_estimate_none = 0,
	_unit_estimate_head_standing,
	_unit_estimate_head_crouching,
	_unit_estimate_gun_position,
	NUMBER_OF_UNIT_ESTIMATE_POSITION_MODES,
};

/* ---------- macros */

#define unit_get(index)			((struct unit_datum*)object_get_and_verify_type(index, _object_mask_unit))
#define unit_try_and_get(index)	((struct unit_datum*)object_try_and_get_and_verify_type(index, _object_mask_unit))

/* ---------- structures */

struct unit_animation
{
	word flags;
	short aiming_screen_index;
	short looking_screen_index;
	short last_ping_animation_index;
	char seat_index;
	char weapon_index;
	char weapon_type_index;
	char state;
	char action;
	char overlay_action;
	char desired_state;
	char base_seat_index;
	char emotion_index;
	struct animation_state action_animation;
	struct animation_state overlay_action_animation;
	struct animation_state soft_ping_animation;
	boolean aiming_with_euler_screen;
	boolean looking_with_euler_screen;
	real_rectangle2d aiming_screen_bounds;
	real_rectangle2d looking_screen_bounds;
	long external_animation_graph_index;
	struct animation_state external_animation;
};

struct unit_speech_item
{
	short priority;
	short vocalization_type;
	long sound_definition_index;
	short delay_time;
	short ai_notification_delay;
	short pause_time;
	word pad;
	struct ai_information_packet ai;
};

struct unit_speech
{
	struct unit_speech_item current;
	struct unit_speech_item queued;
	short damage_minor_decay_timer;
	short damage_minor_sounds;
	short damage_minor_timer;
	short damage_major_timer;
	long last_speech_finished_time;
	boolean played;
	boolean notified_ai;
	boolean finished;
	boolean pad;
	short pre_delay_timer;
	short sound_timer;
	short ai_delay_timer;
	short post_delay_timer;
	long impulse_sound_index;
};

struct unit_attacker
{
	unsigned long game_time_stamp;
	real damage_inflicted;
	long object_index;
	long player_index;
};

struct _unit_datum
{
	long actor_index;
	long swarm_actor_index;
	long swarm_next_unit_index;
	long swarm_prev_unit_index;
	unsigned long flags;
	unsigned long control_flags;
	short timer;
	char shield_sap_timeout;
	char magic_seat_index;
	long persistent_control_timer;
	unsigned long persistent_control_flags;
	long player_index;
	short last_unit_effect_type;
	short override_emotion_animation_index;
	long game_time_at_last_unit_effect;
	real_vector3d desired_facing_vector;
	real_vector3d desired_aiming_vector;
	real_vector3d aiming_vector;
	real_vector3d aiming_velocity;
	real_vector3d desired_looking_vector;
	real_vector3d looking_vector;
	real_vector3d looking_velocity;
	real_vector3d throttle;
	real primary_trigger;
	char aiming_speed;
	char melee_attack_state;
	char melee_continuous_damage_effect_timer;
	byte flaming_death_delay;
	char weapon_drop_delay_ticks;
	char grenade_throw_state;
	short grenade_throw_ticks;
	short grenade_throw_full_power_ticks;
	long grenade_object_index;
	struct unit_animation animation;
	real ambient_illumination;
	real self_illumination;
	real mouth_aperture;
	long last_entrance_attempt;
	short parent_seat_index;
	short current_weapon_index;
	short desired_weapon_index;
	long weapon_object_indices[MAXIMUM_WEAPONS_PER_UNIT];
	long weapon_last_used_at_game_time[MAXIMUM_WEAPONS_PER_UNIT];
	long equipment_object_index;
	char current_grenade_index;
	char desired_grenade_index;
	char grenade_counts[NUMBER_OF_UNIT_GRENADE_TYPES];
	char current_zoom_level;
	char desired_zoom_level;
	char gunner_inactive_ticks;
	byte aiming_change;
	long driver_object_index;
	long gunner_object_index;
	long last_vehicle_index;
	long game_time_at_last_vehicle_exit;
	short fake_encounter_index;
	short fake_squad_index;
	real seat_power[2];
	real integrated_light_power;
	real integrated_light_battery;
	real integrated_night_vision_power;
	real_point3d seat_last_position;
	real_vector3d seat_last_velocity;
	real_vector3d seat_acceleration;
	real_vector3d seat_desired_acceleration;
	real active_camouflage;
	real active_camouflage_super_amount;
	long dialogue_index;
	struct unit_speech speech;
	short last_damage_category;
	short delayed_damage_timer;
	real delayed_damage_peak;
	long delayed_damage_attacker_object_index;
	long flaming_death_attacker_object_index;
	real run_blindly_angle;
	real run_blindly_angle_delta;
	long time_of_death;
	short feign_death_timer;
	short cause_for_camo_regrowth;
	real body_stun;
	short body_stun_ticks;
	short killing_spree_count;
	long killing_spree_last_time;
	struct unit_attacker attackers[MAXIMUM_ATTACKERS_PER_UNIT];
	short user_animation_indices[NUMBER_OF_UNIT_USER_ANIMATIONS];
};

struct unit_datum
{
	long definition_index;
	struct _object_datum object;
	struct _unit_datum unit;
};

/* ---------- prototypes/UNITS.C */

void units_update(void);

boolean unit_update(long unit_index);

void unit_euler_aiming_update(
	real_matrix4x3 const *orientation,
	real_vector3d *aiming_vector,
	real_vector3d const *desired_aiming_vector,
	real_vector3d *aiming_velocity,
	real_rectangle2d const *aiming_bounds,
	real angular_velocity_limit,
	real angular_acceleration_limit);

void unit_unzoom(long unit_index);

void unit_destroy(long unit_index);
void unit_died(long unit_index, boolean feigned);
void unit_get_head_position(long unit_index, union real_point3d *head_position);
void unit_get_camera_position(long unit_index, real_point3d *camera_position);
void unit_estimate_position(
	long unit_index,
	short estimate_mode, 
	real_point3d const *body_position,
	real_vector3d *desired_facing,
	real_vector3d *desired_gun_offset,
	real_point3d *estimated_position);
void unit_get_center_of_mass(long unit_index, real_point3d *center_of_mass);
boolean unit_test_animation_impulse(long unit_index, short animation_impulse);
boolean unit_start_animation_impulse(long unit_index, short animation_impulse, real_vector2d *alignment_vector);
long unit_get_aiming_unit_index(long unit_index);
void unit_get_aiming_vector(long unit_index, real_vector3d *aiming_vector);
void unit_get_looking_vector(long unit_index, real_vector3d *looking_vector);
void unit_get_facing_vector(long unit_index, real_vector3d *facing_vector);
boolean unit_clip_to_aiming_bounds(long unit_index, real_vector3d *vector, boolean use_aiming_screen);
long unit_inventory_get_weapon(long unit_index, short index);
short unit_inventory_next_grenade(long unit_index, short current_index, short delta);

void unit_ready_desired_weapon(long unit_index, boolean immediate);

boolean unit_can_use_weapon(long unit_index, long weapon_index);

void unit_set_possessed(long unit_index, boolean possessed);
void unit_set_actively_controlled(long unit_index, boolean actively_controlled);
boolean unit_is_busy(long object_index);
void unit_scripting_set_emotion_animation(long unit_index, char const *animation_name);

boolean unit_drop_current_weapon(long unit_index, boolean immediate);

boolean unit_throw_grenade_begin(long unit_index, real_vector2d const *alignment_vector);


/* ---------- prototypes/UNIT_DIALOGUE.C */

short unit_test_speech(long unit_index, short priority, boolean allow_recursive_lookup, boolean allow_queue, long *unit_last_speech_time, short *vocalization_type_reference, long *sound_definition_index_reference);
void unit_speak(long unit_index, short play_type, struct unit_speech_item *speech_item);
void unit_notify_impulse_sound(long unit_index, long sound_definition_index, long impulse_sound_index);
boolean unit_is_speaking(long unit_index);
boolean unit_make_damage_sound(long unit_index, struct damage_data *damage_data, boolean died, boolean died_instantly, real body_damage, real shield_damage);
boolean unit_scream(long unit_index, short scream_type);

short unit_get_speech_priority_by_name(char const *name);

/* ---------- globals */

extern short magic_base_animation_seat_index;
extern boolean debug_objects_unit_mouth_apeture;
extern boolean debug_objects_unit_seats;
extern boolean debug_objects_unit_vectors;
extern boolean stun_enable;
extern boolean debug_damage_taken;
extern boolean debug_unit_illumination;
extern boolean debug_unit_animations;
extern boolean debug_unit_all_animations;

/* ---------- public code */

#endif // __UNITS_H
