/*
EDITOR_STUBS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "editor_stubs.h"

/* ---------- public code */

void editor_render(
	void)
{
	return;
}

boolean game_in_editor(
	void)
{
	return FALSE;
}

boolean editor_preprocess_rendered_object(
	long object_index,
	struct render_lighting const *lighting)
{
	return TRUE;
}

boolean editor_should_exit(
	void)
{
	return FALSE;
}

void editor_initialize(
	void)
{
	return;
}

void editor_dispose(
	void)
{
	return;
}

void editor_update(
	void)
{
	return;
}

void editor_initialize_for_new_map(
	void)
{
	return;
}

void editor_dispose_from_old_map(
	void)
{
	return;
}
