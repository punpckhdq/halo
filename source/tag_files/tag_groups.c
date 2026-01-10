/*
TAG_GROUPS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "tag_files.h"
#include "tag_groups.h"

/* ---------- structures */

struct tag_block_definition 
{
	long count;
	char* unk4;
	long pad8;
	long element_size;
};

/* ---------- public code */

long verify_tag_reference(
	struct tag_reference const* reference) 
{
	int index;

	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3055, reference)
	index = tag_loaded(reference->group_tag, reference->name);
	match_vassert(
		"c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3061, reference->index==index, 
		csprintf(temporary, 
			"tag reference \"%s\" and actual index do not match: is %08lX but should be %08lX", 
			reference->name, 
			reference->index, 
			index));
	return index;
}

void* tag_data_get_pointer(
	struct tag_data const *data, 
	long offset, 
	long size) 
{
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3073, size>=0);
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3074, offset>=0 && offset+size<=data->size);
	return (void*)((char*)data->address + offset);
}

void *tag_block_get_element_with_size(
	struct tag_block const *block, 
	long index, 
	long element_size) 
{
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3084, block);
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3085, block->count>=0);
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3086, !block->definition || block->definition->element_size==element_size);
	match_vassert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3089, index>=0 && index<block->definition->count, 
		csprintf(temporary, 
			"#%d is not a valid %s index in [#0,#%d)", 
			index, 
			!block->definition->unk4 ? "unknown" : block->definition->unk4, block->definition->count));
	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_groups.c", 3090, block->address);
	return (void*)((char*)block->address + (index * element_size));
}
