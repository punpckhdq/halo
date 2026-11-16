/*
HS_LIBRARY_EXTERNAL.H

header included in hcex build.
*/

#ifndef __HS_LIBRARY_EXTERNAL_H
#define __HS_LIBRARY_EXTERNAL_H
#pragma once

/* ---------- prototypes/HS_LIBRARY_EXTERNAL.C */

boolean hs_not(boolean value);
void hs_print(char const *string);
long hs_players(void);
void hs_teleport_players_not_in_trigger_volume(short trigger_volume_index, short flag_index);
boolean hs_trigger_volume_test_objects_all(short trigger_volume_index, long object_list_index);
boolean hs_trigger_volume_test_objects_any(short trigger_volume_index, long object_list_index);
boolean hs_trigger_volume_test_objects(short trigger_volume_index, long object_list_index, boolean and);
boolean hs_unit_can_see_object(long unit_index, long object_index, real degrees);
boolean hs_objects_can_see_object(long object_list_index, long object_index, real degrees);
boolean hs_unit_can_see_flag(long unit_index, short flag_index, real degrees);
boolean hs_objects_can_see_flag(long object_list_index, short flag_index, real degrees);
void hs_object_create(short object_name_index);
void hs_object_create_anew(short object_name_index);
void hs_object_destroy(long object_index);
void hs_object_destroy_by_name(short object_name_index);
void hs_object_destroy_all(void);
void hs_object_create_containing(char const *name_string);
void hs_object_create_anew_containing(char const *name_string);
void hs_object_destroy_containing(char const *name_string);
void hs_object_teleport(long object_index, short flag_index);
void hs_object_set_facing(long object_index, short flag_index);
long hs_object_list_get_element(long object_list_index, short element_index);
void hs_object_set_shield(long object_index, real shield);
void hs_object_set_permutation(long object_index, char const *region_name, char *permutation_name);
void hs_objects_predict(long object_list_index);
void hs_objects_delete_by_definition(long definition_index);
void hs_effect_new(long definition_index, short flag_index);
void hs_effect_new_from_object_marker(long definition_index, long object_index, char const *marker_name);
void hs_damage_new(long definition_index, short flag_index);
void hs_damage_object(long definition_index, long object_index);
real hs_sound_get_gain(char const *tag_name);
void hs_sound_set_gain(char const *tag_name, real gain);

#endif // __HS_LIBRARY_EXTERNAL_H
