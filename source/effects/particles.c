/*
PARTICLES.C

symbols in this file:
0008FAF0 0030:
	_particles_initialize (0000)
0008FB20 0010:
	_particles_initialize_for_new_map (0000)
0008FB30 0010:
	_particles_dispose_from_old_map (0000)
0008FB40 0020:
	_particles_dispose (0000)
0008FB60 0020:
	_particle_delete (0000)
0008FB80 0070:
	_particles_stop_on_first_person_weapon (0000)
0008FBF0 0010:
	_particles_disconnect_from_structure_bsp (0000)
0008FC00 00d0:
	_particles_reconnect_to_structure_bsp (0000)
0008FCD0 0010:
	_code_0008fcd0 (0000)
0008FCE0 0040:
	_particle_get_radius (0000)
0008FD20 0060:
	_valid_real_point3d (0000)
0008FD80 0060:
	_valid_real_argb_color (0000)
0008FDE0 0150:
	_code_0008fde0 (0000)
0008FF30 0050:
	_code_0008ff30 (0000)
0008FF80 0180:
	_code_0008ff80 (0000)
00090100 00d0:
	_code_00090100 (0000)
000901D0 00d0:
	_code_000901d0 (0000)
000902A0 03a0:
	_code_000902a0 (0000)
00090640 05a0:
	_particle_new (0000)
00090BE0 0120:
	_particles_update (0000)
0025A8F0 0008:
	??_C@_07GFBFDLBM@gravity?$AA@ (0000)
0025A8F8 0008:
	_rdata_0025a8f8 (0000)
0025A900 0011:
	??_C@_0BB@HHDONMHD@particles_update?$AA@ (0000)
0025A914 0023:
	??_C@_0CD@JPDMOBBK@couldn?8t?5allocate?5particle?5globa@ (0000)
0025A938 0023:
	??_C@_0CD@IKBENHKF@c?3?2halo?2SOURCE?2effects?2particles@ (0000)
0025A95C 0009:
	??_C@_08HIJBMAOA@?$CGdiffuse?$AA@ (0000)
0025A968 0007:
	??_C@_06IOIMBPOK@?$CGlight?$AA@ (0000)
0025A970 0031:
	??_C@_0DB@EEHLCDHM@?$CFs?3?5assert_valid_real_argb_color@ (0000)
0025A9A4 000d:
	??_C@_0N@MNPLGION@?$CGdata?9?$DOcolor?$AA@ (0000)
0025A9B4 002a:
	??_C@_0CK@EAOMHCPC@?$CFs?3?5assert_valid_real_point3d?$CI?$CFf@ (0000)
0025A9E0 0010:
	??_C@_0BA@IFGIHPJG@?$CGdata?9?$DOposition?$AA@ (0000)
0025A9F0 002b:
	??_C@_0CL@HMPNAHGK@?$CFs?3?5assert_valid_real_vector2d?$CI?$CF@ (0000)
0025AA1C 0010:
	??_C@_0BA@OHENJGHL@?$CGdata?9?$DOvelocity?$AA@ (0000)
002DD7A0 0600:
	_data_002dd7a0 (0000)
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
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "game_state.h"
#include "sound_environment_definitions.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "effect_definitions.h"
#include "structures.h"
#include "physics_constants.h"
#include "effects.h"
#include "object_lights.h"
#include "physics_definitions.h"
#include "physics.h"
#include "first_person_weapons.h"
#include "point_physics.h"
#include "particles.h"
#include "material_effects.h"
#include "particle_definitions.h"
#include "material_effect_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
