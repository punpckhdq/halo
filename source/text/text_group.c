/*
TEXT_GROUP.C

*/

/* ---------- headers */

#include "cseries.h"
#include "text_group.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

char *string_list_get_string(
	long tag_index,
	short string_index)
{
	char *string = "<missing string>";

	if (tag_index != NONE)
	{
		struct string_list_group_header *string_list = tag_get(STRING_LISTS_GROUP_TAG, tag_index);

		if (string_index >= 0 && string_index < string_list->string_references.count)
		{
			struct string_list_string_reference *reference = TAG_BLOCK_GET_ELEMENT(&string_list->string_references, string_index, struct string_list_string_reference);

			if (reference->string.size > 0)
			{
				string = reference->string.address;
				string[reference->string.size - 1] = '\0';
			}
		}
	}

	return string;
}

wchar_t *unicode_string_list_get_string(
	long tag_index,
	short string_index)
{
	wchar_t *string = L"<missing string>";

	if (tag_index != NONE)
	{
		struct unicode_string_list_group_header *string_list = tag_get(UNICODE_STRING_LISTS_GROUP_TAG, tag_index);

		if (string_index >= 0 && string_index < string_list->string_references.count)
		{
			struct unicode_string_list_string_reference *reference = TAG_BLOCK_GET_ELEMENT(&string_list->string_references, string_index, struct unicode_string_list_string_reference);

			if (reference->string.size > 0)
			{
				string = reference->string.address;
				string[reference->string.size / sizeof(wchar_t) - 1] = L'\0';
			}
		}
	}

	return string;
}

/* ---------- private code */
