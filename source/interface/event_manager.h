/*
EVENT_MANAGER.H

header included in hcex build.
*/

#ifndef __EVENT_MANAGER_H
#define __EVENT_MANAGER_H
#pragma once

/* ---------- constants */

enum
{
	_event_type_none = 0,
	_event_type_left_analog_stick,
	_event_type_right_analog_stick,
	_event_type_button
};

/* ---------- macros */

/* ---------- structures */

struct event_record
{
	short type;
	short controller_index;
	union
	{
		point2d analog_value;
		struct
		{
			byte button;
			byte value;
		} button_value;
	} value;
};

/* ---------- prototypes/EVENT_MANAGER.C */

void event_manager_initialize(void);
void event_manager_dispose(void);
void event_manager_update(void);
void event_manager_flush(void);
void event_manager_suppress(boolean suppress);
boolean get_next_event(struct event_record *event, short local_player_index);
unsigned long event_manager_time_of_last_event(void);

/* ---------- globals */

/* ---------- public code */

#endif // __EVENT_MANAGER_H
