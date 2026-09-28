/*
SCENERY.H

header included in hcex build.
*/

#ifndef __SCENERY_H
#define __SCENERY_H
#pragma once

/* ---------- headers */

#include "objects.h"

/* ---------- constants */

enum
{
	_scenery_self_animated_bit = 0,
	NUMBER_OF_SCENERY_FLAGS,
};

/* ---------- macros */

#define scenery_get(index) ((struct scenery_datum *)object_get_and_verify_type(index, _object_mask_scenery))

/* ---------- structures */

struct _scenery_datum
{
	long flags;
};

struct scenery_datum
{
	long definition_index;
	struct _object_datum object;
	struct _scenery_datum scenery;
};

struct scenario_scenery_datum;

/* ---------- prototypes/SCENERY.C */

void scenery_initialize(void);
void scenery_initialize_for_new_map(void);
void scenery_dispose_from_old_map(void);
void scenery_dispose(void);
void scenery_place(long scenery_index, struct scenario_scenery_datum *scenario_scenery);
boolean scenery_new(long object_index);
void scenery_delete(long scenery_index);
boolean scenery_update(long scenery_index);
void scenery_animation_start(long scenery_index, long animation_graph_index, char const *animation_name);
void scenery_animation_start_at_frame(long scenery_index, long animation_graph_index, char const *animation_name, short frame_index);
short scenery_get_animation_time(long scenery_index);

/* ---------- globals */

/* ---------- public code */

#endif // __SCENERY_H
