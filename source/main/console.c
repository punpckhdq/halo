/*
CONSOLE.C

symbols in this file:
000EEC10 0060:
	_console_initialize (0000)
000EEC70 0010:
	_console_initialize_for_new_map (0000)
000EEC80 0030:
	_console_open (0000)
000EECB0 0020:
	_console_close (0000)
000EECD0 0010:
	_console_is_active (0000)
000EECE0 0080:
	_console_printf (0000)
000EED60 0080:
	_console_warning (0000)
000EEDE0 0070:
	_code_000eede0 (0000)
000EEE50 0040:
	_code_000eee50 (0000)
000EEE90 0220:
	_code_000eee90 (0000)
000EF0B0 0120:
	_console_startup (0000)
000EF1D0 0020:
	_console_dispose (0000)
000EF1F0 0220:
	_console_update (0000)
0027944C 0007:
	??_C@_06IFDPLABP@halo?$CI?5?$AA@ (0000)
00279454 0009:
	??_C@_08NDPNIJOB@init?3?5?$CFs?$AA@ (0000)
00279460 0004:
	??_C@_03PAIEMGKB@?$AN?6?7?$AA@ (0000)
00279464 0013:
	??_C@_0BD@MEEDMDNF@editor_d?3?2init?4txt?$AA@ (0000)
00279478 000c:
	??_C@_0M@PGLCPLJK@d?3?2init?4txt?$AA@ (0000)
00279484 0014:
	??_C@_0BE@PBECEFPN@key?9?$DOkey_code?$CB?$DNNONE?$AA@ (0000)
00279498 001e:
	??_C@_0BO@NFBJJCKH@c?3?2halo?2SOURCE?2main?2console?4c?$AA@ (0000)
00307740 0010:
	_data_00307740 (0000)
00454D80 09c5:
	_bss_00454d80 (0000)
	_console_dump_to_file (09c4)
*/

/* ---------- headers */

#include "cseries.h"
#include "console.h"

#include "cseries/errors.h"
#include "cseries/profile.h"
#include "editor/editor_stubs.h"
#include "hs/hs.h"
#include "input/input.h"
#include "interface/terminal.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS= 8,
};

/* ---------- macros */

/* ---------- structures */

struct console_globals
{
	boolean active;
	boolean available;
	struct terminal_gets_state input_state;
	char previous_commands[MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS][255];
	short previous_command_count;
	short newest_previous_command_index;
	short selected_previous_command_index;
};

/* ---------- prototypes */

static boolean console_process_command(const char *command);

static char *console_get_text_to_autocomplete(void);

static void console_complete(void);

/* ---------- globals */

boolean console_dump_to_file= FALSE;

static real_argb_color console_color= {1.f, 1.f, 0.3f, 1.f};	// Pink

static struct console_globals console_globals;

/* ---------- public code */

void console_initialize(
	void)
{
	console_globals.input_state.color= console_color;
	csstrcpy(console_globals.input_state.prompt, "halo( ");
	console_globals.input_state.result[0]= 0;
	console_globals.newest_previous_command_index= NONE;
	console_globals.previous_command_count= 0;
	console_globals.selected_previous_command_index= NONE;

	return;
}

void console_initialize_for_new_map(
	void)
{
	return;
}

void console_open(
	void)
{
	if (!console_globals.active)
	{
		console_globals.input_state.result[0]= '\0';
		console_globals.active= terminal_gets_begin(&console_globals.input_state);
		profile_global_enable= FALSE;
	}

	return;
}

void console_close(
	void)
{
	if (console_globals.active)
	{
		terminal_gets_end(&console_globals.input_state);
		console_globals.active= FALSE;
	}

	return;
}

boolean console_is_active(
	void)
{
	return console_globals.active;
}

void console_printf(
	boolean clear,
	const char *format,
	...)
{
	char buffer[1024];
	va_list arglist;

	va_start(arglist, format);

	if (clear)
	{
		terminal_clear();
	}
	
	vsprintf(buffer, format, arglist);
	buffer[255]= '\0';
	
	terminal_printf(0, "%s", buffer);
	if (console_dump_to_file)
	{
		strncat(buffer, "\r\n", NUMBEROF(buffer));
		write_to_error_file(buffer, TRUE);
	}

	va_end(arglist);

	return;
}

void console_warning(
	const char *format,
	...)
{
	char buffer[1024];
	va_list arglist;

	va_start(arglist, format);


	vsprintf(buffer, format, arglist);
	buffer[255]= '\0';

	terminal_printf(global_real_argb_red, "%s", buffer);
	if (console_dump_to_file)
	{
		csstrncat(buffer, "\r\n", NUMBEROF(buffer));
		write_to_error_file(buffer, TRUE);
	}

	va_end(arglist);

	return;
}

static boolean console_process_command(
	const char *command)
{
	boolean valid;
	short previous_command_count;
	short newest_previous_command_index= (console_globals.newest_previous_command_index + 1) % MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS;

	console_globals.newest_previous_command_index= newest_previous_command_index;
	strcpy(console_globals.previous_commands[newest_previous_command_index], command);


	console_globals.previous_command_count= MIN(console_globals.previous_command_count + 1, MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS);
	console_globals.selected_previous_command_index= NONE;

	return hs_compile_and_evaluate(command);
}

static char *console_get_text_to_autocomplete(
	void)
{
	char *result= console_globals.input_state.result;
	char *result_after_space= strrchr(console_globals.input_state.result, ' ') + 1;
	char *result_after_parenthesis= strrchr(console_globals.input_state.result, '(') + 1;
	char *result_after_quote= strrchr(console_globals.input_state.result, '"') + 1;

	if (result <= result_after_space)
	{
		result= result_after_space;
	}
	if (result <= result_after_parenthesis)
	{
		result= result_after_parenthesis;
	}
	if (result <= result_after_quote)
	{
		result= result_after_quote;
	}

	return result;
}

static void console_complete(
	void)
{
	char *matching_items[256];
	char print_buffer[1024];

	char *token= console_get_text_to_autocomplete();
	short count= hs_tokens_enumerate(token, NONE, matching_items, NUMBEROF(matching_items));

	if (count)
	{
		short token_num;

		short last_similar_character_index= SHORT_MAX;
		boolean print_second_column= count > 16;

		print_buffer[0]= '\0';
		console_printf(FALSE, "");

		for (token_num= 0; token_num<count; token_num++)
		{
			short size= MIN(last_similar_character_index, strlen(matching_items[token_num]) - 1);
			short index;

			for (index= 0; ; ++index)
			{
				if (tolower(matching_items[token_num][index]) != tolower(matching_items[0][index]) || index > size)
				{
					break;
				}
			}
			last_similar_character_index= index - 1;

			if (print_second_column)
			{
				csstrcat(print_buffer, matching_items[token_num]);
				csstrcat(print_buffer, "|t");
				if (token_num % 4 == 3)
				{
					console_printf(FALSE, print_buffer);
					print_buffer[0]= '\0';
				}
			}
			else
			{
				console_printf(FALSE, matching_items[token_num]);
			}
		}

		if (print_second_column && (token_num - 1) % 4 != 3)
		{
			console_printf(FALSE, print_buffer);
		}

		strncpy(token, matching_items[0], last_similar_character_index + 1);
		token[last_similar_character_index + 1]= '\0';
		console_globals.input_state.edit.insertion_point_index= &token[last_similar_character_index + 1] - console_globals.input_state.result;
	}

	return;
}

void console_startup(
	void)
{
	char buffer[200];
	FILE *file;
	short newest_previous_command_index;

	if (!game_in_editor())
	{
		file= fopen("d:\\init.txt", "r");
	}
	else
	{
		file= fopen("editor_d:\\init.txt", "r");
	}

	if (file)
	{
		while (fgets(buffer, NUMBEROF(buffer)-1, file))
		{
			strtok(buffer, "\r\n\t");
			newest_previous_command_index= (console_globals.newest_previous_command_index + 1) % MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS;

			console_globals.newest_previous_command_index= newest_previous_command_index;
			strcpy(console_globals.previous_commands[newest_previous_command_index], buffer);

			console_globals.previous_command_count= MIN(console_globals.previous_command_count + 1, MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS);

			console_globals.selected_previous_command_index= NONE;
			if (hs_compile_and_evaluate(buffer))
			{
				error(_error_log, "init: %s", buffer);
			}
		}

		fclose(file);
	}

	return;
}

void console_dispose(
	void)
{
	console_close();

	return;
}

boolean console_update(
	void)
{
	boolean prev_command_valid;
	boolean last_command;

	short i;

	if (console_globals.active)
	{
		for (i= 0; i < console_globals.input_state.key_count; i++)
		{
			struct key_stroke *key= &console_globals.input_state.keys[i];
			match_assert("c:\\halo\\SOURCE\\main\\console.c", 184, key->key_code!=NONE);
			switch (key->key_code)
			{
			case _key_backquote:
				console_close();
				break;
			case _key_return:
			case _keypad_enter:
				if (console_globals.input_state.result[0])
				{
					console_process_command(console_globals.input_state.result);
					console_globals.input_state.result[0]= '\0';
				}
				else
				{
					console_close();
				}
				break;
			case _key_tab:
				console_complete();
				break;
			case _key_up_arrow:
				console_globals.selected_previous_command_index += 2;
			case _key_down_arrow:
				last_command= --console_globals.selected_previous_command_index > 0;
				console_globals.selected_previous_command_index= last_command ? console_globals.selected_previous_command_index : 0;

				prev_command_valid= console_globals.selected_previous_command_index>console_globals.previous_command_count-1;
				console_globals.selected_previous_command_index= 
				(
					prev_command_valid ? 
					console_globals.previous_command_count-1 :
					console_globals.selected_previous_command_index
				);

				if (console_globals.selected_previous_command_index != NONE)
				{
					strcpy(
						console_globals.input_state.result,
						console_globals.previous_commands[
							(console_globals.newest_previous_command_index - console_globals.selected_previous_command_index + MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS) % MAXIMUM_NUMBER_OF_PREVIOUS_COMMANDS
						]
					);
					edit_text_selection_reset(&console_globals.input_state.edit);
				}
			default:
				break;
			}
		}
	}
	else if (input_key_is_down(_key_backquote) == TRUE && !console_globals.active)
	{
		console_globals.input_state.result[0]= '\0';
		console_globals.active= terminal_gets_begin(&console_globals.input_state);
		profile_global_enable= FALSE;
	}

	return console_globals.active;
}
