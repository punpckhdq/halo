/*
VIRTUAL_KEYBOARD.H

header included in hcex build.
*/

#ifndef __VIRTUAL_KEYBOARD_H
#define __VIRTUAL_KEYBOARD_H
#pragma once

/* ---------- constants */

enum
{
	VIRTUAL_KEYBOARD_TAG = 'vcky',
	VIRTUAL_KEYBOARD_VERSION = 2,
};

/* ---------- macros */

#define virtual_keyboard_definition_get(index) ((struct virtual_keyboard *)tag_get(VIRTUAL_KEYBOARD_TAG, index)) /* fake name */

/* ---------- structures */

struct virtual_key
{
	word virtual_key;
	wchar_t lowercase_character;
	wchar_t shift_character;
	wchar_t caps_character;
	wchar_t symbols_character;
	wchar_t shift_caps_character;
	wchar_t shift_symbols_character;
	wchar_t caps_symbols_character;
	struct tag_reference unselected_background_bitmap_tag;
	struct tag_reference selected_background_bitmap_tag;
	struct tag_reference active_background_bitmap_tag;
	struct tag_reference sticky_background_bitmap_tag;
};

struct virtual_keyboard
{
	struct tag_reference font_tag;
	struct tag_reference background_bitmap_tag;
	struct tag_reference special_key_labels_string_list_tag;
	struct tag_block virtual_keys;		// virtual_key
};

/* ---------- prototypes/VIRTUAL_KEYBOARD.C */

boolean virtual_keyboard_initialize(void);
void virtual_keyboard_dispose(void);
boolean virtual_keyboard_launch(wchar_t *text_buffer, word buffer_size, short caption_index);
boolean virtual_keyboard_close(void);
boolean virtual_keyboard_active(void);
void virtual_keyboard_process(void);
void virtual_keyboard_render(void);
boolean virtual_keyboard_last_exit_saved_text(void);

/* ---------- globals */

/* ---------- public code */

#endif // __VIRTUAL_KEYBOARD_H
