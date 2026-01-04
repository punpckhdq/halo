/*
CONSOLE.H

header included in hcex build.
*/

#ifndef __CONSOLE_H
#define __CONSOLE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/CONSOLE.C */

void console_initialize(void);
void console_initialize_for_new_map(void);
void console_open(void);
void console_close(void);
boolean console_is_active(void);
void console_printf(boolean clear, const char *format, ...);
void console_warning(const char *format, ...);
void console_startup(void);
void console_dispose(void);
boolean console_update(void);

/* ---------- globals */

extern boolean console_dump_to_file;

/* ---------- public code */

#endif // __CONSOLE_H
