/*
MATERIAL_EFFECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "material_effects.h"
#include "effects.h"
#include "material_effect_definitions.h"
#include "objects.h"
#include "unit_definitions.h"
#include "network_game_globals.h"
#include "players.h"
#include "collisions.h"
#include "sound_manager.h"
#include "render_debug.h"
#include "game_sound.h"
#include "observer.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

boolean debug_material_effects = FALSE;

/* ---------- public code */

boolean material_effect_visible(
	real_point3d const *origin)
{
	real const maximum_distance = 10.f;
	boolean visible = FALSE;

	if (local_player_count() > 2)
	{
		visible = TRUE;
	}
	else
	{
		short local_player_index;

		for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
		{
			if (local_player_get_player_index(local_player_index) != NONE)
			{
				struct observer_result const *camera = observer_get_camera(local_player_index);

				if (distance_squared3d(&camera->position, origin) < maximum_distance * maximum_distance)
				{
					visible = TRUE;
					break;
				}
			}
		}
	}

	return visible;
}

void material_effect_new_from_point(
	long effects_definition_index,
	short effect_index,
	real_point3d const *origin,
	real scale)
{
	struct material_effects_definition const *definition = material_effects_definition_get(effects_definition_index);

	if (effect_index < definition->effects.count)
	{
		struct material_effect const *effect = TAG_BLOCK_GET_ELEMENT(&definition->effects, effect_index, struct material_effect);
		real const contact_point_cast_height = 0.15f;
		real_point3d contact_point_cast_origin;
		real_vector3d contact_point_cast;
		struct collision_result collision;

		contact_point_cast_origin = *origin;
		contact_point_cast_origin.z += contact_point_cast_height;
		scale_vector3d(global_down3d, 0.3f, &contact_point_cast);

		if (collision_test_vector(
			_collision_test_for_bipeds_passthrough_living_flags,
			&contact_point_cast_origin,
			&contact_point_cast,
			NONE,
			&collision))
		{
			short material_type = scenario_location_underwater(&collision.location, &collision.point, NULL) ? _material_water : collision.material_type;

			material_effect_new(
				effects_definition_index,
				effect_index,
				material_type,
				&collision.point,
				&collision.plane.n,
				&collision.location,
				scale);
		}
		else if (debug_material_effects)
		{
			render_debug_sphere(FALSE, origin, 0.05f, global_real_argb_red);
		}
	}

	return;
}

void material_effect_new(
	long effects_definition_index,
	short effect_index,
	short material_index,
	real_point3d const *origin,
	real_vector3d const *normal,
	struct location const *location,
	real scale)
{
	struct material_effects_definition const *definition = material_effects_definition_get(effects_definition_index);

	if (effect_index < definition->effects.count)
	{
		struct material_effect const *effect = TAG_BLOCK_GET_ELEMENT(&definition->effects, effect_index, struct material_effect);

		if (material_index != NONE && material_index < effect->materials.count)
		{
			struct material_effect_material const *material = TAG_BLOCK_GET_ELEMENT(&effect->materials, material_index, struct material_effect_material);
			real_point3d offset_origin;

			point_from_line3d(origin, normal, 0.01f, &offset_origin);

			if (material->effect.index != NONE)
			{
				effect_new_unattached_from_markers(
					material->effect.index,
					NONE,
					NULL,
					1,
					NULL,
					&offset_origin,
					normal,
					scale,
					0.f,
					NULL,
					NULL,
					FALSE);
			}

			if (material->sound.index != NONE)
			{
				struct sound_location sound_location;

				sound_location.position = offset_origin;
				sound_location.forward = *normal;
				sound_location.translational_velocity = *global_zero_vector3d;
				sound_location.game_location = *location;
				unattached_impulse_sound_new(material->sound.index, &sound_location, scale);
			}

			if (debug_material_effects)
			{
				render_debug_sphere(FALSE, origin, 0.05f, global_real_argb_cyan);
			}
		}
	}

	return;
}

/* ---------- private code */
