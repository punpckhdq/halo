/*
THREAD_WIN32.C
*/

/* ---------- headers */

#include "cseries.h"
#include "thread.h"

#include <stdio.h>

/* ---------- constants */

enum
{
	THREAD_STACK_SIZE= 0x4000
};

/* ---------- structures */

struct thread_globals
{
	long mutex_count;
	long pad_04; // create_mutex() is the only code that reaches the head of this record; nothing reads or writes these four bytes
	struct thread_reference threads[MAXIMUM_THREADS];
	struct mutex_reference mutexes[MAXIMUM_MUTEXES];
};

/* ---------- prototypes */

static struct thread_reference *get_thread_from_pool(void);
static struct mutex_reference *get_mutex_from_pool(void);

/* ---------- globals */

static struct thread_globals thread_globals;

/* ---------- private code */

static struct thread_reference *get_thread_from_pool(
	void)
{
	struct thread_reference *thread= NULL;
	long i;

	for (i= 0; i<MAXIMUM_THREADS; i++)
	{
		if (!thread_globals.threads[i].in_use)
		{
			thread= &thread_globals.threads[i];
			thread->handle= NULL;
			thread->in_use= TRUE;
			break;
		}
	}

	return thread;
}

static struct mutex_reference *get_mutex_from_pool(
	void)
{
	struct mutex_reference *mutex= NULL;
	long i;

	for (i= 0; i<MAXIMUM_MUTEXES; i++)
	{
		if (!thread_globals.mutexes[i].in_use)
		{
			mutex= &thread_globals.mutexes[i];
			mutex->name[0]= 0;
			mutex->handle= NULL;
			mutex->in_use= TRUE;
			break;
		}
	}

	return mutex;
}

/* ---------- public code */

boolean create_thread(
	byte flags,
	thread_proc function,
	void *parameter,
	struct thread_reference **thread_reference)
{
	boolean success= FALSE;
	struct thread_reference *thread;
	unsigned long thread_id;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 107, function);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 108, thread_reference);

	thread= get_thread_from_pool();
	if (thread && (thread->handle= CreateThread(NULL, THREAD_STACK_SIZE, function, parameter, CREATE_SUSPENDED, &thread_id))!=NULL)
	{
		long priority= THREAD_PRIORITY_NORMAL;

		if (TEST_FLAG(flags, _thread_flag_below_normal_priority_bit)) priority= THREAD_PRIORITY_BELOW_NORMAL;
		else if (TEST_FLAG(flags, _thread_flag_above_normal_priority_bit)) priority= THREAD_PRIORITY_ABOVE_NORMAL;

		if (SetThreadPriority(thread->handle, priority) && ResumeThread(thread->handle)!=-1)
		{
			success= TRUE;
		}
		else
		{
			CloseHandle(thread->handle);
			thread= NULL;
		}
	}

	*thread_reference= thread;
	return success;
}

boolean thread_has_exited(
	struct thread_reference *thread_reference)
{
	boolean result= FALSE;
	unsigned long exit_code;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 152, thread_reference);

	if (GetExitCodeThread(thread_reference->handle, &exit_code) && exit_code!=STILL_ACTIVE)
	{
		result= TRUE;
	}

	return result;
}

void dispose_thread(
	struct thread_reference *thread_reference)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 168, thread_reference);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 169, thread_reference->in_use);

	CloseHandle(thread_reference->handle);
	thread_reference->handle= NULL;
	thread_reference->in_use= FALSE;

	return;
}

boolean create_mutex(
	struct mutex_reference **mutex_reference)
{
	boolean success= FALSE;
	struct mutex_reference *mutex;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 184, mutex_reference);

	mutex= get_mutex_from_pool();
	if (mutex)
	{
		_snprintf(mutex->name, sizeof(mutex->name), "mutex_%ld", thread_globals.mutex_count++);
		mutex->handle= CreateMutexA(NULL, FALSE, mutex->name);
		if (mutex->handle)
		{
			success= TRUE;
		}
		else
		{
			mutex= NULL;
		}
	}

	*mutex_reference= mutex;
	return success;
}

boolean take_mutex(
	struct mutex_reference *mutex_reference,
	long timeout)
{
	boolean result= FALSE;
	unsigned long wait_result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 211, mutex_reference);

	wait_result= WaitForSingleObject(mutex_reference->handle, timeout);
	if (wait_result==WAIT_OBJECT_0 || wait_result==WAIT_ABANDONED)
	{
		result= TRUE;
	}

	return result;
}

void release_mutex(
	struct mutex_reference *mutex_reference)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 230, mutex_reference);

	ReleaseMutex(mutex_reference->handle);

	return;
}

void dispose_mutex(
	struct mutex_reference *mutex_reference)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 240, mutex_reference);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 241, mutex_reference->in_use);

	CloseHandle(mutex_reference->handle);
	mutex_reference->name[0]= 0;
	mutex_reference->handle= NULL;
	mutex_reference->in_use= FALSE;

	return;
}
