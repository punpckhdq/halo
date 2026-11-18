/*
TAG_GROUPS.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- public code */

// NOTE: 3000 lines of tool-specific code lives here

#ifdef DEBUG
long verify_tag_reference(
	struct tag_reference const *reference)
{
	long index = NONE;

	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3055, reference);
	index = tag_loaded(reference->group_tag, reference->name);

	match_vassert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3061, reference->index==index,
		csprintf(
			temporary,
			"tag reference \"%s\" and actual index do not match: is %08lX but should be %08lX",
			reference->name,
			reference->index,
			index));

	return index;
}
#endif

void *tag_data_get_pointer(
	struct tag_data const *data,
	long offset,
	long size)
{
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3073, size>=0);
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3074, offset>=0 && offset+size<=data->size);

	return (void *)((byte *)data->address + offset);
}

void *tag_block_get_element_with_size(
	struct tag_block const *block,
	long index,
	long element_size)
{
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3084, block);
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3085, block->count>=0);
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3086, !block->definition || block->definition->element_size==element_size);
	match_vassert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3089, index>=0 && index<block->count,
		csprintf(
			temporary,
			"#%d is not a valid %s index in [#0,#%d)",
			index,
			block->definition ? block->definition->name : "<unknown>",
			block->count));
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3090, block->address);

	return (void *)((byte *)block->address + index*element_size);
}
