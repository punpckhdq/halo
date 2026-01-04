/*
TERMINAL.H

header included in hcex build.
*/

#ifndef __TERMINAL_H
#define __TERMINAL_H
#pragma once

/* ---------- headers */

#include "dialogs/edit_text.h"
#include "input/input.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	TERMINAL_PRINTF_MAXIMUM_LINE_LENGTH= 255,
	TERMINAL_GETS_MAXIMUM_KEYS= 32,
	TERMINAL_GETS_MAXIMUM_LINE_LENGTH= 255,
	TERMINAL_GETS_MAXIMUM_PROMPT_LENGTH= 31,
	TERMINAL_TAB_STOP_COUNT= 3,
};

/* ---------- macros */

/* ---------- structures */

struct terminal_gets_state
{
	short key_count;
	struct key_stroke keys[TERMINAL_GETS_MAXIMUM_KEYS];
	real_argb_color color;
	char prompt[TERMINAL_GETS_MAXIMUM_PROMPT_LENGTH+1];
	char result[TERMINAL_PRINTF_MAXIMUM_LINE_LENGTH+1];
	struct edit_text edit;
};

/* ---------- prototypes/TERMINAL.C */

void terminal_initialize(void);
void terminal_dispose(void);
void terminal_clear(void);
boolean terminal_gets_active(void);
boolean terminal_gets_begin(struct terminal_gets_state *state);
void terminal_gets_end(struct terminal_gets_state *state);
void terminal_draw(void);
boolean terminal_update(void);
void terminal_printf(real_argb_color const *color, char const *format, ...);

/* ---------- globals */

extern boolean terminal_render_enable;

/* ---------- public code */

#endif // __TERMINAL_H
