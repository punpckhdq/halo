/*
WEATHER_PARTICLE_SYSTEMS.C

symbols in this file:
000924D0 0040:
	_weather_particle_system_get (0000)
00092510 0050:
	_weather_particle_system_get_type (0000)
00092560 0030:
	_weather_particle_systems_initialize (0000)
00092590 0070:
	_weather_particle_systems_initialize_for_new_map (0000)
00092600 0020:
	_weather_particle_systems_dispose_from_old_map (0000)
00092620 0020:
	_weather_particle_systems_dispose (0000)
00092640 0030:
	_weather_particle_system_type_delete_particle (0000)
00092670 00a0:
	_weather_particle_system_wrap_point (0000)
00092710 0160:
	_weather_particle_system_new (0000)
00092870 0110:
	_weather_particle_system_delete (0000)
00092980 02c0:
	_weather_particle_system_new_particle (0000)
00092C40 0040:
	_weather_particle_system_box_offset_from_point3d (0000)
00092C80 02b0:
	_weather_particle_update_physics (0000)
00092F30 0100:
	_weather_particle_system_build_clipping_planes (0000)
00093030 0040:
	_weather_particle_system_transform_clip_planes_to_box (0000)
00093070 00b0:
	_weather_polyhedra_find (0000)
00093120 0130:
	_weather_particle_system_update_particle_count (0000)
00093250 0240:
	_weather_particle_system_update (0000)
00093490 0690:
	_weather_particle_system_render (0000)
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
#include "weather_particle_systems.h"
#include "weather_particle_definitions.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "physics.h"
#include "point_physics.h"
#include "fog_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
