/*
THREAD.H

header included in hcex build.
*/

#ifndef __THREAD_H
#define __THREAD_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	MAXIMUM_THREADS = 32,
	MAXIMUM_MUTEXES = 32,
};

enum
{
	MUTEX_NAME_LENGTH = 32,
	THREAD_STACK_SIZE = 0x4000,
};

enum
{
	_thread_attribute_flag_default = 0,
	_thread_attribute_flag_priority_low,
	_thread_attribute_flag_priority_high,
	NUMBER_OF_THREAD_ATTRIBUTE_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct thread
{
	HANDLE object;
	boolean in_use;
};

struct mutex
{
	HANDLE object;
	char name[MUTEX_NAME_LENGTH];
	boolean in_use;
};

/* ---------- prototypes/THREAD_WIN32.C */

boolean create_thread(word flags, LPTHREAD_START_ROUTINE function, void *function_input, struct thread **thread_reference);
boolean thread_has_exited(struct thread *thread_reference);
void dispose_thread(struct thread *thread_reference);
boolean create_mutex(struct mutex **mutex_reference);
boolean take_mutex(struct mutex *mutex_reference, unsigned long milliseconds_to_wait);
void release_mutex(struct mutex *mutex_reference);
void dispose_mutex(struct mutex *mutex_reference);

/* ---------- globals */

/* ---------- public code */

#endif // __THREAD_H
