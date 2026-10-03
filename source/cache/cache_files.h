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

struct scenario_structure_bsp_reference;

/* ---------- prototypes/CACHE_FILES.C */

void scenario_tags_unload(void);
unsigned long cache_files_get_checksum(void);
long scenario_tags_load(char const *name);
boolean scenario_structure_bsp_load(struct scenario_structure_bsp_reference *reference);
void scenario_structure_bsp_unload(struct scenario_structure_bsp_reference *reference);
unsigned long tag_get_group_tag(long tag_index);

/* ---------- prototypes/CACHE_FILES_WINDOWS.C */

boolean cache_files_precache_in_progress(void);
short cache_files_precache_map_status(real *progress);
void cache_files_precache_map_end(void);

/* ---------- globals */

/* ---------- public code */

#endif // __CACHE_FILES_H
