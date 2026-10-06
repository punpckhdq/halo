/*
CACHE_FILES.H

header included in hcex build.
*/

#ifndef __CACHE_FILES_H
#define __CACHE_FILES_H
#pragma once

/* ---------- headers */


/* ---------- constants */

#define CACHE_FILE_HEADER_SIGNATURE 'head' /* fake name */
#define CACHE_FILE_FOOTER_SIGNATURE 'foot' /* fake name */
#define CACHE_FILE_TAGS_HEADER_SIGNATURE 'tags' /* fake name */
#define CACHE_FILE_STRUCTURE_BSP_HEADER_SIGNATURE 'sbsp'

enum
{
	CACHE_FILE_VERSION = 5, /* fake name */
	MAXIMUM_CACHE_FILE_SIZE = 0x11600000, /* fake name */
};

enum
{
	_cached_map_file_in_progress = 0,
	_cached_map_file_success,
	_cached_map_file_failed,
	NUMBER_OF_CACHED_MAP_FILE_PRECACHE_STATES,
};

/* ---------- macros */

/* ---------- structures */

struct cache_file_header
{
	long header_signature;
	long version;
	long size;
	long compressed_file_padding;
	long tags_offset;
	long tags_size;
	long index_buffer_count;
	long index_buffers_offset;
	char name[TAG_STRING_LENGTH+1];
	char build_number[32];
	short scenario_type;
	word pad;
	unsigned long checksum;
	unsigned long unused2[485];
	long footer_signature;
};

struct cache_file_tag_instance
{
	unsigned long group_tag;
	unsigned long parent_group_tags[2];
	long tag_index;
	char *name;
	void *base_address;
	unsigned long unused[2];
};

struct cache_file_tags_header
{
	struct cache_file_tag_instance *tag_instances;
	long scenario_tag_index;
	unsigned long tags_checksum;
	long tag_count;
	long vertex_buffer_count;
	IDirect3DVertexBuffer8 *vertex_buffers;
	long index_buffer_count;
	IDirect3DIndexBuffer8 *index_buffers;
	unsigned long signature;
};

struct cache_file_structure_bsp_header
{
	struct structure_bsp *structure_bsp;
	long vertex_buffer_count; /* fake name */
	IDirect3DVertexBuffer8 *vertex_buffers; /* fake name */
	long lightmap_vertex_buffer_count; /* fake name */
	IDirect3DVertexBuffer8 *lightmap_vertex_buffers; /* fake name */
	unsigned long signature;
};

/* ---------- prototypes/CACHE_FILES.C */

char *cache_files_map_directory(void);
long scenario_tags_load(char const *name);
void scenario_tags_unload(void);
boolean scenario_structure_bsp_load(struct scenario_structure_bsp_reference *reference);
void scenario_structure_bsp_unload(struct scenario_structure_bsp_reference *reference);
unsigned long tag_get_group_tag(long tag_index);
unsigned long cache_files_get_checksum(void);
void cache_files_enable_writes(void);
void cache_files_disable_writes(void);
boolean cache_file_header_verify(struct cache_file_header *header, char const *name, boolean fatal);
boolean cache_files_give_time_to_precache(char const *map_name);

/* ---------- prototypes/CACHE_FILES_WINDOWS.C */

void cache_files_initialize(void);
void cache_files_dispose(void);
boolean cache_file_open(char const *scenario_name, struct cache_file_header *header);
void cache_file_close(void);
short cache_file_read(long tag_index, long offset, long size, void *buffer, boolean *completion_flag_reference, boolean blocking);
void cache_file_promote_read(short request_index);
void cache_file_block_until_not_busy(void);
void tags_header_register_vertex_and_index_buffers(struct cache_file_tags_header *tags_header);
void tags_header_deregister_vertex_and_index_buffers(struct cache_file_tags_header *tags_header);
void structure_bsp_header_register_vertex_buffers(struct cache_file_structure_bsp_header *structure_bsp_header);
void structure_bsp_header_deregister_vertex_buffers(struct cache_file_structure_bsp_header *structure_bsp_header);
void cache_files_precache_set_priority(boolean blocking);
boolean cache_files_precache_in_progress(void);
boolean cache_files_precache_is_copying_map(char const *name);
boolean cache_files_precache_map_loaded(char const *name);
boolean cache_files_precache_map_begin(char const *name, boolean blocking);
short cache_files_precache_map_status(real *progress);
void cache_files_precache_map_queue_end(void);
void cache_files_precache_map_end(void);

/* ---------- globals */

extern struct cache_file_tag_instance *global_tag_instances;

/* ---------- public code */

#endif // __CACHE_FILES_H
