/*
CACHE_FILES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "texture_cache.h"
#include "sound_cache.h"
#include "physical_memory_map.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "input.h"
#include "sound_manager.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "strings/resource.h"
#include "build_number.h"

/* ---------- constants */

enum
{
	TAG_CACHE_GARBAGE_FILL_BYTE = 0xCD, /* fake name */
	STRUCTURE_BSP_LOAD_SOUND_IDLE_MILLISECONDS = 33, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static struct cache_file_tag_instance *cache_file_tag_instance_get(long tag_index);

/* ---------- globals */

struct cache_file_tag_instance *global_tag_instances;

static struct
{
	boolean tags_loaded;
	struct cache_file_header header;
	struct cache_file_tags_header *tags_header;
	struct cache_file_structure_bsp_header *structure_bsp_header;
} cache_file_globals;

/* ---------- public code */

char *cache_files_map_directory(
	void)
{
	static char *alternative_directories[] =
	{
		"d:\\maps_de\\",
		"d:\\maps_fr\\",
		"d:\\maps_es\\",
		"d:\\maps_it\\",
		"d:\\maps\\",
		NULL
	};
	struct file_reference directory_reference;
	char *map_directory;

	switch (XGetLanguage())
	{
	case XC_LANGUAGE_GERMAN:
		map_directory = "d:\\maps_de\\";
		break;
	case XC_LANGUAGE_FRENCH:
		map_directory = "d:\\maps_fr\\";
		break;
	case XC_LANGUAGE_SPANISH:
		map_directory = "d:\\maps_es\\";
		break;
	case XC_LANGUAGE_ITALIAN:
		map_directory = "d:\\maps_it\\";
		break;
	default:
		map_directory = "d:\\maps\\";
		break;
	}

	if (!file_exists(file_reference_create_from_path(&directory_reference, map_directory, TRUE)))
	{
		long directory_index;

		for (directory_index = 0; alternative_directories[directory_index]; directory_index++)
		{
			if (file_exists(file_reference_create_from_path(&directory_reference, alternative_directories[directory_index], TRUE)))
			{
				map_directory = alternative_directories[directory_index];
				break;
			}
		}

		match_vassert("c:\\halo\\SOURCE\\cache\\cache_files.c", 60, alternative_directories[directory_index], "no valid map directory exists");
	}

	return map_directory;
}

long scenario_tags_load(
	char const *name)
{
	boolean completion_flag;
	char const *scenario_name = tag_name_strip_path(name);
	long scenario_index = NONE;

	texture_cache_open();
	sound_cache_open();

	if (cache_file_open(scenario_name, &cache_file_globals.header))
	{
		void *tag_cache_base_address = physical_memory_get_tag_cache_base_address();

		if (cache_file_header_verify(&cache_file_globals.header, name, TRUE))
		{
			memset(tag_cache_base_address, TAG_CACHE_GARBAGE_FILL_BYTE, TAG_CACHE_SIZE);
			cache_file_read(
				NONE,
				cache_file_globals.header.tags_offset,
				cache_file_globals.header.tags_size,
				tag_cache_base_address,
				&completion_flag,
				TRUE);

			while (!completion_flag)
			{
				SwitchToThread();
			}

			cache_file_globals.tags_header = tag_cache_base_address;
			match_vassert(
				"c:\\halo\\SOURCE\\cache\\cache_files.c",
				148,
				cache_file_globals.tags_header->signature==CACHE_FILE_TAGS_HEADER_SIGNATURE,
				csprintf(
					temporary,
					"signature is '%c%c%c%c', should be '%c%c%c%c'",
					((char *)&cache_file_globals.tags_header->signature)[3],
					((char *)&cache_file_globals.tags_header->signature)[2],
					((char *)&cache_file_globals.tags_header->signature)[1],
					((char *)&cache_file_globals.tags_header->signature)[0],
					(char)(CACHE_FILE_TAGS_HEADER_SIGNATURE>>24),
					(char)(CACHE_FILE_TAGS_HEADER_SIGNATURE>>16),
					(char)(CACHE_FILE_TAGS_HEADER_SIGNATURE>>8),
					(char)CACHE_FILE_TAGS_HEADER_SIGNATURE));

			global_tag_instances = cache_file_globals.tags_header->tag_instances;
			tags_header_register_vertex_and_index_buffers(cache_file_globals.tags_header);
			cache_file_globals.tags_loaded = TRUE;
			scenario_index = cache_file_globals.tags_header->scenario_tag_index;
		}
	}

	return scenario_index;
}

void scenario_tags_unload(
	void)
{
	sound_cache_close();
	texture_cache_close();
	cache_file_close();
	tags_header_deregister_vertex_and_index_buffers(cache_file_globals.tags_header);
	cache_file_globals.tags_loaded = FALSE;
	global_tag_instances = NULL;
}

boolean scenario_structure_bsp_load(
	struct scenario_structure_bsp_reference *reference)
{
	struct cache_file_tag_instance *tag_instance;
	byte *tag_cache_base_address = physical_memory_get_tag_cache_base_address();

	memset(
		tag_cache_base_address + cache_file_globals.header.tags_size,
		TAG_CACHE_GARBAGE_FILL_BYTE,
		TAG_CACHE_SIZE - cache_file_globals.header.tags_size);

	{
		boolean completion_flag;

		cache_file_read(
			NONE,
			reference->offset,
			reference->size,
			reference->address,
			&completion_flag,
			TRUE);

		while (!completion_flag)
		{
			SwitchToThread();

			if (system_milliseconds() - sound_render_time() > STRUCTURE_BSP_LOAD_SOUND_IDLE_MILLISECONDS)
			{
				sound_idle();
			}
		}
	}

	cache_file_globals.structure_bsp_header = reference->address;
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 224, cache_file_globals.structure_bsp_header->signature==CACHE_FILE_STRUCTURE_BSP_HEADER_SIGNATURE);
	structure_bsp_header_register_vertex_buffers(cache_file_globals.structure_bsp_header);

	tag_instance = cache_file_tag_instance_get(reference->structure_bsp.index);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 234, !tag_instance->base_address);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 235, tag_instance->group_tag==STRUCTURE_BSP_TAG);
	tag_instance->base_address = cache_file_globals.structure_bsp_header->structure_bsp;

	return TRUE;
}

void scenario_structure_bsp_unload(
	struct scenario_structure_bsp_reference *reference)
{
	struct cache_file_tag_instance *tag_instance;

	structure_bsp_header_deregister_vertex_buffers(cache_file_globals.structure_bsp_header);

	tag_instance = cache_file_tag_instance_get(reference->structure_bsp.index);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 256, tag_instance->base_address);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 257, tag_instance->group_tag==STRUCTURE_BSP_TAG);
	tag_instance->base_address = NULL;
	cache_file_globals.structure_bsp_header = NULL;
}

void tag_files_open(
	void)
{
	cache_files_initialize();
}

void tag_files_close(
	void)
{
	cache_files_dispose();
}

void *tag_get(
	long group_tag,
	long tag_index)
{
	char expected_group_string[16];
	char returned_group_string[16];
	struct cache_file_tag_instance *tag_instance = cache_file_tag_instance_get(tag_index);

	match_vassert(
		"c:\\halo\\SOURCE\\cache\\cache_files.c",
		298,
		tag_instance->group_tag==group_tag || tag_instance->parent_group_tags[0]==group_tag || tag_instance->parent_group_tags[1]==group_tag,
		csprintf(
			temporary,
			"expected tag group '%s' but got '%s' for %08x",
			tag_to_string(group_tag, expected_group_string),
			tag_to_string(tag_instance->group_tag, returned_group_string),
			tag_index));
	match_vassert(
		"c:\\halo\\SOURCE\\cache\\cache_files.c",
		302,
		tag_instance->base_address,
		csprintf(temporary, "can't get() a tag with a base address!"));

	return tag_instance->base_address;
}

char *tag_get_name(
	long tag_index)
{
	return cache_file_tag_instance_get(tag_index)->name;
}

unsigned long tag_get_group_tag(
	long tag_index)
{
	return cache_file_tag_instance_get(tag_index)->group_tag;
}

unsigned long tag_groups_checksum(
	void)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 327, cache_file_globals.tags_loaded);

	return cache_file_globals.tags_header->tags_checksum;
}

unsigned long cache_files_get_checksum(
	void)
{
	return cache_file_globals.header.checksum;
}

long tag_loaded(
	long group_tag,
	char const *name)
{
	long tag_index = NONE;

	if (cache_file_globals.tags_loaded)
	{
		short absolute_index;

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 346, global_tag_instances);

		for (absolute_index = 0; absolute_index < cache_file_globals.tags_header->tag_count; absolute_index++)
		{
			if (group_tag == global_tag_instances[absolute_index].group_tag && !_stricmp(name, global_tag_instances[absolute_index].name))
			{
				tag_index = global_tag_instances[absolute_index].tag_index;
				break;
			}
		}
	}

	return tag_index;
}

void cache_files_enable_writes(
	void)
{
	XPhysicalProtect((void *)TAG_CACHE_BASE_ADDRESS, TAG_CACHE_SIZE, PAGE_READWRITE);
}

void cache_files_disable_writes(
	void)
{
	XPhysicalProtect((void *)TAG_CACHE_BASE_ADDRESS, TAG_CACHE_SIZE, PAGE_READONLY);
	XPhysicalProtect(
		cache_file_globals.tags_header->vertex_buffers,
		cache_file_globals.tags_header->vertex_buffer_count*sizeof(*cache_file_globals.tags_header->vertex_buffers),
		PAGE_READWRITE);
	XPhysicalProtect(
		cache_file_globals.tags_header->index_buffers,
		cache_file_globals.tags_header->index_buffer_count*sizeof(*cache_file_globals.tags_header->index_buffers),
		PAGE_READWRITE);

	if (cache_file_globals.structure_bsp_header)
	{
		XPhysicalProtect(
			cache_file_globals.structure_bsp_header->vertex_buffers,
			cache_file_globals.structure_bsp_header->vertex_buffer_count*sizeof(*cache_file_globals.structure_bsp_header->vertex_buffers),
			PAGE_READWRITE);
		XPhysicalProtect(
			cache_file_globals.structure_bsp_header->lightmap_vertex_buffers,
			cache_file_globals.structure_bsp_header->lightmap_vertex_buffer_count*sizeof(*cache_file_globals.structure_bsp_header->lightmap_vertex_buffers),
			PAGE_READWRITE);
	}
}

boolean tag_block_resize(
	struct tag_block *block,
	long element_count)
{
	error(_error_silent, "tag_block_resize() is not supported with a cache file active");

	return FALSE;
}

boolean tag_data_resize(
	struct tag_data *data,
	long size)
{
	error(_error_silent, "tag_data_resize() is not supported with a cache file active");

	return FALSE;
}

long tag_block_add_element(
	struct tag_block *block)
{
	error(_error_silent, "tag_block_add_element() is not supported with a cache file active");

	return NONE;
}

void tag_block_delete_element(
	struct tag_block *block,
	long index)
{
	error(_error_silent, "tag_block_delete_element() is not supported with a cache file active");
}

long tag_load(
	unsigned long group_tag,
	char const *name,
	unsigned long flags)
{
	error(_error_silent, "tag_load() is not supported with a cache file active");

	return NONE;
}

void tag_unload(
	long index)
{
	error(_error_silent, "tag_unload() is not supported with a cache file active");
}

void tag_file_get_path(
	unsigned long group_tag,
	char const *name,
	char *path)
{
	error(_error_silent, "tag_file_get_path() is not supported with a cache file active");
	path[0] = '\0';
}

void tag_reference_set(
	struct tag_reference *reference,
	unsigned long group_tag,
	char const *name)
{
	error(_error_silent, "tag_reference_set() is not supported with a cache file active");
}

void tag_iterator_new(
	struct tag_iterator *iterator,
	unsigned long key_group_tag)
{
	iterator->iterator.absolute_index = 0;
	iterator->key_group_tag = key_group_tag;
}

long tag_iterator_next(
	struct tag_iterator *iterator)
{
	long tag_index = NONE;

	while (iterator->iterator.absolute_index < cache_file_globals.tags_header->tag_count)
	{
		struct cache_file_tag_instance *tag_instance = &global_tag_instances[iterator->iterator.absolute_index++];

		if (tag_instance &&
			(iterator->key_group_tag == NONE ||
			iterator->key_group_tag == tag_instance->group_tag ||
			iterator->key_group_tag == tag_instance->parent_group_tags[0] ||
			iterator->key_group_tag == tag_instance->parent_group_tags[1]))
		{
			tag_index = tag_instance->tag_index;
			break;
		}
	}

	return tag_index;
}

static struct cache_file_tag_instance *cache_file_tag_instance_get(
	long tag_index)
{
	struct cache_file_tag_instance *tag_instance;
	short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(tag_index);

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 518, cache_file_globals.tags_loaded);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files.c", 519, global_tag_instances);
	match_vassert(
		"c:\\halo\\SOURCE\\cache\\cache_files.c",
		522,
		absolute_index>=0 && absolute_index<cache_file_globals.tags_header->tag_count,
		csprintf(temporary, "i don't think %08x is a tag index", tag_index));

	tag_instance = &global_tag_instances[absolute_index];
	match_vassert(
		"c:\\halo\\SOURCE\\cache\\cache_files.c",
		526,
		!DATUM_INDEX_TO_IDENTIFIER(tag_index) || tag_instance->tag_index==tag_index,
		csprintf(temporary, "i don't think %08x is a tag index", tag_index));

	return tag_instance;
}

boolean cache_file_header_verify(
	struct cache_file_header *header,
	char const *name,
	boolean fatal)
{
	if (header->header_signature != CACHE_FILE_HEADER_SIGNATURE ||
		header->footer_signature != CACHE_FILE_FOOTER_SIGNATURE ||
		header->size < 0 ||
		header->size > MAXIMUM_CACHE_FILE_SIZE ||
		strlen(header->name) > TAG_STRING_LENGTH)
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\cache\\cache_files.c",
				544,
				csprintf(
					temporary,
					"'%s' does not appear to be a cache file",
					name));
		}
	}
	else if (header->version != CACHE_FILE_VERSION)
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\cache\\cache_files.c",
				548,
				csprintf(
					temporary,
					"the cache file '%s' is an old version",
					name));
		}
	}
	else if (strcmp(header->build_number, BUILD_STRING))
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\cache\\cache_files.c",
				553,
				csprintf(
					temporary,
					"the cache file '%s' belongs to a different build (%s)",
					header->name,
					header->build_number));
		}
	}
	else
	{
		return TRUE;
	}

	return FALSE;
}

boolean cache_files_give_time_to_precache(
	char const *map_name)
{
	real progress;
	boolean result = FALSE;

	if (cache_files_precache_map_loaded(map_name))
	{
		result = TRUE;
	}
	else
	{
		if (cache_files_precache_in_progress() && !cache_files_precache_is_copying_map(map_name))
		{
			cache_files_precache_map_end();
		}

		if (cache_files_precache_in_progress())
		{
			short status = cache_files_precache_map_status(&progress);

			if (status == _cached_map_file_failed)
			{
				display_error_damaged_media();
			}
			else if (status == _cached_map_file_success)
			{
				cache_files_precache_map_end();
			}
		}
		else
		{
			cache_files_precache_set_priority(FALSE);

			if (!cache_files_precache_map_begin(map_name, FALSE))
			{
				display_error_damaged_media();
			}
		}
	}

	return result;
}

/* ---------- private code */
