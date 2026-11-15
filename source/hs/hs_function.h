/*
HS_FUNCTION.H

header included in hcex build.
*/

#ifndef __HS_FUNCTION_H
#define __HS_FUNCTION_H
#pragma once

/* ---------- macros */


#ifdef HS_COMPILER_ENABLED
#define HS_FUNCTION_DEFINITION_CREATE(global_name, return_type, name, parse, evaluate, documentation, parameters, formal_parameter_count) \
static struct hs_function_definition const (global_name) = \
{													\
	return_type,									\
	name,											\
	parse,											\
	evaluate,										\
	documentation,									\
	parameters										\
}
#else
#define HS_FUNCTION_DEFINITION_CREATE(global_name, return_type, name, parse, evaluate, documentation, parameters, formal_parameter_count) \
static struct hs_function_definition const (global_name) = \
{													\
	return_type,									\
	evaluate										\
}
#endif

/* ---------- globals */

HS_FUNCTION_DEFINITION_CREATE(begin_definition, _hs_passthrough, "begin", hs_parse_begin, hs_evaluate_begin, "returns the last expression in a sequence after evaluating the sequence in order.", "<expression(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(begin_random_definition, _hs_passthrough, "begin_random", hs_parse_begin, hs_evaluate_begin_random, "evaluates the sequence of expressions in random order and returns the last value evaluated.", "<expression(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(if_definition, _hs_passthrough, "if", hs_parse_if, hs_evaluate_if, "returns one of two values based on the value of a condition.", "<boolean> <then> [<else>]", 0);
HS_FUNCTION_DEFINITION_CREATE(cond_definition, _hs_passthrough, "cond", hs_parse_cond, NULL, "returns the value associated with the first true condition.", "(<boolean1> <result1>, [(<boolean2> <result2>, [...]]", 0);
HS_FUNCTION_DEFINITION_CREATE(set_definition, _hs_passthrough, "set", hs_parse_set, hs_evaluate_set, "set the value of a global variable.", "<variable name> <expression>", 0);
HS_FUNCTION_DEFINITION_CREATE(and_definition, _hs_type_boolean, "and", hs_parse_logical, hs_evaluate_logical, "returns true if all specified expressions are true.", "<boolean(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(or_definition, _hs_type_boolean, "or", hs_parse_logical, hs_evaluate_logical, "returns true if any specified expressions are true.", "<boolean(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(add_definition, _hs_type_real, "+", hs_parse_arithmetic, hs_evaluate_arithmetic, "returns the sum of all specified expressions.", "<number(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(subtract_definition, _hs_type_real, "-", hs_parse_arithmetic, hs_evaluate_arithmetic, "returns the difference of two expressions.", "<number> <number>", 0);
HS_FUNCTION_DEFINITION_CREATE(multiply_definition, _hs_type_real, "*", hs_parse_arithmetic, hs_evaluate_arithmetic, "returns the product of all specified expressions.", "<number(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(divide_definition, _hs_type_real, "/", hs_parse_arithmetic, hs_evaluate_arithmetic, "returns the quotient of two expressions.", "<number> <number>", 0);
HS_FUNCTION_DEFINITION_CREATE(min_definition, _hs_type_real, "min", hs_parse_arithmetic, hs_evaluate_arithmetic, "returns the minimum of all specified expressions.", "<number(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(max_definition, _hs_type_real, "max", hs_parse_arithmetic, hs_evaluate_arithmetic, "returns the maximum of all specified expressions.", "<number(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(equal_definition, _hs_type_boolean, "=", hs_parse_equality, hs_evaluate_equality, "returns true if two expressions are equal", "<expression> <expression>", 0);
HS_FUNCTION_DEFINITION_CREATE(not_equal_definition, _hs_type_boolean, "!=", hs_parse_equality, hs_evaluate_equality, "returns true if two expressions are not equal", "<expression> <expression>", 0);
HS_FUNCTION_DEFINITION_CREATE(gt_definition, _hs_type_boolean, ">", hs_parse_inequality, hs_evaluate_inequality, "returns true if the first number is larger than the second.", "<number> <number>", 0);
HS_FUNCTION_DEFINITION_CREATE(lt_definition, _hs_type_boolean, "<", hs_parse_inequality, hs_evaluate_inequality, "returns true if the first number is smaller than the second.", "<number> <number>", 0);
HS_FUNCTION_DEFINITION_CREATE(gte_definition, _hs_type_boolean, ">=", hs_parse_inequality, hs_evaluate_inequality, "returns true if the first number is larger than or equal to the second.", "<number> <number>", 0);
HS_FUNCTION_DEFINITION_CREATE(lte_definition, _hs_type_boolean, "<=", hs_parse_inequality, hs_evaluate_inequality, "returns true if the first number is smaller than or equal to the second.", "<number> <number>", 0);
HS_FUNCTION_DEFINITION_CREATE(sleep_definition, _hs_type_void, "sleep", hs_parse_sleep, hs_evaluate_sleep, "pauses execution of this script or, optionally, another script, for the specified number of ticks.", "<short> [<script>]", 0);
HS_FUNCTION_DEFINITION_CREATE(sleep_until_definition, _hs_type_void, "sleep_until", hs_parse_sleep_until, hs_evaluate_sleep_until, "pauses execution of this script until the specified condition is true, checking once per second unless a different number of ticks is specified.", "<boolean> [<short>]", 0);
HS_FUNCTION_DEFINITION_CREATE(wake_definition, _hs_type_void, "wake", hs_parse_wake, hs_evaluate_wake, "wakes a sleeping script in the next update.", "<script name>", 0);
HS_FUNCTION_DEFINITION_CREATE(inspect_definition, _hs_type_void, "inspect", hs_parse_inspect, hs_evaluate_inspect, "prints the value of an expression to the screen for debugging purposes.", "<expression>", 0);
HS_FUNCTION_DEFINITION_CREATE(object_to_unit_definition, _hs_type_unit, "unit", hs_parse_object_cast_up, hs_evaluate_object_cast_up, "converts an object to a unit.", "<object>", 0);
HS_FUNCTION_DEFINITION_CREATE(ai_debug_communication_suppress_definition, _hs_type_void, "ai_debug_communication_suppress", hs_parse_debug_string, hs_evaluate_debug_string, "suppresses (or stops suppressing, a set of AI communication types.", "<string(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(ai_debug_communication_ignore_definition, _hs_type_void, "ai_debug_communication_ignore", hs_parse_debug_string, hs_evaluate_debug_string, "ignores (or stops ignoring, a set of AI communication types when printing out communications.", "<string(s)>", 0);
HS_FUNCTION_DEFINITION_CREATE(ai_debug_communication_focus_definition, _hs_type_void, "ai_debug_communication_focus", hs_parse_debug_string, hs_evaluate_debug_string, "focuses (or stops focusing, a set of unit vocalization types.", "<string(s)>", 0);

HS_FUNCTION_1(_hs_type_boolean, not, hs_not, "returns the opposite of the expression.", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, print, hs_print, "prints a string to the console.", _hs_type_string);
HS_FUNCTION_0(_hs_type_object_list, players, hs_players, "returns a list of the players");
HS_FUNCTION_2(_hs_type_void, volume_teleport_players_not_inside, hs_teleport_players_not_in_trigger_volume, "moves all players outside a specified trigger volume to a specified flag.", _hs_type_trigger_volume, _hs_type_cutscene_flag);
HS_FUNCTION_2(_hs_type_boolean, volume_test_object, scenario_trigger_volume_test_object, "returns true if the specified object is within the specified volume.", _hs_type_trigger_volume, _hs_type_object);
HS_FUNCTION_2(_hs_type_boolean, volume_test_objects, hs_trigger_volume_test_objects_any, "returns true if any of the specified objects are within the specified volume.", _hs_type_trigger_volume, _hs_type_object_list);
HS_FUNCTION_2(_hs_type_boolean, volume_test_objects_all, hs_trigger_volume_test_objects_all, "returns true if any of the specified objects are within the specified volume.", _hs_type_trigger_volume, _hs_type_object_list);
HS_FUNCTION_1(_hs_type_void, object_create, hs_object_create, "creates an object from the scenario.", _hs_type_object_name);
HS_FUNCTION_1(_hs_type_void, object_destroy, hs_object_destroy, "destroys an object.", _hs_type_object);
HS_FUNCTION_1(_hs_type_void, object_create_anew, hs_object_create_anew, "creates an object, destroying it first if it already exists.", _hs_type_object_name);
HS_FUNCTION_1(_hs_type_void, object_create_containing, hs_object_create_containing, "creates all objects from the scenario whose names contain the given substring.", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, object_create_anew_containing, hs_object_create_anew_containing, "creates anew all objects from the scenario whose names contain the given substring.", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, object_destroy_containing, hs_object_destroy_containing, "destroys all objects from the scenario whose names contain the given substring.", _hs_type_string);
HS_FUNCTION_0(_hs_type_void, object_destroy_all, hs_object_destroy_all, "destroys all non player objects.");
HS_FUNCTION_2(_hs_type_void, object_teleport, hs_object_teleport, "moves the specified object to the specified flag.", _hs_type_object, _hs_type_cutscene_flag);
HS_FUNCTION_2(_hs_type_void, object_set_facing, hs_object_set_facing, "turns the specified object in the direction of the specified flag.", _hs_type_object, _hs_type_cutscene_flag);
HS_FUNCTION_2(_hs_type_void, object_set_shield, hs_object_set_shield, "sets the shield vitality of the specified object (between 0 and 1).", _hs_type_object, _hs_type_real);
HS_FUNCTION_3(_hs_type_void, object_set_permutation, hs_object_set_permutation, "sets the desired region (use \"\" for all regions) to the permutation with the given name, e.g. (object_set_permutation flood \"right arm\" ~damaged)", _hs_type_object, _hs_type_string, _hs_type_string);
HS_FUNCTION_2(_hs_type_object, list_get, hs_object_list_get_element, "returns an item in an object list.", _hs_type_object_list, _hs_type_short_integer);
HS_FUNCTION_1(_hs_type_short_integer, list_count, object_list_count, "returns the number of objects in a list", _hs_type_object_list);
HS_FUNCTION_2(_hs_type_void, effect_new, hs_effect_new, "starts the specified effect at the specified flag.", _hs_type_effect, _hs_type_cutscene_flag);
HS_FUNCTION_3(_hs_type_void, effect_new_on_object_marker, hs_effect_new_from_object_marker, "starts the specified effect on the specified object at the specified marker.", _hs_type_effect, _hs_type_object, _hs_type_string);
HS_FUNCTION_2(_hs_type_void, damage_new, hs_damage_new, "causes the specified damage at the specified flag.", _hs_type_damage, _hs_type_cutscene_flag);
HS_FUNCTION_2(_hs_type_void, damage_object, hs_damage_object, "causes the specified damage at the specified object.", _hs_type_damage, _hs_type_object);
HS_FUNCTION_3(_hs_type_boolean, objects_can_see_object, hs_objects_can_see_object, "returns true if any of the specified units are looking within the specified number of degrees of the object.", _hs_type_object_list, _hs_type_object, _hs_type_real);
HS_FUNCTION_3(_hs_type_boolean, objects_can_see_flag, hs_objects_can_see_flag, "returns true if any of the specified units are looking within the specified number of degrees of the flag.", _hs_type_object_list, _hs_type_cutscene_flag, _hs_type_real);
HS_FUNCTION_1(_hs_type_void, objects_delete_by_definition, hs_objects_delete_by_definition, "deletes all objects of type <definition>", _hs_type_object_definition);
HS_FUNCTION_2(_hs_type_void, sound_set_gain, hs_sound_set_gain, "absolutely do not use this", _hs_type_string, _hs_type_real);
HS_FUNCTION_1(_hs_type_real, sound_get_gain, hs_sound_get_gain, "absolutely do not use this either", _hs_type_string);

HS_FUNCTION_0(_hs_type_void, script_recompile, hs_recompile, "recompiles scripts.");
HS_FUNCTION_0(_hs_type_void, script_doc, hs_doc, "saves a file called hs_doc.txt with parameters for all script commands.");
HS_FUNCTION_1(_hs_type_void, help, hs_help, "prints a description of the named function.", _hs_type_string);

HS_FUNCTION_2(_hs_type_short_integer, random_range, random_range, "returns a random value in the range [lower bound, upper bound)", _hs_type_short_integer, _hs_type_short_integer);
HS_FUNCTION_2(_hs_type_real, real_random_range, real_random_range, "returns a random value in the range [lower bound, upper bound)", _hs_type_real, _hs_type_real);

HS_FUNCTION_2(_hs_type_void, numeric_countdown_timer_set, numeric_countdown_timer_set, "<milliseconds>, <auto_start>", _hs_type_long_integer, _hs_type_boolean);
HS_FUNCTION_1(_hs_type_short_integer, numeric_countdown_timer_get, numeric_countdown_timer_get, "<digit_index>", _hs_type_short_integer);
HS_FUNCTION_0(_hs_type_void, numeric_countdown_timer_stop, numeric_countdown_timer_stop, "");
HS_FUNCTION_0(_hs_type_void, numeric_countdown_timer_restart, numeric_countdown_timer_restart, "");

HS_FUNCTION_1(_hs_type_void, breakable_surfaces_enable, breakable_surfaces_enable, "enables or disables breakability of all breakable surfaces on level", _hs_type_boolean);

HS_FUNCTION_2(_hs_type_boolean, recording_play, recorded_animation_play, "make the specified unit run the specified cutscene recording.", _hs_type_unit, _hs_type_cutscene_recording);
HS_FUNCTION_2(_hs_type_boolean, recording_play_and_delete, recorded_animation_play_and_delete, "make the specified unit run the specified cutscene recording, deletes the unit when the animation finishes.", _hs_type_unit, _hs_type_cutscene_recording);
HS_FUNCTION_2(_hs_type_boolean, recording_play_and_hover, recorded_animation_play_and_hover, "make the specified vehicle run the specified cutscene recording, hovers the vehicle when the animation finishes.", _hs_type_vehicle, _hs_type_cutscene_recording);
HS_FUNCTION_1(_hs_type_void, recording_kill, recorded_animation_kill, "kill the specified unit's cutscene recording.", _hs_type_unit);
HS_FUNCTION_1(_hs_type_short_integer, recording_time, recorded_animation_get_time_left, "return the time remaining in the specified unit's cutscene recording.", _hs_type_unit);

HS_FUNCTION_2(_hs_type_void, object_set_ranged_attack_inhibited, object_set_ranged_attack_inhibited, "FALSE prevents object from using ranged attack", _hs_type_object, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, object_set_melee_attack_inhibited, object_set_melee_attack_inhibited, "FALSE prevents object from using melee attack", _hs_type_object, _hs_type_boolean);
HS_FUNCTION_0(_hs_type_void, objects_dump_memory, objects_dump_memory, "debugs object memory usage");
HS_FUNCTION_2(_hs_type_void, object_set_collideable, object_scripting_set_collideable, "FALSE prevents any object from colliding with the given object", _hs_type_object, _hs_type_boolean);
HS_FUNCTION_3(_hs_type_void, object_set_scale, objects_scripting_set_scale, "sets the scale for a given object and interpolates over the given number of frames to achieve that scale", _hs_type_object, _hs_type_real, _hs_type_short_integer);
HS_FUNCTION_4(_hs_type_void, objects_attach, objects_scripting_attach, "attaches the second object to the first; both strings can be empty", _hs_type_object, _hs_type_string, _hs_type_object, _hs_type_string);
HS_FUNCTION_2(_hs_type_void, objects_detach, objects_scripting_detach, "detaches from the given parent object the given child object", _hs_type_object, _hs_type_object);
HS_FUNCTION_0(_hs_type_void, garbage_collect_now, garbage_collect_now, "causes all garbage objects except those visible to a player to be collected immediately");
HS_FUNCTION_1(_hs_type_void, object_cannot_take_damage, object_cannot_take_damage, "prevents an object from taking damage", _hs_type_object_list);
HS_FUNCTION_1(_hs_type_void, object_can_take_damage, object_can_take_damage, "allows an object to take damage again", _hs_type_object_list);
HS_FUNCTION_2(_hs_type_void, object_beautify, object_beautify, "makes an object pretty for the remainder of the levels' cutscenes.", _hs_type_object, _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, objects_predict, hs_objects_predict, "loads textures necessary to draw a objects that are about to come on-screen.", _hs_type_object_list);
HS_FUNCTION_1(_hs_type_void, object_type_predict, object_definition_predict, "loads textures necessary to draw an object that's about to come on-screen.", _hs_type_object_definition);
HS_FUNCTION_1(_hs_type_void, object_pvs_set_object, object_pvs_set_object, "sets the specified object as the special place that activates everything it sees.", _hs_type_object);
HS_FUNCTION_1(_hs_type_void, object_pvs_set_camera, object_pvs_set_camera_point, "sets the specified cutscene camera point as the special place that activates everything it sees.", _hs_type_cutscene_camera_point);
HS_FUNCTION_0(_hs_type_void, object_pvs_clear, object_pvs_clear, "removes the special place that activates everything it sees.");
HS_FUNCTION_1(_hs_type_void, object_pvs_activate, object_pvs_activate, "just another (old) name for object_pvs_set_object.", _hs_type_object);

HS_FUNCTION_1(_hs_type_boolean, render_lights, lights_enable, "enables/disables dynamic lights", _hs_type_boolean);

HS_FUNCTION_1(_hs_type_short_integer, scenery_get_animation_time, scenery_get_animation_time, "returns the number of ticks remaining in a custom animation (or zero, if the animation is over).", _hs_type_scenery);
HS_FUNCTION_3(_hs_type_void, scenery_animation_start, scenery_animation_start, "starts a custom animation playing on a piece of scenery", _hs_type_scenery, _hs_type_animation_graph, _hs_type_string);
HS_FUNCTION_4(_hs_type_void, scenery_animation_start_at_frame, scenery_animation_start_at_frame, "starts a custom animation playing on a piece of scenery at a specific frame", _hs_type_scenery, _hs_type_animation_graph, _hs_type_string, _hs_type_short_integer);

HS_FUNCTION_1(_hs_type_void, render_effects, render_effects, "", _hs_type_boolean);

HS_FUNCTION_2(_hs_type_void, unit_can_blink, unit_scripting_can_blink, "allows a unit to blink or not (units never blink when they are dead)", _hs_type_unit, _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, unit_open, unit_open, "opens the hatches on the given unit", _hs_type_unit);
HS_FUNCTION_1(_hs_type_void, unit_close, unit_close, "closes the hatches on a given unit", _hs_type_unit);
HS_FUNCTION_1(_hs_type_void, unit_kill, unit_kill, "kills a given unit, no saving throw", _hs_type_unit);
HS_FUNCTION_1(_hs_type_void, unit_kill_silent, unit_kill_silent, "kills a given unit silently (doesn't make them play their normal death animation or sound)", _hs_type_unit);
HS_FUNCTION_1(_hs_type_short_integer, unit_get_custom_animation_time, unit_get_custom_animation_time, "returns the number of ticks remaining in a unit's custom animation (or zero, if the animation is over).", _hs_type_unit);
HS_FUNCTION_1(_hs_type_void, unit_stop_custom_animation, unit_stop_custom_animation, "stops the custom animation running on the given unit.", _hs_type_unit);
HS_FUNCTION_4(_hs_type_boolean, custom_animation, unit_start_user_animation, "starts a custom animation playing on a unit (interpolates into animation if last parameter is TRUE)", _hs_type_unit, _hs_type_animation_graph, _hs_type_string, _hs_type_boolean);
HS_FUNCTION_4(_hs_type_boolean, custom_animation_list, unit_scripting_start_user_animation_list, "starts a custom animation playing on a unit list (interpolates into animation if last parameter is TRUE)", _hs_type_object_list, _hs_type_animation_graph, _hs_type_string, _hs_type_boolean);

HS_FUNCTION_5(_hs_type_boolean, unit_custom_animation_at_frame, unit_custom_animation_at_frame, "starts a custom animation playing on a unit at a specific frame index(interpolates into animation if next to last parameter is TRUE)", _hs_type_unit, _hs_type_animation_graph, _hs_type_string, _hs_type_boolean, _hs_type_short_integer);
HS_FUNCTION_1(_hs_type_boolean, unit_is_playing_custom_animation, unit_is_playing_custom_animation, "returns TRUE if the given unit is still playing a custom animation", _hs_type_unit);
HS_FUNCTION_2(_hs_type_void, unit_aim_without_turning, unit_aim_without_turning, "allows a unit to aim in place without turning", _hs_type_unit, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, unit_set_emotion, unit_set_emotion, "sets a unit's facial expression (-1 is none, other values depend on unit)", _hs_type_unit, _hs_type_short_integer);
HS_FUNCTION_2(_hs_type_void, unit_set_enterable_by_player, unit_set_enterable_by_player, "can be used to prevent the player from entering a vehicle", _hs_type_unit, _hs_type_boolean);
HS_FUNCTION_3(_hs_type_void, unit_enter_vehicle, unit_scripting_enter_vehicle, "puts the specified unit in the specified vehicle (in the named seat)", _hs_type_unit, _hs_type_vehicle, _hs_type_string);
HS_FUNCTION_3(_hs_type_boolean, vehicle_test_seat_list, unit_scripting_vehicle_test_seat_list, "tests whether the named seat has an object in the object list", _hs_type_vehicle, _hs_type_string, _hs_type_object_list);
HS_FUNCTION_3(_hs_type_boolean, vehicle_test_seat, unit_scripting_vehicle_test_seat, "tests whether the named seat has a specified unit in it", _hs_type_vehicle, _hs_type_string, _hs_type_unit);
HS_FUNCTION_2(_hs_type_void, unit_set_emotion_animation, unit_scripting_set_emotion_animation, "sets the emotion animation to be used for the given unit", _hs_type_unit, _hs_type_string);
HS_FUNCTION_1(_hs_type_void, unit_exit_vehicle, unit_scripting_exit_vehicle, "makes a unit exit its vehicle", _hs_type_unit);
HS_FUNCTION_3(_hs_type_void, unit_set_maximum_vitality, unit_scripting_set_maximum_vitality, "sets a unit's maximum body and shield vitality", _hs_type_unit, _hs_type_real, _hs_type_real);
HS_FUNCTION_3(_hs_type_void, units_set_maximum_vitality, units_scripting_set_maximum_vitality, "sets a group of units' maximum body and shield vitality", _hs_type_object_list, _hs_type_real, _hs_type_real);
HS_FUNCTION_3(_hs_type_void, unit_set_current_vitality, unit_scripting_set_current_vitality, "sets a unit's current body and shield vitality", _hs_type_unit, _hs_type_real, _hs_type_real);
HS_FUNCTION_3(_hs_type_void, units_set_current_vitality, units_scripting_set_current_vitality, "sets a group of units' current body and shield vitality", _hs_type_object_list, _hs_type_real, _hs_type_real);
HS_FUNCTION_3(_hs_type_short_integer, vehicle_load_magic, vehicle_scripting_load_magic, "makes a list of units (named or by encounter) magically get into a vehicle, in the substring-specified seats (e.g. CD-passenger... empty string matches all seats)", _hs_type_unit, _hs_type_string, _hs_type_object_list);
HS_FUNCTION_2(_hs_type_short_integer, vehicle_unload, vehicle_scripting_unload, "makes units get out of a vehicle from the substring-specified seats (e.g. CD-passenger... empty string matches all seats)", _hs_type_unit, _hs_type_string);
HS_FUNCTION_1(_hs_type_void, magic_seat_name, scripting_set_magic_base_seat, "all units controlled by the player will assume the given seat name (valid values are 'asleep', 'alert', 'stand', 'crouch' and 'flee')", _hs_type_string);
HS_FUNCTION_2(_hs_type_void, unit_set_seat, unit_scripting_set_seat, "this unit will assume the named seat", _hs_type_unit, _hs_type_string);
HS_FUNCTION_0(_hs_type_void, magic_melee_attack, scripting_magic_melee_attack, "causes player's unit to start a melee attack");
HS_FUNCTION_1(_hs_type_object_list, vehicle_riders, unit_scripting_unit_riders, "returns a list of all riders in a vehicle", _hs_type_unit);
HS_FUNCTION_1(_hs_type_unit, vehicle_driver, unit_scripting_unit_driver, "returns the driver of a vehicle", _hs_type_unit);
HS_FUNCTION_1(_hs_type_unit, vehicle_gunner, unit_scripting_unit_gunner, "returns the gunner of a vehicle", _hs_type_unit);
HS_FUNCTION_1(_hs_type_real, unit_get_health, unit_scripting_get_health, "returns the health [0,1] of the unit, returns -1 if the unit does not exists", _hs_type_unit);
HS_FUNCTION_1(_hs_type_real, unit_get_shield, unit_scripting_get_shield, "returns the shield [0,1] of the unit, returns -1 if the unit does not exists", _hs_type_unit);
HS_FUNCTION_1(_hs_type_short_integer, unit_get_total_grenade_count, unit_scripting_get_grenade_count, "returns the total number of grenades for the given unit, 0 if it does not exist", _hs_type_unit);
HS_FUNCTION_2(_hs_type_boolean, unit_has_weapon, unit_scripting_has_weapon, "returns TRUE if the <unit> has <object> as a weapon, FALSE otherwise", _hs_type_unit, _hs_type_object_definition);
HS_FUNCTION_2(_hs_type_boolean, unit_has_weapon_readied, unit_scripting_has_weapon_readied, "returns TRUE if the <unit> has <object> as the primary weapon, FALSE otherwise", _hs_type_unit, _hs_type_object_definition);

HS_FUNCTION_1(_hs_type_void, unit_doesnt_drop_items, unit_scripting_doesnt_drop_items, "prevents any of the given units from dropping weapons or grenades when they die", _hs_type_object_list);
HS_FUNCTION_2(_hs_type_void, unit_impervious, unit_scripting_impervious, "prevents any of the given units from being knocked around or playing ping animations", _hs_type_object_list, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, unit_suspended, unit_scripting_suspended, "stops gravity from working on the given unit", _hs_type_unit, _hs_type_boolean);

HS_FUNCTION_0(_hs_type_boolean, unit_solo_player_integrated_night_vision_is_active, unit_solo_player_integrated_night_vision_is_active, "returns whether the night-vision mode could be activated via the flashlight button");

HS_FUNCTION_2(_hs_type_void, units_set_desired_flashlight_state, units_set_desired_flashlight_state, "sets the units' desired flashlight state", _hs_type_object_list, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, unit_set_desired_flashlight_state, unit_set_desired_flashlight_state, "sets the unit's desired flashlight state", _hs_type_unit, _hs_type_boolean);
HS_FUNCTION_1(_hs_type_boolean, unit_get_current_flashlight_state, unit_get_current_flashlight_state, "gets the unit's current flashlight state", _hs_type_unit);

HS_FUNCTION_2(_hs_type_void, device_set_never_appears_locked, device_set_never_appears_locked, "changes a machine's never_appears_locked flag, but only if paul is a bastard", _hs_type_device, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, device_set_power, device_set_power, "immediately sets the power of a named device to the given value", _hs_type_device, _hs_type_real);
HS_FUNCTION_1(_hs_type_real, device_get_power, device_get_power, "gets the current power of a named device", _hs_type_device);
HS_FUNCTION_2(_hs_type_boolean, device_set_position, device_set_desired_position, "set the desired position of the given device (used for devices without explicit device groups)", _hs_type_device, _hs_type_real);
HS_FUNCTION_1(_hs_type_real, device_get_position, device_get_position, "gets the current position of the given device (used for devices without explicit device groups)", _hs_type_device);
HS_FUNCTION_2(_hs_type_void, device_set_position_immediate, device_set_actual_position, "instantaneously changes the position of the given device (used for devices without explicit device groups", _hs_type_device, _hs_type_real);
HS_FUNCTION_1(_hs_type_real, device_group_get, device_group_get_value, "returns the desired value of the specified device group.", _hs_type_device_group);
HS_FUNCTION_2(_hs_type_boolean, device_group_set, device_group_set_desired_value, "changes the desired value of the specified device group.", _hs_type_device_group, _hs_type_real);
HS_FUNCTION_2(_hs_type_void, device_group_set_immediate, device_group_set_actual_value, "instantaneously changes the value of the specified device group.", _hs_type_device_group, _hs_type_real);
HS_FUNCTION_2(_hs_type_void, device_one_sided_set, device_one_sided_set, "TRUE makes the given device one-sided (only able to be opened from one direction), FALSE makes it two-sided", _hs_type_device, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, device_operates_automatically_set, device_operates_automatically_set, "TRUE makes the given device open automatically when any biped is nearby, FALSE makes it not", _hs_type_device, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, device_group_change_only_once_more_set, device_group_change_only_once_more_set, "TRUE allows a device to change states only once", _hs_type_device_group, _hs_type_boolean);

HS_FUNCTION_0(_hs_type_void, breakable_surfaces_reset, breakable_surfaces_reset, "restores all breakable surfaces");

HS_FUNCTION_0(_hs_type_void, cheat_all_powerups, cheat_all_powerups, "drops all powerups near player");
HS_FUNCTION_0(_hs_type_void, cheat_all_weapons, cheat_all_weapons, "drops all weapons near player");
HS_FUNCTION_0(_hs_type_void, cheat_all_vehicles, cheat_all_vehicles, "drops all vehicles on player");

HS_FUNCTION_0(_hs_type_void, cheat_teleport_to_camera, cheat_teleport_to_camera, "teleports player to camera location");
HS_FUNCTION_0(_hs_type_void, cheat_active_camouflage, cheat_active_camouflage, "gives the player active camouflage");
HS_FUNCTION_1(_hs_type_void, cheat_active_camouflage_local_player, cheat_active_camouflage_local_player, "gives the player active camouflage", _hs_type_short_integer);
HS_FUNCTION_0(_hs_type_void, cheats_load, cheats_load, "reloads the cheats.txt file");

HS_FUNCTION_1(_hs_type_void, ai, ai_globals_ai_active, "turns all AI on or off.", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, ai_dialogue_triggers, ai_globals_dialogue_triggers_enabled, "turns impromptu dialogue on or off.", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, ai_grenades, ai_globals_grenades_enabled, "turns grenade inventory on or off.", _hs_type_boolean);

HS_FUNCTION_1(_hs_type_void, ai_free, ai_scripting_free, "removes a group of actors from their encounter and sets them free", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_free_units, ai_scripting_free_units, "removes a set of units from their encounter (if any) and sets them free", _hs_type_object_list);
HS_FUNCTION_2(_hs_type_void, ai_attach, ai_scripting_attach_unit, "attaches the specified unit to the specified encounter.", _hs_type_unit, _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_attach_units, ai_scripting_attach_units, "attaches the specified list of units to the specified encounter.", _hs_type_object_list, _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_attach_free, ai_scripting_attach_free, "attaches a unit to a newly created free actor of the specified type", _hs_type_unit, _hs_type_actor_variant);
HS_FUNCTION_1(_hs_type_void, ai_detach, ai_scripting_detach_unit, "detaches the specified unit from all AI.", _hs_type_unit);
HS_FUNCTION_1(_hs_type_void, ai_detach_units, ai_scripting_detach_units, "detaches the specified list of units from all AI.", _hs_type_object_list);
HS_FUNCTION_1(_hs_type_void, ai_place, ai_scripting_place, "places the specified encounter on the map.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_kill, ai_scripting_kill, "instantly kills the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_kill_silent, ai_scripting_kill_silent, "instantly and silently (no animation or sound played) kills the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_erase, ai_scripting_erase, "erases the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_0(_hs_type_void, ai_erase_all, ai_scripting_erase_all, "erases all AI.");
HS_FUNCTION_1(_hs_type_void, ai_select, ai_scripting_select, "selects the specified encounter.", _hs_type_ai);
HS_FUNCTION_0(_hs_type_void, ai_deselect, ai_scripting_deselect, "clears the selected encounter.");
HS_FUNCTION_1(_hs_type_void, ai_spawn_actor, ai_scripting_spawn_actor, "spawns a single actor in the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_set_respawn, ai_scripting_set_respawn, "enables or disables respawning in the specified encounter.", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_set_deaf, ai_scripting_set_deaf, "enables or disables hearing for actors in the specified encounter.", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_set_blind, ai_scripting_set_blind, "enables or disables sight for actors in the specified encounter.", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_magically_see_encounter, ai_scripting_magically_see_encounter, "makes one encounter magically aware of another.", _hs_type_ai, _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_magically_see_players, ai_scripting_magically_see_players, "makes an encounter magically aware of nearby players.", _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_magically_see_unit, ai_scripting_magically_see_unit, "makes an encounter magically aware of the specified unit.", _hs_type_ai, _hs_type_unit);
HS_FUNCTION_2(_hs_type_void, ai_magically_see_units, ai_scripting_magically_see_units, "makes an encounter magically aware of the specified set of units.", _hs_type_ai, _hs_type_object_list);
HS_FUNCTION_1(_hs_type_void, ai_timer_start, ai_scripting_timer_start, "makes a squad's delay timer start counting.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_timer_expire, ai_scripting_timer_expire, "makes a squad's delay timer expire and releases them to enter combat.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_attack, ai_scripting_attack, "makes the specified platoon(s) go into the attacking state.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_defend, ai_scripting_defend, "makes the specified platoon(s) go into the defending state.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_retreat, ai_scripting_retreat, "makes all squads in the specified platoon(s) maneuver to their designated maneuver squads.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_maneuver, ai_scripting_maneuver, "makes all squads in the specified platoon(s) maneuver to their designated maneuver squads.", _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_maneuver_enable, ai_scripting_maneuver_enable, "enables or disables the maneuver/retreat rule for an encounter or platoon. the rule will still trigger, but none of the actors will be given the order to change squads.", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_migrate, ai_scripting_migrate, "makes all or part of an encounter move to another encounter.", _hs_type_ai, _hs_type_ai);
HS_FUNCTION_3(_hs_type_void, ai_migrate_and_speak, ai_scripting_migrate_and_speak, "makes all or part of an encounter move to another encounter, and say their 'advance' or 'retreat' speech lines.", _hs_type_ai, _hs_type_ai, _hs_type_string);
HS_FUNCTION_2(_hs_type_void, ai_migrate_by_unit, ai_scripting_migrate_by_unit, "makes a named vehicle or group of units move to another encounter.", _hs_type_object_list, _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_allegiance, ai_scripting_allegiance, "creates an allegiance between two teams.", _hs_type_enum_team, _hs_type_enum_team);
HS_FUNCTION_2(_hs_type_void, ai_allegiance_remove, ai_scripting_allegiance_remove, "destroys an allegiance between two teams.", _hs_type_enum_team, _hs_type_enum_team);
HS_FUNCTION_3(_hs_type_void, ai_go_to_vehicle, ai_scripting_go_to_vehicle, "tells a group of actors to get into a vehicle, in the substring-specified seats (e.g. passenger for pelican)... does not interrupt any actors who are already going to vehicles", _hs_type_ai, _hs_type_unit, _hs_type_string);
HS_FUNCTION_3(_hs_type_void, ai_go_to_vehicle_override, ai_scripting_go_to_vehicle_override, "tells a group of actors to get into a vehicle, in the substring-specified seats (e.g. passenger for pelican)... NB: any actors who are already going to vehicles will stop and go to this one instead!", _hs_type_ai, _hs_type_unit, _hs_type_string);
HS_FUNCTION_1(_hs_type_void, ai_exit_vehicle, ai_scripting_exit_vehicle, "tells a group of actors to get out of any vehicles that they are in", _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_braindead, ai_scripting_braindead, "makes a group of actors braindead, or restores them to life (in their initial state)", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_braindead_by_unit, ai_scripting_braindead_by_unit, "makes a list of objects braindead, or restores them to life. if you pass in a vehicle index, it makes all actors in that vehicle braindead (including any built-in guns)", _hs_type_object_list, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_disregard, ai_scripting_ignore, "if TRUE, forces all actors to completely disregard the specified units, otherwise lets them acknowledge the units again", _hs_type_object_list, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_prefer_target, ai_scripting_prefer_target, "if TRUE, *ALL* enemies will prefer to attack the specified units. if FALSE, removes the preference.", _hs_type_object_list, _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, ai_teleport_to_starting_location, ai_scripting_teleport_starting_location, "teleports a group of actors to the starting locations of their current squad(s)", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_teleport_to_starting_location_if_unsupported, ai_scripting_teleport_starting_location_if_unsupported, "teleports a group of actors to the starting locations of their current squad(s), only if they are not supported by solid ground (i.e. if they are falling after switching BSPs)", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_renew, ai_scripting_renew, "refreshes the health and grenade count of a group of actors, so they are as good as new", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_try_to_fight_nothing, ai_scripting_try_to_fight_nothing, "removes the preferential target setting from a group of actors", _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_try_to_fight, ai_scripting_try_to_fight, "causes a group of actors to preferentially target another group of actors", _hs_type_ai, _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_try_to_fight_player, ai_scripting_try_to_fight_player, "causes a group of actors to preferentially target the player", _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_command_list, ai_scripting_command_list, "tells a group of actors to begin executing the specified command list", _hs_type_ai, _hs_type_ai_command_list);
HS_FUNCTION_2(_hs_type_void, ai_command_list_by_unit, ai_scripting_command_list_by_unit, "tells a named unit to begin executing the specified command list", _hs_type_unit, _hs_type_ai_command_list);
HS_FUNCTION_1(_hs_type_void, ai_command_list_advance, ai_scripting_command_list_advance, "tells a group of actors that are running a command list that they may advance further along the list (if they are waiting for a stimulus)", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_command_list_advance_by_unit, ai_scripting_command_list_advance_by_unit, "just like ai_command_list_advance but operates upon a unit instead", _hs_type_unit);
HS_FUNCTION_2(_hs_type_void, ai_force_active, ai_scripting_force_active, "forces an encounter to remain active (i.e. not freeze in place) even if there are no players nearby", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_force_active_by_unit, ai_scripting_force_active_by_unit, "forces a named actor that is NOT in an encounter to remain active (i.e. not freeze in place) even if there are no players nearby", _hs_type_unit, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_set_return_state, ai_scripting_set_return_state, "sets the state that a group of actors will return to when they have nothing to do", _hs_type_ai, _hs_type_enum_ai_default_state);
HS_FUNCTION_2(_hs_type_void, ai_set_current_state, ai_scripting_set_current_state, "sets the current state of a group of actors. WARNING: may have unpredictable results on actors that are in combat", _hs_type_ai, _hs_type_enum_ai_default_state);
HS_FUNCTION_2(_hs_type_void, ai_playfight, ai_scripting_playfight, "sets an encounter to be playfighting or not", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_0(_hs_type_void, ai_reconnect, ai_scripting_reconnect, "reconnects all AI information to the current structure bsp (use this after you create encounters or command lists in sapien, or place new firing points or command list points)");
HS_FUNCTION_2(_hs_type_void, ai_vehicle_encounter, ai_scripting_vehicle_encounter, "sets a vehicle to 'belong' to a particular encounter/squad. any actors who get into the vehicle will be placed in this squad. NB: vehicles potentially drivable by multiple teams need their own encounter!", _hs_type_unit, _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_vehicle_enterable_distance, ai_scripting_vehicle_enterable_distance, "sets a vehicle as being impulsively enterable for actors within a certain distance", _hs_type_unit, _hs_type_real);
HS_FUNCTION_2(_hs_type_void, ai_vehicle_enterable_team, ai_scripting_vehicle_enterable_team, "sets a vehicle as being impulsively enterable for actors on a certain team", _hs_type_unit, _hs_type_enum_team);
HS_FUNCTION_2(_hs_type_void, ai_vehicle_enterable_actor_type, ai_scripting_vehicle_enterable_actor_type, "sets a vehicle as being impulsively enterable for actors of a certain type (grunt, elite, marine etc)", _hs_type_unit, _hs_type_enum_actor_type);
HS_FUNCTION_2(_hs_type_void, ai_vehicle_enterable_actors, ai_scripting_vehicle_enterable_actors, "sets a vehicle as being impulsively enterable for a certain encounter/squad of actors", _hs_type_unit, _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_vehicle_enterable_disable, ai_scripting_vehicle_enterable_disable, "disables actors from impulsively getting into a vehicle (this is the default state for newly placed vehicles)", _hs_type_unit);
HS_FUNCTION_2(_hs_type_void, ai_look_at_object, ai_scripting_look_at_object, "tells an actor to look at an object until further notice", _hs_type_unit, _hs_type_object);
HS_FUNCTION_1(_hs_type_void, ai_stop_looking, ai_scripting_stop_looking, "tells an actor to stop looking at whatever it's looking at", _hs_type_unit);
HS_FUNCTION_2(_hs_type_void, ai_automatic_migration_target, ai_scripting_automatic_migration_target, "enables or disables a squad as being an automatic migration target", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, ai_follow_target_disable, ai_scripting_follow_target_disable, "turns off following for an encounter", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_follow_target_players, ai_scripting_follow_target_players, "sets the follow target for an encounter to be the closest player", _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_follow_target_unit, ai_scripting_follow_target_unit, "sets the follow target for an encounter to be a specific unit", _hs_type_ai, _hs_type_unit);
HS_FUNCTION_2(_hs_type_void, ai_follow_target_ai, ai_scripting_follow_target_ai, "sets the follow target for an encounter to be a group of AI (encounter, squad or platoon)", _hs_type_ai, _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_follow_distance, ai_scripting_follow_distance, "sets the distance threshold which will cause squads to migrate when following someone", _hs_type_ai, _hs_type_real);
HS_FUNCTION_1(_hs_type_void, ai_conversation_stop, ai_scripting_conversation_stop, "stops a conversation from playing or trying to play", _hs_type_conversation);
HS_FUNCTION_1(_hs_type_void, ai_conversation_advance, ai_scripting_conversation_advance, "tells a conversation that it may advance", _hs_type_conversation);
HS_FUNCTION_2(_hs_type_void, ai_link_activation, ai_scripting_link_activation, "links the first encounter so that it will be made active whenever it detects that the second encounter is active", _hs_type_ai, _hs_type_ai);
HS_FUNCTION_2(_hs_type_void, ai_berserk, ai_scripting_berserk, "forces a group of actors to start or stop berserking", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_set_team, ai_scripting_set_team, "makes an encounter change to a new team", _hs_type_ai, _hs_type_enum_team);
HS_FUNCTION_2(_hs_type_void, ai_allow_charge, ai_scripting_allow_charge, "either enables or disables charging behavior for a group of actors", _hs_type_ai, _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, ai_allow_dormant, ai_scripting_allow_dormant, "either enables or disables automatic dormancy for a group of actors", _hs_type_ai, _hs_type_boolean);

HS_FUNCTION_1(_hs_type_boolean, ai_is_attacking, ai_scripting_is_attacking, "returns whether a platoon is in the attacking mode (or if an encounter is specified, returns whether any platoon in that encounter is attacking)", _hs_type_ai);
HS_FUNCTION_1(_hs_type_short_integer, ai_command_list_status, ai_scripting_command_list_status, "gets the status of a number of units running command lists: 0 = none, 1 = finished command list, 2 = waiting for stimulus, 3 = running command list", _hs_type_object_list);
HS_FUNCTION_1(_hs_type_short_integer, ai_going_to_vehicle, ai_scripting_going_to_vehicle, "return the number of actors that are still trying to get into the specified vehicle", _hs_type_unit);
HS_FUNCTION_1(_hs_type_short_integer, ai_living_count, ai_scripting_living_count, "return the number of living actors in the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_real, ai_living_fraction, ai_scripting_living_fraction, "return the fraction [0-1] of living actors in the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_real, ai_strength, ai_scripting_strength, "return the current strength (average body vitality from 0-1) of the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_short_integer, ai_swarm_count, ai_scripting_swarm_count, "return the number of swarm actors in the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_short_integer, ai_nonswarm_count, ai_scripting_nonswarm_count, "return the number of non-swarm actors in the specified encounter and/or squad.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_object_list, ai_actors, object_list_from_ai_reference, "converts an ai reference to an object list.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_short_integer, ai_status, ai_scripting_status, "returns the most severe combat status of a group of actors (0=inactive, 1=noncombat, 2=guarding, 3=search/suspicious, 4=definite enemy(heard or magic awareness), 5=visible enemy, 6=engaging in combat.", _hs_type_ai);
HS_FUNCTION_1(_hs_type_boolean, ai_conversation, ai_scripting_conversation, "tries to add an entry to the list of conversations waiting to play. returns FALSE if the required units could not be found to play the conversation, or if the player is too far away and the 'delay' flag is not set.", _hs_type_conversation);
HS_FUNCTION_1(_hs_type_short_integer, ai_conversation_line, ai_scripting_conversation_line, "returns which line the conversation is currently playing, or 999 if the conversation is not currently playing", _hs_type_conversation);
HS_FUNCTION_1(_hs_type_short_integer, ai_conversation_status, ai_scripting_conversation_status, "returns the status of a conversation (0=none, 1=trying to begin, 2=waiting for guys to get in position, 3=playing, 4=waiting to advance, 5=could not begin, 6=finished successfully, 7=aborted midway", _hs_type_conversation);
HS_FUNCTION_2(_hs_type_boolean, ai_allegiance_broken, ai_scripting_allegiance_broken, "returns whether two teams have an allegiance that is currently broken by traitorous behavior", _hs_type_enum_team, _hs_type_enum_team);

HS_FUNCTION_1(_hs_type_void, camera_control, director_script_camera, "toggles script control of the camera.", _hs_type_boolean);
HS_FUNCTION_2(_hs_type_void, camera_set, scripted_camera_set_absolute, "moves the camera to the specified camera point over the specified number of ticks.", _hs_type_cutscene_camera_point, _hs_type_short_integer);
HS_FUNCTION_3(_hs_type_void, camera_set_relative, scripted_camera_set, "moves the camera to the specified camera point over the specified number of ticks (position is relative to the specified object).", _hs_type_cutscene_camera_point, _hs_type_short_integer, _hs_type_object);
HS_FUNCTION_2(_hs_type_void, camera_set_animation, scripted_camera_set_animation, "begins a prerecorded camera animation.", _hs_type_animation_graph, _hs_type_string);
HS_FUNCTION_1(_hs_type_void, camera_set_first_person, scripted_camera_set_first_person, "makes the scripted camera follow a unit.", _hs_type_unit);
HS_FUNCTION_1(_hs_type_void, camera_set_dead, scripted_camera_set_dead, "makes the scripted camera zoom out around a unit as if it were dead.", _hs_type_unit);
HS_FUNCTION_0(_hs_type_short_integer, camera_time, scripted_camera_time, "returns the number of ticks remaining in the current camera interpolation.");
HS_FUNCTION_0(_hs_type_void, debug_camera_save, director_save_camera, "saves the camera position and facing.");
HS_FUNCTION_0(_hs_type_void, debug_camera_load, director_load_camera, "loads the saved camera position and facing.");

HS_FUNCTION_1(_hs_type_void, game_speed, game_time_set_speed, "changes the game speed.", _hs_type_real);
HS_FUNCTION_1(_hs_type_void, game_variant, game_set_game_variant_from_name, "set the game engine", _hs_type_string);
HS_FUNCTION_0(_hs_type_long_integer, game_time, game_time_get, "gets ticks elapsed since the start of the game.");
HS_FUNCTION_0(_hs_type_enum_game_difficulty, game_difficulty_get, game_difficulty_level_get_ignore_easy, "returns the current difficulty setting, but lies to you and will never return easy, instead returning normal");
HS_FUNCTION_0(_hs_type_enum_game_difficulty, game_difficulty_get_real, game_difficulty_level_get, "returns the actual current difficulty setting without lying");

HS_FUNCTION_0(_hs_type_void, players_unzoom_all, players_unzoom_all, "resets zoom levels on all players");
HS_FUNCTION_1(_hs_type_void, player_enable_input, player_input_enable, "toggle player input. the player can still free-look, but nothing else.", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_boolean, player_camera_control, scripted_player_control_set_camera_control, "enables/disables camera control globally", _hs_type_boolean);
HS_FUNCTION_0(_hs_type_void, player_action_test_reset, player_control_action_test_reset, "resets the player action test state so that all tests will return false.");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_jump, player_control_action_test_jump, "returns true if any player has jumped since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_primary_trigger, player_control_action_test_primary_trigger, "returns true if any player has used primary trigger since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_grenade_trigger, player_control_action_test_grenade_trigger, "returns true if any player has used grenade trigger since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_zoom, player_control_action_test_zoom, "returns true if any player has hit the zoom button since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_action, player_control_action_test_action, "returns true if any player has hit the action key since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_accept, player_control_action_test_accept, "returns true if any player has hit accept since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_back, player_control_action_test_back, "returns true if any player has hit the back key since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_look_relative_up, player_control_action_test_look_relative_up, "returns true if any player has looked up since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_look_relative_down, player_control_action_test_look_relative_down, "returns true if any player has looked down since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_look_relative_left, player_control_action_test_look_relative_left, "returns true if any player has looked left since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_look_relative_right, player_control_action_test_look_relative_right, "returns true if any player has looked right since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_look_relative_all_directions, player_control_action_test_look_relative_all_directions, "returns true if any player has looked up, down, left, and right since the last call to (player_action_test_reset).");
HS_FUNCTION_0(_hs_type_boolean, player_action_test_move_relative_all_directions, player_control_action_test_move_relative_all_directions, "returns true if any player has moved forward, backward, left, and right since the last call to (player_action_test_reset).");
HS_FUNCTION_3(_hs_type_void, player_add_equipment, player_add_equipment, "adds/resets the player's health, shield, and inventory (weapons and grenades) to the named profile. resets if third parameter is true, adds if false.", _hs_type_unit, _hs_type_starting_profile, _hs_type_boolean);

HS_FUNCTION_2(_hs_type_void, debug_teleport_player, debug_player_teleport, "", _hs_type_short_integer, _hs_type_short_integer);

HS_FUNCTION_0(_hs_type_void, map_reset, main_reset_map, "starts the map from the beginning.");
HS_FUNCTION_1(_hs_type_void, map_name, main_set_map_name, "changes the name of the solo player map.", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, multiplayer_map_name, main_set_multiplayer_map_name, "changes the name of the multiplayer map", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, game_difficulty_set, main_set_difficulty, "changes the difficulty setting for the next map to be loaded.", _hs_type_enum_game_difficulty);
HS_FUNCTION_1(_hs_type_void, crash, main_crash, "crashes (for debugging).", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, switch_bsp, scenario_switch_structure_bsp, "takes off your condom and changes to a different structure bsp", _hs_type_short_integer);
HS_FUNCTION_0(_hs_type_short_integer, structure_bsp_index, global_structure_bsp_index_get, "returns the current structure bsp index");
HS_FUNCTION_0(_hs_type_void, version, main_print_version, "prints the build version.");
HS_FUNCTION_0(_hs_type_void, playback, main_set_game_connection_to_film_playback, "starts game in film playback mode");

HS_FUNCTION_0(_hs_type_void, texture_cache_flush, texture_cache_flush, "don't make me kick your ass");

HS_FUNCTION_0(_hs_type_void, sound_cache_flush, sound_cache_flush, "i'm a rebel!");

HS_FUNCTION_0(_hs_type_void, debug_memory, debug_dump_memory, "dumps memory leaks.");
HS_FUNCTION_0(_hs_type_void, debug_memory_by_file, debug_dump_memory_by_file, "dumps memory leaks by source file.");
HS_FUNCTION_1(_hs_type_void, debug_memory_for_file, debug_dump_memory_for_file, "dumps memory leaks from the specified source file.", _hs_type_string);
HS_FUNCTION_0(_hs_type_void, debug_tags, tag_groups_dump_memory, "writes all memory being used by tag files into tag_dump.txt");

HS_FUNCTION_0(_hs_type_void, profile_reset, profile_initialize, "resets profiling data.");
HS_FUNCTION_1(_hs_type_void, profile_dump, profile_dump_to_file, "dumps profile based on a substring.", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, profile_activate, profile_sections_activate, "activates profile sections based on a substring.", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, profile_deactivate, profile_sections_deactivate, "deactivates profile sections based on a substring.", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, profile_graph_toggle, profile_graph_toggle, "enables or disables profile graph display of a particular value.", _hs_type_string);

HS_FUNCTION_1(_hs_type_void, debug_pvs, debug_pvs, "displays the current pvs.", _hs_type_boolean);

HS_FUNCTION_0(_hs_type_void, radiosity_start, radiosity_hack_start, "starts radiosity computation.");
HS_FUNCTION_0(_hs_type_void, radiosity_save, radiosity_hack_save, "saves radiosity solution.");
HS_FUNCTION_0(_hs_type_void, radiosity_debug_point, radiosity_hack_find_point, "tests sun occlusion at a point.");

HS_FUNCTION_0(_hs_type_void, ai_lines, ai_profile_change_render_spray, "cycles through AI line-spray modes");

HS_FUNCTION_0(_hs_type_void, ai_debug_sound_point_set, ai_debug_sound_point_set, "drops the AI debugging sound point at the camera location");
HS_FUNCTION_2(_hs_type_void, ai_debug_vocalize, ai_debug_vocalize, "makes the selected AI vocalize", _hs_type_string, _hs_type_string);
HS_FUNCTION_1(_hs_type_void, ai_debug_teleport_to, ai_debug_teleport_to, "teleports all players to the specified encounter", _hs_type_ai);
HS_FUNCTION_1(_hs_type_void, ai_debug_speak, ai_debug_speak, "makes the currently selected AI speak a vocalization (e.g. ai_speak \"pain minor\")", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, ai_debug_speak_list, ai_debug_speak_list, "makes the currently selected AI speak a list of vocalizations (e.g. ai_speak_list \"involuntary\")", _hs_type_string);

HS_FUNCTION_4(_hs_type_void, fade_in, player_effect_screen_fade_in, "does a screen fade in from a particular color", _hs_type_real, _hs_type_real, _hs_type_real, _hs_type_short_integer);
HS_FUNCTION_4(_hs_type_void, fade_out, player_effect_screen_fade_out, "does a screen fade out to a particular color", _hs_type_real, _hs_type_real, _hs_type_real, _hs_type_short_integer);

HS_FUNCTION_0(_hs_type_void, cinematic_start, cinematic_start, "initializes game to start a cinematic (interruptive) cutscene");
HS_FUNCTION_0(_hs_type_void, cinematic_stop, cinematic_stop, "initializes the game to end a cinematic (interruptive) cutscene");
HS_FUNCTION_0(_hs_type_void, cinematic_skip_start_internal, cinematic_skip_start, "");
HS_FUNCTION_0(_hs_type_void, cinematic_skip_stop_internal, cinematic_skip_stop, "");
HS_FUNCTION_1(_hs_type_void, cinematic_show_letterbox, cinematic_show_letterbox, "sets or removes the letterbox bars", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, cinematic_set_title, cinematic_set_title, "activates the chapter title", _hs_type_cutscene_title);
HS_FUNCTION_2(_hs_type_void, cinematic_set_title_delayed, cinematic_set_title_delayed, "activates the chapter title, delayed by <real> seconds", _hs_type_cutscene_title, _hs_type_real);
HS_FUNCTION_1(_hs_type_void, cinematic_suppress_bsp_object_creation, cinematic_suppress_bsp_object_creation, "suppresses or enables the automatic creation of objects during cutscenes due to a bsp switch", _hs_type_boolean);

HS_FUNCTION_0(_hs_type_void, attract_mode_start, attract_mode_start, "");

HS_FUNCTION_0(_hs_type_void, game_won, main_won_map, "causes the player to successfully finish the current level and move to the next");
HS_FUNCTION_0(_hs_type_void, game_lost, main_lost_map, "causes the player to revert to his previous saved game");
HS_FUNCTION_0(_hs_type_boolean, game_safe_to_save, game_safe_to_save, "returns FALSE if it would be a bad idea to save the player's game right now");
HS_FUNCTION_0(_hs_type_boolean, game_all_quiet, game_all_quiet, "returns FALSE if there are bad guys around, projectiles in the air, etc.");
HS_FUNCTION_0(_hs_type_boolean, game_safe_to_speak, game_safe_to_speak, "returns FALSE if it would be a bad idea to save the player's game right now");
HS_FUNCTION_0(_hs_type_boolean, game_is_cooperative, game_is_cooperative, "returns TRUE if the game is cooperative");
HS_FUNCTION_0(_hs_type_void, game_save, main_save_map_safe, "checks to see if it is safe to save game, then saves (gives up after 8 seconds)");
HS_FUNCTION_0(_hs_type_void, game_save_cancel, main_save_cancel, "cancels any pending game_save, timeout or not");
HS_FUNCTION_0(_hs_type_void, game_save_no_timeout, main_save_map_no_timeout, "checks to see if it is safe to save game, then saves (this version never gives up)");
HS_FUNCTION_0(_hs_type_void, game_save_totally_unsafe, main_save_map_nonsafe, "disregards player's current situation");
HS_FUNCTION_0(_hs_type_boolean, game_saving, main_saving_map, "checks to see if the game is trying to save the map.");
HS_FUNCTION_0(_hs_type_void, game_revert, main_revert_map, "reverts to last saved game, if any (for testing, the first bastard that does this to me gets it in the head)");
HS_FUNCTION_0(_hs_type_void, core_load, main_load_core, "loads debug game state from core\\core.bin");
HS_FUNCTION_0(_hs_type_void, core_load_at_startup, main_load_core_at_startup, "loads debug game state from core\\core.bin as soon as the map is initialized");
HS_FUNCTION_1(_hs_type_void, core_load_name, main_load_core_name, "loads debug game state from core\\<path>", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, core_load_name_at_startup, main_load_core_name_at_startup, "loads debug game state from core\\<path> as soon as the map is initialized", _hs_type_string);
HS_FUNCTION_0(_hs_type_void, core_save, main_save_core, "saves debug game state to core\\core.bin");
HS_FUNCTION_1(_hs_type_void, core_save_name, main_save_core_name, "saves debug game state to core\\<path>", _hs_type_string);
HS_FUNCTION_1(_hs_type_void, game_skip_ticks, main_skip, "skips <short> amount of game ticks. ONLY USE IN CUTSCENES!!!", _hs_type_short_integer);

HS_FUNCTION_0(_hs_type_boolean, game_reverted, game_state_reverted, "don't use this for anything, you black-hearted bastards.");

HS_FUNCTION_3(_hs_type_void, sound_impulse_start, scripted_sound_new, "plays an impulse sound from the specified source object (or \"none\"), with the specified scale.", _hs_type_sound, _hs_type_object, _hs_type_real);
HS_FUNCTION_1(_hs_type_long_integer, sound_impulse_time, scripted_sound_time, "returns the time remaining for the specified impulse sound.", _hs_type_sound);
HS_FUNCTION_1(_hs_type_void, sound_impulse_stop, scripted_sound_stop, "stops the specified impulse sound.", _hs_type_sound);

HS_FUNCTION_1(_hs_type_void, sound_looping_predict, scripted_foley_predict, "your mom.", _hs_type_looping_sound);
HS_FUNCTION_3(_hs_type_void, sound_looping_start, scripted_looping_sound_start, "plays a looping sound from the specified source object (or \"none\"), with the specified scale.", _hs_type_looping_sound, _hs_type_object, _hs_type_real);
HS_FUNCTION_1(_hs_type_void, sound_looping_stop, scripted_looping_sound_stop, "stops the specified looping sound.", _hs_type_looping_sound);
HS_FUNCTION_2(_hs_type_void, sound_looping_set_scale, scripted_looping_sound_set_scale, "changes the scale of the sound (which should affect the volume) within the range 0 to 1.", _hs_type_looping_sound, _hs_type_real);
HS_FUNCTION_2(_hs_type_void, sound_looping_set_alternate, scripted_looping_sound_set_alternate, "enables or disables the alternate loop/alternate end for a looping sound.", _hs_type_looping_sound, _hs_type_boolean);

HS_FUNCTION_2(_hs_type_void, debug_sounds_enable, debug_sound_classes_enable, "enables or disabled all sound classes matching the substring.", _hs_type_string, _hs_type_boolean);
HS_FUNCTION_3(_hs_type_void, debug_sounds_distances, debug_sound_classes_set_distances, "changes the minimum and maximum distances for all sound classes matching the substring.", _hs_type_string, _hs_type_real, _hs_type_real);
HS_FUNCTION_2(_hs_type_void, debug_sounds_wet, debug_sound_classes_set_wet, "changes the reverb level for all sound classes matching the substring.", _hs_type_string, _hs_type_real);
HS_FUNCTION_3(_hs_type_void, sound_class_set_gain, sound_class_set_gain, "changes the gain on the specified sound class(es) to the specified game over the specified number of ticks.", _hs_type_string, _hs_type_real, _hs_type_short_integer);

HS_FUNCTION_1(_hs_type_void, sound_enable, sound_enable, "enables or disables all sound.", _hs_type_boolean);

HS_FUNCTION_2(_hs_type_void, vehicle_hover, vehicle_hover, "stops the vehicle from running real physics and runs fake hovering physics instead.", _hs_type_vehicle, _hs_type_boolean);

HS_FUNCTION_1(_hs_type_boolean, show_hud, scripted_show_hud, "shows or hides the hud", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_boolean, show_hud_help_text, scripted_show_hud_help_text, "shows or hides the hud help text", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, enable_hud_help_flash, scripted_hud_set_flashing_state, "starts/stops the help text flashing", _hs_type_boolean);
HS_FUNCTION_0(_hs_type_void, hud_help_flash_restart, scripted_hud_restart_flashing, "resets the timer for the help text flashing");

HS_FUNCTION_4(_hs_type_void, activate_nav_point_flag, hud_unit_activate_nav_point_with_flag, "activates a nav point type <string> attached to (local) player <unit> anchored to a flag with a vertical offset <real>. If the player is not local to the machine, this will fail", _hs_type_navpoint, _hs_type_unit, _hs_type_cutscene_flag, _hs_type_real);
HS_FUNCTION_4(_hs_type_void, activate_nav_point_object, hud_unit_activate_nav_point_with_object, "activates a nav point type <string> attached to (local) player <unit> anchored to an object with a vertical offset <real>. If the player is not local to the machine, this will fail", _hs_type_navpoint, _hs_type_unit, _hs_type_object, _hs_type_real);
HS_FUNCTION_4(_hs_type_void, activate_team_nav_point_flag, hud_activate_team_nav_point_with_flag, "activates a nav point type <string> attached to a team anchored to a flag with a vertical offset <real>. If the player is not local to the machine, this will fail", _hs_type_navpoint, _hs_type_enum_team, _hs_type_cutscene_flag, _hs_type_real);
HS_FUNCTION_4(_hs_type_void, activate_team_nav_point_object, hud_activate_team_nav_point_with_object, "activates a nav point type <string> attached to a team anchored to an object with a vertical offset <real>. If the player is not local to the machine, this will fail", _hs_type_navpoint, _hs_type_enum_team, _hs_type_object, _hs_type_real);
HS_FUNCTION_2(_hs_type_void, deactivate_nav_point_flag, hud_unit_deactivate_nav_point_with_flag, "deactivates a nav point type attached to a player <unit> anchored to a flag", _hs_type_unit, _hs_type_cutscene_flag);
HS_FUNCTION_2(_hs_type_void, deactivate_nav_point_object, hud_unit_deactivate_nav_point_with_object, "deactivates a nav point type attached to a player <unit> anchored to an object", _hs_type_unit, _hs_type_object);
HS_FUNCTION_2(_hs_type_void, deactivate_team_nav_point_flag, hud_deactivate_team_nav_point_with_flag, "deactivates a nav point type attached to a team anchored to a flag", _hs_type_enum_team, _hs_type_cutscene_flag);
HS_FUNCTION_2(_hs_type_void, deactivate_team_nav_point_object, hud_deactivate_team_nav_point_with_object, "deactivates a nav point type attached to a team anchored to an object", _hs_type_enum_team, _hs_type_object);

HS_FUNCTION_0(_hs_type_void, cls, terminal_clear, "clears console text from the screen");

HS_FUNCTION_1(_hs_type_void, error_overflow_suppression, errors_overflow_suppression_enable, "enables or disables the suppression of error spamming", _hs_type_boolean);

HS_FUNCTION_0(_hs_type_void, structure_lens_flares_place, structure_lens_flares_place, "places lens flares in the structure bsp");

HS_FUNCTION_3(_hs_type_void, player_effect_set_max_translation, scripted_player_effect_set_translation, "<x> <y> <z>", _hs_type_real, _hs_type_real, _hs_type_real);
HS_FUNCTION_3(_hs_type_void, player_effect_set_max_rotation, scripted_player_effect_set_rotation, "<yaw> <pitch> <roll>", _hs_type_real, _hs_type_real, _hs_type_real);
HS_FUNCTION_2(_hs_type_void, player_effect_set_max_rumble, scripted_player_effect_set_rumble, "<left> <right>", _hs_type_real, _hs_type_real);

HS_FUNCTION_2(_hs_type_void, player_effect_start, scripted_player_effect_start, "<max_intensity> <attack time>", _hs_type_real, _hs_type_real);
HS_FUNCTION_1(_hs_type_void, player_effect_stop, scripted_player_effect_stop, "<decay>", _hs_type_real);

HS_FUNCTION_1(_hs_type_void, hud_show_health, scripted_hud_show_health, "hides/shows the health panel", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, hud_blink_health, scripted_hud_blink_health, "starts/stops manual blinking of the health panel", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, hud_show_shield, scripted_hud_show_shield, "hides/shows the shield panel", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, hud_blink_shield, scripted_hud_blink_shield, "starts/stops manual blinking of the shield panel", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, hud_show_motion_sensor, scripted_hud_show_motion_sensor, "hides/shows the motion sensor panel", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, hud_blink_motion_sensor, scripted_hud_blink_motion_sensor, "starts/stops manual blinking of the motion sensor panel", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, hud_show_crosshair, scripted_hud_show_crosshair, "hides/shows the weapon crosshair", _hs_type_boolean);
HS_FUNCTION_0(_hs_type_void, hud_clear_messages, scripted_hud_messages_clear, "clears all non-state messages on the hud");
HS_FUNCTION_1(_hs_type_void, hud_set_help_text, scripted_hud_set_state_message, "displays <message> as the help text", _hs_type_hud_message);
HS_FUNCTION_1(_hs_type_void, hud_set_objective_text, scripted_hud_set_objective, "sets <message> as the current objective", _hs_type_hud_message);

HS_FUNCTION_2(_hs_type_void, hud_set_timer_time, scripted_hud_set_timer_time, "sets the time for the timer to <short> minutes and <short> seconds, and starts and displays timer", _hs_type_short_integer, _hs_type_short_integer);
HS_FUNCTION_2(_hs_type_void, hud_set_timer_warning_time, scripted_hud_set_timer_warning_cutoff, "sets the warning time for the timer to <short> minutes and <short> seconds", _hs_type_short_integer, _hs_type_short_integer);
HS_FUNCTION_3(_hs_type_void, hud_set_timer_position, scripted_hud_set_timer_position, "sets the timer upper left position to (x, y)=>(<short>, <short>)", _hs_type_short_integer, _hs_type_short_integer, _hs_type_enum_hud_corner);
HS_FUNCTION_1(_hs_type_void, show_hud_timer, scripted_hud_show_timer, "displays the hud timer", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, pause_hud_timer, scripted_hud_pause_timer, "pauses or unpauses the hud timer", _hs_type_boolean);
HS_FUNCTION_0(_hs_type_short_integer, hud_get_timer_ticks, scripted_hud_get_timer_ticks, "returns the ticks left on the hud timer");

HS_FUNCTION_1(_hs_type_void, time_code_show, scripted_hud_time_code_show, "shows the time code timer", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, time_code_start, scripted_hud_time_code_start, "starts/stops the time code timer", _hs_type_boolean);
HS_FUNCTION_0(_hs_type_void, time_code_reset, scripted_hud_time_code_reset, "resets the time code timer");

HS_FUNCTION_0(_hs_type_void, rasterizer_decals_flush, rasterizer_decals_flush, "flush all decals");
HS_FUNCTION_0(_hs_type_void, rasterizer_fps_accumulate, rasterizer_fps_accumulate, "average fps");

HS_FUNCTION_4(_hs_type_void, rasterizer_model_ambient_reflection_tint, rasterizer_model_ambient_reflection_tint, "", _hs_type_real, _hs_type_real, _hs_type_real, _hs_type_real);
HS_FUNCTION_0(_hs_type_void, rasterizer_lights_reset_for_new_map, rasterizer_lights_reset_for_new_map, "");

HS_FUNCTION_2(_hs_type_void, script_screen_effect_set_value, rasterizer_script_screen_effect_set_value, "sets a screen effect script value", _hs_type_short_integer, _hs_type_real);

HS_FUNCTION_1(_hs_type_void, cinematic_screen_effect_start, rasterizer_screen_effect_start, "starts screen effect; pass TRUE to clear", _hs_type_boolean);

HS_FUNCTION_5(_hs_type_void, cinematic_screen_effect_set_convolution, rasterizer_screen_effect_set_convolution, "sets the convolution effect", _hs_type_short_integer, _hs_type_short_integer, _hs_type_real, _hs_type_real, _hs_type_real);

HS_FUNCTION_6(_hs_type_void, cinematic_screen_effect_set_filter, rasterizer_screen_effect_set_filter, "sets the filter effect", _hs_type_real, _hs_type_real, _hs_type_real, _hs_type_real, _hs_type_boolean, _hs_type_real);

HS_FUNCTION_3(_hs_type_void, cinematic_screen_effect_set_filter_desaturation_tint, rasterizer_screen_effect_set_filter_desaturation_tint, "sets the desaturation filter tint color", _hs_type_real, _hs_type_real, _hs_type_real);

HS_FUNCTION_2(_hs_type_void, cinematic_screen_effect_set_video, rasterizer_screen_effect_set_video, "sets the video effect: <noise intensity[0,1]>, <overbright: 0=none, 1=2x, 2=4x>", _hs_type_short_integer, _hs_type_real);
HS_FUNCTION_0(_hs_type_void, cinematic_screen_effect_stop, rasterizer_screen_effect_stop, "returns control of the screen effects to the rest of the game");
HS_FUNCTION_1(_hs_type_void, cinematic_set_near_clip_distance, rasterizer_set_near_clip_distance, "", _hs_type_real);

HS_FUNCTION_0(_hs_type_void, enumerate_memory_units, enumerate_memory_units_test, "enumerate memory units");

HS_FUNCTION_0(_hs_type_void, delete_save_game_files, saved_game_files_delete_all_custom_profiles, "delete all custom profile files");

HS_FUNCTION_0(_hs_type_void, fast_setup_network_server, player_ui_fast_setup_network_server, "for zach's multiplayer testing");
HS_FUNCTION_0(_hs_type_void, profile_unlock_solo_levels, player_ui_activate_all_solo_levels, "unlocks all the solo player levels for player 1's profile");
HS_FUNCTION_1(_hs_type_void, player0_look_invert_pitch, player0_look_invert_pitch, "invert player0's look", _hs_type_boolean);
HS_FUNCTION_0(_hs_type_boolean, player0_look_pitch_is_inverted, player0_look_pitch_is_inverted, "returns TRUE if player0's look pitch is inverted");
HS_FUNCTION_0(_hs_type_boolean, player0_joystick_set_is_normal, player0_joystick_set_is_normal, "returns TRUE if player0 is using the normal joystick set");

HS_FUNCTION_1(_hs_type_void, ui_widget_show_path, ui_widget_debug_show_path, "blah blah", _hs_type_boolean);
HS_FUNCTION_1(_hs_type_void, display_scenario_help, display_scenario_help, "display in-game help dialog", _hs_type_short_integer);

HS_FUNCTION_0(_hs_type_void, network_game_start_now, network_game_client_request_immediate_start, "another one for zach");

HS_FUNCTION_1(_hs_type_void, xbox_set_machine_name, xbox_set_machine_name, "YAHSFFZ", _hs_type_string);

struct hs_function_definition const *hs_function_table[] =
{
	&begin_definition,
	&begin_random_definition,
	&if_definition,
	&cond_definition,
	&set_definition,
	&and_definition,
	&or_definition,
	&add_definition,
	&subtract_definition,
	&multiply_definition,
	&divide_definition,
	&min_definition,
	&max_definition,
	&equal_definition,
	&not_equal_definition,
	&gt_definition,
	&lt_definition,
	&gte_definition,
	&lte_definition,
	&sleep_definition,
	&sleep_until_definition,
	&wake_definition,
	&inspect_definition,
	&object_to_unit_definition,
	&ai_debug_communication_suppress_definition,
	&ai_debug_communication_ignore_definition,
	&ai_debug_communication_focus_definition,
	&not_definition,
	&print_definition,
	&players_definition,
	&volume_teleport_players_not_inside_definition,
	&volume_test_object_definition,
	&volume_test_objects_definition,
	&volume_test_objects_all_definition,
	&object_teleport_definition,
	&object_set_facing_definition,
	&object_set_shield_definition,
	&object_set_permutation_definition,
	&object_create_definition,
	&object_destroy_definition,
	&object_create_anew_definition,
	&object_create_containing_definition,
	&object_create_anew_containing_definition,
	&object_destroy_containing_definition,
	&object_destroy_all_definition,
	&list_get_definition,
	&list_count_definition,
	&effect_new_definition,
	&effect_new_on_object_marker_definition,
	&damage_new_definition,
	&damage_object_definition,
	&objects_can_see_object_definition,
	&objects_can_see_flag_definition,
	&objects_delete_by_definition_definition,
	&sound_set_gain_definition,
	&sound_get_gain_definition,
	&script_recompile_definition,
	&script_doc_definition,
	&help_definition,
	&random_range_definition,
	&real_random_range_definition,
	&numeric_countdown_timer_set_definition,
	&numeric_countdown_timer_get_definition,
	&numeric_countdown_timer_stop_definition,
	&numeric_countdown_timer_restart_definition,
	&breakable_surfaces_enable_definition,
	&recording_play_definition,
	&recording_play_and_delete_definition,
	&recording_play_and_hover_definition,
	&recording_kill_definition,
	&recording_time_definition,
	&object_set_ranged_attack_inhibited_definition,
	&object_set_melee_attack_inhibited_definition,
	&objects_dump_memory_definition,
	&object_set_collideable_definition,
	&object_set_scale_definition,
	&objects_attach_definition,
	&objects_detach_definition,
	&garbage_collect_now_definition,
	&object_cannot_take_damage_definition,
	&object_can_take_damage_definition,
	&object_beautify_definition,
	&objects_predict_definition,
	&object_type_predict_definition,
	&object_pvs_activate_definition,
	&object_pvs_set_object_definition,
	&object_pvs_set_camera_definition,
	&object_pvs_clear_definition,
	&render_lights_definition,
	&scenery_get_animation_time_definition,
	&scenery_animation_start_definition,
	&scenery_animation_start_at_frame_definition,
	&render_effects_definition,
	&unit_can_blink_definition,
	&unit_open_definition,
	&unit_close_definition,
	&unit_kill_definition,
	&unit_kill_silent_definition,
	&unit_get_custom_animation_time_definition,
	&unit_stop_custom_animation_definition,
	&unit_custom_animation_at_frame_definition,
	&custom_animation_definition,
	&custom_animation_list_definition,
	&unit_is_playing_custom_animation_definition,
	&unit_aim_without_turning_definition,
	&unit_set_emotion_definition,
	&unit_set_enterable_by_player_definition,
	&unit_enter_vehicle_definition,
	&vehicle_test_seat_list_definition,
	&vehicle_test_seat_definition,
	&unit_set_emotion_animation_definition,
	&unit_exit_vehicle_definition,
	&unit_set_maximum_vitality_definition,
	&units_set_maximum_vitality_definition,
	&unit_set_current_vitality_definition,
	&units_set_current_vitality_definition,
	&vehicle_load_magic_definition,
	&vehicle_unload_definition,
	&magic_seat_name_definition,
	&unit_set_seat_definition,
	&magic_melee_attack_definition,
	&vehicle_riders_definition,
	&vehicle_driver_definition,
	&vehicle_gunner_definition,
	&unit_get_health_definition,
	&unit_get_shield_definition,
	&unit_get_total_grenade_count_definition,
	&unit_has_weapon_definition,
	&unit_has_weapon_readied_definition,
	&unit_doesnt_drop_items_definition,
	&unit_impervious_definition,
	&unit_suspended_definition,
	&unit_solo_player_integrated_night_vision_is_active_definition,
	&units_set_desired_flashlight_state_definition,
	&unit_set_desired_flashlight_state_definition,
	&unit_get_current_flashlight_state_definition,
	&device_set_never_appears_locked_definition,
	&device_get_power_definition,
	&device_set_power_definition,
	&device_set_position_definition,
	&device_get_position_definition,
	&device_set_position_immediate_definition,
	&device_group_get_definition,
	&device_group_set_definition,
	&device_group_set_immediate_definition,
	&device_one_sided_set_definition,
	&device_operates_automatically_set_definition,
	&device_group_change_only_once_more_set_definition,
	&breakable_surfaces_reset_definition,
	&cheat_all_powerups_definition,
	&cheat_all_weapons_definition,
	&cheat_all_vehicles_definition,
	&cheat_teleport_to_camera_definition,
	&cheat_active_camouflage_definition,
	&cheat_active_camouflage_local_player_definition,
	&cheats_load_definition,
	&ai_free_definition,
	&ai_free_units_definition,
	&ai_attach_definition,
	&ai_attach_free_definition,
	&ai_detach_definition,
	&ai_place_definition,
	&ai_kill_definition,
	&ai_kill_silent_definition,
	&ai_erase_definition,
	&ai_erase_all_definition,
	&ai_select_definition,
	&ai_deselect_definition,
	&ai_spawn_actor_definition,
	&ai_set_respawn_definition,
	&ai_set_deaf_definition,
	&ai_set_blind_definition,
	&ai_magically_see_encounter_definition,
	&ai_magically_see_players_definition,
	&ai_magically_see_unit_definition,
	&ai_timer_start_definition,
	&ai_timer_expire_definition,
	&ai_attack_definition,
	&ai_defend_definition,
	&ai_retreat_definition,
	&ai_maneuver_definition,
	&ai_maneuver_enable_definition,
	&ai_migrate_definition,
	&ai_migrate_and_speak_definition,
	&ai_migrate_by_unit_definition,
	&ai_allegiance_definition,
	&ai_allegiance_remove_definition,
	&ai_living_count_definition,
	&ai_living_fraction_definition,
	&ai_strength_definition,
	&ai_swarm_count_definition,
	&ai_nonswarm_count_definition,
	&ai_actors_definition,
	&ai_go_to_vehicle_definition,
	&ai_go_to_vehicle_override_definition,
	&ai_going_to_vehicle_definition,
	&ai_exit_vehicle_definition,
	&ai_braindead_definition,
	&ai_braindead_by_unit_definition,
	&ai_disregard_definition,
	&ai_prefer_target_definition,
	&ai_teleport_to_starting_location_definition,
	&ai_teleport_to_starting_location_if_unsupported_definition,
	&ai_renew_definition,
	&ai_try_to_fight_nothing_definition,
	&ai_try_to_fight_definition,
	&ai_try_to_fight_player_definition,
	&ai_command_list_definition,
	&ai_command_list_by_unit_definition,
	&ai_command_list_advance_definition,
	&ai_command_list_advance_by_unit_definition,
	&ai_command_list_status_definition,
	&ai_is_attacking_definition,
	&ai_force_active_definition,
	&ai_force_active_by_unit_definition,
	&ai_set_return_state_definition,
	&ai_set_current_state_definition,
	&ai_playfight_definition,
	&ai_status_definition,
	&ai_reconnect_definition,
	&ai_vehicle_encounter_definition,
	&ai_vehicle_enterable_distance_definition,
	&ai_vehicle_enterable_team_definition,
	&ai_vehicle_enterable_actor_type_definition,
	&ai_vehicle_enterable_actors_definition,
	&ai_vehicle_enterable_disable_definition,
	&ai_look_at_object_definition,
	&ai_stop_looking_definition,
	&ai_automatic_migration_target_definition,
	&ai_follow_target_disable_definition,
	&ai_follow_target_players_definition,
	&ai_follow_target_unit_definition,
	&ai_follow_target_ai_definition,
	&ai_follow_distance_definition,
	&ai_conversation_definition,
	&ai_conversation_stop_definition,
	&ai_conversation_advance_definition,
	&ai_conversation_line_definition,
	&ai_conversation_status_definition,
	&ai_link_activation_definition,
	&ai_berserk_definition,
	&ai_set_team_definition,
	&ai_allow_charge_definition,
	&ai_allow_dormant_definition,
	&ai_allegiance_broken_definition,
	&camera_control_definition,
	&camera_set_definition,
	&camera_set_relative_definition,
	&camera_set_animation_definition,
	&camera_set_first_person_definition,
	&camera_set_dead_definition,
	&camera_time_definition,
	&debug_camera_load_definition,
	&debug_camera_save_definition,
	&game_speed_definition,
	&game_time_definition,
	&game_variant_definition,
	&game_difficulty_get_definition,
	&game_difficulty_get_real_definition,
	&map_reset_definition,
	&map_name_definition,
	&multiplayer_map_name_definition,
	&game_difficulty_set_definition,
	&crash_definition,
	&switch_bsp_definition,
	&structure_bsp_index_definition,
	&version_definition,
	&playback_definition,
	&texture_cache_flush_definition,
	&sound_cache_flush_definition,
	&debug_memory_definition,
	&debug_memory_by_file_definition,
	&debug_memory_for_file_definition,
	&debug_tags_definition,
	&profile_reset_definition,
	&profile_dump_definition,
	&profile_activate_definition,
	&profile_deactivate_definition,
	&profile_graph_toggle_definition,
	&debug_pvs_definition,
	&radiosity_start_definition,
	&radiosity_save_definition,
	&radiosity_debug_point_definition,
	&ai_definition,
	&ai_dialogue_triggers_definition,
	&ai_grenades_definition,
	&ai_lines_definition,
	&ai_debug_sound_point_set_definition,
	&ai_debug_vocalize_definition,
	&ai_debug_teleport_to_definition,
	&ai_debug_speak_definition,
	&ai_debug_speak_list_definition,
	&fade_in_definition,
	&fade_out_definition,
	&cinematic_start_definition,
	&cinematic_stop_definition,
	&cinematic_skip_start_internal_definition,
	&cinematic_skip_stop_internal_definition,
	&cinematic_show_letterbox_definition,
	&cinematic_set_title_definition,
	&cinematic_set_title_delayed_definition,
	&cinematic_suppress_bsp_object_creation_definition,
	&attract_mode_start_definition,
	&game_won_definition,
	&game_lost_definition,
	&game_safe_to_save_definition,
	&game_all_quiet_definition,
	&game_safe_to_speak_definition,
	&game_is_cooperative_definition,
	&game_save_definition,
	&game_save_cancel_definition,
	&game_save_no_timeout_definition,
	&game_save_totally_unsafe_definition,
	&game_saving_definition,
	&game_revert_definition,
	&game_reverted_definition,
	&core_save_definition,
	&core_save_name_definition,
	&core_load_definition,
	&core_load_at_startup_definition,
	&core_load_name_definition,
	&core_load_name_at_startup_definition,
	&game_skip_ticks_definition,
	&sound_impulse_start_definition,
	&sound_impulse_time_definition,
	&sound_impulse_stop_definition,
	&sound_looping_predict_definition,
	&sound_looping_start_definition,
	&sound_looping_stop_definition,
	&sound_looping_set_scale_definition,
	&sound_looping_set_alternate_definition,
	&debug_sounds_enable_definition,
	&debug_sounds_distances_definition,
	&debug_sounds_wet_definition,
	&sound_enable_definition,
	&sound_class_set_gain_definition,
	&vehicle_hover_definition,
	&players_unzoom_all_definition,
	&player_enable_input_definition,
	&player_camera_control_definition,
	&player_action_test_reset_definition,
	&player_action_test_jump_definition,
	&player_action_test_primary_trigger_definition,
	&player_action_test_grenade_trigger_definition,
	&player_action_test_zoom_definition,
	&player_action_test_action_definition,
	&player_action_test_accept_definition,
	&player_action_test_back_definition,
	&player_action_test_look_relative_up_definition,
	&player_action_test_look_relative_down_definition,
	&player_action_test_look_relative_left_definition,
	&player_action_test_look_relative_right_definition,
	&player_action_test_look_relative_all_directions_definition,
	&player_action_test_move_relative_all_directions_definition,
	&player_add_equipment_definition,
	&debug_teleport_player_definition,
	&show_hud_definition,
	&show_hud_help_text_definition,
	&enable_hud_help_flash_definition,
	&hud_help_flash_restart_definition,
	&activate_nav_point_flag_definition,
	&activate_nav_point_object_definition,
	&activate_team_nav_point_flag_definition,
	&activate_team_nav_point_object_definition,
	&deactivate_nav_point_flag_definition,
	&deactivate_nav_point_object_definition,
	&deactivate_team_nav_point_flag_definition,
	&deactivate_team_nav_point_object_definition,
	&cls_definition,
	&error_overflow_suppression_definition,
	&structure_lens_flares_place_definition,
	&player_effect_set_max_translation_definition,
	&player_effect_set_max_rotation_definition,
	&player_effect_set_max_rumble_definition,
	&player_effect_start_definition,
	&player_effect_stop_definition,
	&hud_show_health_definition,
	&hud_blink_health_definition,
	&hud_show_shield_definition,
	&hud_blink_shield_definition,
	&hud_show_motion_sensor_definition,
	&hud_blink_motion_sensor_definition,
	&hud_show_crosshair_definition,
	&hud_clear_messages_definition,
	&hud_set_help_text_definition,
	&hud_set_objective_text_definition,
	&hud_set_timer_time_definition,
	&hud_set_timer_warning_time_definition,
	&hud_set_timer_position_definition,
	&show_hud_timer_definition,
	&pause_hud_timer_definition,
	&hud_get_timer_ticks_definition,
	&time_code_show_definition,
	&time_code_start_definition,
	&time_code_reset_definition,
	&rasterizer_decals_flush_definition,
	&rasterizer_fps_accumulate_definition,
	&rasterizer_model_ambient_reflection_tint_definition,
	&rasterizer_lights_reset_for_new_map_definition,
	&script_screen_effect_set_value_definition,
	&cinematic_screen_effect_start_definition,
	&cinematic_screen_effect_set_convolution_definition,
	&cinematic_screen_effect_set_filter_definition,
	&cinematic_screen_effect_set_filter_desaturation_tint_definition,
	&cinematic_screen_effect_set_video_definition,
	&cinematic_screen_effect_stop_definition,
	&cinematic_set_near_clip_distance_definition,
	&enumerate_memory_units_definition,
	&delete_save_game_files_definition,
	&fast_setup_network_server_definition,
	&profile_unlock_solo_levels_definition,
	&player0_look_invert_pitch_definition,
	&player0_look_pitch_is_inverted_definition,
	&player0_joystick_set_is_normal_definition,
	&ui_widget_show_path_definition,
	&display_scenario_help_definition,
	&network_game_start_now_definition,
	&xbox_set_machine_name_definition
};

short const hs_function_table_count = NUMBEROF(hs_function_table);

#endif // __HS_FUNCTION_H
