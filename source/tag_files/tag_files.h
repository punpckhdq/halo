/*
TAG_FILES.H

header included in hcex build.
*/

#ifndef __TAG_FILES_H
#define __TAG_FILES_H
#pragma once

/* ---------- constants */

enum
{
	TAG_STRING_LENGTH= 31,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/TAG_FILES.C */

void tag_files_open(void);
void tag_files_close(void);
const char *tag_name_strip_path(char const *name);

/* ---------- globals */

/* ---------- public code */

#endif // __TAG_FILES_H
