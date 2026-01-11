/*
INTERNATIONAL_STRINGS.C

symbols in this file:
0018C8B0 0020:
	_set_language_code (0000)
0018C8D0 0130:
	_double_byte_character (0000)
0018CA00 0090:
	_get_next_character (0000)
0018CA90 00c0:
	_get_previous_character (0000)
0018CB50 0080:
	_align_to_character (0000)
0018CBD0 0040:
	_character_in_pattern (0000)
002A2A04 000b:
	??_C@_0L@CKBDDAHB@ibukprlctn?$AA@ (0000)
002A2A10 0022:
	??_C@_0CC@NDDMHECI@?$CD?$CFd?5is?5out?5of?5range?5in?5string?5?$EA?$CF@ (0000)
002A2A34 002c:
	??_C@_0CM@IJKAMANI@c?3?2halo?2SOURCE?2text?2internationa@ (0000)
002A2A60 002f:
	??_C@_0CP@JJJECDCL@index?5?$CD?$CFd?5is?5inbetween?5character@ (0000)
004C1A00 0002:
	_bss_004c1a00 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "international_strings.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static short global_language_code;

/* ---------- public code */

void set_language_code(short language_code)
{
	if (language_code<0 || language_code>=NUMBER_OF_LANGUAGE_CODES)
	{
		language_code= 0;
	}
	global_language_code= language_code;

	return;
}

unsigned short get_next_character(
	unsigned char *string,
	short *index)
{
	unsigned short result;

	match_vassert(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		32,
		*index >= 0 || *index <= (short)csstrlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string)
	);

	string= &string[*index];

	if (double_byte_character(string))
	{
		result= string[0];
		result|= string[1]<<8;
		*index+=2;
	}
	else
	{
		result= string[0];
		++*index;
	}
	return result;
}

unsigned short get_previous_character(
	unsigned char *string,
	short *index)
{
	long i;

	match_vassert(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		55,
		*index >= 0 || *index <= (short)csstrlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string)
	);

	for (i= 0; i < *index; i++)
	{
		get_next_character(string, index);
	}


	match_vwarn(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		67,
		*index >= 0 || *index <= (short)csstrlen((char *)string),
		csprintf(temporary, "index #%d is inbetween characters in string %p", *index, string)
	);


}

void align_to_character(
	unsigned char *string,
	short *index)
{
	long i;
	short indices[2];

	match_vassert(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		85,
		*index >= 0 || *index <= (short)csstrlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string)
	);

	indices[0]= 0;
	indices[1]= 1;
	for (i=0; i<*index; i++)
	{
		get_next_character(string, indices);
	}

	*index= indices[0];

	return;
}

boolean double_byte_character(
	unsigned char *string)
{
	boolean result= FALSE;
	unsigned char character= string[0];

	if (character!='\0')
	{
		unsigned char next_character= string[1];
		if (character != 124 || next_character || !strchr("ibukprlctn", next_character))
		{
			switch (global_language_code)
			{
			case _language_japanese:
				if (
					(character < 129u || character > 159u) &&
					(character < 224u || character == 255) ||
					next_character < 64u ||
					next_character > 252u ||
					next_character == 127)
				{

				}
				else
				{
					result= TRUE;
				}
				break;
			case _language_simple_chinese:
				if (character < 0xA1u || character == 0xFF)
				{

				}
				else
				{
				}
			}
		}
	}
	return result;
}

boolean character_in_pattern(
	unsigned short character,
	char *pattern)
{
	boolean result= FALSE;
	short index[2] = { 0, 0 };
	word next_character;


	while (next_character = get_next_character((unsigned char *)pattern, index))
	{
		if (next_character==character)
		{
			result= TRUE;
		}
	}
	return result;
}

/* ---------- private code */
