/*
CHEATS.C

symbols in this file:
00094C50 0020:
	_cheats_initialize (0000)
00094C70 0010:
	_cheats_dispose (0000)
00094C80 0010:
	_cheats_dispose_from_old_map (0000)
00094C90 00b0:
	_cheats_update (0000)
00094D40 0090:
	_cheats_load (0000)
00094DD0 0060:
	_cheat_active_camouflage_local_player (0000)
00094E30 0060:
	_cheat_player_index (0000)
00094E90 0010:
	_cheats_initialize_for_new_map (0000)
00094EA0 00b0:
	_cheat_teleport_to_camera (0000)
00094F50 0050:
	_cheat_active_camouflage (0000)
00094FA0 0150:
	_cheat_objects (0000)
000950F0 00d0:
	_cheat_all_weapons (0000)
000951C0 0050:
	_cheat_all_powerups (0000)
00095210 0050:
	_cheat_all_vehicles (0000)
0025AC64 003b:
	??_C@_0DL@LCJIJOPI@Cannot?5execute?5cheats?5attached?5t@ (0000)
0025ACA0 0005:
	??_C@_04JNJDPIIA@?$AN?6?7?$DL?$AA@ (0000)
0025ACA8 000e:
	??_C@_0O@KAFNGCBO@d?3?2cheats?4txt?$AA@ (0000)
0025ACB8 003a:
	??_C@_0DK@OACLODJP@Camera?5is?5outside?5BSP?4?4?4?5cannot?5@ (0000)
0025ACF4 001d:
	??_C@_0BN@DPPGFJCA@c?3?2halo?2SOURCE?2game?2cheats?4c?$AA@ (0000)
0025AD14 0004:
	__real@3ec90fdb (0000)
0043D808 0c81:
	_bss_0043d808 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cheats.h"
#include "players.h"
#include "network_game_globals.h"
#include "units.h"
#include "actor_definitions.h"
#include "console.h"
#include "vehicle_definitions.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "observer.h"
#include "terminal.h"
#include "equipment_definitions.h"
#include "input.h"
#include "director.h"
#include "hs.h"
#include "game_globals.h"
#include "scenario.h"
#include "objects.h"

/* ---------- constants */

enum
{
	MAXIMUM_CHEAT_STRING_LENGTH = 200, /* fake name */
	MAXIMUM_CHEAT_REFERENCES = 16 /* fake name */
};

/* ---------- macros */

#define CHEATS_FILE_PATH "d:\\cheats.txt" /* fake name */

/* ---------- structures */

/* ---------- prototypes */

static long cheat_player_index(void);
static void cheat_objects(struct game_globals_tag_reference const *references, short name_count);

/* ---------- globals */

struct cheat_globals cheat;

static char cheat_strings[NUMBER_OF_GAMEPAD_BUTTONS][MAXIMUM_CHEAT_STRING_LENGTH]; /* fake name */
static boolean cheats_enabled = FALSE; /* fake name */

/* ---------- public code */

void cheats_initialize(
	void)
{
	memset(cheat_strings, 0, sizeof(cheat_strings));

	return;
}

void cheats_dispose(
	void)
{
	return;
}

void cheats_initialize_for_new_map(
	void)
{
	cheats_load();

	return;
}

void cheats_dispose_from_old_map(
	void)
{
	return;
}

void cheats_update(
	void)
{
	if (cheat.controller_enabled)
	{
		short local_player_index;

		for (local_player_index = local_player_get_next(NONE); local_player_index != NONE; local_player_index = local_player_get_next(local_player_index))
		{
			struct gamepad_state const *gamepad = input_get_gamepad_state(local_player_index);

			if (gamepad && gamepad->buttons[_gamepad_binary_button_back] > 0)
			{
				short button_index;

				for (button_index = 0; button_index < NUMBER_OF_GAMEPAD_BUTTONS; button_index++)
				{
					if (cheat_strings[button_index][0] && gamepad->buttons[button_index] > 0)
					{
						director_inhibit_input(local_player_index);

						if (gamepad->buttons[button_index] == 1)
						{
							console_printf(FALSE, cheat_strings[button_index]);

							if (!hs_compile_and_evaluate(cheat_strings[button_index]))
							{
								cheat_strings[button_index][0] = '\0';
							}
						}
					}
				}
			}
		}
	}

	return;
}

void cheats_load(
	void)
{
	FILE *file = fopen(CHEATS_FILE_PATH, "r");

	if (file)
	{
		short cheat_index;

		for (cheat_index = 0; cheat_index < NUMBER_OF_GAMEPAD_BUTTONS; cheat_index++)
		{
			if (!fgets(cheat_strings[cheat_index], sizeof(cheat_strings[cheat_index]) - 1, file))
			{
				break;
			}

			strtok(cheat_strings[cheat_index], "\r\n\t;");

			if ((cheat_index == _gamepad_binary_button_back || cheat_index == _gamepad_binary_button_start) && cheat_strings[cheat_index][0])
			{
				cheat_strings[cheat_index][0] = '\0';
				error(_error_silent, "Cannot execute cheats attached to the back or start button");
			}
		}

		fclose(file);
	}

	return;
}

void cheat_all_weapons(
	void)
{
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_tag_reference *weapons = game_globals->weapon_list.count ? TAG_BLOCK_GET_ELEMENT(&game_globals->weapon_list, 0, struct game_globals_tag_reference) : NULL;

	if (weapons)
	{
		cheat_objects(game_globals->weapon_list.count ? TAG_BLOCK_GET_ELEMENT(&game_globals->weapon_list, 0, struct game_globals_tag_reference) : NULL, (short)game_globals->weapon_list.count);
	}
	else
	{
		long tag_index = NONE;
		short reference_count = 0;
		struct game_globals_tag_reference weapon_references[MAXIMUM_CHEAT_REFERENCES];
		struct tag_iterator iterator;

		tag_iterator_new(&iterator, WEAPON_DEFINITION_TAG);

		while ((tag_index = tag_iterator_next(&iterator)) != NONE && reference_count < NUMBEROF(weapon_references))
		{
			weapon_references[reference_count++].reference.index = tag_index;
		}

		cheat_objects(weapon_references, reference_count);
	}

	return;
}

void cheat_all_powerups(
	void)
{
	struct game_globals *game_globals = scenario_get_game_globals();

	cheat_objects(game_globals->cheat_powerups.count ? TAG_BLOCK_GET_ELEMENT(&game_globals->cheat_powerups, 0, struct game_globals_tag_reference) : NULL, (short)game_globals->cheat_powerups.count);

	return;
}

void cheat_all_vehicles(
	void)
{
	struct game_globals *game_globals = scenario_get_game_globals();

	if (game_globals->multiplayer_information.count)
	{
		cheat_objects(TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information)->vehicles.address, (short)TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information)->vehicles.count);
	}

	return;
}

void cheat_teleport_to_camera(
	void)
{
	long player_index = cheat_player_index();

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (player->local_player_index != NONE)
		{
			struct observer_result const *result = observer_get_camera(player->local_player_index);

			match_assert("c:\\halo\\SOURCE\\game\\cheats.c", 256, result);

			if (result->location.cluster_index != NONE)
			{
				long object_index = unit_get(player->unit_index)->object.parent_object_index;

				if (object_index == NONE)
				{
					object_index = player->unit_index;
				}

				object_set_position(object_index, &result->position, NULL, NULL);
			}
			else
			{
				terminal_printf(global_real_argb_orange, "Camera is outside BSP... cannot initiate teleportation...");
			}
		}
	}

	return;
}

void cheat_active_camouflage(
	void)
{
	long player_index = cheat_player_index();

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		struct unit_datum *unit = unit_get(player->unit_index);

		unit->unit.active_camouflage = 1.f;

		if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
		{
			SET_FLAG(unit->unit.flags, _unit_super_camouflaged_bit, TRUE);
		}

		SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, TRUE);
	}

	return;
}

void cheat_active_camouflage_local_player(
	short local_player_index)
{
	if (local_player_index >= 0 && local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
	{
		long player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE)
		{
			struct player_datum *player = player_get(player_index);
			struct unit_datum *unit = unit_get(player->unit_index);

			unit->unit.active_camouflage = 1.f;

			if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
			{
				SET_FLAG(unit->unit.flags, _unit_super_camouflaged_bit, TRUE);
			}

			SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, TRUE);
		}
	}

	return;
}

/* ---------- private code */

static long cheat_player_index(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;
	long player_index = NONE;

	data_iterator_new(&iterator, player_data);

	while (player = (struct player_datum *)data_iterator_next(&iterator))
	{
		if (player->unit_index != NONE)
		{
			player_index = iterator.index;
			break;
		}
	}

	return player_index;
}

static void cheat_objects(
	struct game_globals_tag_reference const *references,
	short name_count)
{
	long player_index = cheat_player_index();

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		real_point3d origin;
		real_vector3d forward;
		real_vector3d up;
		short name_index;

		unit_get(player->unit_index);
		object_get_origin(player->unit_index, &origin);
		object_get_orientation(player->unit_index, &forward, &up);

		for (name_index = 0; name_index < name_count; name_index++)
		{
			if (references[name_index].reference.index != NONE)
			{
				struct object_placement_data data;
				real angle = arctangent(forward.i, forward.j) + (name_index - name_count / 2) * MIN(2.f * _pi / name_count, _pi / 8.f);

				object_placement_data_new(&data, references[name_index].reference.index, NONE);
				data.position.x = origin.x + cosine(angle) * 1.5f;
				data.position.y = origin.y + sine(angle) * 1.5f;
				data.position.z = origin.z + 0.8f;
				data.forward = forward;
				data.up = up;
				object_new(&data);
			}
		}
	}

	return;
}
