/*
SHELL.H

header included in hcex build.
*/

#ifndef __SHELL_H
#define __SHELL_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/SHELL.C */

boolean shell_initialize(void);
void shell_dispose(void);
boolean shell_application_is_paused(void);
void shell_application_pause(boolean paused);
boolean shell_running_import_tool(void);

/* ---------- prototypes/SHELL_XBOX.C */

boolean shell_platform_initialize(void);
void shell_platform_dispose(void);
void shell_platform_verify(void);
void shell_screen_pause(boolean paused);
void shell_idle(void);

/* ---------- globals */

/* ---------- public code */

#endif // __SHELL_H
