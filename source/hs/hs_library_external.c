/*
HS_LIBRARY_EXTERNAL.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hs_library_external.h"
#include "object_lists.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "console.h"
#include "items.h"
#include "sound_definitions.h"
#include "effects.h"
#include "terminal.h"
#include "object_types.h"
#include "objects.h"
#include "object_definitions.h"
#include "model_definitions.h"
#include "damage.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "hs.h"

/* ---------- public code */

boolean hs_not(
	boolean value)
{
	return !value;
}

void hs_print(
	char const *string)
{
	terminal_printf(global_real_argb_green, string);

	return;
}

long hs_players(
	void)
{
	long player_index;
	long object_list_index = object_list_new();

	for (player_index = data_next_index(player_data, NONE);
		player_index!=NONE;
		player_index = data_next_index(player_data, player_index))
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index!=NONE)
		{
			object_list_add(object_list_index, player->unit_index);
		}
	}

	return object_list_index;
}

void hs_teleport_players_not_in_trigger_volume(
	short trigger_volume_index,
	short flag_index)
{
	long player_index;

	for (player_index = data_next_index(player_data, NONE);
		player_index!=NONE;
		player_index = data_next_index(player_data, player_index))
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index!=NONE
			&& !scenario_trigger_volume_test_object(trigger_volume_index, player->unit_index))
		{
			hs_object_teleport(player->unit_index, flag_index);
		}
	}

	return;
}

boolean hs_trigger_volume_test_objects_all(
	short trigger_volume_index,
	long object_list_index)
{
	return hs_trigger_volume_test_objects(trigger_volume_index, object_list_index, TRUE);
}

boolean hs_trigger_volume_test_objects_any(
	short trigger_volume_index,
	long object_list_index)
{
	return hs_trigger_volume_test_objects(trigger_volume_index, object_list_index, FALSE);
}

boolean hs_trigger_volume_test_objects(
	short trigger_volume_index,
	long object_list_index,
	boolean and)
{
	long reference_index;
	long object_index;
	boolean result = and;

	for (object_index = object_list_get_first(object_list_index, &reference_index);
		object_index!=NONE;
		object_index = object_list_get_next(object_list_index, &reference_index))
	{
		if (scenario_trigger_volume_test_object(trigger_volume_index, object_index))
		{
			if (!and)
			{
				result = TRUE;
				break;
			}
		}
		else if (and)
		{
			result = FALSE;
			break;
		}
	}

#ifdef DEBUG
	BIT_VECTOR_SET_FLAG(hs_debug_data, trigger_volume_index, result);
#endif
	return result;
}

boolean hs_unit_can_see_object(
	long unit_index,
	long object_index,
	real degrees)
{
	boolean result = FALSE;

	if (object_index!=NONE)
	{
		real_point3d target_point;

		if (unit_try_and_get(object_index))
		{
			unit_get_head_position(object_index, &target_point);
		}
		else
		{
			target_point = object_get(object_index)->object.bounding_sphere_center;
		}

		result = unit_can_see_point(unit_index, &target_point, degrees * (_pi / 180.f));
	}

	return result;
}

boolean hs_objects_can_see_object(
	long object_list_index,
	long object_index,
	real degrees)
{
	long reference_index;
	long unit_index;
	boolean result = FALSE;

	for (unit_index = object_list_get_first(object_list_index, &reference_index);
		unit_index!=NONE;
		unit_index = object_list_get_next(object_list_index, &reference_index))
	{
		if (unit_try_and_get(unit_index) && hs_unit_can_see_object(unit_index, object_index, degrees))
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

boolean hs_unit_can_see_flag(
	long unit_index,
	short flag_index,
	real degrees)
{
	boolean result = FALSE;

	if (flag_index)
	{
		result = unit_can_see_point(
			unit_index,
			&TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->cutscene_flags, flag_index, struct scenario_cutscene_flag)->position,
			degrees * (_pi / 180.f));
	}

	return result;
}

boolean hs_objects_can_see_flag(
	long object_list_index,
	short flag_index,
	real degrees)
{
	long reference_index;
	long unit_index;
	boolean result = FALSE;

	for (unit_index = object_list_get_first(object_list_index, &reference_index);
		unit_index!=NONE;
		unit_index = object_list_get_next(object_list_index, &reference_index))
	{
		if (unit_try_and_get(unit_index) && hs_unit_can_see_flag(unit_index, flag_index, degrees))
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

static boolean object_is_or_contains_player(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	boolean result = player_index_from_unit_index(object_index)!=NONE;

	if (!result)
	{
		long child_object_index;
		struct object_datum *child_object;

		for (child_object_index = object->object.first_child_object_index;
			child_object_index!=NONE;
			child_object_index = child_object->object.next_object_index)
		{
			child_object = object_get(child_object_index);

			if (object_is_or_contains_player(child_object_index))
			{
				result = TRUE;
				break;
			}
		}
	}

	if (!result)
	{
		long parent_object_index;
		struct object_datum *parent_object;

		for (parent_object_index = object->object.parent_object_index;
			parent_object_index!=NONE;
			parent_object_index = parent_object->object.parent_object_index)
		{
			parent_object = object_get(parent_object_index);

			if (player_index_from_unit_index(parent_object_index)!=NONE)
			{
				result = TRUE;
				break;
			}
		}
	}

	if (!result
		&& TEST_FLAG(_object_mask_item, object->object.type)
		&& TEST_FLAG(((struct item_datum *)object)->item.flags, _item_belongs_to_player_bit))
	{
		result = TRUE;
	}

	return result;
}

void hs_object_create(
	short object_name_index)
{
	if (object_name_index!=NONE)
	{
		if (object_index_from_name_index(object_name_index)!=NONE)
		{
			error(
				_error_silent,
				"WARNING: object_create - '%s' already exists",
				TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, object_name_index, struct scenario_object_name)->name);
		}
		else
		{
			object_new_by_name(object_name_index);
		}
	}

	return;
}

void hs_object_create_anew(
	short object_name_index)
{
	if (object_name_index!=NONE)
	{
		long object_index = object_index_from_name_index(object_name_index);

		if (object_index!=NONE)
		{
			hs_object_destroy(object_index);
		}

		hs_object_create(object_name_index);
	}

	return;
}

void hs_object_destroy(
	long object_index)
{
	if (object_index!=NONE)
	{
		if (!object_is_or_contains_player(object_index))
		{
			object_delete(object_index);
		}
		else
		{
			error(
				_error_silent,
				"### ERROR a script tried to delete the player (or the horse he rode in on, or his six-shooter)");
		}
	}

	return;
}

void hs_object_destroy_by_name(
	short object_name_index)
{
	if (object_name_index!=NONE)
	{
		long object_index = object_index_from_name_index(object_name_index);

		if (object_index!=NONE)
		{
			hs_object_destroy(object_index);
		}
	}

	return;
}

void hs_object_destroy_all(
	void)
{
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);

		while (player = (struct player_datum *)data_iterator_next(&iterator))
		{
			if (player->unit_index!=NONE
				&& object_get_ultimate_parent(player->unit_index)!=player->unit_index)
			{
				unit_exit_seat_end(player->unit_index);
			}
		}
	}

	{
		struct object_iterator iterator;
		struct object_datum *object;

		object_iterator_new(&iterator, _object_mask_all, 0);

		while (object = (struct object_datum *)object_iterator_next(&iterator))
		{
			if (object->object.parent_object_index==NONE
				&& !object_is_or_contains_player(iterator.index))
			{
				object_delete(iterator.index);
			}
		}
	}

	return;
}

static void hs_object_iterate_names_containing(
	char const *name_string,
	void (*iterator)(short))
{
	short name_index;
	struct scenario *scenario = global_scenario_get();

	match_assert("c:\\halo\\SOURCE\\hs\\hs_library_external.c", 407, iterator);

	for (name_index = 0; name_index<scenario->object_names.count; name_index++)
	{
		struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(
			&scenario->object_names,
			name_index,
			struct scenario_object_name);

		if (strstr(object_name->name, name_string))
		{
			iterator(name_index);
		}
	}

	return;
}

void hs_object_create_containing(
	char const *name_string)
{
	hs_object_iterate_names_containing(name_string, hs_object_create);

	return;
}

void hs_object_create_anew_containing(
	char const *name_string)
{
	hs_object_iterate_names_containing(name_string, hs_object_create_anew);

	return;
}

void hs_object_destroy_containing(
	char const *name_string)
{
	hs_object_iterate_names_containing(name_string, hs_object_destroy_by_name);

	return;
}

static void hs_object_orient(
	long object_index,
	short flag_index,
	boolean position,
	boolean facing)
{
	if (object_index!=NONE)
	{
		struct unit_datum *unit;
		real_vector3d forward;
		struct object_datum *object = object_get(object_index);
		struct scenario_cutscene_flag *flag = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->cutscene_flags,
			flag_index,
			struct scenario_cutscene_flag);
		struct player_datum *player = NULL;

		match_assert_valid_real_point3d("c:\\halo\\SOURCE\\hs\\hs_library_external.c", 460, &flag->position);

		if (position && object->object.parent_object_index!=NONE)
		{
			if (unit_try_and_get(object_index))
			{
				unit_exit_seat_end(object_index);
			}
			else
			{
				object_detach(object_index);
			}
		}

		vector3d_from_euler_angles2d(&forward, &flag->facing);
		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\hs\\hs_library_external.c", 479, &forward);
		object_reset(object_index);
		unit = unit_try_and_get(object_index);

		if (unit)
		{
			real_vector3d relative_forward;
			long player_index = player_index_from_unit_index(object_index);

			if (unit->object.parent_object_index!=NONE)
			{
				real_matrix4x3 inverted_matrix;

				matrix4x3_inverse(
					object_get_node_matrix(unit->object.parent_object_index, unit->object.parent_node_index),
					&inverted_matrix);
				matrix4x3_transform_normal(&inverted_matrix, &forward, &relative_forward);
			}
			else
			{
				relative_forward = forward;
			}

			if (facing)
			{
				unit->unit.desired_facing_vector = forward;
				unit->unit.desired_aiming_vector = forward;
				unit->unit.desired_looking_vector = forward;
			}

			if (player_index!=NONE)
			{
				player = player_get(player_index);

				if (position)
				{
					player_teleport(player_index, NONE, &flag->position);
				}

				if (facing && player->local_player_index!=NONE)
				{
					player_control_set_facing(player->local_player_index, &relative_forward);
				}
			}
		}

		object_set_position(
			object_index,
			(position && !player) ? &flag->position : NULL,
			(facing && !player) ? &forward : NULL,
			NULL);
	}

	return;
}

void hs_object_teleport(
	long object_index,
	short flag_index)
{
	hs_object_orient(object_index, flag_index, TRUE, TRUE);

	return;
}

void hs_object_set_facing(
	long object_index,
	short flag_index)
{
	hs_object_orient(object_index, flag_index, FALSE, TRUE);

	return;
}

long hs_object_list_get_element(
	long object_list_index,
	short element_index)
{
	long reference_index;
	long object_index = object_list_get_first(object_list_index, &reference_index);

	while (element_index>0 && object_index!=NONE)
	{
		object_index = object_list_get_next(object_list_index, &reference_index);
		element_index--;
	}

	return object_index;
}

void hs_object_set_shield(
	long object_index,
	real shield)
{
	if (object_index!=NONE)
	{
		struct object_datum *object = object_get(object_index);

		shield = PIN(shield, 0.f, 1.f);
		object->object.shield_vitality = shield * object->object.maximum_shield_vitality;
	}

	return;
}

void hs_object_set_permutation(
	long object_index,
	char const *region_name,
	char *permutation_name)
{
	if (object_index!=NONE)
	{
		struct object_datum *object = object_get(object_index);
		struct object_definition *definition = object_definition_get(object->definition_index);
		short region_index = NONE;

		if (strcmp(region_name, "") && definition->object.model.index!=NONE)
		{
			short i;
			struct model *model = model_definition_get(definition->object.model.index);

			for (i = 0; i<model->regions.count; i++)
			{
				struct model_region *region = TAG_BLOCK_GET_ELEMENT(&model->regions, i, struct model_region);

				if (!_stricmp(region->name, region_name))
				{
					region_index = i;
					break;
				}
			}
		}

		object_permute_region(object_index, permutation_name, region_index, TRUE);
	}

	return;
}

void hs_objects_predict(
	long object_list_index)
{
	long reference_index;
	long object_index;

	for (object_index = object_list_get_first(object_list_index, &reference_index);
		object_index!=NONE;
		object_index = object_list_get_next(object_list_index, &reference_index))
	{
		object_predict(object_index);
	}

	return;
}

void hs_objects_delete_by_definition(
	long definition_index)
{
	struct object_iterator iterator;
	struct object_datum *object;

	object_iterator_new(&iterator, _object_mask_all, 0);

	while (object = (struct object_datum *)object_iterator_next(&iterator))
	{
		if (object->definition_index==definition_index)
		{
			object_delete(iterator.index);
		}
	}

	objects_memory_compact();

	return;
}

void hs_effect_new(
	long definition_index,
	short flag_index)
{
	real_vector3d direction;
	struct scenario_cutscene_flag *flag = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->cutscene_flags,
		flag_index,
		struct scenario_cutscene_flag);

	vector3d_from_euler_angles2d(&direction, &flag->facing);
	effect_new_unattached_from_markers(
		definition_index,
		NONE,
		global_zero_vector3d,
		1,
		NULL,
		&flag->position,
		&direction,
		1.f,
		1.f,
		NULL,
		NULL,
		TRUE);

	return;
}

void hs_effect_new_from_object_marker(
	long definition_index,
	long object_index,
	char const *marker_name)
{
	if (definition_index!=NONE && object_index!=NONE)
	{
		struct object_marker marker;

		if (object_get_marker_by_name(object_index, marker_name, &marker, 1))
		{
			effect_new_attached_from_markers(
				definition_index,
				NONE,
				object_index,
				marker.node_index,
				1,
				&marker_name,
				&marker.matrix.position,
				&marker.matrix.forward,
				1.f,
				1.f,
				NULL,
				NULL);
		}
	}

	return;
}

void hs_damage_new(
	long definition_index,
	short flag_index)
{
	struct damage_data damage;
	struct scenario_cutscene_flag *flag = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->cutscene_flags,
		flag_index,
		struct scenario_cutscene_flag);

	damage_data_new(&damage, definition_index);
	damage.epicenter = flag->position;
	damage.origin = damage.epicenter;
	scenario_location_from_point(&damage.location, &flag->position);
	area_of_effect_cause_damage(&damage, NONE);

	return;
}

void hs_damage_object(
	long definition_index,
	long object_index)
{
	if (object_index!=NONE)
	{
		struct damage_data damage;

		damage_data_new(&damage, definition_index);
		object_get_origin(object_index, &damage.origin);
		damage.epicenter = damage.origin;
		scenario_location_from_point(&damage.location, &damage.origin);
		object_cause_damage(&damage, object_index, NONE, NONE, NONE, NULL);
	}

	return;
}

static real *hs_sound_get_gain_reference(
	char const *tag_name)
{
	long tag_index = tag_loaded(SOUND_DEFINITION_TAG, tag_name);

	if (tag_index!=NONE)
	{
		return &sound_definition_get(tag_index)->gain;
	}

	tag_index = tag_loaded(LOOPING_SOUND_DEFINITION_TAG, tag_name);

	if (tag_index!=NONE)
	{
		struct looping_sound_definition *looping_sound = looping_sound_definition_get(tag_index);

		if (looping_sound->tracks.count>0)
		{
			return &TAG_BLOCK_GET_ELEMENT(&looping_sound->tracks, 0, struct looping_sound_track)->gain;
		}
	}

	console_printf(FALSE, "the sound '%s' does not exist", tag_name);

	return NULL;
}

real hs_sound_get_gain(
	char const *tag_name)
{
	real *gain_reference = hs_sound_get_gain_reference(tag_name);

	return gain_reference ? *gain_reference : 0.f;
}

void hs_sound_set_gain(
	char const *tag_name,
	real gain)
{
	real *gain_reference = hs_sound_get_gain_reference(tag_name);

	if (gain_reference)
	{
		*gain_reference = gain;
	}

	return;
}
