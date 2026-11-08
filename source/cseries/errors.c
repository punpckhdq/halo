/*
ERRORS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "build_number.h"
#include "terminal.h"

/* ---------- constants */

enum
{
	ERROR_IMMEDIATE_EXIT_CODE = -4998, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct error_global_data
{
	boolean delayed;
	boolean output_to_debug_file;
	boolean display_state;
	boolean recursion_lock;
	boolean overflow_suppression;
	boolean suppress_all;
	short message_buffer_size;
	char message_buffer[ERROR_MESSAGE_BUFFER_MAXIMUM_SIZE];
};

/* ---------- prototypes */

static void reset_error_state(void);

/* ---------- globals */

struct error_global_data error_globals;
boolean find_all_fucked_up_shit = FALSE;
long fucked_up_shit_count = 0;

/* ---------- public code */

void errors_dispose(
	void)
{
	stack_walk_dispose();

	return;
}

void errors_output_to_debug_file(
	boolean output_to_debug_file)
{
	error_globals.output_to_debug_file = output_to_debug_file;

	return;
}

void errors_overflow_suppression_enable(
	boolean overflow_suppression)
{
	error_globals.overflow_suppression = overflow_suppression;

	return;
}

char const *error_get(
	void)
{
	return error_globals.message_buffer;
}

void write_to_error_file(
	char *string,
	boolean date)
{
	static boolean first_line = TRUE;

	char line[1024];
	long time_value;

	if (first_line)
	{
		first_line = FALSE;
		write_to_error_file("\r\n\r\n", FALSE);
		write_to_error_file(BUILD_STRING "(CACHE) ----------------------------------------------\r\n", TRUE);
		sprintf(line, "reference function: %s\r\n", "_write_to_error_file");
		write_to_error_file(line, TRUE);
		sprintf(line, "reference address: %x\r\n", write_to_error_file);
		write_to_error_file(line, TRUE);
	}

	if (error_globals.output_to_debug_file)
	{
		FILE *handle = fopen("d:\\debug.txt", "a+b");

		if (handle)
		{
			if (date)
			{
				struct tm *time_struct;

				time(&time_value);
				time_struct = localtime(&time_value);

				if (time_struct)
				{
					fprintf(
						handle,
						"%02d.%02d.%02d %02d:%02d:%02d  ",
						time_struct->tm_mon + 1,
						time_struct->tm_mday,
						time_struct->tm_year % 100,
						time_struct->tm_hour,
						time_struct->tm_min,
						time_struct->tm_sec);
				}
				else
				{
					fprintf(handle, "<TIME UNAVAILABLE>  ");
				}
			}

			fprintf(handle, "%s", string);
			fclose(handle);
		}
	}

	return;
}

static void reset_error_state(
	void)
{
	error_globals.delayed = FALSE;
	error_globals.message_buffer_size = 0;

	return;
}

void errors_initialize(
	void)
{
	error_globals.output_to_debug_file = TRUE;
	error_globals.overflow_suppression = TRUE;
	reset_error_state();
	stack_walk_initialize();

	return;
}

void error(
	short priority,
	char const *format,
	...)
{
	char buffer[1024];

	match_assert("c:\\halo\\SOURCE\\cseries\\errors.c", 97, priority>=0 && priority<NUMBER_OF_ERROR_MESSAGE_PRIORITIES);

	if (error_globals.overflow_suppression && priority == _error_silent)
	{
		static long last_error_time = 0;
		static long error_count = 0;
		long current_time = system_milliseconds();
		long error_interval = 900;
		long maximum_error_count = 10;

		if (current_time > last_error_time + error_interval)
		{
			error_count = 0;
		}

		last_error_time = current_time;

		if (error_count == maximum_error_count)
		{
			terminal_printf(global_real_argb_white, "too many errors, only printing to debug.txt");
		}

		error_count++;

		if (error_count >= maximum_error_count)
		{
			priority = _error_log;
		}
	}

	if (!error_globals.recursion_lock)
	{
		error_globals.recursion_lock = TRUE;

		if (priority == _error_delayed)
		{
			error_globals.delayed = TRUE;
		}

		if (format)
		{
			va_list arglist;
			long new_size;

			va_start(arglist, format);
			vsprintf(buffer, format, arglist);
			va_end(arglist);
			strcat(buffer, "\r\n");

			if (priority != _error_log)
			{
				terminal_printf(global_real_argb_white, "%s", buffer);
			}

			write_to_error_file(buffer, TRUE);

			new_size = strlen(buffer);

			if (error_globals.message_buffer_size + new_size >= ERROR_MESSAGE_BUFFER_MAXIMUM_SIZE)
			{
				char const *prefix = "[...too many errors to print...]\r\n";
				long prefix_size = strlen(prefix);
				char *search_start = error_globals.message_buffer + PIN(prefix_size + new_size + ERROR_MESSAGE_BUFFER_MAXIMUM_SIZE / 2, 0, error_globals.message_buffer_size - 1);
				char *copy_start = strchr(search_start, '\n');
				long copy_offset = copy_start == NULL ? error_globals.message_buffer_size : copy_start - error_globals.message_buffer + 1;
				long copy_size = error_globals.message_buffer_size - copy_offset;

				match_assert("c:\\halo\\SOURCE\\cseries\\errors.c", 191, prefix_size + copy_size + new_size < ERROR_MESSAGE_BUFFER_MAXIMUM_SIZE);

				strncpy(error_globals.message_buffer, prefix, prefix_size);

				if (copy_size > 0)
				{
					memmove(error_globals.message_buffer + prefix_size, copy_start, copy_size);
				}

				error_globals.message_buffer[prefix_size + copy_size] = '\0';
				error_globals.message_buffer_size = (short)(prefix_size + copy_size);
			}

			if (error_globals.message_buffer_size + new_size < ERROR_MESSAGE_BUFFER_MAXIMUM_SIZE)
			{
				strcpy(error_globals.message_buffer + error_globals.message_buffer_size, buffer);
				error_globals.message_buffer_size += (short)new_size;
			}
		}

		if (priority == _error_immediate)
		{
			system_exit(ERROR_IMMEDIATE_EXIT_CODE);
		}

		error_globals.recursion_lock = FALSE;
	}

	return;
}

boolean errors_handle(
	void)
{
	boolean delayed = error_globals.delayed;

	reset_error_state();

	return delayed;
}

void errors_clear(
	void)
{
	reset_error_state();

	return;
}

/* ---------- private code */
