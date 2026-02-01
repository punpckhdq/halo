/*
EDIT_TEXT.H

header included in hcex build.
*/

#ifndef __EDIT_TEXT_H
#define __EDIT_TEXT_H
#pragma once

/* ---------- constants */

/* ---------- macros */

#define valid_edit_text(edit) (edit) && (edit)->buffer && (edit)->maximum_length > 0 && strlen((edit)->buffer) <= (edit)->maximum_length

/* ---------- structures */

struct edit_text
{
	char *buffer;
	short maximum_length;
	short insertion_point_index;
	short selection_start_index;
};


/* ---------- prototypes/EDIT_TEXT.C */

void edit_text_new(struct edit_text *edit);
void edit_text_handle_key(struct edit_text *edit, struct key_stroke const *key);
boolean edit_text_get_selection_indices(struct edit_text *edit, short *start_index, short *end_index);
void edit_text_selection_reset(struct edit_text *edit);

/* ---------- globals */

/* ---------- public code */

#endif // __EDIT_TEXT_H
