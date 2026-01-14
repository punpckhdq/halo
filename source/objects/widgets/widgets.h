/*
WIDGETS.H

header included in hcex build.
*/

#ifndef __WIDGETS_H
#define __WIDGETS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/WIDGETS.C */

void widgets_initialize(void);
void widgets_initialize_for_new_map(void);
void widgets_dispose_from_old_map(void);
void widgets_dispose(void);
void widgets_new(long object_index);
void widgets_delete(long object_index);

/* ---------- globals */

/* ---------- public code */

#endif // __WIDGETS_H
