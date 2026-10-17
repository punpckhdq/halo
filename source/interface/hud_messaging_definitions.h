/*
HUD_MESSAGING_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __HUD_MESSAGING_DEFINITIONS_H
#define __HUD_MESSAGING_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	HUD_STATE_MESSAGES_TAG = 'hmt ', /* fake name */
	// TODO: identify version
};

enum
{
	MAX_TEXT_MESSAGES = 4,
	MAX_MESSAGE_LENGTH = 63,
	MAX_STATE_TEXT_PANELS = 8,
	MAX_STATE_MESSAGES = 1024,
	MAXIMUM_MESSAGE_STRING_DATA_SIZE = 65536,
};

enum
{
	_hud_message_type_text = 0,
	_hud_message_type_icon,
	NUMBER_OF_HUD_MESSAGE_TYPES,
};

enum
{
	HUD_CUSTOM_MESSAGE_LENGTH = 255,
};

enum
{
	_hud_icon_specific_button_start = 0,
	_hud_icon_specific_button_end = 17,
	_hud_icon_remapped_button_start = 18,
	_hud_icon_remapped_button_end = 31,
	_hud_icon_misc_start = 32,
	_hud_icon_misc_end = 39,
	NUMBER_OF_HUD_ICON_TYPES = 40,
	NUMBER_OF_HUD_BUTTON_ICONS = 32,
	NUMBER_OF_HUD_CUSTOM_ICONS = 8,
};

/* ---------- macros */

#define hud_state_messages_get(index) ((struct hud_state_messages *)tag_get(HUD_STATE_MESSAGES_TAG, (index))) /* fake name */

/* ---------- structures */

struct hud_state_message_definition
{
	char name[32];
	word text_start_index;
	word element_start_index;
	byte element_count;
	byte pad[3];
	long unused[6];
};

struct hud_state_message_element
{
	byte type;
	byte data;
};

struct hud_state_messages
{
	struct tag_data text_data;
	struct tag_block elements;		// hud_state_message_element
	struct tag_block messages;		// hud_state_message_definition
	long unused[21];
};

/* ---------- globals */

/* ---------- public code */

#endif // __HUD_MESSAGING_DEFINITIONS_H
