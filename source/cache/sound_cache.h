/*
SOUND_CACHE.H

header included in hcex build.
*/

#ifndef __SOUND_CACHE_H
#define __SOUND_CACHE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

#define sound_cache_sound_loaded(sound) _sound_cache_sound_request((sound), FALSE, FALSE, FALSE)

/* ---------- structures */

/* ---------- prototypes/XBOX_SOUND_CACHE.C */

void sound_cache_new(void);
void sound_cache_delete(void);
void sound_cache_open(void);
void sound_cache_flush(void);
void sound_cache_close(void);
void sound_cache_idle(void);
void sound_cache_sound_new(long tag_index, struct sound_permutation *sound);
void sound_cache_sound_delete(struct sound_permutation *sound);
boolean _sound_cache_sound_request(struct sound_permutation *sound, boolean block, boolean load, boolean reference);
void sound_cache_sound_finished(struct sound_permutation *sound);
void sound_cache_sound_hardware_lock(struct sound_permutation *sound);
void sound_cache_sound_hardware_unlock(struct sound_permutation *sound);
void sound_cache_debug_render(void);

/* ---------- globals */

extern boolean debug_sound_cache;
extern boolean debug_sound_reference_counts;

/* ---------- public code */

#endif // __SOUND_CACHE_H
