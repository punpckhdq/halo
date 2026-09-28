/*
PARTICLE_SYSTEMS.C

symbols in this file:
0008DCF0 0040:
	_particle_systems_initialize (0000)
0008DD30 0020:
	_particle_systems_initialize_for_new_map (0000)
0008DD50 0030:
	_particle_system_orphan (0000)
0008DD80 00b0:
	_code_0008dd80 (0000)
0008DE30 0010:
	_particle_systems_dispose (0000)
0008DE40 0010:
	_particle_systems_disconnect_from_structure_bsp (0000)
0008DE50 0140:
	_particle_systems_reconnect_to_structure_bsp (0000)
0008DF90 00b0:
	_code_0008df90 (0000)
0008E040 0090:
	_code_0008e040 (0000)
0008E0D0 0070:
	_code_0008e0d0 (0000)
0008E140 0040:
	_code_0008e140 (0000)
0008E180 0190:
	_code_0008e180 (0000)
0008E310 0030:
	_code_0008e310 (0000)
0008E340 0060:
	_particle_systems_dispose_from_old_map (0000)
0008E3A0 0350:
	_code_0008e3a0 (0000)
0008E6F0 0100:
	_code_0008e6f0 (0000)
0008E7F0 0680:
	_code_0008e7f0 (0000)
0008EE70 0550:
	_code_0008ee70 (0000)
0008F3C0 0110:
	_code_0008f3c0 (0000)
0008F4D0 0170:
	_code_0008f4d0 (0000)
0008F640 0120:
	_code_0008f640 (0000)
0008F760 0080:
	_particle_systems_update (0000)
0008F7E0 00a0:
	_particle_systems_render (0000)
0008F880 00d0:
	_particle_system_new_unattached (0000)
0008F950 01a0:
	_particle_system_new_attached (0000)
0025A6B8 0020:
	_rdata_0025a6b8 (0000)
	_ground_error (0018)
0025A6D8 001a:
	??_C@_0BK@BEJKBDOP@particle?5system?5particles?$AA@ (0000)
0025A6F4 0011:
	??_C@_0BB@DDOBODPO@particle?5systems?$AA@ (0000)
0025A708 0066:
	??_C@_0GG@NBDBKOPN@creation_function_index?$DO?$DN0?5?$CG?$CG?5cr@ (0000)
0025A770 0009:
	??_C@_08PKKGOGAD@particle?$AA@ (0000)
0025A77C 002a:
	??_C@_0CK@PCEEHMOG@c?3?2halo?2SOURCE?2effects?2particle_@ (0000)
0025A7A8 0092:
	??_C@_0JC@KDIJANOF@type_state_definition?9?$DOparticle_@ (0000)
0025A840 0081:
	??_C@_0IB@LHKMAGCA@system_definition?9?$DOsystem_update@ (0000)
0025A8C4 002c:
	??_C@_0CM@MKEIGADN@particle_systems?5?$CG?$CG?5particle_sys@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "object_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "ai_communication.h"
#include "unicode.h"
#include "units.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "render_debug.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "physics.h"
#include "first_person_weapons.h"
#include "point_physics.h"
#include "particle_systems.h"
#include "particle_system_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
