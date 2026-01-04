/*
ERRORS.C

symbols in this file:
0007D860 0010:
	_errors_dispose (0000)
0007D870 0010:
	_errors_output_to_debug_file (0000)
0007D880 0010:
	_errors_overflow_suppression_enable (0000)
0007D890 0010:
	_error_get (0000)
0007D8A0 0130:
	_write_to_error_file (0000)
0007D9D0 0010:
	_code_0007d9d0 (0000)
0007D9E0 0020:
	_errors_initialize (0000)
0007DA00 0270:
	_error (0000)
0007DC70 0020:
	_errors_handle (0000)
0007DC90 0010:
	_errors_clear (0000)
00257C68 0015:
	??_C@_0BF@MNPBN@?$DMTIME?5UNAVAILABLE?$DO?5?5?$AA@ (0000)
00257C80 0020:
	??_C@_0CA@LIAFHJOP@?$CF02d?4?$CF02d?4?$CF02d?5?$CF02d?3?$CF02d?3?$CF02d?5?5?$AA@ (0000)
00257CA0 000d:
	??_C@_0N@MOPBLMMO@d?3?2debug?4txt?$AA@ (0000)
00257CB0 0018:
	??_C@_0BI@PFJPDCFK@reference?5address?3?5?$CFx?$AN?6?$AA@ (0000)
00257CC8 0019:
	??_C@_0BJ@KFNJAEPH@reference?5function?3?5?$CFs?$AN?6?$AA@ (0000)
00257CE4 0015:
	??_C@_0BF@OBFANCCF@_write_to_error_file?$AA@ (0000)
00257D00 0054:
	??_C@_0FE@EJMLAPFK@halobeta?5xbox?501?401?414?42342?$CICACH@ (0000)
00257D54 0005:
	??_C@_04FMMHFHOO@?$AN?6?$AN?6?$AA@ (0000)
00257D60 0047:
	??_C@_0EH@CIPINKCN@prefix_size?5?$CL?5copy_size?5?$CL?5new_si@ (0000)
00257DA8 0023:
	??_C@_0CD@BCANCLDH@?$FL?4?4?4too?5many?5errors?5to?5print?4?4?4?$FN@ (0000)
00257DCC 002c:
	??_C@_0CM@FGNANMFL@too?5many?5errors?0?5only?5printing?5t@ (0000)
00257DF8 003b:
	??_C@_0DL@DEAPBIIL@priority?$DO?$DN0?5?$CG?$CG?5priority?$DMNUMBER_O@ (0000)
00257E34 0020:
	??_C@_0CA@JEDAOIGL@c?3?2halo?2SOURCE?2cseries?2errors?4c?$AA@ (0000)
002DCD2C 0001:
	_data_002dcd2c (0000)
0031DF2C 0010:
	_bss_0031df2c (0000)
	_find_all_fucked_up_shit (0008)
	_fucked_up_shit_count (000c)
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"

#include <time.h>

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void write_to_error_file(char *string, boolean date)
{
	static unsigned char first_line= TRUE;

	char line[1024];
	long time_value;

	if (first_line)
	{
		first_line= FALSE;
		write_to_error_file("\r\n\r\n", FALSE);
		write_to_error_file("halobeta xbox 01.01.14.2342(CACHE) ----------------------------------------------\r\n", TRUE);
		sprintf(line, "reference function: %s\r\n", "_write_to_error_file");
		write_to_error_file(line, TRUE);
		sprintf(line, "reference address: %x\r\n", write_to_error_file);
		write_to_error_file(line, TRUE);
	}

	if (error_globals.output_to_debug_file)
	{
		FILE* handle= fopen("d:\\debug.txt", "a+b");
		if (handle)
		{
			if (date)
			{
				long timeptr;
				struct tm *_time;

				time(&timeptr);
				_time= localtime(&timeptr);
				if (_time)
				{
					fprintf(
					  handle,
					  "%02d.%02d.%02d %02d:%02d:%02d  ",
					  _time->tm_mon + 1,
					  _time->tm_mday,
					  _time->tm_year % 100,
					  _time->tm_hour,
					  _time->tm_min,
					  _time->tm_sec);
				}
				else
				{
					fprintf(handle, "<TIME UNAVAILABLE>  ");
				}
			}
			fprintf(handle, "%s", string);
			fclose(handle);
		}
	}

	return;
}

/* ---------- private code */
