/*
CINEMATICS.H

header included in hcex build.
*/

#ifndef __CINEMATICS_H
#define __CINEMATICS_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_ACTIVE_CINEMATIC_TITLES = 4 /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct cinematic_title_datum
{
	short title_index;
	short title_timer;
};

struct cinematic_globals_definition
{
	real letter_box_amount;
	long letter_box_last_game_time;
	boolean letter_box;
	boolean cinematic_in_progress;
	boolean cinematic_skip_in_progress;
	boolean cinematic_suppress_bsp_object_creation;
	struct cinematic_title_datum active_titles[MAXIMUM_ACTIVE_CINEMATIC_TITLES];
};

/* ---------- prototypes/CINEMATICS.C */

void cinematic_initialize(void);
void cinematic_dispose(void);
void cinematic_initialize_for_new_map(void);
void cinematic_dispose_from_old_map(void);
void cinematic_start(void);
boolean cinematic_can_be_skipped(void);
void cinematic_skip_start(void);
void cinematic_skip_stop(void);
void cinematic_show_letterbox(boolean show);
void draw_quad(rectangle2d *rect, pixel32 color);
void cinematic_set_title_delayed(short index, real delay);
void cinematic_force_title(short index);
void cinematic_suppress_bsp_object_creation(boolean suppress);
void cinematic_stop(void);
boolean cinematic_in_progress(void);
void cinematic_set_title(short index);
void cinematic_render(void);

/* ---------- globals */

extern struct cinematic_globals_definition *cinematic_globals;

/* ---------- public code */

#endif // __CINEMATICS_H
