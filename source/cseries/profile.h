/*
PROFILE.H
*/

#ifndef __PROFILE_H
#define __PROFILE_H
#pragma once

/* ---------- constants */

enum
{
	PROFILE_SECTION_HISTORY_FRAMES = 120, /* fake name */
};

enum
{
	_profile_sort_name = 0,		/* fake name */
	_profile_sort_average_time,	/* fake name */
	_profile_sort_total_time,	/* fake name */
	NUMBER_OF_PROFILE_SORT_MODES
};

enum
{
	_profile_dump_format_text = 0,	/* fake name */
	_profile_dump_format_table,		/* fake name */
	NUMBER_OF_PROFILE_DUMP_FORMAT_MODES
};

/* ---------- macros */

#define profile_enter(section)							\
if (profile_global_enable && section.active)	\
{														\
	profile_enter_private(&section);					\
}														\

#define profile_exit(section)							\
if (profile_global_enable && section.active)	\
{														\
	profile_exit_private(&section);						\
}														\

/* ---------- structures */

struct profile_section
{
	char const *name;
	long section_index;
	boolean active;
	short stack_depth;
	__int64 start_time;
	long duration_total_calls;
	__int64 duration_total_time;
	long duration_calls[PROFILE_SECTION_HISTORY_FRAMES];
	__int64 duration_time[PROFILE_SECTION_HISTORY_FRAMES];
	long interval_count;
	long interval_calls;
	__int64 interval_time;
	long total_calls;
	__int64 total_time;
	long peak_interval_calls;
	__int64 peak_interval_time;
};

struct profile_frame_iterator /* fake name */
{
	short current_buffer_index;
	short next_buffer_index; /* fake name */
};

/* ---------- prototypes/PROFILE.C */

void profile_initialize(void);
void profile_rasterizer_stats(real value, unsigned long count);
void profile_tick_start(void);
void profile_tick_end(void);
void profile_render_start(void);
void profile_render_end(void);
void profile_render_window_start(boolean player);
void profile_render_window_end(void);
void profile_texture_start(void);
void profile_texture_end(void);
void profile_frame_start(void);
void profile_frame_end(void);
void profile_idle_start(void);
void profile_idle_end(void);
void profile_seconds_elapsed(real seconds_elapsed);
void profile_lapsed_frames(short lapsed_frames, boolean at_minimum, char *string);
void profile_lapsed_msec(long msec);
void find_profile_section(struct profile_section *section);
void profile_enter_private(struct profile_section *section);
void profile_exit_private(struct profile_section *section);
long compare_profile_sections(void const *a, void const *b);
void profile_dump(char const *name, short sort_mode, short format_mode, short maximum_section_count, char *buffer);
void profile_dump_to_file(char *name);
void profile_sections_activate(char const *name);
void profile_sections_deactivate(char const *name);
short profile_find_frame_value(char const *name, short *section_index_reference);
short profile_find_game_value(char const *name, short *section_index_reference);
real profile_frame_get_value(struct profile_frame_iterator const *iterator, short value, short section_index);
void profile_frame_iterator_new(struct profile_frame_iterator *iterator);
boolean profile_frame_iterator_next(struct profile_frame_iterator *iterator, __int64 *vbl_count);
void profile_frame_get_messages(struct profile_frame_iterator const *iterator, short *message_count, short maximum_message_count, char **message_strings, point2d *message_locations, real_argb_color const **message_colors);
long profile_frame_get_stalls(struct profile_frame_iterator const *iterator, short *stall_count, real *stall_msec);
void profile_rasterizer_stalls(long stalls, short stall_count, __int64 stall_clocks, __int64 stall_time);

/* ---------- globals */

extern boolean profile_timebase_ticks;
extern boolean profile_global_enable;
extern boolean profile_dump_frames;
extern boolean profile_dump_lost_frames;
extern boolean profile_display;
extern boolean profile_graph;

#endif // __PROFILE_H
