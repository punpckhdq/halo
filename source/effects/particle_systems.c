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
	_particle_system_delete (0000)
0008DE30 0010:
	_particle_systems_dispose (0000)
0008DE40 0010:
	_particle_systems_disconnect_from_structure_bsp (0000)
0008DE50 0140:
	_particle_systems_reconnect_to_structure_bsp (0000)
0008DF90 00b0:
	_particle_system_next_type_state_index (0000)
0008E040 0090:
	_particle_system_next_particle_state_index (0000)
0008E0D0 0070:
	_particle_system_update_default (0000)
0008E140 0040:
	_particle_system_new_particle_default (0000)
0008E180 0190:
	_particle_system_update_particle_default (0000)
0008E310 0030:
	_particle_system_update_explosion (0000)
0008E340 0060:
	_particle_systems_dispose_from_old_map (0000)
0008E3A0 0350:
	_particle_system_new_particles (0000)
0008E6F0 0100:
	_randomize_particle_variables (0000)
0008E7F0 0680:
	_particle_system_update (0000)
0008EE70 0550:
	_particle_system_render (0000)
0008F3C0 0110:
	_particle_system_new_particle_explosion (0000)
0008F4D0 0170:
	_particle_system_new_particle_jet (0000)
0008F640 0120:
	_particle_system_initialize (0000)
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
#include "particle_systems.h"
#include "particle_system_definitions.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "render_debug.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "physics.h"
#include "first_person_weapons.h"
#include "point_physics.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
