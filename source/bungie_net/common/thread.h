/*
THREAD.H

header included in hcex build.
*/

#ifndef __THREAD_H
#define __THREAD_H
#pragma once

/* ---------- headers */

#ifdef xbox
#include <xtl.h>
#endif

/* ---------- constants */

enum
{
	MAXIMUM_THREADS= 32,
	MAXIMUM_MUTEXES= 32,
	MUTEX_NAME_LENGTH= 32
};

// bits in create_thread()'s flags parameter
enum
{
	_thread_flag_below_normal_priority_bit= 1,
	_thread_flag_above_normal_priority_bit
};

/* ---------- macros */

/* ---------- structures */

struct thread_reference
{
	HANDLE handle;
	boolean in_use;
};

struct mutex_reference
{
	HANDLE handle;
	char name[MUTEX_NAME_LENGTH];
	boolean in_use;
};


/* ---------- prototypes/THREAD_WIN32.C */

boolean create_thread(byte flags, unsigned long (__stdcall *function)(void *parameter), void *parameter, struct thread_reference **thread_reference);
boolean thread_has_exited(struct thread_reference *thread_reference);
void dispose_thread(struct thread_reference *thread_reference);
boolean create_mutex(struct mutex_reference **mutex_reference);
boolean take_mutex(struct mutex_reference *mutex_reference, long timeout);
void release_mutex(struct mutex_reference *mutex_reference);
void dispose_mutex(struct mutex_reference *mutex_reference);

/* ---------- globals */

/* ---------- public code */

#endif // __THREAD_H
