/*
UI_WIDGET_TEXT_SEARCH_AND_REPLACE_FUNCTIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ui_widget_text_search_and_replace_functions.h"
#include "ui_widget.h"
#include "cheats.h"
#include "game.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "input.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "build_number.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

typedef wchar_t const *(*search_and_replace_proc)(struct widget_instance *widget); /* fake name */

/* ---------- prototypes */

static wchar_t const *widget_replace_function_null(struct widget_instance *widget);
static wchar_t const *widget_controller(struct widget_instance *widget);

/* ---------- globals */

static search_and_replace_proc search_and_replace_functions[NUMBER_OF_SEARCH_AND_REPLACE_FUNCTIONS] = /* fake name */
{
	widget_replace_function_null,
	widget_controller,
};

/* ---------- public code */

static wchar_t const *widget_replace_function_null(
	struct widget_instance *widget)
{
	return L"";
}

static wchar_t const *widget_controller(
	struct widget_instance *widget)
{
	static wchar_t buffer[2];

	switch (widget->local_player_index)
	{
	case NONE:
		buffer[0] = L'1';
		break;
	case 0:
		buffer[0] = L'1';
		break;
	case 1:
		buffer[0] = L'2';
		break;
	case 2:
		buffer[0] = L'3';
		break;
	case 3:
		buffer[0] = L'4';
		break;
	default:
		buffer[0] = L'?';
		break;
	}

	buffer[NUMBEROF(buffer) - 1] = L'\0';

	return buffer;
}

wchar_t const *ui_widget_search_and_replace_invoke(
	struct widget_instance *widget,
	short function)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_text_search_and_replace_functions.c", 45, widget);

	return (function >= 0 && function < NUMBEROF(search_and_replace_functions)) ? search_and_replace_functions[function](widget) : L"<invalid>";
}

/* ---------- private code */
