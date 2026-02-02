/*
INTERNATIONAL_STRINGS.H

header included in hcex build.
*/

#ifndef __INTERNATIONAL_STRINGS_H
#define __INTERNATIONAL_STRINGS_H
#pragma once

/* ---------- constants */

/* right placement? language.h doesn't exist */
enum
{
	_german= 0,
	_french,
	_spanish,
	_italian,
	_english,
	_japanese,
	_unknown,
	NUMBER_OF_SUPPORTED_LANGUAGES,
};

enum
{
	_language_roman= 0,
	_language_japanese,
	_language_simple_chinese,
	_language_traditional_chinese,
	_language_korean_wansung,
	_language_korean_johab,
	NUMBER_OF_LANGUAGE_CODES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/INTERNATIONAL_STRINGS.C */

void set_language_code(short language_code);
word get_next_character(byte *string, short *index);
word get_previous_character(byte *string, short *index);
void align_to_character(unsigned char *string, short *index);
boolean double_byte_character(byte *string);
boolean character_in_pattern(word character, char *pattern);

/* ---------- globals */

/* ---------- public code */

#endif // __INTERNATIONAL_STRINGS_H
