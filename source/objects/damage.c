/*
DAMAGE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "damage.h"
#include "object_types.h"
#include "damage_effect_definitions.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "bungie_net/network/transport.h"
#include "collisions.h"
#include "items.h"
#include "vehicles.h"
#include "bipeds.h"
#include "structures.h"
#include "physics_constants.h"
#include "hud.h"
#include "effects.h"
#include "network_server_message_handler.h"
#include "devices.h"
#include "projectiles.h"
#include "player_effects.h"
#include "object_lists.h"
#include "collision_model_definitions.h"
#include "collision_usage.h"
#include "cheats.h"
#include "game.h"
#include "game_engine.h"
#include "game_statistics.h"
#include "ai.h"
#include "units.h"
#include "unit_definitions.h"
#include "input.h"
#include "draw_string.h"
#include "rasterizer.h"
#include "breakable_surfaces.h"
#include "periodic_functions.h"

/* ---------- constants */

enum
{
	_damage_effect_marker_normal = 0, /* fake name */
	_damage_effect_marker_incident, /* fake name */
	_damage_effect_marker_negative_incident, /* fake name */
	_damage_effect_marker_reflection, /* fake name */
	_damage_effect_marker_gravity, /* fake name */
	NUMBER_OF_DAMAGE_EFFECT_MARKERS, /* fake name */
};

enum
{
	/* fake name */
	_damage_area_of_effect_collision_flags =
		FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_structure_bit) | _collision_test_objects_sight_blocking_flags,
};

#define MAXIMUM_SHIELD_OVERCHARGE 3.f /* fake name */
#define SHIELD_OVERCHARGE_RATE (1.f / TICKS_PER_SECOND) /* fake name */
#define SHIELD_OVERCHARGE_DECAY_RATE ((MAXIMUM_SHIELD_OVERCHARGE - 1.f) / (90 * TICKS_PER_SECOND)) /* fake name */
#define DAMAGE_DECAY_RATE (0.5f / TICKS_PER_SECOND) /* fake name */
#define RECENT_DAMAGE_DECAY_DELAY (2 * TICKS_PER_SECOND) /* fake name */
#define DAMAGE_ACCELERATION_UPWARD_BIAS 0.45f /* fake name */

/* ---------- prototypes */

static void object_destroy_notify_children(long object_index);
static void area_of_effect_cause_damage_to_object(struct damage_data *damage_data, long object_index, boolean damage_next_object);
static long get_player_index_from_object_or_parents(long object_index);
static void object_damage_body(long object_index, short region_index, short node_index, real_vector3d const *object_normal, struct damage_resistance const *damage_resistance, struct damage_material const *damage_material, struct damage_definition const *damage_definition, struct damage_data *damage_data, unsigned long *being_damaged_flags, real *body_damage_reference, real *body_damage_multiplier_reference, real total_damage);
static void object_damage_shield(long object_index, struct damage_resistance const *damage_resistance, struct damage_material const *damage_material, struct damage_definition const *damage_definition, struct damage_data *damage_data, unsigned long *being_damaged_flags, real *shield_damage_reference, real *total_damage_reference);
static void object_damage_aftermath(long object_index, struct damage_data *damage_data, unsigned long being_damaged_flags, real shield_damage, real body_damage, real body_damage_multiplier, short body_part);
static void damage_effect_new_on_object(long effect_definition_index, long object_index);
static void damage_effect_new_at_location(long effect_definition_index, long object_index, short node_index, real_point3d const *position, real_vector3d const *direction, real_vector3d const *normal);
static void object_destroy_region(long object_index, short region_index);
static void object_permutation_shield_regions(long object_index, boolean active);

/* ---------- globals */

boolean debug_damage;

static long damage_debug_object_index; /* fake name */

/* ---------- public code */

void damage_initialize(
	void)
{
	return;
}

void damage_dispose(
	void)
{
	return;
}

void damage_initialize_for_new_map(
	void)
{
	damage_debug_object_index = NONE;

	return;
}

void damage_dispose_from_old_map(
	void)
{
	return;
}

void damage_render_debug(
	void)
{
	return;
}

void object_initialize_vitality(
	long object_index,
	real *custom_body_vitality,
	real *custom_shield_vitality)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *definition = object_definition_get(object->definition_index);
	real maximum_body_vitality = 0.f;
	real maximum_shield_vitality = 0.f;

	if (definition->object.collision_model.index != NONE)
	{
		struct collision_model *collision_model = collision_model_definition_get(definition->object.collision_model.index);

		if (collision_model)
		{
			maximum_body_vitality = collision_model->resistance.maximum_body_vitality;
			maximum_shield_vitality = collision_model->resistance.maximum_shield_vitality;
		}
	}

	if (custom_body_vitality)
	{
		maximum_body_vitality = *custom_body_vitality;
	}

	if (custom_shield_vitality)
	{
		maximum_shield_vitality = *custom_shield_vitality;
	}

	object->object.maximum_body_vitality = maximum_body_vitality;
	object->object.maximum_shield_vitality = maximum_shield_vitality;
	object->object.body_vitality = maximum_body_vitality > 0.f ? 1.f : 0.f;
	object->object.shield_vitality = maximum_shield_vitality > 0.f ? 1.f : 0.f;

	return;
}

real object_get_actual_body_vitality(
	long object_index,
	boolean ignore_difficulty)
{
	real body_vitality = object_get(object_index)->object.body_vitality;
	real result = object_get_maximum_body_vitality(object_index, ignore_difficulty) * body_vitality;

	return result;
}

real object_get_actual_shield_vitality(
	long object_index,
	boolean ignore_difficulty)
{
	real shield_vitality = object_get(object_index)->object.shield_vitality;
	real result = object_get_maximum_shield_vitality(object_index, ignore_difficulty) * shield_vitality;

	return result;
}

real object_get_maximum_body_vitality(
	long object_index,
	boolean ignore_difficulty)
{
	struct object_datum *object = object_get(object_index);
	real result = object->object.maximum_body_vitality;

	if (!ignore_difficulty)
	{
		result *= game_difficulty_get_team_value(_game_difficulty_enemy_vitality_scale, object->object.owner_team_index);
	}

	return result;
}

real object_get_maximum_shield_vitality(
	long object_index,
	boolean ignore_difficulty)
{
	struct object_datum *object = object_get(object_index);
	real result = object->object.maximum_shield_vitality;

	if (!ignore_difficulty)
	{
		result *= game_difficulty_get_team_value(_game_difficulty_enemy_shield_scale, object->object.owner_team_index);
	}

	return result;
}

void object_damage_update(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (object_definition->object.collision_model.index != NONE)
	{
		struct collision_model *collision_model = collision_model_definition_get(object_definition->object.collision_model.index);

		if (collision_model)
		{
			if (TEST_FLAG(object->object.damage_flags, _object_die_act_of_god_no_statistics_bit) ||
				TEST_FLAG(object->object.damage_flags, _object_die_act_of_god_bit) ||
				TEST_FLAG(object->object.damage_flags, _object_die_act_of_god_silent_bit))
			{
				if (!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
				{
					struct game_globals_falling_damage *falling_damage = TAG_BLOCK_GET_ELEMENT(
						&scenario_get_game_globals()->falling_damage,
						0,
						struct game_globals_falling_damage);

					if (falling_damage->falling_damage.index != NONE)
					{
						struct damage_data damage_data;

						damage_data_new(&damage_data, falling_damage->falling_damage.index);
						damage_data.scale = 1.f;
						SET_FLAG(damage_data.flags, _damage_kill_instantly_bit, TRUE);

						if (TEST_FLAG(object->object.damage_flags, _object_die_act_of_god_silent_bit))
						{
							SET_FLAG(damage_data.flags, _damage_silent_bit, TRUE);
						}

						if (TEST_FLAG(object->object.damage_flags, _object_die_act_of_god_no_statistics_bit))
						{
							SET_FLAG(damage_data.flags, _damage_no_statistics_bit, TRUE);
						}

						object_cause_damage(&damage_data, object_index, NONE, NONE, NONE, NULL);
					}
				}

				SET_FLAG(object->object.damage_flags, _object_die_act_of_god_bit, FALSE);
				SET_FLAG(object->object.damage_flags, _object_die_act_of_god_silent_bit, FALSE);
				SET_FLAG(object->object.damage_flags, _object_die_act_of_god_no_statistics_bit, FALSE);
			}

			SET_FLAG(object->object.damage_flags, _object_shield_charging_bit, FALSE);

			if (object->object.maximum_shield_vitality > 0.f &&
				!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
			{
				if (TEST_FLAG(object->object.damage_flags, _object_shield_over_charging_bit))
				{
					object->object.shield_vitality += SHIELD_OVERCHARGE_RATE;

					if (object->object.shield_vitality >= MAXIMUM_SHIELD_OVERCHARGE)
					{
						object->object.shield_vitality = MAXIMUM_SHIELD_OVERCHARGE;
						SET_FLAG(object->object.damage_flags, _object_shield_over_charging_bit, FALSE);
					}
					else
					{
						SET_FLAG(object->object.damage_flags, _object_shield_charging_bit, TRUE);
					}
				}
				else if (object->object.shield_vitality > 1.f && game_engine_running())
				{
					long player_index = player_index_from_unit_index(object_index);
					real overcharge = object->object.shield_vitality - 1.f;

					if (SHIELD_OVERCHARGE_DECAY_RATE > overcharge)
					{
						object->object.shield_vitality = 1.f;
						hud_tick_shield(player_index, overcharge);
					}
					else
					{
						object->object.shield_vitality -= SHIELD_OVERCHARGE_DECAY_RATE;
						hud_tick_shield(player_index, SHIELD_OVERCHARGE_DECAY_RATE);
					}
				}
				else if (object->object.shield_vitality < 1.f)
				{
					if (object->object.shield_stun_ticks == 0)
					{
						real shield_recharge = collision_model->resistance.runtime_shield_recharge_velocity;

						shield_recharge *= game_difficulty_get_team_value(
							_game_difficulty_enemy_recharge_scale,
							object->object.owner_team_index);

						if (TEST_FLAG(object->object.damage_flags, _object_shield_depleted_bit))
						{
							damage_effect_new_on_object(collision_model->resistance.shield_recharging_effect.index, object_index);
							SET_FLAG(object->object.damage_flags, _object_shield_depleted_bit, FALSE);
							object_permutation_shield_regions(object_index, TRUE);
						}

						SET_FLAG(object->object.damage_flags, _object_shield_charging_bit, TRUE);
						object->object.shield_vitality += shield_recharge;

						if (object->object.shield_vitality > 1.f)
						{
							object->object.shield_vitality = 1.f;
							SET_FLAG(object->object.damage_flags, _object_shield_charging_bit, FALSE);
						}
					}
					else
					{
						object->object.shield_stun_ticks--;
					}
				}
			}

			if (object->object.body_damage_decay_timer != NONE)
			{
				object->object.body_damage_decay_timer++;

				if (object->object.body_damage_decay_timer >= 0)
				{
					object->object.current_body_damage -= DAMAGE_DECAY_RATE;
				}

				if (object->object.body_damage_decay_timer >= RECENT_DAMAGE_DECAY_DELAY)
				{
					object->object.recent_body_damage -= DAMAGE_DECAY_RATE;
				}

				object->object.current_body_damage = MAX(0.f, object->object.current_body_damage);
				object->object.recent_body_damage = MAX(0.f, object->object.recent_body_damage);

				if (object->object.current_body_damage == 0.f && object->object.recent_body_damage == 0.f)
				{
					object->object.body_damage_decay_timer = NONE;
				}
			}

			if (object->object.shield_damage_decay_timer != NONE)
			{
				object->object.shield_damage_decay_timer++;

				if (object->object.shield_damage_decay_timer >= 0)
				{
					object->object.current_shield_damage -= DAMAGE_DECAY_RATE;
				}

				if (object->object.shield_damage_decay_timer >= RECENT_DAMAGE_DECAY_DELAY)
				{
					object->object.recent_shield_damage -= DAMAGE_DECAY_RATE;
				}

				object->object.current_shield_damage = MAX(0.f, object->object.current_shield_damage);
				object->object.recent_shield_damage = MAX(0.f, object->object.recent_shield_damage);

				if (object->object.current_shield_damage == 0.f && object->object.recent_shield_damage == 0.f)
				{
					object->object.shield_damage_decay_timer = NONE;
				}
			}
		}
	}

	return;
}

void damage_data_new(
	struct damage_data *damage_data,
	long definition_index)
{
	memset(damage_data, 0, sizeof(*damage_data));
	damage_data->definition_index = definition_index;
	damage_data->material_type = NONE;
	damage_data->owner_player_index = NONE;
	damage_data->owner_object_index = NONE;
	damage_data->owner_team_index = NONE;
	damage_data->location.cluster_index = NONE;
	damage_data->scale = 1.f;
	damage_data->multiplier = 1.f;

	return;
}

boolean object_restore_body(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	boolean restored = FALSE;

	if (!TEST_FLAG(object->object.damage_flags, _object_dead_bit) &&
		object->object.body_vitality < 1.f)
	{
		object->object.body_vitality = 1.f;
		restored = TRUE;
	}

	return restored;
}

void object_deplete_body(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	word damage_flags = object->object.damage_flags;

	if (!TEST_FLAG(damage_flags, _object_dead_bit))
	{
		struct object_definition *definition;

		SET_FLAG(damage_flags, _object_dead_bit, TRUE);
		object->object.damage_flags = damage_flags;

		definition = object_definition_get(object->definition_index);

		if (definition->object.collision_model.index != NONE)
		{
			struct collision_model *collision_model = collision_model_definition_get(definition->object.collision_model.index);

			damage_effect_new_on_object(collision_model->resistance.body_depleted_effect.index, object_index);
		}

		if (object->object.type == _object_type_vehicle)
		{
			long unit_index = object->object.first_child_object_index;

			while (unit_index != NONE)
			{
				struct unit_datum *unit = (struct unit_datum *)object_get(unit_index);

				if (unit->object.type == _object_type_biped &&
					(unit->unit.player_index == NONE || !cheat.deathless_player) &&
					unit->unit.parent_seat_index != NONE)
				{
					unit_kill(unit_index);
				}

				unit_index = unit->object.next_object_index;
			}
		}

		object_deplete_shield(object_index);
	}

	return;
}

void object_deplete_shield(
	long object_index)
{
	struct object_datum *object = object_get(object_index);

	if (!TEST_FLAG(object->object.damage_flags, _object_shield_depleted_bit))
	{
		struct object_definition *definition = object_definition_get(object->definition_index);

		if (definition->object.collision_model.index != NONE)
		{
			struct collision_model *collision_model = collision_model_definition_get(definition->object.collision_model.index);

			damage_effect_new_on_object(collision_model->resistance.shield_depleted_effect.index, object_index);
		}

		object->object.current_shield_damage = 0.f;
		SET_FLAG(object->object.damage_flags, _object_shield_depleted_bit, TRUE);
		object_permutation_shield_regions(object_index, FALSE);
	}

	return;
}

boolean object_double_charge_shield(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	boolean charged = object->object.shield_vitality <= 1.f;

	if (charged)
	{
		SET_FLAG(object->object.damage_flags, _object_shield_over_charging_bit, TRUE);

		if (object->object.shield_vitality == 0.f)
		{
			object->object.shield_vitality = 0.01f;
		}

		object->object.shield_stun_ticks = 0;
	}

	return charged;
}

static void object_destroy_notify_children(
	long object_index)
{
	long child_object_index = object_get(object_index)->object.first_child_object_index;

	while (child_object_index != NONE)
	{
		long next_object_index = object_get(child_object_index)->object.next_object_index;

		if (!object_type_handle_parent_destroyed(child_object_index))
		{
			object_destroy_notify_children(child_object_index);
		}

		child_object_index = next_object_index;
	}

	return;
}

void object_destroy(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *definition = object_definition_get(object->definition_index);

	object_deplete_body(object_index);

	if (definition->object.collision_model.index != NONE)
	{
		struct collision_model *collision_model = collision_model_definition_get(definition->object.collision_model.index);

		damage_effect_new_on_object(collision_model->resistance.body_destroyed_effect.index, object_index);
	}

	object_destroy_notify_children(object_index);
	object_delete(object_index);

	return;
}

void area_of_effect_cause_damage(
	struct damage_data *damage_data,
	long unlucky_object_index)
{
	long object_indices[64];
	short object_number;
	struct damage_effect_definition *definition = damage_effect_definition_get(damage_data->definition_index);
	short object_count = objects_in_sphere(
		0,
		0,
		&damage_data->location,
		&damage_data->origin,
		definition->cutoff_radius,
		object_indices,
		NUMBEROF(object_indices));

	for (object_number = 0; object_number < object_count; object_number++)
	{
		area_of_effect_cause_damage_to_object(damage_data, object_indices[object_number], FALSE);
	}

	breakable_surface_damage_area_of_effect(damage_data);

	return;
}

static void area_of_effect_cause_damage_to_object(
	struct damage_data *damage_data,
	long object_index,
	boolean damage_next_object)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	struct damage_effect_definition *damage_effect = damage_effect_definition_get(damage_data->definition_index);
	boolean can_damage = !TEST_FLAG(object->object.flags, _object_invisible_bit);
	boolean did_damage = FALSE;
	boolean infection_form = FALSE;

	match_collision_log_begin_user("c:\\halo\\SOURCE\\objects\\damage.c", 601, _collision_user_area_damage);

	if (can_damage &&
		TEST_FLAG(_object_mask_unit, object->object.type) &&
		damage_effect->damage.area_of_effect_core_radius > _real_epsilon)
	{
		real_vector3d x_axis;
		real_vector3d y_axis;
		real_vector3d axis;
		struct collision_result collision;
		short ray_index;
		boolean blocked = TRUE;

		vector_from_points3d(&damage_data->epicenter, &object->object.bounding_sphere_center, &axis);
		normalize3d(perpendicular3d(&axis, &x_axis));
		normalize3d(cross_product3d(&axis, &x_axis, &y_axis));

		for (ray_index = 0; ray_index < 4; ray_index++)
		{
			real_vector3d offset;
			real_point3d offset_point;

			switch (ray_index)
			{
			case 0:
				scale_vector3d(&x_axis, damage_effect->damage.area_of_effect_core_radius, &offset);
				break;
			case 1:
				scale_vector3d(&x_axis, -damage_effect->damage.area_of_effect_core_radius, &offset);
				break;
			case 2:
				scale_vector3d(&y_axis, damage_effect->damage.area_of_effect_core_radius, &offset);
				break;
			case 3:
				scale_vector3d(&y_axis, -damage_effect->damage.area_of_effect_core_radius, &offset);
				break;
			}

			collision_test_vector(
				_damage_area_of_effect_collision_flags,
				&damage_data->epicenter,
				&offset,
				object_get_ultimate_parent(object_index),
				&collision);
			offset_point = collision.point;

			if (!collision_test_line(
				_damage_area_of_effect_collision_flags,
				&offset_point,
				&object->object.bounding_sphere_center,
				object_get_ultimate_parent(object_index),
				&collision))
			{
				blocked = FALSE;
			}
		}

		if (blocked)
		{
			can_damage = FALSE;
		}
	}
	else
	{
		struct collision_result collision;

		if (collision_test_line(
			_damage_area_of_effect_collision_flags,
			&damage_data->epicenter,
			&object->object.bounding_sphere_center,
			object_get_ultimate_parent(object_index),
			&collision))
		{
			can_damage = FALSE;
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\objects\\damage.c", 660);

	if (TEST_FLAG(damage_effect->damage.flags, _damage_does_not_hurt_owner_bit) &&
		object_index == damage_data->owner_object_index)
	{
		can_damage = FALSE;
	}

	if (TEST_FLAG(damage_effect->damage.flags, _damage_does_not_hurt_friends_bit) &&
		!game_team_is_enemy(object->object.owner_team_index, damage_data->owner_team_index))
	{
		can_damage = FALSE;
	}

	if (can_damage && TEST_FLAG(damage_effect->damage.flags, _damage_infection_form_pop_bit))
	{
		can_damage = FALSE;

		if (TEST_FLAG(_object_mask_unit, object->object.type) &&
			TEST_FLAG(unit_definition_get(object->definition_index)->unit.flags, _unit_is_inconsequential_bit) &&
			object_index != damage_data->owner_object_index)
		{
			real infection_form_toughness = game_difficulty_get_value(_game_difficulty_infection_form_toughness);

			can_damage = TRUE;

			if ((infection_form_toughness > 0.f || TEST_FLAG(damage_effect->damage.flags, _damage_does_not_hurt_infection_forms_bit)) &&
				TEST_FLAG(damage_data->flags, _damage_damaged_one_object_bit))
			{
				can_damage = FALSE;
			}

			if (infection_form_toughness > 0.f && real_random() < infection_form_toughness * 0.25f)
			{
				can_damage = FALSE;
			}

			infection_form = TRUE;
		}
	}

	SET_FLAG(damage_data->flags, _damage_area_of_effect_bit, TRUE);

	if (can_damage)
	{
		real distance;
		real scale;
		real falloff_range;
		real falloff_scale;

		vector_from_points3d(&damage_data->epicenter, &object->object.bounding_sphere_center, &damage_data->direction);
		distance = normalize3d(&damage_data->direction);
		falloff_range = damage_effect->cutoff_radius - damage_effect->falloff_radius;
		falloff_scale = 1.f - (distance - damage_effect->falloff_radius) / falloff_range;
		scale = falloff_range > 0.f ? falloff_scale : 1.f;
		scale = PIN(scale, 0.f, 1.f);

		if (!TEST_FLAG(damage_effect->flags, _damage_effect_dont_scale_damage_by_distance_bit))
		{
			damage_data->scale = scale;
		}

		if (scale > 0.f)
		{
			object_cause_damage(damage_data, object_index, NONE, NONE, NONE, NULL);
			did_damage = TRUE;
		}

		if (object_definition->object.collision_model.index != NONE)
		{
			struct collision_model *collision_model = collision_model_definition_get(object_definition->object.collision_model.index);

			if (TEST_FLAG(collision_model->resistance.flags, _damage_resistance_children_take_area_damage_bit) &&
				object->object.first_child_object_index != NONE)
			{
				area_of_effect_cause_damage_to_object(damage_data, object->object.first_child_object_index, TRUE);
			}
		}
	}

	if (infection_form && (!can_damage || did_damage))
	{
		SET_FLAG(damage_data->flags, _damage_damaged_one_object_bit, TRUE);
	}

	if (damage_next_object && object->object.next_object_index != NONE)
	{
		area_of_effect_cause_damage_to_object(damage_data, object->object.next_object_index, TRUE);
	}

	return;
}

static long get_player_index_from_object_or_parents(
	long object_index)
{
	long player_index = NONE;

	while (object_index != NONE)
	{
		if (unit_try_and_get(object_index))
		{
			player_index = player_index_from_unit_index(object_index);
			break;
		}

		object_index = object_get(object_index)->object.parent_object_index;
	}

	return player_index;
}

void object_cause_damage(
	struct damage_data *damage_data,
	long object_index,
	short node_index,
	short region_index,
	short material_index,
	real_vector3d const *object_normal)
{
	static struct damage_material default_damage_material;
	boolean caused_damage;
	real total_damage;
	short damaged_object_count;
	long damaged_object_indices[16];
	struct damage_effect_definition *damage_effect = damage_effect_definition_get(damage_data->definition_index);
	struct damage_definition *damage_definition = &damage_effect->damage;
	boolean damage_adjusted_for_difficulty = FALSE;
	boolean allow_parent_body_damage_shielding = TRUE;

	match_assert("c:\\halo\\SOURCE\\objects\\damage.c", 824, region_index==NONE || (region_index>=0 && region_index<MAXIMUM_REGIONS_PER_OBJECT));

	if (damage_data->owner_player_index != NONE)
	{
		damage_debug_object_index = object_index;
	}

	total_damage = damage_data->multiplier * ((1.f - damage_data->scale) * damage_definition->damage_minimum +
		damage_data->scale * real_random_range(damage_definition->damage_lower_bound, damage_definition->damage_upper_bound));

	if (damage_data->owner_object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(damage_data->owner_object_index);

		if (unit)
		{
			long actor_index;

			if (unit->unit.gunner_object_index != NONE)
			{
				unit = unit_get(unit->unit.gunner_object_index);
			}

			actor_index = unit->unit.swarm_actor_index != NONE ? unit->unit.swarm_actor_index : unit->unit.actor_index;

			if (actor_index != NONE)
			{
				ai_adjust_damage(actor_index, damage_data, &total_damage);
			}
		}
	}

	if (game_engine_running())
	{
		total_damage *= game_engine_get_damage_multiplier(
			get_player_index_from_object_or_parents(damage_data->owner_object_index),
			get_player_index_from_object_or_parents(object_index));
	}
	else if (damage_data->owner_team_index != NONE &&
		game_team_is_enemy(damage_data->owner_team_index, _game_team_player))
	{
		total_damage *= game_difficulty_get_value(_game_difficulty_enemy_damage_scale);
		damage_adjusted_for_difficulty = TRUE;
	}

	damaged_object_count = 0;
	caused_damage = FALSE;

	if (TEST_FLAG(damage_data->flags, _damage_area_of_effect_bit) ||
		TEST_FLAG(damage_data->flags, _damage_kill_instantly_bit))
	{
		damaged_object_indices[damaged_object_count++] = object_index;
	}
	else
	{
		long damaged_object_index = object_index;

		while (damaged_object_index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\objects\\damage.c", 894, damaged_object_count<sizeof(damaged_object_indices)/sizeof(long));
			damaged_object_indices[damaged_object_count++] = damaged_object_index;
			damaged_object_index = object_get(damaged_object_index)->object.parent_object_index;
		}
	}

	{
		struct object_datum *object = object_get(object_index);
		struct object_definition *object_definition = object_definition_get(object->definition_index);

		if (object_definition->object.collision_model.index != NONE)
		{
			struct collision_model *collision_model = collision_model_definition_get(object_definition->object.collision_model.index);

			allow_parent_body_damage_shielding = !TEST_FLAG(collision_model->resistance.flags, _damage_resistance_parent_never_takes_body_damage_for_us_bit);
		}

		if (object->object.umbrella_shield_object_index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\objects\\damage.c", 919, damaged_object_count<sizeof(damaged_object_indices)/sizeof(long));
			damaged_object_indices[damaged_object_count++] = object->object.umbrella_shield_object_index;
		}

		if (!TEST_FLAG(damage_data->flags, _damage_area_of_effect_bit) &&
			object->object.type == _object_type_vehicle)
		{
			struct unit_datum *vehicle = unit_get(object_index);
			struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
			long child_object_index = vehicle->object.first_child_object_index;

			damage_data->multiplier = (1.f - damage_definition->vehicle_passthrough_penalty) * vehicle_definition->unit.child_damage_fraction;

			while (child_object_index != NONE)
			{
				struct object_datum *child_object = object_get(child_object_index);

				match_assert("c:\\halo\\SOURCE\\objects\\damage.c", 939, damaged_object_count<sizeof(damaged_object_indices)/sizeof(long));

				if (child_object->object.type == _object_type_biped)
				{
					long player_index = unit_get(child_object_index)->unit.player_index;

					if (player_index != NONE || child_object_index == vehicle->unit.driver_object_index)
					{
						SET_FLAG(damage_data->flags, _damage_bypasses_shields_bit, player_index == NONE);
						object_cause_damage(damage_data, child_object_index, NONE, NONE, NONE, NULL);
						SET_FLAG(damage_data->flags, _damage_bypasses_shields_bit, FALSE);
					}
				}

				child_object_index = child_object->object.next_object_index;
			}

			damage_data->multiplier = 1.f;
		}
	}

	{
		short damaged_object_index;

		for (damaged_object_index = 0; damaged_object_index < damaged_object_count; damaged_object_index++)
		{
			struct unit_datum *unit = unit_try_and_get(damaged_object_indices[damaged_object_index]);

			if (unit)
			{
				if (unit->unit.player_index != NONE)
				{
					player_effect_start(
						unit->unit.player_index,
						damage_data,
						&damage_data->direction,
						damage_data->scale,
						total_damage);
				}
				else if (cheat.reflexive_damage_effects)
				{
					player_effect_start(
						local_player_get_player_index(0),
						damage_data,
						&damage_data->direction,
						damage_data->scale,
						total_damage);
				}
			}
		}
	}

	while (total_damage > 0.f && damaged_object_count-- > 0)
	{
		long current_object_index = damaged_object_indices[damaged_object_count];
		struct object_datum *current_object = object_get(current_object_index);
		struct object_definition *current_object_definition = object_definition_get(current_object->definition_index);
		real shield_damage = 0.f;
		real body_damage = 0.f;
		real body_damage_multiplier = 0.f;
		unsigned long being_damaged_flags = 0;
		short body_part = NONE;

		if (current_object_definition->object.collision_model.index != NONE)
		{
			struct damage_material *damage_material;
			struct collision_model *collision_model = collision_model_definition_get(current_object_definition->object.collision_model.index);
			boolean kill_instantly = TEST_FLAG(damage_data->flags, _damage_kill_instantly_bit);

			if (node_index >= 0 && node_index < collision_model->nodes.count)
			{
				body_part = TAG_BLOCK_GET_ELEMENT(
					&collision_model->nodes,
					node_index,
					struct collision_node)->runtime_damage_part;
			}

			if (damage_adjusted_for_difficulty)
			{
				SET_FLAG(being_damaged_flags, _object_being_damaged_multiplied_by_difficulty_bit, TRUE);
			}

			if (damage_data->owner_team_index != NONE &&
				!game_team_is_enemy(current_object->object.owner_team_index, damage_data->owner_team_index))
			{
				SET_FLAG(being_damaged_flags, _object_being_damaged_by_friendly_bit, TRUE);
			}

			if (damaged_object_count == 0 &&
				material_index >= 0 &&
				material_index < collision_model->resistance.materials.count)
			{
				damage_material = TAG_BLOCK_GET_ELEMENT(
					&collision_model->resistance.materials,
					material_index,
					struct damage_material);
			}
			else if (collision_model->resistance.indirect_damage_material_index >= 0 &&
				collision_model->resistance.indirect_damage_material_index < collision_model->resistance.materials.count)
			{
				damage_material = TAG_BLOCK_GET_ELEMENT(
					&collision_model->resistance.materials,
					collision_model->resistance.indirect_damage_material_index,
					struct damage_material);
			}
			else
			{
				damage_material = &default_damage_material;
			}

			damage_data->material_type = damage_material->type;

			if (cheat.omnipotent && damage_data->owner_player_index != NONE)
			{
				kill_instantly = TRUE;
			}

			if (damage_definition->side_effect == _damage_side_effect_lethal_to_the_unsuspecting &&
				unit_unsuspecting(current_object_index, &damage_data->epicenter) &&
				!TEST_FLAG(current_object->object.damage_flags, _object_cannot_take_damage_bit))
			{
				kill_instantly = TRUE;
			}

			if (kill_instantly && !TEST_FLAG(current_object->object.damage_flags, _object_dead_bit))
			{
				current_object->object.body_vitality = 0.f;
				object_deplete_body(current_object_index);
				SET_FLAG(being_damaged_flags, _object_being_damaged_body_depleted_bit, TRUE);
				SET_FLAG(being_damaged_flags, _object_being_damaged_killed_instantly_bit, TRUE);
			}

			if (!TEST_FLAG(damage_data->flags, _damage_bypasses_shields_bit) &&
				!TEST_FLAG(damage_definition->flags, _damage_skips_shields_bit) &&
				current_object->object.maximum_shield_vitality > 0.f &&
				(damaged_object_count == 0 || TEST_FLAG(collision_model->resistance.flags, _damage_resistance_takes_shield_damage_for_children_bit)))
			{
				object_damage_shield(
					current_object_index,
					&collision_model->resistance,
					damage_material,
					damage_definition,
					damage_data,
					&being_damaged_flags,
					&shield_damage,
					&total_damage);
			}

			if ((damaged_object_count == 0 ||
				(allow_parent_body_damage_shielding && TEST_FLAG(collision_model->resistance.flags, _damage_resistance_takes_body_damage_for_children_bit))) &&
				!TEST_FLAG(damage_definition->flags, _damage_only_hurts_shields_bit))
			{
				if (TEST_FLAG(collision_model->resistance.flags, _damage_resistance_only_hurt_by_explosives_bit) &&
					!TEST_FLAG(damage_definition->flags, _damage_detonates_explosives_bit))
				{
					total_damage = 0.f;
				}

				object_damage_body(
					current_object_index,
					damaged_object_count == 0 ? region_index : NONE,
					damaged_object_count == 0 ? node_index : NONE,
					damaged_object_count == 0 ? object_normal : NULL,
					&collision_model->resistance,
					damage_material,
					damage_definition,
					damage_data,
					&being_damaged_flags,
					&body_damage,
					&body_damage_multiplier,
					total_damage);
				damaged_object_count = 0;
			}

			if (!caused_damage && (shield_damage > _real_epsilon || body_damage > _real_epsilon))
			{
				if (shield_damage > body_damage)
				{
					damage_data->material_type = collision_model->resistance.shield_material_type;
					damage_data->material_effect_scale = current_object->object.shield_vitality;
				}
				else
				{
					damage_data->material_effect_scale = PIN(current_object->object.body_vitality, 0.f, 1.f);
				}

				if (debug_damage && current_object_index == damage_debug_object_index)
				{
					console_printf(
						FALSE,
						"%s: \"%s\" \"%s\" k=%0.2f S[%3.2f] B[%3.2f]",
						strrchr(tag_get_name(damage_data->definition_index), '\\') + 1,
						material_get_name(damage_material->type),
						damage_material->name,
						damage_data->scale,
						shield_damage,
						body_damage);
				}

				caused_damage = TRUE;
			}
		}

		object_damage_aftermath(
			current_object_index,
			damage_data,
			being_damaged_flags,
			shield_damage,
			body_damage,
			body_damage_multiplier,
			body_part);

		if (TEST_FLAG(being_damaged_flags, _object_being_damaged_body_destroyed_bit))
		{
			object_delete(current_object_index);
		}
	}

	return;
}

void object_can_take_damage(
	long object_list_index)
{
	long reference_index;
	long object_index = object_list_get_first(object_list_index, &reference_index);

	while (object_index != NONE)
	{
		struct object_datum *object = object_get(object_index);

		SET_FLAG(object->object.damage_flags, _object_cannot_take_damage_bit, FALSE);
		object_index = object_list_get_next(object_list_index, &reference_index);
	}

	return;
}

void object_cannot_take_damage(
	long object_list_index)
{
	long reference_index;
	long object_index = object_list_get_first(object_list_index, &reference_index);

	while (object_index != NONE)
	{
		struct object_datum *object = object_get(object_index);

		SET_FLAG(object->object.damage_flags, _object_cannot_take_damage_bit, TRUE);
		object_index = object_list_get_next(object_list_index, &reference_index);
	}

	return;
}

void object_set_ranged_attack_inhibited(
	long object_index,
	boolean inhibited)
{
	if (object_index != NONE)
	{
		struct object_datum *object = object_get(object_index);

		SET_FLAG(object->object.damage_flags, _object_ranged_attack_inhibited_bit, inhibited);
	}

	return;
}

void object_set_melee_attack_inhibited(
	long object_index,
	boolean inhibited)
{
	if (object_index != NONE)
	{
		struct object_datum *object = object_get(object_index);

		SET_FLAG(object->object.damage_flags, _object_melee_attack_inhibited_bit, inhibited);
	}

	return;
}

/* ---------- private code */

static void object_damage_body(
	long object_index,
	short region_index,
	short node_index,
	real_vector3d const *object_normal,
	struct damage_resistance const *damage_resistance,
	struct damage_material const *damage_material,
	struct damage_definition const *damage_definition,
	struct damage_data *damage_data,
	unsigned long *being_damaged_flags,
	real *body_damage_reference,
	real *body_damage_multiplier_reference,
	real total_damage)
{
	real maximum_body_vitality;
	real inverse_maximum_body_vitality;
	real actual_body_damage;
	struct object_datum *object = object_get(object_index);
	real body_damage = damage_material->body_damage_multiplier * total_damage;
	boolean ignore_difficulty = FALSE;

	if (TEST_FLAG(damage_resistance->flags, _damage_resistance_only_hurt_while_occupied_bit) &&
		object->object.type == _object_type_vehicle &&
		unit_get(object_index)->unit.driver_object_index == NONE)
	{
		body_damage = 0.f;
	}

	if (!game_engine_running() &&
		damage_definition->category == _damage_category_falling &&
		object->object.owner_team_index == _game_team_player)
	{
		ignore_difficulty = TRUE;
	}

	maximum_body_vitality = object_get_maximum_body_vitality(object_index, ignore_difficulty);
	inverse_maximum_body_vitality = maximum_body_vitality > 0.f ? 1.f / maximum_body_vitality : 0.f;
	actual_body_damage = body_damage;

	if (TEST_FLAG(*being_damaged_flags, _object_being_damaged_by_friendly_bit))
	{
		actual_body_damage = (1.f - damage_resistance->friendly_damage_resistance) * body_damage;

		if (TEST_FLAG(*being_damaged_flags, _object_being_damaged_multiplied_by_difficulty_bit))
		{
			real difficulty_scale = game_difficulty_get_value(_game_difficulty_enemy_damage_scale);

			if (difficulty_scale > 0.f)
			{
				actual_body_damage /= difficulty_scale;
			}
		}
	}

	actual_body_damage *= inverse_maximum_body_vitality;
	match_vassert("c:\\halo\\SOURCE\\objects\\damage.c", 1295, damage_material->type>=0 && damage_material->type<NUMBER_OF_MATERIAL_TYPES, "damage_material->type>=0 && damage_material->type<NUMBER_OF_MATERIAL_TYPES");
	actual_body_damage *= damage_definition->material_modifiers[damage_material->type];

	if (!TEST_FLAG(object->object.damage_flags, _object_cannot_take_damage_bit))
	{
		if (body_damage > 0.f && TEST_FLAG(damage_material->flags, _damage_material_head_bit))
		{
			if (TEST_FLAG(damage_definition->flags, _damage_can_cause_headshots_bit))
			{
				if (game_engine_running() ||
					object->object.type != _object_type_biped ||
					unit_get(object_index)->unit.player_index == NONE)
				{
					object->object.body_vitality = 0.f;
					SET_FLAG(*being_damaged_flags, _object_being_damaged_killed_instantly_bit, TRUE);

					if (game_engine_running())
					{
						SET_FLAG(*being_damaged_flags, _object_being_damaged_force_hard_ping_bit, TRUE);
					}
				}
			}
			else if (TEST_FLAG(damage_definition->flags, _damage_can_cause_multiplayer_headshots_bit) && game_engine_running())
			{
				actual_body_damage *= 2.f;

				if (actual_body_damage > object->object.body_vitality)
				{
					SET_FLAG(*being_damaged_flags, _object_being_damaged_force_hard_ping_bit, TRUE);
				}
			}
		}

		object->object.body_vitality -= actual_body_damage;
	}

	if (region_index != NONE && !TEST_FLAG(object->object.regions_destroyed_flags, region_index))
	{
		struct damage_region *region = TAG_BLOCK_GET_ELEMENT(&damage_resistance->regions, region_index, struct damage_region);

		object->object.region_damage[region_index] = (byte)(actual_body_damage * 255.f + object->object.region_damage[region_index]);

		if (region->damage_threshold > 0.f && object->object.region_damage[region_index] / 255.f > region->damage_threshold)
		{
			object_destroy_region(object_index, region_index);
			SET_FLAG(*being_damaged_flags, _object_being_damaged_region_destroyed_bit, TRUE);
		}
	}

	object->object.body_damage_decay_timer = 0;
	object->object.current_body_damage += actual_body_damage;
	object->object.recent_body_damage += actual_body_damage;

	if (object->object.current_body_damage > 1.f)
	{
		object->object.current_body_damage = 1.f;
	}

	if (object->object.recent_body_damage > 1.f)
	{
		object->object.recent_body_damage = 1.f;
	}

	if (cheat.deathless_player &&
		object->object.body_vitality < 0.f &&
		TEST_FLAG(_object_mask_unit, object->object.type))
	{
		boolean player_controlled = unit_get(object_index)->unit.player_index != NONE;

		if (!player_controlled && object->object.type == _object_type_vehicle)
		{
			long child_object_index = object->object.first_child_object_index;

			while (child_object_index != NONE)
			{
				struct unit_datum *child_unit = (struct unit_datum *)object_get(child_object_index);

				if (TEST_FLAG(_object_mask_unit, child_unit->object.type) && child_unit->unit.player_index != NONE)
				{
					player_controlled = TRUE;
					break;
				}

				child_object_index = child_unit->object.next_object_index;
			}
		}

		if (player_controlled)
		{
			object->object.body_vitality = 0.f;
		}
	}

	{
		real body_vitality = object_get_actual_body_vitality(object_index, FALSE);

		if (damage_resistance->body_destroyed_threshold < 0.f &&
			body_vitality < damage_resistance->body_destroyed_threshold)
		{
			object_destroy(object_index);
			SET_FLAG(*being_damaged_flags, _object_being_damaged_body_depleted_bit, TRUE);
			SET_FLAG(*being_damaged_flags, _object_being_damaged_body_destroyed_bit, TRUE);
		}
		else if (body_vitality < 0.f)
		{
			if (!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
			{
				short damage_region_index;

				for (damage_region_index = 0; damage_region_index < damage_resistance->regions.count; damage_region_index++)
				{
					struct damage_region *region = TAG_BLOCK_GET_ELEMENT(
						&damage_resistance->regions,
						damage_region_index,
						struct damage_region);

					if (TEST_FLAG(region->flags, _object_region_dies_when_object_dies_bit))
					{
						object_destroy_region(object_index, damage_region_index);
					}
				}

				object_deplete_body(object_index);
				SET_FLAG(*being_damaged_flags, _object_being_damaged_body_depleted_bit, TRUE);
			}
		}
		else if (body_vitality < damage_resistance->body_damaged_effect_threshold &&
			!TEST_FLAG(object->object.damage_flags, _object_passed_body_damage_threshold_bit))
		{
			damage_effect_new_on_object(damage_resistance->body_damaged_effect.index, object_index);
			SET_FLAG(object->object.damage_flags, _object_passed_body_damage_threshold_bit, TRUE);
		}
	}

	if (TEST_FLAG(damage_data->flags, _damage_create_localized_effect_bit) &&
		damage_resistance->localized_damage_effect.index != NONE)
	{
		damage_effect_new_at_location(
			damage_resistance->localized_damage_effect.index,
			object_index,
			node_index,
			&damage_data->epicenter,
			&damage_data->direction,
			object_normal);
	}

	if (TEST_FLAG(damage_data->flags, _damage_area_of_effect_bit) &&
		body_damage > damage_resistance->area_damage_effect_threshold &&
		damage_resistance->area_damage_effect.index != NONE &&
		damage_definition->category != _damage_category_flame)
	{
		damage_effect_new_on_object(damage_resistance->area_damage_effect.index, object_index);
	}

	*body_damage_reference = body_damage;
	*body_damage_multiplier_reference = damage_material->body_damage_multiplier;

	return;
}

static void object_damage_shield(
	long object_index,
	struct damage_resistance const *damage_resistance,
	struct damage_material const *damage_material,
	struct damage_definition const *damage_definition,
	struct damage_data *damage_data,
	unsigned long *being_damaged_flags,
	real *shield_damage_reference,
	real *total_damage_reference)
{
	struct object_datum *object = object_get(object_index);
	real total_damage = *total_damage_reference;
	real shield_damage = total_damage;
	boolean negligible_damage = FALSE;
	boolean ignore_difficulty = FALSE;

	if (!game_engine_running() &&
		damage_definition->category == _damage_category_falling &&
		object->object.owner_team_index == _game_team_player)
	{
		ignore_difficulty = TRUE;
	}

	if (object->object.shield_vitality > 0.f)
	{
		real maximum_shield_vitality = object_get_maximum_shield_vitality(object_index, ignore_difficulty);
		real inverse_maximum_shield_vitality = maximum_shield_vitality > 0.f ? 1.f / maximum_shield_vitality : 0.f;

		if (!TEST_FLAG(*being_damaged_flags, _object_being_damaged_by_friendly_bit) ||
			!TEST_FLAG(damage_resistance->flags, _damage_resistance_always_shields_friendly_damage_bit))
		{
			shield_damage = (1.f - damage_material->shield_leak_fraction) * total_damage;

			if (object->object.shield_vitality <= damage_resistance->shield_failure_threshold &&
				damage_resistance->shield_failure_threshold > 0.f)
			{
				real shield_failure = transition_function_evaluate(
					damage_resistance->shield_failure_function,
					object->object.shield_vitality / damage_resistance->shield_failure_threshold);

				shield_damage *= (1.f - damage_resistance->maximum_shield_failure) * shield_failure + damage_resistance->maximum_shield_failure;
			}
		}

		if (TEST_FLAG(object->object.damage_flags, _object_shield_over_charging_bit))
		{
			shield_damage = total_damage;
			total_damage = 0.f;
		}
		else
		{
			real actual_shield_damage;
			real normalized_shield_damage;

			if (shield_damage < 0.f)
			{
				shield_damage = 0.f;
			}

			total_damage -= shield_damage;

			if (TEST_FLAG(*being_damaged_flags, _object_being_damaged_by_friendly_bit) &&
				TEST_FLAG(*being_damaged_flags, _object_being_damaged_multiplied_by_difficulty_bit))
			{
				real difficulty_scale = game_difficulty_get_value(_game_difficulty_enemy_damage_scale);

				if (difficulty_scale > 0.f)
				{
					shield_damage /= difficulty_scale;
				}
			}

			actual_shield_damage = damage_material->shield_damage_multiplier * shield_damage;
			match_vassert("c:\\halo\\SOURCE\\objects\\damage.c", 1550, damage_resistance->shield_material_type>=0 && damage_resistance->shield_material_type<NUMBER_OF_MATERIAL_TYPES, "damage_resistance->shield_material_type>=0 && damage_resistance->shield_material_type<NUMBER_OF_MATERIAL_TYPES");
			actual_shield_damage *= damage_definition->material_modifiers[damage_resistance->shield_material_type];

			if (actual_shield_damage < _real_epsilon)
			{
				negligible_damage = TRUE;
			}

			normalized_shield_damage = actual_shield_damage * inverse_maximum_shield_vitality;

			if (normalized_shield_damage > object->object.shield_vitality ||
				damage_definition->side_effect == _damage_side_effect_emp)
			{
				real excess_damage = actual_shield_damage - maximum_shield_vitality * object->object.shield_vitality;

				if (excess_damage > 0.f)
				{
					total_damage += excess_damage;
				}

				object->object.shield_vitality = 0.f;

				if (!TEST_FLAG(object->object.damage_flags, _object_shield_depleted_bit))
				{
					object_deplete_shield(object_index);
					SET_FLAG(*being_damaged_flags, _object_being_damaged_shield_depleted_bit, TRUE);
				}
			}
			else
			{
				if (!TEST_FLAG(object->object.damage_flags, _object_cannot_take_damage_bit))
				{
					object->object.shield_vitality -= normalized_shield_damage;
				}

				if (!TEST_FLAG(object->object.damage_flags, _object_passed_shield_damage_threshold_bit) &&
					object->object.shield_vitality < damage_resistance->shield_damaged_effect_threshold)
				{
					damage_effect_new_on_object(damage_resistance->shield_damaged_effect.index, object_index);
					SET_FLAG(object->object.damage_flags, _object_passed_shield_damage_threshold_bit, TRUE);
				}
			}
		}

		if (!negligible_damage)
		{
			real normalized_damage = (*total_damage_reference - total_damage) * inverse_maximum_shield_vitality;

			object->object.shield_damage_decay_timer = 0;

			if (!TEST_FLAG(object->object.damage_flags, _object_shield_depleted_bit))
			{
				object->object.current_shield_damage = 1.f;
			}

			object->object.recent_shield_damage += normalized_damage;

			if (object->object.current_shield_damage > 1.f)
			{
				object->object.current_shield_damage = 1.f;
			}

			if (object->object.recent_shield_damage > 1.f)
			{
				object->object.recent_shield_damage = 1.f;
			}
		}
	}
	else
	{
		shield_damage = 0.f;
		object->object.shield_vitality = 0.f;
	}

	if (shield_damage >= damage_resistance->minimum_shield_stun_damage ||
		object->object.shield_vitality == 0.f)
	{
		object->object.shield_stun_ticks = (short)(damage_resistance->shield_stun_time * TICKS_PER_SECOND);
	}

	*shield_damage_reference = shield_damage;
	*total_damage_reference = total_damage;

	return;
}

static void object_damage_aftermath(
	long object_index,
	struct damage_data *damage_data,
	unsigned long being_damaged_flags,
	real shield_damage,
	real body_damage,
	real body_damage_multiplier,
	short body_part)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	struct damage_effect_definition *damage_effect = damage_effect_definition_get(damage_data->definition_index);

	if (object_definition->object.acceleration_scale > _real_epsilon)
	{
		real_vector3d acceleration;
		real_vector3d adjusted_direction = damage_data->direction;

		adjusted_direction.k += DAMAGE_ACCELERATION_UPWARD_BIAS;
		normalize3d(&adjusted_direction);
		scale_vector3d(
			&adjusted_direction,
			damage_effect->damage.instantaneous_acceleration * object_definition->object.acceleration_scale / TICKS_PER_SECOND,
			&acceleration);

		switch (object->object.type)
		{
		case _object_type_projectile:
			projectile_accelerate(object_index, &acceleration);
			break;
		case _object_type_weapon:
		case _object_type_equipment:
		case _object_type_garbage:
			item_accelerate(
				object_index,
				&acceleration,
				damage_data->scale > 0.5f && TEST_FLAG(damage_effect->damage.flags, _damage_detonates_explosives_bit));
			break;
		case _object_type_biped:
		case _object_type_vehicle:
		{
			struct unit_datum *unit = (struct unit_datum *)object;

			if (damage_effect->damage.instantaneous_acceleration > _real_epsilon &&
				!TEST_FLAG(unit->unit.flags, _unit_impervious_bit))
			{
				if (object->object.type == _object_type_biped)
				{
					biped_accelerate(object_index, &acceleration);
				}
				else if (object->object.type == _object_type_vehicle)
				{
					if (TEST_FLAG(damage_effect->damage.flags, _damage_detonates_explosives_bit))
					{
						scale_vector3d(&acceleration, 2.f, &acceleration);
					}

					vehicle_accelerate(object_index, &acceleration);
				}
			}
			break;
		}
		}
	}

	if (game_engine_can_score() && !TEST_FLAG(damage_data->flags, _damage_no_statistics_bit))
	{
		game_statistics_record_damage(
			object_index,
			shield_damage + body_damage,
			damage_data->owner_player_index,
			damage_data->owner_object_index,
			damage_data->owner_team_index);

		if (TEST_FLAG(being_damaged_flags, _object_being_damaged_body_depleted_bit))
		{
			game_statistics_record_kill(
				object_index,
				damage_data->owner_player_index,
				damage_data->owner_object_index,
				damage_data->owner_team_index);
		}
	}
	else if (game_engine_can_score())
	{
		long player_index = player_index_from_unit_index(object_index);

		game_engine_player_killed(player_index, object_index, player_index, TRUE);
	}

	if (TEST_FLAG(_object_mask_unit, object->object.type))
	{
		unit_damage_aftermath(
			object_index,
			damage_data,
			being_damaged_flags,
			shield_damage,
			body_damage,
			body_damage_multiplier,
			body_part);
	}

	return;
}

static void damage_effect_new_on_object(
	long effect_definition_index,
	long object_index)
{
	effect_new_from_object(effect_definition_index, object_index, object_index, NONE, 0.f, 0.f, NULL, NULL);

	return;
}

static void damage_effect_new_at_location(
	long effect_definition_index,
	long object_index,
	short node_index,
	real_point3d const *position,
	real_vector3d const *direction,
	real_vector3d const *normal)
{
	real_vector3d effect_vectors[NUMBER_OF_DAMAGE_EFFECT_MARKERS];
	real_point3d effect_points[NUMBER_OF_DAMAGE_EFFECT_MARKERS];
	real_vector3d approximate_incident;
	short marker_index;
	char const *effect_marker_names[NUMBER_OF_DAMAGE_EFFECT_MARKERS] =
	{
		"normal",
		"incident",
		"negative incident",
		"reflection",
		"gravity"
	};

	effect_vectors[_damage_effect_marker_gravity] = *global_down3d;
	approximate_incident = *direction;

	if (normalize3d(&approximate_incident) == 0.f)
	{
		approximate_incident = *global_forward3d;
	}

	scale_vector3d(&approximate_incident, -1.f, &effect_vectors[_damage_effect_marker_incident]);
	effect_vectors[_damage_effect_marker_negative_incident] = approximate_incident;

	if (!normal)
	{
		real_point3d object_origin;
		real_vector3d approximate_normal;

		object_get_origin(object_index, &object_origin);
		vector_from_points3d(&object_origin, position, &approximate_normal);

		if (normalize3d(&approximate_normal) == 0.f)
		{
			approximate_normal = object_get(object_index)->object.forward;
		}

		effect_vectors[_damage_effect_marker_normal] = approximate_normal;
		reflect_vector3d(&approximate_incident, &approximate_normal, &effect_vectors[_damage_effect_marker_reflection]);
	}
	else
	{
		effect_vectors[_damage_effect_marker_normal] = *normal;
		reflect_vector3d(&approximate_incident, normal, &effect_vectors[_damage_effect_marker_reflection]);
	}

	for (marker_index = 0; marker_index < NUMBER_OF_DAMAGE_EFFECT_MARKERS; marker_index++)
	{
		effect_points[marker_index] = *position;
	}

	if (object_index != NONE && node_index != NONE)
	{
		effect_new_attached_from_markers(
			effect_definition_index,
			object_index,
			object_index,
			node_index,
			NUMBER_OF_DAMAGE_EFFECT_MARKERS,
			effect_marker_names,
			effect_points,
			effect_vectors,
			1.f,
			0.f,
			NULL,
			NULL);
	}
	else
	{
		effect_new_unattached_from_markers(
			effect_definition_index,
			object_index,
			global_zero_vector3d,
			NUMBER_OF_DAMAGE_EFFECT_MARKERS,
			effect_marker_names,
			effect_points,
			effect_vectors,
			1.f,
			0.f,
			NULL,
			NULL,
			FALSE);
	}

	return;
}

static void object_destroy_region(
	long object_index,
	short region_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (object_definition->object.collision_model.index != NONE)
	{
		struct collision_model *collision_model = collision_model_definition_get(object_definition->object.collision_model.index);

		match_assert("c:\\halo\\SOURCE\\objects\\damage.c", 1818, region_index>=0 && region_index<MAXIMUM_REGIONS_PER_OBJECT);

		if (!TEST_FLAG(object->object.regions_destroyed_flags, region_index))
		{
			struct damage_region *region = TAG_BLOCK_GET_ELEMENT(
				&collision_model->resistance.regions,
				region_index,
				struct damage_region);

			damage_effect_new_on_object(region->destroyed_effect.index, object_index);
			object_permute_region(object_index, "~damaged", region_index, TRUE);

			if (TEST_FLAG(region->flags, _object_region_inhibits_melee_attack_bit))
			{
				SET_FLAG(object->object.damage_flags, _object_melee_attack_inhibited_bit, TRUE);
			}

			if (TEST_FLAG(region->flags, _object_region_inhibits_ranged_attack_bit))
			{
				SET_FLAG(object->object.damage_flags, _object_ranged_attack_inhibited_bit, TRUE);
			}

			if (TEST_FLAG(region->flags, _object_region_inhibits_walking_bit))
			{
				SET_FLAG(object->object.damage_flags, _object_walking_inhibited_bit, TRUE);
			}

			if (TEST_FLAG(region->flags, _object_region_forces_drop_weapon_bit))
			{
				SET_FLAG(object->object.damage_flags, _object_cannot_hold_weapon_bit, TRUE);
			}

			if (TEST_FLAG(region->flags, _object_region_forces_object_to_die_bit))
			{
				object_deplete_body(object_index);
			}

			SET_FLAG(object->object.regions_destroyed_flags, region_index, TRUE);
			object_type_handle_region_destroyed(object_index, region_index, region->flags);
		}
	}

	return;
}

static void object_permutation_shield_regions(
	long object_index,
	boolean active)
{
	short region_index;
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	struct collision_model *collision_model = collision_model_definition_get(object_definition->object.collision_model.index);

	for (region_index = 0; region_index < collision_model->resistance.regions.count; region_index++)
	{
		struct damage_region *region = TAG_BLOCK_GET_ELEMENT(
			&collision_model->resistance.regions,
			region_index,
			struct damage_region);

		if (TEST_FLAG(region->flags, _object_region_missing_when_shield_is_zero_bit) &&
			region->permutations.count > 1)
		{
			object->object.region_permutations[region_index] = !active;
		}
	}

	return;
}

void render_debug_object_damage(
	void)
{
	if (debug_damage)
	{
		char textstring[2048];
		rectangle2d bounds = render.camera.window_bounds;

		bounds.x0 += 320;

		if (damage_debug_object_index == NONE)
		{
			_snprintf(textstring, sizeof(textstring), "no object to debug|n(point and press space)");
		}
		else
		{
			struct object_datum *object = object_try_and_get(damage_debug_object_index);

			if (object)
			{
				_snprintf(
					textstring,
					sizeof(textstring),
					"%s|nbody %0.3f|n  current %0.3f|n  recent %0.3f|nshield %0.3f|n  current %0.3f|n  recent %0.3f|n",
					strrchr(tag_get_name(object->definition_index), '\\'),
					object->object.body_vitality,
					object->object.current_body_damage,
					object->object.recent_body_damage,
					object->object.shield_vitality,
					object->object.current_shield_damage,
					object->object.recent_shield_damage);
			}
			else
			{
				damage_debug_object_index = NONE;
			}
		}

		draw_string_set_format(NONE, 0, 0);
		draw_string_set_color(global_real_argb_white);
		rasterizer_draw_string(&bounds, NULL, NULL, 0, textstring);

		if (input_key_is_down(_key_space))
		{
			struct collision_result collision;
			real_vector3d v;
			long unit_index = NONE;

			if (render.local_player_index != NONE)
			{
				unit_index = player_get(local_player_get_player_index(render.local_player_index))->unit_index;
			}

			scale_vector3d(&render.camera.forward, 50.f, &v);

			if (collision_test_vector(
				FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_objects_bit),
				&render.camera.position,
				&v,
				unit_index,
				&collision))
			{
				match_assert("c:\\halo\\SOURCE\\objects\\damage.c", 1940, collision.type==_collision_result_object);
				damage_debug_object_index = collision.object_index;
			}
		}
	}

	return;
}
