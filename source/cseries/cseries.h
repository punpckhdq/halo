/*
CSERIES.H
*/

#ifndef __CSERIES_H
#define __CSERIES_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

/* ---------- constants */

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

enum
{
	UNSIGNED_LONG_MAX= 4294967295,
	LONG_MAX= 2147483647L,
	LONG_MIN= (-2147483648L),
	LONG_BITS= 32,
	LONG_BITS_BITS= 5,

	UNSIGNED_SHORT_MAX= 65535,
	SHORT_MAX= 32767,
	SHORT_MIN= (-32768),
	SHORT_BITS= 16,
	SHORT_BITS_BITS= 4,

	UNSIGNED_CHAR_MAX= 255,
	CHAR_MAX= 127,
	CHAR_MIN= (-128),
	CHAR_BITS= 8,
	CHAR_BITS_BITS= 3
};

#define NONE -1

/* ---------- macros */

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

/* on non-matching builds give actual source line info for assertions */
#ifdef NON_MATCHING
	#define MATCH_FILE(file) __FILE__
	#define MATCH_LINE(line) __LINE__
#else
	#define MATCH_FILE(file) file
	#define MATCH_LINE(line) line
#endif

#define match_halt(file, line) do { display_assert(NULL, MATCH_FILE(file), MATCH_LINE(line), TRUE); halt_and_catch_fire(); } while (FALSE);
#define match_vhalt(file, line, string) do { display_assert(string, MATCH_FILE(file), MATCH_LINE(line), TRUE); halt_and_catch_fire(); } while (FALSE);
#define match_assert(file, line, expr) if (!(expr)) { display_assert(#expr, MATCH_FILE(file), MATCH_LINE(line), TRUE); system_exit(-1); }
#define match_vassert(file, line, expr, string) if (!(expr)) { display_assert(string, MATCH_FILE(file), MATCH_LINE(line), TRUE); system_exit(-1); }
#define match_warn(file, line, expr) if (!(expr)) { display_assert(#expr, MATCH_FILE(file), MATCH_LINE(line), FALSE); }
#define match_vwarn(file, line, expr, string) if (!(expr)) { display_assert(string, MATCH_FILE(file), MATCH_LINE(line), FALSE); }

#define halt() match_halt(__FILE__, __LINE__)
#define vhalt(string) match_vhalt(__FILE__, __LINE__, string)
#define assert(expr) match_assert(__FILE__, __LINE__, expr)
#define vassert(expr, string) match_vassert(__FILE__, __LINE__, expr, string)
#define warn(expr) match_warn(__FILE__, __LINE__, expr)
#define vwarn(expr, string) match_vwarn(__FILE__, __LINE__, expr, string)

#define ABS(x) ((x>=0) ? (x) : -(x))

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define FLOOR(n,floor) ((n)<(floor)?(floor):(n))
#define CEILING(n,ceiling) ((n)>(ceiling)?(ceiling):(n))
#define PIN(n,floor,ceiling) ((n)<(floor) ? (floor) : CEILING(n,ceiling))

#define FLAG(b) (1<<(b))
#define TEST_FLAG(flags, bit) ((flags)&(unsigned)FLAG(bit))
#define SET_FLAG(f, b, v) ((v) ? ((f)|=(unsigned)FLAG(b)) : ((f)&=(unsigned)~FLAG(b)))

#define BIT_VECTOR_SIZE_IN_LONGS(bit_count) (((bit_count) + (LONG_BITS - 1)) >> LONG_BITS_BITS)
#define BIT_VECTOR_SIZE_IN_BYTES(bit_count) (sizeof(long) * BIT_VECTOR_SIZE_IN_LONGS(bit_count))
#define BIT_VECTOR_TEST_FLAG(bit_vector, bit) (TEST_FLAG(bit_vector[(bit) >> LONG_BITS_BITS], ((bit) & (LONG_BITS - 1))))
#define BIT_VECTOR_SET_FLAG(bit_vector, bit, enable) (SET_FLAG(bit_vector[(bit) >> LONG_BITS_BITS], ((bit) & (LONG_BITS - 1)), enable))

#define VALID_FLAGS(flags, bits) (!((flags)>>bits))

#define NUMBEROF(array) (sizeof(array) / sizeof(array[0]))

/* ---------- fixed math */

#define SHORT_FIXED_TO_LONG(f) ((f)>>CHAR_BITS)

/* ---------- types */

typedef unsigned char byte;
typedef unsigned short word;

typedef byte boolean;

typedef unsigned long tag;

/* ---------- prototypes/CSERIES.C */

void cseries_initialize(void);
void cseries_dispose(void);
tag string_to_tag(const char *s);
char *tag_to_string(tag t, char *s);
long strnlen(const char *string, long n);
char *strnupr(char *string, int n);
char *strnlwr(char *string, int n);
char *strupr(char *string);
char *strlwr(char *string);
char *csprintf(char *buffer, char *format, ...);
void display_assert(char *information, char *file, long line, boolean fatal);
long csmemcmp(const void *p1, const void *p2, unsigned long size);
void *csmemmove(void *destination, const void *source, unsigned long size);
void *csmemset(void *buffer, long c, unsigned long size);
char *csstrcat(char *s1, const char *s2);
long csstrcmp(const char *s1, const char *s2);
char *csstrncat(char *s1, const char *s2, unsigned long size);
long csstrncmp(const char *s1, const char *s2, unsigned long size);
char *csstrncpy(char *s1, const char *s2, unsigned long size);
char *csstrtok(char *s1, const char *s2);
unsigned long csstrlen(const char *s1);
char *csstrcpy(char *destination, const char *source);
void *csmemcpy(void *destination, const void *source, unsigned long size);
long csstrcasecmp(const char *s1, const char *s2);
char *stristr(char const *haystack, char const *needle);
unsigned long string_hash(char const *string);

/* ---------- prototypes/CSERIES_WINDOWS.C */

void system_exit(long code);

/* ---------- prototypes/MAIN.C */

void halt_and_catch_fire(void);

/* ---------- prototypes/DEBUG_MEMORY.C */

void debug_memory_manager_initialize(void);
/*void debug_memory_manager_dispose(void);*/
void check_memory_status(struct memory_status *, const char *);
void debug_check_memory(const char *, long);
void debug_dump_memory_for_file(const char *);
void debug_dump_memory_by_file(void);
void *debug_malloc(unsigned int, boolean, const char *, long);
void debug_free(void *, const char *, long);
void *debug_realloc(void *, unsigned int, boolean, const char *, long);
void debug_dump_memory(void);

/* ---------- prototypes/STACK_WALK_WINDOWS.C */

void stack_walk_disregard_symbol_names(boolean disregard);

/* ---------- macros */

#ifndef BUILDING_CSERIES
#define memcmp csmemcmp
#define memmove csmemmove
#define memset csmemset
#define strcat csstrcat
#define strcmp csstrcmp
#define strncat csstrncat
#define strncmp csstrncmp
#define strncpy csstrncpy
#define strtok csstrtok
#define strlen csstrlen
#define strcpy csstrcpy
#define memcpy csmemcpy

#define match_malloc(file, line, size) debug_malloc(size, FALSE, MATCH_FILE(file), MATCH_LINE(line))
#define match_free(file, line, ptr) debug_free(ptr, MATCH_FILE(file), MATCH_LINE(line))
#define match_realloc(file, line, ptr, size) debug_realloc(ptr, size, FALSE, MATCH_FILE(file), MATCH_LINE(line))

#define malloc(size) match_malloc(__FILE__, __LINE__, size)
#define free(ptr) match_free(__FILE__, __LINE__, ptr)
#define realloc(ptr, size) match_realloc(__FILE__, __LINE__, ptr, size)
#endif

/* ---------- globals */

/* comm */
char temporary[256];

extern const union real_argb_color *const global_real_argb_white;
extern const union real_argb_color *const global_real_argb_grey;
extern const union real_argb_color *const global_real_argb_black;
extern const union real_argb_color *const global_real_argb_red;
extern const union real_argb_color *const global_real_argb_green;
extern const union real_argb_color *const global_real_argb_blue;
extern const union real_argb_color *const global_real_argb_cyan;
extern const union real_argb_color *const global_real_argb_yellow;
extern const union real_argb_color *const global_real_argb_magenta;
extern const union real_argb_color *const global_real_argb_pink;
extern const union real_argb_color *const global_real_argb_lightblue;
extern const union real_argb_color *const global_real_argb_orange;
extern const union real_argb_color *const global_real_argb_purple;
extern const union real_argb_color *const global_real_argb_aqua;
extern const union real_argb_color *const global_real_argb_darkgreen;
extern const union real_argb_color *const global_real_argb_salmon;
extern const union real_argb_color *const global_real_argb_violet;

extern const union real_rgb_color *const global_real_rgb_white;
extern const union real_rgb_color *const global_real_rgb_grey;
extern const union real_rgb_color *const global_real_rgb_black;
extern const union real_rgb_color *const global_real_rgb_red;
extern const union real_rgb_color *const global_real_rgb_green;
extern const union real_rgb_color *const global_real_rgb_blue;
extern const union real_rgb_color *const global_real_rgb_cyan;
extern const union real_rgb_color *const global_real_rgb_yellow;
extern const union real_rgb_color *const global_real_rgb_magenta;
extern const union real_rgb_color *const global_real_rgb_pink;
extern const union real_rgb_color *const global_real_rgb_lightblue;
extern const union real_rgb_color *const global_real_rgb_orange;
extern const union real_rgb_color *const global_real_rgb_purple;
extern const union real_rgb_color *const global_real_rgb_aqua;
extern const union real_rgb_color *const global_real_rgb_darkgreen;
extern const union real_rgb_color *const global_real_rgb_salmon;
extern const union real_rgb_color *const global_real_rgb_violet;

#endif // __CSERIES_H
