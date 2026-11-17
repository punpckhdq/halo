/*
ERRORS.H
*/

#ifndef __ERRORS_H
#define __ERRORS_H
#pragma once

/* ---------- constants */

enum
{
	_error_immediate = 0,
	_error_delayed,
	_error_silent,
	_error_log,
	NUMBER_OF_ERROR_MESSAGE_PRIORITIES,
};

enum
{
	ERROR_MESSAGE_BUFFER_MAXIMUM_SIZE = 2048,
};

/* ---------- structures */

/* ---------- prototypes/ERRORS.C */

void errors_dispose(void);
void errors_output_to_debug_file(boolean output_to_debug_file);
void errors_overflow_suppression_enable(boolean overflow_suppression);
char const *error_get(void);
void errors_initialize(void);
void error(short priority, char const *format, ...);
boolean errors_handle(void);
void errors_clear(void);

void write_to_error_file(char *string, boolean date);

/* ---------- globals */

extern struct error_global_data error_globals;
extern boolean find_all_fucked_up_shit;
extern long unexpected_error_count;

/* ---------- public code */



#endif // __ERRORS_H
