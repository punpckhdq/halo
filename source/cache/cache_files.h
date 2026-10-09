/*
CACHE_FILES.H

header included in hcex build.
*/

#ifndef __CACHE_FILES_H
#define __CACHE_FILES_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_cached_map_file_in_progress = 0,
	_cached_map_file_success,
	_cached_map_file_failed,
	NUMBER_OF_CACHED_MAP_FILE_PRECACHE_STATES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/CACHE_FILES.C */

unsigned long cache_files_get_checksum(void);
boolean cache_files_give_time_to_precache(char const *map_name);
void scenario_tags_unload(void);
long scenario_tags_load(char const *name);
boolean scenario_structure_bsp_load(struct scenario_structure_bsp_reference *reference);
void scenario_structure_bsp_unload(struct scenario_structure_bsp_reference *reference);

/* ---------- prototypes/CACHE_FILES_WINDOWS.C */

boolean cache_files_precache_in_progress(void);

short cache_files_precache_map_status(real *progress);

boolean cache_files_precache_map_loaded(char *map_name);

void cache_files_precache_map_end(void);

unsigned long tag_get_group_tag(long tag_index);

/* ---------- globals */

/* ---------- public code */

#endif // __CACHE_FILES_H
