/*
DATA.C

symbols in this file:
001089A0 0020:
	_data_allocation_size (0000)
001089C0 00d0:
	_data_initialize (0000)
00108A90 00b0:
	_datum_try_and_get (0000)
00108B40 00d0:
	_datum_get (0000)
00108C10 00b0:
	_data_verify (0000)
00108CC0 0030:
	_code_00108cc0 (0000)
00108CF0 0050:
	_data_new (0000)
00108D40 0030:
	_data_dispose (0000)
00108D70 0020:
	_data_make_invalid (0000)
00108D90 00a0:
	_datum_new_at_index (0000)
00108E30 00c0:
	_datum_new (0000)
00108EF0 0050:
	_datum_delete (0000)
00108F40 0090:
	_data_delete_all (0000)
00108FD0 0060:
	_data_iterator_new (0000)
00109030 00e0:
	_data_iterator_next (0000)
00109110 0090:
	_data_next_index (0000)
001091A0 0090:
	_data_prev_index (0000)
00109230 0110:
	_data_compact (0000)
00109340 0020:
	_data_make_valid (0000)
0027D310 0005:
	??_C@_04MEMAJGDJ@name?$AA@ (0000)
0027D318 0010:
	??_C@_0BA@HDDJANCC@maximum_count?$DO0?$AA@ (0000)
0027D328 001d:
	??_C@_0BN@PKENIDAC@c?3?2halo?2SOURCE?2memory?2data?4c?$AA@ (0000)
0027D348 002d:
	??_C@_0CN@EMICPANB@identifier?5?$HM?$HM?5?$CBdata?9?$DOidentifier_@ (0000)
0027D378 000c:
	??_C@_0M@FOPPJIAE@data?9?$DOvalid?$AA@ (0000)
0027D384 0029:
	??_C@_0CJ@PGGNIIAJ@?$CFs?5index?5?$CD?$CFd?5?$CI0x?$CFx?$CJ?5is?5unused?5or@ (0000)
0027D3B0 002a:
	??_C@_0CK@PGJBLDEL@?$CFs?5data?5array?5?$EA?$CFp?5is?5bad?5or?5not?5@ (0000)
0027D3DC 0016:
	??_C@_0BG@NCCMBKGF@iterator?9?$DOdata?9?$DOvalid?$AA@ (0000)
0027D3F4 0031:
	??_C@_0DB@COCANJJJ@uninitialized?5iterator?5passed?5to@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "data.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void datum_initialize(struct data_array *data, struct datum_header *header);

/* ---------- globals */

/* ---------- public code */

void data_verify(
	struct data_array *data)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 457, data);

	match_vassert(
		"c:\\halo\\SOURCE\\memory\\data.c",
		470,
		data->data &&
		data->signature=='d@t@' &&
		data->maximum_count>=0 &&
		data->count>=0 &&
		data->count<=data->maximum_count &&
		data->first_free_absolute_index>=0 &&
		data->first_free_absolute_index<=data->maximum_count &&
		data->actual_count>=0 &&
		data->actual_count<=data->count,
		csprintf(temporary, "%s data array @%p is bad or not allocated", data->name, data));

	return;
}

void datum_delete(
	struct data_array *data,
	long index)
{
	struct datum_header *header= (struct datum_header *)datum_get(data, index);
	header->identifier= 0;

	if (((short)index)<data->first_free_absolute_index)
	{
		data->first_free_absolute_index= index;
	}

	if (((short)index)+1==data->count)
	{
		do
		{
			header=(struct datum_header *)((byte *)header-data->size);
			data->count--;
		}
		while (data->count > 0 && !header->identifier);
	}
	data->actual_count--;

	return;
}

/* ---------- private code */
