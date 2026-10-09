/*
PLACEHOLDER_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __PLACEHOLDER_DEFINITIONS_H
#define __PLACEHOLDER_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "objects.h"

/* ---------- constants */

enum
{
	PLACEHOLDER_DEFINITION_TAG = 'plac',
	PLACEHOLDER_DEFINITION_VERSION = 2,
};

/* ---------- macros */

/* ---------- structures */

struct placeholder_datum /* fake name */
{
	long definition_index;
	struct _object_datum object;
	byte unknown[88];
};

/* ---------- prototypes/PLACEHOLDER_DEFINITIONS.C */

void placeholder_initialize(void);
void placeholder_initialize_for_new_map(void);
void placeholder_dispose_from_old_map(void);
void placeholder_dispose(void);
void placeholder_place(long placeholder_index, struct scenario_placeholder_datum *scenario_placeholder);
boolean placeholder_new(long object_index);
void placeholder_delete(long placeholder_index);

/* ---------- globals */

/* ---------- public code */

#endif // __PLACEHOLDER_DEFINITIONS_H
