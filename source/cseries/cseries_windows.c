/*
CSERIES_WINDOWS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "build_number.h"
#include "strings/resource.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static char *exception_code_get_string(unsigned long exception_code);

/* ---------- globals */

/* ---------- public code */

#ifdef DEBUG
void display_debug_string(
	char const *string)
{
	OutputDebugString(string);

	return;
}
#endif

void system_exit(
	long code)
{
	halt_and_catch_fire();

	return;
}

void system_unique_identifier_get(
	struct system_unique_identifier *identifier)
{
	match_halt("c:\\halo\\SOURCE\\cseries\\cseries_windows.c", 65);

	return;
}

boolean system_unique_identifiers_equal(
	struct system_unique_identifier const *identifier1,
	struct system_unique_identifier const *identifier2)
{
	struct system_unique_identifier default_identifier;

	memset(&default_identifier, 0, sizeof(default_identifier));

	return memcmp(identifier1, &default_identifier, sizeof(default_identifier)) && !memcmp(identifier1, identifier2, sizeof(*identifier1));
}

unsigned long system_milliseconds(
	void)
{
	return GetTickCount();
}

unsigned long system_seconds(
	void)
{
	return time(NULL);
}

void system_get_user_name(
	char *name,
	short maximum_length)
{
	strncpy(name, "xbox", maximum_length);

	return;
}

void *system_calloc(
	unsigned int num,
	unsigned int size)
{
	return GlobalAlloc(GMEM_ZEROINIT, num * size);
}

void *system_malloc(
	unsigned int size)
{
	return GlobalAlloc(GMEM_FIXED, size);
}

void system_free(
	void *pointer)
{
	GlobalFree(pointer);

	return;
}

void *system_realloc(
	void *pointer,
	long size)
{
	void *result;

	match_assert("c:\\halo\\SOURCE\\cseries\\cseries_windows.c", 156, size>=0);

	if (!pointer)
	{
		match_assert("c:\\halo\\SOURCE\\cseries\\cseries_windows.c", 157, pointer||size);
		result = GlobalAlloc(GMEM_FIXED, size);
	}
	else if (size)
	{
		result = GlobalReAlloc(pointer, size, GMEM_MOVEABLE);
	}
	else
	{
		GlobalFree(pointer);
		result = NULL;
	}

	return result;
}

unsigned int system_get_used_memory_size(
	void *pointer)
{
	return GlobalSize(pointer);
}

void system_memory_information_get(
	struct system_memory_information *information)
{
	MEMORYSTATUS status;

	memset(&status, 0, sizeof(status));
	status.dwLength = sizeof(status);
	GlobalMemoryStatus(&status);

	memset(information, 0, sizeof(*information));
	information->free = status.dwAvailPhys;
	information->total = status.dwTotalPhys;

	return;
}

void system_show_wait_cursor(
	void)
{
	return;
}

void system_alert(
	char const *string)
{
	return;
}

void system_kill_screen_saver(
	void)
{
	return;
}

long generic_exception_filter(
	unsigned long exception_code,
	PEXCEPTION_POINTERS exception_information)
{
	char *exception_string = exception_code_get_string(exception_code);

	stack_walk_with_context(NULL, 0, exception_information->ContextRecord);

	if (exception_string)
	{
		error(_error_silent, "%s", exception_string);
	}
	else
	{
		error(_error_silent, "unknown exception %08lX", exception_code);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

/* ---------- private code */

static char *exception_code_get_string(
	unsigned long exception_code)
{
	char *result = NULL;

	switch (exception_code)
	{
	case EXCEPTION_BREAKPOINT:
		result = "EXCEPTION_BREAKPOINT";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		result = "EXCEPTION_DATATYPE_MISALIGNMENT";
		break;
	case EXCEPTION_SINGLE_STEP:
		result = "EXCEPTION_SINGLE_STEP";
		break;
	case EXCEPTION_ACCESS_VIOLATION:
		result = "EXCEPTION_ACCESS_VIOLATION";
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		result = "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		result = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		result = "EXCEPTION_FLT_DENORMAL_OPERAND";
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		result = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		result = "EXCEPTION_FLT_INEXACT_RESULT";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		result = "EXCEPTION_FLT_INVALID_OPERATION";
		break;
	case EXCEPTION_FLT_OVERFLOW:
		result = "EXCEPTION_FLT_OVERFLOW";
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		result = "EXCEPTION_FLT_STACK_CHECK";
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		result = "EXCEPTION_FLT_UNDERFLOW";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		result = "EXCEPTION_INT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_INT_OVERFLOW:
		result = "EXCEPTION_INT_OVERFLOW";
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		result = "EXCEPTION_PRIV_INSTRUCTION";
		break;
	}

	return result;
}
