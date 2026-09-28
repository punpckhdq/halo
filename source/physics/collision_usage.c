/*
COLLISION_USAGE.C

*/

/* ---------- headers */

#include "cseries.h"
#include "collision_usage.h"
#include "game.h"
#include "editor_stubs.h"
#include "render_cameras.h"
#include "rasterizer.h"
#include "interface.h"
#include "draw_string.h"

/* ---------- structures */

struct collision_log
{
	long calls;
	__int64 elapsed_time;
};

struct collision_function
{
	struct collision_log total_all_users;
	struct collision_log usage_by_user[NUMBER_OF_COLLISION_USER_TYPES];
};

struct collision_period
{
	boolean reset_upon_next_use;
	boolean valid;
	long period_count;
	struct collision_function function[NUMBER_OF_COLLISION_FUNCTION_TYPES];
};

struct collision_overall_usage
{
	short user_type;
	unsigned short pad;
	struct collision_log total_all_periods;
	struct collision_log usage_by_period[NUMBER_OF_COLLISION_TIME_PERIODS];
};

/* ---------- prototypes */

/* ---------- globals */

const char *global_collision_function_names[] =
{
	"vector-structure",
	"vector-objects",
	"features-in-sphere",
	"model-vector",
	"object-bsp-vector",
	"structure-bsp-vector",
	"object-bsp-sphere",
	"structure-bsp-sphere",
	NULL
};

const char *global_collision_user_names[] =
{
	"????",
	"ai-look",
	"ai-los",
	"ai-comm",
	"ai-fire",
	"ai-melee",
	"aim",
	"biped",
	"melee",
	"decal",
	"areadmg",
	"item",
	"obsrv",
	"pt-phys",
	"proj",
	"light",
	"sound",
	"veh",
	"limp",
	"object",
	"ui",
	"debug",
	NULL
};

boolean global_collision_log_enable = TRUE;

boolean collision_log_render_enable = FALSE;
boolean collision_log_detailed = FALSE;
boolean collision_log_extended = FALSE;
boolean collision_log_totals_only = FALSE;
boolean collision_log_time = FALSE;

short global_current_collision_user_depth = 0;
short global_current_collision_users[MAXIMUM_COLLISION_USER_STACK_DEPTH];

short collision_usage_current_period = NONE;
struct collision_period collision_usage_buffer[NUMBER_OF_COLLISION_TIME_PERIODS];

struct collision_period collision_usage_current;

/* ---------- public code */

void collision_log_initialize(
	void)
{
	memset(&collision_usage_buffer, 0, sizeof(collision_usage_buffer));
	
	match_collision_log_begin_user("c:\\halo\\SOURCE\\physics\\collision_usage.c", 150, _collision_user_unknown);

	return;
}

void collision_log_enable(
	boolean enable)
{
	global_collision_log_enable = enable;

	return;
}

static void collision_log_store_period(
	short time_period,
	boolean unused)
{
	match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 167, collision_usage_current_period == NONE);
	match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 168, (time_period >= 0) && (time_period < NUMBER_OF_COLLISION_TIME_PERIODS));

	memset(&collision_usage_current, 0, sizeof(collision_usage_current));
	collision_usage_current_period = time_period;

	return;
}

void collision_log_begin_period(
	short time_period)
{
	collision_log_store_period(time_period, TRUE);

	return;
}

void collision_log_continue_period(
	short time_period)
{
	collision_log_store_period(time_period, FALSE);

	return;
}

void collision_log_end_period(
	void)
{
	match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 198,
		(collision_usage_current_period >= 0) && (collision_usage_current_period < NUMBER_OF_COLLISION_TIME_PERIODS));

	collision_usage_current.reset_upon_next_use = TRUE;

	collision_usage_buffer[collision_usage_current_period] = collision_usage_current;
	collision_usage_current_period = NONE;

	return;
}

static void collision_log_format_usage(
	const struct collision_log *usage,
	char *buffer)
{
	if (collision_log_time)
	{
		__int64 frequency;
		QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
		sprintf(buffer, "%d/%.2f", usage->calls, (real)usage->elapsed_time * MILLISECONDS_PER_SECOND / frequency);
	}
	else
	{
		sprintf(buffer, "%d", usage->calls);
	}

	return;
}

static int __cdecl collision_log_compare_usage(
	const void *a,
	const void *b)
{
	const struct collision_overall_usage *usage_a = a;
	const struct collision_overall_usage *usage_b = b;
	if (usage_a->total_all_periods.calls > usage_b->total_all_periods.calls)
		return -1;
	return usage_a->total_all_periods.calls < usage_b->total_all_periods.calls;
}

void collision_log_render(
	void)
{
	if (collision_log_render_enable)
	{
		char linebuf[2048];
		short function_index;
		short debug_string_position = rasterizer_globals.frame_bounds.y1 - 30;

		for (function_index = 0; function_index < NUMBER_OF_COLLISION_FUNCTION_TYPES; function_index++)
		{
			boolean recorded = FALSE;
			short time_period_index;
			if (function_index < _collision_function_vector_intersect_model || collision_log_extended)
			{
				for (time_period_index = 0; time_period_index < NUMBER_OF_COLLISION_TIME_PERIODS; time_period_index++)
				{
					if (collision_usage_buffer[time_period_index].reset_upon_next_use &&
						collision_usage_buffer[time_period_index].function[function_index].total_all_users.calls > 0)
					{
						recorded = TRUE;
					}
				}
				if (recorded)
				{
					struct collision_overall_usage total_usage;
					struct collision_overall_usage overall_usage[NUMBER_OF_COLLISION_USER_TYPES];
					short user_type_index;
					memset(&total_usage, 0, sizeof(total_usage));
					memset(overall_usage, 0, sizeof(overall_usage));
					for (user_type_index = 0; user_type_index < NUMBER_OF_COLLISION_USER_TYPES; user_type_index++)
					{
						overall_usage[user_type_index].user_type = user_type_index;
						if (user_type_index != _collision_user_debugging || collision_log_extended)
						{
							for (time_period_index = 0; time_period_index < NUMBER_OF_COLLISION_TIME_PERIODS; time_period_index++)
							{
								if (collision_usage_buffer[time_period_index].reset_upon_next_use)
								{
									overall_usage[user_type_index].usage_by_period[time_period_index] = collision_usage_buffer[time_period_index].function[function_index].usage_by_user[user_type_index];
									total_usage.usage_by_period[time_period_index].calls += collision_usage_buffer[time_period_index].function[function_index].usage_by_user[user_type_index].calls;
									total_usage.usage_by_period[time_period_index].elapsed_time += collision_usage_buffer[time_period_index].function[function_index].usage_by_user[user_type_index].elapsed_time;
								}
								overall_usage[user_type_index].total_all_periods.calls += overall_usage[user_type_index].usage_by_period[time_period_index].calls;
								overall_usage[user_type_index].total_all_periods.elapsed_time += overall_usage[user_type_index].usage_by_period[time_period_index].elapsed_time;
								total_usage.total_all_periods.calls += overall_usage[user_type_index].usage_by_period[time_period_index].calls;
								total_usage.total_all_periods.elapsed_time += overall_usage[user_type_index].usage_by_period[time_period_index].elapsed_time;
							}
						}
					}
					qsort(overall_usage, NUMBER_OF_COLLISION_USER_TYPES, sizeof(overall_usage[0]), collision_log_compare_usage);
					sprintf(linebuf, "%s:", global_collision_function_names[function_index]);
					if (collision_log_totals_only)
					{
						char tempstring[512];
						strcpy(tempstring, " ");
						if (collision_log_detailed)
						{
							for (time_period_index = 0; time_period_index < NUMBER_OF_COLLISION_TIME_PERIODS; time_period_index++)
							{
								char tempbuf[256];
								short length = strlen(tempstring);
								collision_log_format_usage(&total_usage.usage_by_period[time_period_index], tempbuf);
								_snprintf(tempstring + length, sizeof(tempstring) - length, "%c%s", time_period_index ? '/' : ' ', tempbuf);
							}
						}
						else
						{
							char tempbuf[256];
							short length = strlen(tempstring);
							collision_log_format_usage(&total_usage.total_all_periods, tempbuf);
							_snprintf(tempstring + length, sizeof(tempstring) - length, " %s", tempbuf);
						}
						strcat(linebuf, tempstring);
					}
					else
					{
						short index;

						for (index = 0; index <= _collision_user_ai_melee; index++)
						{
							short user_index = overall_usage[index].user_type;
							match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 342, (user_index >= 0) && (user_index < NUMBER_OF_COLLISION_USER_TYPES));
							if (overall_usage[index].total_all_periods.calls > 0)
							{
								char tempstring[512];
								_snprintf(tempstring, sizeof(tempstring), " %s", global_collision_user_names[user_index]);
								if (collision_log_detailed)
								{
									for (time_period_index = 0; time_period_index < NUMBER_OF_COLLISION_TIME_PERIODS; time_period_index++)
									{
										char tempbuf[256];
										short length = strlen(tempstring);
										collision_log_format_usage(&overall_usage[index].usage_by_period[time_period_index], tempbuf);
										_snprintf(tempstring + length, sizeof(tempstring) - length, "%c%s", time_period_index ? '/' : ' ', tempbuf);
									}
								}
								else
								{
									char tempbuf[256];
									short length = strlen(tempstring);
									collision_log_format_usage(&overall_usage[index].total_all_periods, tempbuf);
									_snprintf(tempstring + length, sizeof(tempstring) - length, " %s", tempbuf);
								}
								strcat(linebuf, tempstring);
							}
						}
					}
					{
						rectangle2d bounds;
						point2d cursor;
						bounds.x0 = rasterizer_globals.frame_bounds.x0;
						bounds.y0 = debug_string_position;
						bounds.y1 = bounds.x1 = SHRT_MAX;
						interface_set_bitmap_text_draw_mode(_interface_font_terminal, NONE, 0, 0, _interface_color_table_dialog, 0);
						draw_string_set_color(global_real_argb_white);
						draw_string_set_tab_stops(NULL, 0);
						rasterizer_draw_string(&bounds, NULL, &cursor, 0, linebuf);
						debug_string_position -= cursor.y - bounds.y0;
					}
				}
			}
		}
	}

	return;
}

static short collision_log_get_current_user(
	short collision_function)
{
	short user;

	match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 403, global_current_collision_user_depth > 0);

	user = global_current_collision_users[global_current_collision_user_depth - 1];
	match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 406, (user >= 0) && (user < NUMBER_OF_COLLISION_USER_TYPES));
	match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 407,
		(collision_function >= 0) && (collision_function < NUMBER_OF_COLLISION_FUNCTION_TYPES));

	if (!game_in_progress() || game_in_editor() || !global_collision_log_enable)
	{
		user = NONE;
	}
	else if (collision_usage_current_period == NONE)
	{
		user = NONE;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\physics\\collision_usage.c", 424,
			(collision_usage_current_period >= 0) && (collision_usage_current_period < NUMBER_OF_COLLISION_TIME_PERIODS));
	}

	return user;
}

void collision_log_start_time(
	__int64 *start_time)
{
	QueryPerformanceCounter((PLARGE_INTEGER)start_time);

	return;
}

void collision_log_end_time(
	short collision_function,
	__int64 start_time)
{
	__int64 end_time;
	short user;

	QueryPerformanceCounter((PLARGE_INTEGER)&end_time);

	user = collision_log_get_current_user(collision_function);

	if (user != NONE)
	{
		__int64 elapsed_time = end_time - start_time;

		collision_usage_current.function[collision_function].total_all_users.elapsed_time += elapsed_time;
		collision_usage_current.function[collision_function].usage_by_user[user].elapsed_time += elapsed_time;
	}

	return;
}

void collision_log_usage(
	short collision_function)
{
	short user = collision_log_get_current_user(collision_function);
	if (user != NONE)
	{
		collision_usage_current.function[collision_function].total_all_users.calls++;
		collision_usage_current.function[collision_function].usage_by_user[user].calls++;
	}

	return;
}

void collision_log_display(
	char *buffer)
{
	if (collision_usage_buffer[0].reset_upon_next_use)
	{
		sprintf(buffer + strlen(buffer), "sphere % 3df % 3db, str-vec % 3d/% 3d, obj-vec % 3d/% 3d|n",
			collision_usage_buffer[0].function[_collision_function_vector_bounds_object].total_all_users.calls,
			collision_usage_buffer[0].function[_collision_function_sphere_intersect_bsp_structure].total_all_users.calls +
			collision_usage_buffer[0].function[_collision_function_sphere_intersect_bsp_object].total_all_users.calls,
			collision_usage_buffer[0].function[_collision_function_vector_structure].total_all_users.calls,
			collision_usage_buffer[0].function[_collision_function_vector_intersect_bsp_structure].total_all_users.calls,
			collision_usage_buffer[0].function[_collision_function_vector_objects].total_all_users.calls,
			collision_usage_buffer[0].function[_collision_function_vector_intersect_bsp_object].total_all_users.calls);
	}

	return;
}
