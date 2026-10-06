/*
PREDICTED_RESOURCES.H

header included in hcex build.
*/

#ifndef __PREDICTED_RESOURCES_H
#define __PREDICTED_RESOURCES_H
#pragma once

/* ---------- constants */

enum
{
	_predicted_resource_bitmap = 0,
	_predicted_resource_sound,
	NUMBER_OF_PREDICTED_RESOURCE_TYPES /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct predicted_resource
{
	short type;
	short resource_index;
	long tag_index;
};

/* ---------- prototypes/PREDICTED_RESOURCES.C */

void predicted_resources_precache(struct tag_block *predicted_resources);

/* ---------- globals */

/* ---------- public code */

#endif // __PREDICTED_RESOURCES_H
