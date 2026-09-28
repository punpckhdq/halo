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

void errors_initialize(void);
void errors_dispose(void);
void error(long priority, const char *format, ...);

void write_to_error_file(char *string, boolean date);

/* ---------- globals */

extern struct error_global_data error_globals;

/* ---------- public code */



#endif // __ERRORS_H
