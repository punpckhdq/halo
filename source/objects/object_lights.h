/*
OBJECT_LIGHTS.H

header included in hcex build.
*/

#ifndef __OBJECT_LIGHTS_H
#define __OBJECT_LIGHTS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/OBJECT_LIGHTS.C */

void lights_initialize(void);
void lights_dispose(void);
void lights_initialize_for_new_map(void);
void lights_dispose_from_old_map(void);
boolean lights_enable(boolean enable);
long light_new(long definition_index, long object_index, short object_attachment_index, short object_function_index, short object_change_color_index);
void light_delete(long light_index);

void light_disconnect_from_map(long light_index);
void light_reconnect_to_map(long light_index);

/* ---------- globals */

/* ---------- public code */

#endif // __OBJECT_LIGHTS_H
