/*
ITEMS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "items.h"
#include "item_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "actor_definitions.h"
#include "network_messages.h"
#include "collisions.h"
#include "collision_usage.h"
#include "collision_bsp_definitions.h"
#include "bsp3d.h"
#include "breakable_surfaces.h"
#include "sound_manager.h"
#include "meter_definitions.h"
#include "game_sound.h"
#include "game_engine.h"
#include "scenario.h"
#include "structures.h"
#include "physics_constants.h"
#include "editor_stubs.h"
#include "effects.h"
#include "physics.h"
#include "material_effects.h"
#include "material_effect_definitions.h"

/* ---------- constants */

enum
{
	_collision_test_item_flags =
		_collision_test_environment_flags |
		FLAG(_collision_test_ignore_invisible_surfaces_bit) |
		FLAG(_collision_test_objects_bit) |
		(_collision_test_objects_all_types_flags & ~(FLAG(_collision_test_objects_weapons_bit) | FLAG(_collision_test_objects_equipment_bit))) |
		FLAG(_collision_test_try_to_keep_location_valid_bit), /* fake name */
	_object_item_rest_type_mask = _object_mask_scenery | _object_mask_device, /* fake name */
};

/* ---------- prototypes */

static void verify_item_location(long item_index);
static void item_adjust_for_angular_velocity_change(long object_index);
static void item_align_to_normal_and_point(long item_index, real_vector3d const *normal, real_point3d const *position, real_point3d *new_position);

/* ---------- globals */

static struct profile_section item_update_section =
{
	"item_update",
	NONE,
	TRUE
};

/* ---------- public code */

void items_initialize(
	void)
{
	return;
}

void items_initialize_for_new_map(
	void)
{
	return;
}

void items_dispose_from_old_map(
	void)
{
	return;
}

void items_dispose(
	void)
{
	return;
}

boolean item_new(
	long item_index)
{
	struct item_datum *item = item_get(item_index);

	SET_FLAG(item->object.flags, _object_dynamic_lighting_recompute_bit, TRUE);
	SET_FLAG(item->object.flags, _object_static_lighting_recompute_bit, TRUE);
	item->item.last_owned_time = game_time_get();
	item->item.ignore_object_index = NONE;

	return TRUE;
}

void item_delete(
	long item_index)
{
	return;
}

static void verify_item_location(
	long item_index)
{
	return;
}

boolean item_update(
	long item_index)
{
	static real const item_maximum_speed = 0.1f;
	struct item_datum *item = item_get(item_index);
	struct item_definition const *definition = item_definition_get(item->definition_index);

	profile_enter(item_update_section);
	match_collision_log_begin_user("c:\\halo\\SOURCE\\items\\items.c", 174, _collision_user_items);

	if (TEST_FLAG(item->object.flags, _object_connected_to_map_bit) &&
		item->object.parent_object_index == NONE)
	{
		if (TEST_FLAG(definition->item.flags, _item_always_maintains_z_up_bit) &&
			!realcmp(item->object.up.k, 1.f))
		{
			real_vector3d left;

			item->object.up = *global_up3d;
			cross_product3d(&item->object.up, &item->object.forward, &left);
			cross_product3d(&left, &item->object.up, &item->object.forward);

			if (normalize3d(&item->object.forward) == 0.f)
			{
				item->object.forward = *global_forward3d;
			}
		}

		if (!TEST_FLAG(item->object.flags, _object_at_rest_bit))
		{
			real_vector3d new_velocity;
			real_point3d new_position;
			struct collision_result collision;

			new_velocity = item->object.translational_velocity;

			if (!TEST_FLAG(definition->item.flags, _item_antigravity_bit))
			{
				new_velocity.k -= global_gravity;
			}

			new_position.x = item->object.position.x + new_velocity.i;
			new_position.y = item->object.position.y + new_velocity.j;
			new_position.z = item->object.position.z + new_velocity.k;

			if (collision_test_line(
				_collision_test_item_flags,
				&item->object.position,
				&new_position,
				item->item.ignore_object_index,
				&collision))
			{
				real impact_scale;

				point_from_line3d(&new_position, &collision.plane.n, 0.05f, &new_position);
				impact_scale = magnitude3d(&new_velocity) / item_maximum_speed;
				impact_scale = PIN(impact_scale, 0.f, 1.f);

				if (definition->item.material_effects.index != NONE &&
					material_effect_visible(&collision.point))
				{
					material_effect_new(
						definition->item.material_effects.index,
						_material_effect_impact,
						collision.material_type,
						&collision.point,
						&collision.plane.n,
						&collision.location,
						impact_scale);
				}

				if (definition->item.collision_sound.index != NONE)
				{
					struct sound_location location;

					location.position = new_position;
					location.forward = collision.plane.n;
					location.translational_velocity = *global_zero_vector3d;
					location.game_location = item->object.location;
					unattached_impulse_sound_new(
						definition->item.collision_sound.index,
						&location,
						impact_scale);
				}

				if ((collision.type == _collision_result_structure ||
					(collision.type == _collision_result_object &&
					TEST_FLAG(_object_item_rest_type_mask, object_get_type(collision.object_index)))) &&
					collision.plane.n.k > 0.7071f &&
					-dot_product3d(&collision.plane.n, &new_velocity) < 0.05f)
				{
					new_position = collision.point;
					item_align_to_normal_and_point(
						item_index,
						&collision.plane.n,
						&collision.point,
						&new_position);
					new_velocity.k = 0.f;
					new_velocity.j = 0.f;
					new_velocity.i = 0.f;
					scale_vector3d(
						&collision.plane.n,
						dot_product3d(&collision.plane.n, &item->object.angular_velocity),
						&item->object.angular_velocity);

					if (!game_engine_running() &&
						item->object.owner_player_index == NONE)
					{
						object_set_garbage(item_index, TRUE);
					}

					SET_FLAG(item->object.flags, _object_at_rest_bit, TRUE);

					switch (collision.type)
					{
					case _collision_result_structure:
						SET_FLAG(item->item.flags, _item_on_structure_bit, TRUE);
						item->item.rested_surface_index = (short)collision.surface_index;
						item->item.bsp_index = global_structure_bsp_index_get();
						break;
					case _collision_result_object:
					{
						real_matrix4x3 const *node_matrix;

						SET_FLAG(item->item.flags, _item_on_object_bit, TRUE);
						item->item.item_on_rest_object_index = collision.object_index;
						node_matrix = object_get_node_matrix(item->item.item_on_rest_object_index, 0);
						matrix4x3_inverse_transform_point(
							node_matrix,
							&collision.point,
							&item->item.item_rest_object_offset);
						break;
					}
					default:
						match_assert("c:\\halo\\SOURCE\\items\\items.c", 283, !"unreachable");
						break;
					}

					item->item.rotation_axis = collision.plane.n;
					item_adjust_for_angular_velocity_change(item_index);
					item->item.ignore_object_index = NONE;
				}
				else
				{
					real bounce = -dot_product3d(&new_velocity, &collision.plane.n) * 1.4f;

					if (collision.type != _collision_result_structure)
					{
						bounce = MIN(1.5f, bounce);
					}

					vector_from_line3d(&new_velocity, &collision.plane.n, bounce, &new_velocity);
					new_position = collision.point;

					if (collision_test_point(_collision_test_item_flags, &new_position, item_index))
					{
						point_from_line3d(&collision.point, &collision.plane.n, 0.05f, &new_position);
					}

					collision_test_point(_collision_test_item_flags, &new_position, item_index);
					verify_item_location(item_index);
				}
			}

			item->object.translational_velocity = new_velocity;
			object_translate(item_index, &new_position, &collision.location);
			verify_item_location(item_index);
		}
		else if (!TEST_FLAG(definition->item.flags, _item_antigravity_bit))
		{
			struct object_marker marker;
			real_point3d position;

			if (object_get_marker_by_name(item_index, "ground point", &marker, 1))
			{
				point_from_line3d(&marker.matrix.position, &item->item.rotation_axis, 0.125f, &position);
			}
			else
			{
				position = item->object.position;
			}

			if (TEST_FLAG(item->item.flags, _item_on_structure_bit) &&
				item->item.rested_surface_index != NONE &&
				item->item.bsp_index == global_structure_bsp_index_get())
			{
				struct collision_bsp *collision_bsp = global_collision_bsp_get();
				struct collision_surface const *surface = TAG_BLOCK_GET_ELEMENT(
					&collision_bsp->surfaces,
					item->item.rested_surface_index,
					struct collision_surface);

				if (TEST_FLAG(surface->flags, _collision_surface_breakable_bit) &&
					!breakable_surface_extant(surface->breakable_surface_index))
				{
					real_vector3d gravity;

					scale_vector3d(global_down3d, global_gravity, &gravity);
					SET_FLAG(item->item.flags, _item_on_structure_bit, FALSE);
					item->item.rested_surface_index = NONE;
					item_accelerate(item_index, &gravity, FALSE);
				}
			}
			else if (TEST_FLAG(item->item.flags, _item_on_object_bit))
			{
				if (object_try_and_get(item->item.item_on_rest_object_index))
				{
					real_point3d origin;
					real_matrix4x3 const *node_matrix = object_get_node_matrix(item->item.item_on_rest_object_index, 0);

					matrix4x3_transform_point(node_matrix, &item->item.item_rest_object_offset, &origin);
					item_align_to_normal_and_point(
						item_index,
						&item->item.rotation_axis,
						&origin,
						NULL);
				}
				else
				{
					real_vector3d gravity;

					scale_vector3d(global_down3d, global_gravity, &gravity);
					SET_FLAG(item->item.flags, _item_on_object_bit, FALSE);
					item_accelerate(item_index, &gravity, FALSE);
				}
			}

			scale_vector3d(&item->object.angular_velocity, 0.9f, &item->object.angular_velocity);
			item_adjust_for_angular_velocity_change(item_index);
		}

		if (TEST_FLAG(item->item.flags, _item_has_nonzero_angular_velocity_bit))
		{
			struct object_marker marker;
			real_matrix4x3 ground_point_matrix;
			real_vector3d left;

			if (TEST_FLAG(item->object.flags, _object_at_rest_bit) &&
				object_get_marker_by_name(item_index, "ground point", &marker, 1))
			{
				ground_point_matrix = marker.matrix;
				rotate_vector_about_axis(
					&ground_point_matrix.forward,
					&item->item.rotation_axis,
					item->item.rotation_sine,
					item->item.rotation_cosine);
				rotate_vector_about_axis(
					&ground_point_matrix.up,
					&item->item.rotation_axis,
					item->item.rotation_sine,
					item->item.rotation_cosine);
				cross_product3d(
					&ground_point_matrix.up,
					&ground_point_matrix.forward,
					&ground_point_matrix.left);
				cross_product3d(
					&ground_point_matrix.left,
					&ground_point_matrix.up,
					&ground_point_matrix.forward);
				normalize3d(&ground_point_matrix.forward);
				normalize3d(&ground_point_matrix.left);
				normalize3d(&ground_point_matrix.up);
				object_align_marker_to_matrix(object_get(item_index), &marker, &ground_point_matrix);
			}
			else
			{
				rotate_vector_about_axis(
					&item->object.forward,
					&item->item.rotation_axis,
					item->item.rotation_sine,
					item->item.rotation_cosine);
				rotate_vector_about_axis(
					&item->object.up,
					&item->item.rotation_axis,
					item->item.rotation_sine,
					item->item.rotation_cosine);
				verify_item_location(item_index);
			}

			normalize3d(&item->object.up);
			cross_product3d(&item->object.up, &item->object.forward, &left);
			cross_product3d(&left, &item->object.up, &item->object.forward);
			normalize3d(&item->object.forward);
		}

		verify_item_location(item_index);
	}

	if (item->item.detonation_ticks > 0)
	{
		if (--item->item.detonation_ticks == 0)
		{
			effect_new_from_object(
				definition->item.detonation_effect.index,
				item_index,
				item_index,
				NONE,
				0.f,
				0.f,
				NULL,
				NULL);
			object_delete(item_index);
		}
	}

	if (TEST_FLAG(item->item.flags, _item_in_unit_inventory_bit))
	{
		item->item.last_owned_time = game_time_get();
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\items\\items.c", 468);
	profile_exit(item_update_section);

	return TRUE;
}

boolean dangerous_items_near_player(
	void)
{
	struct object_iterator iterator;
	struct item_datum *item;

	object_iterator_new(&iterator, _object_mask_item, 1);

	while (item = (struct item_datum *)object_iterator_next(&iterator))
	{
		if (item->item.detonation_ticks > 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void item_in_unit_inventory(
	long item_index,
	long owner_unit_index)
{
	struct item_datum *item = item_get(item_index);

	if (owner_unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(owner_unit_index);

		SET_FLAG(item->item.flags, _item_in_unit_inventory_bit, TRUE);
		SET_FLAG(item->item.flags, _item_belongs_to_player_bit, unit->unit.player_index != NONE);
		item->object.owner_player_index = unit->unit.player_index;
		object_set_garbage(item_index, FALSE);
		SET_FLAG(item->item.flags, _item_on_structure_bit, FALSE);
		SET_FLAG(item->item.flags, _item_does_not_accelerate_bit, FALSE);
		item->object.location.leaf_index = NONE;
		item->object.location.cluster_index = NONE;
		scenario_location_award_bonus(&item->object.location);
	}
	else
	{
		SET_FLAG(item->item.flags, _item_in_unit_inventory_bit, FALSE);
		SET_FLAG(item->item.flags, _item_belongs_to_player_bit, FALSE);
	}

	return;
}

void item_get_position_even_if_in_inventory(
	long item_index,
	real_point3d *position)
{
	struct item_datum *item = item_try_and_get(item_index);

	position->x = 0.f;
	position->y = 0.f;
	position->z = 0.f;

	if (item)
	{
		if (TEST_FLAG(item->item.flags, _item_in_unit_inventory_bit))
		{
			if (item->object.owner_player_index != NONE)
			{
				struct player_datum *player = player_get(item->object.owner_player_index);

				if (player->unit_index != NONE)
				{
					*position = unit_get(player->unit_index)->object.bounding_sphere_center;
				}
			}
		}
		else
		{
			*position = item->object.bounding_sphere_center;
		}
	}

	return;
}

void item_accelerate(
	long item_index,
	real_vector3d const *acceleration,
	boolean detonates_explosives)
{
	struct item_datum *item = item_get(item_index);
	struct item_definition const *definition = item_definition_get(item->definition_index);

	if (!TEST_FLAG(item->item.flags, _item_does_not_accelerate_bit))
	{
		match_collision_log_begin_user("c:\\halo\\SOURCE\\items\\items.c", 536, _collision_user_items);

		if (item->object.parent_object_index == NONE)
		{
			struct object_marker marker;

			if (detonates_explosives &&
				!game_engine_running() &&
				TEST_FLAG(definition->item.flags, _item_destroyed_by_explosions_bit))
			{
				item_detonate(item_index);
			}

			if (TEST_FLAG(item->item.flags, _item_on_structure_bit))
			{
				if (!(magnitude_squared3d(acceleration) < _real_epsilon))
				{
					if (object_get_marker_by_name(item_index, "ground point", &marker, 1))
					{
						real_plane3d collision_plane;
						real_point3d new_point;
						struct collision_bsp *collision_bsp = global_collision_bsp_get();
						struct collision_surface const *surface = TAG_BLOCK_GET_ELEMENT(
							&collision_bsp->surfaces,
							item->item.rested_surface_index,
							struct collision_surface);

						bsp3d_get_plane_from_designator(
							&collision_bsp->bsp3d,
							surface->plane_designator,
							&collision_plane);
						point_from_line3d(
							&marker.matrix.position,
							&collision_plane.n,
							0.05f - (dot_product3d((real_vector3d const *)&marker.matrix.position, &collision_plane.n) - collision_plane.d),
							&new_point);
						object_translate(item_index, &new_point, NULL);
					}

					SET_FLAG(item->object.flags, _object_at_rest_bit, FALSE);
					SET_FLAG(item->item.flags, _item_on_structure_bit, FALSE);
				}
			}
			else
			{
				SET_FLAG(item->object.flags, _object_at_rest_bit, FALSE);
			}

			add_vectors3d(
				&item->object.translational_velocity,
				acceleration,
				&item->object.translational_velocity);

			if (item->item.ignore_object_index == NONE &&
				TEST_FLAG(item->item.flags, _item_on_structure_bit) &&
				magnitude_squared3d(acceleration) < _real_epsilon)
			{
				real_vector3d angular_acceleration;
				real_vector3d up;

				if (object_get_marker_by_name(item_index, "ground point", &marker, 1))
				{
					up = marker.matrix.up;
				}
				else
				{
					up = *global_up3d;
				}

				scale_vector3d(&up, real_random_range(-_pi2, _pi2), &angular_acceleration);
				add_vectors3d(
					&item->object.angular_velocity,
					&angular_acceleration,
					&item->object.angular_velocity);
			}
			else
			{
				real_vector3d random_angular_velocity;
				real normal_magnitude;
				real magnitude = magnitude3d(acceleration);

				if (magnitude < _real_epsilon)
				{
					magnitude = real_random();
				}

				cross_product3d(global_up3d, acceleration, &random_angular_velocity);
				normal_magnitude = normalize3d(&random_angular_velocity);

				if (!(normal_magnitude > 0.f))
				{
					random_direction3d(&random_angular_velocity);
				}

				scale_vector3d(
					&random_angular_velocity,
					real_random() * _pi2 * magnitude,
					&random_angular_velocity);
				add_vectors3d(
					&item->object.angular_velocity,
					&random_angular_velocity,
					&item->object.angular_velocity);
				verify_item_location(item_index);
			}

			item_adjust_for_angular_velocity_change(item_index);
			verify_item_location(item_index);
			object_set_garbage(item_index, FALSE);
		}

		match_collision_log_end_user("c:\\halo\\SOURCE\\items\\items.c", 651);
	}

	return;
}

void item_detonate(
	long item_index)
{
	struct item_datum *item = item_get(item_index);
	struct item_definition const *definition = item_definition_get(item->definition_index);

	if (!item->item.detonation_ticks)
	{
		effect_new_from_object(
			definition->item.detonating_effect.index,
			item_index,
			item_index,
			NONE,
			0.f,
			0.f,
			NULL,
			NULL);
		item->item.detonation_ticks = (short)(real_random_range(
			definition->item.detonation_delay_timer_lower_bound,
			definition->item.detonation_delay_timer_upper_bound) * TICKS_PER_SECOND);
	}

	return;
}

static void item_adjust_for_angular_velocity_change(
	long object_index)
{
	struct item_datum *item = item_get(object_index);
	real angular_speed = magnitude3d(&item->object.angular_velocity);

	if (angular_speed != 0.f)
	{
		SET_FLAG(item->item.flags, _item_has_nonzero_angular_velocity_bit, TRUE);

		if (!TEST_FLAG(item->object.flags, _object_at_rest_bit))
		{
			scale_vector3d(&item->object.angular_velocity, 1.f / angular_speed, &item->item.rotation_axis);
		}

		item->item.rotation_sine = sine(angular_speed);
		item->item.rotation_cosine = cosine(angular_speed);
	}
	else
	{
		SET_FLAG(item->item.flags, _item_has_nonzero_angular_velocity_bit, FALSE);
		item->item.rotation_sine = 0.f;
		item->item.rotation_cosine = 1.f;
	}

	return;
}

static void item_align_to_normal_and_point(
	long item_index,
	real_vector3d const *normal,
	real_point3d const *position,
	real_point3d *new_position)
{
	struct object_marker marker;
	struct item_datum *item = item_get(item_index);

	match_assert("c:\\halo\\SOURCE\\items\\items.c", 713, normal);

	if (object_get_marker_by_name(item_index, "ground point", &marker, 1))
	{
		real_matrix4x3 ground_point_matrix;
		real_vector3d forward;
		real scale;

		if (!position)
		{
			position = &marker.matrix.position;
		}

		if (!new_position)
		{
			new_position = &marker.matrix.position;
		}

		scale = square_root((dot_product3d(normal, &marker.matrix.up) + 1.f) * 2.f);

		if (scale > 0.01)
		{
			real_quaternion q;

			match_assert("c:\\halo\\SOURCE\\items\\items.c", 732, scale>0);
			cross_product3d(&marker.matrix.up, normal, &q.v);
			scale_vector3d(&q.v, 1.f / scale, &q.v);
			q.w = scale / 2.f;
			quaternion_transform_point(
				&q,
				(real_point3d const *)&marker.matrix.forward,
				(real_point3d *)&forward);
		}
		else
		{
			real_vector3d left;

			cross_product3d(normal, &marker.matrix.forward, &left);
			cross_product3d(&left, normal, &forward);
		}

		normalize3d(&forward);
		matrix4x3_from_point_and_vectors(&ground_point_matrix, position, &forward, normal);
		match_assert("c:\\halo\\SOURCE\\items\\items.c", 755, valid_real_matrix4x3(&ground_point_matrix));
		object_align_marker_to_matrix(object_get(item_index), &marker, &ground_point_matrix);
		*new_position = item->object.position;
	}

	return;
}
