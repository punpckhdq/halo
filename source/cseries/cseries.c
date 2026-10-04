/*
CSERIES.C
*/

/* ---------- headers */

#include "cseries.h"

#undef memcmp
#undef memmove
#undef memset
#undef strcat
#undef strcmp
#undef strncat
#undef strncmp
#undef strncpy
#undef strtok
#undef strlen
#undef strcpy
#undef memcpy

/* ---------- constants */

enum
{
	MAXIMUM_MEMCPY_MEMMOVE_SIZE = 0x10000000,
	MAXIMUM_MEMSET_SIZE = 0x10000000,
	MAXIMUM_MEMCMP_SIZE = 0x10000000,
	MAXIMUM_STRING_SIZE = 0x2000,
};

/* ---------- macros */

#define cseries_match_assert(file, line, expr) if (!(expr)) { stack_walk(FALSE); error(_error_silent, "EXCEPTION %s in %s,#%d: %s", "halt", MATCH_FILE(file), MATCH_LINE(line), STRINGIFY(expr)); system_exit(-1); }
#define cseries_assert(expr) cseries_match_assert(__FILE__, __LINE__, expr)

/* ---------- structures */

/* ---------- prototypes */

extern void stack_walk(boolean);

/* ---------- globals */

char temporary[256];

static const real_argb_color global_real_argb_color_table[17] =
{
	{ 1.f, 1.f,		1.f,	1.f  },
	{ 1.f, .5f,		.5f,	.5f  },
	{ 1.f, .0f,		.0f,	.0f  },
	{ 1.f, 1.f,		.0f,	.0f  },
	{ 1.f, .0f,		1.f,	.0f  },
	{ 1.f, .0f,		.0f,	1.f  },
	{ 1.f, .0f,		1.f,	1.f  },
	{ 1.f, 1.f,		1.f,	.0f  },
	{ 1.f, 1.f,		.0f,	1.f  },
	{ 1.f, 1.f,		.41f,	.7f  },
	{ 1.f, .39f,	.58f,	.93f },
	{ 1.f, 1.f,		.5f,	.0f },
	{ 1.f, .44f,	.05f,	.43f },
	{ 1.f, .5f,		1.f,	.83f },
	{ 1.f, .0f,		.39f,	.0f },
	{ 1.f, 1.f,		.63f,	.48f },
	{ 1.f, .81f,	.13f,	.56f }
};

const real_argb_color *global_real_argb_white = &global_real_argb_color_table[0];
const real_argb_color *global_real_argb_grey = &global_real_argb_color_table[1];
const real_argb_color *global_real_argb_black = &global_real_argb_color_table[2];
const real_argb_color *global_real_argb_red = &global_real_argb_color_table[3];
const real_argb_color *global_real_argb_green = &global_real_argb_color_table[4];
const real_argb_color *global_real_argb_blue = &global_real_argb_color_table[5];
const real_argb_color *global_real_argb_cyan = &global_real_argb_color_table[6];
const real_argb_color *global_real_argb_yellow = &global_real_argb_color_table[7];
const real_argb_color *global_real_argb_magenta = &global_real_argb_color_table[8];
const real_argb_color *global_real_argb_pink = &global_real_argb_color_table[9];
const real_argb_color *global_real_argb_lightblue = &global_real_argb_color_table[10];
const real_argb_color *global_real_argb_orange = &global_real_argb_color_table[11];
const real_argb_color *global_real_argb_purple = &global_real_argb_color_table[12];
const real_argb_color *global_real_argb_aqua = &global_real_argb_color_table[13];
const real_argb_color *global_real_argb_darkgreen = &global_real_argb_color_table[14];
const real_argb_color *global_real_argb_salmon = &global_real_argb_color_table[15];
const real_argb_color *global_real_argb_violet = &global_real_argb_color_table[16];

const real_rgb_color *global_real_rgb_white = &global_real_argb_color_table[0].rgb;
const real_rgb_color *global_real_rgb_grey = &global_real_argb_color_table[1].rgb;
const real_rgb_color *global_real_rgb_black = &global_real_argb_color_table[2].rgb;
const real_rgb_color *global_real_rgb_red = &global_real_argb_color_table[3].rgb;
const real_rgb_color *global_real_rgb_green = &global_real_argb_color_table[4].rgb;
const real_rgb_color *global_real_rgb_blue = &global_real_argb_color_table[5].rgb;
const real_rgb_color *global_real_rgb_cyan = &global_real_argb_color_table[6].rgb;
const real_rgb_color *global_real_rgb_yellow = &global_real_argb_color_table[7].rgb;
const real_rgb_color *global_real_rgb_magenta = &global_real_argb_color_table[8].rgb;
const real_rgb_color *global_real_rgb_pink = &global_real_argb_color_table[9].rgb;
const real_rgb_color *global_real_rgb_lightblue = &global_real_argb_color_table[10].rgb;
const real_rgb_color *global_real_rgb_orange = &global_real_argb_color_table[11].rgb;
const real_rgb_color *global_real_rgb_purple = &global_real_argb_color_table[12].rgb;
const real_rgb_color *global_real_rgb_aqua = &global_real_argb_color_table[13].rgb;
const real_rgb_color *global_real_rgb_darkgreen = &global_real_argb_color_table[14].rgb;
const real_rgb_color *global_real_rgb_salmon = &global_real_argb_color_table[15].rgb;
const real_rgb_color *global_real_rgb_violet = &global_real_argb_color_table[16].rgb;

/* ---------- public code */

void cseries_initialize(
	void)
{
	debug_memory_manager_initialize();
	profile_initialize();
	profile_global_enable = FALSE;

	return;
};

void cseries_dispose(
	void)
{
	debug_dump_memory();

	return;
};

tag string_to_tag(
	const char *s)
{
	tag t = *(tag*)s;
	
	return SWAP4(t);
}

char *tag_to_string(
	tag t,
	char *s)
{
	*(unsigned long *)s = SWAP4(t);
	s[4] = '\0';

	return s;
}

long strnlen(
	char const *s,
	long size)
{
	long length = 0;

	while (length < size && *s++ != '\0')
	{
		++length;
	}

	return length;
}

char *strnupr(
	char *string,
	long n)
{
	unsigned char *p;
	
	for (p = (unsigned char *)string; *p && n-->0; p++)
	{
		*p = toupper(*p);
	}
	
	return string;
}

char *strnlwr(
	char *string,
	long n)
{
	unsigned char *p;
	
	for (p = (unsigned char *)string; *p && n-->0; p++)
	{
		*p = tolower(*p);
	}
	
	return string;
}

char *strupr(
	char *string)
{
	unsigned char *p;
	
	for (p = (unsigned char *)string; *p; p++)
	{
		*p = toupper(*p);
	}
	
	return string;
}

char *strlwr(
	char *string)
{
	unsigned char *p;
	
	for (p = (unsigned char *)string; *p; p++)
	{
		*p = tolower(*p);
	}
	
	return string;
}

char *csprintf(
	char *buffer,
	char *format,
	...)
{
	va_list arglist;
	
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	
	return buffer;
}

void display_assert(
	char *information,
	char *file,
	long line,
	boolean fatal)
{
	if (fatal)
	{
		stack_walk(FALSE);
	}
	
	error(_error_silent, "EXCEPTION %s in %s,#%d: %s", fatal ? "halt" : "warn", file, line, information ? information : "<no reason given>");
}

long csmemcmp(
	const void *p1,
	const void *p2,
	unsigned long size)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 255, p1 && p2);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 256, size>=0 && size<=MAXIMUM_MEMCMP_SIZE);

	return memcmp(p1, p2, size);
}

void *csmemmove(
	void *destination,
	const void *source,
	unsigned long size)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 267, destination && source);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 268, size>=0 && size<=MAXIMUM_MEMCPY_MEMMOVE_SIZE);

	return memmove(destination, source, size);
}

void *csmemset(
	void *buffer,
	long c,
	unsigned long size)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 279, buffer);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 280, size>=0 && size<=MAXIMUM_MEMSET_SIZE);

	return memset(buffer, c, size);
}

char *csstrcat(
	char *s1,
	const char *s2)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 290, s1 && s2);

	return strcat(s1, s2);
}

long csstrcmp(
	const char *s1,
	const char *s2)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 300, s1 && s2);

	return strcmp(s1, s2);
}

char *csstrncat(
	char *s1,
	const char *s2,
	unsigned long size)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 311, s1 && s2);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 312, size>=0 && size<MAXIMUM_STRING_SIZE);

	return strncat(s1, s2, size);
}

long csstrncmp(
	const char *s1,
	const char *s2,
	unsigned long size)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 323, s1 && s2);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 324, size>=0 && size<MAXIMUM_STRING_SIZE);

	return strncmp(s1, s2, size);
}

char *csstrncpy(
	char *s1,
	const char *s2,
	unsigned long size)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 335, s1 && s2);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 336, size>=0 && size<MAXIMUM_STRING_SIZE);

	return strncpy(s1, s2, size);
}

char *csstrtok(
	char *s1,
	const char *s2)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 346, s2);

	return strtok(s1, s2);
}

unsigned long csstrlen(
	const char *s1)
{
	long size;
	
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 357, s1);
	size = strlen(s1);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 359, size>=0 && size<MAXIMUM_STRING_SIZE);
	
	return size;
}

char *csstrcpy(
	char *destination,
	const char *source)
{
	long source_size = strlen(source);
	long destination_size = strlen(destination);
	
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 371, source_size>=0 && source_size<MAXIMUM_STRING_SIZE);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 372, source+source_size<destination || destination+source_size<source);
	
	return strcpy(destination, source);
}

void *csmemcpy(
	void *destination,
	const void *source,
	unsigned long size)
{
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 383, destination && source);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 384, size>=0 && size<MAXIMUM_MEMCPY_MEMMOVE_SIZE);
	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 385, (byte *)source+size<=(byte *)destination || (byte *)destination+size<=(byte *)source);
	
	return memcpy(destination, source, size);
}


long csstrcasecmp(
	char const *s1,
	char const *s2)
{
	size_t character_index;
	long result;

	cseries_match_assert("c:\\halo\\SOURCE\\cseries\\cseries.c", 397, s1 && s2);

	for (character_index = 0; ; ++character_index)
	{
		long a = towlower(s1[character_index]);
		long b = towlower(s2[character_index]);

		if (a == 0)
		{
			result = b != 0 ? -1 : 0;
			break;
		}

		if (b == 0)
		{
			result = a != 0;
			break;
		}

		if (a != b)
		{
			result = a <= b ? -1 : 1;
			break;
		}
	}
	
	return result;
}

char *stristr(
	char const *haystack,
	char const *needle)
{
	char c;
	char sc;
	unsigned long length;

	if ((c = *needle++) != 0)
	{
		length = csstrlen(needle);
		do
		{
			do
			{
				if ((sc = *haystack++) == 0)
				{
					return NULL;
				}
			}
			while (sc != c);
		}
		while (_strnicmp(haystack, needle, length) != 0);
		haystack--;
	}

	return (char *)haystack;
}

unsigned long string_hash(
	const char *string)
{
	unsigned long hash;
	
	crc_new(&hash);
	crc_checksum_buffer(&hash, string, csstrlen(string));
	
	return hash;
}

/* ---------- private code */
