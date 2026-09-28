/*
THREAD_WIN32.C
*/

/* ---------- headers */

#include "cseries.h"
#include "thread.h"

/* ---------- prototypes */

static struct thread *get_thread_from_pool(void);
static struct mutex *get_mutex_from_pool(void);

/* ---------- globals */

static unsigned long mutex_count;
static struct thread thread_pool[MAXIMUM_THREADS];
static struct mutex mutex_pool[MAXIMUM_MUTEXES];

/* ---------- public code */

boolean create_thread(
	word flags,
	LPTHREAD_START_ROUTINE function,
	void *function_input,
	struct thread **thread_reference)
{
	boolean success = FALSE;
	struct thread *thread;
	unsigned long unused_thread_id;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 107, function);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 108, thread_reference);

	thread = get_thread_from_pool();
	if (thread && (thread->object = CreateThread(NULL, THREAD_STACK_SIZE, function, function_input, CREATE_SUSPENDED, &unused_thread_id))!=NULL)
	{
		long priority = THREAD_PRIORITY_NORMAL;

		if (TEST_FLAG(flags, _thread_attribute_flag_priority_low))
		{
			priority = THREAD_PRIORITY_BELOW_NORMAL;
		}
		else if (TEST_FLAG(flags, _thread_attribute_flag_priority_high))
		{
			priority = THREAD_PRIORITY_ABOVE_NORMAL;
		}

		if (SetThreadPriority(thread->object, priority) && ResumeThread(thread->object)!=-1)
		{
			success = TRUE;
		}
		else
		{
			CloseHandle(thread->object);
			thread = NULL;
		}
	}

	*thread_reference = thread;
	return success;
}

boolean thread_has_exited(
	struct thread *thread_reference)
{
	boolean result = FALSE;
	unsigned long exit_code;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 152, thread_reference);

	if (GetExitCodeThread(thread_reference->object, &exit_code) && exit_code!=STILL_ACTIVE)
	{
		result = TRUE;
	}

	return result;
}

void dispose_thread(
	struct thread *thread_reference)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 168, thread_reference);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 169, thread_reference->in_use);

	CloseHandle(thread_reference->object);
	thread_reference->object = NULL;
	thread_reference->in_use = FALSE;

	return;
}

boolean create_mutex(
	struct mutex **mutex_reference)
{
	boolean success = FALSE;
	struct mutex *mutex;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 184, mutex_reference);

	mutex = get_mutex_from_pool();
	if (mutex)
	{
		_snprintf(mutex->name, sizeof(mutex->name), "mutex_%ld", mutex_count++);
		mutex->object = CreateMutexA(NULL, FALSE, mutex->name);
		if (mutex->object)
		{
			success = TRUE;
		}
		else
		{
			mutex = NULL;
		}
	}

	*mutex_reference = mutex;
	return success;
}

boolean take_mutex(
	struct mutex *mutex_reference,
	unsigned long milliseconds_to_wait)
{
	boolean result = FALSE;
	unsigned long wait_result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 211, mutex_reference);

	wait_result = WaitForSingleObject(mutex_reference->object, milliseconds_to_wait);
	if (wait_result==WAIT_OBJECT_0 || wait_result==WAIT_ABANDONED)
	{
		result = TRUE;
	}

	return result;
}

void release_mutex(
	struct mutex *mutex_reference)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 230, mutex_reference);

	ReleaseMutex(mutex_reference->object);

	return;
}

void dispose_mutex(
	struct mutex *mutex_reference)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 240, mutex_reference);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\thread_win32.c", 241, mutex_reference->in_use);

	CloseHandle(mutex_reference->object);
	mutex_reference->name[0] = 0;
	mutex_reference->object = NULL;
	mutex_reference->in_use = FALSE;

	return;
}

/* ---------- private code */

static struct thread *get_thread_from_pool(
	void)
{
	struct thread *thread = NULL;
	long i;

	for (i = 0; i<MAXIMUM_THREADS; i++)
	{
		if (!thread_pool[i].in_use)
		{
			thread = &thread_pool[i];
			thread->object = NULL;
			thread->in_use = TRUE;
			break;
		}
	}

	return thread;
}

static struct mutex *get_mutex_from_pool(
	void)
{
	struct mutex *mutex = NULL;
	long i;

	for (i = 0; i<MAXIMUM_MUTEXES; i++)
	{
		if (!mutex_pool[i].in_use)
		{
			mutex = &mutex_pool[i];
			mutex->name[0] = 0;
			mutex->object = NULL;
			mutex->in_use = TRUE;
			break;
		}
	}

	return mutex;
}
