/*
EVENT_MANAGER.H

header included in hcex build.
*/

#ifndef __EVENT_MANAGER_H
#define __EVENT_MANAGER_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/EVENT_MANAGER.C */

void event_manager_initialize(void);
void event_manager_dispose(void);
unsigned long event_manager_time_of_last_event(void);

void event_manager_update(void);
void event_manager_flush(void);
void event_manager_suppress(boolean suppress);

/* ---------- globals */

/* ---------- public code */

#endif // __EVENT_MANAGER_H
