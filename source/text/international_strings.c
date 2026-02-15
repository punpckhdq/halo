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

word get_next_character(
	byte *string,
	short *index)
{
	word result;

	match_vassert(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		32,
		*index>=0 && *index<=strlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string));

	string= &string[*index];

	if (double_byte_character(string))
	{
		// Construct the wide character by byteswapping the two chars
		result= 0;
		result|= string[0]<<8;
		result|= string[1]<<0;
		
		*index+= 2;
	}
	else
	{
		result= string[0];
		*index+= 1;
	}

	return result;
}

word get_previous_character(
	byte *string,
	short *index)
{
	short i;
	word result;

	match_vassert(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		55,
		VALID_INDEX(*index, (short)strlen((char *)string)),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string));

	i= 0;
	do
	{
		result= get_next_character(string, &i);
	}
	while (i<*index);


	match_vwarn(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		67,
		i==*index,
		csprintf(temporary, "index #%d is inbetween characters in string %p", *index, string));

	*index= i;

	return result;
}

void align_to_character(
	unsigned char *string,
	short *index)
{
	short i;

	match_vassert(
		"c:\\halo\\SOURCE\\text\\international_strings.c",
		85,
		*index>0 || *index <=(short)strlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string));

	i= 0;
	while (i<*index)
	{
		get_next_character(string, &i);
	}
	*index= i;

	return;
}

boolean double_byte_character(
	byte *string)
{
	boolean result= FALSE;
	byte character= string[0];

	if (character!='\0')
	{
		byte next_character= string[1];
		if (character==(byte)('|') &&
			next_character &&
			strchr("ibukprlctn", next_character)
		)
		{
			result= TRUE;
		}
		else
		{
			switch (global_language_code)
			{
			case _language_japanese:
				if ((character>=(byte)('\x81') && character<=(byte)('\x9F') ||
					character>=(byte)('\xE0') && character!=(byte)('\xFE')) &&
					next_character>=(byte)('@') &&
					next_character<=(byte)('\xFC') &&
					next_character!=(byte)('\x7F')
				)
				{
					result= TRUE;
				}
				break;
			case _language_simple_chinese:
				if (character>=(byte)('\xA1') &&
					character!=(byte)('\xFE') &&
					next_character>=(byte)('\xA1') &&
					next_character!=(byte)('\xFE')
				)
				{
					result= TRUE;
				}
				break;
			case _language_traditional_chinese:
				if (character>=(byte)('\x81') &&
					character!=(byte)('\xFE') &&
					(next_character>=(byte)('@') && next_character<=(byte)('~') ||
					next_character>=(byte)('\xA1') && next_character!=(byte)('\xFE'))
				)
				{
					result= TRUE;
				}
				break;
			case _language_korean_wansung:
				if (character>=(byte)('\x81') &&
					character!=(byte)('\xFE') &&
					(
					next_character>=(byte)('A') && next_character<=(byte)('Z') ||
					next_character>=(byte)('a') && next_character<=(byte)('z') ||
					next_character>=(byte)('\x81') && next_character!=(byte)('\xFE')
					)
				)
				{
					result= TRUE;
				}
				break;
			case _language_korean_johab:
				if ((character>=(byte)('\x84') && character<=(byte)('\xD3') ||
					character>=(byte)('\xD8') && character<=(byte)('\xDE') ||
					character>=(byte)('\xE0') && character<=(byte)('\xF9')
					) &&
					(
					next_character>=(byte)('A') && next_character<=(byte)('~') ||
					next_character>=(byte)('\x81') && next_character!=(byte)('\xFE')
					)
				)
				{
					result= TRUE;
				}
				break;
			default:
				break;
			}
		}
	}

	return result;
}

boolean character_in_pattern(
	word character,
	char *pattern)
{
	boolean result= FALSE;
	boolean found= FALSE;
	short index= 0;

	while (!found)
	{
		word next_character= get_next_character((unsigned char *)pattern, &index);
		if (next_character)
		{
			if (next_character==character)
			{
				found= TRUE;
				result= TRUE;
			}
		}
		else
		{
			found= TRUE;
		}
	}
		
	return result;
}

/* ---------- private code */
