/*
UI_WIDGET_EVENT_HANDLER_FUNCTIONS.H

header included in hcex build.
*/

#ifndef __UI_WIDGET_EVENT_HANDLER_FUNCTIONS_H
#define __UI_WIDGET_EVENT_HANDLER_FUNCTIONS_H
#pragma once

/* ---------- constants */

enum
{
	NUMBER_OF_MULTIPLAYER_LEVELS = 13	/* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct _single_player_level_data
{
	char *scenario_tag_path;
	boolean level_completed[4];
};

struct _persistant_game_data_info
{
	char map_name[MAXIMUM_FILENAME_LENGTH+1];
	short difficulty;
	char map_index;
	boolean valid;
	boolean corrupted;
};

/* ---------- prototypes/UI_WIDGET_EVENT_HANDLER_FUNCTIONS.C */

boolean ui_widget_event_handler_function_invoke(struct widget_instance *widget, struct event_record *event, short function, boolean *widget_deleted);

/* ---------- globals */

extern struct enum_definition event_handler_functions;
extern struct _single_player_level_data single_player_level_data[];
extern struct _persistant_game_data_info persistant_game_data_info;

/* ---------- public code */

#endif // __UI_WIDGET_EVENT_HANDLER_FUNCTIONS_H
