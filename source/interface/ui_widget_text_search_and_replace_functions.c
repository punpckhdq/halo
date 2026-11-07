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

/* ---------- prototypes */

static wchar_t *widget_replace_function_null(struct widget_instance *widget);
static wchar_t *widget_controller(struct widget_instance *widget);

/* ---------- globals */

static wchar_t *(*replace_function_list[])(struct widget_instance *widget) =
{
	widget_replace_function_null,
	widget_controller,
};

/* ---------- public code */

wchar_t *ui_widget_search_and_replace_invoke(
	struct widget_instance *widget,
	short function)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_text_search_and_replace_functions.c", 45, widget);

	return (function >= 0 && function < NUMBEROF(replace_function_list)) ? replace_function_list[function](widget) : L"<invalid>";
}

static wchar_t *widget_replace_function_null(
	struct widget_instance *widget)
{
	return L"";
}

static wchar_t *widget_controller(
	struct widget_instance *widget)
{
	static wchar_t result[2];

	switch (widget->local_player_index)
	{
	case NONE:
		result[0] = L'1';
		break;
	case 0:
		result[0] = L'1';
		break;
	case 1:
		result[0] = L'2';
		break;
	case 2:
		result[0] = L'3';
		break;
	case 3:
		result[0] = L'4';
		break;
	default:
		result[0] = L'?';
		break;
	}

	result[NUMBEROF(result) - 1] = L'\0';

	return result;
}
