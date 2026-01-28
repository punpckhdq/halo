/*
OBJECT_DELETED_PROCS.C

symbols in this file:
00128700 0030:
	_object_deleted_procs_call (0000)
0030B378 000c:
	_object_deleted_procs (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "objects.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

object_deleted_proc object_deleted_procs[3];

/* ---------- public code */

void object_deleted_procs_call(
	long deleted_object_index)
{
	long i=0;

	do
	{
		(object_deleted_procs[i])(deleted_object_index);
		i++;
	}
	while (i<NUMBEROF(object_deleted_procs));

	return;
}

/* ---------- private code */
