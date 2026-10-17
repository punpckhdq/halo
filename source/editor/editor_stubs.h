/*
EDITOR_STUBS.H
*/

#ifndef __EDITOR_STUBS_H
#define __EDITOR_STUBS_H
#pragma once

/* ---------- prototypes/EDITOR_STUBS.C */

void editor_render(void);
boolean game_in_editor(void);
boolean editor_preprocess_rendered_object(long object_index, struct render_lighting const *lighting);
boolean editor_should_exit(void);
void editor_initialize(void);
void editor_dispose(void);
void editor_update(void);
void editor_initialize_for_new_map(void);
void editor_dispose_from_old_map(void);

#endif // __EDITOR_STUBS_H
