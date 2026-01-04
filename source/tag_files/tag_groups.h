/*
TAG_GROUPS.H

header included in hcex build.
*/

#ifndef __TAG_GROUPS_H
#define __TAG_GROUPS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct tag_block
{
	long count;
	void *address;
	struct tag_block_definition *definition;
};

struct tag_reference
{
	unsigned long group_tag;
	char *name;
	long name_length;
	long index;
};

struct tag_data
{
	long size;
	unsigned long pad;
	long file_offset;
	void *address;
	struct tag_data_definition *definition;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __TAG_GROUPS_H
