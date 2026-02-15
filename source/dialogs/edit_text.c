/*
EDIT_TEXT.C

symbols in this file:
00085920 0080:
	_code_00085920 (0000)
000859A0 0070:
	_edit_text_selection_reset (0000)
00085A10 00a0:
	_edit_text_get_selection_indices (0000)
00085AB0 0060:
	_edit_text_new (0000)
00085B10 0340:
	_edit_text_handle_key (0000)
00259730 0016:
	??_C@_0BG@FJHACPE@valid_edit_text?$CIedit?$CJ?$AA@ (0000)
00259748 0023:
	??_C@_0CD@CHJIFMCD@c?3?2halo?2SOURCE?2dialogs?2edit_text@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "edit_text.h"

#include "input/input.h"
#include "text/international_strings.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void edit_text_fix_selection(struct edit_text *edit);

/* ---------- globals */

/* ---------- public code */

void edit_text_new(
	struct edit_text *edit)
{
	match_assert("c:\\halo\\SOURCE\\dialogs\\edit_text.c", 25, valid_edit_text(edit));
	edit_text_selection_reset(edit);

	return;
}

void edit_text_handle_key(
	struct edit_text *edit,
	struct key_stroke const *key)
{
	boolean backspace;

	match_assert("c:\\halo\\SOURCE\\dialogs\\edit_text.c", 35, valid_edit_text(edit));
	edit_text_fix_selection(edit);
	
	if (key->key_code==_key_backspace)
	{
		backspace= TRUE;
	}

	if (key->key_code==_key_backspace ||
		key->key_code==_key_delete)
	{
		short selection_end;
		short selection_start;

		if (edit_text_get_selection_indices(
			edit,
			&selection_start,
			&selection_end))
		{
			memmove(
				&edit->buffer[selection_start],
				&edit->buffer[selection_end],
				strlen(&edit->buffer[selection_end])+1);
			edit->insertion_point_index= selection_start;
			edit->selection_start_index= NONE;
		}
		else if (key->key_code==_key_backspace &&
				edit->insertion_point_index > 0)
		{
			short src_index= edit->insertion_point_index;
			get_previous_character(
				(unsigned char *)edit->buffer,
				&edit->insertion_point_index);
			memmove(
				&edit->buffer[edit->insertion_point_index],
				&edit->buffer[src_index],
				strlen(&edit->buffer[src_index])+1);
		}
		else if (key->key_code == _key_delete)
		{
			if (edit->insertion_point_index < strlen(edit->buffer))
			{
				short next_index= edit->insertion_point_index;
				get_next_character(
					(unsigned char *)edit->buffer,
					&next_index
				);
				memmove(
					&edit->buffer[edit->insertion_point_index],
					&edit->buffer[next_index],
					strlen(&edit->buffer[next_index])+1);
			}
		}
	}
	else if (
		key->key_code==_key_left_arrow ||
		key->key_code==_key_right_arrow)
	{
		short selection_end;
		short selection_start;

		if (TEST_FLAG(key->modifier_flags, 0) ||
			!edit_text_get_selection_indices(
				edit,
				&selection_start,
				&selection_end))
		{
			if (TEST_FLAG(key->modifier_flags, 0) && edit->selection_start_index==NONE)
			{
				edit->selection_start_index= edit->insertion_point_index;
			}

			if (key->key_code==_key_left_arrow && edit->insertion_point_index>0)
			{
				get_previous_character(
					(unsigned char *)edit->buffer,
					&edit->insertion_point_index);
			}
			else if (key->key_code==_key_right_arrow)
			{
				if (edit->insertion_point_index<strlen(edit->buffer))
				{
					get_next_character(
						(unsigned char *)edit->buffer,
						&edit->insertion_point_index);
				}
			}

			if (edit->selection_start_index==edit->insertion_point_index)
			{
				edit->selection_start_index= NONE;
			}
		}
		else
		{
			if (key->key_code==_key_left_arrow)
			{
				edit->insertion_point_index= selection_start;
				edit->selection_start_index= NONE;
			}
			else
			{
				edit->insertion_point_index= selection_end;
				edit->selection_start_index= NONE;
			}
		}
	}
	else if (key->ascii_code && key->ascii_code!=(byte)NONE)
	{
		short selection_end;
		short selection_start;

		if (edit_text_get_selection_indices(
			edit,
			&selection_start,
			&selection_end))
		{
			memmove(
				&edit->buffer[selection_start+1],
				&edit->buffer[selection_end],
				strlen(&edit->buffer[selection_end])+1);

			edit->insertion_point_index= selection_start;
			edit->selection_start_index= NONE;
			edit->buffer[edit->insertion_point_index++]= key->ascii_code;
		}
		else if (strlen(edit->buffer)<edit->maximum_length)
		{
			memmove(
				&edit->buffer[edit->insertion_point_index+1],
				&edit->buffer[edit->insertion_point_index],
				strlen(&edit->buffer[edit->insertion_point_index])+1);
			edit->buffer[edit->insertion_point_index++]= key->ascii_code;
		}
	}

	align_to_character(
		(unsigned char *)edit->buffer,
		&edit->insertion_point_index);
	
	return;
}

boolean edit_text_get_selection_indices(struct edit_text *edit, short *start_index, short *end_index)
{
	boolean result= FALSE;
	match_assert("c:\\halo\\SOURCE\\dialogs\\edit_text.c", 174, valid_edit_text(edit));
	edit_text_fix_selection(edit);

	if (edit->selection_start_index != NONE)
	{
		*start_index= edit->selection_start_index > edit->insertion_point_index ? edit->insertion_point_index : edit->selection_start_index;
		*end_index= edit->selection_start_index > edit->insertion_point_index ? edit->selection_start_index : edit->insertion_point_index;
		result= TRUE;
	}

	return result;
}

void edit_text_selection_reset(
	struct edit_text *edit)
{
	match_assert("c:\\halo\\SOURCE\\dialogs\\edit_text.c", 159, valid_edit_text(edit));
	edit_text_fix_selection(edit);
	edit->insertion_point_index= (short)strlen(edit->buffer);
	edit->selection_start_index= NONE;

	return;
}

/* ---------- private code */

static void edit_text_fix_selection(
	struct edit_text *edit)
{
	short length= (short)strlen(edit->buffer);
	edit->insertion_point_index= PIN(edit->insertion_point_index, 0, length);
	edit->selection_start_index= PIN(edit->selection_start_index, NONE, length);

	if (edit->insertion_point_index == edit->selection_start_index)
	{
		edit->selection_start_index= NONE;
	}

	align_to_character((unsigned char *)edit->buffer, &edit->insertion_point_index);
	if (edit->selection_start_index != NONE)
	{
		align_to_character((unsigned char*)edit->buffer, &edit->selection_start_index);
	}

	return;
}
