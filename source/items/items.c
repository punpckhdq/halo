/*
ITEMS.C

symbols in this file:
000E60A0 0020:
	_object_get_type (0000)
000E60C0 0010:
	_items_initialize (0000)
000E60D0 0010:
	_items_initialize_for_new_map (0000)
000E60E0 0010:
	_items_dispose_from_old_map (0000)
000E60F0 0010:
	_items_dispose (0000)
000E6100 0040:
	_item_new (0000)
000E6140 0010:
	_item_delete (0000)
000E6150 0010:
	_verify_item_location (0000)
000E6160 0050:
	_dangerous_items_near_player (0000)
000E61B0 00a0:
	_item_in_unit_inventory (0000)
000E6250 0090:
	_item_get_position_even_if_in_inventory (0000)
000E62E0 0090:
	_item_detonate (0000)
000E6370 00c0:
	_item_adjust_for_angular_velocity_change (0000)
000E6430 00c0:
	_valid_real_vector3d_axes3 (0000)
000E64F0 0060:
	_valid_real_matrix4x3 (0000)
000E6550 03b0:
	_item_accelerate (0000)
000E6900 0230:
	_item_align_to_normal_and_point (0000)
000E6B30 0930:
	_item_update (0000)
00278FB0 0004:
	_rdata_00278fb0 (0000)
00278FB4 000c:
	??_C@_0M@HIOGOKBN@item_update?$AA@ (0000)
00278FC0 000d:
	??_C@_0N@MIPPJENF@ground?5point?$AA@ (0000)
00278FD0 001d:
	??_C@_0BN@FFDGPJIN@c?3?2halo?2SOURCE?2items?2items?4c?$AA@ (0000)
00278FF0 002b:
	??_C@_0CL@EEILLIKO@valid_real_matrix4x3?$CI?$CGground_poi@ (0000)
0027901C 0008:
	??_C@_07GAHNFPHG@scale?$DO0?$AA@ (0000)
00279024 0004:
	__real@3f350481 (0000)
00306498 05f8:
	_data_00306498 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "items.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "actor_definitions.h"
#include "network_messages.h"
#include "collisions.h"
#include "sound_manager.h"
#include "meter_definitions.h"
#include "game_sound.h"
#include "structures.h"
#include "physics_constants.h"
#include "editor_stubs.h"
#include "effects.h"
#include "physics.h"
#include "material_effects.h"
#include "material_effect_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
