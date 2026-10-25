/*
STRUCTURE_RUNTIME_DECALS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "structures.h"
#include "objects.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "game_state.h"
#include "collisions.h"
#include "decals.h"

/* ---------- constants */

/* ---------- macros */

#define DECAL_YAW_SCALE ((2.f * _pi) / (2 * CHAR_MAX))						/* fake name */
#define DECAL_PITCH_SCALE (_pi / (2 * CHAR_MAX))							/* fake name */

/* ---------- structures */

struct structure_decals_globals_definition
{
	boolean reset_decals;
	byte pad[3];
};

/* ---------- prototypes */

/* ---------- globals */

static struct structure_decals_globals_definition *structure_decals_globals;

/* ---------- public code */

void structure_decals_initialize(
	void)
{
	structure_decals_globals = game_state_malloc("structure decals", NULL, sizeof(*structure_decals_globals));
	match_assert("c:\\halo\\SOURCE\\structures\\structure_runtime_decals.c", 28, structure_decals_globals);

	return;
}

void structure_decals_initialize_for_new_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structure_runtime_decals.c", 36, structure_decals_globals);
	structure_decals_globals->reset_decals = FALSE;

	return;
}

void structure_decals_reconnect_to_structure_bsp(
	void)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structure_runtime_decals.c", 45, structure_decals_globals);
	structure_decals_globals->reset_decals = TRUE;

	return;
}

void structure_decals_disconnect_from_structure_bsp(
	void)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	if (structure_bsp->runtime_decals.count)
	{
		short cluster_index;
		short cluster_count = (short)structure_bsp->clusters.count;

		for (cluster_index = 0; cluster_index < cluster_count; cluster_index++)
		{
			struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);
			boolean cluster_has_decals = cluster->first_runtime_decal_index != NONE && cluster->runtime_decal_count;

			if (cluster_has_decals)
			{
				decals_delete_permanent_from_cluster(cluster_index);
			}
		}
	}

	return;
}

void structure_decals_dispose_from_old_map(
	void)
{
	return;
}

void structure_decals_dispose(
	void)
{
	return;
}

void structure_decals_update(
	unsigned long *old_combined_pvs,
	unsigned long *new_combined_pvs,
	short cluster_count)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	match_assert("c:\\halo\\SOURCE\\structures\\structure_runtime_decals.c", 98, structure_decals_globals);

	if (structure_bsp->runtime_decals.count)
	{
		short cluster_index;

		for (cluster_index = 0; cluster_index < cluster_count; cluster_index++)
		{
			struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);
			boolean cluster_has_decals = cluster->first_runtime_decal_index != NONE && cluster->runtime_decal_count;
			boolean delete_decals = cluster_has_decals &&
				!structure_decals_globals->reset_decals &&
				BIT_VECTOR_TEST_FLAG(old_combined_pvs, cluster_index) &&
				!BIT_VECTOR_TEST_FLAG(new_combined_pvs, cluster_index);
			boolean create_decals = cluster_has_decals &&
				(!BIT_VECTOR_TEST_FLAG(old_combined_pvs, cluster_index) || structure_decals_globals->reset_decals) &&
				BIT_VECTOR_TEST_FLAG(new_combined_pvs, cluster_index);

			if (delete_decals)
			{
				decals_delete_permanent_from_cluster(cluster_index);
			}
			else if (create_decals)
			{
				long decal_index;

				for (decal_index = 0; decal_index < cluster->runtime_decal_count; decal_index++)
				{
					struct structure_runtime_decal *decal = TAG_BLOCK_GET_ELEMENT(
						&structure_bsp->runtime_decals,
						cluster->first_runtime_decal_index + decal_index,
						struct structure_runtime_decal);
					long definition_index = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->decal_palette,
						decal->palette_index,
						struct scenario_decal_palette_entry)->reference.index;
					struct decal_definition *definition = decal_definition_get(definition_index);

					{
						real_vector3d direction;
						real_euler_angles2d angles;

						angles.yaw = decal->yaw * DECAL_YAW_SCALE;
						angles.pitch = decal->pitch * DECAL_PITCH_SCALE;
						vector3d_from_euler_angles2d(&direction, &angles);
						decal_new(definition_index, &decal->position, &direction, 1.f, TRUE, NONE, NULL);
					}
				}
			}
		}
	}

	structure_decals_globals->reset_decals = FALSE;

	return;
}

/* ---------- private code */
