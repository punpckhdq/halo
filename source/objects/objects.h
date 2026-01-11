/*
OBJECTS.H

file has inline function assertions.
*/

#ifndef __OBJECTS_H
#define __OBJECTS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct object_header
{
	byte pad0_pad8[8];
	struct object* object;
};

struct object
{
	byte pad0_pad64[0x64];
	short type_id;
};

/* ---------- prototypes/OBJECT.C */

struct object* object_get_and_verify_type(long object_index, unsigned long type_mask);

void object_export_function_values(long object_index);

void object_render_debug(long object_index);

/* ---------- globals */

/* ---------- public code */

#endif // __OBJECTS_H
