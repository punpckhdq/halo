/*
CSERIES_WINDOWS.H
*/

#ifndef __CSERIES_WINDOWS_H
#define __CSERIES_WINDOWS_H
#pragma once

/* ---------- includes */

#define DEBUG_KEYBOARD
#include <xtl.h>
#include <xbdm.h>

/* ---------- prototypes/CSERIES_WINDOWS.C */

#ifdef DEBUG
void display_debug_string(char const *string);
#endif
void system_exit(long code);
void system_unique_identifier_get(struct system_unique_identifier *identifier);
boolean system_unique_identifiers_equal(struct system_unique_identifier const *identifier1, struct system_unique_identifier const *identifier2);
unsigned long system_milliseconds(void);
unsigned long system_seconds(void);
void system_get_user_name(char *name, short maximum_length);
void *system_calloc(unsigned int num, unsigned int size);
void *system_malloc(unsigned int size);
void system_free(void *pointer);
void *system_realloc(void *pointer, long size);
unsigned int system_get_used_memory_size(void *pointer);
void system_memory_information_get(struct system_memory_information *information);
void system_show_wait_cursor(void);
void system_alert(char const *string);
void system_kill_screen_saver(void);
long generic_exception_filter(unsigned long exception_code, PEXCEPTION_POINTERS exception_information);

#endif // __CSERIES_WINDOWS_H
