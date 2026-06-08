/*
TAG_FILES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "tag_files.h"

/* ---------- public code */

const char *tag_name_strip_path(
	char const *name)
{
	const char *stripped;

	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_files.c", 1374, name);

	stripped = strrchr(name, '\\');

	if (stripped!=NULL)
	{
		return stripped + 1;
	}
	else
	{
		return name;
	}
}
