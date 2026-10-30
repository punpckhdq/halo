/*
PLAYERS.C

symbols in this file:
000A9470 0070:
	_players_initialize (0000)
000A94E0 00b0:
	_players_initialize_for_new_map (0000)
000A9590 0020:
	_players_dispose_from_old_map (0000)
000A95B0 0030:
	_players_dispose (0000)
000A95E0 0020:
	_machine_get_player_list (0000)
000A9600 0050:
	_machine_add_player (0000)
000A9650 0060:
	_local_player_exists (0000)
000A96B0 0050:
	_find_unused_local_player_index (0000)
000A9700 0020:
	_player_delete (0000)
000A9720 0010:
	_players_get_respawn_failure (0000)
000A9730 0050:
	_local_player_get_player_index (0000)
000A9780 00a0:
	_local_player_set_player_index (0000)
000A9820 0010:
	_local_player_count (0000)
000A9830 0040:
	_local_player_get_next (0000)
000A9870 0050:
	_player_index_from_unit_index (0000)
000A98C0 0090:
	_player_died (0000)
000A9950 0010:
	_players_are_all_dead (0000)
000A9960 00c0:
	_players_set_local_player_unit (0000)
000A9A20 0010:
	_players_get_combined_pvs_local (0000)
000A9A30 0010:
	_players_get_combined_pvs (0000)
000A9A40 0020:
	_player_input_enable (0000)
000A9A60 0020:
	_player_input_enabled (0000)
000A9A80 00f0:
	_any_player_is_in_the_air (0000)
000A9B70 0050:
	_any_player_is_dead (0000)
000A9BC0 0040:
	_is_player_in_trigger (0000)
000A9C00 00e0:
	_player_pseudo_kill (0000)
000A9CE0 0120:
	_player_control_fix_for_loaded_game_state (0000)
000A9E00 0050:
	_player_get_starting_location_count (0000)
000A9E50 00c0:
	_player_get_starting_location (0000)
000A9F10 0070:
	_placement_data_set_change_color (0000)
000A9F80 0070:
	_create_weapon (0000)
000A9FF0 0170:
	_players_compute_combined_pvs (0000)
000AA160 0020:
	_players_compute_local_player_count (0000)
000AA180 0010:
	_player_examine_nearby_unit (0000)
000AA190 0090:
	_unit_should_autopick_weapon (0000)
000AA220 0020:
	_player_reset_action_result (0000)
000AA240 00c0:
	_player_handle_weapon_swap (0000)
000AA300 00b0:
	_player_over_shield_screen_effect (0000)
000AA3B0 00b0:
	_player_active_camo_screen_effect (0000)
000AA460 0090:
	_player_health_pack_screen_effect (0000)
000AA4F0 0040:
	_player_powerup_on (0000)
000AA530 0030:
	_player_powerup_additional (0000)
000AA560 0030:
	_player_powerup_off (0000)
000AA590 0070:
	_players_handle_deleted_object (0000)
000AA600 0020:
	_random_direction3d (0000)
000AA620 0040:
	_valid_real_vector2d (0000)
000AA660 0120:
	_player_new (0000)
000AA780 0150:
	_player_add_equipment (0000)
000AA8D0 0110:
	_player_aiming_vector_from_facing (0000)
000AA9E0 0510:
	_player_teleport_internal (0000)
000AAEF0 0060:
	_player_teleport (0000)
000AAF50 00d0:
	_find_best_starting_location_index (0000)
000AB020 0330:
	_player_spawn (0000)
000AB350 00f0:
	_player_set_action_result (0000)
000AB440 0250:
	_player_handle_action (0000)
000AB690 00f0:
	_player_handle_powerup (0000)
000AB780 00a0:
	_player_handle_powerup_minor (0000)
000AB820 0070:
	_player_update_powerups (0000)
000AB890 01a0:
	_players_debug_render (0000)
000ABA30 0090:
	_debug_player_teleport (0000)
000ABAC0 01d0:
	_players_respawn_coop (0000)
000ABC90 0140:
	_player_teleport_on_bsp_switch (0000)
000ABDD0 02e0:
	_players_reconnect_to_structure_bsp (0000)
000AC0B0 01c0:
	_player_examine_nearby_vehicle (0000)
000AC270 00b0:
	_player_examine_nearby_device (0000)
000AC320 0150:
	_player_handle_powerup_equipment (0000)
000AC470 06e0:
	_players_update_before_game (0000)
000ACB50 0320:
	_player_examine_nearby_item (0000)
000ACE70 00f0:
	_player_examine_nearby_objects (0000)
000ACF60 0330:
	_players_update_after_game (0000)
0025CED8 006c:
	_rdata_0025ced8 (0000)
0025CF44 001a:
	??_C@_0BK@OOAKICCI@players_update_after_game?$AA@ (0000)
0025CF60 001b:
	??_C@_0BL@FKGHDGFM@players_update_before_game?$AA@ (0000)
0025CF7C 0010:
	??_C@_0BA@CEEHCBBD@players?5globals?$AA@ (0000)
0025CF8C 0006:
	??_C@_05EINLALJG@teams?$AA@ (0000)
0025CF94 0008:
	??_C@_07BCGOFIOM@players?$AA@ (0000)
0025CF9C 001a:
	??_C@_0BK@CGPNFENM@failed?5to?5create?5a?5player?$AA@ (0000)
0025CFB8 001e:
	??_C@_0BO@IAEAGICD@c?3?2halo?2SOURCE?2game?2players?4c?$AA@ (0000)
0025CFD8 004f:
	??_C@_0EP@PCIBAKOM@local_player_index?$DO?$DNNONE?5?$CG?$CG?5loca@ (0000)
0025D028 009b:
	??_C@_0JL@EEPEKCFO@tried?5to?5fix?5broken?5player?5contr@ (0000)
0025D0C8 0069:
	??_C@_0GJ@LOLKJFPO@failed?5to?5correct?5player?5control@ (0000)
0025D134 0031:
	??_C@_0DB@FKMNJOMP@corrected?5player?5control?5for?5res@ (0000)
0025D168 0070:
	??_C@_0HA@DONIDEOA@?$CI?$CIlocal_player_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIloc@ (0000)
0025D1D8 002b:
	??_C@_0CL@OKOJEGMM@Could?5not?5attach?5starting?5weapon@ (0000)
0025D204 0021:
	??_C@_0CB@EBNMJKAL@player?9?$DOlocal_player_index?$CB?$DNNONE@ (0000)
0025D228 002f:
	??_C@_0CP@BNOOKFKL@couldn?8t?5teleport?5player?5into?5a?5@ (0000)
0025D258 0019:
	??_C@_0BJ@LHNHGGK@player?9?$DOunit_index?$CB?$DNNONE?$AA@ (0000)
0025D274 0006:
	??_C@_05MHMEBBGO@scale?$AA@ (0000)
0025D27C 0025:
	??_C@_0CF@PKKNMABJ@magnitude3d?$CI?$CGbest_adjustment_vec@ (0000)
0025D2A4 0032:
	??_C@_0DC@CLGKJEDK@source_unit_index?$DN?$DNNONE?5?$HM?$HM?5local@ (0000)
0025D2D8 0008:
	__real@3fe6a09e60000000 (0000)
0025D2E0 003a:
	??_C@_0DK@GFFNPIMP@powerup_type?$DO?$DN0?5?$CG?$CG?5powerup_type?$DM@ (0000)
0025D31C 0016:
	??_C@_0BG@HMDKOAOM@no?5players?5in?5the?5bsp?$AA@ (0000)
0025D334 0013:
	??_C@_0BD@ELAHGGCE@seat_index?5?$CB?$DN?5NONE?$AA@ (0000)
0025D348 0046:
	??_C@_0EG@POGHKPOA@?$CINONE?5?$DN?$DN?5control_data?4zoom_level@ (0000)
0025D390 008c:
	??_C@_0IM@DKBPHNBM@?$CINONE?5?$DN?$DN?5control_data?4grenade_in@ (0000)
0025D420 0085:
	??_C@_0IF@OCHKLACI@?$CINONE?5?$DN?$DN?5control_data?4weapon_ind@ (0000)
0025D4A8 004c:
	??_C@_0EM@ECBIAFIL@?$CINONE?5?$DN?$DN?5action?9?$DOdesired_zoom_le@ (0000)
0025D4F8 0095:
	??_C@_0JF@HCBNBPOG@?$CINONE?5?$DN?$DN?5action?9?$DOdesired_grenade@ (0000)
0025D590 008e:
	??_C@_0IO@DMKDCDID@?$CINONE?5?$DN?$DN?5action?9?$DOdesired_weapon_@ (0000)
0025D620 0018:
	??_C@_0BI@GKAKFFBF@action?9?$DOprimary_trigger?$AA@ (0000)
0025D638 0027:
	??_C@_0CH@OILDLPCJ@?$CFs?3?5assert_valid_real_vector2d?$CI?$CF@ (0000)
0025D660 0012:
	??_C@_0BC@MPDBBHIH@?$CGaction?9?$DOthrottle?$AA@ (0000)
0025D674 001b:
	??_C@_0BL@PMBMFGJI@action?9?$DOdesired_facing?4yaw?$AA@ (0000)
0025D690 001d:
	??_C@_0BN@NJOPCJPO@action?9?$DOdesired_facing?4pitch?$AA@ (0000)
0025D6B0 0042:
	??_C@_0EC@JMPLFNIK@action_index?$DO?$DN0?5?$CG?$CG?5action_index?$DM@ (0000)
0025D6F4 003b:
	??_C@_0DL@NOPAIALJ@?$CB?$CB?$CBWARNING?$CB?$CB?$CB?5teleported?5player?5@ (0000)
0025D730 0004:
	__real@3c360b61 (0000)
002DEE08 0c18:
	_data_002dee08 (0000)
00453408 005d:
	_bss_00453408 (0000)
	_debug_render_player_teleport (005c)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "render.h"
#include "render_debug.h"
#include "network_game_globals.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "game_state.h"
#include "network_messages.h"
#include "collisions.h"
#include "text_group.h"
#include "vehicles.h"
#include "ai_debug.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "weapons.h"
#include "bipeds.h"
#include "observer.h"
#include "editor_stubs.h"
#include "hud.h"
#include "network_client_manager.h"
#include "effects.h"
#include "network_server_message_handler.h"
#include "network_server_manager.h"
#include "object_types.h"
#include "player_ui.h"
#include "damage_effect_definitions.h"
#include "devices.h"
#include "projectiles.h"
#include "player_effects.h"
#include "hud_messaging.h"
#include "equipment_definitions.h"
#include "equipment.h"
#include "profile.h"

/* ---------- constants */

/* ---------- macros */

#define MAXIMUM_WEAPONS_PER_PLAYER 2 /* fake name */

/* ---------- structures */

struct team_datum /* fake name */
{
	short identifier;
	short pad;
	long unused[15]; /* fake name */
};

struct players_global_data
{
	long local_player_network_identifier;
	long local_player_indices[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	long local_player_failed_teleport_unit_indices[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	short local_player_count;
	short players_are_double_speed_duration;
	boolean all_dead;
	boolean input_inhibited;
	short bsp_switch_trigger_index;
	short respawn_failure_reason;
	boolean force_respawn;
	char local_player_triggered_switch : 4;
	byte bsp_check_recursive_switch_ticks : 4;
	unsigned long combined_pvs[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
	unsigned long combined_pvs_local[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
};

/* ---------- prototypes */

static void machine_add_player(long machine_index, long player_index);
static boolean is_player_in_trigger(long unit_index, short switch_trigger_index);
static void player_pseudo_kill(long player_index, long source_unit_index);
static long create_weapon(struct starting_weapon_info const *weapon_info, long unit_index);
static void players_compute_combined_pvs(unsigned long *combined_pvs, boolean local_only);
static short players_compute_local_player_count(void);
static void player_examine_nearby_unit(long player_index, long unit_index);
static void player_reset_action_result(long player_index);
static boolean player_handle_weapon_swap(long player_index);
static void player_over_shield_screen_effect(long player_index);
static void player_active_camo_screen_effect(long player_index);
static void player_health_pack_screen_effect(long player_index);
static void player_powerup_on(long player_index, short powerup_type);
static void player_powerup_additional(long player_index, short powerup_type);
static void player_powerup_off(long player_index, short powerup_type);
static boolean player_teleport_internal(long player_index, long source_unit_index, real_point3d const *position);
static void player_spawn(long player_index);
static void player_set_action_result(long player_index, short action_result, long action_object_index, short action_seat_index);
static boolean player_handle_action(long player_index);
static void player_update_powerups(long player_index);
static void player_teleport_on_bsp_switch(long player_index, long source_unit_index, real_point3d const *position);
static void player_examine_nearby_vehicle(long player_index, long vehicle_index);
static void player_examine_nearby_device(long player_index, long device_index);
static void player_handle_powerup_equipment(long player_index, long equipment_index);
static void player_examine_nearby_item(long player_index, long item_index);
static void player_examine_nearby_objects(long player_index);

/* ---------- globals */

struct data_array *player_data;
struct data_array *team_data;
struct players_global_data *players_globals;

static struct profile_section players_update_before_game_section =
{
	"players_update_before_game",
	NONE,
	TRUE
}; /* fake name */

static struct profile_section players_update_after_game_section =
{
	"players_update_after_game",
	NONE,
	TRUE
}; /* fake name */

static long machine_to_player_table[MAXIMUM_NETWORK_MACHINE_COUNT][MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];

/* ---------- public code */

void players_initialize(
	void)
{
	player_data = game_state_data_new("players", NETWORK_GAME_MAXIMUM_PLAYER_COUNT, sizeof(struct player_datum));
	team_data = game_state_data_new("teams", NETWORK_GAME_MAXIMUM_PLAYER_COUNT, sizeof(struct team_datum));
	players_globals = game_state_malloc("players globals", NULL, sizeof(struct players_global_data));
	memset(players_globals->local_player_indices, NONE, sizeof(players_globals->local_player_indices));
	players_globals->local_player_network_identifier = NONE;
	players_globals->local_player_count = 0;
	player_control_initialize();

	return;
}

void players_initialize_for_new_map(
	void)
{
	player_control_dispose();
	memset(players_globals, 0, sizeof(struct players_global_data));
	memset(players_globals->local_player_indices, NONE, sizeof(players_globals->local_player_indices));
	memset(players_globals->local_player_failed_teleport_unit_indices, NONE, sizeof(players_globals->local_player_failed_teleport_unit_indices));
	players_globals->local_player_network_identifier = NONE;
	players_globals->input_inhibited = FALSE;
	players_globals->players_are_double_speed_duration = 0;
	players_globals->all_dead = FALSE;
	players_globals->bsp_switch_trigger_index = NONE;
	players_globals->respawn_failure_reason = _player_respawn_failure_none;
	data_make_valid(player_data);
	data_make_valid(team_data);
	memset(machine_to_player_table, NONE, sizeof(machine_to_player_table));

	return;
}

void players_dispose_from_old_map(
	void)
{
	data_make_invalid(player_data);
	data_make_invalid(team_data);

	return;
}

void players_dispose(
	void)
{
	if (player_data)
	{
		player_data = NULL;
	}

	if (team_data)
	{
		team_data = NULL;
	}

	if (players_globals)
	{
		players_globals = NULL;
	}

	return;
}

long *machine_get_player_list(
	long machine_index)
{
	return machine_to_player_table[DATUM_INDEX_TO_ABSOLUTE_INDEX(machine_index)];
}

static void machine_add_player(
	long machine_index,
	long player_index)
{
	long *player_list = machine_get_player_list(machine_index);
	long index;

	for (index = 0; index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; index++)
	{
		if (player_list[index] == NONE)
		{
			player_list[index] = player_index;
			break;
		}
	}

	match_vassert("c:\\halo\\SOURCE\\game\\players.c", 240, index != MAXIMUM_NUMBER_OF_LOCAL_PLAYERS, "failed to create a player");

	return;
}

boolean local_player_exists(
	long local_player_index)
{
	boolean exists = FALSE;
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->local_player_index == local_player_index)
		{
			exists = TRUE;
			break;
		}
	}

	return exists;
}

long find_unused_local_player_index(
	void)
{
	long local_player_index;
	long unused_local_player_index = NONE;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		if (input_has_gamepad((short)local_player_index) && !local_player_exists(local_player_index))
		{
			unused_local_player_index = local_player_index;
			break;
		}
	}

	if (unused_local_player_index == NONE)
	{
		for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
		{
			if (!local_player_exists(local_player_index))
			{
				unused_local_player_index = local_player_index;
				break;
			}
		}
	}

	return unused_local_player_index;
}

long player_new(
	long machine_index,
	long player_index,
	short local_player_index,
	struct network_player *network_player_data)
{
	if (player_index == NONE)
	{
		player_index = datum_new(player_data);
	}
	else
	{
		player_index = datum_new_at_index(player_data, player_index);
	}

	match_assert("c:\\halo\\SOURCE\\game\\players.c", 309, ((local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)) || (local_player_index==NONE));

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);

		ustrncpy(player->name, !network_player_data ? L"" : network_player_data->name, 11);
		player->name[11] = 0;
		player->local_player_index = local_player_index;
		player->unit_index = NONE;
		player->dead_unit_index = NONE;
		player->squad_index = NONE;
		player->cluster_index = NONE;
		player->aim_assist_unit_index = NONE;
		player->multiplayer.speed_multiplier = 1.f;
		player->team_index = _game_team_player;
		player_reset_action_result(player_index);
		player->quit_out_of_game_time = NONE;
		player->quit_out_of_game = FALSE;

		if (network_player_data)
		{
			memcpy(&player->network_player_data, network_player_data, sizeof(player->network_player_data));
		}
	}

	machine_add_player(machine_index, player_index);

	return player_index;
}

void player_delete(
	long player_index)
{
	datum_delete(player_data, player_index);

	return;
}

void player_add_equipment(
	long unit_index,
	short profile_index,
	boolean reset)
{
	if (unit_index != NONE && profile_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(unit_index);

		if (unit->unit.player_index != NONE)
		{
			struct scenario *scenario = global_scenario_get();
			struct scenario_starting_profile *profile = TAG_BLOCK_GET_ELEMENT(&scenario->starting_profiles, profile_index, struct scenario_starting_profile);
			long weapon_index;
			short grenade_type;

			if (reset)
			{
				unit_delete_all_weapons(unit_index);
				unit->object.shield_vitality = 0.f;
				unit->object.body_vitality = 0.f;

				for (grenade_type = 0; grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_type++)
				{
					unit->unit.grenade_counts[grenade_type] = 0;
				}
			}

			if (profile->starting_weapons[0].weapon.index != NONE)
			{
				weapon_index = create_weapon(&profile->starting_weapons[0], unit_index);

				if (weapon_index != NONE && !unit_add_weapon_to_inventory(unit_index, weapon_index, reset ? _unit_add_weapon_as_current_weapon : _unit_add_weapon_normally))
				{
					error(_error_silent, "Could not attach starting weapon to player");
					object_delete(weapon_index);
				}
			}

			if (profile->starting_weapons[1].weapon.index != NONE)
			{
				weapon_index = create_weapon(&profile->starting_weapons[1], unit_index);

				if (weapon_index != NONE && !unit_add_weapon_to_inventory(unit_index, weapon_index, _unit_add_weapon_normally))
				{
					error(_error_silent, "Could not attach starting weapon to player");
					object_delete(weapon_index);
				}
			}

			unit->object.shield_vitality += profile->starting_shield;
			unit->object.body_vitality += profile->starting_health;

			for (grenade_type = 0; grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_type++)
			{
				unit->unit.grenade_counts[grenade_type] += profile->starting_grenade_counts[grenade_type];
			}
		}
	}

	return;
}

void player_aiming_vector_from_facing(
	long player_index,
	real_vector3d *aiming_vector,
	real_euler_angles2d const *desired_facing)
{
	struct player_datum *player = player_get(player_index);

	vector3d_from_euler_angles2d(aiming_vector, desired_facing);

	if (player->unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(player->unit_index);

		if (unit->object.parent_object_index != NONE)
		{
			struct vehicle_datum *vehicle = object_try_and_get_and_verify_type(unit->object.parent_object_index, _object_mask_vehicle);

			if (vehicle)
			{
				struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				if (!TEST_FLAG(seat->flags, _unit_seat_has_third_person_camera_bit))
				{
					real_matrix4x3 vehicle_matrix;

					cross_product3d(&vehicle->object.up, global_right3d, &vehicle_matrix.forward);

					if (normalize3d(&vehicle_matrix.forward) == 0.f)
					{
						cross_product3d(&vehicle->object.up, global_down3d, &vehicle_matrix.forward);
						normalize3d(&vehicle_matrix.forward);
					}

					matrix4x3_rotation_from_vectors(&vehicle_matrix, &vehicle_matrix.forward, &vehicle->object.up);
					matrix4x3_transform_normal(&vehicle_matrix, aiming_vector, aiming_vector);
				}
			}
		}
	}

	return;
}

boolean players_respawn_coop(
	void)
{
	boolean respawned = FALSE;

	players_globals->respawn_failure_reason = _player_respawn_failure_none;

	if (!players_globals->force_respawn && (dangerous_projectiles_near_player() || any_unit_is_dangerous()))
	{
		players_globals->respawn_failure_reason = _player_respawn_failure_combat;
	}
	else if (!players_globals->force_respawn && ai_enemies_attacking_player())
	{
		players_globals->respawn_failure_reason = _player_respawn_failure_enemies;
	}
	else
	{
		struct data_iterator iterator;
		struct player_datum *player;
		long living_unit_index = NONE;

		data_iterator_new(&iterator, player_data);

		while (player = data_iterator_next(&iterator))
		{
			if (player->unit_index != NONE)
			{
				long parent_index = object_get_ultimate_parent(player->unit_index);
				boolean airborne = FALSE;

				if (parent_index == player->unit_index)
				{
					struct biped_datum *biped = biped_try_and_get(player->unit_index);

					if (biped)
					{
						airborne = TEST_FLAG(biped->biped.flags, _biped_airborne_bit);
					}
				}
				else
				{
					struct vehicle_datum *vehicle = object_try_and_get_and_verify_type(parent_index, _object_mask_vehicle);

					if (vehicle)
					{
						airborne = vehicle->vehicle.airborne_ticks > 0;
					}
				}

				if (!airborne)
				{
					living_unit_index = player->unit_index;
				}
				else
				{
					players_globals->respawn_failure_reason = _player_respawn_failure_moving;
				}
			}
		}

		if (living_unit_index != NONE)
		{
			respawned = TRUE;
			data_iterator_new(&iterator, player_data);

			while (player = data_iterator_next(&iterator))
			{
				if (player->unit_index == NONE)
				{
					player_spawn(iterator.index);

					if (player->unit_index != NONE)
					{
						respawned = player_teleport(iterator.index, living_unit_index, &object_get(living_unit_index)->object.bounding_sphere_center);
					}
					else
					{
						respawned = FALSE;
					}
				}
			}
		}

		players_globals->force_respawn = players_globals->force_respawn && !respawned;

		if (respawned)
		{
			players_globals->respawn_failure_reason = _player_respawn_failure_none;
		}
	}

	return respawned;
}

short players_get_respawn_failure(
	void)
{
	return players_globals->respawn_failure_reason;
}

void players_update_before_game(
	void)
{
	struct player_action actions[NETWORK_GAME_MAXIMUM_PLAYER_COUNT];

	profile_enter(players_update_before_game_section);

	if (update_client_dequeue(actions))
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);

		while (player = data_iterator_next(&iterator))
		{
			short action_index = (short)iterator.index;
			struct player_action *action = &actions[action_index];

			match_assert("c:\\halo\\SOURCE\\game\\players.c", 598, action_index>=0 && action_index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT);




			match_assert_valid_real("c:\\halo\\SOURCE\\game\\players.c", 603, action->desired_facing.pitch);
			match_assert_valid_real("c:\\halo\\SOURCE\\game\\players.c", 604, action->desired_facing.yaw);
			match_assert_valid_real_vector2d("c:\\halo\\SOURCE\\game\\players.c", 605, &action->throttle);
			match_assert_valid_real("c:\\halo\\SOURCE\\game\\players.c", 606, action->primary_trigger);

			match_assert("c:\\halo\\SOURCE\\game\\players.c", 608, (NONE == action->desired_weapon_index) || ((action->desired_weapon_index >= 0) && (action->desired_weapon_index <= MAXIMUM_WEAPONS_PER_UNIT)));
			match_assert("c:\\halo\\SOURCE\\game\\players.c", 609, (NONE == action->desired_grenade_index) || ((action->desired_grenade_index >= 0) && (action->desired_grenade_index <= NUMBER_OF_UNIT_GRENADE_TYPES)));
			match_assert("c:\\halo\\SOURCE\\game\\players.c", 610, (NONE == action->desired_zoom_level) || ((action->desired_zoom_level >= 0)));

			if (player->unit_index == NONE && !game_in_editor())
			{
				if (game_engine_running())
				{
					if (game_engine_should_spawn_player(iterator.index))
					{
						game_engine_prespawn_player_update(iterator.index);
						player_spawn(iterator.index);

						if (player->unit_index != NONE)
						{
							game_engine_postspawn_player_update(iterator.index);
						}
						else
						{
							player->respawn_timer = 1;
						}
					}
				}
				else if (!main_menu_is_active())
				{
					if (player->statistics.deaths == 0)
					{
						player_spawn(iterator.index);
					}
					else if (!players_are_all_dead())
					{
						main_respawn(players_globals->force_respawn);
					}
				}
			}

			if (player->unit_index != NONE && unit_controllable(player->unit_index))
			{
				struct unit_datum *unit = unit_get(player->unit_index);

				if (!players_globals->input_inhibited)
				{
					struct unit_control_data control_data;
					long weapon_index;

					if (TEST_FLAG(action->control_flags, _unit_control_action_bit) && unit->object.parent_object_index == NONE && !player_handle_action(iterator.index))
					{
						SET_FLAG(action->control_flags, _unit_control_weapon_reload_bit, TRUE);
					}

					if (TEST_FLAG(action->control_flags, _unit_control_swap_weapons_bit) && unit->object.parent_object_index == NONE)
					{
						if (!player->swapped_weapons)
						{
							player->swapped_weapons = player_handle_weapon_swap(iterator.index);
						}
					}
					else
					{
						player->swapped_weapons = FALSE;
					}

					if (TEST_FLAG(action->control_flags, _unit_control_use_equipment_bit) && unit->unit.equipment_object_index != NONE)
					{
						player_handle_powerup_equipment(iterator.index, unit->unit.equipment_object_index);
						unit_delete_current_equipment(player->unit_index);
					}

					weapon_index = unit_inventory_get_weapon(player->unit_index, unit_get(player->unit_index)->unit.current_weapon_index);

					if (weapon_index != NONE && weapon_must_be_readied(weapon_index))
					{
						if (TEST_FLAG(action->control_flags, _unit_control_weapon_primary_trigger_bit) || TEST_FLAG(action->control_flags, _unit_control_weapon_secondary_trigger_bit))
						{
							unit_drop_current_weapon(player->unit_index, TRUE);
						}

						action->desired_weapon_index = unit->unit.current_weapon_index;
					}

					control_data.control_flags = (word)action->control_flags;
					player_aiming_vector_from_facing(iterator.index, &control_data.aiming_vector, &action->desired_facing);
					control_data.looking_vector = control_data.aiming_vector;
					control_data.facing_vector = control_data.aiming_vector;
					control_data.throttle.i = action->throttle.i;
					control_data.throttle.j = action->throttle.j;
					control_data.throttle.k = 0.f;
					control_data.primary_trigger = action->primary_trigger;
					control_data.animation_state = 3;
					control_data.weapon_index = action->desired_weapon_index;
					control_data.grenade_index = action->desired_grenade_index;
					control_data.zoom_level = action->desired_zoom_level;
					control_data.aiming_speed = 0;
					match_assert("c:\\halo\\SOURCE\\game\\players.c", 739, (NONE == control_data.weapon_index) || ((control_data.weapon_index >= 0) && (control_data.weapon_index <= MAXIMUM_WEAPONS_PER_UNIT)));
					match_assert("c:\\halo\\SOURCE\\game\\players.c", 740, (NONE == control_data.grenade_index) || ((control_data.grenade_index >= 0) && (control_data.grenade_index <= NUMBER_OF_UNIT_GRENADE_TYPES)));
					match_assert("c:\\halo\\SOURCE\\game\\players.c", 741, (NONE == control_data.zoom_level) || ((control_data.zoom_level >= 0)));
					unit_control(player->unit_index, &control_data);
				}
				else if (unit->unit.swarm_actor_index == NONE && unit->unit.actor_index == NONE)
				{
					struct unit_control_data control_data;

					control_data.animation_state = 3;
					control_data.aiming_speed = 0;
					control_data.control_flags = 0;
					control_data.weapon_index = NONE;
					control_data.grenade_index = NONE;
					control_data.zoom_level = NONE;
					control_data.throttle = *global_zero_vector3d;
					control_data.primary_trigger = 0.f;
					control_data.facing_vector = unit->unit.desired_facing_vector;
					control_data.aiming_vector = unit->unit.desired_aiming_vector;
					control_data.looking_vector = unit->unit.desired_looking_vector;
					unit_control(player->unit_index, &control_data);
				}
			}
		}

		players_compute_combined_pvs(players_globals->combined_pvs_local, TRUE);
		players_compute_combined_pvs(players_globals->combined_pvs, FALSE);
		players_globals->local_player_count = players_compute_local_player_count();
	}
	else
	{
		match_halt("c:\\halo\\SOURCE\\game\\players.c", 779);
	}

	profile_exit(players_update_before_game_section);

	return;
}

void players_update_after_game(
	void)
{
	static boolean map_lost = FALSE;
	struct data_iterator iterator;
	struct player_datum *player;

	profile_enter(players_update_after_game_section);

	if (players_globals->players_are_double_speed_duration > 0)
	{
		players_globals->players_are_double_speed_duration--;

		if (players_globals->players_are_double_speed_duration == 0)
		{
			game_set_players_are_double_speed(FALSE);
		}
	}

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (!player->is_blocking_teleporter)
		{
			if (player->telefrag_timeout > 0)
			{
				player->telefrag_timeout--;
			}
		}
		else
		{
			long telefrag_timeout = player->telefrag_timeout;

			if (telefrag_timeout >= 3 * TICKS_PER_SECOND)
			{
				if (player->unit_index != NONE && !TEST_FLAG(unit_get(player->unit_index)->object.damage_flags, _object_die_act_of_god_bit))
				{
					if (player->local_player_index != NONE)
					{
						wchar_t *string;

						multiplayer_game_text_get_string(string, 183);
						hud_print_message(player->local_player_index, string);
					}

					player_telefrag_effect_stop(iterator.index);
					unit_kill(player->unit_index);
				}
			}
			else
			{
				player_telefrag_effect_start(iterator.index, telefrag_timeout / (real)(3 * TICKS_PER_SECOND));
			}
		}

		player->is_blocking_teleporter = FALSE;

		if (player->unit_index != NONE)
		{
			player_update_powerups(iterator.index);
		}

		if (player->unit_index != NONE)
		{
			long parent_index = object_get_ultimate_parent(player->unit_index);

			if (!TEST_FLAG(object_get(parent_index)->object.flags, _object_outside_of_map_bit))
			{
				struct scenario *scenario = global_scenario_get();
				short trigger_index;

				for (trigger_index = 0; trigger_index < scenario->bsp_switch_trigger_volumes.count; trigger_index++)
				{
					struct scenario_bsp_switch_trigger_volume *switch_trigger = TAG_BLOCK_GET_ELEMENT(&scenario->bsp_switch_trigger_volumes, trigger_index, struct scenario_bsp_switch_trigger_volume);

					if (switch_trigger->source_bsp_index == global_structure_bsp_index && scenario_trigger_volume_test_object(switch_trigger->trigger_volume_index, player->unit_index))
					{
						if (players_globals->local_player_triggered_switch != NONE && players_globals->local_player_triggered_switch != player->local_player_index)
						{
							error(_error_silent, "!!!WARNING!!! teleported player triggering a bsp switch!!!");
						}

						players_globals->bsp_check_recursive_switch_ticks = 0;
						players_globals->local_player_triggered_switch = (char)player->local_player_index;
						players_globals->bsp_switch_trigger_index = trigger_index;
						main_switch_structure_bsp(switch_trigger->destination_bsp_index);
					}
				}
			}
		}

		player_reset_action_result(iterator.index);
		player_examine_nearby_objects(iterator.index);
	}

	if (players_globals->local_player_triggered_switch != NONE)
	{
		players_globals->bsp_check_recursive_switch_ticks++;

		if (players_globals->bsp_check_recursive_switch_ticks > 12)
		{
			players_globals->local_player_triggered_switch = NONE;
			players_globals->bsp_check_recursive_switch_ticks = 0;
		}
	}

	if (players_globals->all_dead)
	{
		if (!game_engine_running() && !map_lost)
		{
			main_lost_map();
			map_lost = TRUE;
		}
	}
	else if (map_lost)
	{
		map_lost = FALSE;
	}

	profile_exit(players_update_after_game_section);

	return;
}

long local_player_get_player_index(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\game\\players.c", 943, local_player_index>=NONE && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return local_player_index == NONE ? NONE : players_globals->local_player_indices[local_player_index];
}

void local_player_set_player_index(
	short local_player_index,
	long player_index)
{
	match_assert("c:\\halo\\SOURCE\\game\\players.c", 952, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	if (players_globals->local_player_indices[local_player_index] != NONE)
	{
		player_get(players_globals->local_player_indices[local_player_index])->local_player_index = NONE;
	}

	players_globals->local_player_indices[local_player_index] = player_index;

	if (player_index != NONE)
	{
		player_get(players_globals->local_player_indices[local_player_index])->local_player_index = local_player_index;
	}

	return;
}

short local_player_count(
	void)
{
	return players_globals->local_player_count;
}

short local_player_get_next(
	short local_player_index)
{
	short index;
	short next_local_player_index = NONE;

	for (index = 0; index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; index++)
	{
		if (players_globals->local_player_indices[index] != NONE && index > local_player_index && (index < next_local_player_index || next_local_player_index == NONE))
		{
			next_local_player_index = index;
		}
	}

	return next_local_player_index;
}

long player_index_from_unit_index(
	long unit_index)
{
	struct data_iterator iterator;
	struct player_datum *player;
	long player_index = NONE;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->unit_index == unit_index)
		{
			player_index = iterator.index;
		}
	}

	return player_index;
}

void player_died(
	long player_index)
{
	struct data_iterator iterator;
	struct player_datum *player = player_get(player_index);

	player->dead_unit_index = player->unit_index;
	player->unit_index = NONE;

	if (player->local_player_index != NONE)
	{
		player_control_new_unit(player->local_player_index, NONE);
	}

	players_globals->all_dead = TRUE;
	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->unit_index != NONE)
		{
			players_globals->all_dead = FALSE;
		}
	}

	return;
}

boolean players_are_all_dead(
	void)
{
	return players_globals->all_dead;
}

void players_set_local_player_unit(
	short local_player_index,
	long new_unit_index)
{
	long old_unit_index = player_control_get_unit_index(local_player_index);
	struct player_datum *player;

	match_assert("c:\\halo\\SOURCE\\game\\players.c", 1060, game_connection()==_game_connection_local);

	if (old_unit_index != NONE)
	{
		unit_get(old_unit_index)->unit.player_index = NONE;
		unit_set_actively_controlled(old_unit_index, FALSE);
	}

	if (new_unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(new_unit_index);

		unit_set_actively_controlled(new_unit_index, TRUE);
		unit->unit.player_index = local_player_get_player_index(local_player_index);
	}

	player = player_get(local_player_get_player_index(local_player_index));
	player->unit_index = new_unit_index;
	player->dead_unit_index = NONE;
	player_control_new_unit(local_player_index, new_unit_index);

	return;
}

unsigned long const *players_get_combined_pvs_local(
	void)
{
	return players_globals->combined_pvs_local;
}

unsigned long const *players_get_combined_pvs(
	void)
{
	return players_globals->combined_pvs;
}

void player_input_enable(
	boolean enable)
{
	players_globals->input_inhibited = !enable;

	return;
}

boolean player_input_enabled(
	void)
{
	return !players_globals->input_inhibited;
}

boolean any_player_is_in_the_air(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);
			long parent_index = object_get_ultimate_parent(player->unit_index);
			struct object_datum *parent = object_get(parent_index);

			if (TEST_FLAG(parent->object.flags, _object_outside_of_map_bit))
			{
				return TRUE;
			}

			if (unit->object.parent_object_index == NONE)
			{
				if (unit->object.type == _object_type_biped)
				{
					if (biped_flying_through_air(player->unit_index))
					{
						return TRUE;
					}
				}
				else if (unit->object.type == _object_type_vehicle)
				{
					if (((struct vehicle_datum *)unit)->vehicle.airborne_ticks > 2)
					{
						return TRUE;
					}
				}
			}
			else
			{
				struct vehicle_datum *vehicle = object_try_and_get_and_verify_type(unit->object.parent_object_index, _object_mask_vehicle);

				if (vehicle)
				{
					struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

					if (TEST_FLAG(definition->unit.flags, _unit_causes_passenger_dialogue_bit) && vehicle->vehicle.airborne_ticks > 2)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

boolean any_player_is_dead(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->unit_index == NONE)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static boolean is_player_in_trigger(
	long unit_index,
	short switch_trigger_index)
{
	return switch_trigger_index != NONE && scenario_trigger_volume_test_object(TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->bsp_switch_trigger_volumes, switch_trigger_index, struct scenario_bsp_switch_trigger_volume)->trigger_volume_index, unit_index);
}

static void player_teleport_on_bsp_switch(
	long player_index,
	long source_unit_index,
	real_point3d const *position)
{
	struct player_datum *player = player_get(player_index);
	long unit_index = player->unit_index;
	struct biped_datum *biped = biped_try_and_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\game\\players.c", 1227, player_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\game\\players.c", 1228, position);

	if (biped)
	{
		short trigger_index = players_globals->bsp_switch_trigger_index;
		struct scenario_bsp_switch_trigger_volume *switch_trigger = trigger_index == NONE ? NULL : TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->bsp_switch_trigger_volumes, trigger_index, struct scenario_bsp_switch_trigger_volume);
		boolean outside_trigger = trigger_index != NONE && !scenario_trigger_volume_test_object(switch_trigger->trigger_volume_index, unit_index);

		if (scenario_leaf_index_from_point(&biped->object.bounding_sphere_center) == NONE || outside_trigger)
		{
			boolean success = FALSE;

			if (biped->object.parent_object_index != NONE && biped->object.parent_object_index != biped_get(source_unit_index)->object.parent_object_index)
			{
				unit_exit_seat_end(unit_index);
			}

			if (biped->object.parent_object_index == NONE)
			{
				success = player_teleport_internal(player_index, source_unit_index, position);
			}
			else
			{
				success = TRUE;
			}

			players_globals->force_respawn = !success;
		}
	}

	return;
}

static boolean player_teleport_internal(
	long player_index,
	long source_unit_index,
	real_point3d const *position)
{
	static real_point3d const adjustment_weights[] =
	{
		{ 1.f, 0.f, 0.f },
		{ 0.f, 1.f, 0.f },
		{ 0.f, -1.f, 0.f },
		{ 0.70710677f, -0.70710677f, 0.f },
		{ 0.70710677f, 0.70710677f, 0.f },
		{ 0.57735026f, 0.57735026f, 0.57735026f },
		{ 0.57735026f, 0.57735026f, -0.57735026f },
		{ 0.57735026f, -0.57735026f, 0.57735026f },
		{ 0.57735026f, -0.57735026f, -0.57735026f },
	};
	struct player_datum *player = player_get(player_index);
	long unit_index = player->unit_index;
	struct biped_datum *biped = biped_get(unit_index);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\game\\players.c", 1275, source_unit_index==NONE || local_player_count()>1);

	if (source_unit_index != NONE && object_get_ultimate_parent(source_unit_index) != source_unit_index)
	{
		long vehicle_index = object_get_ultimate_parent(source_unit_index);
		struct unit_datum *source_unit = unit_get(source_unit_index);
		struct object_datum *vehicle = object_get(vehicle_index);
		real_vector3d best_adjustment_vector;
		real radius;
		real scale;
		real_matrix4x3 adjustment_matrix;
		short weight_index;

		best_adjustment_vector = vehicle->object.translational_velocity;
		best_adjustment_vector.k = 0.f;
		source_unit_index = vehicle_index;

		if (!(magnitude_squared3d(&best_adjustment_vector) > 0.f))
		{
			if (vehicle->object.forward.k < 0.70710677f)
			{
				best_adjustment_vector = vehicle->object.forward;
			}
			else
			{
				best_adjustment_vector = vehicle->object.up;
			}
		}

		best_adjustment_vector.k = 0.f;
		radius = biped_definition_get(biped->definition_index)->biped.collision_radius;
		scale = radius * 3.f + vehicle->object.bounding_sphere_radius;
		match_assert("c:\\halo\\SOURCE\\game\\players.c", 1317, magnitude3d(&best_adjustment_vector));
		match_assert("c:\\halo\\SOURCE\\game\\players.c", 1318, scale);
		negate_vector3d(&best_adjustment_vector, &best_adjustment_vector);
		normalize3d(&best_adjustment_vector);
		matrix4x3_from_point_and_vectors(&adjustment_matrix, &vehicle->object.bounding_sphere_center, &best_adjustment_vector, global_up3d);
		adjustment_matrix.scale = scale;

		for (weight_index = 0; weight_index < NUMBEROF(adjustment_weights) && !success; weight_index++)
		{
			real_point3d anchor_point;

			matrix4x3_transform_point(&adjustment_matrix, &adjustment_weights[weight_index], &anchor_point);
			success = biped_fix_position(unit_index, source_unit_index, &anchor_point, NULL, 2.f, FALSE, FALSE, TRUE);

			if (!success)
			{
				short attempt_index;

				for (attempt_index = 0; attempt_index < 8 && !success; attempt_index++)
				{
					real_vector3d random_offset = *global_zero_vector3d;
					real_point3d new_position;

					random_direction3d(&random_offset);
					point_from_line3d(&anchor_point, &random_offset, radius, &new_position);
					success = biped_fix_position(unit_index, source_unit_index, &new_position, NULL, 2.f, FALSE, FALSE, TRUE);
				}
			}
		}
	}
	else
	{
		success = biped_fix_position(unit_index, source_unit_index, position, NULL, 2.f, FALSE, FALSE, TRUE);
	}

	player->cluster_index = NONE;

	if (success)
	{
		struct scenario *scenario = global_scenario_get();
		short trigger_index;

		match_assert("c:\\halo\\SOURCE\\game\\players.c", 1386, player->unit_index!=NONE);

		for (trigger_index = 0; trigger_index < scenario->bsp_switch_trigger_volumes.count; trigger_index++)
		{
			struct scenario_bsp_switch_trigger_volume *switch_trigger = TAG_BLOCK_GET_ELEMENT(&scenario->bsp_switch_trigger_volumes, trigger_index, struct scenario_bsp_switch_trigger_volume);

			if (switch_trigger->source_bsp_index == global_structure_bsp_index && scenario_trigger_volume_test_object(switch_trigger->trigger_volume_index, player->unit_index))
			{
				success = FALSE;
				break;
			}
		}
	}

	if (success)
	{
		biped->object.translational_velocity = *global_zero_vector3d;

		if (source_unit_index != NONE)
		{
			real_vector3d forward = unit_get(source_unit_index)->object.forward;
			struct biped_datum *source_biped = biped_try_and_get(source_unit_index);

			if (source_biped && source_biped->biped.elevator_object_index != NONE)
			{
				biped->biped.elevator_object_index = source_biped->biped.elevator_object_index;
				biped->biped.elevator_ticks = source_biped->biped.elevator_ticks;
			}

			biped->unit.desired_facing_vector = forward;
			biped->unit.desired_aiming_vector = forward;
			biped->unit.desired_looking_vector = forward;

			if (player->local_player_index != NONE)
			{
				player_control_set_facing(player->local_player_index, &forward);
			}
		}

		if (source_unit_index != NONE)
		{
			long effect_definition_index = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information)->coop_respawn_effect.index;

			if (effect_definition_index != NONE)
			{
				players_compute_combined_pvs(players_globals->combined_pvs, FALSE);
				effect_new_from_object(effect_definition_index, unit_index, unit_index, NONE, 0.f, 0.f, NULL, NULL);
			}
		}
	}
	else
	{
		error(_error_silent, "couldn't teleport player into a valid location");
		match_assert("c:\\halo\\SOURCE\\game\\players.c", 1451, player->local_player_index!=NONE);
		player_pseudo_kill(player_index, source_unit_index);
	}

	return success;
}

boolean player_teleport(
	long player_index,
	long source_unit_index,
	real_point3d const *position)
{
	struct player_datum *player = player_get(player_index);
	long unit_index = player->unit_index;
	struct biped_datum *biped = biped_try_and_get(unit_index);
	boolean success = FALSE;

	if (biped)
	{
		if (biped->object.parent_object_index != NONE)
		{
			unit_exit_seat_end(unit_index);
		}

		success = player_teleport_internal(player_index, source_unit_index, position);
	}

	return success;
}

static void player_pseudo_kill(
	long player_index,
	long source_unit_index)
{
	struct player_datum *player = player_get(player_index);

	if (player->unit_index != NONE)
	{
		long unit_index;
		struct unit_datum *unit;
		long weapon_index;

		if (game_engine_can_score())
		{
			game_statistics_record_kill(player->unit_index, NONE, NONE, NONE);
		}

		players_globals->local_player_failed_teleport_unit_indices[player->local_player_index] = player->unit_index;
		player_died(player_index);
		unit_index = players_globals->local_player_failed_teleport_unit_indices[player->local_player_index];
		unit = unit_get(unit_index);
		weapon_index = unit_inventory_get_weapon(unit_index, unit_get(unit_index)->unit.current_weapon_index);
		unit->unit.player_index = NONE;
		object_deactivate(unit_index);
		object_set_visibility(unit_index, FALSE);

		if (weapon_index != NONE)
		{
			object_set_visibility(weapon_index, FALSE);
		}

		if (source_unit_index != NONE)
		{
			player->dead_unit_index = source_unit_index;
		}

		players_globals->all_dead = FALSE;
	}

	return;
}

void players_reconnect_to_structure_bsp(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;

	if (players_globals->bsp_switch_trigger_index != NONE && local_player_count() > 1)
	{
		struct scenario *scenario = global_scenario_get();
		struct scenario_bsp_switch_trigger_volume *switch_trigger = TAG_BLOCK_GET_ELEMENT(&scenario->bsp_switch_trigger_volumes, players_globals->bsp_switch_trigger_index, struct scenario_bsp_switch_trigger_volume);
		long source_unit_index = NONE;
		real_point3d good_point;
		boolean found = FALSE;
		boolean use_safe_flag = FALSE;

		if (switch_trigger->safe_flag_index != NONE)
		{
			real offset = 0.f;

			good_point = TAG_BLOCK_GET_ELEMENT(&scenario->cutscene_flags, switch_trigger->safe_flag_index, struct scenario_cutscene_flag)->position;

			while (offset < 0.3f && collision_test_point(FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_ignore_invisible_surfaces_bit) | FLAG(_collision_test_structure_bit) | FLAG(_collision_test_objects_scenery_bit), &good_point, NONE))
			{
				good_point.z += 0.05f;
				offset += 0.05f;
			}

			use_safe_flag = offset < 0.3f;
		}

		data_iterator_new(&iterator, player_data);

		while ((player = data_iterator_next(&iterator)) && !found)
		{
			if (player->unit_index != NONE && is_player_in_trigger(player->unit_index, players_globals->bsp_switch_trigger_index))
			{
				real_point3d biped_good_point;
				real dummy;
				real width;

				biped_get_physics_pill(player->unit_index, &biped_good_point, &dummy, &width);

				if ((scenario_leaf_index_from_point(&biped_good_point) == NONE ? NONE : TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, scenario_leaf_index_from_point(&biped_good_point) & LONG_MAX, struct structure_leaf)->cluster_index) != NONE)
				{
					if (!use_safe_flag)
					{
						good_point = biped_good_point;
					}
					else
					{
						good_point.z += width;
					}

					source_unit_index = player->unit_index;
					found = TRUE;
				}
			}
		}

		match_vassert("c:\\halo\\SOURCE\\game\\players.c", 1598, found, "no players in the bsp");

		if (found)
		{
			short local_player_index;

			for (local_player_index = local_player_get_next(NONE); local_player_index != NONE; local_player_index = local_player_get_next(local_player_index))
			{
				long player_index = local_player_get_player_index(local_player_index);
				struct player_datum *local_player = player_get(player_index);

				if (local_player->unit_index != NONE && local_player->unit_index != source_unit_index)
				{
					player_teleport_on_bsp_switch(player_index, source_unit_index, &good_point);
					player_get(player_index)->cluster_index = NONE;
				}
			}
		}

		players_globals->bsp_switch_trigger_index = NONE;
	}

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		player->cluster_index = NONE;
	}

	return;
}

void player_control_fix_for_loaded_game_state(
	void)
{
	short controller_index = player_ui_get_single_player_local_player_controller(0);

	if (controller_index == NONE)
	{
		controller_index = 0;
	}

	if (local_player_get_player_index(controller_index) == NONE)
	{
		if (player_spawn_count == 1)
		{
			short local_player_index;

			for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
			{
				long player_index = local_player_get_player_index(local_player_index);

				if (player_index != NONE)
				{
					struct player_datum *player = player_get(player_index);

					local_player_set_player_index(local_player_index, NONE);
					player_control_new_unit(local_player_index, NONE);
					local_player_set_player_index(controller_index, player_index);
					player_control_new_unit(controller_index, player->unit_index);
					hud_fix_weapon_data(local_player_index, controller_index);
					hud_fix_unit_data(local_player_index, controller_index);
					error(_error_silent, "corrected player control for restored saved game");
					break;
				}
			}

			if (local_player_index == MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
			{
				error(_error_silent, "failed to correct player control for restored saved game... probably won't be able to control the player");
			}
		}
		else
		{
			error(_error_silent, "tried to fix broken player control for a game w/ player_spawn_count= %d... but we don't allow restored games for anything other than player_spawn_count= 1", player_spawn_count);
		}
	}

	return;
}

short player_get_starting_location_count(
	void)
{
	struct scenario *scenario = global_scenario_get();
	short starting_location_count = (short)scenario->players.count;

	if (ai_debug.selected_encounter_index != NONE)
	{
		struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(&scenario->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_debug.selected_encounter_index), struct encounter_definition);

		if (encounter->player_starting_locations.count > 0)
		{
			starting_location_count = (short)encounter->player_starting_locations.count;
		}
	}

	return starting_location_count;
}

struct scenario_player *player_get_starting_location(
	short starting_location_index)
{
	struct scenario *scenario = global_scenario_get();
	struct scenario_player *starting_location = NULL;

	if (starting_location_index >= 0 && starting_location_index < scenario->players.count)
	{
		starting_location = TAG_BLOCK_GET_ELEMENT(&scenario->players, starting_location_index, struct scenario_player);
	}

	if (ai_debug.selected_encounter_index != NONE)
	{
		struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(&scenario->ai_encounters, DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_debug.selected_encounter_index), struct encounter_definition);

		if (starting_location_index >= 0 && starting_location_index < encounter->player_starting_locations.count)
		{
			starting_location = TAG_BLOCK_GET_ELEMENT(&encounter->player_starting_locations, starting_location_index, struct scenario_player);

			if (encounter->runtime_structure_bsp_reference_index >= 0 && encounter->runtime_structure_bsp_reference_index < scenario->structure_bsp_references.count)
			{
				starting_location->bsp_index = encounter->runtime_structure_bsp_reference_index;
			}
		}
	}

	return starting_location;
}

void placement_data_set_change_color(
	struct object_placement_data *placement_data,
	real_rgb_color const *color)
{
	long change_color_index;

	for (change_color_index = 0; change_color_index < NUMBEROF(placement_data->change_colors); change_color_index++)
	{
		placement_data->change_colors[change_color_index] = *color;
	}

	return;
}

long find_best_starting_location_index(
	long player_index)
{
	short starting_location_count = player_get_starting_location_count();
	short best_starting_location_index = NONE;
	real best_rating = 0.f;
	short starting_location_index;

	for (starting_location_index = 0; starting_location_index < starting_location_count; starting_location_index++)
	{
		struct scenario_player *starting_location = player_get_starting_location(starting_location_index);
		real rating = game_engine_get_starting_location_rating(player_index, starting_location);

		rating *= pow(real_random_range(0.f, 1.f), 0.5);

		if (rating > best_rating)
		{
			best_starting_location_index = starting_location_index;
			best_rating = rating;
		}
	}

	return best_starting_location_index;
}

static long create_weapon(
	struct starting_weapon_info const *weapon_info,
	long unit_index)
{
	long weapon_index = NONE;

	if (weapon_info->weapon.index != NONE)
	{
		struct object_placement_data placement_data;

		object_placement_data_new(&placement_data, weapon_info->weapon.index, unit_index);
		weapon_index = object_new(&placement_data);

		if (weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(weapon_index);

			weapon->weapon.magazines[0].rounds_total = weapon_info->rounds_total;
			weapon->weapon.magazines[0].rounds_loaded = weapon_info->rounds_loaded;
		}
	}

	return weapon_index;
}

static void player_spawn(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	long unit_index = NONE;

	if (!game_engine_running() && player->local_player_index != NONE)
	{
		unit_index = players_globals->local_player_failed_teleport_unit_indices[player->local_player_index];
		players_globals->local_player_failed_teleport_unit_indices[player->local_player_index] = NONE;

		if (unit_index != NONE && TEST_FLAG(unit_get(unit_index)->object.damage_flags, _object_dead_bit))
		{
			object_delete(unit_index);
			unit_index = NONE;
		}
	}

	if (!game_engine_running() && unit_index != NONE)
	{
		long weapon_index = unit_inventory_get_weapon(unit_index, unit_get(unit_index)->unit.current_weapon_index);

		match_assert("c:\\halo\\SOURCE\\game\\players.c", 1850, player->local_player_index!=NONE);

		object_activate(unit_index);
		object_set_visibility(unit_index, TRUE);
		players_set_local_player_unit(player->local_player_index, unit_index);

		if (weapon_index != NONE)
		{
			object_set_visibility(weapon_index, TRUE);
		}
	}
	else
	{
		short starting_location_count = player_get_starting_location_count();
		short starting_location_index = (short)find_best_starting_location_index(player_index);

		if (starting_location_index != NONE)
		{
			struct game_globals *game_globals = scenario_get_game_globals();
			struct game_globals_player_information *player_information = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information);

			if (player_information->player_unit.index != NONE)
			{
				struct scenario_player *starting_location = player_get_starting_location(starting_location_index);
				struct object_placement_data placement_data;
				real_rgb_color change_color;
				long spawned_unit_index;

				object_placement_data_new(&placement_data, game_engine_running() ? TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0, struct game_globals_multiplayer_information)->player_unit.index : player_information->player_unit.index, NONE);
				placement_data.position = starting_location->position;
				vector3d_from_angle(&placement_data.forward, starting_location->facing);
				placement_data.up = *global_up3d;
				change_color = game_engine_player_get_change_color(player_index);
				placement_data_set_change_color(&placement_data, &change_color);
				spawned_unit_index = object_new(&placement_data);

				if (spawned_unit_index != NONE)
				{
					struct unit_datum *unit = unit_try_and_get(spawned_unit_index);

					if (unit)
					{
						struct player_datum *spawned_player = player_get(player_index);

						unit->object.owner_player_index = player_index;
						unit->object.owner_team_index = (short)spawned_player->team_index;
						unit->unit.player_index = player_index;
						spawned_player->unit_index = spawned_unit_index;
						unit_set_actively_controlled(spawned_unit_index, TRUE);

						if (spawned_player->local_player_index != NONE)
						{
							player_control_new_unit(spawned_player->local_player_index, spawned_unit_index);
						}

						if (!game_engine_running())
						{
							struct scenario *spawn_scenario = global_scenario_get();

							if (spawn_scenario->starting_profiles.count > 1 && spawned_player->statistics.deaths > 0)
							{
								player_add_equipment(spawned_player->unit_index, 1, TRUE);
							}
							else if (spawn_scenario->starting_profiles.count)
							{
								player_add_equipment(spawned_player->unit_index, 0, TRUE);
							}
						}
					}
				}
			}
		}
	}

	memset(player->powerup_durations, 0, sizeof(player->powerup_durations));
	player_reset_action_result(player_index);

	if (player->local_player_index != NONE)
	{
		observer_obsolete_position(player->local_player_index);
	}

	return;
}

static void players_compute_combined_pvs(
	unsigned long *combined_pvs,
	boolean local_only)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	memset(combined_pvs, 0, BIT_VECTOR_SIZE_IN_BYTES(MAXIMUM_CLUSTERS_PER_STRUCTURE));

	if (!game_in_editor())
	{
		short activating_cluster_index = objects_get_activating_cluster_index();
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);

		while (player = data_iterator_next(&iterator))
		{
			if (!local_only || player->local_player_index != NONE)
			{
				if (player->unit_index != NONE)
				{
					long parent_index = object_get_ultimate_parent(player->unit_index);
					struct object_datum *parent = object_get(parent_index);

					if (parent->object.location.cluster_index != NONE)
					{
						player->cluster_index = parent->object.location.cluster_index;
					}
				}

				if (player->cluster_index != NONE)
				{
					bit_vector_or((short)structure_bsp->clusters.count, combined_pvs, structure_bsp_get_cluster_pvs(structure_bsp, player->cluster_index), combined_pvs);
				}
			}
		}

		if (activating_cluster_index != NONE)
		{
			bit_vector_or((short)structure_bsp->clusters.count, combined_pvs, structure_bsp_get_cluster_pvs(structure_bsp, activating_cluster_index), combined_pvs);
		}
	}
	else
	{
		if (scenario_leaf_index_from_point(&observer_get_camera(0)->position) != NONE)
		{
			short cluster_index = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, scenario_leaf_index_from_point(&observer_get_camera(0)->position) & LONG_MAX, struct structure_leaf)->cluster_index;

			if (cluster_index != NONE)
			{
				bit_vector_or((short)structure_bsp->clusters.count, combined_pvs, structure_bsp_get_cluster_pvs(structure_bsp, cluster_index), combined_pvs);
			}
		}
	}

	return;
}

static short players_compute_local_player_count(
	void)
{
	short local_player_index;
	short local_player_count = 0;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		if (players_globals->local_player_indices[local_player_index] != NONE)
		{
			local_player_count++;
		}
	}

	return local_player_count;
}

static void player_examine_nearby_objects(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	if (player->unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(player->unit_index);

		if (unit->object.parent_object_index == NONE)
		{
			unsigned long object_mask = _object_mask_biped | _object_mask_vehicle | _object_mask_weapon | _object_mask_equipment | _object_mask_garbage | _object_mask_control;
			long object_indices[16];
			short object_count = objects_in_sphere(0, object_mask, &unit->object.location, &unit->object.bounding_sphere_center, unit->object.bounding_sphere_radius, object_indices, NUMBEROF(object_indices));
			short object_index;

			for (object_index = 0; object_index < object_count; object_index++)
			{
				struct object_datum *object = object_get(object_indices[object_index]);

				switch (object->object.type)
				{
				case _object_type_biped:
					player_examine_nearby_unit(player_index, object_indices[object_index]);
					break;
				case _object_type_vehicle:
					player_examine_nearby_vehicle(player_index, object_indices[object_index]);
					break;
				case _object_type_weapon:
				case _object_type_equipment:
					player_examine_nearby_item(player_index, object_indices[object_index]);
					break;
				case _object_type_control:
					player_examine_nearby_device(player_index, object_indices[object_index]);
					break;
				}
			}
		}
	}

	return;
}

static void player_examine_nearby_vehicle(
	long player_index,
	long vehicle_index)
{
	struct player_datum *player = player_get(player_index);
	struct vehicle_datum *vehicle = vehicle_get(vehicle_index);

	if (!TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
	{
		struct game_globals_player_control *player_control = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_control, 0, struct game_globals_player_control);
		real minimum_up = cosine(_pi / 2.f - player_control->minimum_vehicle_flipping_angle);

		if (vehicle_get(vehicle_index)->object.up.k > minimum_up)
		{
			if (!unit_overcharged(player->unit_index) &&
				magnitude_squared3d(&unit_get(player->unit_index)->object.translational_velocity) < 0.01f &&
				magnitude_squared3d(&vehicle_get(vehicle_index)->object.angular_velocity) < 0.01f)
			{
				short seat_index = NONE;
				short seat_result = unit_find_nearby_seat(player->unit_index, vehicle_index, &seat_index);

				switch (seat_result)
				{
				case _unit_nearby_seat_result_empty:
					match_assert("c:\\halo\\SOURCE\\game\\players.c", 2108, seat_index != NONE);
					player_set_action_result(player_index, _player_action_result_enter_vehicle, vehicle_index, seat_index);
					break;

				case _unit_nearby_seat_result_occupied_by_friend:
					match_assert("c:\\halo\\SOURCE\\game\\players.c", 2113, seat_index != NONE);
					player_set_action_result(player_index, _player_action_result_evict_from_vehicle, vehicle_index, seat_index);
					break;
				}
			}
		}
		else if (!TEST_FLAG(vehicle->vehicle.flags, _vehicle_upending_bit) && vehicle->unit.driver_object_index == NONE)
		{
			player_set_action_result(player_index, _player_action_result_flip_vehicle, vehicle_index, NONE);
		}
	}

	return;
}

static void player_examine_nearby_unit(
	long player_index,
	long unit_index)
{
	return;
}

static void player_examine_nearby_device(
	long player_index,
	long device_index)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = unit_get(player->unit_index);
	struct device_datum *device = device_get(device_index);
	real_point3d camera;

	unit_get_camera_position(player->unit_index, &camera);

	if (fast_vector_intersects_sphere(&camera, &unit->unit.aiming_vector, &device->object.bounding_sphere_center, (real)(device->object.bounding_sphere_radius)) &&
		device_frontfacing(device_index, &camera, &unit->unit.aiming_vector) &&
		device_can_change_position(device_index))
	{
		player_set_action_result(player_index, _player_action_result_touch_device, device_index, NONE);
	}

	return;
}

boolean unit_should_autopick_weapon(
	long unit_index,
	long weapon_index)
{
	struct weapon_datum *weapon = weapon_try_and_get(weapon_index);
	struct weapon_definition *definition = weapon_definition_get(weapon->definition_index);
	long weapon_count = unit_get_weapon_count(unit_index);

	boolean autopick = (unit_approve_weapon_pickup(unit_index, weapon_index) && TEST_FLAG(definition->weapon.flags, _weapon_doesnt_count_toward_maximum_bit)) ||
		weapon_count == 0 ||
		(!game_engine_running() && unit_approve_weapon_pickup(unit_index, weapon_index) && weapon_count < MAXIMUM_WEAPONS_PER_PLAYER) ||
		game_engine_force_autopickup(unit_index, weapon_index);

	return autopick;
}

static void player_examine_nearby_item(
	long player_index,
	long item_index)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = unit_get(player->unit_index);
	struct item_datum *item = item_get(item_index);

	if (item->object.parent_object_index == NONE && item->item.ignore_object_index != player->unit_index)
	{
		struct equipment_datum *equipment;
		struct weapon_datum *weapon;
		short weapon_index;

		for (weapon_index = 0; weapon_index < MAXIMUM_WEAPONS_PER_UNIT; weapon_index++)
		{
			long unit_weapon_index = unit->unit.weapon_object_indices[weapon_index];
			short rounds_picked_up;

			if (unit_weapon_index != NONE && weapon_handle_potential_inventory_item(unit_weapon_index, item_index, player->local_player_index, &rounds_picked_up))
			{
				if (rounds_picked_up > 0)
				{
					hud_picked_up_ammunition(player->local_player_index, weapon_get(unit_weapon_index)->definition_index, rounds_picked_up);
				}
				break;
			}
		}

		equipment = equipment_try_and_get(item_index);

		if (equipment)
		{
			struct equipment_definition *definition = equipment_definition_get(equipment->definition_index);

			if (definition->equipment.powerup_type == _equipment_powerup_grenade)
			{
				if (unit_add_grenade_to_inventory(player->unit_index, item_index))
				{
					hud_picked_up_grenade(player->local_player_index, equipment->definition_index);
				}
			}
			else if (definition->equipment.powerup_type != _equipment_powerup_none)
			{
				long current_equipment_index = unit_get_current_equipment(player->unit_index);

				if (current_equipment_index == NONE)
				{
					player_handle_powerup_equipment(player_index, item_index);
				}
				else
				{
					struct equipment_datum *current_equipment = equipment_get(current_equipment_index);
					struct equipment_definition *current_definition = equipment_definition_get(equipment->definition_index);

					if (definition->equipment.powerup_type != current_definition->equipment.powerup_type)
					{
						player_set_action_result(player_index, _player_action_result_swap_for_powerup, item_index, NONE);
					}
				}
			}
		}

		weapon = weapon_try_and_get(item_index);

		if (weapon && unit_can_use_weapon(player->unit_index, item_index))
		{
			struct weapon_definition *definition = weapon_definition_get(weapon->definition_index);
			boolean firing = FALSE;
			long current_weapon_index;
			long weapon_count;
			boolean current_weapon_doesnt_count;

			if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit) || TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_secondary_trigger_bit))
			{
				firing = TRUE;
			}

			current_weapon_index = unit_inventory_get_weapon(player->unit_index, unit_get(player->unit_index)->unit.current_weapon_index);
			weapon_count = unit_get_weapon_count(player->unit_index);
			current_weapon_doesnt_count = FALSE;

			if (weapon_count >= MAXIMUM_WEAPONS_PER_PLAYER && current_weapon_index != NONE && !TEST_FLAG(definition->weapon.flags, _weapon_doesnt_count_toward_maximum_bit) && TEST_FLAG(weapon_definition_get(weapon_get(current_weapon_index)->definition_index)->weapon.flags, _weapon_doesnt_count_toward_maximum_bit))
			{
				current_weapon_doesnt_count = TRUE;
			}

			if (!firing || !TEST_FLAG(definition->weapon.flags, _weapon_must_be_readied_bit))
			{
				if (unit_should_autopick_weapon(player->unit_index, item_index))
				{
					if (unit_add_weapon_to_inventory(player->unit_index, item_index, _unit_add_weapon_as_current_weapon))
					{
						hud_picked_up_weapon(player->local_player_index, weapon_get(item_index)->definition_index);
						player_control_unzoom(player->unit_index);
					}
				}
				else if (!current_weapon_doesnt_count && unit_approve_weapon_swap(player->unit_index, item_index))
				{
					struct weapon_datum *current_weapon = weapon_try_and_get(current_weapon_index);

					if (weapon_count == 1 && current_weapon && current_weapon->definition_index != weapon->definition_index)
					{
						player_set_action_result(player_index, _player_action_result_add_weapon_to_inventory, item_index, NONE);
					}
					else
					{
						player_set_action_result(player_index, _player_action_result_swap_for_weapon, item_index, NONE);
					}
				}
			}
		}
	}

	return;
}

static void player_reset_action_result(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	player->action_result = _player_action_result_reload;
	player->action_object_index = NONE;

	return;
}

static void player_set_action_result(
	long player_index,
	short action_result,
	long action_object_index,
	short action_seat_index)
{
	struct player_datum *player = player_get(player_index);
	boolean replace = FALSE;

	if (action_result == _player_action_result_flip_vehicle)
	{
		replace = TRUE;
	}
	else if (action_result == player->action_result)
	{
		real_point3d *unit_position = &object_get(player->unit_index)->object.position;
		real_point3d *old_position = &object_get(player->action_object_index)->object.position;
		real_point3d *new_position = &object_get(action_object_index)->object.position;

		replace = distance3d(unit_position, old_position) > distance3d(unit_position, new_position);
	}
	else
	{
		if (action_result > player->action_result)
		{
			replace = TRUE;
		}
	}

	if (replace)
	{
		player->action_result = action_result;
		player->action_object_index = action_object_index;
		player->action_seat_index = action_seat_index;
	}

	return;
}

static boolean player_handle_weapon_swap(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = unit_get(player->unit_index);
	boolean handled = FALSE;

	switch (player->action_result)
	{
	case _player_action_result_swap_for_weapon:
		if (unit_drop_current_weapon(player->unit_index, TRUE) && unit_add_weapon_to_inventory(player->unit_index, player->action_object_index, _unit_add_weapon_as_current_weapon))
		{
			hud_picked_up_weapon(player->local_player_index, weapon_get(player->action_object_index)->definition_index);
			player_control_unzoom(player->unit_index);
		}

		handled = TRUE;
		break;
	case _player_action_result_add_weapon_to_inventory:
		if (unit_add_weapon_to_inventory(player->unit_index, player->action_object_index, _unit_add_weapon_as_current_weapon))
		{
			hud_picked_up_weapon(player->local_player_index, weapon_get(player->action_object_index)->definition_index);
		}
		break;
	}

	return handled;
}

static boolean player_handle_action(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = unit_get(player->unit_index);
	boolean handled = FALSE;

	switch (player->action_result)
	{
	case _player_action_result_touch_device:
		device_touched(player->action_object_index, player->unit_index);
		handled = TRUE;
		break;
	case _player_action_result_swap_for_powerup:
		unit_drop_current_equipment(player->unit_index);

		if (unit_add_equipment_to_inventory(player->unit_index, player->action_object_index, 0))
		{
			hud_picked_up_powerup(player->local_player_index, equipment_get(player->action_object_index)->definition_index);
		}

		handled = TRUE;
		break;
	case _player_action_result_enter_vehicle:
	case _player_action_result_evict_from_vehicle:
	{
		long occupant_unit_index = NONE;

		if (unit_can_enter_seat(player->unit_index, player->action_object_index, player->action_seat_index, &occupant_unit_index))
		{
			unit_enter_seat(player->unit_index, player->action_object_index, player->action_seat_index);
		}
		else if (occupant_unit_index != NONE)
		{
			struct unit_datum *occupant_unit = unit_get(occupant_unit_index);

			if (occupant_unit->unit.actor_index != NONE)
			{
				ai_try_vehicle_eviction(occupant_unit->unit.actor_index, player->unit_index, TRUE);
			}
		}
		break;
	}
	case _player_action_result_flip_vehicle:
	{
		struct unit_datum *player_unit = unit_get(player->unit_index);
		struct vehicle_datum *vehicle = vehicle_get(player->action_object_index);

		player_unit->unit.last_vehicle_index = player->action_object_index;
		player_unit->unit.game_time_at_last_vehicle_exit = game_time_get();

		if (fabs(vehicle->object.forward.k) > 0.70710677f)
		{
			vehicle->vehicle.upending_type = _vehicle_upending_along_left + (vehicle->object.forward.k < 0.f);
		}
		else
		{
			real_matrix4x3 vehicle_matrix;
			real_matrix4x3 player_matrix;
			real_vector3d vector;

			vector_from_points3d(&object_get_world_matrix(player->unit_index, &player_matrix)->position, &object_get_world_matrix(player->action_object_index, &vehicle_matrix)->position, &vector);
			cross_product3d(global_up3d, &vector, &vector);
			vehicle->vehicle.upending_type = _vehicle_upending_along_back + (dot_product3d(&vector, &vehicle->object.forward) > 0.f);
		}

		SET_FLAG(vehicle->vehicle.flags, _vehicle_upending_bit, TRUE);
		vehicle->vehicle.upending_ticks = 0;
		handled = TRUE;
		break;
	}
	}

	return handled;
}

static void player_over_shield_screen_effect(
	long player_index)
{
	static short screen_flash_type = _screen_flash_type_invert;
	static real max_intensity = 1.f;
	static short fade_function = _transition_function_linear;
	static real alpha = 0.f;
	static real red = 0.8f;
	static real green = 0.f;
	static real blue = 0.8f;
	static real duration = 2.f;

	if (player_index != NONE && player_get(player_index)->local_player_index != NONE)
	{
		struct screen_flash_definition screen_flash = { 0 };

		screen_flash.fade_function = fade_function;
		screen_flash.type = screen_flash_type;
		screen_flash.priority = _screen_flash_high_priority;
		screen_flash.duration = duration;
		screen_flash.max_intensity = max_intensity;
		screen_flash.zero_scale_factor = 0.f;
		screen_flash.screen_flash_color.alpha = alpha;
		screen_flash.screen_flash_color.red = red;
		screen_flash.screen_flash_color.green = green;
		screen_flash.screen_flash_color.blue = blue;
		player_effect_screen_flash(player_index, &screen_flash, 1.f);
	}

	return;
}

static void player_active_camo_screen_effect(
	long player_index)
{
	static short screen_flash_type = _screen_flash_type_darken;
	static real max_intensity = 1.f;
	static short fade_function = _transition_function_linear;
	static real alpha = 0.f;
	static real red = 0.35f;
	static real green = 0.35f;
	static real blue = 0.f;
	static real duration = 2.f;

	if (player_index != NONE && player_get(player_index)->local_player_index != NONE)
	{
		struct screen_flash_definition screen_flash = { 0 };

		screen_flash.fade_function = fade_function;
		screen_flash.type = screen_flash_type;
		screen_flash.priority = _screen_flash_high_priority;
		screen_flash.duration = duration;
		screen_flash.max_intensity = max_intensity;
		screen_flash.zero_scale_factor = 0.f;
		screen_flash.screen_flash_color.alpha = alpha;
		screen_flash.screen_flash_color.red = red;
		screen_flash.screen_flash_color.green = green;
		screen_flash.screen_flash_color.blue = blue;
		player_effect_screen_flash(player_index, &screen_flash, 1.f);
	}

	return;
}

static void player_health_pack_screen_effect(
	long player_index)
{
	if (player_index != NONE && player_get(player_index)->local_player_index != NONE)
	{
		struct screen_flash_definition screen_flash = { 0 };

		screen_flash.fade_function = _transition_function_early;
		screen_flash.type = _screen_flash_type_tint;
		screen_flash.priority = _screen_flash_high_priority;
		screen_flash.duration = 2.f;
		screen_flash.max_intensity = 0.5f;
		screen_flash.zero_scale_factor = 0.f;
		screen_flash.screen_flash_color.alpha = 1.f;
		screen_flash.screen_flash_color.red = 0.917647f;
		screen_flash.screen_flash_color.green = 0.917647f;
		screen_flash.screen_flash_color.blue = 0.917647f;
		player_effect_screen_flash(player_index, &screen_flash, 1.f);
	}

	return;
}

static void player_handle_powerup_equipment(
	long player_index,
	long equipment_index)
{
	struct player_datum *player = player_get(player_index);
	struct equipment_definition *definition = equipment_definition_get(equipment_get(equipment_index)->definition_index);
	short duration = (short)(definition->equipment.powerup_duration * TICKS_PER_SECOND);

	if (duration > 0)
	{
		boolean picked_up = TRUE;

		if (definition->equipment.powerup_type == _equipment_powerup_double_speed)
		{
			players_globals->players_are_double_speed_duration += duration;
			game_set_players_are_double_speed(TRUE);
		}
		else if (definition->equipment.powerup_type == _equipment_powerup_over_shield)
		{
			picked_up = object_double_charge_shield(player->unit_index);

			if (picked_up)
			{
				player_over_shield_screen_effect(player_index);
			}
		}
		else if (definition->equipment.powerup_type == _equipment_powerup_health)
		{
			picked_up = object_restore_body(player->unit_index);

			if (picked_up)
			{
				player_health_pack_screen_effect(player_index);
			}
		}
		else
		{
			short player_powerup_type;

			switch (definition->equipment.powerup_type)
			{
			case _equipment_powerup_active_camouflage:
				player_powerup_type = _player_powerup_active_camouflage;
				break;
			case _equipment_powerup_full_spectrum_vision:
				player_powerup_type = _player_powerup_full_spectrum_vision;
				break;
			default:
				match_halt("c:\\halo\\SOURCE\\game\\players.c", 2763);
			}

			picked_up = player_handle_powerup(player_index, player_powerup_type, duration);

			if (picked_up && player_powerup_type == _player_powerup_active_camouflage)
			{
				player_active_camo_screen_effect(player_index);
			}
		}

		if (picked_up)
		{
			hud_picked_up_powerup(player->local_player_index, equipment_get(equipment_index)->definition_index);

			if (player->local_player_index != NONE)
			{
				equipment_handle_pickup(equipment_index);
			}

			object_delete(equipment_index);
		}
	}

	return;
}

boolean player_handle_powerup(
	long player_index,
	short powerup_type,
	short duration_in_ticks)
{
	struct player_datum *player = player_get(player_index);
	boolean handled = FALSE;

	match_assert("c:\\halo\\SOURCE\\game\\players.c", 2798, powerup_type>=0 && powerup_type<NUMBER_OF_PLAYER_POWERUPS);

	if (powerup_type == _player_powerup_active_camouflage)
	{
		long unit_index = player->unit_index;
		struct unit_datum *unit = unit_get(unit_index);

		if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
		{
			return FALSE;
		}
	}

	if (player->powerup_durations[powerup_type] == 0)
	{
		player_powerup_on(player_index, powerup_type);
	}
	else if (!game_engine_running())
	{
		player_powerup_additional(player_index, powerup_type);
	}

	player->powerup_durations[powerup_type] += duration_in_ticks;
	handled = TRUE;

	return handled;
}

void player_handle_powerup_minor(
	long player_index,
	short powerup_type,
	short minor_duration)
{
	struct player_datum *player = player_get(player_index);

	match_assert("c:\\halo\\SOURCE\\game\\players.c", 2841, powerup_type>=0 && powerup_type<NUMBER_OF_PLAYER_POWERUPS);

	if (player->powerup_durations[powerup_type] == 0)
	{
		player_powerup_on(player_index, powerup_type);
	}

	player->powerup_durations[powerup_type] = MAX(player->powerup_durations[powerup_type], minor_duration);

	return;
}

static void player_update_powerups(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	short powerup_type;

	for (powerup_type = 0; powerup_type < NUMBER_OF_PLAYER_POWERUPS; powerup_type++)
	{
		if (player->powerup_durations[powerup_type] > 0)
		{
			player->powerup_durations[powerup_type]--;

			if (player->powerup_durations[powerup_type] == 0)
			{
				player_powerup_off(player_index, powerup_type);
			}
		}
	}

	return;
}

static void player_powerup_on(
	long player_index,
	short powerup_type)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = unit_get(player->unit_index);

	switch (powerup_type)
	{
	case _player_powerup_active_camouflage:
		SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, TRUE);
		unit->unit.cause_for_camo_regrowth = 0;
		break;
	}

	return;
}

static void player_powerup_additional(
	long player_index,
	short powerup_type)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = unit_get(player->unit_index);

	switch (powerup_type)
	{
	case _player_powerup_active_camouflage:
		SET_FLAG(unit->unit.flags, _unit_super_camouflaged_bit, TRUE);
		break;
	}

	return;
}

static void player_powerup_off(
	long player_index,
	short powerup_type)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = unit_get(player->unit_index);

	switch (powerup_type)
	{
	case _player_powerup_active_camouflage:
		SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, FALSE);
		break;
	}

	return;
}

void players_handle_deleted_object(
	long object_index)
{
	struct object_datum *object = object_get(object_index);

	if (TEST_FLAG(_object_mask_unit, object->object.type))
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);

		while (player = data_iterator_next(&iterator))
		{
			if (player->unit_index == object_index)
			{
				player_died(iterator.index);
			}
		}
	}

	return;
}

boolean debug_render_player_teleport = FALSE;

void players_debug_render(
	void)
{
	if (debug_render_player_teleport)
	{
		short iteration;
		short local_player_index;

		for (iteration = 0, local_player_index = local_player_get_next(NONE); iteration < 2 && local_player_index != NONE; iteration++, local_player_index = local_player_get_next(local_player_index))
		{
			long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;

			if (unit_index != NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);
				real_point3d good_point;
				real dummy;

				biped_get_physics_pill(unit_index, &good_point, &dummy, &dummy);

				if (biped_fix_position(NONE, unit_index, &good_point, &good_point, 2.f, FALSE, TRUE, TRUE))
				{
					struct biped_definition *definition = biped_definition_get(unit->definition_index);
					real_point3d base;
					real height;
					real width;
					real_vector3d height_vector;
					struct collision_result collision;

					biped_get_physics_pill(unit_index, &base, &height, &width);
					good_point.z += definition->biped.collision_radius;
					scale_vector3d(global_up3d, height, &height_vector);

					if (collision_test_pill(FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_ignore_invisible_surfaces_bit) | FLAG(_collision_test_structure_bit) | FLAG(_collision_test_objects_scenery_bit), &good_point, &height_vector, width, unit_index, &collision))
					{
						render_debug_pill(FALSE, &good_point, &height_vector, width, global_real_argb_red);
					}
					else
					{
						render_debug_pill(FALSE, &good_point, &height_vector, width, global_real_argb_green);
					}
				}
			}
		}
	}

	return;
}

void debug_player_teleport(
	short source_local_player_index,
	short destination_local_player_index)
{
	long source_unit_index = local_player_get_player_index(source_local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(source_local_player_index))->unit_index;
	long destination_unit_index = local_player_get_player_index(destination_local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(destination_local_player_index))->unit_index;

	if (source_unit_index != NONE && destination_unit_index != NONE)
	{
		player_teleport_internal(player_index_from_unit_index(source_unit_index), destination_unit_index, &unit_get(destination_unit_index)->object.bounding_sphere_center);
	}

	return;
}

/* ---------- private code */
