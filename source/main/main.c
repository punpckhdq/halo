/*
MAIN.C

symbols in this file:
000EF6F0 0010:
	_main_get_seconds_elapsed (0000)
000EF700 0080:
	_gamepad_button_is_down (0000)
000EF780 0010:
	_game_connection_set (0000)
000EF790 0010:
	_game_connection (0000)
000EF7A0 0010:
	_main_disallow_persistent_storage (0000)
000EF7B0 0060:
	_main_set_map_name (0000)
000EF810 0010:
	_main_defer_map_map_change (0000)
000EF820 0030:
	_main_set_multiplayer_map_name (0000)
000EF850 0010:
	_main_get_map_name (0000)
000EF860 0010:
	_main_get_multiplayer_map_name (0000)
000EF870 0020:
	_main_set_difficulty (0000)
000EF890 0010:
	_main_get_difficulty (0000)
000EF8A0 0040:
	_code_000ef8a0 (0000)
000EF8E0 01d0:
	_code_000ef8e0 (0000)
000EFAB0 0020:
	_main_reset_map (0000)
000EFAD0 0020:
	_main_revert_map (0000)
000EFAF0 0020:
	_main_skip_cinematic (0000)
000EFB10 0010:
	_main_save_map_nonsafe (0000)
000EFB20 0010:
	_main_saving_map (0000)
000EFB30 0010:
	_main_save_cancel (0000)
000EFB40 0040:
	_main_save_map_safe (0000)
000EFB80 0010:
	_main_won_map (0000)
000EFB90 0010:
	_main_lost_map (0000)
000EFBA0 0020:
	_main_respawn (0000)
000EFBC0 0020:
	_main_save_core (0000)
000EFBE0 0050:
	_main_save_core_name (0000)
000EFC30 0020:
	_main_load_core (0000)
000EFC50 0020:
	_main_load_core_at_startup (0000)
000EFC70 0050:
	_main_load_core_name (0000)
000EFCC0 0050:
	_main_load_core_name_at_startup (0000)
000EFD10 0060:
	_main_switch_structure_bsp (0000)
000EFD70 0030:
	_main_skip (0000)
000EFDA0 0040:
	_main_queue_map_name (0000)
000EFDE0 0050:
	_code_000efde0 (0000)
000EFE30 0020:
	_main_goto_main_menu (0000)
000EFE50 0050:
	_main_menu_precache_resources (0000)
000EFEA0 0020:
	_main_menu_unload (0000)
000EFEC0 0010:
	_main_menu_ensure_player_queues_exist (0000)
000EFED0 0010:
	_main_menu_fade_active (0000)
000EFEE0 0010:
	_main_menu_switch_to_single_player (0000)
000EFEF0 0010:
	_main_set_game_connection_to_film_playback (0000)
000EFF00 0170:
	_main_get_solo_level_from_name (0000)
000F0070 0010:
	_main_get_current_solo_level (0000)
000F0080 0020:
	_main_get_solo_level_name (0000)
000F00A0 0010:
	_main_run_demos (0000)
000F00B0 0070:
	_code_000f00b0 (0000)
000F0120 01f0:
	_compute_window_bounds (0000)
000F0310 0040:
	_main_get_window_count (0000)
000F0350 00d0:
	_code_000f0350 (0000)
000F0420 0180:
	_code_000f0420 (0000)
000F05A0 0020:
	_code_000f05a0 (0000)
000F05C0 0030:
	_code_000f05c0 (0000)
000F05F0 00b0:
	_code_000f05f0 (0000)
000F06A0 0020:
	_code_000f06a0 (0000)
000F06C0 00e0:
	_code_000f06c0 (0000)
000F07A0 0030:
	_code_000f07a0 (0000)
000F07D0 0040:
	_code_000f07d0 (0000)
000F0810 0040:
	_code_000f0810 (0000)
000F0850 0060:
	_code_000f0850 (0000)
000F08B0 0020:
	_code_000f08b0 (0000)
000F08D0 0020:
	_code_000f08d0 (0000)
000F08F0 0040:
	_code_000f08f0 (0000)
000F0930 0010:
	_code_000f0930 (0000)
000F0940 0220:
	_code_000f0940 (0000)
000F0B60 0050:
	_code_000f0b60 (0000)
000F0BB0 0030:
	_code_000f0bb0 (0000)
000F0BE0 0010:
	_code_000f0be0 (0000)
000F0BF0 05a0:
	_code_000f0bf0 (0000)
000F1190 0210:
	_main_rasterizer_throttle (0000)
000F13A0 0020:
	_code_000f13a0 (0000)
000F13C0 0020:
	_main_taking_screenshot (0000)
000F13E0 00b0:
	_main_movie_start (0000)
000F1490 0020:
	_main_movie_stop (0000)
000F14B0 0010:
	_main_stop_time (0000)
000F14C0 0010:
	_main_start_time (0000)
000F14D0 0010:
	_main_crash (0000)
000F14E0 0010:
	_main_print_version (0000)
000F14F0 00c0:
	_main_vertical_blank_interrupt_handler (0000)
000F15B0 0070:
	_main_save_current_solo_map (0000)
000F1620 00c0:
	_main_load_last_solo_map (0000)
000F16E0 0040:
	_main_save_map_no_timeout (0000)
000F1720 00e0:
	_main_load_ui_scenario (0000)
000F1800 0090:
	_main_menu_load (0000)
000F1890 0020:
	_main_roll_credits (0000)
000F18B0 0130:
	_main_pregame_render (0000)
000F19E0 01b0:
	_set_window_camera_values (0000)
000F1B90 0090:
	_main_present_frame (0000)
000F1C20 00c0:
	_code_000f1c20 (0000)
000F1CE0 0050:
	_code_000f1ce0 (0000)
000F1D30 01f0:
	_code_000f1d30 (0000)
000F1F20 02a0:
	_main_framerate_render (0000)
000F21C0 02a0:
	_halt_and_catch_fire (0000)
000F2460 0040:
	_main_loop_of_death (0000)
000F24A0 01c0:
	_code_000f24a0 (0000)
000F2660 0670:
	_main_loop (0000)
002795A8 003a:
	??_C@_0DK@GMPCKHNN@button_index?$DO?$DN0?5?$CG?$CG?5button_index?$DM@ (0000)
002795E4 001b:
	??_C@_0BL@EKABHJPH@c?3?2halo?2SOURCE?2main?2main?4c?$AA@ (0000)
00279600 0037:
	??_C@_0DH@DGKBOONK@?$CIgamepad_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIgamepad_i@ (0000)
00279638 0013:
	??_C@_0BD@OOOKAGFE@j?$DMMAXIMUM_GAMEPADS?$AA@ (0000)
00279650 0049:
	??_C@_0EJ@DIAKEACK@?$CIdesired_controllers?$FLi?$FN?$DO?$DN0?$CJ?5?$CG?$CG?5?$CI@ (0000)
0027969C 002c:
	??_C@_0CM@EOEJOPGF@game_connection?$CI?$CJ?5?$DN?$DN?5_game_conne@ (0000)
002796C8 0009:
	??_C@_08FLLKDKH@core?4bin?$AA@ (0000)
002796D4 003b:
	??_C@_0DL@LKJEADLD@warning?0?5core?5file?5name?5will?5be?5@ (0000)
00279710 002c:
	??_C@_0CM@BPJFOCNH@tried?5to?5switch?5to?5invalid?5struc@ (0000)
0027973C 002c:
	??_C@_0CM@IOHNOMHF@tried?5to?5switch?5to?5current?5struc@ (0000)
00279768 0030:
	??_C@_0DA@FGIPPFOE@cannot?5skip?5more?5than?515?5frames?5@ (0000)
00279798 0029:
	??_C@_0CJ@GNBMIDNC@scenario?9?$DOtype?$DN?$DN_scenario_type_m@ (0000)
002797C4 000e:
	??_C@_0O@OJNPMJKK@num_players?$DO0?$AA@ (0000)
002797D4 0039:
	??_C@_0DJ@LGIIGICN@horizontal_index?$DO?$DN0?5?$CG?$CG?5horizonta@ (0000)
00279810 0033:
	??_C@_0DD@NNOBFMAG@vertical_index?$DO?$DN0?5?$CG?$CG?5vertical_in@ (0000)
00279844 0019:
	??_C@_0BJ@LDIFOEOI@player_index?$DMnum_players?$AA@ (0000)
00279860 0017:
	??_C@_0BH@PGKHPIFN@main_new_map?$CI?$CJ?5failed?4?$AA@ (0000)
00279878 0014:
	??_C@_0BE@HPJEDLPC@game_load?$CI?$CJ?5failed?4?$AA@ (0000)
00279890 0041:
	??_C@_0EB@NLDLOJCN@manual?5skipping?5doesn?8t?5work?5out@ (0000)
002798D4 000c:
	??_C@_0M@NBLPJHAH@unsafe?5save?$AA@ (0000)
002798E0 0017:
	??_C@_0BH@JAFGAFKC@gave?5up?5trying?5to?5save?$AA@ (0000)
002798F8 001f:
	??_C@_0BP@JOHDPJOL@?$DLcore_load_name_at_startup?5?$CFs?6?$AA@ (0000)
00279918 0004:
	??_C@_03CCGKKFHG@a?$CLt?$AA@ (0000)
0027991C 000d:
	??_C@_0N@HMFDKLFI@map_name?5?$CFs?6?$AA@ (0000)
0027992C 0003:
	??_C@_02BKFDOEMK@wt?$AA@ (0000)
00279930 000f:
	??_C@_0P@JFNDGJMB@d?3?2?$CFs_init?4txt?$AA@ (0000)
00279940 001e:
	??_C@_0BO@EFIKKGLE@?$CFs_slow_?$CFd_?$CFd_?$CFd_?$CFd_?$CFd_?$CFd?4bin?$AA@ (0000)
00279960 0008:
	__real@3fa26e978d4fdf3c (0000)
00279968 0013:
	??_C@_0BD@BANPCAEP@?5des?5?$CFd?5targ?$CF6I64d?$AA@ (0000)
0027997C 000d:
	??_C@_0N@ONKBMNKN@?5MAINTAIN?5?$CFd?$AA@ (0000)
0027998C 000d:
	??_C@_0N@MLBLHBF@?5RESTORE?5?5?$CFd?$AA@ (0000)
0027999C 000d:
	??_C@_0N@KHJNGILI@?5FAILDOWN?5?$CFd?$AA@ (0000)
002799AC 000d:
	??_C@_0N@KAJPDIPB@?$CI?$CFs?$CF2d?1?$CF2d?$CJ?5?$AA@ (0000)
002799BC 0003:
	??_C@_02LEBOADDI@dn?$AA@ (0000)
002799C0 000c:
	??_C@_0M@FONKJGHM@?$CIok?5?5?5?$CF2d?$CJ?5?$AA@ (0000)
002799CC 0006:
	??_C@_05MPKPLMMK@ignor?$AA@ (0000)
002799D4 0009:
	??_C@_08NGBFCLJE@?$CI?$CFs?$CF2d?$CJ?5?$AA@ (0000)
002799E0 0006:
	??_C@_05KGKFECIJ@fail?5?$AA@ (0000)
002799E8 003a:
	??_C@_0DK@KGLGNIMB@last?$CF6I64d?5init?$CF6I64d?5achv?$CF6I64d@ (0000)
00279A24 0019:
	??_C@_0BJ@GBFFHJIM@?$CF6I64d?$CItarg?$CF6I64d?5?$CFs?$CF2d?$CJ?$AA@ (0000)
00279A40 0009:
	??_C@_08EEAEDAMO@LAPSED?5?5?$AA@ (0000)
00279A4C 0009:
	??_C@_08KMBEHCPI@SYNCED?5?5?$AA@ (0000)
00279A58 0009:
	??_C@_08PLAGFMEO@THROTTLE?$AA@ (0000)
00279A68 004a:
	??_C@_0EK@OBDHCGCK@stuck?5waiting?5for?5VBLANK?5callbac@ (0000)
00279AB4 0006:
	??_C@_05PAONDCEJ@movie?$AA@ (0000)
00279ABC 0019:
	??_C@_0BJ@MFOKCFOP@main_globals?4movie?$DN?$DNNULL?$AA@ (0000)
00279AD8 002a:
	??_C@_0CK@IJIKLLCA@chucky?5was?5here?$CB?5?5NULL?5belongs?5t@ (0000)
00279B04 0031:
	??_C@_0DB@HPIOCFDH@halobeta?5xbox?501?401?414?42342?5Jan?5@ (0000)
00279B38 0038:
	??_C@_0DI@IKBOMHNJ@Couldn?8t?5create?5a?5file?5to?5write?5@ (0000)
00279B70 0011:
	??_C@_0BB@HAHOOKLD@z?3?2last_solo?4txt?$AA@ (0000)
00279B84 0028:
	??_C@_0CI@KPDFMEMF@?$CBmain_globals?4main_menu_scenario@ (0000)
00279BAC 000d:
	??_C@_0N@CBIELCEF@levels?2ui?2ui?$AA@ (0000)
00279BBC 0023:
	??_C@_0CD@IMDNHNHI@congratulations?0?5you?5won?5the?5gam@ (0000)
00279BE0 0014:
	??_C@_0BE@BDDOCJAN@movie?2frame?$CF06d?4tga?$AA@ (0000)
00279BF4 0019:
	??_C@_0BJ@CKCGDFO@error?5opening?5saved?5film?$AA@ (0000)
00279C10 0015:
	??_C@_0BF@LPIHHLNO@?$CFdscreenshot?$CFd?$CFd?4tif?$AA@ (0000)
00279C28 003b:
	??_C@_0DL@LMMABMNB@halobeta?5xbox?501?401?414?42342?5buil@ (0000)
00279C64 001f:
	??_C@_0BP@HEAODFIJ@old?5tags?2internal?5system?5plain?$AA@ (0000)
00279C84 0012:
	??_C@_0BC@FLAPBDJB@end?5of?5saved?5film?$AA@ (0000)
00279C98 0018:
	??_C@_0BI@HBMJOOEK@the?5game?5host?5went?5down?$AA@ (0000)
00307818 0034:
	_global_difficulty_level (0000)
	_player_spawn_count (0004)
	_global_frame_rate_throttle (0006)
	_global_screenshot_size (0008)
00455750 0a63:
	_bss_00455750 (0000)
	_debug_force_frame_rate_update (06d0)
	_debug_no_drawing (06d1)
	_debug_game_save (06d2)
	_debug_frame_rate (06d3)
	_display_framerate (06d4)
	_display_vblank_deltas (06d5)
	_display_precache_progress (06d6)
	_global_screenshot_count (06d8)
*/

/* ---------- headers */

#include "cseries.h"
#include "main.h"
#include "real_math.h"
#include "game.h"
#include "integer_math.h"
#include "input.h"
#include "shell.h"
#include "event_manager.h"
#include "telnet_console.h"
#include "ui_widget.h"
#include "editor_stubs.h"
#include "console.h"
#include "debug_keys.h"
#include "scenario.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/*
struct _main_globals
{
	long long last_time_clocks; // 0x0
	unsigned long last_time_msec; // 0x8
	long long last_render_clocks; // 0x10
	long long last_vblank_index; // 0x18
	long long last_initial_vblank_index; // 0x20
	long long last_achievable_vblank_index; // 0x28
	long long last_present_vblank_index; // 0x30
	unsigned char did_time_overflow_occur; // 0x38
	float seconds_elapsed; // 0x3C
	short connection; // 0x40
	unsigned short screenshot_identifier; // 0x42
	bitmap_data *movie; // 0x44
	long recording_start_tick; // 0x48
	long recording_stop_tick; // 0x4C
	long recording_frame_index; // 0x50
	float recording_dt; // 0x54
	unsigned char reset_map; // 0x58
	unsigned char rename_map; // 0x59
	unsigned char revert_map; // 0x5A
	unsigned char skip_cinematic; // 0x5B
	unsigned char save_map; // 0x5C
	unsigned char save_map_safely; // 0x5D
	unsigned char save_map_timeout; // 0x5E
	unsigned char saving_map; // 0x5F
	long ticks_until_next_save_check; // 0x60
	long ticks_unable_to_save; // 0x64
	unsigned long map_change_load_timer; // 0x68
	short safe_intervals; // 0x6C
	unsigned char won_map; // 0x6E
	unsigned char lost_map; // 0x6F
	unsigned char respawn; // 0x70
	unsigned char save_core; // 0x71
	unsigned char load_core; // 0x72
	unsigned char load_core_at_startup; // 0x73
	short switch_to_structure_bsp_index; // 0x74
	unsigned char main_menu_scenario_loaded; // 0x76
	unsigned char want_to_be_at_main_menu; // 0x77
	unsigned char run_xdemos; // 0x78
	unsigned char fade_to_dashboard; // 0x79
	unsigned char exit_to_dashboard; // 0x7A
	unsigned char want_to_exit; // 0x7B
	long idle_timeout; // 0x7C
	long idle_last_interesting; // 0x80
	long idle_last_activity; // 0x84
	unsigned char playback_last_recording; // 0x88
	unsigned char halt_time_scale; // 0x89
	unsigned char restart_time; // 0x8A
	unsigned char load_last_solo_level; // 0x8B
	unsigned char cutscene_skip; // 0x8C
	short skip_ticks; // 0x8E
	short loss_timer; // 0x90
	short respawn_timer; // 0x92
	unsigned char queue_map; // 0x94
	unsigned char pad0[3]; // 0x95
	unsigned char solo_try_and_load_from_persistent_storage; // 0x98
	char soloplayer_map_name[256]; // 0x99
	char multiplayer_map_name[256]; // 0x199
	char queued_map_name[256]; // 0x299
	unsigned char want_to_connect; // 0x399
	char connect_address[32]; // 0x39A
	char connect_password[9]; // 0x3BA
	short vblank_interval_current; // 0x3C4
	short vblank_interval_minimum; // 0x3C6
	unsigned char vblank_interval_held; // 0x3C8
	short vblank_failure_count[6]; // 0x3CA
	long long vblank_last_failure_time[6]; // 0x3D8
	unsigned long *vblank_flip_counter; // 0x408
	short vblank_flip_delta_next_index; // 0x40C
	short vblank_flip_deltas[15]; // 0x40E
};
*/

struct _main_globals
{
	byte __unknown00[216];
	real seconds_elapsed;
	short connection;
	byte __unknownDE[22];
	boolean reset_map;
	boolean rename_map;
	boolean revert_map;
	boolean skip_cinematic;
	boolean save_map;
	boolean save_map_safely;
	boolean save_map_timeout;
	boolean saving_map;
	long ticks_until_next_save_check;
	long ticks_unable_to_save;
	unsigned long map_change_load_timer;
	short safe_intervals;
	boolean won_map;
	boolean lost_map;
	boolean respawn;
	boolean save_core;
	boolean load_core;
	boolean load_core_at_startup;
	short switch_to_structure_bsp_index;
	boolean __unknown112;
	boolean want_to_be_at_main_menu;
	boolean run_xdemos;
	byte __unknown115;
	boolean halt_time_scale;
	byte __unknown117[0x9];
	boolean queue_map;
	byte __unknown121[0x4];
	boolean solo_try_and_load_from_persistent_storage;
	char soloplayer_map_name[256];
	char multiplayer_map_name[256];
	char queued_map_name[256];
	byte __unknown125[682];
};

/* ---------- prototypes */

extern void create_local_players(void);
extern void code_000f1c20(void);
extern void code_000f1ce0(void);

extern void main_rasterizer_throttle(void);
extern void main_pregame_render(void);
extern void main_present_frame(void);

/* ---------- globals */

static struct _main_globals main_globals;

/* ---------- public code */

static void main_reset_map_private(
	void)
{
	if (!game_time_get_paused())
	{
		scenario_switch_structure_bsp(0);
		game_dispose_from_old_map();
		input_flush();
		game_initialize_for_new_map();
		create_local_players();
		game_time_start();
		game_initial_pulse();
		ui_widgets_disable_pause_game(30);

		main_globals.reset_map = FALSE;
	}

	return;
}

void main_loop_of_death(
	void)
{
	while (TRUE)
	{
		input_frame_begin();
		input_update();
		shell_idle();
		event_manager_update();
		telnet_console_process();
		process_ui_widgets();
		main_pregame_render();
		main_rasterizer_throttle();
		main_present_frame();
		input_frame_end();
	}

	return;
}

void main_loop(
	void)
{
	if (!game_in_editor())
	{
		strncpy(main_globals.soloplayer_map_name, "levels\\b30\\b30", NUMBEROF(main_globals.soloplayer_map_name)-1);
		main_globals.soloplayer_map_name[NUMBEROF(main_globals.soloplayer_map_name)-1] = '\0';
	}

	main_globals.want_to_be_at_main_menu = (game_in_editor()==FALSE);
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.halt_time_scale = TRUE;

	console_initialize();
	debug_keys_initialize();
	game_initialize();
	console_startup();
	code_000f1c20();
	code_000f1ce0();

	while (TRUE)
	{
		if (game_in_editor())
		{
			if (main_globals.reset_map)
			{
				main_reset_map_private();
			}
		}
		else
		{
			if (main_globals.switch_to_structure_bsp_index!=NONE)
			{
				scenario_switch_structure_bsp(main_globals.switch_to_structure_bsp_index);
				main_globals.switch_to_structure_bsp_index = NONE;
				hud_load(0);
			}

			if (main_globals.lost_map)
			{

			}

			if (main_globals.won_map)
			{

			}

			if (main_globals.respawn)
			{

			}

			if (main_globals.saving_map)
			{

			}

			if (main_globals.rename_map)
			{

			}

			if (main_globals.revert_map)
			{

			}

			if (main_globals.skip_cinematic)
			{

			}

			if (main_globals.reset_map)
			{
				main_reset_map_private();
			}

			if (main_globals.save_core)
			{
				//game_state_save_core(bss_00455750.__unknown125);
				main_globals.save_core = FALSE;
			}

			if (main_globals.load_core)
			{
				//game_state_save_core(bss_00455750.__unknown125);
				main_globals.load_core = FALSE;
			}

			if (main_globals.want_to_be_at_main_menu)
			{
				main_menu_load();
			}

			if (main_globals.__unknown117[1])
			{
				main_load_last_solo_map();
			}

			if (main_globals.run_xdemos)
			{
				main_globals.run_xdemos = FALSE;
				xbox_demos_launch();
			}

			if (main_globals.__unknown117[2])
			{
				code_000f05f0();
			}

			if (main_globals.queue_map)
			{
				//bss_00455750.queue_map = FALSE;
			}
		}

		profile_frame_start();
		input_frame_begin();
		input_update();
		input_abstraction_update();
		shell_idle();
		event_manager_update();
		telnet_console_process();

		if (!shell_application_is_paused())
		{
			if (main_globals.connection==1)
			{

			}

//            int32_t eax_9 = (int32_t)data_85582c;
//            (uint8_t)ebx = 1;
//            
//            if (eax_9 == 1)
//            {
//                if (!sub_519b20(ecx_1))
//                {
//                    sub_4d3c50(6);
//                    var_1c = "the game host went down";
//                    sub_47da00(2, "the game host went down");
//                    sub_519fd0();
//                }
//            }
//            else if (eax_9 == 2)
//            {
//                char eax_11 = sub_519b20(ecx_1);
//                char eax_12;
//                
//                if (eax_11)
//                    eax_12 = sub_519a60();
//                
//                if (!eax_11 || !eax_12)
//                {
//                    sub_4d3c50(1);
//                    var_1c = "the game host went down";
//                    sub_47da00(2, "the game host went down");
//                    sub_519fd0();
//                }
//            }
//            else if (eax_9 == 3)
//            {
//                sub_47da00(2, "end of saved film");
//                int32_t eax_35 = (int32_t)data_85582c;
//                
//                if (eax_35 == 1)
//                    sub_519af0();
//                else if (eax_35 == 2)
//                {
//                    sub_519af0();
//                    sub_519a30();
//                }
//                
//                sub_4956c0();
//                sub_495370();
//                sub_4ef490();
//                sub_4ef1d0();
//                return;
//            }
//            
//            sub_4f0bf0();
//            sub_4d8bd0();
//            sub_5b6360();
//            
//            if (sub_485e60())
//            {
//            label_4f2a3e:
//                
//                if (sub_485e80())
//                    goto label_4f2a47;
//            }
//            else
//            {
//                if (!sub_4be900(0x55))
//                {
//                    if (sub_4be900(0))
//                        goto label_4f2a47;
//                    
//                    goto label_4f2a3e;
//                }
//                
//            label_4f2a47:
//                void* eax_17 = data_855830;
//                
//                if (eax_17)
//                {
//                    sub_46af60(eax_17);
//                    data_855830 = 0;
//                }
//                
//                char eax_18 = sub_4974e0();
//                
//                if (!eax_18)
//                {
//                    data_855860 = 0xffff;
//                    data_855848 = eax_18;
//                    data_855844 = 1;
//                    data_85585b = eax_18;
//                }
//            }
//            
//            if (!sub_4a4f50())
//            {
//                sub_47fb80();
//                    sub_4f18b0(st0_1);
//                top += 1;
//                sub_47fbb0();
//            }
//            else
//            {
//                sub_4d3160();
//                char eax_20;
//                int32_t ecx_2;
//                eax_20 = sub_4ef1f0();
//                
//                if (!eax_20 || data_85582c)
//                {
//                    sub_4ef4c0(ecx_2);
//                    sub_494c90();
//                    uint32_t ecx_3 = (uint32_t)data_855866;
//                    uint32_t var_8_1 = ecx_3;
//                    uint32_t var_18_4 = ecx_3;
//                        /*   unimplemented  {fild st0, dword [ebp-0x4]} */
//                        /*   unimplemented  {fmul st0, dword [&data_855828]} */
//                        float var_18_5 = (float)/*   float var_18_5 =
//                        fconvert.s(unimplemented  {fstp dword [esp], st0}) */;
//                    /*   unimplemented  {fstp dword [esp], st0} */
//                    int32_t ecx_4 = sub_4a7fe0(var_18_5);
//                    int32_t eax_21 = (int32_t)data_85582c;
//                    
//                    if (eax_21 > 0 && eax_21 <= 2)
//                    {
//                        char eax_22;
//                        eax_22 = sub_519d50();
//                        
//                        if (!eax_22)
//                        {
//                            ecx_4 = sub_4d3c50(1);
//                            sub_519fd0();
//                        }
//                    }
//                    
//                    uint32_t var_8_2 = (uint32_t)data_855866;
//                    int32_t var_18_6 = ecx_4;
//                        /*   unimplemented  {fild st0, dword [ebp-0x4]} */
//                        /*   unimplemented  {fmul st0, dword [&data_855828]} */
//                        float var_18_7 = (float)/*   float var_18_7 =
//                        fconvert.s(unimplemented  {fstp dword [esp], st0}) */;
//                    /*   unimplemented  {fstp dword [esp], st0} */
//                    sub_4a5390(var_18_7);
//                    
//                    if (data_855862)
//                        (uint8_t)ebx = 1;
//                    else if (!data_855866)
//                        (uint8_t)ebx = 0;
//                    else if (sub_4a4fa0())
//                        (uint8_t)ebx = 1;
//                    else if (sub_4a4e50() > 0)
//                        (uint8_t)ebx = 1;
//                    else
//                    {
//                            sub_4a5030();
//                        /*   unimplemented  {call sub_4a5030} */
//                          long double temp3_1 = (long double)1f;
//                        /*   unimplemented  {fcomp st0, dword [&data_642f64[4]]}
//                            f- temp3_1 */ - temp3_1;
//                        bool c0_1 = /*   bool c0_1 =
//                            unimplemented  {fcomp st0, dword [&data_642f64[4]]}
//                            f< temp3_1 */ < temp3_1;
//                        bool c2_1 = FCMP_UO(/*   bool c2_1 = is_unordered.t(
//                            unimplemented  {fcomp st0, dword [&data_642f64[4]]}, 
//                            temp3_1) */, temp3_1);
//                        bool c3_1 = /*   bool c3_1 =
//                            unimplemented  {fcomp st0, dword [&data_642f64[4]]}
//                            f== temp3_1 */ == temp3_1;
//                        /*   unimplemented  {fcomp st0, dword [&data_642f64[4]]} */
//                        int32_t eax_26;
//                        (uint16_t)eax_26 = (c0_1 ? 1 : 0) << 8 | (c2_1 ? 1 : 0) << 0xa
//                            | (c3_1 ? 1 : 0) << 0xe | (top & 7) << 0xb;
//                            bool p_1 = /*   bool p_1 = unimplemented  {test ah, 0x5} */;
//                        
//                        if (!p_1)
//                            (uint8_t)ebx = 1;
//                        else
//                            (uint8_t)ebx = 0;
//                    }
//                    
//                    char eax_27 = sub_4974e0();
//                    int32_t eax_28;
//                    
//                    if (eax_27)
//                        eax_28 = sub_4a4e10();
//                    
//                    char eax_29;
//                    
//                    eax_29 = !eax_27 || eax_28 >= 3 ? 1 : 0;
//                    
//                    (uint8_t)ebx &= eax_29;
//                    sub_53c970(1);
//                    uint32_t var_8_3 = (uint32_t)data_855866;
//                        /*   unimplemented  {fild st0, dword [ebp-0x4]} */
//                        /*   unimplemented  {fmul st0, dword [&data_855828]} */
//                        float var_18_8 = (float)/*   float var_18_8 =
//                        fconvert.s(unimplemented  {fstp dword [esp], st0}) */;
//                    /*   unimplemented  {fstp dword [esp], st0} */
//                    sub_475c60(var_18_8);
//                    uint32_t var_8_4 = (uint32_t)data_855866;
//                        /*   unimplemented  {fild st0, dword [ebp-0x4]} */
//                        /*   unimplemented  {fmul st0, dword [&data_855828]} */
//                        float var_18_9 = (float)/*   float var_18_9 =
//                        fconvert.s(unimplemented  {fstp dword [esp], st0}) */;
//                    /*   unimplemented  {fstp dword [esp], st0} */
//                    sub_47b450(var_18_9);
//                    sub_53c990();
//                    uint32_t var_8_5 = (uint32_t)data_855866;
//                        /*   unimplemented  {fild st0, dword [ebp-0x4]} */
//                        /*   unimplemented  {fmul st0, dword [&data_855828]} */
//                      float var_18_10 = (float)/*   float var_18_10 =
//                        fconvert.s(unimplemented  {fstp dword [esp], st0}) */;
//                    /*   unimplemented  {fstp dword [esp], st0} */
//                    top = top;
//                    sub_49b5c0(var_18_10);
//                }
//                
//                if (data_855848)
//                    sub_4f06c0();
//                
//                if ((uint8_t)ebx && !data_855e21)
//                {
//                    sub_47fb80();
//                        /*   unimplemented  {fld st0, dword [&data_855828]} */
//                      var_1c = (double)/*   var_1c.q =
//                        fconvert.d(unimplemented  {fstp qword [esp], st0}) */;
//                    /*   unimplemented  {fstp qword [esp], st0} */
//                    sub_4f24a0();
//                    sub_47fbb0();
//                }
//            }
//            
//            sub_4f1190();
//            
//            if ((uint8_t)ebx && !data_855e21)
//                sub_4f1b90();
		}

		input_frame_end();
		profile_frame_end();
		code_000f0940();
		
		if (main_globals.__unknown117[0])
		{
			//main_globals.__unknown117[0] = 0;
			//*(_DWORD *)&main_globals.__unknown00[176] = system_milliseconds();
			//*(_DWORD *)&main_globals.__unknown00[184] = dword_70D400;
			//*(_DWORD *)&main_globals.__unknown00[188] = dword_70D404;
			main_globals.halt_time_scale = TRUE;
		}
	}

	game_dispose_from_old_map();
	game_dispose();
	debug_keys_dispose();
	console_dispose();

	return;
}

/* ---------- private code */
