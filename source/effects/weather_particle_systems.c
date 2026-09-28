/*
WEATHER_PARTICLE_SYSTEMS.C

symbols in this file:
000924D0 0040:
	_code_000924d0 (0000)
00092510 0050:
	_code_00092510 (0000)
00092560 0030:
	_weather_particle_systems_initialize (0000)
00092590 0070:
	_weather_particle_systems_initialize_for_new_map (0000)
00092600 0020:
	_weather_particle_systems_dispose_from_old_map (0000)
00092620 0020:
	_weather_particle_systems_dispose (0000)
00092640 0030:
	_code_00092640 (0000)
00092670 00a0:
	_code_00092670 (0000)
00092710 0160:
	_weather_particle_system_new (0000)
00092870 0110:
	_weather_particle_system_delete (0000)
00092980 02c0:
	_code_00092980 (0000)
00092C40 0040:
	_code_00092c40 (0000)
00092C80 02b0:
	_code_00092c80 (0000)
00092F30 0100:
	_code_00092f30 (0000)
00093030 0040:
	_code_00093030 (0000)
00093070 00b0:
	_code_00093070 (0000)
00093120 0130:
	_code_00093120 (0000)
00093250 0240:
	_code_00093250 (0000)
00093490 0690:
	_code_00093490 (0000)
00093B20 00e0:
	_weather_particle_systems_render (0000)
0025AAFC 0004:
	_one_over_char_max (0000)
0025AB00 0032:
	??_C@_0DC@LLJNAPDG@c?3?2halo?2SOURCE?2effects?2weather_p@ (0000)
0025AB34 003d:
	??_C@_0DN@CIHCKGAO@type_index?$DO?$DN0?5?$CG?$CG?5type_index?$DMdefi@ (0000)
0025AB74 0033:
	??_C@_0DD@FLIDKJLH@couldn?8t?5allocate?5weather?5partic@ (0000)
0025ABA8 0012:
	??_C@_0BC@EPMBJEML@weather?5particles?$AA@ (0000)
0025ABBC 001f:
	??_C@_0BP@EMHAEIEA@system?9?$DOdefinition_index?$DN?$DNNONE?$AA@ (0000)
0025ABDC 0024:
	??_C@_0CE@MLAKBDEP@too?5many?5weather?5polyhedra?5visib@ (0000)
0025AC00 003b:
	??_C@_0DL@JLJDEDI@box_count?$DMMAXIMUM_NUMBER_OF_VISI@ (0000)
002DDDAE 0001:
	_weather (0000)
0043D590 0274:
	_bss_0043d590 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cheats.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "object_definitions.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "objects.h"
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structure_bsp_definitions.h"
#include "render.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "physics_definitions.h"
#include "physics.h"
#include "point_physics.h"
#include "fog_definitions.h"
#include "weather_particle_systems.h"
#include "weather_particle_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
