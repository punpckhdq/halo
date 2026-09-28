/*
OVERHEAD_MAP.C

symbols in this file:
000CFB20 0010:
	_overhead_map_initialize (0000)
000CFB30 0010:
	_overhead_map_initialize_for_new_map (0000)
000CFB40 0010:
	_overhead_map_dispose_from_old_map (0000)
000CFB50 00e0:
	_overhead_map_post_rasterize (0000)
00270BD0 000c:
	_rdata_00270bd0 (0000)
00453CD0 0030:
	_bss_00453cd0 (0000)
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct
{
	boolean bss_00453cd0;
	short data_853cd2;
	short data_853cd4;
	short data_853cd6;
	long data_853cd8;
	long data_853cdc;
	long data_853ce0;
	double data_853ce4;
	short data_853cec;
	short data_853cee;
	long data_853cf0;
	void* data_853cf8;
} overhead_map_globals;

/* ---------- public code */

void overhead_map_initialize(
	void)
{
	return;
}

void overhead_map_initialize_for_new_map(
	void)
{
	return;
}

void overhead_map_dispose_from_old_map(
	void)
{
	overhead_map_globals.bss_00453cd0 = FALSE;

	return;
}

void overhead_map_post_rasterize(
	void* a1,
	void* a2,
	double* a3)
{
	double* v1;
	void* v2;

	if (overhead_map_globals.bss_00453cd0)
	{
		overhead_map_globals.data_853ce4 = *a3;

		v1 = a3;
		v2 = overhead_map_globals.data_853cf8;
	}

	return;
}

/* ---------- private code */
