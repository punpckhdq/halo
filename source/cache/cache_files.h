/*
CACHE_FILES.H

header included in hcex build.
*/

#ifndef __CACHE_FILES_H
#define __CACHE_FILES_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/CACHE_FILES.C */

unsigned long cache_files_get_checksum(void);

/* ---------- prototypes/CACHE_FILES_WINDOWS.C */

boolean cache_files_precache_in_progress(void);

short cache_files_precache_map_status(real *progress);

void cache_files_precache_map_end(void);

/* ---------- globals */

/* ---------- public code */

#endif // __CACHE_FILES_H
