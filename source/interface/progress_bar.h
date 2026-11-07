/*
PROGRESS_BAR.H

header included in hcex build.
*/

#ifndef __PROGRESS_BAR_H
#define __PROGRESS_BAR_H
#pragma once

/* ---------- headers */


/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PROGRESS_BAR.C */

void progress_bar_initialize(void);
void progress_bar_dispose(void);
void progress_bar_begin(boolean ui_is_up);
void progress_bar_end(void);
boolean progress_bar_is_active(void);
void progress_bar_eachframe(void);
void progress_bar_display(real progress);
void progress_bar_enable(boolean enable);

/* ---------- globals */

/* ---------- public code */

#endif // __PROGRESS_BAR_H
