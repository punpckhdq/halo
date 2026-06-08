/*
SHADERS.C

symbols in this file:
0017FF60 0120:
	_shader_get_vertex_shader_permutation (0000)
00180080 0050:
	_shader_is_mirror (0000)
001800D0 0010:
	_shader_is_destructable (0000)
001800E0 0010:
	_shader_is_double_sided (0000)
001800F0 0090:
	_shader_is_decal (0000)
00180180 0050:
	_shader_is_water_decal (0000)
001801D0 0050:
	_shader_ignores_effect (0000)
00180220 0020:
	_shader_type_is_transparent (0000)
00180240 0020:
	_shader_type_is_lightmapped (0000)
00180260 0020:
	_shader_type_is_vertex_lit (0000)
00180280 0020:
	_shader_type_is_valid_for_environment (0000)
001802A0 0020:
	_shader_type_is_valid_for_model (0000)
001802C0 0020:
	_shader_type_is_valid_for_modifier (0000)
001802E0 0150:
	_shader_environment_texture_animation_evaluate (0000)
00180430 0020:
	_numeric_countdown_timer_set (0000)
00180450 0190:
	_numeric_countdown_timer_get (0000)
001805E0 0010:
	_numeric_countdown_timer_stop (0000)
001805F0 0010:
	_numeric_countdown_timer_restart (0000)
00180600 0060:
	_numeric_countdown_timer_update (0000)
00180660 02e0:
	_shader_texture_animation_evaluate (0000)
002A0878 0021:
	??_C@_0CB@MIIPNOLL@c?3?2halo?2SOURCE?2shaders?2shaders?4c@ (0000)
002A089C 0022:
	??_C@_0CC@DMMMPAKI@diffuse?9?$DOv_animation_period?$CB?$DN0?40@ (0000)
002A08C0 0022:
	??_C@_0CC@JCBEGPL@diffuse?9?$DOu_animation_period?$CB?$DN0?40@ (0000)
002A08E4 0009:
	??_C@_08NPKNFIIJ@v_offset?$AA@ (0000)
002A08F0 0009:
	??_C@_08OGCAGEEM@u_offset?$AA@ (0000)
002A08FC 0016:
	??_C@_0BG@PELELMOE@v_transform_reference?$AA@ (0000)
002A0914 0016:
	??_C@_0BG@MNMMBBKE@u_transform_reference?$AA@ (0000)
002A0930 0063:
	??_C@_0GD@NAFENCB@texture_animation?9?$DOr_source?$DO?$DN0?5?$CG@ (0000)
002A0998 0063:
	??_C@_0GD@FJMMEFHI@texture_animation?9?$DOv_source?$DO?$DN0?5?$CG@ (0000)
002A0A00 0063:
	??_C@_0GD@PNPOEBPC@texture_animation?9?$DOu_source?$DO?$DN0?5?$CG@ (0000)
002A0A64 0012:
	??_C@_0BC@KMIJDGHK@texture_animation?$AA@ (0000)
004C0898 000c:
	_numeric_countdown_timer_milliseconds (0000)
	_numeric_countdown_timer_on (0004)
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"
#include "game.h"
#include "shaders.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static long numeric_countdown_timer_milliseconds;
static boolean numeric_countdown_timer_on;

/* ---------- public code */

boolean shader_is_destructable(struct shader *shader)
{
	return FALSE;
}

boolean shader_is_double_sided(struct shader *shader)
{
	return FALSE;
}

void numeric_countdown_timer_set(long milliseconds, boolean auto_start)
{
	numeric_countdown_timer_milliseconds = milliseconds;
	numeric_countdown_timer_on = auto_start;
}

short numeric_countdown_timer_get(short digit_index)
{
	long digit = 0;

	switch (digit_index)
	{
	case NONE:
		digit = numeric_countdown_timer_milliseconds;
		break;
	case 0:
		digit = numeric_countdown_timer_milliseconds % 10;
		break;
	case 1:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND/100) % 10;
		break;
	case 2:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND/10) % 10;
		break;
	case 3:
		digit = numeric_countdown_timer_milliseconds / MILLISECONDS_PER_SECOND % 10;
		break;
	case 4:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND*10) % 6;
		break;
	case 5:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND*SECONDS_PER_MINUTE) % 10;
		break;
	case 6:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND*SECONDS_PER_MINUTE*10) % 6;
		break;
	case 7:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND*SECONDS_PER_MINUTE*MINUTES_PER_HOUR) % 10;
		break;
	case 8:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND*SECONDS_PER_MINUTE*MINUTES_PER_HOUR*10) % 10;
		break;
	}

	return digit;
}

void numeric_countdown_timer_stop(void)
{
	numeric_countdown_timer_on = FALSE;
}

void numeric_countdown_timer_restart(void)
{
	numeric_countdown_timer_on = TRUE;
}

void numeric_countdown_timer_update(void)
{
	static long previous_game_time;

	if (numeric_countdown_timer_on)
	{
		long game_time = MILLISECONDS_PER_SECOND * game_time_get() / TICKS_PER_SECOND;

		if (game_time >= previous_game_time)
		{
			numeric_countdown_timer_milliseconds = numeric_countdown_timer_milliseconds - game_time + previous_game_time;

			if (numeric_countdown_timer_milliseconds < 0)
			{
				numeric_countdown_timer_milliseconds = 0;
			}
		}

		previous_game_time = game_time;
	}
}

/* ---------- private code */
