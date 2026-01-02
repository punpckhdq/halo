/*
DEBUG_KEYS.H

header included in hcex build.
*/

#ifndef __DEBUG_KEYS_H
#define __DEBUG_KEYS_H
#pragma once

/* ---------- constants */

enum
{
	_debug_key_no_modifier= 0,
	_debug_key_shift,
	_debug_key_ctrl,
	_debug_key_shift_ctrl,
	NUMBER_OF_DEBUG_KEY_MODIFIERS,
};

/* ---------- macros */

/* ---------- structures */

struct debug_key
{
	const char *name;
	short key_code;
	short modifier;
	void (*function)(boolean down);
	boolean toggle_variable;
	boolean *variable;
};

/* ---------- prototypes/DEBUG_KEYS.C */

void debug_keys_initialize(void);
void debug_keys_dispose(void);
void debug_keys_update(void);

/* ---------- globals */

/* ---------- public code */

#endif // __DEBUG_KEYS_H
